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
#if !defined( OEM_BUILD )

#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "AI/Monsters.h"
#include "Weapons/Weapons.h"
#include "nodes.h"
#include "player.h"
#include "Game/GameRules.h"

enum rpg_e {
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD,		// to reload
	RPG_FIRE2,		// to empty
	RPG_HOLSTER1,	// loaded
	RPG_DRAW1,		// loaded
	RPG_HOLSTER2,	// unloaded
	RPG_DRAW_UL,	// unloaded
	RPG_IDLE_UL,	// unloaded idle
	RPG_FIDGET_UL,	// unloaded fidget
};

enum rpgRocketMode
{
	ROCKET_UNGUIDED,
	ROCKET_LASERGUIDED,
	ROCKET_PLAYERGUIDED
};

LINK_WEAPON_TO_CLASS( weapon_rpg, CRpg );

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS( laser_spot, CLaserSpot );

//=========================================================
//=========================================================
CLaserSpot *CLaserSpot::CreateSpot( void )
{
	CLaserSpot *pSpot = GetClassPtr( (CLaserSpot *)NULL );
	pSpot->Spawn();

	pSpot->pev->classname = MAKE_STRING("laser_spot");

	return pSpot;
}

//=========================================================
//=========================================================
void CLaserSpot::Spawn( void )
{
	Precache( );
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;

	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/laserdot.spr");
	UTIL_SetOrigin( pev, pev->origin );
};

//=========================================================
// Suspend- make the laser sight invisible. 
//=========================================================
void CLaserSpot::Suspend( float flSuspendTime )
{
	pev->effects |= EF_NODRAW;
	
	SetThink( &CLaserSpot::Revive );
	pev->nextthink = gpGlobals->time + flSuspendTime;
}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
void CLaserSpot::Revive( void )
{
	pev->effects &= ~EF_NODRAW;

	SetThink( NULL );
}

void CLaserSpot::Precache( void )
{
	PRECACHE_MODEL("sprites/laserdot.spr");
};

LINK_ENTITY_TO_CLASS( rpg_rocket, CRpgRocket );

//=========================================================
//=========================================================
CRpgRocket *CRpgRocket::CreateRpgRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher )
{
	CRpgRocket *pRocket = GetClassPtr( (CRpgRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	pRocket->pev->angles = vecAngles;
	pRocket->Spawn();
	pRocket->SetTouch( &CRpgRocket::RocketTouch );
	pRocket->m_pLauncher = pLauncher;// remember what RPG fired me. 
	pRocket->m_pLauncher->m_cActiveRockets++;// register this missile as active for the launcher
	pRocket->pev->owner = pOwner->edict();

	return pRocket;
}

//=========================================================
//=========================================================
void CRpgRocket :: Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/rpgrocket.mdl");
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	pev->classname = MAKE_STRING("rpg_rocket");

	SetThink( &CRpgRocket::IgniteThink );
	SetTouch( &CRpgRocket::ExplodeTouch );

	pev->angles.x -= 30;
	UTIL_MakeVectors( pev->angles );
	pev->angles.x = -(pev->angles.x + 30);

	pev->velocity = gpGlobals->v_forward * 500;
	pev->gravity = 0.5;

	pev->nextthink = gpGlobals->time + 0.01;

	pev->dmg = gSkillData.plrDmgRPG;
}

//=========================================================
//=========================================================
void CRpgRocket :: RocketTouch ( CBaseEntity *pOther )
{
	if ( m_pLauncher )
	{
		// my launcher is still around, tell it I'm dead.
		m_pLauncher->m_cActiveRockets--;
	}

	ALERT( at_console, "\nBANG!!!!" );

	SET_VIEW( m_pLauncher->m_pPlayer->edict(), m_pLauncher->m_pPlayer->edict() );
	m_pLauncher->m_pPlayer->pev->fov = m_pLauncher->m_pPlayer->m_iFOV = m_iFOV = 0;

	STOP_SOUND( edict(), CHAN_VOICE, "weapons/rocket1.wav" );
	ExplodeTouch( pOther );
}

//=========================================================
//=========================================================
void CRpgRocket :: Precache( void )
{
	PRECACHE_MODEL("models/rpgrocket.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND ("weapons/rocket1.wav");
}


void CRpgRocket :: IgniteThink( void  )
{
	static float flTimeToFollow = 0;

	if ( m_pLauncher->m_iRocketMode == 2 )
	{
		m_pLauncher->m_pPlayer->pev->fov = m_pLauncher->m_pPlayer->m_iFOV = m_iFOV;
		pev->angles = m_pLauncher->m_pPlayer->pev->v_angle;
	}

	flTimeToFollow += 0.01;

	// set to follow laser spot
	if ( flTimeToFollow > 0.35 )
	{
		SetThink( &CRpgRocket::FollowThink );
		pev->movetype = MOVETYPE_FLY;
		flTimeToFollow = 0;

		// rocket trail
		MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );

			WRITE_BYTE( TE_BEAMFOLLOW );
			WRITE_SHORT( entindex() );	// entity
			WRITE_SHORT( m_iTrail );	// model
			WRITE_BYTE( 120 );			// life
			WRITE_BYTE( 24 );			// width
			WRITE_BYTE( 230 );			// r, g, b
			WRITE_BYTE( 224 );			// r, g, b
			WRITE_BYTE( 255 );			// r, g, b
			WRITE_BYTE( 200 );			// brightness

		MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

		// make rocket sound
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5 );
	
		m_flIgniteTime = gpGlobals->time;
	}

	pev->nextthink = gpGlobals->time + 0.01;
}

