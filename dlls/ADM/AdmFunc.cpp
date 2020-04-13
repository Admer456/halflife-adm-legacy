/*

======= adm_functional.cpp ===========================

	Functional entities. (func_)

	func_loadbar - a brush which moves in a direction, "fills" the more times you trigger it, and fires things from 5 percentages
	It's planned for this to be dynamic, looking up keyvalues and assigning to each percentage a target
*/

#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#define F_PERCENTAGE1 1 << 0
#define F_PERCENTAGE2 1 << 1
#define F_PERCENTAGE3 1 << 2
#define F_PERCENTAGE4 1 << 3
#define F_PERCENTAGE5 1 << 4

#define SF_LOADMOVE 1 << 0
#define SF_LOADTARG 1 << 1

// ========================================================================================================= //
//		FUNC_ ENTITIES
// ========================================================================================================= //

class CFuncNovis : public CBaseEntity
{
public:
	void Spawn();
};

LINK_ENTITY_TO_CLASS(func_novis, CFuncNovis);

void CFuncNovis::Spawn()
{
	pev->classname = ALLOC_STRING("func_novis");
	pev->effects = EF_NODRAW;
	pev->solid = SOLID_BSP;
	pev->flags = FL_WORLDBRUSH;
	SET_MODEL(ENT(pev), STRING(pev->model));    // set size and link into world
	pev->movetype = MOVETYPE_PUSHSTEP;
}

class CFuncLoadbar : public CBaseEntity
{
public:
	void Spawn();
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value);

private:
	float m_flNudge = 0.5;

	void TryPercentage1(CBaseEntity *pActivator, USE_TYPE USETYPE, float value);
	void TryPercentage2(CBaseEntity *pActivator, USE_TYPE USETYPE, float value);
	void TryPercentage3(CBaseEntity *pActivator, USE_TYPE USETYPE, float value);
	void TryPercentage4(CBaseEntity *pActivator, USE_TYPE USETYPE, float value);
	void TryPercentage5(CBaseEntity *pActivator, USE_TYPE USETYPE, float value);

	int m_iPercentage1; // percentages are from 1 to 100
	int m_iPercentage2;
	int m_iPercentage3;
	int m_iPercentage4;
	int m_iPercentage5;

	bool m_fPercentage1 = false;
	bool m_fPercentage2 = false;
	bool m_fPercentage3 = false;
	bool m_fPercentage4 = false;
	bool m_fPercentage5 = false;

	int m_iCompletion = 0; // completion goes from 0 to 255

	string_t m_iszTarget1;
	string_t m_iszTarget2;
	string_t m_iszTarget3;
	string_t m_iszTarget4;
	string_t m_iszTarget5;
};

LINK_ENTITY_TO_CLASS(func_loadbar, CFuncLoadbar);

void CFuncLoadbar::Spawn()
{
	SET_MODEL(ENT(pev), STRING(pev->model)); // Set brush model
	m_iCompletion = 0;
}

void CFuncLoadbar::KeyValue(KeyValueData *pkvd)
{
	if (KeyvalueToken(t1))
	{
		m_iszTarget1 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "t2"))
	{
		m_iszTarget2 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "t3"))
	{
		m_iszTarget3 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "t4"))
	{
		m_iszTarget4 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "t5"))
	{
		m_iszTarget5 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "per1"))
	{
		m_iPercentage1 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "per2"))
	{
		m_iPercentage2 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "per3"))
	{
		m_iPercentage3 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "per4"))
	{
		m_iPercentage4 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "per5"))
	{
		m_iPercentage5 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else
		KeyvaluesFromBase(CBaseEntity);
}

void CFuncLoadbar::Use(CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value)
{
	m_iCompletion += 5;

	if (m_iCompletion > 255)
		m_iCompletion -= 5;

	if (pev->spawnflags & SF_LOADMOVE)
	{
		Vector vecDestination = pev->origin;
		vecDestination = vecDestination + UTIL_VecPolarCoords(m_flNudge, pev->angles);

		UTIL_SetOrigin(pev, vecDestination);
	}

	if (pev->spawnflags & SF_LOADTARG)
	{
		TryPercentage1(pActivator, useType, value);
		TryPercentage2(pActivator, useType, value);
		TryPercentage3(pActivator, useType, value);
		TryPercentage4(pActivator, useType, value);
		TryPercentage5(pActivator, useType, value);
	}
}

