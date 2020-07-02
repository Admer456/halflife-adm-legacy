/*

======= adm_triggers.cpp ==============================

	Triggers and map utilities.

	trigger_valueop
	- change a keyvalue of another entity
	trigger_changevalue
	- the thing from SoHL that I copy-pasted for reference - doesn't work in my mod because I have an insane implementation of my own
	trigger_date and trigger_date2
	- can trigger something only if a certain time is met
	trigger_difficulty
	- triggers something depending on the chosen difficulty
	trigger_timer
	- quite simply, a trigger that automatically triggers stuff every X seconds


	util_consoleprinter
	- literally prints stuff to the console
	util_rotator
	- rotates things by changing their origins and angle KVs
	util_rot_noorg
	- rotates solid entities that don't have an origin brush - don't use on point entities or origin'ed solid entities

*/

#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "player.h"
#include "Base/SaveRestore.h"
#include "Game/GameRules.h"
#include "skill.h"
#include "adm/shared/AdmTime.h"

#include <cmath>
#include <string>

// ========================================================================================================= //
//		UTILITIES
// ========================================================================================================= //

void SUB_KillTargets(string_t target)
{
	if (target)
	{
		edict_t *pentKillTarget = NULL;

		ALERT(at_aiconsole, "KillTarget: %s\n", STRING(target));
		pentKillTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(target));

		while (!FNullEnt(pentKillTarget))
		{
			UTIL_Remove(CBaseEntity::Instance(pentKillTarget));

			ALERT(at_aiconsole, "killing %s\n", STRING(pentKillTarget->v.classname));
			pentKillTarget = FIND_ENTITY_BY_TARGETNAME(pentKillTarget, STRING(target));
		}
	}

	else
		return;
}

// ========================================================================================================= //
//		TRIGGER_ ENTITIES
// ========================================================================================================= //

//--------------------------
//-----trigger_valueop------
//--------------------------
/*
	THIS is a bad idea. >:D
	changes entity keyvalues
	and performs other operations on them, when triggered
*/

class CTriggerValueOperator : public CBaseDelay // a.k.a. CChangeValue
{
public:
	void				Spawn(void);
	void				KeyValue(KeyValueData *pkvd);
	void				GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64]);
	void				SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64]);

	void				Use			(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void				UseString	(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void				UseVector	(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void				UseInt		(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void				UseFloat	(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

private:
	string_t			m_iszNewValue;
	string_t			m_iszTargetKey;
	string_t			m_iszTargetEntity;
	string_t			m_iszTargetEntityForFire;

	unsigned short int	iOperatingMode;
	USE_TYPE			triggerType;
};

enum ValueOpMode
{
	Replace,
	Add,
	Subtract,
	Multiply,
	Divide,
	Modulo,

	Sin,
	Cos,
	Tg,
	Ctg,

	AND,
	NAND,

	Greater,
	GreaterOrEqual,
	Less,
	LessOrEqual
};

#define	SF_VAL_FLOAT	1
#define	SF_VAL_INT		2
#define	SF_VAL_STRING	4
#define	SF_VAL_VECTOR	8
#define SF_VAL_RAD		16
#define SF_VAL_DEG		32
#define SF_VAL_ORAD		64
#define SF_VAL_ODEG		128

LINK_ENTITY_TO_CLASS(trigger_valueop, CTriggerValueOperator);

void CTriggerValueOperator::Spawn(void) {}

void CTriggerValueOperator::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "NewValue"))
	{
		m_iszNewValue = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "TargetKey"))
	{
		m_iszTargetKey = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "TargetEntity"))
	{
		m_iszTargetEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "TargetEntityForFire"))
	{
		m_iszTargetEntityForFire = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "iOperatingMode"))
	{
		iOperatingMode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);
		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;

		case 1:
			triggerType = USE_ON;
			break;

		case 2:
			triggerType = USE_SET;
			break;

		case 3:
			triggerType = USE_TOGGLE;
			break;
		}
		pkvd->fHandled = TRUE;
	}

	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerValueOperator::GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "NewValue"))
		sprintf(&szTheValue, "%s", STRING(m_iszNewValue));

	else if (FStrEq(szTargetKeyName, "TargetKey"))
		sprintf(&szTheValue, "%s", STRING(m_iszTargetKey));

	else if (FStrEq(szTargetKeyName, "TargetKey"))
		sprintf(&szTheValue, "%s", STRING(m_iszTargetEntity));

	else if (FStrEq(szTargetKeyName, "TargetEntityForFire"))
		sprintf(&szTheValue, "%s", STRING(m_iszTargetEntityForFire));

	else if (FStrEq(szTargetKeyName, "iOperatingMode"))
		sprintf(&szTheValue, "%i", iOperatingMode);

	else if (FStrEq(szTargetKeyName, "triggerstate"))
		sprintf(&szTheValue, "%i", triggerType);

	else
		sprintf(&szTheValue, "null (no matching KV)");
}

void CTriggerValueOperator::SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "NewValue"))
		m_iszNewValue = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "TargetKey"))
		m_iszTargetKey = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "TargetKey"))
		m_iszTargetEntity = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "TargetEntityForFire"))
		m_iszTargetEntityForFire = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "iOperatingMode"))
		iOperatingMode = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "triggerstate"))
	{
		int type = atoi(szTheValue);
		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;

		case 1:
			triggerType = USE_ON;
			break;

		case 2:
			triggerType = USE_SET;
			break;

		case 3:
			triggerType = USE_TOGGLE;
			break;
		}
	}

	else
		ALERT(at_error, "No matching KV %s in %s \n", szTargetKeyName, STRING(pev->targetname));
}

void CTriggerValueOperator::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (FBitSet(pev->spawnflags, SF_VAL_INT))
	{
		UseInt(pActivator, pCaller, useType, value);
	}
	else if (FBitSet(pev->spawnflags, SF_VAL_FLOAT))
	{
		UseFloat(pActivator, pCaller, useType, value);
	}
	else if (FBitSet(pev->spawnflags, SF_VAL_STRING))
	{
		UseString(pActivator, pCaller, useType, value);
	}
	else if (FBitSet(pev->spawnflags, SF_VAL_VECTOR))
	{
		UseVector(pActivator, pCaller, useType, value);
	}
}

