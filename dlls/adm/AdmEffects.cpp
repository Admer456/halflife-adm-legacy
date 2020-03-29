/*

======= adm_effects.cpp ==============================

	Effects.

	// These are fully server side right now
	env_viewsway
	- view swaying entity
	env_rendertransition
	- smooth transition from one render amount to the other

	// These use the Efx API and they're parametric
	efx_particle
	- parametric particles
	efx_smoke
	- parametric smoke
	efx_funnel
	- parametric funnel of sprites
	efx_explosion
	- parametric explosion, can hurt if needed

*/

#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "AI/Monsters.h"
//#include "customentity.h"
#include "effects.h"
#include "Weapons/Weapons.h"
#include "decals.h"
#include "func_break.h"
#include "shake.h"
//#include "doors.h"
#include "AdmEffects.h"

// ========================================================================================================= //
//		ENV_ ENTITIIES
// ========================================================================================================= //

// AdmSrc - View swaying entity
// This can be used for anything you wish. A normal view sway like the spectator cam at the end of a round,
// or a drunk vision of some sort. Or just pretending that the player is high as hell. :P
class CViewSway : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Calculate(void);
	void	EXPORT SwayThink(void);
	void	EXPORT SwayStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void	EXPORT SwayUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void	EXPORT SwayStop(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void	KeyValue(KeyValueData *pkvd);
	

	void	SwayViewAngles(Vector &vecPlayerView, CBaseEntity *pPlayer);
	void	SwayPunchAngles(Vector &vecPlayerView, CBaseEntity *pPlayer);

	void	Sway_CalcSines(Vector &vecPlayerView);
	void	Sway_CalcComplexSines(Vector &vecPlayerView);

	/*	inline	float	GetXIntensity( void )	{ return vecSwayIntensity.x; }
		inline	float	GetYIntensity( void )	{ return vecSwayIntensity.y; }
		inline	float	GetZIntensity( void )	{ return vecSwayIntensity.z; }
		inline	float	GetFrequency( void )	{ return fFrequency; } */

	inline	void	SetXIntensity(float xIntensity) { vecSwayIntensity.x = xIntensity; }
	inline	void	SetYIntensity(float yIntensity) { vecSwayIntensity.y = yIntensity; }
	inline	void	SetZIntensity(float zIntensity) { vecSwayIntensity.z = zIntensity; }
	inline	void	SetFrequency(float frequency) { fFrequency = frequency; }
	inline	void	SetDelay(float Delay) { m_iszDelay = Delay; }

private:
	Vector	vecSwayIntensity;
	float	fViewAngle;
	float	fFrequency;
	float	m_iszDelay;
};

LINK_ENTITY_TO_CLASS(env_viewsway, CViewSway);

#define SF_SIMPLE_SINES		1
#define SF_COMPLEX_SINES	2
#define SF_VIEW_ANGLES		4
#define SF_PUNCH_ANGLES		8
#define SF_CONTINUOUS		16
#define SF_NO_THINKING		32
#define SF_START_ON			64

void CViewSway::Spawn(void)
{
	SetThink(NULL);
	SetUse(NULL);

	fViewAngle = 0.0f;

	if (pev->spawnflags & SF_NO_THINKING)
	{
		SetUse(&CViewSway::SwayUse);
		ALERT(at_console, "\nviewsway: NO_THINKING");
	}

	else if ((pev->spawnflags & SF_START_ON) & (pev->spawnflags & SF_CONTINUOUS))
	{
		SetThink(&CViewSway::SwayThink);
		SetUse(&CViewSway::SwayStop);
		ALERT(at_console, "\nviewsway: START_ON and CONT");
	}

	else if (pev->spawnflags & SF_CONTINUOUS)
	{
		SetUse(&CViewSway::SwayStart);
		ALERT(at_console, "\nviewsway: CONT");
	}

	//	SetUse(&CViewSway::SwayStart); // for debugging
}

void CViewSway::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "xIntensity"))
	{
		SetXIntensity(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "yIntensity"))
	{
		SetYIntensity(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "zIntensity"))
	{
		SetZIntensity(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "fFrequency"))
	{
		SetFrequency(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "fDelay"))
	{
		SetDelay(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}

	else
		CBaseEntity::KeyValue(pkvd);
}

