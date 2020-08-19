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
/*

===== buttons.cpp ========================================================

  button-related code

*/

#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "Base/SaveRestore.h"
#include "Func/CBaseDoor.h"

#if !defined ( _WIN32 )
#include <string.h> // memset())))
#endif


#define SF_GLOBAL_SET			1	// Set global state to initial state on spawn

class CEnvGlobal : public CPointEntity
{
public:
	void	Spawn( void );
	void	KeyValue( KeyValueData *pkvd );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];
	
	string_t	m_globalstate;
	int			m_triggermode;
	int			m_initialstate;
};

TYPEDESCRIPTION CEnvGlobal::m_SaveData[] =
{
	DEFINE_FIELD( CEnvGlobal, m_globalstate, FIELD_STRING ),
	DEFINE_FIELD( CEnvGlobal, m_triggermode, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvGlobal, m_initialstate, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CEnvGlobal, CBaseEntity );

LINK_ENTITY_TO_CLASS( env_global, CEnvGlobal );

void CEnvGlobal::KeyValue( KeyValueData *pkvd )
{
	pkvd->fHandled = TRUE;

	if ( FStrEq(pkvd->szKeyName, "globalstate") )		// State name
		m_globalstate = ALLOC_STRING( pkvd->szValue );
	else if ( FStrEq(pkvd->szKeyName, "triggermode") )
		m_triggermode = atoi( pkvd->szValue );
	else if ( FStrEq(pkvd->szKeyName, "initialstate") )
		m_initialstate = atoi( pkvd->szValue );
	else 
		CPointEntity::KeyValue( pkvd );
}

void CEnvGlobal::Spawn( void )
{
	if ( !m_globalstate )
	{
		REMOVE_ENTITY( ENT(pev) );
		return;
	}
	if ( FBitSet( pev->spawnflags, SF_GLOBAL_SET ) )
	{
		if ( !gGlobalState.EntityInTable( m_globalstate ) )
			gGlobalState.EntityAdd( m_globalstate, gpGlobals->mapname, (GLOBALESTATE)m_initialstate );
	}
}


void CEnvGlobal::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	GLOBALESTATE oldState = gGlobalState.EntityGetState( m_globalstate );
	GLOBALESTATE newState;

	switch( m_triggermode )
	{
	case 0:
		newState = GLOBAL_OFF;
		break;

	case 1:
		newState = GLOBAL_ON;
		break;

	case 2:
		newState = GLOBAL_DEAD;
		break;

	default:
	case 3:
		if ( oldState == GLOBAL_ON )
			newState = GLOBAL_OFF;
		else if ( oldState == GLOBAL_OFF )
			newState = GLOBAL_ON;
		else
			newState = oldState;
	}

	if ( gGlobalState.EntityInTable( m_globalstate ) )
		gGlobalState.EntitySetState( m_globalstate, newState );
	else
		gGlobalState.EntityAdd( m_globalstate, gpGlobals->mapname, newState );
}