void CTriggerValueOperator::UseInt(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)    
{
	// 1. Find entity and load data
	CBaseEntity *pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_iszTargetEntity));
	
	if ( FStrEq( STRING( m_iszTargetEntity ), "world" ) )
	{
		pTarget = UTIL_FindEntityByClassname( NULL, "worldspawn" );
	}

	if ( pTarget == nullptr )
	{
		ALERT( at_error, "Target %s doesn't exist (%s %s)", STRING(m_iszTargetEntity), STRING(pev->targetname), STRING(pev->classname) );
		return;
	}

	char szOldValue[64];
	pTarget->GetKeyValue(*szOldValue, (char*)STRING(m_iszTargetKey));

	int iOldValue = atoi(szOldValue);
	int iNewValue = atoi(STRING(m_iszNewValue));
	int iResult;
	bool fAction = false;

	// 2. Check op mode and do the operations
	switch (iOperatingMode)
	{
	default:
		ALERT(at_console, "\nInvalid operating mode for %s, using Replace", STRING(pev->targetname));

	case Replace: 
		iResult = iNewValue;
		fAction = true;
		break;

	case Add: 
		iResult = iNewValue + iOldValue;
		fAction = true;
		break;

	case Subtract:
		iResult = iOldValue - iNewValue;
		fAction = true;
		break;

	case Multiply:
		iResult = iOldValue * iNewValue;
		fAction = true;
		break;

	case Divide:
		iResult = iOldValue / iNewValue;
		fAction = true;
		break;

	case Modulo:
		iResult = iOldValue % iNewValue;
		fAction = true;
		break;
	
	case AND: 
		iResult = iOldValue;
		fAction = (iOldValue & iNewValue);
		break;
	
	case NAND: 
		iResult = iOldValue;
		fAction = !(iOldValue & iNewValue);
		break;

	case Greater:
		iResult = iOldValue;
		fAction = (iOldValue < iNewValue);
		break;

	case GreaterOrEqual:
		iResult = iOldValue;
		fAction = (iOldValue <= iNewValue);
		break;

	case Less:
		iResult = iOldValue;
		fAction = (iOldValue > iNewValue);
		break;

	case LessOrEqual:
		iResult = iOldValue;
		fAction = (iOldValue >= iNewValue);
		break;
	}

	// 3. If there's an action after the operation, fire it
	if (fAction)
	{
		FireTargets(STRING(m_iszTargetEntityForFire), pActivator, pCaller, triggerType, value);
	}

	// 4. Get result string from the int, and set the keyvalue for the target entity
	char szResult[64];
	sprintf(szResult, "%i", iResult);

	pTarget->SetKeyValue((char*)STRING(m_iszTargetKey), szResult);
}

void CTriggerValueOperator::UseFloat(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)  
{
	// 1. Find entity and load data
	CBaseEntity *pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_iszTargetEntity));
	
	if ( pTarget == nullptr )
	{
		ALERT( at_error, "Target %s doesn't exist (%s %s)", STRING( m_iszTargetEntity ), STRING( pev->targetname ), STRING( pev->classname ) );
		return;
	}

	char szOldValue[64];
	pTarget->GetKeyValue(*szOldValue, (char*)STRING(m_iszTargetKey));

	ALERT(at_console, "\n%s szOldValue: %s",
		STRING(pev->targetname),
		szOldValue);

	float flOldValue = atof(szOldValue);
	float flNewValue = atof(STRING(m_iszNewValue));
	float flResult;
	bool fAction = false;

	if (FBitSet(pev->spawnflags, SF_VAL_RAD))
	{
		UTIL_Deg2Rad(flOldValue);
		UTIL_Deg2Rad(flNewValue);
	}
	
	if (FBitSet(pev->spawnflags, SF_VAL_DEG))
	{
		UTIL_Rad2Deg(flOldValue);
		UTIL_Rad2Deg(flNewValue);
	}

	// 2. Check op mode and do the operations
	switch (iOperatingMode)
	{
	default:
		ALERT(at_console, "\nInvalid operating mode for %s, using Replace", STRING(pev->targetname));

	case Replace:
		flResult = flNewValue;
		fAction = true;
		break;

	case Add:
		flResult = flOldValue + flNewValue;
		fAction = true;
		break;

	case Subtract:
		flResult = flOldValue - flNewValue;
		fAction = true;
		break;

	case Multiply:
		flResult = flOldValue * flNewValue;
		fAction = true;
		break;

	case Divide:
		flResult = flOldValue / flNewValue;
		fAction = true;
		break;

	case Sin:
		flResult = sin(flNewValue);
		fAction = true;
		break;

	case Cos:
		flResult = cos(flNewValue);
		fAction = true;
		break;

	case Tg:
		flResult = tan(flNewValue);
		fAction = true;
		break;

	case Ctg:
		flResult = (cos(flNewValue) / sin(flNewValue));
		fAction = true;
		break;

	case Greater:
		flResult = flOldValue;
		fAction = (flNewValue > flOldValue);
		break;

	case GreaterOrEqual:
		flResult = flOldValue;
		fAction = (flNewValue >= flOldValue);
		break;

	case Less:
		flResult = flOldValue;
		fAction = (flNewValue <= flOldValue);
		break;

	case LessOrEqual:
		flResult = flOldValue;
		fAction = (flNewValue <= flOldValue);
		break;
	}

	// 3. If there's an action after the operation, fire it
	if (fAction)
	{
		FireTargets(STRING(m_iszTargetEntityForFire), pActivator, pCaller, triggerType, value);
	}

	if (FBitSet(pev->spawnflags, SF_VAL_ORAD))
		UTIL_Deg2Rad(flResult);

	if (FBitSet(pev->spawnflags, SF_VAL_ODEG))
		UTIL_Rad2Deg(flResult);

	// 4. Get result string from the float, and set the keyvalue for the target entity
	char szResult[64];
	sprintf(szResult, "%f", flResult);

	pTarget->SetKeyValue((char*)STRING(m_iszTargetKey), szResult);
}

void CTriggerValueOperator::UseString(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) 
{
	// 1. Find entity and load data
	CBaseEntity *pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_iszTargetEntity));
	
	if ( pTarget == nullptr )
	{
		ALERT( at_error, "Target %s doesn't exist (%s %s)", STRING( m_iszTargetEntity ), STRING( pev->targetname ), STRING( pev->classname ) );
		return;
	}

	char szOldValue[64];
	pTarget->GetKeyValue(*szOldValue, (char*)STRING(m_iszTargetKey));

	std::string strOldValue, strNewValue, strResult;
	bool fAction = false;

	strOldValue = szOldValue;
	strNewValue = STRING(m_iszNewValue);

	// 2. Check op mode and do the operations
	switch (iOperatingMode)
	{
	default:
		ALERT(at_console, "\nInvalid operating mode for %s, using Replace", STRING(pev->targetname));

	case Replace: 
		strResult = strNewValue;
		fAction = true;
		break;

	case Add: 
		strResult = strOldValue + strNewValue;
		fAction = true;
		break;
	}

	// 3. If there's an action after the operation, fire it
	if (fAction)
	{
		FireTargets(STRING(m_iszTargetEntityForFire), pActivator, pCaller, triggerType, value);
	}

	// 4. Get result string, and set the keyvalue
	char szResult[64];
	sprintf(szResult, "%s", strResult.c_str());

	pTarget->SetKeyValue((char*)STRING(m_iszTargetKey), szResult);
}

