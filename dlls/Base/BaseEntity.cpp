/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include	"Base/ExtDLL.h"
#include	"Util.h"
#include	"Base/CBase.h"
#include	"Base/SaveRestore.h"
#include	"Game/Client.h"
#include	"decals.h"
#include	"Game/GameRules.h"
#include	"Game/Game.h"

void EntvarsKeyvalue( entvars_t *pev, KeyValueData *pkvd );

void PM_Move ( struct playermove_s *ppmove, int server );
void PM_Init ( struct playermove_s *ppmove  );
char PM_FindTextureType( char *name );

extern Vector VecBModelOrigin( entvars_t* pevBModel );
extern DLL_GLOBAL Vector		g_vecAttackDir;
extern DLL_GLOBAL int			g_iSkillLevel;

static DLL_FUNCTIONS gFunctionTable = 
{
	GameDLLInit,				//pfnGameInit
	DispatchSpawn,				//pfnSpawn
	DispatchThink,				//pfnThink
	DispatchUse,				//pfnUse
	DispatchTouch,				//pfnTouch
	DispatchBlocked,			//pfnBlocked
	DispatchKeyValue,			//pfnKeyValue
	DispatchSave,				//pfnSave
	DispatchRestore,			//pfnRestore
	DispatchObjectCollsionBox,	//pfnAbsBox

	SaveWriteFields,			//pfnSaveWriteFields
	SaveReadFields,				//pfnSaveReadFields

	SaveGlobalState,			//pfnSaveGlobalState
	RestoreGlobalState,			//pfnRestoreGlobalState
	ResetGlobalState,			//pfnResetGlobalState

	ClientConnect,				//pfnClientConnect
	ClientDisconnect,			//pfnClientDisconnect
	ClientKill,					//pfnClientKill
	ClientPutInServer,			//pfnClientPutInServer
	ClientCommand,				//pfnClientCommand
	ClientUserInfoChanged,		//pfnClientUserInfoChanged
	ServerActivate,				//pfnServerActivate
	ServerDeactivate,			//pfnServerDeactivate

	PlayerPreThink,				//pfnPlayerPreThink
	PlayerPostThink,			//pfnPlayerPostThink

	StartFrame,					//pfnStartFrame
	ParmsNewLevel,				//pfnParmsNewLevel
	ParmsChangeLevel,			//pfnParmsChangeLevel

	GetGameDescription,         //pfnGetGameDescription    Returns string describing current .dll game.
	PlayerCustomization,        //pfnPlayerCustomization   Notifies .dll of new customization for player.

	SpectatorConnect,			//pfnSpectatorConnect      Called when spectator joins server
	SpectatorDisconnect,        //pfnSpectatorDisconnect   Called when spectator leaves the server
	SpectatorThink,				//pfnSpectatorThink        Called when spectator sends a command packet (usercmd_t)

	Sys_Error,					//pfnSys_Error				Called when engine has encountered an error

	PM_Move,					//pfnPM_Move
	PM_Init,					//pfnPM_Init				Server version of player movement initialization
	PM_FindTextureType,			//pfnPM_FindTextureType
	
	SetupVisibility,			//pfnSetupVisibility        Set up PVS and PAS for networking for this client
	UpdateClientData,			//pfnUpdateClientData       Set up data sent only to specific client
	AddToFullPack,				//pfnAddToFullPack
	CreateBaseline,				//pfnCreateBaseline			Tweak entity baseline for network encoding, allows setup of player baselines, too.
	RegisterEncoders,			//pfnRegisterEncoders		Callbacks for network encoding
	GetWeaponData,				//pfnGetWeaponData
	CmdStart,					//pfnCmdStart
	CmdEnd,						//pfnCmdEnd
	ConnectionlessPacket,		//pfnConnectionlessPacket
	GetHullBounds,				//pfnGetHullBounds
	CreateInstancedBaselines,   //pfnCreateInstancedBaselines
	InconsistentFile,			//pfnInconsistentFile
	AllowLagCompensation,		//pfnAllowLagCompensation
};

static void SetObjectCollisionBox( entvars_t *pev );

extern "C" {

	int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion )
{
	if ( !pFunctionTable || interfaceVersion != INTERFACE_VERSION )
	{
		return FALSE;
	}
	
	memcpy( pFunctionTable, &gFunctionTable, sizeof( DLL_FUNCTIONS ) );
	return TRUE;
}

int GetEntityAPI2( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion )
{
	if ( !pFunctionTable || *interfaceVersion != INTERFACE_VERSION )
	{
		// Tell engine what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return FALSE;
	}
	
	memcpy( pFunctionTable, &gFunctionTable, sizeof( DLL_FUNCTIONS ) );
	return TRUE;
}

}


int DispatchSpawn( edict_t *pent )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);

	if (pEntity)
	{
		// Initialize these or entities who don't link to the world won't have anything in here
		pEntity->pev->absmin = pEntity->pev->origin - Vector(1,1,1);
		pEntity->pev->absmax = pEntity->pev->origin + Vector(1,1,1);

		pEntity->Spawn();

		// Try to get the pointer again, in case the spawn function deleted the entity.
		// TODO: Spawn() should really return a code to ask that the entity be deleted, but
		// that would touch too much code for me to do that right now.
		pEntity = (CBaseEntity *)GET_PRIVATE(pent);

		if ( pEntity )
		{
			if ( g_pGameRules && !g_pGameRules->IsAllowedToSpawn( pEntity ) )
				return -1;	// return that this entity should be deleted
			if ( pEntity->pev->flags & FL_KILLME )
				return -1;
		}


		// Handle global stuff here
		if ( pEntity && pEntity->pev->globalname ) 
		{
			const globalentity_t *pGlobal = gGlobalState.EntityFromTable( pEntity->pev->globalname );
			if ( pGlobal )
			{
				// Already dead? delete
				if ( pGlobal->state == GLOBAL_DEAD )
					return -1;
				else if ( !FStrEq( STRING(gpGlobals->mapname), pGlobal->levelName ) )
					pEntity->MakeDormant();	// Hasn't been moved to this level yet, wait but stay alive
				// In this level & not dead, continue on as normal
			}
			else
			{
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd( pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON );
//				ALERT( at_console, "Added global entity %s (%s)\n", STRING(pEntity->pev->classname), STRING(pEntity->pev->globalname) );
			}
		}

	}

	return 0;
}

