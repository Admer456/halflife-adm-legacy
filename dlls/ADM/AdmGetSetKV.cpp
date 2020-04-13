/*

======= adm_getsetkv.cpp ===========================

	Spaghetti for changing and setting keyvalues.
	This file serves just for the base classes.
	Individual entities implement their GetSetKV
	right alongside their other methods, e.g. 
	adm_triggers.cpp.

	As you can see, the downside of this approach
	is that every entity has to have this implemented
	separately.

	Obviously, it's not gonna work with multi_manager,
	but why would you want to do that, lol?
*/

#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

void CBaseDelay::GetKeyValueBase(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "delay"))
		sprintf(&szTheValue, "%f", m_flDelay);

	else if (FStrEq(szTargetKeyName, "killtarget"))
		sprintf(&szTheValue, "%s", STRING(m_iszKillTarget));

	else
		GetKeyValueCustom(szTheValue, szTargetKeyName);
}

void CBaseDelay::SetKeyValueBase(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "delay"))
		m_flDelay = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "killtarget"))
		m_iszKillTarget = ALLOC_STRING(szTheValue);

	else
		SetKeyValueCustom(szTargetKeyName, szTheValue);
}

void CBaseAnimating::GetKeyValueBase(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "delay"))
		sprintf(&szTheValue, "%f", m_flDelay);

	else if (FStrEq(szTargetKeyName, "killtarget"))
		sprintf(&szTheValue, "%s", STRING(m_iszKillTarget));

	else if (FStrEq(szTargetKeyName, "framerate"))
		sprintf(&szTheValue, "%f", m_flFrameRate);

	else if (FStrEq(szTargetKeyName, "groundspeed"))
		sprintf(&szTheValue, "%f", m_flGroundSpeed);

	else if (FStrEq(szTargetKeyName, "lastevent"))
		sprintf(&szTheValue, "%f", m_flLastEventCheck);

	else if (FStrEq(szTargetKeyName, "sequencefinished"))
		sprintf(&szTheValue, "%i", m_fSequenceFinished);

	else if (FStrEq(szTargetKeyName, "sequenceloops"))
		sprintf(&szTheValue, "%i", m_fSequenceLoops);

	else
		GetKeyValueCustom(szTheValue, szTargetKeyName);
}

void CBaseAnimating::SetKeyValueBase(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "delay"))
		m_flDelay = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "killtarget"))
		m_iszKillTarget = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "framerate"))
		m_flFrameRate = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "groundspeed"))
		m_flGroundSpeed = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "lastevent"))
		m_flLastEventCheck = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "sequencefinished"))
		m_fSequenceFinished = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "sequenceloops"))
		m_fSequenceLoops = atoi(szTheValue);

	else
		SetKeyValueCustom(szTargetKeyName, szTheValue);
}

