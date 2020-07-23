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

===== lights.cpp ========================================================

  spawn and think functions for editor-placed lights

*/

#include "Base/extdll.h"
#include "Util.h"
#include "Base/cbase.h"

class CLight : public CPointEntity
{
public:
	virtual void	KeyValue( KeyValueData* pkvd ); 
	virtual void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	int				GetStyle() { return m_iStyle; }

private:
	int		m_iStyle;
	int		m_iszPattern;
};
LINK_ENTITY_TO_CLASS( light, CLight );

TYPEDESCRIPTION	CLight::m_SaveData[] = 
{
	DEFINE_FIELD( CLight, m_iStyle, FIELD_INTEGER ),
	DEFINE_FIELD( CLight, m_iszPattern, FIELD_STRING ),
};

IMPLEMENT_SAVERESTORE( CLight, CPointEntity );

//
// Cache user-entity-field values until spawn is called.
//
void CLight :: KeyValue( KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style"))
	{
		m_iStyle = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		pev->angles.x = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pattern"))
	{
		m_iszPattern = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
	{
		CPointEntity::KeyValue( pkvd );
	}
}

/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) LIGHT_START_OFF
Non-displayed light.
Default light value is 300
Default style is 0
If targeted, it will toggle between on or off.
*/

void CLight :: Spawn( void )
{
	if (FStringNull(pev->targetname))
	{       // inert light
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	if (m_iStyle >= 32)
	{
//		CHANGE_METHOD(ENT(pev), em_use, light_use);
		if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
			LIGHT_STYLE(m_iStyle, "a");
		else if (m_iszPattern)
			LIGHT_STYLE(m_iStyle, (char *)STRING( m_iszPattern ));
		else
			LIGHT_STYLE(m_iStyle, "m");
	}
}


void CLight :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if (m_iStyle >= 32)
	{
		if ( !ShouldToggle( useType, !FBitSet(pev->spawnflags, SF_LIGHT_START_OFF) ) )
			return;

		if (FBitSet(pev->spawnflags, SF_LIGHT_START_OFF))
		{
			if (m_iszPattern)
				LIGHT_STYLE(m_iStyle, (char *)STRING( m_iszPattern ));
			else
				LIGHT_STYLE(m_iStyle, "m");
			ClearBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
		else
		{
			LIGHT_STYLE(m_iStyle, "a");
			SetBits(pev->spawnflags, SF_LIGHT_START_OFF);
		}
	}
}

//
// shut up spawn functions for new spotlights
//
LINK_ENTITY_TO_CLASS( light_spot, CLight );

class CLightFading : public CLight
{
public:
	void		Spawn() override;
	void		KeyValue( KeyValueData* pkvd ) override;
	void		Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value ) override;
	void		Think() override;

	int			Save( CSave& save ) override;
	int			Restore( CRestore& restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];

	enum LightModes
	{
		Light_Off,
		Light_On
	};

private:
	int			lightMode;
	char		lightIntensity[2];
	float		lightUpdateTime{0.1f}; // lightmaps refresh at 10 Hz by default
};

LINK_ENTITY_TO_CLASS( light_fading, CLightFading );

TYPEDESCRIPTION CLightFading::m_SaveData[] =
{
	DEFINE_FIELD( CLightFading, lightMode, FIELD_FLOAT ),
	DEFINE_FIELD( CLightFading, lightUpdateTime, FIELD_FLOAT )
};

IMPLEMENT_SAVERESTORE( CLightFading, CLight );

void CLightFading::Spawn()
{
	CLight::Spawn();

	if ( pev->spawnflags & SF_LIGHT_START_OFF )
	{
		lightMode = Light_Off;
		lightIntensity[0] = 'a';
	}

	else
	{
		lightMode = Light_On;
		lightIntensity[0] = 'z';
	}

	lightIntensity[1] = '\0';

	LIGHT_STYLE( GetStyle(), lightIntensity );

	pev->nextthink = -1;
}

void CLightFading::KeyValue( KeyValueData* pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "lightFrequency" ) )
	{
		lightUpdateTime = 1.0f / (atof( pkvd->szValue ));
		pkvd->fHandled = TRUE;
	}

	else
	{
		CLight::KeyValue( pkvd );
	}
}

void CLightFading::Use( CBaseEntity * activator, CBaseEntity * caller, USE_TYPE useType, float value )
{
	lightMode = !lightMode;

	pev->nextthink = gpGlobals->time + 0.001;
}

void CLightFading::Think()
{
	int time = gpGlobals->time * 100;
	bool canSwitchLights = time % 10 == 0;

	if ( lightMode == Light_On && lightIntensity[0] < 'z' )
	{
		lightIntensity[0]++;
	}
	else if ( lightMode == Light_Off && lightIntensity[0] > 'a' )
	{
		lightIntensity[0]--;
	}

	LIGHT_STYLE( GetStyle(), lightIntensity );

	pev->nextthink = gpGlobals->time + lightUpdateTime;
}

class CEnvLight : public CLight
{
public:
	void	KeyValue( KeyValueData* pkvd ); 
	void	Spawn( void );
};

LINK_ENTITY_TO_CLASS( light_environment, CEnvLight );

void CEnvLight::KeyValue( KeyValueData* pkvd )
{
	if (FStrEq(pkvd->szKeyName, "_light"))
	{
		int r, g, b, v, j;
		char szColor[64];
		j = sscanf( pkvd->szValue, "%d %d %d %d\n", &r, &g, &b, &v );
		if (j == 1)
		{
			g = b = r;
		}
		else if (j == 4)
		{
			r = r * (v / 255.0);
			g = g * (v / 255.0);
			b = b * (v / 255.0);
		}

		// simulate qrad direct, ambient,and gamma adjustments, as well as engine scaling
		r = pow( r / 114.0, 0.6 ) * 264;
		g = pow( g / 114.0, 0.6 ) * 264;
		b = pow( b / 114.0, 0.6 ) * 264;

		pkvd->fHandled = TRUE;
		sprintf( szColor, "%d", r );
		CVAR_SET_STRING( "sv_skycolor_r", szColor );
		sprintf( szColor, "%d", g );
		CVAR_SET_STRING( "sv_skycolor_g", szColor );
		sprintf( szColor, "%d", b );
		CVAR_SET_STRING( "sv_skycolor_b", szColor );
	}
	else
	{
		CLight::KeyValue( pkvd );
	}
}


void CEnvLight :: Spawn( void )
{
	char szVector[64];
	UTIL_MakeAimVectors( pev->angles );

	sprintf( szVector, "%f", gpGlobals->v_forward.x );
	CVAR_SET_STRING( "sv_skyvec_x", szVector );
	sprintf( szVector, "%f", gpGlobals->v_forward.y );
	CVAR_SET_STRING( "sv_skyvec_y", szVector );
	sprintf( szVector, "%f", gpGlobals->v_forward.z );
	CVAR_SET_STRING( "sv_skyvec_z", szVector );

	CLight::Spawn( );
}