void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd )
{
	if ( !pkvd || !pentKeyvalue )
		return;

	EntvarsKeyvalue( VARS(pentKeyvalue), pkvd );

	// If the key was an entity variable, or there's no class set yet, don't look for the object, it may
	// not exist yet.
	if ( pkvd->fHandled || pkvd->szClassName == NULL )
		return;

	// Get the actualy entity object
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pentKeyvalue);

	if ( !pEntity )
		return;

	pEntity->KeyValue( pkvd );
}


// HACKHACK -- this is a hack to keep the node graph entity from "touching" things (like triggers)
// while it builds the graph
BOOL gTouchDisabled = FALSE;
void DispatchTouch( edict_t *pentTouched, edict_t *pentOther )
{
	if ( gTouchDisabled )
		return;

	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pentTouched);
	CBaseEntity *pOther = (CBaseEntity *)GET_PRIVATE( pentOther );

	if ( pEntity && pOther && ! ((pEntity->pev->flags | pOther->pev->flags) & FL_KILLME) )
		pEntity->Touch( pOther );
}


void DispatchUse( edict_t *pentUsed, edict_t *pentOther )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pentUsed);
	CBaseEntity *pOther = (CBaseEntity *)GET_PRIVATE(pentOther);

	if (pEntity && !(pEntity->pev->flags & FL_KILLME) )
		pEntity->Use( pOther, pOther, USE_TOGGLE, 0 );
}

void DispatchThink( edict_t *pent )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);
	if (pEntity)
	{
		if ( FBitSet( pEntity->pev->flags, FL_DORMANT ) )
			ALERT( at_error, "Dormant entity %s is thinking!!\n", STRING(pEntity->pev->classname) );
				
		pEntity->Think();
	}
}

void DispatchBlocked( edict_t *pentBlocked, edict_t *pentOther )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE( pentBlocked );
	CBaseEntity *pOther = (CBaseEntity *)GET_PRIVATE( pentOther );

	if (pEntity)
		pEntity->Blocked( pOther );
}

void DispatchSave( edict_t *pent, SAVERESTOREDATA *pSaveData )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);
	
	if ( pEntity && pSaveData )
	{
		ENTITYTABLE *pTable = &pSaveData->pTable[ pSaveData->currentIndex ];

		if ( pTable->pent != pent )
			ALERT( at_error, "ENTITY TABLE OR INDEX IS WRONG!!!!\n" );

		if ( pEntity->ObjectCaps() & FCAP_DONT_SAVE )
			return;

		// These don't use ltime & nextthink as times really, but we'll fudge around it.
		if ( pEntity->pev->movetype == MOVETYPE_PUSH )
		{
			float delta = pEntity->pev->nextthink - pEntity->pev->ltime;
			pEntity->pev->ltime = gpGlobals->time;
			pEntity->pev->nextthink = pEntity->pev->ltime + delta;
		}

		pTable->location = pSaveData->size;		// Remember entity position for file I/O
		pTable->classname = pEntity->pev->classname;	// Remember entity class for respawn

		CSave saveHelper( pSaveData );
		pEntity->Save( saveHelper );

		pTable->size = pSaveData->size - pTable->location;	// Size of entity block is data size written to block
	}
}


// Find the matching global entity.  Spit out an error if the designer made entities of
// different classes with the same global name
CBaseEntity *FindGlobalEntity( string_t classname, string_t globalname )
{
	edict_t *pent = FIND_ENTITY_BY_STRING( NULL, "globalname", STRING(globalname) );
	CBaseEntity *pReturn = CBaseEntity::Instance( pent );
	if ( pReturn )
	{
		if ( !FClassnameIs( pReturn->pev, STRING(classname) ) )
		{
			ALERT( at_console, "Global entity found %s, wrong class %s\n", STRING(globalname), STRING(pReturn->pev->classname) );
			pReturn = NULL;
		}
	}

	return pReturn;
}