TYPEDESCRIPTION CMultiSource::m_SaveData[] =
{
	//!!!BUGBUG FIX
	DEFINE_ARRAY( CMultiSource, m_rgEntities, FIELD_EHANDLE, MS_MAX_TARGETS ),
	DEFINE_ARRAY( CMultiSource, m_rgTriggered, FIELD_INTEGER, MS_MAX_TARGETS ),
	DEFINE_FIELD( CMultiSource, m_iTotal, FIELD_INTEGER ),
	DEFINE_FIELD( CMultiSource, m_globalstate, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE( CMultiSource, CBaseEntity );

LINK_ENTITY_TO_CLASS( multisource, CMultiSource );
//
// Cache user-entity-field values until spawn is called.
//

void CMultiSource::KeyValue( KeyValueData *pkvd )
{
	if (	FStrEq(pkvd->szKeyName, "style") ||
				FStrEq(pkvd->szKeyName, "height") ||
				FStrEq(pkvd->szKeyName, "killtarget") ||
				FStrEq(pkvd->szKeyName, "value1") ||
				FStrEq(pkvd->szKeyName, "value2") ||
				FStrEq(pkvd->szKeyName, "value3"))
		pkvd->fHandled = TRUE;
	else if ( FStrEq(pkvd->szKeyName, "globalstate") )
	{
		m_globalstate = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else 
		CPointEntity::KeyValue( pkvd );
}

#define SF_MULTI_INIT		1

void CMultiSource::Spawn()
{ 
	// set up think for later registration

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time + 0.1;
	pev->spawnflags |= SF_MULTI_INIT;	// Until it's initialized
	SetThink(&CMultiSource::Register);
}

void CMultiSource::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{ 
	int i = 0;

	// Find the entity in our list
	while (i < m_iTotal)
		if ( m_rgEntities[i++] == pCaller )
			break;

	// if we didn't find it, report error and leave
	if (i > m_iTotal)
	{
		ALERT(at_console, "MultiSrc:Used by non member %s.\n", STRING(pCaller->pev->classname));
		return;	
	}

	// CONSIDER: a Use input to the multisource always toggles.  Could check useType for ON/OFF/TOGGLE

	m_rgTriggered[i-1] ^= 1;

	// 
	if ( IsTriggered( pActivator ) )
	{
		ALERT( at_aiconsole, "Multisource %s enabled (%d inputs)\n", STRING(pev->targetname), m_iTotal );
		USE_TYPE useType = USE_TOGGLE;
		if ( m_globalstate )
			useType = USE_ON;
		SUB_UseTargets( NULL, useType, 0 );
	}
}


BOOL CMultiSource::IsTriggered( CBaseEntity * )
{
	// Is everything triggered?
	int i = 0;

	// Still initializing?
	if ( pev->spawnflags & SF_MULTI_INIT )
		return 0;

	while (i < m_iTotal)
	{
		if (m_rgTriggered[i] == 0)
			break;
		i++;
	}

	if (i == m_iTotal)
	{
		if ( !m_globalstate || gGlobalState.EntityGetState( m_globalstate ) == GLOBAL_ON )
			return 1;
	}
	
	return 0;
}

void CMultiSource::Register(void)
{ 
	edict_t *pentTarget	= NULL;

	m_iTotal = 0;
	memset( m_rgEntities, 0, MS_MAX_TARGETS * sizeof(EHANDLE) );

	SetThink(&CMultiSource::SUB_DoNothing);

	// search for all entities which target this multisource (pev->targetname)

	pentTarget = FIND_ENTITY_BY_STRING(NULL, "target", STRING(pev->targetname));

	while (!FNullEnt(pentTarget) && (m_iTotal < MS_MAX_TARGETS))
	{
		CBaseEntity *pTarget = CBaseEntity::Instance(pentTarget);
		if ( pTarget )
			m_rgEntities[m_iTotal++] = pTarget;

		pentTarget = FIND_ENTITY_BY_STRING( pentTarget, "target", STRING(pev->targetname));
	}

	pentTarget = FIND_ENTITY_BY_STRING(NULL, "classname", "multi_manager");
	while (!FNullEnt(pentTarget) && (m_iTotal < MS_MAX_TARGETS))
	{
		CBaseEntity *pTarget = CBaseEntity::Instance(pentTarget);
		if ( pTarget && pTarget->HasTarget(pev->targetname) )
			m_rgEntities[m_iTotal++] = pTarget;

		pentTarget = FIND_ENTITY_BY_STRING( pentTarget, "classname", "multi_manager" );
	}

	pev->spawnflags &= ~SF_MULTI_INIT;
}


// Make this button behave like a door (HACKHACK)
// This will disable use and make the button solid
// rotating buttons were made SOLID_NOT by default since their were some
// collision problems with them...
#define SF_MOMENTARY_DOOR		0x0001

class CMomentaryRotButton : public CBaseToggle
{
public:
	void	Spawn ( void );
	void	KeyValue( KeyValueData *pkvd );
	virtual int	ObjectCaps( void ) 
	{ 
		int flags = (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_INDIRECT_USE; 
		if ( pev->spawnflags & SF_MOMENTARY_DOOR )
			return flags;
		return flags | FCAP_CONTINUOUS_USE;
	}
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT Off( void );
	void	EXPORT Return( void );
	void	UpdateSelf( float value );
	void	UpdateSelfReturn( float value );
	void	UpdateAllButtons( float value, int start );

	void	PlaySound( void );
	void	UpdateTarget( float value );

	static CMomentaryRotButton *Instance( edict_t *pent ) { return (CMomentaryRotButton *)GET_PRIVATE(pent);};
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	int		m_lastUsed;
	int		m_direction;
	float	m_returnSpeed;
	vec3_t	m_start;
	vec3_t	m_end;
	int		m_sounds;
};
TYPEDESCRIPTION CMomentaryRotButton::m_SaveData[] =
{
	DEFINE_FIELD( CMomentaryRotButton, m_lastUsed, FIELD_INTEGER ),
	DEFINE_FIELD( CMomentaryRotButton, m_direction, FIELD_INTEGER ),
	DEFINE_FIELD( CMomentaryRotButton, m_returnSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( CMomentaryRotButton, m_start, FIELD_VECTOR ),
	DEFINE_FIELD( CMomentaryRotButton, m_end, FIELD_VECTOR ),
	DEFINE_FIELD( CMomentaryRotButton, m_sounds, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CMomentaryRotButton, CBaseToggle );

LINK_ENTITY_TO_CLASS( momentary_rot_button, CMomentaryRotButton );

void CMomentaryRotButton::Spawn( void )
{
	CBaseToggle::AxisDir( pev );

	if ( pev->speed == 0 )
		pev->speed = 100;

	if ( m_flMoveDistance < 0 ) 
	{
		m_start = pev->angles + pev->movedir * m_flMoveDistance;
		m_end = pev->angles;
		m_direction = 1;		// This will toggle to -1 on the first use()
		m_flMoveDistance = -m_flMoveDistance;
	}
	else
	{
		m_start = pev->angles;
		m_end = pev->angles + pev->movedir * m_flMoveDistance;
		m_direction = -1;		// This will toggle to +1 on the first use()
	}

	if ( pev->spawnflags & SF_MOMENTARY_DOOR )
		pev->solid		= SOLID_BSP;
	else
		pev->solid		= SOLID_NOT;

	pev->movetype	= MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model) );

	char *pszSound = ButtonSound( m_sounds );
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);
	m_lastUsed = 0;
}

void CMomentaryRotButton::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "returnspeed"))
	{
		m_returnSpeed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue( pkvd );
}

void CMomentaryRotButton::PlaySound( void )
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);
}

