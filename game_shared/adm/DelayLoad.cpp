#include "Base/ExtDLL.h"
#include "Util.h"

#include "DelayLoad.h"

/*
* Delay-loading helper function, from Solokiller's HLEnhanced.
*
* You can find HLEnhanced here:
* https://github.com/SamVanheer/HLEnhanced
*
* Gotta delay load fmod.dll and others which may come in the future
* If it hadn't been for this, I would've never managed to load fmod.dll
* from folders like adm/cl_dlls/
* It's magic.
*
* To specify new DLLs to delay-load, go to the project properties,
* and find Linker -> Input -> Delay Loaded Dlls
*/


#ifdef WIN32
HMODULE DelayLoad_LoadGameLib( const char* pszDLL, const char* pszLocalPath )
{
//	ASSERT( pszDLL );

	char szGameDir[ MAX_PATH ];
	char szPath[ MAX_PATH ];

	if ( !UTIL_GetGameDir( szGameDir, sizeof( szGameDir ) ) )
		return nullptr;

//	GetCurrentDirectoryA( MAX_PATH, szGameDir );

	int iResult;

	if ( pszLocalPath )
		iResult = snprintf( szPath, ARRAYSIZE( szPath ), "%s/%s/%s", szGameDir, pszLocalPath, pszDLL );
	else
		iResult = snprintf( szPath, ARRAYSIZE( szPath ), "%s/%s", szGameDir, pszDLL );

	if ( iResult < 0 || static_cast<size_t>(iResult) >= ARRAYSIZE( szPath ) )
		return nullptr;

	return LoadLibraryA( szPath );
}

/*
*	@brief Handles loading of shared delay loaded libraries
*/
HMODULE DelayLoad_HandleSharedLibs( unsigned dliNotify, PDelayLoadInfo pdli )
{
	return nullptr;
}

FARPROC WINAPI DelayHook(
	unsigned        dliNotify,
	PDelayLoadInfo  pdli
)
{
	if ( dliNotify == dliNotePreLoadLibrary )
	{
		return reinterpret_cast<FARPROC>(DelayLoad_LoadDelayLoadLib( dliNotify, pdli ));
	}

	return nullptr;
}

ExternC PfnDliHook __pfnDliNotifyHook2 = DelayHook;

ExternC PfnDliHook   __pfnDliFailureHook2 = nullptr;
#endif

#ifdef WIN32
#include "DelayLoad.h"

//See DelayHook in DelayLoad.cpp
HMODULE DelayLoad_LoadDelayLoadLib( unsigned dliNotify, PDelayLoadInfo pdli )
{
	if ( strcmp( pdli->szDll, "sqlite3.dll" ) == 0 )
	{
		return DelayLoad_LoadGameLib( pdli->szDll );
	}
	else if ( !strcmp( pdli->szDll, "fmod.dll" ) == 0 )
	{
		return DelayLoad_LoadGameLib( pdli->szDll );
	}

	return nullptr;
}
#endif
	