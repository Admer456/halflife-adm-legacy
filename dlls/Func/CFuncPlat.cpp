#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "CBasePlatTrain.h"
#include "CFuncPlat.h"

static void PlatSpawnInsideTrigger( entvars_t* pevPlatform )
{
	GetClassPtr( (CPlatTrigger *)NULL )->SpawnInsideTrigger( GetClassPtr( (CFuncPlat *)pevPlatform ) );
}

LINK_ENTITY_TO_CLASS( func_plat, CFuncPlat );

void CFuncPlat::Setup( void )
{
	//pev->noiseMovement = MAKE_STRING("plats/platmove1.wav");
	//pev->noiseStopMoving = MAKE_STRING("plats/platstop1.wav");

	if ( m_flTLength == 0 )
		m_flTLength = 80;
	if ( m_flTWidth == 0 )
		m_flTWidth = 10;

	pev->angles = g_vecZero;

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin( pev, pev->origin );		// set size and link into world
	UTIL_SetSize( pev, pev->mins, pev->maxs );
	SET_MODEL( ENT( pev ), STRING( pev->model ) );

	// vecPosition1 is the top position, vecPosition2 is the bottom
	m_vecPosition1 = pev->origin;
	m_vecPosition2 = pev->origin;
	if ( m_flHeight != 0 )
		m_vecPosition2.z = pev->origin.z - m_flHeight;
	else
		m_vecPosition2.z = pev->origin.z - pev->size.z + 8;
	if ( pev->speed == 0 )
		pev->speed = 150;

	if ( m_volume == 0 )
		m_volume = 0.85;
}

void CFuncPlat::Precache()
{
	CBasePlatTrain::Precache();
	//PRECACHE_SOUND("plats/platmove1.wav");
	//PRECACHE_SOUND("plats/platstop1.wav");
	if ( !IsTogglePlat() )
		PlatSpawnInsideTrigger( pev );		// the "start moving" trigger
}

void CFuncPlat::Spawn()
{
	Setup();

	Precache();

	// If this platform is the target of some button, it starts at the TOP position,
	// and is brought down by that button.  Otherwise, it starts at BOTTOM.
	if ( !FStringNull( pev->targetname ) )
	{
		UTIL_SetOrigin( pev, m_vecPosition1 );
		m_toggle_state = TS_AT_TOP;
		SetUse( &CFuncPlat::PlatUse );
	}
	else
	{
		UTIL_SetOrigin( pev, m_vecPosition2 );
		m_toggle_state = TS_AT_BOTTOM;
	}
}

//
// Create a trigger entity for a platform.
//
void CPlatTrigger::SpawnInsideTrigger( CFuncPlat *pPlatform )
{
	m_pPlatform = pPlatform;
	// Create trigger entity, "point" it at the owning platform, give it a touch method
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	pev->origin = pPlatform->pev->origin;

	// Establish the trigger field's size
	Vector vecTMin = m_pPlatform->pev->mins + Vector( 25, 25, 0 );
	Vector vecTMax = m_pPlatform->pev->maxs + Vector( 25, 25, 8 );
	vecTMin.z = vecTMax.z - (m_pPlatform->m_vecPosition1.z - m_pPlatform->m_vecPosition2.z + 8);
	if ( m_pPlatform->pev->size.x <= 50 )
	{
		vecTMin.x = (m_pPlatform->pev->mins.x + m_pPlatform->pev->maxs.x) / 2;
		vecTMax.x = vecTMin.x + 1;
	}
	if ( m_pPlatform->pev->size.y <= 50 )
	{
		vecTMin.y = (m_pPlatform->pev->mins.y + m_pPlatform->pev->maxs.y) / 2;
		vecTMax.y = vecTMin.y + 1;
	}
	UTIL_SetSize( pev, vecTMin, vecTMax );
}