void CBaseToggle::GetKeyValueBase(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "delay"))
		sprintf(&szTheValue, "%f", m_flDelay);

	else if (FStrEq(szTargetKeyName, "killtarget"))
		sprintf(&szTheValue, "%s", STRING(m_iszKillTarget));

	else if (FStrEq(szTargetKeyName, "framerate"))
		sprintf(&szTheValue, "%f", m_flFrameRate);

	else if (FStrEq(szTargetKeyName, "groundspeed"))
		sprintf(&szTheValue, "%f", m_flGroundSpeed);

	else if (FStrEq(szTargetKeyName, "lastevent"))
		sprintf(&szTheValue, "%f", m_flLastEventCheck);

	else if (FStrEq(szTargetKeyName, "sequencefinished"))
		sprintf(&szTheValue, "%i", m_fSequenceFinished);

	else if (FStrEq(szTargetKeyName, "sequenceloops"))
		sprintf(&szTheValue, "%i", m_fSequenceLoops);

	else if (FStrEq(szTargetKeyName, "activatefinished"))
		sprintf(&szTheValue, "%f", m_flActivateFinished);

	else if (FStrEq(szTargetKeyName, "wait"))
		sprintf(&szTheValue, "%f", m_flWait);

	else if (FStrEq(szTargetKeyName, "lip"))
		sprintf(&szTheValue, "%f", m_flLip);

	else if (FStrEq(szTargetKeyName, "m_flTWidth"))
		sprintf(&szTheValue, "%f", m_flTWidth);

	else if (FStrEq(szTargetKeyName, "m_flTLength"))
		sprintf(&szTheValue, "%f", m_flTLength);

	else if (FStrEq(szTargetKeyName, "m_vecPosition1"))
		sprintf(&szTheValue, "%f %f %f", m_vecPosition1.x, m_vecPosition1.y, m_vecPosition1.z);

	else if (FStrEq(szTargetKeyName, "m_vecPosition2"))
		sprintf(&szTheValue, "%f %f %f", m_vecPosition2.x, m_vecPosition2.y, m_vecPosition2.z);

	else if (FStrEq(szTargetKeyName, "m_vecAngle1"))
		sprintf(&szTheValue, "%f %f %f", m_vecAngle1.x, m_vecAngle1.y, m_vecAngle1.z);

	else if (FStrEq(szTargetKeyName, "m_vecAngle2"))
		sprintf(&szTheValue, "%f %f %f", m_vecAngle2.x, m_vecAngle2.y, m_vecAngle2.z);

	else if (FStrEq(szTargetKeyName, "m_cTriggersLeft"))
		sprintf(&szTheValue, "%i", m_cTriggersLeft);

	else if (FStrEq(szTargetKeyName, "m_flHeight"))
		sprintf(&szTheValue, "%f", m_flHeight);

	else if (FStrEq(szTargetKeyName, "m_vecFinalDest"))
		sprintf(&szTheValue, "%f %f %f", m_vecFinalDest.x, m_vecFinalDest.y, m_vecFinalDest.z);

	else if (FStrEq(szTargetKeyName, "m_vecFinalAngle"))
		sprintf(&szTheValue, "%f %f %f", m_vecFinalAngle.x, m_vecFinalAngle.y, m_vecFinalAngle.z);

	else if (FStrEq(szTargetKeyName, "m_bitsDamageInflict"))
		sprintf(&szTheValue, "%i", m_bitsDamageInflict);

	else if (FStrEq(szTargetKeyName, "master"))
		sprintf(&szTheValue, "%s", STRING(m_sMaster));

	else
		GetKeyValueCustom(szTheValue, szTargetKeyName);
}

void CBaseToggle::SetKeyValueBase(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "delay"))
		m_flDelay = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "killtarget"))
		m_iszKillTarget = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "framerate"))
		m_flFrameRate = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "groundspeed"))
		m_flGroundSpeed = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "lastevent"))
		m_flLastEventCheck = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "sequencefinished"))
		m_fSequenceFinished = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "activatefinished"))
		m_flActivateFinished = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "wait"))
		m_fSequenceLoops = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "lip"))
		m_fSequenceLoops = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_flTWidth"))
		m_flTWidth = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_flTLength"))
		m_flTLength = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_vecPosition1"))
		sscanf(szTheValue, "%f %f %f", &m_vecPosition1.x, &m_vecPosition1.y, &m_vecPosition1.z);

	else if (FStrEq(szTargetKeyName, "m_vecPosition2"))
		sscanf(szTheValue, "%f %f %f", &m_vecPosition2.x, &m_vecPosition2.y, &m_vecPosition2.z);

	else if (FStrEq(szTargetKeyName, "m_vecAngle1"))
		sscanf(szTheValue, "%f %f %f", &m_vecAngle1.x, &m_vecAngle1.y, &m_vecAngle1.z);

	else if (FStrEq(szTargetKeyName, "m_vecAngle2"))
		sscanf(szTheValue, "%f %f %f", &m_vecAngle2.x, &m_vecAngle2.y, &m_vecAngle2.z);

	else if (FStrEq(szTargetKeyName, "m_cTriggersLeft"))
		m_cTriggersLeft = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_flHeight"))
		m_flHeight = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_cTriggersLeft"))
		m_cTriggersLeft = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_vecFinalDest"))
		sscanf(szTheValue, "%f %f %f", &m_vecFinalDest.x, &m_vecFinalDest.y, &m_vecFinalDest.z);

	else if (FStrEq(szTargetKeyName, "m_vecFinalAngle"))
		sscanf(szTheValue, "%f %f %f", &m_vecFinalAngle.x, &m_vecFinalAngle.y, &m_vecFinalAngle.z);

	else if (FStrEq(szTargetKeyName, "m_bitsDamageInflict"))
		m_bitsDamageInflict = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "master"))
		m_sMaster = ALLOC_STRING(szTheValue);

	else
		SetKeyValueCustom(szTargetKeyName, szTheValue);
}

