#include "HUD/HUD.h"
#include "CL_Util.h"
#include "ParseMsg.h"
#include "triangleapi.h"
#include "com_model.h"

#include "SimpleSky.h"

extern float v_frametime;

enum SkyPoints
{
	LeftBackBottom,
	RightBackBottom,
	RightFrontBottom,
	LeftFrontBottom,

	LeftBackTop,
	RightBackTop,
	RightFrontTop,
	LeftFrontTop
};

const Vector skyCubeReferenceVertices[] =
{
	Vector(  1,  1, -1 ), // left back bottom
	Vector( -1,  1, -1 ), // right back bottom
	Vector( -1, -1, -1 ), // right front bottom
	Vector(  1, -1, -1 ), // left front bottom

	Vector(  1,  1,  1 ), // left back top
	Vector( -1,  1,  1 ), // right back top
	Vector( -1, -1,  1 ), // right front top
	Vector(  1, -1,  1 ), // left front top
};

Vector skyCubeVertices[] =
{
	Vector(  1,  1, -1 ), // left back bottom
	Vector( -1,  1, -1 ), // right back bottom
	Vector( -1, -1, -1 ), // right front bottom
	Vector(  1, -1, -1 ), // left front bottom

	Vector(  1,  1,  1 ), // left back top
	Vector( -1,  1,  1 ), // right back top
	Vector( -1, -1,  1 ), // right front top
	Vector(  1, -1,  1 ), // left front top
};

// Yaw and pitch offsets for each cube vertex
int angleOffsets[][2] =
{
	{ 45,		-45 }, // left back bottom
	{ 45 + 90,	-45 }, // right back bottom
	{ 45 + 180,	-45 }, // right front bottom
	{ 45 + 270,	-45 }, // left front bottom

	{ 45 + 180,	 45 }, // left back top
	{ 45 + 270,	 45 }, // right back top
	{ 45,		 45 }, // right front top
	{ 45 + 90,	 45 }, // left front top
};

// UV coordinates for quad rendering
float uvCoords[][2] =
{
	{ 1.0f, 1.0f }, // bottom right
	{ 0.0f, 1.0f }, // bottom left
	{ 0.0f, 0.0f }, // top left
	{ 1.0f, 0.0f }, // top right
};

// Indices for each face of the cube
int skyCubeFaces[][4] =
{
	{ LeftFrontTop,		LeftBackTop,		RightBackTop,		RightFrontTop	},	// Top face
	{ RightFrontBottom,	RightBackBottom,	LeftBackBottom,		LeftFrontBottom	},	// Bottom face
	{ RightBackBottom,	RightFrontBottom,	RightFrontTop,		RightBackTop	},	// Left face
	{ LeftFrontBottom,	LeftBackBottom,		LeftBackTop,		LeftFrontTop	},	// Right face
	{ RightFrontBottom, LeftFrontBottom,	LeftFrontTop,		RightFrontTop	},	// Front face
	{ LeftBackBottom,	RightBackBottom,	RightBackTop,		LeftBackTop		},	// Back face
};

// Screw DECLARE_MESSAGE and HOOK_MESSAGE
// I'm gonna overhaul the HUD system in the next release
int Message_SimpleSky( const char* name, int size, void* buffer )
{
	BEGIN_READ( buffer, size );

	SimpleSky::SkyParameters sky;

	sky.skyFlags = READ_BYTE();
	sky.rotationSpeed = READ_SHORT() / 10.0f;
	sky.wobbleFrequency = READ_SHORT() / 10.0f;
	sky.wobbleScale = READ_SHORT();
	sky.skyDistance = READ_BYTE();

	if ( sky.skyDistance > 18 )
		sky.skyDistance = 1 << 18;
	else
		sky.skyDistance = 1 << sky.skyDistance;

	sky.skyName = READ_STRING();

	g_Sky.SetParameters( sky );

	return 1;
}

void SimpleSky::InitExtension()
{
	memset( skyName, 0, 32U );
	for ( int i = 0; i < 6; i++ )
		skyTextures[i] = nullptr;

	gEngfuncs.pfnHookUserMsg( "SSky", &Message_SimpleSky );
}

void SimpleSky::Think()
{
	UpdateSkyCube();
}

void SimpleSky::Render( triangleapi_t* r )
{
	if ( skyTextures[0] == nullptr )
		return;

	r->CullFace( TRI_NONE );
	r->RenderMode( kRenderNormal );
	r->Color4ub( 255, 255, 255, 255 );

	for ( int i = 0; i < 6; i++ )
	{
		RenderSide( r, i );
	}
}

void SimpleSky::RenderSide( triangleapi_t* r, const int& face )
{
	if ( skyFlags & SkyFlag_Rotates )
	{
		RenderRotatedSide( r, face );
	}
	else
	{
		RenderRegularSide( r, face );
	}
}

void SimpleSky::RenderRegularSide( triangleapi_t* r, const int& face )
{
	int* indices = skyCubeFaces[face];
	float radius = skyDistance / 1.41421356237f;

	r->SpriteTexture( skyTextures[face], 0 );

	r->Begin( TRI_QUADS );

	for ( int i = 0; i < 4; i++ )
	{
		r->TexCoord2f( uvCoords[i][0], uvCoords[i][1] );
		r->Vertex3fv( skyCubeVertices[indices[i]] * radius );
	}

	r->End();
}