void CFuncLoadbar::TryPercentage1(CBaseEntity *pActivator, USE_TYPE USETYPE, float value)
{
	ALERT(at_console, "\nloadbar TP1, completed %d target at %d", (int)(m_iCompletion * 0.392), m_iPercentage1);

	if ((int)(m_iCompletion * 0.392) < m_iPercentage1)
	{
		return;
	}

	else if ((int)(m_iCompletion * 0.392) >= m_iPercentage1)
	{
		if (m_fPercentage1)
		{
			return;
		}

		FireTargets(STRING(m_iszTarget1), pActivator, this, USETYPE, value);
		m_fPercentage1 = true;
	}
}

void CFuncLoadbar::TryPercentage2(CBaseEntity *pActivator, USE_TYPE USETYPE, float value)
{
	if ((int)(m_iCompletion * 0.392) < m_iPercentage2)
	{
		return;
	}

	else if ((int)(m_iCompletion * 0.392) >= m_iPercentage2)
	{
		if (m_fPercentage2)
		{
			return;
		}

		FireTargets(STRING(m_iszTarget2), pActivator, this, USETYPE, value);
		m_fPercentage2 = true;
	}
}

void CFuncLoadbar::TryPercentage3(CBaseEntity *pActivator, USE_TYPE USETYPE, float value)
{
	if ((int)(m_iCompletion * 0.392) < m_iPercentage3)
	{
		return;
	}

	else if ((int)(m_iCompletion * 0.392) >= m_iPercentage3)
	{
		if (m_fPercentage3)
		{
			return;
		}

		FireTargets(STRING(m_iszTarget3), pActivator, this, USETYPE, value);
		m_fPercentage3 = true;
	}
}

void CFuncLoadbar::TryPercentage4(CBaseEntity *pActivator, USE_TYPE USETYPE, float value)
{
	if ((int)(m_iCompletion * 0.392) < m_iPercentage4)
	{
		return;
	}

	else if ((int)(m_iCompletion * 0.392) >= m_iPercentage4)
	{
		if (m_fPercentage4)
		{
			return;
		}

		FireTargets(STRING(m_iszTarget4), pActivator, this, USETYPE, value);
		m_fPercentage4 = true;
	}
}

void CFuncLoadbar::TryPercentage5(CBaseEntity *pActivator, USE_TYPE USETYPE, float value)
{
	if ((int)(m_iCompletion * 0.392) < m_iPercentage5)
	{
		return;
	}

	else if ((int)(m_iCompletion * 0.392) >= m_iPercentage5)
	{
		if (m_fPercentage5)
		{
			return;
		}

		FireTargets(STRING(m_iszTarget5), pActivator, this, USETYPE, value);
		m_fPercentage5 = true;
	}
}

#include <chrono>
using namespace std::chrono;

class CFuncLag : public CBaseEntity
{
public:
	void Use( CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value );
};

LINK_ENTITY_TO_CLASS( func_lag, CFuncLag );

// force optimisations so we get equal results in debug and release builds
#pragma optimize("s", on)
#pragma optimize("g", on)

void CFuncLag::Use( CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value )
{
	TraceResult tr;

	float currentFrametime = gpGlobals->frametime;

	auto lagStart = std::chrono::system_clock::now();

	for ( int height = 0; height < 256; height++ )
	{
		for ( int width = 0; width < 256; width++ )
		{
			UTIL_TraceLine( pev->origin, pev->origin + Vector( 1024, width - 128, height - 128 ), ignore_monsters, ENT( pev ), &tr );
		}
	}

	auto lagEnd = std::chrono::system_clock::now();

	duration<float> lagTime = lagEnd - lagStart;

	float framerateCost = 60.0 - (1 / (0.016 + lagTime.count()));

	ALERT( at_console, "\nLag successful, with 65'536 tracelines at a cost of %f seconds, costing us %f frames per second if we were at 60fps\n", lagTime.count(), framerateCost );
}