void CBaseButton::GetKeyValueBase(char &szTheValue, char szTargetKeyName[64])
{
	if (FStrEq(szTargetKeyName, "delay"))
		sprintf(&szTheValue, "%f", m_flDelay);

	else if (FStrEq(szTargetKeyName, "killtarget"))
		sprintf(&szTheValue, "%s", STRING(m_iszKillTarget));

	else if (FStrEq(szTargetKeyName, "framerate"))
		sprintf(&szTheValue, "%f", m_flFrameRate);

	else if (FStrEq(szTargetKeyName, "groundspeed"))
		sprintf(&szTheValue, "%f", m_flGroundSpeed);

	else if (FStrEq(szTargetKeyName, "lastevent"))
		sprintf(&szTheValue, "%f", m_flLastEventCheck);

	else if (FStrEq(szTargetKeyName, "sequencefinished"))
		sprintf(&szTheValue, "%i", m_fSequenceFinished);

	else if (FStrEq(szTargetKeyName, "sequenceloops"))
		sprintf(&szTheValue, "%i", m_fSequenceLoops);

	else if (FStrEq(szTargetKeyName, "activatefinished"))
		sprintf(&szTheValue, "%f", m_flActivateFinished);

	else if (FStrEq(szTargetKeyName, "wait"))
		sprintf(&szTheValue, "%f", m_flWait);

	else if (FStrEq(szTargetKeyName, "lip"))
		sprintf(&szTheValue, "%f", m_flLip);

	else if (FStrEq(szTargetKeyName, "m_flTWidth"))
		sprintf(&szTheValue, "%f", m_flTWidth);

	else if (FStrEq(szTargetKeyName, "m_flTLength"))
		sprintf(&szTheValue, "%f", m_flTLength);

	else if (FStrEq(szTargetKeyName, "m_vecPosition1"))
		sprintf(&szTheValue, "%f %f %f", m_vecPosition1.x, m_vecPosition1.y, m_vecPosition1.z);

	else if (FStrEq(szTargetKeyName, "m_vecPosition2"))
		sprintf(&szTheValue, "%f %f %f", m_vecPosition2.x, m_vecPosition2.y, m_vecPosition2.z);

	else if (FStrEq(szTargetKeyName, "m_vecAngle1"))
		sprintf(&szTheValue, "%f %f %f", m_vecAngle1.x, m_vecAngle1.y, m_vecAngle1.z);

	else if (FStrEq(szTargetKeyName, "m_vecAngle2"))
		sprintf(&szTheValue, "%f %f %f", m_vecAngle2.x, m_vecAngle2.y, m_vecAngle2.z);

	else if (FStrEq(szTargetKeyName, "m_cTriggersLeft"))
		sprintf(&szTheValue, "%i", m_cTriggersLeft);

	else if (FStrEq(szTargetKeyName, "m_flHeight"))
		sprintf(&szTheValue, "%f", m_flHeight);

	else if (FStrEq(szTargetKeyName, "m_vecFinalDest"))
		sprintf(&szTheValue, "%f %f %f", m_vecFinalDest.x, m_vecFinalDest.y, m_vecFinalDest.z);

	else if (FStrEq(szTargetKeyName, "m_vecFinalAngle"))
		sprintf(&szTheValue, "%f %f %f", m_vecFinalAngle.x, m_vecFinalAngle.y, m_vecFinalAngle.z);

	else if (FStrEq(szTargetKeyName, "m_bitsDamageInflict"))
		sprintf(&szTheValue, "%i", m_bitsDamageInflict);

	else if (FStrEq(szTargetKeyName, "master"))
		sprintf(&szTheValue, "%s", STRING(m_sMaster));

	else if (FStrEq(szTargetKeyName, "m_fStayPushed"))
		sprintf(&szTheValue, "%i", m_fStayPushed);

	else if (FStrEq(szTargetKeyName, "m_fRotating"))
		sprintf(&szTheValue, "%i", m_fRotating);

	else if (FStrEq(szTargetKeyName, "m_strChangeTarget"))
		sprintf(&szTheValue, "%s", STRING(m_strChangeTarget));

	// m_ls struct
	else if (FStrEq(szTargetKeyName, "sLockedSound"))
		sprintf(&szTheValue, "%s", STRING(m_ls.sLockedSound));

	else if (FStrEq(szTargetKeyName, "sLockedSentence"))
		sprintf(&szTheValue, "%s", STRING(m_ls.sLockedSentence));

	else if (FStrEq(szTargetKeyName, "sUnlockedSound"))
		sprintf(&szTheValue, "%s", STRING(m_ls.sUnlockedSound));

	else if (FStrEq(szTargetKeyName, "sUnlockedSentence"))
		sprintf(&szTheValue, "%s", STRING(m_ls.sUnlockedSentence));

	else if (FStrEq(szTargetKeyName, "iLockedSentence"))
		sprintf(&szTheValue, "%i", m_ls.iLockedSentence);

	else if (FStrEq(szTargetKeyName, "iUnlockedSentence"))
		sprintf(&szTheValue, "%i", m_ls.iUnlockedSentence);

	else if (FStrEq(szTargetKeyName, "flwaitSound"))
		sprintf(&szTheValue, "%f", m_ls.flwaitSound);

	else if (FStrEq(szTargetKeyName, "flwaitSentence"))
		sprintf(&szTheValue, "%f", m_ls.flwaitSentence);

	else if (FStrEq(szTargetKeyName, "bEOFLocked"))
		sprintf(&szTheValue, "%i", m_ls.bEOFLocked);

	else if (FStrEq(szTargetKeyName, "bEOFUnlocked"))
		sprintf(&szTheValue, "%i", m_ls.bEOFUnlocked);

	// end m_ls struct

	else if (FStrEq(szTargetKeyName, "m_bLockedSound"))
		sprintf(&szTheValue, "%i", m_bLockedSound);

	else if (FStrEq(szTargetKeyName, "m_bLockedSentence"))
		sprintf(&szTheValue, "%i", m_bLockedSentence);

	else if (FStrEq(szTargetKeyName, "m_bUnlockedSound"))
		sprintf(&szTheValue, "%i", m_bUnlockedSound);

	else if (FStrEq(szTargetKeyName, "m_bUnlockedSentence"))
		sprintf(&szTheValue, "%i", m_bUnlockedSentence);

	else if (FStrEq(szTargetKeyName, "m_sounds"))
		sprintf(&szTheValue, "%i", m_sounds);

	else
		GetKeyValueCustom(szTheValue, szTargetKeyName);
}