void CViewSway::Calculate(void)
{
	int	iSwayPlayerIndex;

	for (iSwayPlayerIndex = 0; iSwayPlayerIndex <= gpGlobals->maxClients; iSwayPlayerIndex++)
	{
		CBaseEntity	*pPlayer = UTIL_PlayerByIndex(iSwayPlayerIndex);

		if (!pPlayer)
			continue;

		Vector vecPlayerView = pPlayer->pev->v_angle;

		if (pev->spawnflags & SF_SIMPLE_SINES)
			Sway_CalcSines(vecPlayerView);

		if (pev->spawnflags & SF_COMPLEX_SINES)
			Sway_CalcComplexSines(vecPlayerView);

		if (pev->spawnflags & SF_VIEW_ANGLES)
			SwayViewAngles(vecPlayerView, pPlayer);

		if (pev->spawnflags & SF_PUNCH_ANGLES)
			SwayPunchAngles(vecPlayerView, pPlayer);
	}

	fViewAngle += (0.1 * fFrequency);

	if (fViewAngle > 360)
		fViewAngle = 0;
}

void EXPORT CViewSway::SwayStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	SetThink(&CViewSway::SwayThink);
	SetUse(&CViewSway::SwayStop);
	pev->nextthink = gpGlobals->time + 0.2;
	ALERT(at_console, "\n\n----\nSwayStart\nSetThink SwayThink\nSetUse SwayStop");
}

void EXPORT CViewSway::SwayUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	Calculate();
	ALERT(at_console, "\n\n----\nSwayUse\nCalculate");
}

void EXPORT CViewSway::SwayStop(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	SetThink(NULL);
	SetUse(&CViewSway::SwayStart);
	ALERT(at_console, "\n\n----\nSwayStop\nSetThink NULL\nSetUse SwayStart");
}

void EXPORT CViewSway::SwayThink(void)
{
	Calculate();
	pev->nextthink = gpGlobals->time + m_iszDelay;
}

void CViewSway::SwayViewAngles(Vector &vecPlayerView, CBaseEntity *pPlayer)
{
	pPlayer->pev->fixangle = 1;

	pPlayer->pev->v_angle.x += vecPlayerView.x;
	pPlayer->pev->v_angle.y += vecPlayerView.y;
	pPlayer->pev->v_angle.z = vecPlayerView.z;

	pPlayer->pev->angles.x += vecPlayerView.x;
	pPlayer->pev->angles.y += vecPlayerView.y;
	pPlayer->pev->angles.z = vecPlayerView.z;
}

void CViewSway::SwayPunchAngles(Vector &vecPlayerView, CBaseEntity *pPlayer)
{
	pPlayer->pev->fixangle = 0;

	pPlayer->pev->punchangle.x += vecPlayerView.x;
	pPlayer->pev->punchangle.y += vecPlayerView.y;
	pPlayer->pev->punchangle.z += vecPlayerView.z;
}

void CViewSway::Sway_CalcSines(Vector &vecPlayerView)
{
	vecPlayerView.x = 0.5 * vecSwayIntensity.x * sin(fViewAngle);
	vecPlayerView.y = 1.4 * vecSwayIntensity.y * sin(fViewAngle);
	vecPlayerView.z = 5.0 * vecSwayIntensity.z * sin(fViewAngle);
}

void CViewSway::Sway_CalcComplexSines(Vector &vecPlayerView)
{
	vecPlayerView.x = 0.45 * vecSwayIntensity.x * sin(1.7 * fViewAngle)  * sin(4.3 * fViewAngle);
	vecPlayerView.y = 0.55 * vecSwayIntensity.y * sin(1.33 * fViewAngle) * sin(0.25 * fViewAngle) + 0.08 * vecSwayIntensity.y * sin(4.3 * fViewAngle);
	vecPlayerView.z = 1.26 * vecSwayIntensity.z * sin(1.4 * fViewAngle)  * sin(1.0 * fViewAngle) + 3.0  * vecSwayIntensity.z * sin(4.2 * fViewAngle);
}

// COspreyDustManager class methods
COspreyDustManager* COspreyDustManager::CreateManager()
{
	COspreyDustManager *pManager = GetClassPtr( (COspreyDustManager *)nullptr );
	return pManager;
}