void CRpgRocket::EmptyFuelThink( void )
{
	ALERT( at_console, "EmptyFuelThink\n" );

	if ( m_pLauncher->m_iRocketMode == 2 )
	{
		m_pLauncher->m_pPlayer->pev->fov = m_pLauncher->m_pPlayer->m_iFOV = m_iFOV;
		pev->angles = m_pLauncher->m_pPlayer->pev->v_angle;
	}

	pev->gravity = 0;
	pev->nextthink = gpGlobals->time + 0.01;
}

void CRpgRocket :: FollowThink( void  )
{
	ALERT( at_console, "FollowThink\n" );

	CBaseEntity *pOther = NULL;
	Vector vecTarget;
	Vector vecDir;
	float flDist, flMax, flDot;
	TraceResult tr;

	static Vector vecOldAngles = pev->angles;

	static float flFOVCounter = 0;
	static bool fInRocket = false;

//	static float flRollAngle = 0;

	// Playerguided missile view stuff
	if ( m_pLauncher->m_iRocketMode == ROCKET_PLAYERGUIDED && !fInRocket )
	{
		m_pLauncher->m_pPlayer->pev->fov = m_pLauncher->m_pPlayer->m_iFOV = m_iFOV;
		fInRocket = true;
	}
	else if ( m_pLauncher->m_iRocketMode != ROCKET_PLAYERGUIDED )
	{
		fInRocket = false;
	}

	if ( fInRocket )
	{
//		Vector vecOldAngleDir, vecAngleDir;
//		UTIL_MakeVectorsPrivate( vecOldAngles, vecOldAngleDir, NULL, NULL );
//		UTIL_MakeVectorsPrivate( pev->angles, vecAngleDir, NULL, NULL );

		pev->angles = m_pLauncher->m_pPlayer->pev->v_angle;	
		pev->angles.x *= -1;
		UTIL_MakeAimVectors( pev->angles );
	
//		flRollAngle = DotProduct( gpGlobals->v_right, pev->velocity );
//		flRollAngle /= 12.5f;
//		pev->fixangle = 1;
//		m_pLauncher->m_pPlayer->pev->punchangle.z = pev->v_angle.z = pev->angles.z = flRollAngle;

//		ALERT( at_console, "\nroll %f", flRollAngle );

		if ( flFOVCounter > 1 || m_iFOV < 100 )
		{
			m_iFOV += 1;
			m_pLauncher->m_pPlayer->pev->fov = m_pLauncher->m_pPlayer->m_iFOV = m_iFOV;
			flFOVCounter = 0;
		}
		else
		{
			flFOVCounter += 0.065;
		}

		m_pLauncher->m_iRocketMode = 2;
		m_pLauncher->m_fSpotActive = FALSE;
	}
	else
	{
		UTIL_MakeAimVectors( pev->angles );
	}

	// Velocity calculations
	vecTarget = gpGlobals->v_forward;
	flMax = 8192;
	
	// Examine all entities within a reasonable radius
	while ((pOther = UTIL_FindEntityByClassname( pOther, "laser_spot" )) != NULL)
	{
		UTIL_TraceLine ( pev->origin, pOther->pev->origin, dont_ignore_monsters, ENT(pev), &tr );
		// ALERT( at_console, "%f\n", tr.flFraction );
		if (tr.flFraction >= 0.90)
		{
			vecDir = pOther->pev->origin - pev->origin;
			flDist = vecDir.Length( );
			vecDir = vecDir.Normalize( );
			flDot = DotProduct( gpGlobals->v_forward, vecDir );
			if ((flDot > 0) && (flDist * (1 - flDot) < flMax))
			{
				flMax = flDist * (1 - flDot);
				vecTarget = vecDir;
			}
		}
	}

	pev->angles = UTIL_VecToAngles( vecTarget );

	// this acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	float flSpeed = pev->velocity.Length();
	if (gpGlobals->time - m_flIgniteTime < 3.0)
	{
		pev->velocity = pev->velocity * 0.2 + vecTarget * (flSpeed * 0.8 + 400);
		if (pev->waterlevel == 3)
		{
			// go slow underwater
			if (pev->velocity.Length() > 300)
			{
				pev->velocity = pev->velocity.Normalize() * 300;
			}
			UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1, pev->origin, 4 );
		} 
		else 
		{
			if (pev->velocity.Length() > m_flRocketFuel * 50)
			{
				pev->velocity = pev->velocity.Normalize() * m_flRocketFuel * 50;
			}
		}
	}
	else
	{
		m_flRocketFuel -= 0.01;

		if ( m_flRocketFuel <= 0 )
		{
			SetThink( &CRpgRocket::EmptyFuelThink );

			STOP_SOUND( ENT( pev ), CHAN_VOICE, "weapons/rocket1.wav" );
			pev->movetype = MOVETYPE_TOSS;
			FBitClear( pev->effects, EF_LIGHT );
		}

		pev->velocity = vecTarget * flSpeed;

//		pev->velocity = pev->velocity * 0.2 + vecTarget * flSpeed * 0.798;
//		if (pev->waterlevel == 0 && pev->velocity.Length() < 200)
//		{
//			Detonate( );
//		}
	}
	// ALERT( at_console, "%.0f\n", flSpeed );

	if ( fInRocket ) // solve the inverted pitch view issue
	{
		pev->angles.x *= -1;
		gpGlobals->v_forward.z *= -1;
	}

	vecOldAngles = pev->angles;

	pev->nextthink = gpGlobals->time + 0.01;
}