void CBaseButton::SetKeyValueBase(char szTargetKeyName[64], char szTheValue[64])
{
	if (FStrEq(szTargetKeyName, "delay"))
		m_flDelay = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "killtarget"))
		m_iszKillTarget = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "framerate"))
		m_flFrameRate = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "groundspeed"))
		m_flGroundSpeed = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "lastevent"))
		m_flLastEventCheck = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "sequencefinished"))
		m_fSequenceFinished = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "activatefinished"))
		m_flActivateFinished = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "wait"))
		m_fSequenceLoops = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "lip"))
		m_fSequenceLoops = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_flTWidth"))
		m_flTWidth = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_flTLength"))
		m_flTLength = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_vecPosition1"))
		sscanf(szTheValue, "%f %f %f", &m_vecPosition1.x, &m_vecPosition1.y, &m_vecPosition1.z);

	else if (FStrEq(szTargetKeyName, "m_vecPosition2"))
		sscanf(szTheValue, "%f %f %f", &m_vecPosition2.x, &m_vecPosition2.y, &m_vecPosition2.z);

	else if (FStrEq(szTargetKeyName, "m_vecAngle1"))
		sscanf(szTheValue, "%f %f %f", &m_vecAngle1.x, &m_vecAngle1.y, &m_vecAngle1.z);

	else if (FStrEq(szTargetKeyName, "m_vecAngle2"))
		sscanf(szTheValue, "%f %f %f", &m_vecAngle2.x, &m_vecAngle2.y, &m_vecAngle2.z);

	else if (FStrEq(szTargetKeyName, "m_cTriggersLeft"))
		m_cTriggersLeft = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_flHeight"))
		m_flHeight = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_cTriggersLeft"))
		m_cTriggersLeft = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_vecFinalDest"))
		sscanf(szTheValue, "%f %f %f", &m_vecFinalDest.x, &m_vecFinalDest.y, &m_vecFinalDest.z);

	else if (FStrEq(szTargetKeyName, "m_vecFinalAngle"))
		sscanf(szTheValue, "%f %f %f", &m_vecFinalAngle.x, &m_vecFinalAngle.y, &m_vecFinalAngle.z);

	else if (FStrEq(szTargetKeyName, "m_bitsDamageInflict"))
		m_bitsDamageInflict = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "master"))
		m_sMaster = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_fStayPushed"))
		m_fStayPushed = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_fRotating"))
		m_fRotating = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_strChangeTarget"))
		m_strChangeTarget = ALLOC_STRING(szTheValue);

	// m_ls struct
	else if (FStrEq(szTargetKeyName, "sLockedSound"))
		m_ls.sLockedSound = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "sLockedSentence"))
		m_ls.sLockedSentence = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "sUnlockedSound"))
		m_ls.sUnlockedSound = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "sUnlockedSentence"))
		m_ls.sUnlockedSentence = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "iLockedSentence"))
		m_ls.iLockedSentence = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "iUnlockedSentence"))
		m_ls.iUnlockedSentence = atoi(szTheValue);

	else if (FStrEq(szTargetKeyName, "flwaitSound"))
		m_ls.flwaitSound = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "flwaitSentence"))
		m_ls.flwaitSentence = atof(szTheValue);

	else if (FStrEq(szTargetKeyName, "bEOFLocked"))
		m_ls.bEOFLocked = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "bEOFLocked"))
		m_ls.bEOFLocked = ALLOC_STRING(szTheValue);

	// end of m_ls

	else if (FStrEq(szTargetKeyName, "m_bLockedSound"))
		m_bLockedSound = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_bLockedSentence"))
		m_bLockedSentence = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_bUnlockedSound"))
		m_bUnlockedSound = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_bUnlockedSentence"))
		m_bUnlockedSentence = ALLOC_STRING(szTheValue);

	else if (FStrEq(szTargetKeyName, "m_sounds"))
		m_sounds = atoi(szTheValue);

	else
		SetKeyValueCustom(szTargetKeyName, szTheValue);
}