// BUGBUG: This design causes a latentcy.  When the button is retriggered, the first impulse
// will send the target in the wrong direction because the parameter is calculated based on the
// current, not future position.
void CMomentaryRotButton::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	pev->ideal_yaw = CBaseToggle::AxisDelta( pev->spawnflags, pev->angles, m_start ) / m_flMoveDistance;

	UpdateAllButtons( pev->ideal_yaw, 1 );

	// Calculate destination angle and use it to predict value, this prevents sending target in wrong direction on retriggering
	Vector dest = pev->angles + pev->avelocity * (pev->nextthink - pev->ltime);
	float value1 = CBaseToggle::AxisDelta( pev->spawnflags, dest, m_start ) / m_flMoveDistance;
	UpdateTarget( value1 );

}

void CMomentaryRotButton::UpdateAllButtons( float value, int start )
{
	// Update all rot buttons attached to the same target
	edict_t *pentTarget = NULL;
	for (;;)
	{

		pentTarget = FIND_ENTITY_BY_STRING(pentTarget, "target", STRING(pev->target));
		if (FNullEnt(pentTarget))
			break;

		if ( FClassnameIs( VARS(pentTarget), "momentary_rot_button" ) )
		{
			CMomentaryRotButton *pEntity = CMomentaryRotButton::Instance(pentTarget);
			if ( pEntity )
			{
				if ( start )
					pEntity->UpdateSelf( value );
				else
					pEntity->UpdateSelfReturn( value );
			}
		}
	}
}

void CMomentaryRotButton::UpdateSelf( float value )
{
	BOOL fplaysound = FALSE;

	if ( !m_lastUsed )
	{
		fplaysound = TRUE;
		m_direction = -m_direction;
	}
	m_lastUsed = 1;

	pev->nextthink = pev->ltime + 0.1;
	if ( m_direction > 0 && value >= 1.0 )
	{
		pev->avelocity = g_vecZero;
		pev->angles = m_end;
		return;
	}
	else if ( m_direction < 0 && value <= 0 )
	{
		pev->avelocity = g_vecZero;
		pev->angles = m_start;
		return;
	}
	
	if (fplaysound)
		PlaySound();

	// HACKHACK -- If we're going slow, we'll get multiple player packets per frame, bump nexthink on each one to avoid stalling
	if ( pev->nextthink < pev->ltime )
		pev->nextthink = pev->ltime + 0.1;
	else
		pev->nextthink += 0.1;
	
	pev->avelocity = (m_direction * pev->speed) * pev->movedir;
	SetThink( &CMomentaryRotButton::Off );
}

void CMomentaryRotButton::UpdateTarget( float value )
{
	if (!FStringNull(pev->target))
	{
		edict_t* pentTarget	= NULL;
		for (;;)
		{
			pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
			if (FNullEnt(pentTarget))
				break;
			CBaseEntity *pEntity = CBaseEntity::Instance(pentTarget);
			if ( pEntity )
			{
				pEntity->Use( this, this, USE_SET, value );
			}
		}
	}
}

void CMomentaryRotButton::Off( void )
{
	pev->avelocity = g_vecZero;
	m_lastUsed = 0;
	if ( FBitSet( pev->spawnflags, SF_PENDULUM_AUTO_RETURN ) && m_returnSpeed > 0 )
	{
		SetThink( &CMomentaryRotButton::Return );
		pev->nextthink = pev->ltime + 0.1;
		m_direction = -1;
	}
	else
		SetThink( NULL );
}

void CMomentaryRotButton::Return( void )
{
	float value = CBaseToggle::AxisDelta( pev->spawnflags, pev->angles, m_start ) / m_flMoveDistance;

	UpdateAllButtons( value, 0 );	// This will end up calling UpdateSelfReturn() n times, but it still works right
	if ( value > 0 )
		UpdateTarget( value );
}


