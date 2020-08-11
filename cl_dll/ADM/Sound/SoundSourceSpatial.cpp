#include "WRect.h"
#include "CL_DLL.h"

#include "SoundSystemCore.h"
#include "BaseSound.h"
#include "Channel.h"
#include "SoundSource.h"
#include "SoundSourceSpatial.h"

namespace AdmSound
{
	SoundSourceSpatial::SoundSourceSpatial( const char* soundPath, const Vector& soundPosition, const Vector& soundVelocity )
		: SoundSource( soundPath )
	{
		// Initialise 3D sound properties
		SetPosition( soundPosition );
		SetVelocity( soundVelocity );

		channel->setMode( FMOD_3D );
		channel->set3DMinMaxDistance( minDistance, maxDistance );
	}

	void SoundSourceSpatial::Update()
	{
		SoundSource::Update();

		// Update 3D sound properties only when needed 
		if ( needsUpdate )
		{
			channel->set3DAttributes( &position, &velocity );
			channel->set3DMinMaxDistance( minDistance, maxDistance );

			needsUpdate = false;
		}
	}

	void SoundSourceSpatial::SetPosition( const Vector& newPosition )
	{
		position.x = newPosition.x;
		position.y = newPosition.y;
		position.z = newPosition.z;

		needsUpdate = true;
	}

	void SoundSourceSpatial::SetVelocity( const Vector& newVelocity )
	{
		velocity.x = newVelocity.x;
		velocity.y = newVelocity.y;
		velocity.z = newVelocity.z;

		needsUpdate = true;
	}
}