void CTriggerValueOperator::UseVector(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// 1. Find entity and load data
	CBaseEntity *pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_iszTargetEntity));

	if ( pTarget == nullptr )
	{
		ALERT( at_error, "Target %s doesn't exist (%s %s)", STRING( m_iszTargetEntity ), STRING( pev->targetname ), STRING( pev->classname ) );
		return;
	}

	char szOldValue[64];
	pTarget->GetKeyValue(*szOldValue, (char*)STRING(m_iszTargetKey));

	Vector vecOld, vecNew, vecResult;
	bool fAction = false;

	sscanf(szOldValue, "%f %f %f",
		&vecOld.x,
		&vecOld.y,
		&vecOld.z); // sorry for the mess

	sscanf(STRING(m_iszNewValue), "%f %f %f",
		&vecNew.x,
		&vecNew.y,
		&vecNew.z);

	ALERT(at_console, "\nNew vector: %f %f %f",
		vecNew.x,
		vecNew.y,
		vecNew.z);

	if (FBitSet(pev->spawnflags, SF_VAL_RAD))
	{
		UTIL_Deg2Rad(vecOld.x);
		UTIL_Deg2Rad(vecOld.y);
		UTIL_Deg2Rad(vecOld.z);
		UTIL_Deg2Rad(vecNew.x);
		UTIL_Deg2Rad(vecNew.y);
		UTIL_Deg2Rad(vecNew.z);
	}

	if (FBitSet(pev->spawnflags, SF_VAL_DEG))
	{
		UTIL_Rad2Deg(vecOld.x);
		UTIL_Rad2Deg(vecOld.y);
		UTIL_Rad2Deg(vecOld.z);
		UTIL_Rad2Deg(vecNew.x);
		UTIL_Rad2Deg(vecNew.y);
		UTIL_Rad2Deg(vecNew.z);
	}

	// 2. Check op mode and do the operations
	switch (iOperatingMode)
	{
	case Replace:
		vecResult = vecNew;
		fAction = true;
		break;

	case Add:
		vecResult = vecOld + vecNew;
		fAction = true;
		break;

	case Subtract:
		vecResult = vecOld - vecNew;
		fAction = true;
		break;

	case Divide:
		vecResult = vecOld / vecNew.x; // only pitch is taken into account, we can't multiply and divide vectors just like that
		fAction = true; 
		break;

	case Multiply:
		vecResult = vecOld * vecNew.x;
		fAction = true;
		break;

	case Greater:
		vecResult = vecOld;
		fAction = (vecOld.Length() < vecNew.Length()); // can only compare their lengths, it'd be kinda uneasy to do them component by component xd
		break;

	case GreaterOrEqual:
		vecResult = vecOld;
		fAction = (vecOld.Length() <= vecNew.Length());
		break;

	case Less:
		vecResult = vecOld;
		fAction = (vecOld.Length() > vecNew.Length());
		break;

	case LessOrEqual:
		vecResult = vecOld;
		fAction = (vecOld.Length() >= vecNew.Length());
		break;
	}

	// 3. If there's an action after the operation, fire it 
	if (fAction)
	{
		FireTargets(STRING(m_iszTargetEntityForFire), pActivator, pCaller, triggerType, value);
	}

	if (FBitSet(pev->spawnflags, SF_VAL_ORAD))
	{
		UTIL_Deg2Rad(vecResult.x);
		UTIL_Deg2Rad(vecResult.y);
		UTIL_Deg2Rad(vecResult.z);
	}

	if (FBitSet(pev->spawnflags, SF_VAL_ODEG))
	{	
		UTIL_Rad2Deg(vecResult.x);
		UTIL_Rad2Deg(vecResult.y);
		UTIL_Rad2Deg(vecResult.z);
	}

	// 4. Get result string, and set the keyvalue
	char szResult[64];
	sprintf(szResult, "%f %f %f",
		vecResult.x,
		vecResult.y,
		vecResult.z);

	pTarget->SetKeyValue((char*)STRING(m_iszTargetKey), szResult);
}

//-----------------------
//-----trigger_date------
//-----------------------
/*
	Triggers something if a specific date is met.
	E.g. every Christmas, you get some gifts in the map, or whatever.
*/

class CTriggerDate : public CBaseEntity
{
public:
	void				Spawn(void);

	void				KeyValue(KeyValueData *pkvd);
	void				GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64]);
	void				SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64]);

	void				Use(CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value);

private:
	int					m_iYear;
	int					m_iMonth;
	int					m_iDay;
};

LINK_ENTITY_TO_CLASS(trigger_date, CTriggerDate);

void CTriggerDate::Spawn(void) {} // Nothing uwu

void CTriggerDate::KeyValue(KeyValueData *pkvd)
{
	if (KeyvalueToken(year))
	{
		KeyvalueToInt(m_iYear);
	}

	else if (KeyvalueToken(month))
	{
		KeyvalueToInt(m_iMonth);
	}

	else if (KeyvalueToken(day))
	{
		KeyvalueToInt(m_iDay);
	}

	else
		KeyvaluesFromBase(CBaseEntity);
}

void CTriggerDate::GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "year"))
	{
		sprintf(&szTheValue, "%d", m_iYear);
	}

	else if (FStrEq(szTargetKeyName, "month"))
	{
		sprintf(&szTheValue, "%d", m_iMonth);
	}

	else if (FStrEq(szTargetKeyName, "day"))
	{
		sprintf(&szTheValue, "%d", m_iDay);
	}

	else
	{
		sprintf(&szTheValue, "null");
	}
}

void CTriggerDate::SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "year"))
		m_iYear = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "month"))
		m_iMonth = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "day"))
		m_iDay = atoi(szTheValue);

	else 
		ALERT(at_error, "No matching keyvlaue %s for %s", szTargetKeyName, STRING(pev->targetname));
}

void CTriggerDate::Use(CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value)
{
	int iYear, iMonth, iDay;
	getDate(iYear, iMonth, iDay);

	if (isDate(m_iYear, m_iMonth, m_iDay))
	{
		SUB_UseTargets(pActivator, useType, value);
	}
}

//-----------------------
//-----trigger_date2-----
//-----------------------
/*
	Triggers something if a specific time of day is met.
	3 AM horrors!!!
*/
class CTriggerDate2 : public CBaseEntity
{
public:
	void				Spawn(void);

	void				KeyValue(KeyValueData *pkvd);
	void				GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64]);
	void				SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64]);

	void				Use(CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value);

private:
	int					m_iHour;
	int					m_iMinute;
	int					m_iSecond;
};

LINK_ENTITY_TO_CLASS(trigger_date2, CTriggerDate2);

void CTriggerDate2::Spawn(void) {} // Nothing uwu

void CTriggerDate2::KeyValue(KeyValueData *pkvd)
{
	if (KeyvalueToken(hour))
	{
		KeyvalueToInt(m_iHour);
	}

	else if (KeyvalueToken(minute))
	{
		KeyvalueToInt(m_iMinute);
	}

	else if (KeyvalueToken(second))
	{
		KeyvalueToInt(m_iSecond);
	}

	else
		KeyvaluesFromBase(CBaseEntity);
}

