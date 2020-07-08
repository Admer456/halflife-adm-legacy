#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#include "../shared/ADM/AdmSound.h"

extern int gmsgFSSMusic;
SoundTable g_ServerSoundTable;

class CAudioMusic : public CBaseEntity
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

private:
	string_t m_iszSong;
};

LINK_ENTITY_TO_CLASS( audio_music, CAudioMusic );

void CAudioMusic::Spawn( void )
{
	g_ServerSoundTable.AddString( STRING(m_iszSong) );
}

void CAudioMusic::KeyValue( KeyValueData *pkvd )
{
	if ( KeyvalueToken( m_iszSong ) )
	{
		KeyvalueToString( m_iszSong );
	}
	else
	{
		KeyvaluesFromBase( CBaseEntity );
	}
}

void CAudioMusic::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	MESSAGE_BEGIN( MSG_ALL, gmsgFSSMusic );
		WRITE_STRING( STRING( m_iszSong ) );
	MESSAGE_END();
}

/*
	message(sound) : "WAV Name"
	health(integer) : "Volume (10 = loudest)" : 10
	preset(choices) :"Dynamic Presets" : 0 =
	[
		0: "None"
		1: "Huge Machine"
		2: "Big Machine"
		3: "Machine"
		4: "Slow Fade in"
		5: "Fade in"
		6: "Quick Fade in"
		7: "Slow Pulse"
		8: "Pulse"
		9: "Quick pulse"
		10: "Slow Oscillator"
		11: "Oscillator"
		12: "Quick Oscillator"
		13: "Grunge pitch"
		14: "Very low pitch"
		15: "Low pitch"
		16: "High pitch"
		17: "Very high pitch"
		18: "Screaming pitch"
		19: "Oscillate spinup/down"
		20: "Pulse spinup/down"
		21: "Random pitch"
		22: "Random pitch fast"
		23: "Incremental Spinup"
		24: "Alien"
		25: "Bizzare"
		26: "Planet X"
		27: "Haunted"
	]
	volstart(integer) : "Start Volume" : 0
	fadein(integer) : "Fade in time (0-100)" : 0
	fadeout(integer) : "Fade out time (0-100)" : 0
	pitch(integer) : "Pitch (> 100 = higher)" : 100
	pitchstart(integer) : "Start Pitch" : 100
	spinup(integer) : "Spin up time (0-100)" : 0
	spindown(integer) : "Spin down time (0-100)" : 0
	lfotype(integer) : "LFO type 0)off 1)sqr 2)tri 3)rnd" : 0
	lforate(integer) : "LFO rate (0-1000)" : 0
	lfomodpitch(integer) : "LFO mod pitch (0-100)" : 0
	lfomodvol(integer) : "LFO mod vol (0-100)" : 0
	cspinup(integer) : "Incremental spinup count" : 0
	spawnflags(flags) =
	[
		1 : "Play Everywhere" : 0
		2 : "Small Radius" : 0
		4 : "Medium Radius" : 1
		8 : "Large Radius" : 0
		16 : "Start Silent":0
		32 : "Not Toggled":0
	]
*/