void CRpgRocket::Detonate( void )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector( 0, 0, 8 );
	UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -40 ), ignore_monsters, ENT( pev ), &tr );

	m_pLauncher->m_pTrackingRocket = nullptr;

	ALERT( at_console, "\nBANG!!!!" );

	if ( m_pLauncher->m_iRocketMode == 2 )
	{
		SET_VIEW( m_pLauncher->m_pPlayer->edict(), m_pLauncher->m_pPlayer->edict() );
		m_pLauncher->m_pPlayer->pev->fov = m_pLauncher->m_pPlayer->m_iFOV = m_iFOV = 0;
	}

	Explode( &tr, DMG_BLAST );
}

#endif



void CRpg::Reload( void )
{
	int iResult;

	if ( m_iClip == 1 )
	{
		// don't bother with any of this if don't need to reload.
		return;
	}

	if ( m_pPlayer->ammo_rockets <= 0 )
		return;

	// because the RPG waits to autoreload when no missiles are active while  the LTD is on, the
	// weapons code is constantly calling into this function, but is often denied because 
	// a) missiles are in flight, but the LTD is on
	// or
	// b) player is totally out of ammo and has nothing to switch to, and should be allowed to
	//    shine the designator around
	//
	// Set the next attack time into the future so that WeaponIdle will get called more often
	// than reload, allowing the RPG LTD to be updated
	
	m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

	if ( m_cActiveRockets && m_fSpotActive )
	{
		// no reloading when there are active missiles tracking the designator.
		// ward off future autoreload attempts by setting next attack time into the future for a bit. 
		return;
	}

#ifndef CLIENT_DLL
	if ( m_pSpot && m_fSpotActive )
	{
		m_pSpot->Suspend( 2.1 );
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
	}
#endif

	if ( m_iClip == 0 )
		iResult = DefaultReload( RPG_MAX_CLIP, RPG_RELOAD, 2 );
	
	if ( iResult )
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	
}