void CTriggerDate2::GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "hour"))
	{
		sprintf(&szTheValue, "%d", m_iHour);
	}

	else if (FStrEq(szTargetKeyName, "minute"))
	{
		sprintf(&szTheValue, "%d", m_iMinute);
	}

	else if (FStrEq(szTargetKeyName, "second"))
	{
		sprintf(&szTheValue, "%d", m_iSecond);
	}

	else
	{
		sprintf(&szTheValue, "null");
	}
}

void CTriggerDate2::SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "hour"))
		m_iHour = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "minute"))
		m_iMinute = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "second"))
		m_iSecond = atoi(szTheValue);

	else
		ALERT(at_error, "No matching keyvalue %s for %s", szTargetKeyName, STRING(pev->targetname));
}

void CTriggerDate2::Use(CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value)
{
	int iHour, iMinute, iSecond;
	getTime(iHour, iMinute, iSecond);

	if (isTime(m_iHour, m_iMinute, m_iSecond))
	{
		SUB_UseTargets(pActivator, useType, value);
	}
}

//----------------------
//--trigger_difficulty--
//----------------------
/*
	Triggers something only if the difficulty is easy, or medium, or hard. Or NOT one of those.
	So you got 6 possible combinations, basically.
	NOT Hard, for example, is the same as having an Easy and Medium linked together
*/

class CTriggerDifficulty : public CBaseEntity
{
public:
	void				Spawn(void);

	void				KeyValue(KeyValueData *pkvd);
	void				GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64]);
	void				SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64]);

	void				Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

private:
	int					m_iSkill;
	int					m_iszKillTarget;
};

LINK_ENTITY_TO_CLASS(trigger_difficulty, CTriggerDifficulty);

void CTriggerDifficulty::Spawn(void)
{
	// Nothing
}

void CTriggerDifficulty::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "skill"))
	{
		m_iSkill = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else if (FStrEq(pkvd->szKeyName, "killtarget"))
	{
		m_iszKillTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else
		CBaseEntity::KeyValue(pkvd);
}

void CTriggerDifficulty::GetKeyValueCustom(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "skill"))
	{
		sprintf(&szTheValue, "%d", m_iSkill);
	}

	else if (FStrEq(szTargetKeyName, "killtarget"))
	{
		sprintf(&szTheValue, "%s", STRING(m_iszKillTarget));
	}

	else
	{
		sprintf(&szTheValue, "null");
		return;
	}
}

void CTriggerDifficulty::SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "skill"))
	{
		m_iSkill = atoi(szTheValue);
	}

	else if (FStrEq(szTargetKeyName, "killtarget"))
	{
		m_iszKillTarget = ALLOC_STRING(szTargetKeyName);
	}

	else
	{
		ALERT(at_error, "No matching keyvalue %s for %s", szTargetKeyName, STRING(pev->targetname));
	}
}

void CTriggerDifficulty::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (m_iSkill < 0 && m_iSkill > 6)
	{
		ALERT( at_console, "\ntrigger_difficulty, out of range skill level %i!", m_iSkill );
		return;
	}

	if (m_iSkill < 4)
	{
		if (m_iSkill == g_iSkillLevel)
		{
			SUB_UseTargets(this, useType, value);
			SUB_KillTargets(m_iszKillTarget);
		}
	}

	else
	{
		m_iSkill -= 3; // Gotta make it comparable to easy, medium and hard

		if (m_iSkill != g_iSkillLevel)
		{
			SUB_UseTargets(this, useType, value);
			SUB_KillTargets(m_iszKillTarget);
		}

		m_iSkill += 3;
	}
}

#define SF_TIMER_STARTON 1 << 0

//-----------------------
//-----trigger_timer-----
//-----------------------
/*
	Triggers something every X seconds.
	Or every once in a while. :)
*/

class CTriggerTimer : public CBaseEntity
{
public:
	void				Spawn();
	void				KeyValue(KeyValueData *pkvd);
//	int ObjectCaps() override { return FCAP_ACROSS_TRANSITION };

	void EXPORT			UseStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT			UseStop(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	void EXPORT			TimerThink();

private:
	CBaseEntity*		m_pActivator;
	USE_TYPE			m_useType;
	float				m_value;
	float				m_flDelay;
	float				m_flDelay2;
};

LINK_ENTITY_TO_CLASS(trigger_timer, CTriggerTimer);

void CTriggerTimer::Spawn()
{
	SetThink(NULL);
	SetUse(NULL);

	if (FBitSet(pev->spawnflags, SF_TIMER_STARTON))
	{
		SetUse(&CTriggerTimer::UseStop);
		SetThink(&CTriggerTimer::TimerThink);

		m_pActivator = this;
		m_useType = USE_SET;
		m_value = 0;
	}
	else
	{
		SetUse(&CTriggerTimer::UseStart);
		SetThink(NULL);
	}

	if (m_flDelay2 <= m_flDelay)
		pev->nextthink = gpGlobals->time + m_flDelay;
	else
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(m_flDelay, m_flDelay2);
}

void CTriggerTimer::KeyValue(KeyValueData *pkvd)
{
	if (KeyvalueToken(delay))
	{
		KeyvalueToFloat(m_flDelay);
	}

	else if (KeyvalueToken(delay2))
	{
		KeyvalueToFloat(m_flDelay2);
	}

	else
		KeyvaluesFromBase(CBaseEntity);
}

void EXPORT CTriggerTimer::UseStart(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	ALERT(at_console, "\nCTriggerTimer::UseStart");

	m_pActivator = pActivator;
	m_useType = useType;
	m_value = value;

	SetUse(&CTriggerTimer::UseStop);
	SetThink(&CTriggerTimer::TimerThink);

	if (m_flDelay2 <= m_flDelay)
		pev->nextthink = gpGlobals->time + m_flDelay;
	else
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(m_flDelay, m_flDelay2);
}

void EXPORT CTriggerTimer::UseStop(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	ALERT(at_console, "\nCTriggerTimer::UseStop");

	m_pActivator = pActivator;
	m_useType = useType;
	m_value = value;

	SetUse(&CTriggerTimer::UseStart);
	SetThink(NULL);
}

void EXPORT CTriggerTimer::TimerThink(void)
{
	SUB_UseTargets(m_pActivator, m_useType, m_value);

	ALERT(at_console, "\nCTriggerTimer::TimerThink");

	if (m_flDelay2 <= m_flDelay)
		pev->nextthink = gpGlobals->time + m_flDelay;
	else
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(m_flDelay, m_flDelay2);
}

// ========================================================================================================= //
//		UTIL_ ENTITIIES
// ========================================================================================================= //

//-----------------------
//--util_consoleprinter--
//-----------------------
/*
	This entity has one job to do.
	You give it a message, and it'll
	print it to the console. :3

	It also supports the message prefixes,
	like ERROR:, WARNING:, NOTE:, logging
	to the server console and whatnot. :)
*/
class CUtilConsolePrinter : public CBaseEntity
{
public:
	void				KeyValue( KeyValueData* pkvd );
	void				GetKeyValueCustom( char& szTheValue, char szTargetKeyName[64] );
	void				SetKeyValueCustom( char szTargetKeyName[64], char szTheValue[64] );
	void				Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );

private:
	string_t			m_strMessage;
	int					m_iPrefix;
	ALERT_TYPE			m_alertPrefix;
};