int DispatchRestore( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);

	if ( pEntity && pSaveData )
	{
		entvars_t tmpVars;
		Vector oldOffset;

		CRestore restoreHelper( pSaveData );
		if ( globalEntity )
		{
			CRestore tmpRestore( pSaveData );
			tmpRestore.PrecacheMode( 0 );
			tmpRestore.ReadEntVars( "ENTVARS", &tmpVars );

			// HACKHACK - reset the save pointers, we're going to restore for real this time
			pSaveData->size = pSaveData->pTable[pSaveData->currentIndex].location;
			pSaveData->pCurrentData = pSaveData->pBaseData + pSaveData->size;
			// -------------------


			const globalentity_t *pGlobal = gGlobalState.EntityFromTable( tmpVars.globalname );
			
			// Don't overlay any instance of the global that isn't the latest
			// pSaveData->szCurrentMapName is the level this entity is coming from
			// pGlobla->levelName is the last level the global entity was active in.
			// If they aren't the same, then this global update is out of date.
			if ( !FStrEq( pSaveData->szCurrentMapName, pGlobal->levelName ) )
				return 0;

			// Compute the new global offset
			oldOffset = pSaveData->vecLandmarkOffset;
			CBaseEntity *pNewEntity = FindGlobalEntity( tmpVars.classname, tmpVars.globalname );
			if ( pNewEntity )
			{
//				ALERT( at_console, "Overlay %s with %s\n", STRING(pNewEntity->pev->classname), STRING(tmpVars.classname) );
				// Tell the restore code we're overlaying a global entity from another level
				restoreHelper.SetGlobalMode( 1 );	// Don't overwrite global fields
				pSaveData->vecLandmarkOffset = (pSaveData->vecLandmarkOffset - pNewEntity->pev->mins) + tmpVars.mins;
				pEntity = pNewEntity;// we're going to restore this data OVER the old entity
				pent = ENT( pEntity->pev );
				// Update the global table to say that the global definition of this entity should come from this level
				gGlobalState.EntityUpdate( pEntity->pev->globalname, gpGlobals->mapname );
			}
			else
			{
				// This entity will be freed automatically by the engine.  If we don't do a restore on a matching entity (below)
				// or call EntityUpdate() to move it to this level, we haven't changed global state at all.
				return 0;
			}

		}

		if ( pEntity->ObjectCaps() & FCAP_MUST_SPAWN )
		{
			pEntity->Restore( restoreHelper );
			pEntity->Spawn();
		}
		else
		{
			pEntity->Restore( restoreHelper );
			pEntity->Precache( );
		}

		// Again, could be deleted, get the pointer again.
		pEntity = (CBaseEntity *)GET_PRIVATE(pent);

#if 0
		if ( pEntity && pEntity->pev->globalname && globalEntity ) 
		{
			ALERT( at_console, "Global %s is %s\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->model) );
		}
#endif

		// Is this an overriding global entity (coming over the transition), or one restoring in a level
		if ( globalEntity )
		{
//			ALERT( at_console, "After: %f %f %f %s\n", pEntity->pev->origin.x, pEntity->pev->origin.y, pEntity->pev->origin.z, STRING(pEntity->pev->model) );
			pSaveData->vecLandmarkOffset = oldOffset;
			if ( pEntity )
			{
				UTIL_SetOrigin( pEntity->pev, pEntity->pev->origin );
				pEntity->OverrideReset();
			}
		}
		else if ( pEntity && pEntity->pev->globalname ) 
		{
			const globalentity_t *pGlobal = gGlobalState.EntityFromTable( pEntity->pev->globalname );
			if ( pGlobal )
			{
				// Already dead? delete
				if ( pGlobal->state == GLOBAL_DEAD )
					return -1;
				else if ( !FStrEq( STRING(gpGlobals->mapname), pGlobal->levelName ) )
				{
					pEntity->MakeDormant();	// Hasn't been moved to this level yet, wait but stay alive
				}
				// In this level & not dead, continue on as normal
			}
			else
			{
				ALERT( at_error, "Global Entity %s (%s) not in table!!!\n", STRING(pEntity->pev->globalname), STRING(pEntity->pev->classname) );
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd( pEntity->pev->globalname, gpGlobals->mapname, GLOBAL_ON );
			}
		}
	}
	return 0;
}


void DispatchObjectCollsionBox( edict_t *pent )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(pent);
	if (pEntity)
	{
		pEntity->SetObjectCollisionBox();
	}
	else
		SetObjectCollisionBox( &pent->v );
}


void SaveWriteFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount )
{
	CSave saveHelper( pSaveData );
	saveHelper.WriteFields( pname, pBaseData, pFields, fieldCount );
}


void SaveReadFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount )
{
	CRestore restoreHelper( pSaveData );
	restoreHelper.ReadFields( pname, pBaseData, pFields, fieldCount );
}


edict_t * EHANDLE::Get( void ) 
{ 
	if (m_pent)
	{
		if (m_pent->serialnumber == m_serialnumber) 
			return m_pent; 
		else
			return NULL;
	}
	return NULL; 
};

edict_t * EHANDLE::Set( edict_t *pent ) 
{ 
	m_pent = pent;  
	if (pent) 
		m_serialnumber = m_pent->serialnumber; 
	return pent; 
};


EHANDLE :: operator CBaseEntity *() 
{ 
	return (CBaseEntity *)GET_PRIVATE( Get( ) ); 
};


CBaseEntity * EHANDLE :: operator = (CBaseEntity *pEntity)
{
	if (pEntity)
	{
		m_pent = ENT( pEntity->pev );
		if (m_pent)
			m_serialnumber = m_pent->serialnumber;
	}
	else
	{
		m_pent = NULL;
		m_serialnumber = 0;
	}
	return pEntity;
}

EHANDLE :: operator int ()
{
	return Get() != NULL;
}

CBaseEntity * EHANDLE :: operator -> ()
{
	return (CBaseEntity *)GET_PRIVATE( Get( ) ); 
}


// give health
int CBaseEntity :: TakeHealth( float flHealth, int bitsDamageType )
{
	if (!pev->takedamage)
		return 0;

// heal
	if ( pev->health >= pev->max_health )
		return 0;

	pev->health += flHealth;

	if (pev->health > pev->max_health)
		pev->health = pev->max_health;

	return 1;
}

// inflict damage on this entity.  bitsDamageType indicates type of damage inflicted, ie: DMG_CRUSH

int CBaseEntity :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	Vector			vecTemp;

	if (!pev->takedamage)
		return 0;

	// TODO: some entity types may be immune or resistant to some bitsDamageType
	
	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin). 
	if ( pevAttacker == pevInflictor )	
	{
		vecTemp = pevInflictor->origin - ( VecBModelOrigin(pev) );
	}
	else
	// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - ( VecBModelOrigin(pev) );
	}

// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();
		
// save damage based on the target's armor level