void CRpg::Spawn( )
{
	Precache( );
	m_iId = WEAPON_RPG;

	SET_MODEL(ENT(pev), "models/w_rpg.mdl");
	m_fSpotActive = 1;
	m_iRocketMode = 1;
	m_pTrackingRocket = nullptr;
	m_flRocketFuel = 100;

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{
		// more default ammo in multiplay. 
		m_iDefaultAmmo = RPG_DEFAULT_GIVE * 2;
	}
	else
	{
		m_iDefaultAmmo = RPG_DEFAULT_GIVE;
	}

	FallInit();// get ready to fall down.
}

void CRpg::Precache( void )
{
	PRECACHE_MODEL("models/w_rpg.mdl");
	PRECACHE_MODEL("models/v_rpg.mdl");
	PRECACHE_MODEL("models/p_rpg.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	UTIL_PrecacheOther( "laser_spot" );
	UTIL_PrecacheOther( "rpg_rocket" );

	PRECACHE_SOUND("weapons/rocketfire1.wav");
	PRECACHE_SOUND("weapons/glauncher.wav"); // alternative fire sound

	m_usRpg = PRECACHE_EVENT ( 1, "events/rpg.sc" );
}

int CRpg::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "rockets";
	p->iMaxAmmo1 = ROCKET_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = RPG_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_RPG;
	p->iFlags = 0;
	p->iWeight = RPG_WEIGHT;

	return 1;
}

