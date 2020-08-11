# Code style & standards guidelines

This page is meant to show future contributors and programmers - who will use this mod base in their project - the coding standards that they should follow. Not following the standards may result in pull requests not getting merged directly, or not getting merged at all depending on the severity.

## Introduction - Half-Life SDK's inconsistencies

In the HL SDK, coding style is all over the place.

Observe the following function from Player.cpp:
```cpp
/* 
 *
 */
Vector VecVelocityForDamage(float flDamage)
{
	Vector vec(RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));

	if (flDamage > -50)
		vec = vec * 0.7;
	else if (flDamage > -200)
		vec = vec * 2;
	else
		vec = vec * 10;
	
	return vec;
}
```

There is Hungarian notation in the function's name, as well as the variables in it. 
There's no space between the braces and the function arguments. `(float flDamage)`

Now, let's observe another function which is a bit below it:
```cpp
//=========================================================
// TraceAttack
//=========================================================
void CBasePlayer :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if ( pev->takedamage )
	{
		m_LastHitGroup = ptr->iHitgroup;

		switch ( ptr->iHitgroup )
		{
      [cut out this part]
		}

		SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.
		TraceBleed( flDamage, vecDir, ptr, bitsDamageType );
		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );
	}
}
```
Firstly, we notice that the comments above them are differently formatted. Secondly, there's no Hungarian notation in the function name, albeit it's still present in arguments and local variables.
Weirdly enough, there's a space from the bracket at the start, but not at the end. There's also a space before and after the `::`, which is a little unusual.
The call to `SpawnBlood` doesn't have spaces between brackets and arguments, but the calls to `TraceBleed` and `AddMultiDamage` do.

There are a few cases where Hungarian notation looks confusing, such as this:
```cpp
typedef struct sentenceg
{
	char szgroupname[CBSENTENCENAME_MAX];
	int count;
	unsigned char rgblru[CSENTENCE_LRU_MAX];

} SENTENCEG;
```
Also in this code example, there's a lot of `typedef struct someType_s someType_t;`-style type definitions.

Now, let's take a look at some clientside GUI code in vgui_ClassMenu.cpp:
```cpp
//======================================
// Key inputs for the Class Menu
bool CClassMenuPanel::SlotInput( int iSlot )
{
	if ( (iSlot < 0) || (iSlot > 9) )
		return false;
	if ( !m_pButtons[ iSlot ] )
		return false;

	// Is the button pushable? (0 is special case)
	if (iSlot == 0)
	{
		// Selects Civilian and RandomPC
		if ( gViewPort->GetValidClasses(g_iTeamNumber) == -1 )
		{
			m_pButtons[ 0 ]->fireActionSignal();
			return true;
		}

		// Select RandomPC
		iSlot = 10;
	}

	if ( !(m_pButtons[ iSlot ]->IsNotValid()) )
	{
		m_pButtons[ iSlot ]->fireActionSignal();
		return true;
	}

	return false;
}
```
Notice how there's a space between angular brackets `[ iSlot ]` which differs greatly from the rest of the SDK. Also, methods of VGUI classes tend to be in `camelCase` with a lowercase start.   
This is inconsistent with the rest of the SDK, where most methods are `CamelCase` with an uppercase start.

Lastly, we'll take a look at classes. For example, in Player.h: 
```cpp
	BOOL				m_fInitHUD;				// True when deferred HUD restart msg needs to be sent
	BOOL				m_fGameHUDInitialized;
	int					m_iTrain;				// Train control position
	BOOL				m_fWeapon;				// Set this to FALSE to force a reset of the current weapon HUD info

	EHANDLE				m_pTank;				// the tank which the player is currently controlling,  NULL if no tank
	float				m_fDeadTime;			// the time at which the player died  (used in PlayerDeathThink())

	BOOL			m_fNoPlayerSound;	// a debugging feature. Player makes no sound if this is true. 
	BOOL			m_fLongJump; // does this player have the longjump module?

	float       m_tSneaking;
	int			m_iUpdateTime;		// stores the number of frame ticks before sending HUD update messages
	int			m_iClientHealth;	// the health currently known by the client.  If this changes, send a new
	int			m_iClientBattery;	// the Battery currently known by the client.  If this changes, send a new
	int			m_iHideHUD;		// the players hud weapon info is to be hidden
	int			m_iClientHideHUD;
	int			m_iFOV;			// field of view
	int			m_iClientFOV;	// client's known FOV
	// usable player items 
	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem *m_pActiveItem;
	CBasePlayerItem *m_pClientActiveItem;  // client version of the active item
	CBasePlayerItem *m_pLastItem;
```
Tabs are not consistent. This is not GitHub's tab rendering, these are actual tabs in the code, and they're different in numbers.

# Advanced Development Mod style

The ADM style and HL SDK's average style differ significantly, however, the enforced style will be the ADM one. Eventually, the entire HL SDK (except engine interfaces etc.) will be reformatted accordingly.

Before I define the standards in ADM, I first have to point out the:

## Inconsistencies in ADM

I should note that there are chunks of code that are from 2018 and early 2019, that still haven't been refactored nor reformatted. Refer to this page in case you get confused when seeing those inconsistencies.   
Candidate sources include: AdmTriggers.cpp, AdmStory.cpp, AdmEffects.cpp and AdmAudioEntities.cpp.

Eventually, the entire SDK will be reformatted according to these guidelines.

## Naming

Classes: `CamelCase_UnderscoreTolerant` - no prefixing with `C`, only interfaces may be prefixed with `I`   
Structs, and classes that represent some sort of data: `camelCase` - no suffixing with `_t`   
Namespaces: `CamelCase`   
Variables: `camelCase` - including member variables of classes (no prefixing with `m_`)   
Global variables: `g_CamelCase`
Constants: `CamelCase_UnderscoreTolerant`

Hungarian notation is highly discouraged. Prefixing pointers with `p`, and prefixing for specific data types is redundant.   

## Files

Ideally, if you're implementing a new class, for example `FuncOscillating`, it will have two files: `FuncOscillating.cpp` and `FuncOscillating.hpp`.   
The .cpp file will include all the necessary headers and implement the class methods, whereas the .hpp file will declare the class itself, as well as all the necessary enums, structs and constants it uses.

Right now, there are still many .cpp files that have multiple classes and implementations in them, but that will be finished by the time ADM 0.5 comes out.

Also, all ADM-specific code will go into `ADM` folders. Similarly, if you are making your own mod, it'd be nice to have a folder for it.   

## Brackets, tabs, newlines and spaces

Function calls and definitions: `FunctionName( dataType functionArgument )`
If, while, for and other keywords: `switch ( condition )`
Constructor calls: `Vector3( 0.25f, 0.2f, 30.0f )`

In function definitions and if statements, you must always enter a new line before the curly bracket, such as:
```cpp
void SomeFunction()
{
  if ( condition )
  {
    code
  }
}
```

## Macros

Usage of macros should be minimal except for where it's appropriate, such as in save-restore tables.   
Instead of defining constants via macros, use `constexpr`.

## Other

It'd be nice to suffix all float lvalues with `f`, such as in the Vector3 construction above. Doubles are rarely ever used, if at all.   

This page may change from time to time, mostly to fill in some missing things. Existing rules are unlikely to change.