// figure momentum add (don't let hurt brushes or other triggers move player)
	if ((!FNullEnt(pevInflictor)) && (pev->movetype == MOVETYPE_WALK || pev->movetype == MOVETYPE_STEP) && (pevAttacker->solid != SOLID_TRIGGER) )
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();

		float flForce = flDamage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;
		
		if (flForce > 1000.0) 
			flForce = 1000.0;
		pev->velocity = pev->velocity + vecDir * flForce;
	}

// do the damage
	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		Killed( pevAttacker, GIB_NORMAL );
		return 0;
	}

	return 1;
}


void CBaseEntity :: Killed( entvars_t *pevAttacker, int iGib )
{
	pev->takedamage = DAMAGE_NO;
	pev->deadflag = DEAD_DEAD;
	UTIL_Remove( this );
}


CBaseEntity *CBaseEntity::GetNextTarget( void )
{
	if ( FStringNull( pev->target ) )
		return NULL;
	edict_t *pTarget = FIND_ENTITY_BY_TARGETNAME ( NULL, STRING(pev->target) );
	if ( FNullEnt(pTarget) )
		return NULL;

	return Instance( pTarget );
}

// Global Savedata for Delay
TYPEDESCRIPTION	CBaseEntity::m_SaveData[] = 
{
	DEFINE_FIELD( CBaseEntity, m_pGoalEnt, FIELD_CLASSPTR ),

	DEFINE_FIELD( CBaseEntity, m_pfnThink, FIELD_FUNCTION ),		// TODO: Build table of these!!!
	DEFINE_FIELD( CBaseEntity, m_pfnTouch, FIELD_FUNCTION ),
	DEFINE_FIELD( CBaseEntity, m_pfnUse, FIELD_FUNCTION ),
	DEFINE_FIELD( CBaseEntity, m_pfnBlocked, FIELD_FUNCTION ),
};


int CBaseEntity::Save( CSave &save )
{
	if ( save.WriteEntVars( "ENTVARS", pev ) )
		return save.WriteFields( "BASE", this, m_SaveData, ARRAYSIZE(m_SaveData) );

	return 0;
}

int CBaseEntity::Restore( CRestore &restore )
{
	int status;

	status = restore.ReadEntVars( "ENTVARS", pev );
	if ( status )
		status = restore.ReadFields( "BASE", this, m_SaveData, ARRAYSIZE(m_SaveData) );

    if ( pev->modelindex != 0 && !FStringNull(pev->model) )
	{
		Vector mins, maxs;
		mins = pev->mins;	// Set model is about to destroy these
		maxs = pev->maxs;


		PRECACHE_MODEL( (char *)STRING(pev->model) );
		SET_MODEL(ENT(pev), STRING(pev->model));
		UTIL_SetSize(pev, mins, maxs);	// Reset them
	}

	return status;
}


// Initialize absmin & absmax to the appropriate box
void SetObjectCollisionBox( entvars_t *pev )
{
	if ( (pev->solid == SOLID_BSP) && 
		 (pev->angles.x || pev->angles.y|| pev->angles.z) )
	{	// expand for rotation
		float		max, v;
		int			i;

		max = 0;
		for (i=0 ; i<3 ; i++)
		{
			v = fabs( ((float *)pev->mins)[i]);
			if (v > max)
				max = v;
			v = fabs( ((float *)pev->maxs)[i]);
			if (v > max)
				max = v;
		}
		for (i=0 ; i<3 ; i++)
		{
			((float *)pev->absmin)[i] = ((float *)pev->origin)[i] - max;
			((float *)pev->absmax)[i] = ((float *)pev->origin)[i] + max;
		}
	}
	else
	{
		pev->absmin = pev->origin + pev->mins;
		pev->absmax = pev->origin + pev->maxs;
	}

	pev->absmin.x -= 1;
	pev->absmin.y -= 1;
	pev->absmin.z -= 1;
	pev->absmax.x += 1;
	pev->absmax.y += 1;
	pev->absmax.z += 1;
}


void CBaseEntity::SetObjectCollisionBox( void )
{
	::SetObjectCollisionBox( pev );
}


int	CBaseEntity :: Intersects( CBaseEntity *pOther )
{
	if ( pOther->pev->absmin.x > pev->absmax.x ||
		 pOther->pev->absmin.y > pev->absmax.y ||
		 pOther->pev->absmin.z > pev->absmax.z ||
		 pOther->pev->absmax.x < pev->absmin.x ||
		 pOther->pev->absmax.y < pev->absmin.y ||
		 pOther->pev->absmax.z < pev->absmin.z )
		 return 0;
	return 1;
}

void CBaseEntity :: MakeDormant( void )
{
	SetBits( pev->flags, FL_DORMANT );
	
	// Don't touch
	pev->solid = SOLID_NOT;
	// Don't move
	pev->movetype = MOVETYPE_NONE;
	// Don't draw
	SetBits( pev->effects, EF_NODRAW );
	// Don't think
	pev->nextthink = 0;
	// Relink
	UTIL_SetOrigin( pev, pev->origin );
}

int CBaseEntity :: IsDormant( void )
{
	return FBitSet( pev->flags, FL_DORMANT );
}

BOOL CBaseEntity :: IsInWorld( void )
{
	// increased from +/-4096 to +/-32768, and 2000 to 8000 -Admer
	// position 
	if ( pev->origin.x >=  MAX_EIW_DISTANCE ) return FALSE;
	if ( pev->origin.y >=  MAX_EIW_DISTANCE ) return FALSE;
	if ( pev->origin.z >=  MAX_EIW_DISTANCE ) return FALSE;
	if ( pev->origin.x <= -MAX_EIW_DISTANCE ) return FALSE;
	if ( pev->origin.y <= -MAX_EIW_DISTANCE ) return FALSE;
	if ( pev->origin.z <= -MAX_EIW_DISTANCE ) return FALSE;
	// speed
	if ( pev->velocity.x >=  MAX_EIW_VELOCITY ) return FALSE;
	if ( pev->velocity.y >=  MAX_EIW_VELOCITY ) return FALSE;
	if ( pev->velocity.z >=  MAX_EIW_VELOCITY ) return FALSE;
	if ( pev->velocity.x <= -MAX_EIW_VELOCITY ) return FALSE;
	if ( pev->velocity.y <= -MAX_EIW_VELOCITY ) return FALSE;
	if ( pev->velocity.z <= -MAX_EIW_VELOCITY ) return FALSE;

	return TRUE;
}