void COspreyDustManager::Spawn()
{
	PRECACHE_MODEL( "sprites/vp_oriented.spr" );

	for ( int i = 0; i < 32; i++ )
	{
		m_prgParticles[ i ] = CSprite::SpriteCreate( "sprites/vp_oriented.spr", m_pParent->pev->origin, true );

		m_prgParticles[ i ]->pev->origin.z += 16;
		m_prgParticles[ i ]->pev->velocity.x += RANDOM_FLOAT( -128, 128 );
		m_prgParticles[ i ]->pev->velocity.y += RANDOM_FLOAT( -128, 128 );
		m_prgParticles[ i ]->pev->framerate = 10;
		m_prgParticles[ i ]->pev->renderamt = 0;
		m_prgParticles[ i ]->pev->rendermode = kRenderTransAdd;
		m_prgParticles[ i ]->pev->rendercolor = Vector( 210, 210, 255 );
		m_prgParticles[ i ]->pev->scale = RANDOM_FLOAT( 0.5, 2.0 );
	}

	pev->nextthink = gpGlobals->time + 0.02;
}

void COspreyDustManager::Think()
{
	DustEffect();
	pev->nextthink = gpGlobals->time + 0.02;
}

void COspreyDustManager::DustEffect()
{
	UTIL_TraceLine( m_pParent->pev->origin, 
					m_pParent->pev->origin - Vector( 0, 0, 4096 ),
					ignore_monsters, ENT( m_pParent->pev ), &traceOsprey );

	float height = m_pParent->pev->origin.z - traceOsprey.vecEndPos.z;

	// Osprey is close enough to ground, dust is rising
	if ( height < 512 )
	{
		for ( int i = 0; i < 32; i++ )
		{
			// XY - sprite, Z - osprey
			Vector vecSprite = m_prgParticles[ i ]->pev->origin;
			vecSprite.z = m_pParent->pev->origin.z;

			UTIL_TraceLine( vecSprite, 
							vecSprite - Vector(0,0,4096),
							ignore_monsters, ENT(m_pParent->pev), &traceSprite);

			Vector vecDelta = m_prgParticles[ i ]->pev->origin - m_pParent->pev->origin;
			Vector vecCurrentVel = m_prgParticles[ i ]->pev->velocity;

			// The movement is being done on a 2D plane, so we will zero all the Z dimensions
			vecDelta.z = 0;
			vecCurrentVel.z = 0;
			m_prgParticles[ i ]->pev->origin.z = traceSprite.vecEndPos.z + 8;

			float distance = vecDelta.Length();

			// 90° pitch, variable yaw depending on the direction
			// only works on VP_ORIENTED sprites
			m_prgParticles[ i ]->pev->angles.x = 90;
			m_prgParticles[ i ]->pev->angles.y = UTIL_VecToYaw( vecCurrentVel );

			// As the sprite gets farther away, it gets bigger too
			m_prgParticles[ i ]->pev->scale = distance * distance / 60000;
			
			// between 50 and 128 units away from the osprey,
			// the renderamt will go from 0 to 255 respectively
			// same goes for between 128 and 256
			float newRenderAmt;
			if ( distance > 50 && distance < 128 )
				newRenderAmt = min( max( 0, (3.27 * distance - 163) ), 255 );
			else if ( distance > 128 && distance < 256 )
				newRenderAmt = min( max( 0, (-2 * distance + 510) ), 255 );
			else
				newRenderAmt = 0;

			m_prgParticles[ i ]->pev->renderamt *= 0.92;
			m_prgParticles[ i ]->pev->renderamt += newRenderAmt*0.08;

			// If we're too close from the osprey, push away, else slow down
			if ( distance < 50 )
				m_prgParticles[ i ]->pev->velocity = vecCurrentVel * 0.85 + (vecDelta*8)*0.15;
			else
				m_prgParticles[ i ]->pev->velocity = vecCurrentVel / 1.01;

			// If sprites are close enough to the osprey, they will push each other away
			if ( distance < 256 )
			{
				for ( int j = 0; j < 32; j++ )
				{
					if ( j == i )
						continue;

					Vector vecPDelta = m_prgParticles[ i ]->pev->origin - m_prgParticles[ j ]->pev->origin;
					float particleDistance = vecPDelta.Length() + 1.0;

					// Sprites have to be a minimum of 180 units away from each other
					// Then they push each other away randomly, achieving chaotic movement
					if ( particleDistance < 180 )
					{
						m_prgParticles[ i ]->pev->velocity = vecCurrentVel + vecPDelta / 16.0;

						float flRand = particleDistance / 48.0;

						m_prgParticles[ i ]->pev->velocity.x += RANDOM_FLOAT( -flRand, flRand );
						m_prgParticles[ i ]->pev->velocity.y += RANDOM_FLOAT( -flRand, flRand );
					}
				}
			}

			// If sprites are far away enough and are invisible, 
			// teleport them back to the starting position
			else if ( distance > 256 && m_prgParticles[ i ]->pev->renderamt < 2 )
			{
				m_prgParticles[ i ]->pev->origin = m_pParent->pev->origin;
//				m_prgParticles[ i ]->pev->origin.z = tr.vecEndPos.z + 8;
				m_prgParticles[ i ]->pev->velocity = m_prgParticles[ i ]->pev->velocity * 1.3;
			}
		}
	}

	// Disappear slowly
	else
	{
		for ( int i = 0; i < 32; i++ )
		{
			float sprX = m_prgParticles[ i ]->pev->origin.x;
			float sprY = m_prgParticles[ i ]->pev->origin.y;
			float &parX = m_pParent->pev->origin.x;
			float &parY = m_pParent->pev->origin.y;

			m_prgParticles[ i ]->pev->origin.x = sprX * 0.98 + parX * 0.02;
			m_prgParticles[ i ]->pev->origin.y = sprY * 0.98 + parY * 0.02;

			m_prgParticles[ i ]->pev->renderamt /= 1.05;
		}
	}
}