LINK_ENTITY_TO_CLASS( util_consoleprinter, CUtilConsolePrinter );

void CUtilConsolePrinter::KeyValue( KeyValueData* pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_strMessage" ) )
	{
		m_strMessage = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "m_iPrefix" ) )
	{
		m_iPrefix = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else
	{
		CBaseEntity::KeyValue( pkvd );
	}
}

void CUtilConsolePrinter::GetKeyValueCustom( char& szTheValue, char szTargetKeyName[64] )
{
	if ( FStrEq( szTargetKeyName, "m_strMessage" ) )
		printf( &szTheValue, "%s", STRING( m_strMessage ) );

	else if ( FStrEq( szTargetKeyName, "m_iPrefix" ) )
		printf( &szTheValue, "%i", m_iPrefix );

	else
		printf( &szTheValue, "%s", "null (no matching KV)" );
}

void CUtilConsolePrinter::SetKeyValueCustom( char szTargetKeyName[64], char szTheValue[64] )
{
	if ( FStrEq( szTargetKeyName, "m_strMessage" ) )
		m_strMessage = ALLOC_STRING( szTheValue );

	else if ( FStrEq( szTargetKeyName, "m_iPrefix" ) )
		m_iPrefix = atoi( szTheValue );

	else
		ALERT( at_error, "No matching KV %s in %s", szTargetKeyName, STRING( pev->targetname ) );
}

void CUtilConsolePrinter::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	switch (m_iPrefix)
	{
	case 0:
		m_alertPrefix = at_notice;
		break;

	case 1:
		m_alertPrefix = at_console;
		break;

	case 2:
		m_alertPrefix = at_aiconsole;
		break;

	case 3:
		m_alertPrefix = at_warning;
		break;

	case 4:
		m_alertPrefix = at_error;
		break;

	case 5:
		m_alertPrefix = at_logged;
		break;

	default:
		m_alertPrefix = at_error;
		break;
	}

	ALERT( m_alertPrefix, (char*)STRING( m_strMessage ) );
}

#define Deg2Rad (M_PI / 360) // A lil' bit ugly, but will work :) Trigonometric functions here apparently take radians only, so :p

//--------------------------
//-------util_rotator-------
//--------------------------
/*
	This entity has two origins. The first origin is the target's
	central origin. That's what it'll primarily rotate around. The
	secondary origin will usually be outside of the target's
	bounding box. Imagine this as Earth and the Sun. The Earth
	rotates around its primary origin point, but also around
	the Sun.

	It has multiple operation modes:

	0 - rotate once, set angles
	1 - rotate once, add angles
	2 - rotate every x seconds, set angles
	3 - rotate every x seconds, add angles
	4 - rotate every x seconds, set angles
	5 - rotate every x seconds, add angles

	6-11 follow the same style, except both origins are taken
	into consideration. Could've used spawnflags TBH,
*/
class CUtilRotator : public CBaseToggle
{
public:
	void				KeyValue(KeyValueData* pkvd);
	void				GetKeyValueCustom(char& szTheValue, char szTargetKeyName[64]);
	void				SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64]);
	void				Spawn(void);

	int					ObjectCaps() override { return FCAP_ACROSS_TRANSITION; }

	void EXPORT			RotateThink( void );
	void EXPORT			RotateStart( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );
	void EXPORT			RotateUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );
	void EXPORT			RotateStop( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );

	void				RotateApply(CBaseEntity* pTarget); // Just to discern between the actual target member and this function declaration
	float				DistToOrigin(void);

	// How it works:
	// 
	// RotateThink:
	// RotateApply:
	//       x = RadiusPriSec * cos(angle.pitch) * cos(angle.yaw) + PrimaryOrigin.x;
	//       y = RadiusPriSec * cos(angle.pitch) * sin(angle.yaw) + PrimaryOrigin.y;
	//       z = RadiusPriSec * sin(angle.pitch) + PrimaryOrigin.z;

private:
	Vector				m_vecPrimaryOrigin;
	Vector				m_vecSecondaryOrigin;
	Vector				m_vecOriginalOrigin;

	float				m_flDelay;
	int					m_iOperationMode;

	Vector				m_vecPrimaryAngles;
	Vector				m_vecPrimaryAnglePhase = { 0, 0, 0 };
	Vector				m_vecSecondaryAngles;
	Vector				m_vecSecondaryAnglePhase = { 0, 0, 0 };
	Vector				m_vecOriginalAngles;

	string_t			m_iszPrimaryTarget;
	string_t			m_iszSecondaryTarget;

	CBaseEntity*		pPrimaryTarget;		// PrimaryTarget is actually an entity whose origin will be taken.
	CBaseEntity*		pSecondaryTarget;	// Look at PrimaryTarget.
	CBaseEntity*		pTarget;			// Now this is the entity which will get affected by the rotations. Hope that clears up the confusion.

	char				szPrimaryOrigin[64];
	char				szSecondaryOrigin[64];
	char				szPrimaryAngles[64];
	char				szSecondaryAngles[64];

	qboolean			m_fIsUsingPredefRadiusXY;
	qboolean			m_fIsUsingPredefRadiusZ;

	float				m_flRadiusXY;
	float				m_flRadiusZ;
};

LINK_ENTITY_TO_CLASS(util_rotator, CUtilRotator);

#define SF_PR_ROTATING	1
#define SF_SC_ROTATING	2
#define SF_START_ON		4