void CMomentaryRotButton::UpdateSelfReturn( float value )
{
	if ( value <= 0 )
	{
		pev->avelocity = g_vecZero;
		pev->angles = m_start;
		pev->nextthink = -1;
		SetThink( NULL );
	}
	else
	{
		pev->avelocity = -m_returnSpeed * pev->movedir;
		pev->nextthink = pev->ltime + 0.1;
	}
}


//----------------------------------------------------------------
// Spark
//----------------------------------------------------------------

class CEnvSpark : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	EXPORT SparkThink(void);
	void	EXPORT SparkStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT SparkStop(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	KeyValue(KeyValueData *pkvd);
	
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	float	m_flDelay;
};


TYPEDESCRIPTION CEnvSpark::m_SaveData[] =
{
	DEFINE_FIELD( CEnvSpark, m_flDelay, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE( CEnvSpark, CBaseEntity );

LINK_ENTITY_TO_CLASS(env_spark, CEnvSpark);
LINK_ENTITY_TO_CLASS(env_debris, CEnvSpark);

void CEnvSpark::Spawn(void)
{
	SetThink( NULL );
	SetUse( NULL );

	if (FBitSet(pev->spawnflags, 32)) // Use for on/off
	{
		if (FBitSet(pev->spawnflags, 64)) // Start on
		{
			SetThink(&CEnvSpark::SparkThink);	// start sparking
			SetUse(&CEnvSpark::SparkStop);		// set up +USE to stop sparking
		}
		else
			SetUse(&CEnvSpark::SparkStart);
	}
	else
		SetThink(&CEnvSpark::SparkThink);
		
	pev->nextthink = gpGlobals->time + ( 0.1 + RANDOM_FLOAT ( 0, 1.5 ) );

	if (m_flDelay <= 0)
		m_flDelay = 1.5;

	Precache( );
}


void CEnvSpark::Precache(void)
{
	PRECACHE_SOUND( "buttons/spark1.wav" );
	PRECACHE_SOUND( "buttons/spark2.wav" );
	PRECACHE_SOUND( "buttons/spark3.wav" );
	PRECACHE_SOUND( "buttons/spark4.wav" );
	PRECACHE_SOUND( "buttons/spark5.wav" );
	PRECACHE_SOUND( "buttons/spark6.wav" );
}

void CEnvSpark::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "MaxDelay"))
	{
		m_flDelay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;	
	}
	else if (	FStrEq(pkvd->szKeyName, "style") ||
				FStrEq(pkvd->szKeyName, "height") ||
				FStrEq(pkvd->szKeyName, "killtarget") ||
				FStrEq(pkvd->szKeyName, "value1") ||
				FStrEq(pkvd->szKeyName, "value2") ||
				FStrEq(pkvd->szKeyName, "value3"))
		pkvd->fHandled = TRUE;
	else
		CBaseEntity::KeyValue( pkvd );
}

void DoSpark( entvars_t *pev, const Vector &location );

void EXPORT CEnvSpark::SparkThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1 + RANDOM_FLOAT (0, m_flDelay);
	DoSpark( pev, pev->origin );
}

void EXPORT CEnvSpark::SparkStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetUse(&CEnvSpark::SparkStop);
	SetThink(&CEnvSpark::SparkThink);
	pev->nextthink = gpGlobals->time + (0.1 + RANDOM_FLOAT ( 0, m_flDelay));
}

void EXPORT CEnvSpark::SparkStop(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetUse(&CEnvSpark::SparkStart);
	SetThink(NULL);
}

#define SF_BTARGET_USE		0x0001
#define SF_BTARGET_ON		0x0002

class CButtonTarget : public CBaseEntity
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	int	ObjectCaps( void );
	
};

LINK_ENTITY_TO_CLASS( button_target, CButtonTarget );

void CButtonTarget::Spawn( void )
{
	pev->movetype	= MOVETYPE_PUSH;
	pev->solid		= SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->takedamage = DAMAGE_YES;

	if ( FBitSet( pev->spawnflags, SF_BTARGET_ON ) )
		pev->frame = 1;
}

void CButtonTarget::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !ShouldToggle( useType, (int)pev->frame ) )
		return;
	pev->frame = 1-pev->frame;
	if ( pev->frame )
		SUB_UseTargets( pActivator, USE_ON, 0 );
	else
		SUB_UseTargets( pActivator, USE_OFF, 0 );
}


int	CButtonTarget :: ObjectCaps( void )
{
	int caps = CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;

	if ( FBitSet(pev->spawnflags, SF_BTARGET_USE) )
		return caps | FCAP_IMPULSE_USE;
	else
		return caps;
}


int CButtonTarget::TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	Use( Instance(pevAttacker), this, USE_TOGGLE, 0 );

	return 1;
}