int CBaseEntity::ShouldToggle( USE_TYPE useType, BOOL currentState )
{
	if ( useType != USE_TOGGLE && useType != USE_SET )
	{
		if ( (currentState && useType == USE_ON) || (!currentState && useType == USE_OFF) )
			return 0;
	}
	return 1;
}


int	CBaseEntity :: DamageDecal( int bitsDamageType )
{
	if ( pev->rendermode == kRenderTransAlpha )
		return -1;

	if ( pev->rendermode != kRenderNormal )
		return DECAL_BPROOF1;

	return DECAL_GUNSHOT1 + RANDOM_LONG(0,4);
}

void CBaseEntity::GetKeyValue(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "target"))
		sprintf(&szTheValue, "%s", STRING(pev->target));

	else if (FStrEq(szTargetKeyName, "targetname"))
		sprintf(&szTheValue, "%s", STRING(pev->targetname));

	else if (FStrEq(szTargetKeyName, "netname"))
		sprintf(&szTheValue, "%s", STRING(pev->netname));
	
	else if (FStrEq(szTargetKeyName, "message"))
		sprintf(&szTheValue, "%s", STRING(pev->message));

	else if (FStrEq(szTargetKeyName, "classname"))
		sprintf(&szTheValue, "%s", STRING(pev->classname));

	else if (FStrEq(szTargetKeyName, "globalname"))
		sprintf(&szTheValue, "%s", STRING(pev->globalname));

	else if (FStrEq(szTargetKeyName, "origin"))
		sprintf(&szTheValue, "%f %f %f", pev->origin.x, pev->origin.y, pev->origin.z);

	else if (FStrEq(szTargetKeyName, "oldorigin"))
		sprintf(&szTheValue, "%f %f %f", pev->oldorigin.x, pev->oldorigin.y, pev->oldorigin.z);

	else if (FStrEq(szTargetKeyName, "velocity"))
		sprintf(&szTheValue, "%f %f %f", pev->velocity.x, pev->velocity.y, pev->velocity.z);

	else if (FStrEq(szTargetKeyName, "movedir"))
		sprintf(&szTheValue, "%f %f %f", pev->movedir.x, pev->movedir.y, pev->movedir.z);

	else if (FStrEq(szTargetKeyName, "angles"))
		sprintf(&szTheValue, "%f %f %f", pev->angles.x, pev->angles.y, pev->angles.z);

	else if (FStrEq(szTargetKeyName, "avelocity"))
		sprintf(&szTheValue, "%f %f %f", pev->avelocity.x, pev->avelocity.y, pev->avelocity.z);

	else if (FStrEq(szTargetKeyName, "punchangle"))
		sprintf(&szTheValue, "%f %f %f", pev->punchangle.x, pev->punchangle.y, pev->punchangle.z);

	else if (FStrEq(szTargetKeyName, "v_angle")) // TBH you could've just used env_viewsway instead of this, you stubborn one
		sprintf(&szTheValue, "%f %f %f", pev->v_angle.x, pev->v_angle.y, pev->v_angle.z);

	else if (FStrEq(szTargetKeyName, "endpos"))
		sprintf(&szTheValue, "%f %f %f", pev->endpos.x, pev->endpos.y, pev->endpos.z);

	else if (FStrEq(szTargetKeyName, "startpos"))
		sprintf(&szTheValue, "%f %f %f", pev->startpos.x, pev->startpos.y, pev->startpos.z);

	else if (FStrEq(szTargetKeyName, "impacttime"))
		sprintf(&szTheValue, "%f", pev->impacttime);

	else if (FStrEq(szTargetKeyName, "starttime"))
		sprintf(&szTheValue, "%f", pev->starttime);

	else if (FStrEq(szTargetKeyName, "fixangle"))
		sprintf(&szTheValue, "%d", pev->fixangle);

	else if (FStrEq(szTargetKeyName, "idealpitch"))
		sprintf(&szTheValue, "%f", pev->idealpitch);

	else if (FStrEq(szTargetKeyName, "pitch_speed"))
		sprintf(&szTheValue, "%f", pev->pitch_speed);

	else if (FStrEq(szTargetKeyName, "ideal_yaw"))
		sprintf(&szTheValue, "%f", pev->ideal_yaw);

	else if (FStrEq(szTargetKeyName, "yaw_speed"))
		sprintf(&szTheValue, "%f", pev->yaw_speed);

	else if (FStrEq(szTargetKeyName, "modelindex"))
		sprintf(&szTheValue, "%d", pev->modelindex);

	else if (FStrEq(szTargetKeyName, "model"))
		sprintf(&szTheValue, "%s", STRING(pev->model));

	else if (FStrEq(szTargetKeyName, "viewmodel"))
		sprintf(&szTheValue, "%s", STRING(pev->viewmodel));

	else if (FStrEq(szTargetKeyName, "weaponmodel"))
		sprintf(&szTheValue, "%s", STRING(pev->weaponmodel));

	else if (FStrEq(szTargetKeyName, "absmin"))
		sprintf(&szTheValue, "%f %f %f", pev->absmin.x, pev->absmin.y, pev->absmin.z);

	else if (FStrEq(szTargetKeyName, "absmax"))
		sprintf(&szTheValue, "%f %f %f", pev->absmax.x, pev->absmax.y, pev->absmax.z);

	else if (FStrEq(szTargetKeyName, "mins"))
		sprintf(&szTheValue, "%f %f %f", pev->mins.x, pev->mins.y, pev->mins.z);

	else if (FStrEq(szTargetKeyName, "maxs"))
		sprintf(&szTheValue, "%f %f %f", pev->maxs.x, pev->maxs.y, pev->maxs.z);

	else if (FStrEq(szTargetKeyName, "size"))
		sprintf(&szTheValue, "%f %f %f", pev->size.x, pev->size.y, pev->size.z);

	else if (FStrEq(szTargetKeyName, "ltime"))
		sprintf(&szTheValue, "%f", pev->ltime);

	else if (FStrEq(szTargetKeyName, "nextthink"))
		sprintf(&szTheValue, "%f", pev->nextthink);
	
	else if (FStrEq(szTargetKeyName, "movetype"))
		sprintf(&szTheValue, "%d", pev->movetype);

	else if (FStrEq(szTargetKeyName, "solid"))
		sprintf(&szTheValue, "%d", pev->solid);

	else if (FStrEq(szTargetKeyName, "skin"))
		sprintf(&szTheValue, "%d", pev->skin);

	else if (FStrEq(szTargetKeyName, "body"))
		sprintf(&szTheValue, "%d", pev->body);

	else if (FStrEq(szTargetKeyName, "effects"))
		sprintf(&szTheValue, "%d", pev->effects);
	
	else if (FStrEq(szTargetKeyName, "gravity"))
		sprintf(&szTheValue, "%f", pev->gravity);

	else if (FStrEq(szTargetKeyName, "friction"))
		sprintf(&szTheValue, "%f", pev->friction);

	else if (FStrEq(szTargetKeyName, "light_level"))
		sprintf(&szTheValue, "%d", pev->light_level);

	else if (FStrEq(szTargetKeyName, "sequence"))
		sprintf(&szTheValue, "%d", pev->sequence);

	else if (FStrEq(szTargetKeyName, "gaitsequence"))
		sprintf(&szTheValue, "%d", pev->gaitsequence);

	else if (FStrEq(szTargetKeyName, "frame"))
		sprintf(&szTheValue, "%f", pev->frame);

	else if (FStrEq(szTargetKeyName, "animtime"))
		sprintf(&szTheValue, "%f", pev->animtime);

	else if (FStrEq(szTargetKeyName, "framerate"))
		sprintf(&szTheValue, "%f", pev->framerate);

	else if (FStrEq(szTargetKeyName, "controller"))
		sprintf(&szTheValue, "%s", pev->controller);

	else if (FStrEq(szTargetKeyName, "blending"))
		sprintf(&szTheValue, "%s", pev->blending);

	else if (FStrEq(szTargetKeyName, "scale"))
		sprintf(&szTheValue, "%f", pev->scale);

	else if (FStrEq(szTargetKeyName, "rendermode"))
		sprintf(&szTheValue, "%d", pev->rendermode);

	else if (FStrEq(szTargetKeyName, "renderamt"))
		sprintf(&szTheValue, "%f", pev->renderamt);

	else if (FStrEq(szTargetKeyName, "rendercolor"))
		sprintf(&szTheValue, "%f %f %f", pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z);

	else if (FStrEq(szTargetKeyName, "renderfx"))
		sprintf(&szTheValue, "%d", pev->renderfx);

	else if (FStrEq(szTargetKeyName, "health"))
		sprintf(&szTheValue, "%f", pev->health);

	else if (FStrEq(szTargetKeyName, "frags"))
		sprintf(&szTheValue, "%f", pev->frags);

	else if (FStrEq(szTargetKeyName, "weapons"))
		sprintf(&szTheValue, "%d", pev->weapons);

	else if (FStrEq(szTargetKeyName, "takedamage"))
		sprintf(&szTheValue, "%f", pev->takedamage);

	else if (FStrEq(szTargetKeyName, "deadflag"))
		sprintf(&szTheValue, "%d", pev->deadflag);

	else if (FStrEq(szTargetKeyName, "view_ofs"))
		sprintf(&szTheValue, "%f %f %f", pev->view_ofs.x, pev->view_ofs.y, pev->view_ofs.z);

	else if (FStrEq(szTargetKeyName, "button"))
		sprintf(&szTheValue, "%d", pev->button);

	else if (FStrEq(szTargetKeyName, "impulse"))
		sprintf(&szTheValue, "%d", pev->impulse);

	else if (FStrEq(szTargetKeyName, "max_health"))
		sprintf(&szTheValue, "%f", pev->max_health);

	else if (FStrEq(szTargetKeyName, "teleport_time"))
		sprintf(&szTheValue, "%f", pev->teleport_time);

	else if (FStrEq(szTargetKeyName, "armortype"))
		sprintf(&szTheValue, "%f", pev->armortype);

	else if (FStrEq(szTargetKeyName, "armorvalue"))
		sprintf(&szTheValue, "%f", pev->armorvalue);

	else if (FStrEq(szTargetKeyName, "waterlevel"))
		sprintf(&szTheValue, "%d", pev->waterlevel);

	else if (FStrEq(szTargetKeyName, "watertype"))
		sprintf(&szTheValue, "%d", pev->watertype);

	else if (FStrEq(szTargetKeyName, "dmg_take"))
		sprintf(&szTheValue, "%f", pev->dmg_take);
	
	else if (FStrEq(szTargetKeyName, "dmg_save"))
		sprintf(&szTheValue, "%f", pev->dmg_save);

	else if (FStrEq(szTargetKeyName, "dmg"))
		sprintf(&szTheValue, "%f", pev->dmg);

	else if (FStrEq(szTargetKeyName, "dmgtime"))
		sprintf(&szTheValue, "%f", pev->dmgtime);

	else if (FStrEq(szTargetKeyName, "noise"))
		sprintf(&szTheValue, "%s", STRING(pev->noise));

	else if (FStrEq(szTargetKeyName, "noise1"))
		sprintf(&szTheValue, "%s", STRING(pev->noise1));

	else if (FStrEq(szTargetKeyName, "noise2"))
		sprintf(&szTheValue, "%s", STRING(pev->noise2));

	else if (FStrEq(szTargetKeyName, "noise3"))
		sprintf(&szTheValue, "%s", STRING(pev->noise3));

	else
		GetKeyValueBase(szTheValue, szTargetKeyName);

}