// Temporary class for a certain test
class CTraceChecker : public CBaseEntity
{
public:
	void		Spawn( void );
	void		Think( void );
};

LINK_ENTITY_TO_CLASS( env_tracechecker, CTraceChecker );

void CTraceChecker::Spawn()
{
	pev->nextthink = gpGlobals->time + 1.5;
}

void CTraceChecker::Think( void )
{
	static Vector anglesAim = Vector( 0, 0, 0 );

	Vector startPosition = pev->origin;
	Vector endPosition;

	TraceResult tr;

	anglesAim.y += 0.09;
	if ( anglesAim.y > 360.f )
		anglesAim.y -= 360.f;

	endPosition = startPosition + UTIL_VecPolarCoords( 256.f, anglesAim );

	UTIL_TraceLine( startPosition, endPosition, ignore_monsters, ENT( pev ), &tr );

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
	WRITE_BYTE( TE_LINE );
	WRITE_COORD( startPosition.x );
	WRITE_COORD( startPosition.y );
	WRITE_COORD( startPosition.z );
	WRITE_COORD( tr.vecEndPos.x );
	WRITE_COORD( tr.vecEndPos.y );
	WRITE_COORD( tr.vecEndPos.z );
	WRITE_SHORT( 1 );
	WRITE_BYTE( tr.flFraction * 255 );
	WRITE_BYTE( 240 );
	WRITE_BYTE( 0 );
	MESSAGE_END();

	pev->nextthink = gpGlobals->time + 0.01;
}

// Temporary class for Valentine's Day
// It uses a heart model and attracts some sprites to it. :3
class CHeart : public CBaseAnimating
{
public:
	void		Spawn( void );
	void		Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void		Think( void );

	enum		particleMode
	{
		HeartPrtInvisible = 0,
		HeartPrtFadeIn,
		HeartPrtRadiate,

		HeartPrt_MAX
	};

private:
	CSprite		*m_prgParticles[ 32 ];
	int			m_particleMode;
	string_t	m_iszSprite;
	int			m_iParticleRenderAmt;
};

LINK_ENTITY_TO_CLASS( env_heart, CHeart );

