#pragma once
namespace AdmSound
{
	class SoundSourceSpatial : public SoundSource
	{
	public:
		SoundSourceSpatial( const char* soundPath, uint16_t soundFlags, const Vector& soundPosition, const Vector& soundVelocity );
		virtual ~SoundSourceSpatial() = default;
		
		virtual void	Update() override;

		void			SetPosition( const Vector& newPosition );
		void			SetVelocity( const Vector& newVelocity );

	private:
		FMOD_VECTOR		position;
		FMOD_VECTOR		velocity;

		float			minDistance;
		float			maxDistance;

		bool			needsUpdate{ false };
	};
}