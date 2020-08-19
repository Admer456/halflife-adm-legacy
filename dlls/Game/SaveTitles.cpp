#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#include <string>
#include <vector>
#include <fstream>

class SaveTitle
{
public:
	SaveTitle( const char* map, const char* title )
	{
		mapName = map;
		titleName = title;
	}

	bool Matches( const char* map )
	{
		return !stricmp( mapName.c_str(), map );
	}

	const char* GetTitle()
	{
		return titleName.c_str();
	}

private:
	std::string mapName;
	std::string titleName;
};

static SaveTitle hardcodedMapTitles[] =
{
	// =================================
	// Half-Life maps
	// =================================

	// Hazard course
	SaveTitle( "t0a0",	"#T0A0TITLE" ),
	SaveTitle( "t0a0a",	"#T0A0TITLE" ),
	SaveTitle( "t0a0b",	"#T0A0TITLE" ),
	SaveTitle( "t0a0b1","#T0A0TITLE" ),
	SaveTitle( "t0a0b2","#T0A0TITLE" ),
	SaveTitle( "t0a0c",	"#T0A0TITLE" ),
	SaveTitle( "t0a0d",	"#T0A0TITLE" ),

	// Black Mesa inbound
	SaveTitle( "c0a0",	"#C0A0TITLE" ),
	SaveTitle( "c0a0a",	"#C0A0TITLE" ),
	SaveTitle( "c0a0b",	"#C0A0TITLE" ),
	SaveTitle( "c0a0c",	"#C0A0TITLE" ),
	SaveTitle( "c0a0d",	"#C0A0TITLE" ),
	SaveTitle( "c0a0e",	"#C0A0TITLE" ),

	// Anomalous materials
	SaveTitle( "c1a0",	"#C0A1TITLE" ),
	SaveTitle( "c1a0a",	"#C0A1TITLE" ),
	SaveTitle( "c1a0b",	"#C0A1TITLE" ),
	SaveTitle( "c1a0c",	"#C0A1TITLE" ),
	SaveTitle( "c1a0d",	"#C0A1TITLE" ),
	SaveTitle( "c1a0e",	"#C0A1TITLE" ),

	// Unforeseen consequences
	SaveTitle( "c1a1",	"#C1A1TITLE" ),
	SaveTitle( "c1a1a",	"#C1A1TITLE" ),
	SaveTitle( "c1a1b",	"#C1A1TITLE" ),
	SaveTitle( "c1a1c",	"#C1A1TITLE" ),
	SaveTitle( "c1a1d",	"#C1A1TITLE" ),
	SaveTitle( "c1a1f",	"#C1A1TITLE" ),

	// Office complex
	SaveTitle( "c1a2",	"#C1A2TITLE" ),
	SaveTitle( "c1a2a",	"#C1A2TITLE" ),
	SaveTitle( "c1a2b",	"#C1A2TITLE" ),
	SaveTitle( "c1a2c",	"#C1A2TITLE" ),
	SaveTitle( "c1a2d",	"#C1A2TITLE" ),

	// We've got hostiles
	SaveTitle( "c1a3",	"#C1A3TITLE" ),
	SaveTitle( "c1a3a",	"#C1A3TITLE" ),
	SaveTitle( "c1a3b",	"#C1A3TITLE" ),
	SaveTitle( "c1a3c",	"#C1A3TITLE" ),
	SaveTitle( "c1a3d",	"#C1A3TITLE" ),

	// Blast pit
	SaveTitle( "c1a4",	"#C1A4TITLE" ),
	SaveTitle( "c1a4b",	"#C1A4TITLE" ),
	SaveTitle( "c1a4d",	"#C1A4TITLE" ),
	SaveTitle( "c1a4e",	"#C1A4TITLE" ),
	SaveTitle( "c1a4f",	"#C1A4TITLE" ),
	SaveTitle( "c1a4g",	"#C1A4TITLE" ),
	SaveTitle( "c1a4i",	"#C1A4TITLE" ),
	SaveTitle( "c1a4j",	"#C1A4TITLE" ),
	SaveTitle( "c1a4k",	"#C1A4TITLE" ),

	// Power up
	SaveTitle( "c2a1",	"#C2A1TITLE" ),
	SaveTitle( "c2a1a",	"#C2A1TITLE" ),
	SaveTitle( "c2a1b",	"#C2A1TITLE" ),

	// On a rail
	SaveTitle( "c2a2",	"#C2A2TITLE" ),
	SaveTitle( "c2a2a",	"#C2A2TITLE" ),
	SaveTitle( "c2a2b1","#C2A2TITLE" ),
	SaveTitle( "c2a2b2","#C2A2TITLE" ),
	SaveTitle( "c2a2c",	"#C2A2TITLE" ),
	SaveTitle( "c2a2d",	"#C2A2TITLE" ),
	SaveTitle( "c2a2e",	"#C2A2TITLE" ),
	SaveTitle( "c2a2f",	"#C2A2TITLE" ),
	SaveTitle( "c2a2g",	"#C2A2TITLE" ),
	SaveTitle( "c2a2h",	"#C2A2TITLE" ),
	
	// Apprehension
	SaveTitle( "c2a3",	"#C2A3TITLE" ),
	SaveTitle( "c2a3a",	"#C2A3TITLE" ),
	SaveTitle( "c2a3b",	"#C2A3TITLE" ),
	SaveTitle( "c2a3c",	"#C2A3TITLE" ),
	SaveTitle( "c2a3d",	"#C2A3TITLE" ),
	SaveTitle( "c2a3e",	"#C2A3TITLE" ),
	
	// Residue processing
	SaveTitle( "c2a4",	"#C2A4TITLE1" ),
	SaveTitle( "c2a4a",	"#C2A4TITLE1" ),
	SaveTitle( "c2a4b",	"#C2A4TITLE1" ),
	SaveTitle( "c2a4c",	"#C2A4TITLE1" ),

	// Questionable ethics
	SaveTitle( "c2a4d",	"#C2A4TITLE2" ),
	SaveTitle( "c2a4e",	"#C2A4TITLE2" ),
	SaveTitle( "c2a4f",	"#C2A4TITLE2" ),
	SaveTitle( "c2a4g",	"#C2A4TITLE2" ),
	
	// Surface tension
	SaveTitle( "c2a5",	"#C2A5TITLE" ),
	SaveTitle( "c2a5a",	"#C2A5TITLE" ),
	SaveTitle( "c2a5b",	"#C2A5TITLE" ),
	SaveTitle( "c2a5c",	"#C2A5TITLE" ),
	SaveTitle( "c2a5d",	"#C2A5TITLE" ),
	SaveTitle( "c2a5e",	"#C2A5TITLE" ),
	SaveTitle( "c2a5f",	"#C2A5TITLE" ),
	SaveTitle( "c2a5g",	"#C2A5TITLE" ),
	SaveTitle( "c2a5w",	"#C2A5TITLE" ),
	SaveTitle( "c2a5x",	"#C2A5TITLE" ),
	
	// Forget about Freeman
	SaveTitle( "c3a1",	"#C3A1TITLE" ),
	SaveTitle( "c3a1a",	"#C3A1TITLE" ),
	SaveTitle( "c3a1b",	"#C3A1TITLE" ),
	
	// Lambda core
	SaveTitle( "c3a2",	"#C3A2TITLE" ),
	SaveTitle( "c3a2a",	"#C3A2TITLE" ),
	SaveTitle( "c3a2b",	"#C3A2TITLE" ),
	SaveTitle( "c3a2c",	"#C3A2TITLE" ),
	SaveTitle( "c3a2d",	"#C3A2TITLE" ),
	SaveTitle( "c3a2e",	"#C3A2TITLE" ),
	SaveTitle( "c3a2f",	"#C3A2TITLE" ),
	
	// Xen
	SaveTitle( "c4a1",	"#C4A1TITLE" ),
	SaveTitle( "c4a1a",	"#C4A1TITLE" ),
	SaveTitle( "c4a1b",	"#C4A1TITLE" ),
	SaveTitle( "c4a1c",	"#C4A1TITLE" ),
	SaveTitle( "c4a1d",	"#C4A1TITLE" ),
	SaveTitle( "c4a1e",	"#C4A1TITLE" ),
	SaveTitle( "c4a1f",	"#C4A1TITLE" ),
	
	// Gonarch's lair
	SaveTitle( "c4a2",	"#C4A2TITLE" ),
	SaveTitle( "c4a2a",	"#C4A2TITLE" ),
	SaveTitle( "c4a2b",	"#C4A1ATITLE" ),
	
	// Interloper
	SaveTitle( "c4a3",	"#C4A3TITLE" ),
	
	// Nihilanth
	SaveTitle( "c5a1",	"#C5TITLE" ),
	
	// =================================
	// Half-Life: Opposing Force maps
	// =================================

	// Incoming
	SaveTitle( "of0a0",	"#OF1A1TITLE" ),
	
	// Welcome to Black Mesa
	SaveTitle( "of1a1",	"#OF1A3TITLE" ),
	SaveTitle( "of1a2",	"#OF1A3TITLE" ),
	SaveTitle( "of1a3",	"#OF1A3TITLE" ),
	SaveTitle( "of1a4",	"#OF1A3TITLE" ),
	SaveTitle( "of1a4b","#OF1A3TITLE" ),
	
	// We are pulling out
	SaveTitle( "of1a5",	"#OF1A5TITLE" ),
	SaveTitle( "of1a5b","#OF1A5TITLE" ),
	SaveTitle( "of1a6",	"#OF1A5TITLE" ),
	
	// Missing in action
	SaveTitle( "of2a1",	"#OF2A1TITLE" ),
	SaveTitle( "of2a1b","#OF2A1TITLE" ),
	SaveTitle( "of2a2",	"#OF2A1TITLE" ),
	SaveTitle( "of2a3",	"#OF2A1TITLE" ),
	
	// Friendly fire
	SaveTitle( "of2a4",	"#OF2A4TITLE" ),
	SaveTitle( "of3a5",	"#OF2A4TITLE" ),
	SaveTitle( "of3a5b","#OF2A4TITLE" ),
	
	// We are not alone
	SaveTitle( "of3a1",	"#OF3A1TITLE" ),
	SaveTitle( "of3a1b","#OF3A1TITLE" ),
	SaveTitle( "of3a2",	"#OF3A1TITLE" ),
	SaveTitle( "of3a3",	"#OF3A1TITLE" ),
	
	// Crush depth
	SaveTitle( "of3a4",	"#OF3A3TITLE" ),
	SaveTitle( "of3a5",	"#OF3A3TITLE" ),
	SaveTitle( "of3a6",	"#OF3A3TITLE" ),
	
	// Vicarious reality
	SaveTitle( "of4a1",	"#OF4A1TITLE" ),
	SaveTitle( "of4a2",	"#OF4A1TITLE" ),
	SaveTitle( "of4a3",	"#OF4A1TITLE" ),
	
	// Pit Worm's nest
	SaveTitle( "of4a4",	"#OF4A4TITLE" ),
	SaveTitle( "of4a5",	"#OF4A4TITLE" ),
	
	// Foxtrot uniform
	SaveTitle( "of5a1",	"#OF5A1TITLE" ),
	SaveTitle( "of5a2",	"#OF5A1TITLE" ),
	SaveTitle( "of5a3",	"#OF5A1TITLE" ),
	SaveTitle( "of5a4",	"#OF5A1TITLE" ),
	
	// The package
	SaveTitle( "of6a1",	"#OF6A1TITLE" ),
	SaveTitle( "of6a2",	"#OF6A1TITLE" ),
	SaveTitle( "of6a3",	"#OF6A1TITLE" ),
	SaveTitle( "of6a4",	"#OF6A1TITLE" ),
	
	// Worlds collide
	SaveTitle( "of6a4b","#OF6A4TITLE" ),
	SaveTitle( "of6a5",	"#OF6A4TITLE" ),
	
	// Conclusion
	SaveTitle( "of7a0",	"#OF7A0TITLE" )
};