void CBaseEntity :: SetKeyValue(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "classname")) // Seriously, who would even want to change THIS?!
		pev->classname = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "globalname"))
		pev->globalname = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "origin"))
	{
		sscanf(szTheValue, "%f %f %f", &pev->origin.x, &pev->origin.y, &pev->origin.z);
		UTIL_SetOrigin(pev, pev->origin);
	}

	else if (FStrEq(szTargetKeyName, "oldorigin"))
		sscanf(szTheValue, "%f %f %f", &pev->oldorigin.x, &pev->oldorigin.y, &pev->oldorigin.z);

	else if (FStrEq(szTargetKeyName, "velocity"))
		sscanf(szTheValue, "%f %f %f", &pev->velocity.x, &pev->velocity.y, &pev->velocity.z);

	else if (FStrEq(szTargetKeyName, "movedir"))
		sscanf(szTheValue, "%f %f %f", &pev->movedir.x, &pev->movedir.y, &pev->movedir.z);

	else if (FStrEq(szTargetKeyName, "angles"))
		sscanf(szTheValue, "%f %f %f", &pev->angles.x, &pev->angles.y, &pev->angles.z);

	else if (FStrEq(szTargetKeyName, "avelocity"))
		sscanf(szTheValue, "%f %f %f", &pev->avelocity.x, &pev->avelocity.y, &pev->avelocity.z);

	else if (FStrEq(szTargetKeyName, "punchangle"))
		sscanf(szTheValue, "%f %f %f", &pev->punchangle.x, &pev->punchangle.y, &pev->punchangle.z);

	else if (FStrEq(szTargetKeyName, "v_angle")) // You're a mad man
		sscanf(szTheValue, "%f %f %f", &pev->v_angle.x, &pev->v_angle.y, &pev->v_angle.z);

	else if (FStrEq(szTargetKeyName, "endpos"))
		sscanf(szTheValue, "%f %f %f", &pev->endpos.x, &pev->endpos.y, &pev->endpos.z);

	else if (FStrEq(szTargetKeyName, "startpos"))
		sscanf(szTheValue, "%f %f %f", &pev->startpos.x, &pev->startpos.y, &pev->startpos.z);

	else if (FStrEq(szTargetKeyName, "impacttime"))
		sscanf(szTheValue, "%f", &pev->impacttime);

	else if (FStrEq(szTargetKeyName, "starttime"))
		sscanf(szTheValue, "%f", &pev->starttime);

	else if (FStrEq(szTargetKeyName, "fixangle"))
		sscanf(szTheValue, "%d", &pev->fixangle);

	else if (FStrEq(szTargetKeyName, "starttime"))
		sscanf(szTheValue, "%f", &pev->starttime);

	else if (FStrEq(szTargetKeyName, "idealpitch"))
		sscanf(szTheValue, "%f", &pev->idealpitch);

	else if (FStrEq(szTargetKeyName, "pitch_speed"))
		sscanf(szTheValue, "%f", &pev->pitch_speed);

	else if (FStrEq(szTargetKeyName, "ideal_yaw"))
		sscanf(szTheValue, "%f", &pev->ideal_yaw);

	else if (FStrEq(szTargetKeyName, "modelindex"))
		sscanf(szTheValue, "%d", &pev->modelindex);

	else if (FStrEq(szTargetKeyName, "model"))
		pev->model = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "viewmodel"))
		pev->viewmodel = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "weaponmodel"))
		pev->weaponmodel = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "absmin"))
		sscanf(szTheValue, "%f %f %f", &pev->absmin.x, &pev->absmin.y, &pev->absmin.z);

	else if (FStrEq(szTargetKeyName, "absmax"))
		sscanf(szTheValue, "%f %f %f", &pev->absmax.x, &pev->absmax.y, &pev->absmax.z);

	else if (FStrEq(szTargetKeyName, "mins"))
		sscanf(szTheValue, "%f %f %f", &pev->mins.x, &pev->mins.y, &pev->mins.z);

	else if (FStrEq(szTargetKeyName, "maxs"))
		sscanf(szTheValue, "%f %f %f", &pev->maxs.x, &pev->maxs.y, &pev->maxs.z);

	else if (FStrEq(szTargetKeyName, "size"))
		sscanf(szTheValue, "%f %f %f", &pev->size.x, &pev->size.y, &pev->size.z);

	else if (FStrEq(szTargetKeyName, "ltime"))
		sscanf(szTheValue, "%f", &pev->ltime);

	else if (FStrEq(szTargetKeyName, "nextthink"))
		sscanf(szTheValue, "%f", &pev->nextthink);

	else if (FStrEq(szTargetKeyName, "movetype"))
		sscanf(szTheValue, "%d", &pev->movetype);

	else if (FStrEq(szTargetKeyName, "solid"))
		sscanf(szTheValue, "%d", &pev->solid);

	else if (FStrEq(szTargetKeyName, "skin"))
		sscanf(szTheValue, "%d", &pev->skin);

	else if (FStrEq(szTargetKeyName, "body"))
		sscanf(szTheValue, "%d", &pev->body);

	else if (FStrEq(szTargetKeyName, "effects"))
		sscanf(szTheValue, "%d", &pev->effects);

	else if (FStrEq(szTargetKeyName, "gravity"))
		sscanf(szTheValue, "%f", &pev->gravity);

	else if (FStrEq(szTargetKeyName, "friction"))
		sscanf(szTheValue, "%f", &pev->friction);
	
	else if (FStrEq(szTargetKeyName, "light_level"))
		sscanf(szTheValue, "%d", &pev->light_level);

	else if (FStrEq(szTargetKeyName, "sequence"))
		sscanf(szTheValue, "%d", &pev->sequence);

	else if (FStrEq(szTargetKeyName, "gaitsequence"))
		sscanf(szTheValue, "%d", &pev->gaitsequence);

	else if (FStrEq(szTargetKeyName, "frame"))
		sscanf(szTheValue, "%f", &pev->frame);

	else if (FStrEq(szTargetKeyName, "animtime"))
		sscanf(szTheValue, "%f", &pev->animtime);

	else if (FStrEq(szTargetKeyName, "framerate"))
		sscanf(szTheValue, "%f", &pev->framerate);

	else if (FStrEq(szTargetKeyName, "controller"))
		sscanf(szTheValue, "%s", &pev->controller);

	else if (FStrEq(szTargetKeyName, "blending"))
		sscanf(szTheValue, "%s", &pev->blending);

	else if (FStrEq(szTargetKeyName, "scale"))
		sscanf(szTheValue, "%f", &pev->scale);

	else if (FStrEq(szTargetKeyName, "rendermode"))
		sscanf(szTheValue, "%d", &pev->rendermode);

	else if (FStrEq(szTargetKeyName, "renderamt"))
		sscanf(szTheValue, "%f", &pev->renderamt);

	else if (FStrEq(szTargetKeyName, "rendercolor"))
		sscanf(szTheValue, "%f %f %f", &pev->rendercolor.x, &pev->rendercolor.y, &pev->rendercolor.z);

	else if (FStrEq(szTargetKeyName, "renderfx"))
		sscanf(szTheValue, "%d", &pev->renderfx);

	else if (FStrEq(szTargetKeyName, "health"))
		sscanf(szTheValue, "%f", &pev->health);

	else if (FStrEq(szTargetKeyName, "frags"))
		sscanf(szTheValue, "%f", &pev->frags);

	else if (FStrEq(szTargetKeyName, "weapons"))
		sscanf(szTheValue, "%d", &pev->weapons);

	else if (FStrEq(szTargetKeyName, "takedamage"))
		sscanf(szTheValue, "%f", &pev->takedamage);

	else if (FStrEq(szTargetKeyName, "takedamage"))
		sscanf(szTheValue, "%f", &pev->takedamage);

	else if (FStrEq(szTargetKeyName, "deadflag"))
		sscanf(szTheValue, "%d", &pev->deadflag);

	else if (FStrEq(szTargetKeyName, "view_ofs"))
		sscanf(szTheValue, "%f %f %f", &pev->view_ofs.x, &pev->view_ofs.y, &pev->view_ofs.z);

	else if (FStrEq(szTargetKeyName, "button"))
		sscanf(szTheValue, "%d", &pev->button);

	else if (FStrEq(szTargetKeyName, "impulse"))
		sscanf(szTheValue, "%d", &pev->impulse);

	else if (FStrEq(szTargetKeyName, "max_health"))
		sscanf(szTheValue, "%f", &pev->max_health);

	else if (FStrEq(szTargetKeyName, "teleport_time"))
		sscanf(szTheValue, "%f", &pev->teleport_time);

	else if (FStrEq(szTargetKeyName, "armortype"))
		sscanf(szTheValue, "%f", &pev->armortype);

	else if (FStrEq(szTargetKeyName, "armorvalue"))
		sscanf(szTheValue, "%f", &pev->armorvalue);

	else if (FStrEq(szTargetKeyName, "waterlevel"))
		sscanf(szTheValue, "%d", &pev->waterlevel);

	else if (FStrEq(szTargetKeyName, "watertype"))
		sscanf(szTheValue, "%d", &pev->watertype);

	else if (FStrEq(szTargetKeyName, "target"))
		pev->target = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "targetname"))
		pev->targetname = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "netname"))
		pev->netname = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "message"))
		pev->message = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "dmg_take"))
		sscanf(szTheValue, "%f", &pev->dmg_take);

	else if (FStrEq(szTargetKeyName, "dmg_save"))
		sscanf(szTheValue, "%f", &pev->dmg_save);

	else if (FStrEq(szTargetKeyName, "dmg"))
		sscanf(szTheValue, "%f", &pev->dmg);

	else if (FStrEq(szTargetKeyName, "dmgtime"))
		sscanf(szTheValue, "%f", &pev->dmgtime);

	else if (FStrEq(szTargetKeyName, "noise"))
		pev->noise = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "noise1"))
		pev->noise1 = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "noise2"))
		pev->noise2 = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "noise3"))
		pev->noise3 = ALLOC_STRING(szTheValue);

	else
		SetKeyValueBase(szTargetKeyName, szTheValue);
}


// NOTE: szName must be a pointer to constant memory, e.g. "monster_class" because the entity
// will keep a pointer to it after this call.
CBaseEntity * CBaseEntity::Create( char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner )
{
	edict_t	*pent;
	CBaseEntity *pEntity;

	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szName ));
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, "NULL Ent in Create!\n" );
		return NULL;
	}
	pEntity = Instance( pent );
	pEntity->pev->owner = pentOwner;
	pEntity->pev->origin = vecOrigin;
	pEntity->pev->angles = vecAngles;
	DispatchSpawn( pEntity->edict() );
	return pEntity;
}