void CUtilRotator::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "pr_origin" ) )
	{
		sscanf( pkvd->szValue, "%f %f %f\n",
			&m_vecPrimaryOrigin.x,
			&m_vecPrimaryOrigin.y,
			&m_vecPrimaryOrigin.z ); // I apologise if this style really bothers you. :p

		sprintf( szPrimaryOrigin, "%f %f %f",
			m_vecPrimaryOrigin.x,
			m_vecPrimaryOrigin.y,
			m_vecPrimaryOrigin.z );

		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "sc_origin" ) )
	{
		sscanf( pkvd->szValue, "%f %f %f\n",
			&m_vecSecondaryOrigin.x,
			&m_vecSecondaryOrigin.y,
			&m_vecSecondaryOrigin.z );

		sprintf( szSecondaryOrigin, "%f %f %f",
			m_vecSecondaryOrigin.x,
			m_vecSecondaryOrigin.y,
			m_vecSecondaryOrigin.z );

		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "pr_angles" ) )
	{
		sscanf( pkvd->szValue, "%f %f %f\n",
			&m_vecPrimaryAngles.x,
			&m_vecPrimaryAngles.y,
			&m_vecPrimaryAngles.z );

		sprintf( szPrimaryAngles, "%f %f %f",
			m_vecPrimaryAngles.x,
			m_vecPrimaryAngles.y,
			m_vecPrimaryAngles.z );

		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "sc_angles" ) )
	{
		sscanf( pkvd->szValue, "%f %f %f\n",
			&m_vecSecondaryAngles.x,
			&m_vecSecondaryAngles.y,
			&m_vecSecondaryAngles.z );

		sprintf( szSecondaryAngles, "%f %f %f",
			m_vecSecondaryAngles.x,
			m_vecSecondaryAngles.y,
			m_vecSecondaryAngles.z );

		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "pr_target" ) )
	{
		m_iszPrimaryTarget = ALLOC_STRING( pkvd->szValue );
	}

	else if ( FStrEq( pkvd->szKeyName, "sc_target" ) )
	{
		m_iszSecondaryTarget = ALLOC_STRING( pkvd->szValue );
	}

	else if ( FStrEq( pkvd->szKeyName, "delay" ) )
	{
		m_flDelay = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "opmode" ) )
	{
		m_iOperationMode = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "radius1" ) )
	{
		if ( FStrEq( pkvd->szValue, "0" ) )
		{
			m_fIsUsingPredefRadiusXY = FALSE;
			ALERT( at_console, "\nPredefRadiusXY FALSE" );
		}

		else
		{
			m_flRadiusXY = atof( pkvd->szValue );
			m_fIsUsingPredefRadiusXY = TRUE;
			ALERT( at_console, "\nPredefRadiusXY TRUE" );
		}
		pkvd->fHandled = TRUE;
	}

	else if ( FStrEq( pkvd->szKeyName, "radius2" ) )
	{
		if ( FStrEq( pkvd->szValue, "0" ) )
		{
			m_fIsUsingPredefRadiusZ = FALSE;
		}

		else
		{
			m_flRadiusZ = atof( pkvd->szValue );
			m_fIsUsingPredefRadiusZ = TRUE;
		}
		pkvd->fHandled = TRUE;
	}

	else
		CBaseEntity::KeyValue( pkvd );
}

void CUtilRotator::GetKeyValueCustom( char& szTheValue, char szTargetKeyName[64] )
{
	if ( FStrEq( szTargetKeyName, "pr_origin" ) )
		sprintf( &szTheValue, "%f %f %f", m_vecPrimaryOrigin.x, m_vecPrimaryOrigin.y, m_vecPrimaryOrigin.z );

	else if ( FStrEq( szTargetKeyName, "sc_origin" ) )
		sprintf( &szTheValue, "%f %f %f", m_vecSecondaryOrigin.x, m_vecSecondaryOrigin.y, m_vecSecondaryOrigin.z );

	else if ( FStrEq( szTargetKeyName, "pr_angles" ) )
		sprintf( &szTheValue, "%f %f %f", m_vecPrimaryAngles.x, m_vecPrimaryAngles.y, m_vecPrimaryAngles.z );

	else if ( FStrEq( szTargetKeyName, "sc_angles" ) )
		sprintf( &szTheValue, "%f %f %f", m_vecSecondaryAngles.x, m_vecSecondaryAngles.y, m_vecSecondaryAngles.z );

	else if ( FStrEq( szTargetKeyName, "pr_target" ) )
		sprintf( &szTheValue, "%s", STRING( m_iszPrimaryTarget ) );

	else if ( FStrEq( szTargetKeyName, "sc_target" ) )
		sprintf( &szTheValue, "%s", STRING( m_iszSecondaryTarget ) );

	else
		printf( &szTheValue, "%s", "null (no matching KV)" );
}

void CUtilRotator::SetKeyValueCustom( char szTargetKeyName[64], char szTheValue[64] )
{
	if ( FStrEq( szTargetKeyName, "pr_origin" ) )
		sscanf( szTheValue, "%f %f %f", &m_vecPrimaryOrigin.x, &m_vecPrimaryOrigin.y, &m_vecPrimaryOrigin.z );

	else if ( FStrEq( szTargetKeyName, "sc_origin" ) )
		sscanf( szTheValue, "%f %f %f", &m_vecPrimaryAngles.x, &m_vecPrimaryAngles.y, &m_vecPrimaryAngles.z );

	else if ( FStrEq( szTargetKeyName, "pr_angles" ) )
		sscanf( szTheValue, "%f %f %f", &m_vecPrimaryAngles.x, &m_vecPrimaryAngles.y, &m_vecPrimaryAngles.z );

	else if ( FStrEq( szTargetKeyName, "sc_angles" ) )
		sscanf( szTheValue, "%f %f %f", &m_vecSecondaryAngles.x, &m_vecSecondaryAngles.y, &m_vecSecondaryAngles.z );

	else if ( FStrEq( szTargetKeyName, "pr_target" ) )
		m_iszPrimaryTarget = ALLOC_STRING( szTheValue );

	else if ( FStrEq( szTargetKeyName, "pr_target" ) )
		m_iszPrimaryTarget = ALLOC_STRING( szTheValue );

	else
		ALERT( at_error, "No matching KV %s in %s", szTargetKeyName, STRING( pev->targetname ) );
}

void CUtilRotator::Spawn( void )
{
	UTIL_SetOrigin( pev, pev->origin );

	SetThink( NULL );
	SetUse( NULL );

	switch ( m_iOperationMode )
	{
	case 0:	// Rotate once, set angles
		SetUse( &CUtilRotator::RotateUse );
		break;

	case 1:	// Rotate once, add angles
		SetUse( &CUtilRotator::RotateUse );
		break;

	case 2:	// Rotate every x seconds, set angles
		SetUse( &CUtilRotator::RotateStart );
		SetThink( &CUtilRotator::RotateThink );
		break;

	case 3:	// Rotate every x seconds, add angles
		SetUse( &CUtilRotator::RotateStart );
		SetThink( &CUtilRotator::RotateThink );
		break;

	case 4: // Rotate every x seconds, accelerate angles
		SetUse( &CUtilRotator::RotateStart );
		SetThink( &CUtilRotator::RotateThink );

	default:
		ALERT( at_error, "\nBoop, util_rotator (%s) has no operation mode, fix this please\n", STRING( pev->targetname ) );
		break;
	}
}

void EXPORT CUtilRotator::RotateThink( void )
{
	RotateApply( pTarget );

	pTarget->SUB_NullThink( m_flDelay );
	pev->nextthink = pev->ltime + m_flDelay;
}

void EXPORT CUtilRotator::RotateStart( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	pTarget = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ) );

	if ( pTarget )
	{
		ALERT( at_aiconsole, "\nutil_rotator, entity found!\n" );
	}

	m_vecOriginalOrigin = pTarget->pev->origin;
	m_vecOriginalAngles = pTarget->pev->angles;

	if ( m_iszPrimaryTarget )
	{
		pPrimaryTarget = UTIL_FindEntityByTargetname( NULL, STRING( m_iszPrimaryTarget ) );

		Vector vecDistanceP = pev->origin - pPrimaryTarget->pev->origin;
		m_vecPrimaryAngles = UTIL_VecToAngles( vecDistanceP );
	}

	if ( m_vecPrimaryAngles )
	{
		m_vecPrimaryAnglePhase = m_vecPrimaryAngles;
	}

	if ( m_iszSecondaryTarget ) // Will finish this later on
	{
		pSecondaryTarget = UTIL_FindEntityByTargetname( NULL, STRING( m_iszSecondaryTarget ) );

		Vector vecDistanceS = pev->origin - pSecondaryTarget->pev->origin;
		m_vecSecondaryAngles = UTIL_VecToAngles( vecDistanceS );
		m_flRadiusXY = m_flRadiusZ = vecDistanceS.Length();
	}

	if ( m_vecSecondaryAngles )
	{
		m_vecSecondaryAnglePhase = m_vecSecondaryAngles;
	}

	SetThink( &CUtilRotator::RotateThink );
	SetUse( &CUtilRotator::RotateStop );

	pTarget->SUB_NullThink( 0.001 );
	pev->nextthink = pev->ltime + m_flDelay;
}