void SimpleSky::RenderRotatedSide( triangleapi_t* r, const int& face )
{
	int* indices = skyCubeFaces[face];

	r->SpriteTexture( skyTextures[face], 0 );

	r->Begin( TRI_QUADS );

	for ( int i = 0; i < 4; i++ )
	{
		int* angles = angleOffsets[indices[i]];
		float yaw = angles[0] + currentRotationAngle;
		float pitch = angles[1];
		float radius = skyDistance / sqrt( 2.0f );

		float toRad = 3.14159f / 180.0f;

		float x = cos( yaw * toRad ) * sin( pitch * toRad ) * radius;
		float y = sin( yaw * toRad ) * sin( pitch * toRad ) * radius;
		float z = (pitch > 0) ? radius : -radius; // It's usually cos(pitch), but we're only rotating the yaw
		z /= sqrt( 2.0f );
		z *= 0.70710678118f;

		r->TexCoord2f( uvCoords[i][0], uvCoords[i][1] );
		r->Vertex3f( x, y, z );
	}

	r->End();
}

void SimpleSky::SetParameters( const SimpleSky::SkyParameters& skyParams )
{
	skyFlags = skyParams.skyFlags;
	rotationSpeed = skyParams.rotationSpeed;
	wobbleFrequency = skyParams.wobbleFrequency;
	wobbleScale = skyParams.wobbleScale;
	skyDistance = skyParams.skyDistance;

	if ( strlen( skyParams.skyName ) >= 31 )
	{
		gEngfuncs.Con_Printf( "ERROR: sky name is too long! (%i is larger than 31)\nUsing \"dusk\" skybox", strlen( skyParams.skyName ) );
		sprintf( skyName, "dusk" );
	}
	else 
	{
		sprintf( skyName, skyParams.skyName );

		UpdateSkyTexture();
	}
}

void SimpleSky::UpdateSkyCube()
{
	static float wobblingCycle = 0.0f;
	static float squeezingCycle = 0.0f;

	wobblingCycle += wobbleFrequency * 0.016;
	squeezingCycle += wobbleFrequency * 0.016;

	currentRotationAngle += rotationSpeed * 0.016;

	if ( wobblingCycle > 2 * 3.14159 )
		wobblingCycle -= 2 * 3.14159;
	
	if ( squeezingCycle > 2 * 3.14159 )
		squeezingCycle -= 2 * 3.14159;

	ResetSkyCube();

	if ( skyFlags & SkyFlag_Squeeze )
	{
		SqueezeSkyCube( squeezingCycle );
	}

	if ( skyFlags & SkyFlag_Wobble )
	{
		WobbleSkyCube( wobblingCycle );
	}
}

void SimpleSky::WobbleSkyCube( const float& cycle )
{
	float radius = (wobbleScale / skyDistance);

	for ( int i = 0; i < 7; i++ )
	{
		float& x = skyCubeVertices[i].x;
		float& y = skyCubeVertices[i].y;
		float& z = skyCubeVertices[i].z;

		skyCubeVertices[i].x += sin( cycle * x * y ) * cos( z * cycle * 2.0f ) * radius;
		skyCubeVertices[i].y += cos( cycle * y * z ) * cos( y * cycle * 2.0f ) * radius;
		skyCubeVertices[i].z += sin( cycle * z * x ) * sin( z * cycle * 2.0f ) * radius;
	}
}

void SimpleSky::SqueezeSkyCube( const float& cycle )
{
	float radius = wobbleScale / skyDistance;

	for ( int i = 0; i < 7; i++ )
	{
		if ( i < 4 )
		{
			skyCubeVertices[i].x *= 1.0f + sin( cycle ) * radius;
			skyCubeVertices[i].y *= 1.0f + sin( cycle ) * radius;
		}

		else
		{
			skyCubeVertices[i].x *= 1.0f - sin( cycle ) * radius;
			skyCubeVertices[i].y *= 1.0f - sin( cycle ) * radius;
		}
	}
}

void SimpleSky::ResetSkyCube()
{
	memcpy( skyCubeVertices, skyCubeReferenceVertices, sizeof( Vector ) * 8 );
	return;

	/*
	for ( int i = 0; i < 7; i++ )
	{
		// X axis pattern: 
		//  1 -1 -1  1  1 -1 -1  1
		//  0  1  2  3  4  5  6  7
		if ( i == 0 )
			skyCubeVertices[i].x = 1;
		else if ( (((i + 1)/2) % 2) )
			skyCubeVertices[i].x = -1;
		else 
			skyCubeVertices[i].x = 1;

		// Y axis pattern:
		// 1  1 -1 -1  1  1 -1 -1
		// 0  1  2  3  4  5  6  7
		if ( i <= 1 )
			skyCubeVertices[i].y = 1;
		else if ( (i / 2) % 2 )
			skyCubeVertices[i].y = -1;
		else
			skyCubeVertices[i].y = 1;

		// Z axis pattern:
		// 1  1  1  1 -1 -1 -1 -1
		// 0  1  2  3  4  5  6  7
		if ( i < 4 )
			skyCubeVertices[i].z = -1;
		else 
			skyCubeVertices[i].z = 1;
	}*/
}

void SimpleSky::UpdateSkyTexture()
{
	// A little note about textures:
	// Sprites that are larger than 128x128 will get subdivided
	// So for example, a 256x256 sprite will be four 128x128 sprites
	// In this case, sky textures
	// So for now, this only serves for demonstration purposes
	// and I wouldn't exactly recommend you to use it in a project - yet 

	for ( int i = 0; i < 6; i++ )
	{
		char textureName[64];
		sprintf( textureName, "gfx/env/%s%s.tga", skyName, skySides[i] );
		skyTextures[i] = gEngfuncs.LoadMapSprite( textureName );

		if ( skyTextures[i] == nullptr )
		{
			gEngfuncs.Con_Printf( "ERROR: can't load sky texture %s!\n", textureName );
		}
	}
}