const char* FindHardcodedMapTitle( const char* mapName )
{
	for ( SaveTitle& entry : hardcodedMapTitles )
	{
		if ( entry.Matches( mapName ) )
		{
			return entry.GetTitle();
		}
	}

	return nullptr;
}

const char* FindMapTitleInFile( const char* mapName )
{
	char titleFileName[64];
	char modDirectory[32];

	static std::vector<SaveTitle> titles;
	titles.clear();

	std::string currentLine;

	// Assemble the path
	UTIL_GetGameDir( modDirectory, 32 );
	sprintf( titleFileName, "%s/maps/maptitles.txt", modDirectory );

	// Open the file
	std::ifstream titleFile( titleFileName );

	if ( !titleFile.is_open() )
	{
		ALERT( at_error, "Could not open maps/maptitles.txt!\n" );
		return nullptr;
	}
	
	int lineNumber = -1;

	// TODO: This is SERIOUS, we need to parse this elsewhere, not every time we're saving!
	// Check every line
	while ( std::getline( titleFile, currentLine ) )
	{
		lineNumber++;

		// Ignore lines starting with //
		if ( currentLine.find( "//" ) <= 1 )
			continue;

		// If a line has a ", it may be added
		// We'll enforce rules better later, once I make my litle text parsing library
		if ( currentLine.find( '"' ) == std::string::npos )
			continue;
			
		// There has to be at least one space before a " too
		if ( currentLine.find( '"' ) < currentLine.find( ' ' ) )
			continue;

		// The line is eligible to be added, now we just have to split it up
		std::string mapNameString, saveTitleString;

		// The way we'll do this is to find the first space from the start of the line
		size_t firstSpace = currentLine.find( ' ' );
		mapNameString = currentLine.substr( 0, firstSpace );

		// As for the title string, we'll just grab everything between the quotation marks
		size_t firstQuote = currentLine.find( '"' );
		size_t secondQuote = currentLine.find( '"', firstQuote + 1 );
		
		// It sometimes happens
		if ( secondQuote == std::string::npos )
		{
			ALERT( at_error, "Syntax error at line %i, no second '\"' found!\nSkipping...\n", lineNumber );
			continue;
		}

		saveTitleString = currentLine.substr( firstQuote + 1, (secondQuote - firstQuote)-1 );

		// Finally, add this to our temporary list of SaveTitles
		titles.push_back( SaveTitle( mapNameString.c_str(), saveTitleString.c_str() ) );
	}

	// Check every title
	for ( SaveTitle& entry : titles )
	{
		if ( entry.Matches( mapName ) )
		{
			return entry.GetTitle();
		}
	}

	// If you didn't find any, it's okay, just return a nullptr
	return nullptr;
}

const char* FindMapTitle( const char* mapName )
{
	// First, try finding it in maps/maptitles.txt
	const char* titleName = FindMapTitleInFile( mapName );

	// If not available, try finding it in one of the 150 hardcoded map titles
	if ( !titleName )
	{
		titleName = FindHardcodedMapTitle( mapName );
	}

	return titleName;
}

/*
================================
SV_SaveGameComment

The engine calls this everytime a player saves the game,
to read the save title provided by this DLL

Thank you Solokiller and vasiavasiavasia95!
================================
*/
extern "C" void DLLEXPORT SV_SaveGameComment( char* buffer, int bufferSize )
{
	const char* mapName = STRING( gpGlobals->mapname );

	// I dunno, it might happen
	if ( !mapName || !mapName[0] )
	{
		ALERT( at_error, "Map name is somehow empty?!\n" );
		abort();
	}

	const char* mapTitle = FindMapTitle( mapName );

	// If you can't do even that, well, set the raw map name then
	if ( !mapTitle )
	{
		mapTitle = mapName;
	}

	strncpy( buffer, mapTitle, bufferSize - 1 );
	buffer[bufferSize - 1] = '\0';

	ALERT( at_console, "Save title buffer size: %i\n", bufferSize );
}