void EXPORT CUtilRotator::RotateUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	if ( m_iszPrimaryTarget )
		pPrimaryTarget = UTIL_FindEntityByTargetname( NULL, STRING( m_iszPrimaryTarget ) );

	if ( m_iszSecondaryTarget )
		pSecondaryTarget = UTIL_FindEntityByTargetname( NULL, STRING( m_iszSecondaryTarget ) );

	pTarget = UTIL_FindEntityByTargetname( NULL, STRING( pev->target ) );

	if ( pTarget )
	{
		ALERT( at_aiconsole, "\nutil_rotator, entity found!\n" );
	}

	m_vecOriginalOrigin = pTarget->pev->origin;
	m_vecOriginalAngles = pTarget->pev->angles;

	if ( m_vecPrimaryAngles )
		m_vecPrimaryAnglePhase = m_vecPrimaryAngles;

	if ( m_vecSecondaryAngles )
		m_vecSecondaryAnglePhase = m_vecSecondaryAngles;

	RotateApply( pTarget );
}

void EXPORT CUtilRotator::RotateStop(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SetThink(NULL);
	SetUse(&CUtilRotator::RotateStart);
}

void CUtilRotator::RotateApply(CBaseEntity* pTarget)
{
	float flRadius = DistToOrigin();
	Vector vecNewOrigin;

	if (pev->spawnflags & SF_PR_ROTATING)
	{
		switch (m_iOperationMode)
		{
		case 0:
		case 2:

			if (pTarget->pev->angles != m_vecPrimaryAngles)
				pTarget->pev->angles = m_vecPrimaryAngles;

			break;

		case 1:
			pTarget->pev->angles = pTarget->pev->angles + m_vecPrimaryAngles;
			break;

		case 3:

			if (pTarget->pev->avelocity != m_vecPrimaryAngles)
				pTarget->pev->avelocity = m_vecPrimaryAngles;
			
			break;

		case 4:
			pTarget->pev->avelocity = pTarget->pev->avelocity + m_vecPrimaryAngles;
			break;

		default:
			ALERT(at_error, "\nOperation mode undefined for %s", STRING(pev->targetname));
			break;
		}
	}

	if (pev->spawnflags & SF_SC_ROTATING)
	{
		switch (m_iOperationMode)
		{
		case 0:
		case 2:

			if (m_fIsUsingPredefRadiusXY == TRUE)
			{
				vecNewOrigin.x = m_flRadiusXY * cos(m_vecSecondaryAngles.x * Deg2Rad) * cos(m_vecSecondaryAngles.y * Deg2Rad) + m_vecPrimaryOrigin.x;
				vecNewOrigin.y = m_flRadiusXY * cos(m_vecSecondaryAngles.x * Deg2Rad) * sin(m_vecSecondaryAngles.y * Deg2Rad) + m_vecPrimaryOrigin.y;
			}
			else
			{
				vecNewOrigin.x = DistToOrigin() * cos(m_vecSecondaryAngles.x * Deg2Rad) * cos(m_vecSecondaryAngles.y * Deg2Rad) + m_vecPrimaryOrigin.x;
				vecNewOrigin.y = DistToOrigin() * cos(m_vecSecondaryAngles.x * Deg2Rad) * sin(m_vecSecondaryAngles.y * Deg2Rad) + m_vecPrimaryOrigin.y;
			}

			if (m_fIsUsingPredefRadiusZ == TRUE)
				vecNewOrigin.z = m_flRadiusZ * sin(m_vecSecondaryAngles.x) + m_vecPrimaryOrigin.z; // SPHERICAL COORDINATES FTW WOOOOOOOO
			else
				vecNewOrigin.z = DistToOrigin() * sin(m_vecSecondaryAngles.x) + m_vecPrimaryOrigin.z;
			break;

		case 1:
		case 3:
			if (m_fIsUsingPredefRadiusXY == TRUE)
			{
				vecNewOrigin.x = m_flRadiusXY * cos(m_vecSecondaryAnglePhase.x * Deg2Rad) * cos(m_vecSecondaryAnglePhase.y * Deg2Rad) + m_vecPrimaryOrigin.x;
				vecNewOrigin.y = m_flRadiusXY * cos(m_vecSecondaryAnglePhase.x * Deg2Rad) * sin(m_vecSecondaryAnglePhase.y * Deg2Rad) + m_vecPrimaryOrigin.y;
			}
			else
			{
				vecNewOrigin.x = DistToOrigin() * cos(m_vecSecondaryAnglePhase.x * Deg2Rad) * cos(m_vecSecondaryAnglePhase.y * Deg2Rad) + m_vecPrimaryOrigin.x;
				vecNewOrigin.y = DistToOrigin() * cos(m_vecSecondaryAnglePhase.x * Deg2Rad) * sin(m_vecSecondaryAnglePhase.y * Deg2Rad) + m_vecPrimaryOrigin.y;
			}

			if (m_fIsUsingPredefRadiusZ == TRUE)
				vecNewOrigin.z = m_flRadiusZ * sin(m_vecSecondaryAnglePhase.x * Deg2Rad) + m_vecPrimaryOrigin.z;
			else
				vecNewOrigin.z = DistToOrigin() * sin(m_vecSecondaryAnglePhase.x * Deg2Rad) + m_vecPrimaryOrigin.z;

			m_vecSecondaryAnglePhase = m_vecSecondaryAnglePhase + (m_vecSecondaryAngles / CVAR_GET_FLOAT("fps_max")); // Hell yeah, let's keep it all in sync :ok_hand:
			//	m_vecSecondaryAnglePhase.Normalize();
			if (m_vecSecondaryAnglePhase.x > 360)
				m_vecSecondaryAnglePhase.x = 0;

			if (m_vecSecondaryAnglePhase.y > 360)
				m_vecSecondaryAnglePhase.y = 0;

			if (m_vecSecondaryAnglePhase.z > 360)
				m_vecSecondaryAnglePhase.z = 0;
			break;

		default:
			ALERT(at_error, "\nOperation mode undefined for %s", STRING(pev->targetname));
			break;
		}

		UTIL_SetOrigin(pTarget->pev, vecNewOrigin);
	}
}