void CHeart::Spawn()
{
	m_particleMode = HeartPrtInvisible;
	m_iParticleRenderAmt = 0;

	PRECACHE_MODEL( "sprites/animglow01.spr" );
	PRECACHE_MODEL( "models/heart.mdl" );

	SET_MODEL( ENT( pev ), "models/heart.mdl" );

	pev->renderamt = 255;
	pev->rendermode = kRenderTransTexture;

	pev->movetype = MOVETYPE_FLY;

	ResetSequenceInfo();

	for ( int i = 0; i < 32; i++ )
	{
		m_prgParticles[ i ] = CSprite::SpriteCreate( "sprites/animglow01.spr", pev->origin, true );

		m_prgParticles[ i ]->pev->rendermode = kRenderTransAdd;
		m_prgParticles[ i ]->pev->rendercolor = Vector( 200, 0, 0 );
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

void CHeart::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_particleMode++;

	if ( m_particleMode == HeartPrt_MAX )
		m_particleMode = HeartPrtInvisible;
}

void CHeart::Think()
{
	switch ( m_particleMode )
	{
	case HeartPrtInvisible: 
		
		break;

	case HeartPrtFadeIn: 
		
//		ALERT( at_console, "\nHeartParticleFadeIn" );

		m_iParticleRenderAmt += 4;

		if ( m_iParticleRenderAmt > 255 )
		{
			m_particleMode = HeartPrtRadiate;
			break;
		}

		for ( int i = 0; i < 32; i++ )
		{
			m_prgParticles[ i ]->pev->renderamt = m_iParticleRenderAmt;
			m_prgParticles[ i ]->pev->velocity = m_prgParticles[ i ]->pev->velocity + Vector( RANDOM_FLOAT( -4, 4 ), RANDOM_FLOAT( -4, 4 ), RANDOM_FLOAT( -4, 4 ) );
		}

		break;

	case HeartPrtRadiate:

//		ALERT( at_console, "\nHeartParticleRadiate" );

		StudioFrameAdvance();

		if ( m_fSequenceFinished )
		{
			// ResetSequenceInfo();
			// hack to avoid reloading model every frame
			pev->animtime = gpGlobals->time;
			pev->framerate = 0.25;
			m_fSequenceFinished = FALSE;
			m_flLastEventCheck = gpGlobals->time;
			pev->frame = 0;
		}

		for ( int i = 0; i < 32; i++ )
		{
			Vector vecDelta = m_prgParticles[ i ]->pev->origin - pev->origin;
			Vector vecCurrentVel = m_prgParticles[ i ]->pev->velocity;

			if ( vecDelta.Length() > 300 )
				m_prgParticles[ i ]->pev->velocity = vecCurrentVel * 0.8 + (-vecDelta)*0.2;
			else if ( vecDelta.Length() < 100 )
				m_prgParticles[ i ]->pev->velocity = vecCurrentVel * 0.8 + (vecDelta)*0.2;
			else
				m_prgParticles[ i ]->pev->velocity = vecCurrentVel / 1.01;

			m_prgParticles[ i ]->pev->scale = 300.0 / (1.0 + vecDelta.Length());

			for ( int j = 0; j < 32; j++ )
			{
				if ( j == i )
					continue;

				Vector vecPDelta = m_prgParticles[ i ]->pev->origin - m_prgParticles[ j ]->pev->origin;

				if ( vecPDelta.Length() < 180 )
				{
					m_prgParticles[ i ]->pev->velocity = vecCurrentVel + vecPDelta / 16.0;

					float flRand = RANDOM_FLOAT( 0, vecPDelta.Length() / 72.0 );

					m_prgParticles[ i ]->pev->velocity = m_prgParticles[ i ]->pev->velocity + Vector( flRand, flRand, flRand );
				}
			}
		}

		break;
	}

	pev->nextthink = gpGlobals->time + 0.01;
}

// Temporary class for Valentine's Day
// It uses a heart model and attracts some sprites to it. :3
class CSpriteAura : public CBaseEntity
{
public:
	void		Spawn( void );
	void		Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void		Think( void );

	enum		particleMode
	{
		HeartPrtInvisible = 0,
		HeartPrtFadeIn,
		HeartPrtRadiate,

		HeartPrt_MAX
	};

private:
	CSprite		*m_prgParticles[ 32 ];
	int			m_particleMode;
	string_t	m_iszSprite;
	int			m_iParticleRenderAmt;
};

LINK_ENTITY_TO_CLASS( env_aura, CSpriteAura );

void CSpriteAura::Spawn()
{
	m_particleMode = HeartPrtInvisible;
	m_iParticleRenderAmt = 0;

	PRECACHE_MODEL( "sprites/animglow01.spr" );
	
	pev->movetype = MOVETYPE_FLY;

	for ( int i = 0; i < 32; i++ )
	{
		m_prgParticles[ i ] = CSprite::SpriteCreate( "sprites/animglow01.spr", pev->origin, true );

		m_prgParticles[ i ]->pev->rendermode = kRenderTransAdd;
		m_prgParticles[ i ]->pev->rendercolor = Vector( 210, 210, 255 );
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

void CSpriteAura::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_particleMode++;

	if ( m_particleMode == HeartPrt_MAX )
		m_particleMode = HeartPrtInvisible;
}

void CSpriteAura::Think()
{
	switch ( m_particleMode )
	{
	case HeartPrtInvisible:

		break;

	case HeartPrtFadeIn:

		m_iParticleRenderAmt += 4;

		if ( m_iParticleRenderAmt > 255 )
		{
			m_particleMode = HeartPrtRadiate;
			break;
		}

		for ( int i = 0; i < 32; i++ )
		{
			m_prgParticles[ i ]->pev->renderamt = m_iParticleRenderAmt;
			m_prgParticles[ i ]->pev->velocity = m_prgParticles[ i ]->pev->velocity + Vector( RANDOM_FLOAT( -4, 4 ), RANDOM_FLOAT( -4, 4 ), RANDOM_FLOAT( -4, 4 ) );
		}

		break;

	case HeartPrtRadiate:

		for ( int i = 0; i < 32; i++ )
		{
			Vector vecDelta = m_prgParticles[ i ]->pev->origin - pev->origin;
			Vector vecCurrentVel = m_prgParticles[ i ]->pev->velocity;

			if ( vecDelta.Length() > 300 )
				m_prgParticles[ i ]->pev->velocity = vecCurrentVel * 0.85 + (-vecDelta)*0.15;
			else if ( vecDelta.Length() < 100 )
				m_prgParticles[ i ]->pev->velocity = vecCurrentVel * 0.6 + (vecDelta)*0.4;
			else
				m_prgParticles[ i ]->pev->velocity = vecCurrentVel / 1.01;

			m_prgParticles[ i ]->pev->scale = 300.0 / (1.0 + vecDelta.Length());

			for ( int j = 0; j < 32; j++ )
			{
				if ( j == i )
					continue;

				Vector vecPDelta = m_prgParticles[ i ]->pev->origin - m_prgParticles[ j ]->pev->origin;

				if ( vecPDelta.Length() < 180 )
				{
					m_prgParticles[ i ]->pev->velocity = vecCurrentVel + vecPDelta / 16.0;

					float flRand = RANDOM_FLOAT( 0, vecPDelta.Length() / 72.0 );

					m_prgParticles[ i ]->pev->velocity = m_prgParticles[ i ]->pev->velocity + Vector( flRand, flRand, flRand );
				}
			}
		}

		break;
	}

	pev->nextthink = gpGlobals->time + 0.01;
}

  // ============================================================================================== //
 // ----------------------------------- PARAMETRIC EFFECTS --------------------------------------- //
// ============================================================================================== //

// AdmSrc - CEfxTest
// Purely hardcoded, test effect entity.
// It's an example of using the EFX API in the HL DLL. Apparently, you cannot send two messages in one function call. Interesting.
class CEfxTest : public CBaseEntity
{
public:
	void Spawn(void);
	
//	void KeyValue(KeyValueData *pkvd);
//	void GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64]);
//	void SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64]);

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

/*private:
	string_t m_iszParticle1;
	string_t m_iszParticle2;
	string_t m_iszParticle3;
	string_t m_iszParticle4; */
};

LINK_ENTITY_TO_CLASS(efx_test, CEfxTest);

void CEfxTest::Spawn(void)
{
	// Nothing uwu
}

void CEfxTest::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_IMPLOSION); // effect
	WRITE_COORD(pev->origin.x); // coords
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_BYTE(128); // radius
	WRITE_BYTE(32);	// count
	WRITE_BYTE(2);	// life in 0.1s
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_IMPLOSION); // effect
	WRITE_COORD(pev->origin.x); // coords
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_BYTE(256); // radius
	WRITE_BYTE(8);	// count
	WRITE_BYTE(4);	// life in 0.1s
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_TRACER);
	WRITE_COORD(pev->origin.x); // coords
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD((pev->origin.x + 64)); // coords
	WRITE_COORD((pev->origin.y + 64));
	WRITE_COORD((pev->origin.z + 64));
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_LINE);
	WRITE_COORD(pev->origin.x); // coords
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD((pev->origin.x + 64)); // coords
	WRITE_COORD((pev->origin.y + 64));
	WRITE_COORD((pev->origin.z + 64));
	WRITE_SHORT(5);
	WRITE_BYTE(255);
	WRITE_BYTE(128);
	WRITE_BYTE(0);
	MESSAGE_END();

	/*
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, origin );
		WRITE_BYTE( TE_BLOODSTREAM );
		WRITE_COORD( origin.x );
		WRITE_COORD( origin.y );
		WRITE_COORD( origin.z );
		WRITE_COORD( direction.x );
		WRITE_COORD( direction.y );
		WRITE_COORD( direction.z );
		WRITE_BYTE( color );
		WRITE_BYTE( V_min( amount, 255 ) );
		MESSAGE_END(); 
	*/ // Dis mah referens uwu
}