/*

====== story.cpp ==================================

	Story entities and logic.

*/

#include "Base/ExtDLL.h"
#include "Util.h"
//#include "steam_util.h"
#include "Base/CBase.h"
#include "player.h"
#include "Base/SaveRestore.h"
#include "Game/Game.h"
#include "Game/GameRules.h"

// ----- CStoryGlobal - story_value -----
//
// Holds a story value. Right now I think it's gonna work with env_global
class CStoryGlobal : public CBaseEntity
{
public:
	void	Spawn(void);
	
	void	KeyValue(KeyValueData *pkvd);
	void	GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64]);
	void	SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64]);

	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
	virtual int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & FCAP_ACROSS_TRANSITION; }

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iStoryValue;
	GLOBALESTATE m_gStoryValue;
};

LINK_ENTITY_TO_CLASS(story_value, CStoryGlobal);

TYPEDESCRIPTION CStoryGlobal::m_SaveData[] =
{
	DEFINE_GLOBAL_FIELD(CStoryGlobal, m_iStoryValue, FIELD_INTEGER),
};

void CStoryGlobal::Spawn(void)
{
//	gGlobalState.EntityAdd(pev->targetname, gpGlobals->mapname, GLOBAL_OFF);
	gGlobalState.EntitySetState(pev->targetname, GLOBAL_OFF);
	gGlobalState.DumpGlobals(); // DumpGlobals is for debugging, can be manually called via impulse 104
}

void CStoryGlobal::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "storyvalue"))
	{
		m_iStoryValue = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else
		pkvd->fHandled = FALSE;
}

void CStoryGlobal::GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "storyvalue"))
	{
		sprintf(&szTheValue, "%d", m_iStoryValue);
	}

	else
	{
		sprintf(&szTheValue, "null");
		return;
	}
}

void CStoryGlobal::SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "storyvalue"))
	{
		m_iStoryValue = atoi(szTheValue);
	}

	else
	{
		ALERT(at_error, "No matching keyvalue %s for %s", szTargetKeyName, STRING(pev->targetname));
	}
}

int CStoryGlobal::Save(CSave &save)
{
	if (!CBaseEntity::Save(save))
		return 0;

	return save.WriteFields("STORY_VALUE", this, m_SaveData, ARRAYSIZE(m_SaveData));
}

int CStoryGlobal::Restore(CRestore &restore)
{
	if (!CBaseEntity::Restore(restore))
		return 0;

	int status = restore.ReadFields("STORY_VALUE", this, m_SaveData, ARRAYSIZE(m_SaveData));
	return status;
}

// ----- CStoryTrigger - story_trigger -----
//
// Reads a story value from its targeted global state, and executes according to the setup
class CStoryTrigger : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	void	KeyValue(KeyValueData *pkvd);
	void	GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64]);
	void	SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64]);

	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & FCAP_ACROSS_TRANSITION; }

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iStoryValue;
	GLOBALESTATE m_gStoryValue;
};

LINK_ENTITY_TO_CLASS(story_trigger, CStoryTrigger);

TYPEDESCRIPTION CStoryTrigger::m_SaveData[] =
{
	DEFINE_FIELD(CStoryTrigger, m_iStoryValue, FIELD_INTEGER),
};

void CStoryTrigger::Spawn(void)
{
	// Nothing :P
	gGlobalState.EntityGetState(pev->target);
}

void CStoryTrigger::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "storyvalue"))
	{
		m_iStoryValue = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		pkvd->fHandled = FALSE;
}

void CStoryTrigger::GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "storyvalue"))
	{
		sprintf(&szTheValue, "%d", m_iStoryValue);
	}

	else
	{
		sprintf(&szTheValue, "null");
		return;
	}
}

void CStoryTrigger::SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "storyvalue"))
	{
		m_iStoryValue = atoi(szTheValue);
	}

	else
	{
		ALERT(at_error, "No matching keyvalue %s for %s", szTargetKeyName, STRING(pev->targetname));
	}
}

int CStoryTrigger::Save(CSave &save)
{
	if (!CBaseEntity::Save(save))
		return 0;

	return save.WriteFields("STORY_TRIGGER", this, m_SaveData, ARRAYSIZE(m_SaveData));
}

int CStoryTrigger::Restore(CRestore &restore)
{
	if (!CBaseEntity::Restore(restore))
		return 0;

	int status = restore.ReadFields("STORY_TRIGGER", this, m_SaveData, ARRAYSIZE(m_SaveData));
	return status;
}

void CStoryTrigger::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	int iStoryValue;
	char szStoryValue[64];

	CBaseEntity *pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));

	// If the entity is already in the map, we might as well use that one instead of the globals
	if (pTarget)
	{
		pTarget->GetKeyValueCustom(*szStoryValue, "storyvalue");
		// This gets the keyvalue from our entity and stores it as a string

		iStoryValue = atoi(szStoryValue);
		// And then a simple conversion from string to int
	}
	else
	{
		m_gStoryValue = gGlobalState.EntityGetState(pev->target); 
		// Here we get the choice itself, if the entity is in another map

		switch (m_gStoryValue)
		{
		case GLOBAL_OFF:
			iStoryValue = 0; break;

		case GLOBAL_ON:
			iStoryValue = 1; break;

		case GLOBAL_DEAD:
			iStoryValue = 2; break;

		case ADM_CHOICE1:
			iStoryValue = 3; break;

		case ADM_CHOICE2:
			iStoryValue = 4; break;

		case ADM_CHOICE3:
			iStoryValue = 5; break;

		case ADM_CHOICE4:
			iStoryValue = 6; break;

		case ADM_CHOICE5:
			iStoryValue = 7; break;
		}
	}
		
	if (iStoryValue == m_iStoryValue) // This is purely debugging stuff, nothing special - will require developer 2
	{
		SUB_UseTargets(this, useType, value);
		ALERT(at_aiconsole, "%s triggered %s\nStoryvalue %d is a match\n", STRING(pev->targetname), STRING(pev->target), iStoryValue);
	}

	else
	{
		ALERT(at_aiconsole, "%s did not trigger %s\nStoryvalue is %d, required %d\n", STRING(pev->targetname), STRING(pev->target), iStoryValue, m_iStoryValue);
	}
}