float CUtilRotator::DistToOrigin(void)
{
	Vector vecCalculator = m_vecPrimaryOrigin - m_vecSecondaryOrigin;

	return vecCalculator.Length();
}

#undef Deg2Rad

//--------------------------
//------util_rot_noorg------
//--------------------------
/*
	This entity is meant to rotate brush entities
	which don't have origin brushes, properly.
*/

class CUtilRotateNoOrigin : public CBaseEntity
{
public:
	void				Spawn(void);
	void				KeyValue(KeyValueData *pkvd);
	void				GetKeyValueCustom(char& szTheValue, char szTargetKeyName[64]);
	void				SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64]);
	
	void				Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void				Debug_DrawLine(Vector a, Vector b);

private:
	Vector				m_vecNewAngles = g_vecZero;
};

LINK_ENTITY_TO_CLASS(util_rot_noorg, CUtilRotateNoOrigin);

void CUtilRotateNoOrigin::Spawn(void)
{
	// nothin'
	SetUse(&CUtilRotateNoOrigin::Use);
}

void CUtilRotateNoOrigin::KeyValue(KeyValueData *pkvd)
{
	if		(KeyvalueToken(newPitch))	{ KeyvalueToFloat(m_vecNewAngles.x); }
	else if (KeyvalueToken(newYaw))		{ KeyvalueToFloat(m_vecNewAngles.y); }
	else if (KeyvalueToken(newRoll))	{ KeyvalueToFloat(m_vecNewAngles.z); }
	else	/*if no remaining KVs*/		{ KeyvaluesFromBase(CBaseEntity); }
}

void CUtilRotateNoOrigin::GetKeyValueCustom(char& szTheValue, char szTargetKeyName[64])
{
	if ( FStrEq( szTargetKeyName, "newPitch" ) )
		sprintf( &szTheValue, "%f", m_vecNewAngles.x );
	else if ( FStrEq( szTargetKeyName, "newYaw" ) )
		sprintf( &szTheValue, "%f", m_vecNewAngles.y );
	else if ( FStrEq( szTargetKeyName, "newRoll" ) )
		sprintf( &szTheValue, "%f", m_vecNewAngles.z );
	else
		sprintf( &szTheValue, "null" );
}

void CUtilRotateNoOrigin::SetKeyValueCustom(char szTargetKeyName[64], char szTheValue[64])
{
	if ( FStrEq( szTargetKeyName, "newPitch" ) )
		m_vecNewAngles.x = atof( szTheValue );
	else if ( FStrEq( szTargetKeyName, "newYaw" ) )
		m_vecNewAngles.y = atof( szTheValue );
	else if ( FStrEq( szTargetKeyName, "newRoll" ) )
		m_vecNewAngles.z = atof( szTheValue );
	else
		ALERT( at_error, "\nCould not find keyvalue %s for entity %s %s", szTargetKeyName, STRING( pev->classname ), STRING( pev->targetname ) );
}

void CUtilRotateNoOrigin::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity *pTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));

	static Vector vecBrushOrigin; // origin we get from the brush itself, not pev->origin
	Vector vecEndupOrigin; // position where our brush ends up, after rotating it around its pev->origin pivot
	Vector vecRealOrigin; // pev->origin origin
	Vector anglesNew; // new angles
	Vector anglesOld; // old angles
	Vector anglesFromOrigin; // angles from the origin to the brush
	Vector vecDelta; // difference between original BrushOrigin and the EndupOrigin
	Vector vecNewOrigin; // the final calculated pev->origin
	float flRadius; // radius from 0,0,0 to BrushOrigin
	static bool fSetBrushOrigin = false;

	// -1. set origin to 0 0 0 so this can work properly
	UTIL_SetOrigin(pTarget->pev, Vector(0, 0, 0));
	pTarget->pev->angles = Vector(0, 0, 0);;

	// 0. basic data - get angles and current position
	anglesNew = m_vecNewAngles;
	anglesOld = pTarget->pev->angles;

	vecRealOrigin = pTarget->pev->origin;
	
	if (!fSetBrushOrigin)
	{
		vecBrushOrigin = (pTarget->pev->absmin + pTarget->pev->absmax) / 2;
		fSetBrushOrigin = true;
	}
	
	vecBrushOrigin.z = 0; // TEMPORARY

	anglesFromOrigin.x = asin(vecBrushOrigin.z / vecBrushOrigin.Length());
	anglesFromOrigin.y = atan2(vecBrushOrigin.y, vecBrushOrigin.x);
	anglesFromOrigin.z = 0;

	UTIL_Rad2Deg(anglesFromOrigin.x);
	UTIL_Rad2Deg(anglesFromOrigin.y);

	// 1. get radius
	flRadius = vecBrushOrigin.Length();

	// 2. calculate where it ends
	vecEndupOrigin = vecRealOrigin + UTIL_VecPolarCoords(flRadius, anglesOld + anglesNew + anglesFromOrigin);

	// 3. calculate the difference between where it'll end and the current position
	vecDelta = vecEndupOrigin - vecBrushOrigin;

	// 4. subtract the difference from the actual pos
	vecNewOrigin = vecRealOrigin - vecDelta;

	// 5. set that stuff
	UTIL_SetOrigin(pTarget->pev, vecNewOrigin);
	pTarget->pev->angles = anglesNew;
}

// Quick and lazy attempt at making a SoHL "movewith"-like thing
// To-do: rewrite in June
class CUtilMoveWith : public CBaseEntity
{
public:
	void				Spawn( void );
	void				Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );
	void				Think( void );

	void				KeyValue( KeyValueData* pkvd );

private:
	string_t			m_iszTarget, m_iszParent;
	CBaseEntity*		m_pTarget;
	CBaseEntity*		m_pParent;

	bool				m_fMoveActive;
};

LINK_ENTITY_TO_CLASS( util_movewith, CUtilMoveWith );

void CUtilMoveWith::Spawn()
{
	m_fMoveActive = false;

	m_pTarget = NULL;
	m_pParent = NULL;

	pev->nextthink = gpGlobals->time + 1.5;
}

void CUtilMoveWith::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	m_pTarget = UTIL_FindEntityByTargetname( NULL, STRING( m_iszTarget ) );
	m_pParent = UTIL_FindEntityByTargetname( NULL, STRING( m_iszParent ) );

	m_fMoveActive = true;
}

void CUtilMoveWith::Think()
{
	if ( m_fMoveActive )
	{
		UTIL_SetOrigin( m_pTarget->pev, m_pParent->pev->origin );
		m_pTarget->pev->angles = m_pParent->pev->angles;
	}

	pev->nextthink = gpGlobals->time + 0.001;
}

void CUtilMoveWith::KeyValue( KeyValueData* pkvd )
{
	if ( KeyvalueToken( m_iszTarget ) )
	{
		KeyvalueToString( m_iszTarget );
	}
	else if ( KeyvalueToken( m_iszParent ) )
	{
		KeyvalueToString( m_iszParent );
	}
	else
	{
		KeyvaluesFromBase( CBaseEntity );
	}
}







