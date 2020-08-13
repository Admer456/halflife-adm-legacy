#pragma once

class EnvSky2D final : public CBaseEntity
{
public:

	// Copy-pasted from SimpleSky.h on the clientside
	// Very sorry! This is only temporary and will be moved in one of the next commits
	enum SkyFlags : uint8_t
	{
		SkyFlag_None = 0,

		// Instead of being relative to world origin, it latches onto the player's view
		SkyFlag_FollowsPlayerView = 1 << 0,

		// The top and bottom of the sky get scaled differently in real-time
		SkyFlag_Squeeze = 1 << 1,

		// Sky rotates along the Z axis
		SkyFlag_Rotates = 1 << 2,

		// Vertices randomly go in different directions
		SkyFlag_Wobble = 1 << 3
	};

	struct SkyParameters
	{
		int					skyFlags{ 0 };
		float				rotationSpeed{ 0.0f };
		float				wobbleFrequency{ 0.0f };
		float				wobbleScale{ 0.0f };
		unsigned int		skyDistance{ 0 };
		char*				skyName{ nullptr };
	};

public:
	void					Spawn() override;
	void					KeyValue( KeyValueData* pkvd ) override;
	void					Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value ) override;

private:
	SkyParameters			skyParams;
	string_t				skyName;
};

extern int gmsgSky2D;
