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

		Vector			GetPosition()
		{
			Vector pos;
			pos.x = position.x;
			pos.y = position.y;
			pos.z = position.z;

			return pos;
		}

	private:
		FMOD_VECTOR		position;
		FMOD_VECTOR		velocity;

		float			minDistance{ 0.1f };
		float			maxDistance{ 512.0f };

		bool			needsUpdate{ false };
	};
}