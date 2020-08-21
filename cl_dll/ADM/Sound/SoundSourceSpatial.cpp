#include "WRect.h"
#include "CL_DLL.h"

#include "SoundSystemCore.h"
#include "BaseSound.h"
#include "Channel.h"
#include "SoundSource.h"
#include "SoundSourceSpatial.h"

namespace AdmSound
{
	SoundSourceSpatial::SoundSourceSpatial( const char* soundPath, uint16_t soundFlags, const Vector& soundPosition, const Vector& soundVelocity )
		: SoundSource( soundPath, soundFlags )
	{
		// Initialise 3D sound properties
		SetPosition( soundPosition );
		SetVelocity( soundVelocity );

		sound.GetFMODSound()->setMode( FMOD_3D );
		channel->setMode( FMOD_3D );
		channel->set3DSpread( 45.f ); // Set some stereo spread in case we have a non-mono sound sample
		channel->set3DMinMaxDistance( minDistance, maxDistance );
	}

	void SoundSourceSpatial::Update()
	{
		SoundSource::Update();

		channel->set3DAttributes( &position, &velocity );
		//channel->set3DMinMaxDistance( minDistance, maxDistance );

		// This bool will be removed later
		// Turns out, the position and velocity need to be updated every frame
		needsUpdate = false;
	}

	void SoundSourceSpatial::SetPosition( const Vector& newPosition )
	{
		position = VectorToFMODVector( newPosition );
		needsUpdate = true;
	}

	void SoundSourceSpatial::SetVelocity( const Vector& newVelocity )
	{
		velocity = VectorToFMODVector( newVelocity );
		needsUpdate = true;
	}
}