int CRpg::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CRpg::Deploy( )
{
	if ( m_iClip == 0 )
	{
		return DefaultDeploy( "models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW_UL, "rpg" );
	}

	return DefaultDeploy( "models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW1, "rpg" );
}

BOOL CRpg::CanHolster( void )
{
	if ( m_fSpotActive && m_cActiveRockets )
	{
		// can't put away while guiding a missile.
		return FALSE;
	}

	return TRUE;
}

void CRpg::Holster( int skiplocal /* = 0 */ )
{
	m_fInReload = FALSE;// cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	SendWeaponAnim( RPG_HOLSTER1 );

#ifndef CLIENT_DLL
	if (m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NEVER );
		m_pSpot = NULL;
	}
#endif

}



void CRpg::PrimaryAttack()
{
	UpdateSpot();

	m_fPrimaryFire = TRUE;

#ifndef CLIENT_DLL
//	ALERT( at_console, "\nPrimaryAttack" );
	
//	ALERT( at_console, "PRIM atk %d\t fuel %f\t burn %f\t time %f\t fire %d\t idle %f\n",
//		   m_fInAttack,
//		   m_flGivenFuel,
//		   m_pPlayer->m_flNextAmmoBurn,
//		   UTIL_WeaponTimeBase(),
//		   m_fPrimaryFire,
//		   m_flTimeWeaponIdle);
#endif

	if ( m_iClip )
	{
//		if ( m_pTrackingRocket != nullptr )
//			return; // there's a rocket out there being controlled by our player, let's not fire another one
		
		if ( m_fInAttack == 0 )
		{
			Spinup();

			m_pPlayer->m_flNextAmmoBurn = UTIL_WeaponTimeBase();
			
			m_fInAttack = 1;
			
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
			m_pPlayer->m_flStartCharge = gpGlobals->time;
			m_pPlayer->m_flAmmoStartCharge = UTIL_WeaponTimeBase() + 10; // max charge time
//			m_pPlayer->m_flNextAttack
		}

		else if ( m_fInAttack == 1 )
		{
			if ( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
			{
				Spinup();
				m_fInAttack = 2;
			}
		}

		else // if ( m_fInAttack == 2 )
		{
					// during the charging process, eat one bit of ammo every once in a while
			if ( UTIL_WeaponTimeBase() >= m_pPlayer->m_flNextAmmoBurn && m_pPlayer->m_flNextAmmoBurn != 1000 )
			{
				Spinup();
				m_pPlayer->m_flNextAmmoBurn = UTIL_WeaponTimeBase() + 0.01;
			}

			if ( UTIL_WeaponTimeBase() >= m_pPlayer->m_flAmmoStartCharge )
			{
				// don't eat any more ammo after gun is fully charged.
				m_pPlayer->m_flNextAmmoBurn = 1000;
			}
		}
	}

	else
	{
		PlayEmptySound( );
	}
}


void CRpg::SecondaryAttack()
{
	m_iRocketMode++;
	if ( m_iRocketMode > ROCKET_PLAYERGUIDED )
		m_iRocketMode = ROCKET_UNGUIDED;

	if ( m_iRocketMode == ROCKET_LASERGUIDED )
		m_fSpotActive = TRUE;
	else
		m_fSpotActive = FALSE;

#ifndef CLIENT_DLL
	if (!m_fSpotActive && m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NORMAL );
		m_pSpot = NULL;
	}

	ALERT( at_console, "\nm_iRocketMode = %d, m_fSpotActive = %d", m_iRocketMode, m_fSpotActive );

#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
}


void CRpg::WeaponIdle( void )
{
	UpdateSpot( );
	ResetEmptySound( );

#ifndef CLIENT_DLL
//	ALERT( at_console, "\nWeaponIdle" );
	
//	ALERT( at_console, "IDLE atk %d\t fuel %f\t burn %f\t time %f\t fire %d\t idle %f\n",
//		   m_fInAttack,
//		   m_flGivenFuel,
//		   m_pPlayer->m_flNextAmmoBurn,
//		   UTIL_WeaponTimeBase(),
//		   m_fPrimaryFire,
//		   m_flTimeWeaponIdle);
#endif

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_fInAttack != 0 && !m_fPrimaryFire )
	{
		FireRocket();
		m_flGivenFuel = 0;
		m_fInAttack = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
	}
	else
	{
		if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
		{
			int iAnim;
			float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
			if ( flRand <= 0.75 || m_fSpotActive )
			{
				if ( m_iClip == 0 )
					iAnim = RPG_IDLE_UL;
				else
					iAnim = RPG_IDLE;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 15.0;
			}
			else
			{
				if ( m_iClip == 0 )
					iAnim = RPG_FIDGET_UL;
				else
					iAnim = RPG_FIDGET;

				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
			}

			SendWeaponAnim( iAnim );
		}
		else
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
		}

		if ( !m_fPrimaryFire )
		{
			m_fInAttack = 0;
		}
		else
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
		}

		m_fPrimaryFire = FALSE;
	}
}

void CRpg::Spinup( void )
{
//	m_flRocketFuel -= 0.3;
	m_flGivenFuel += 0.3;
}

void CRpg::FireRocket( void )
{
	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL

		// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

	CRpgRocket *pRocket = CRpgRocket::CreateRpgRocket( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this );
	pRocket->m_iFOV = 24;
	pRocket->m_flRocketFuel = m_flGivenFuel;

	if ( m_iRocketMode == ROCKET_PLAYERGUIDED )
	{
		m_pTrackingRocket = pRocket;
		SET_VIEW( m_pPlayer->edict(), ENT( pRocket->pev ) );
	}

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );// RpgRocket::Create stomps on globals, so remake.
	pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );
#endif

		// firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usRpg );

	m_iClip--;

	m_flNextPrimaryAttack = GetNextAttackDelay( 1.5 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
}

void CRpg::UpdateSpot( void )
{

#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpot::CreateSpot();
		}

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( );;
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );
		
		UTIL_SetOrigin( m_pSpot->pev, tr.vecEndPos );
	}
#endif

}


class CRpgAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_rpgammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_rpgammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int iGive;

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
		{
			// hand out more ammo per rocket in multiplayer.
			iGive = AMMO_RPGCLIP_GIVE * 2;
		}
		else
		{
			iGive = AMMO_RPGCLIP_GIVE;
		}

		if (pOther->GiveAmmo( iGive, "rockets", ROCKET_MAX_CARRY ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_rpgclip, CRpgAmmo );

#endif
