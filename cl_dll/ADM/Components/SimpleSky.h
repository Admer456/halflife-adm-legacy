//========================================
//	SimpleSky
// 
//	A sky renderer that supports
//	128x128 TGA skies, with the option
//	to rotate them too
//========================================
class SimpleSky final : public CBaseClientExtension
{
public:
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
		float				rotationSpeed{ 0.0f }; // rotation speed in angles/sec
		float				wobbleFrequency{ 0.0f }; // wobbling frequency in Hz
		float				wobbleScale{ 0.0f }; // wobbling scale in GoldSRC units
		unsigned int		skyDistance{ 0 }; // power of 2 in GoldSRC units; distance of each sky plane from the world origin
		char*				skyName{ nullptr };
	};

public:
	void					InitExtension() override;
	
	void					Think() override;
	void					Reset() override;

	void					Render( triangleapi_t* r ); // Called in HUD_DrawTransparentTriangles
	void					RenderSide( triangleapi_t* r, const int& face );
	void					RenderRegularSide( triangleapi_t* r, const int& face );
	void					RenderRotatedSide( triangleapi_t* r, const int& face );
	
	void					SetParameters( const SkyParameters& skyParams );
	void					UpdateSkyCube();
	void					WobbleSkyCube( const float& cycle );
	void					SqueezeSkyCube( const float& cycle );
	void					ResetSkyCube();

	void					UpdateSkyTexture();

private:
	constexpr static const char* skySides[] = { "up", "dn", "lf", "rt", "ft", "bk" };

	int						skyFlags{ SkyFlag_None };
	float					rotationSpeed{ 0.0f };
	float					currentRotationAngle{ 0.0f };
	float					wobbleFrequency{ 5.0f };
	float					wobbleScale{ 10.0f };
	float					skyDistance{ 8192.0f }; // How far each plane is from the world origin; must be smaller than the actual render distance of the map

	bool					needsUpdate{ false };

	char					skyName[32];
	model_t*				skyTextures[6];
};

extern SimpleSky g_Sky;
