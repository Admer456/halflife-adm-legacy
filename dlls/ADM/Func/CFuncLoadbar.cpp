#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "CFuncLoadbar.h"

LINK_ENTITY_TO_CLASS( func_loadbar, CFuncLoadbar );

TYPEDESCRIPTION CFuncLoadbar::m_SaveData[] =
{
	DEFINE_FIELD( CFuncLoadbar, m_iPercentage1, FIELD_INTEGER ),
	DEFINE_FIELD( CFuncLoadbar, m_iPercentage2, FIELD_INTEGER ),
	DEFINE_FIELD( CFuncLoadbar, m_iPercentage3, FIELD_INTEGER ),
	DEFINE_FIELD( CFuncLoadbar, m_iPercentage4, FIELD_INTEGER ),
	DEFINE_FIELD( CFuncLoadbar, m_iPercentage5, FIELD_INTEGER ),

	DEFINE_FIELD( CFuncLoadbar, m_fPercentage1, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFuncLoadbar, m_fPercentage2, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFuncLoadbar, m_fPercentage3, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFuncLoadbar, m_fPercentage4, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFuncLoadbar, m_fPercentage5, FIELD_BOOLEAN ),

	DEFINE_FIELD( CFuncLoadbar, m_iCompletion, FIELD_BOOLEAN ),

	DEFINE_FIELD( CFuncLoadbar, m_iszTarget1, FIELD_STRING ),
	DEFINE_FIELD( CFuncLoadbar, m_iszTarget2, FIELD_STRING ),
	DEFINE_FIELD( CFuncLoadbar, m_iszTarget3, FIELD_STRING ),
	DEFINE_FIELD( CFuncLoadbar, m_iszTarget4, FIELD_STRING ),
	DEFINE_FIELD( CFuncLoadbar, m_iszTarget5, FIELD_STRING )
};

IMPLEMENT_SAVERESTORE( CFuncLoadbar, CBaseEntity );

void CFuncLoadbar::Spawn()
{
	SET_MODEL( ENT( pev ), STRING( pev->model ) ); // Set brush model
	m_iCompletion = 0;
}

void CFuncLoadbar::KeyValue( KeyValueData *pkvd )
{
	if ( KeyvalueToken( t1 ) )
	{
		m_iszTarget1 = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "t2" ) )
	{
		m_iszTarget2 = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "t3" ) )
	{
		m_iszTarget3 = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "t4" ) )
	{
		m_iszTarget4 = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "t5" ) )
	{
		m_iszTarget5 = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "per1" ) )
	{
		m_iPercentage1 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "per2" ) )
	{
		m_iPercentage2 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "per3" ) )
	{
		m_iPercentage3 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "per4" ) )
	{
		m_iPercentage4 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "per5" ) )
	{
		m_iPercentage5 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else
		KeyvaluesFromBase( CBaseEntity );
}

void CFuncLoadbar::Use( CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value )
{
	m_iCompletion += 5;

	if ( m_iCompletion > 255 )
	{
		m_iCompletion -= 5;
		return;
	}

	if ( pev->spawnflags & SF_LOADMOVE )
	{
		Vector vecDestination = pev->origin;
		vecDestination = vecDestination + UTIL_VecPolarCoords( m_flNudge, pev->angles );

		UTIL_SetOrigin( pev, vecDestination );
	}

	if ( pev->spawnflags & SF_LOADTARG )
	{
		TryPercentage1( pActivator, useType, value );
		TryPercentage2( pActivator, useType, value );
		TryPercentage3( pActivator, useType, value );
		TryPercentage4( pActivator, useType, value );
		TryPercentage5( pActivator, useType, value );
	}
}

void CFuncLoadbar::TryPercentage1( CBaseEntity *pActivator, USE_TYPE USETYPE, float value )
{
	if ( (int)(m_iCompletion * 0.392) < m_iPercentage1 )
	{
		return;
	}

	else if ( (int)(m_iCompletion * 0.392) >= m_iPercentage1 )
	{
		if ( m_fPercentage1 )
		{
			return;
		}

		FireTargets( STRING( m_iszTarget1 ), pActivator, this, USETYPE, value );
		m_fPercentage1 = true;
	}
}

void CFuncLoadbar::TryPercentage2( CBaseEntity *pActivator, USE_TYPE USETYPE, float value )
{
	if ( (int)(m_iCompletion * 0.392) < m_iPercentage2 )
	{
		return;
	}

	else if ( (int)(m_iCompletion * 0.392) >= m_iPercentage2 )
	{
		if ( m_fPercentage2 )
		{
			return;
		}

		FireTargets( STRING( m_iszTarget2 ), pActivator, this, USETYPE, value );
		m_fPercentage2 = true;
	}
}

void CFuncLoadbar::TryPercentage3( CBaseEntity *pActivator, USE_TYPE USETYPE, float value )
{
	if ( (int)(m_iCompletion * 0.392) < m_iPercentage3 )
	{
		return;
	}

	else if ( (int)(m_iCompletion * 0.392) >= m_iPercentage3 )
	{
		if ( m_fPercentage3 )
		{
			return;
		}

		FireTargets( STRING( m_iszTarget3 ), pActivator, this, USETYPE, value );
		m_fPercentage3 = true;
	}
}

void CFuncLoadbar::TryPercentage4( CBaseEntity *pActivator, USE_TYPE USETYPE, float value )
{
	if ( (int)(m_iCompletion * 0.392) < m_iPercentage4 )
	{
		return;
	}

	else if ( (int)(m_iCompletion * 0.392) >= m_iPercentage4 )
	{
		if ( m_fPercentage4 )
		{
			return;
		}

		FireTargets( STRING( m_iszTarget4 ), pActivator, this, USETYPE, value );
		m_fPercentage4 = true;
	}
}

void CFuncLoadbar::TryPercentage5( CBaseEntity *pActivator, USE_TYPE USETYPE, float value )
{
	if ( (int)(m_iCompletion * 0.392) < m_iPercentage5 )
	{
		return;
	}

	else if ( (int)(m_iCompletion * 0.392) >= m_iPercentage5 )
	{
		if ( m_fPercentage5 )
		{
			return;
		}

		FireTargets( STRING( m_iszTarget5 ), pActivator, this, USETYPE, value );
		m_fPercentage5 = true;
	}
}