//
// When the platform's trigger field is touched, the platform ???
//
void CPlatTrigger::Touch( CBaseEntity *pOther )
{
	// Ignore touches by non-players
	entvars_t*	pevToucher = pOther->pev;
	if ( !FClassnameIs( pevToucher, "player" ) )
		return;

	// Ignore touches by corpses
	if ( !pOther->IsAlive() || !m_pPlatform || !m_pPlatform->pev )
		return;

	// Make linked platform go up/down.
	if ( m_pPlatform->m_toggle_state == TS_AT_BOTTOM )
		m_pPlatform->GoUp();
	else if ( m_pPlatform->m_toggle_state == TS_AT_TOP )
		m_pPlatform->pev->nextthink = m_pPlatform->pev->ltime + 1;// delay going down
}


//
// Used by SUB_UseTargets, when a platform is the target of a button.
// Start bringing platform down.
//
void CFuncPlat::PlatUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( IsTogglePlat() )
	{
		// Top is off, bottom is on
		BOOL on = (m_toggle_state == TS_AT_BOTTOM) ? TRUE : FALSE;

		if ( !ShouldToggle( useType, on ) )
			return;

		if ( m_toggle_state == TS_AT_TOP )
			GoDown();
		else if ( m_toggle_state == TS_AT_BOTTOM )
			GoUp();
	}
	else
	{
		SetUse( NULL );

		if ( m_toggle_state == TS_AT_TOP )
			GoDown();
	}
}


//
// Platform is at top, now starts moving down.
//
void CFuncPlat::GoDown( void )
{
	if ( pev->noiseMovement )
		EMIT_SOUND( ENT( pev ), CHAN_STATIC, (char*)STRING( pev->noiseMovement ), m_volume, ATTN_NORM );

	ASSERT( m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP );
	m_toggle_state = TS_GOING_DOWN;
	SetMoveDone( &CFuncPlat::CallHitBottom );
	LinearMove( m_vecPosition2, pev->speed );
}


//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncPlat::HitBottom( void )
{
	if ( pev->noiseMovement )
		STOP_SOUND( ENT( pev ), CHAN_STATIC, (char*)STRING( pev->noiseMovement ) );

	if ( pev->noiseStopMoving )
		EMIT_SOUND( ENT( pev ), CHAN_WEAPON, (char*)STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );

	ASSERT( m_toggle_state == TS_GOING_DOWN );
	m_toggle_state = TS_AT_BOTTOM;
}


//
// Platform is at bottom, now starts moving up
//
void CFuncPlat::GoUp( void )
{
	if ( pev->noiseMovement )
		EMIT_SOUND( ENT( pev ), CHAN_STATIC, (char*)STRING( pev->noiseMovement ), m_volume, ATTN_NORM );

	ASSERT( m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN );
	m_toggle_state = TS_GOING_UP;
	SetMoveDone( &CFuncPlat::CallHitTop );
	LinearMove( m_vecPosition1, pev->speed );
}


//
// Platform has hit top.  Pauses, then starts back down again.
//
void CFuncPlat::HitTop( void )
{
	if ( pev->noiseMovement )
		STOP_SOUND( ENT( pev ), CHAN_STATIC, (char*)STRING( pev->noiseMovement ) );

	if ( pev->noiseStopMoving )
		EMIT_SOUND( ENT( pev ), CHAN_WEAPON, (char*)STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );

	ASSERT( m_toggle_state == TS_GOING_UP );
	m_toggle_state = TS_AT_TOP;

	if ( !IsTogglePlat() )
	{
		// After a delay, the platform will automatically start going down again.
		SetThink( &CFuncPlat::CallGoDown );
		pev->nextthink = pev->ltime + 3;
	}
}


void CFuncPlat::Blocked( CBaseEntity *pOther )
{
	ALERT( at_aiconsole, "%s Blocked by %s\n", STRING( pev->classname ), STRING( pOther->pev->classname ) );
	// Hurt the blocker a little
	pOther->TakeDamage( pev, pev, 1, DMG_CRUSH );

	if ( pev->noiseMovement )
		STOP_SOUND( ENT( pev ), CHAN_STATIC, (char*)STRING( pev->noiseMovement ) );

	// Send the platform back where it came from
	ASSERT( m_toggle_state == TS_GOING_UP || m_toggle_state == TS_GOING_DOWN );
	if ( m_toggle_state == TS_GOING_UP )
		GoDown();
	else if ( m_toggle_state == TS_GOING_DOWN )
		GoUp();
}
