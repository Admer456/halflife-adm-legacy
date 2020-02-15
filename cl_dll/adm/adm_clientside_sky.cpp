/*

======= adm_clientside_sky.cpp ===========================

	1st November 2019
	Custom sky rendering
	Started off as a bet with Step4enko saying "You can't implement trigger_changesky"
	Time to prove him wrong, lol

*/

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

#include "triangleapi.h"

extern vec3_t v_origin;

// cube orientation
enum CubeOrientation
{
	cBLD, cBRD, cFLD, cFRD,
	cBLU, cBRU, cFLU, cFRU
};

enum CubeOrientationDiv2
{
	BLD, BCD, BRD,
	CLD, CCD, CRD,
	FLD, FCD, FRD,

	BLC, BCC, BRC,
	CLC, CCC, CRC,
	FLC, FCC, FRC,

	BLU, BCU, BRU,
	CLU, CCU, CRU,
	FLU, FCU, FRU
};

enum PlaneTexCoords
{
	BottomRight,	BottomLeft,
	TopLeft,		TopRight
};

const char *szSkyExt[ 6 ] = { "ft", "bk", "lf", "rt", "up", "dn" };

float texCoords[ 4 ][ 2 ] =
{
	{ 1.0f, 1.0f },		// bottom-right
	{ 0.0f, 1.0f },		// bottom-left
	{ 0.0f, 0.0f },		// top-left
	{ 1.0f, 0.0f }		// top-right
};

Vector CubeVertsDiv2[ 27 ] =
{
	Vector( -1, -1, -1 ),	// back		left	down
	Vector( 0, -1, -1 ),	// back		centre	down
	Vector( 1, -1, -1 ),	// back		right	down

	Vector( -1, 0, -1 ),	// centre	left	down
	Vector( 0, 0, -1 ),		// centre	centre	down
	Vector( 1, 0, -1 ),		// centre	right	down

	Vector( -1, 1, -1 ),	// front	left	down
	Vector( 0, 1, -1 ),		// front	centre	down
	Vector( 1, 1, -1 ),		// front	right	down

	Vector( -1, -1, 0 ),	// back		left	centre
	Vector( 0, -1, 0 ),		// back		centre	centre
	Vector( 1, -1, 0 ),		// back		right	centre

	Vector( -1, 0, 0 ),		// centre	left	centre
	Vector( 0, 0, 0 ),		// centre	centre	centre
	Vector( 1, 0, 0 ),		// centre	right	centre

	Vector( -1, 1, 0 ),		// front	left	centre
	Vector( 0, 1, 0 ),		// front	centre	centre
	Vector( 1, 1, 0 ),		// front	right	centre

	Vector( -1, -1, 1 ),	// back		left	up
	Vector( 0, -1, 1 ),		// back		centre	up
	Vector( 1, -1, 1 ),		// back		right	up
												
	Vector( -1, 0, 1 ),		// centre	left	up
	Vector( 0, 0, 1 ),		// centre	centre	up
	Vector( 1, 0, 1 ),		// centre	right	up
												
	Vector( -1, 1, 1 ),		// front	left	up
	Vector( 0, 1, 1 ),		// front	centre	up
	Vector( 1, 1, 1 ),		// front	right	up
};

// all verts must follow the same path: bottom-right to bottom-left, to top-left to top-right
float *cubeQuadsDiv2[ 24 ][ 4 ] = { 0 };

void UpdateQuadsDiv2( Vector vecs[27] )
{
	// front: bottom-left corner
	cubeQuadsDiv2[ 0 ][ 0 ] = vecs[ FCD ];
	cubeQuadsDiv2[ 0 ][ 1 ] = vecs[ FLD ];
	cubeQuadsDiv2[ 0 ][ 2 ] = vecs[ FLC ];
	cubeQuadsDiv2[ 0 ][ 3 ] = vecs[ FCC ];
	// bottom-right corner
	cubeQuadsDiv2[ 1 ][ 0 ] = vecs[ FRD ];
	cubeQuadsDiv2[ 1 ][ 1 ] = vecs[ FCD ];
	cubeQuadsDiv2[ 1 ][ 2 ] = vecs[ FCC ];
	cubeQuadsDiv2[ 1 ][ 3 ] = vecs[ FRC ];
	// top-left corner
	cubeQuadsDiv2[ 2 ][ 0 ] = vecs[ FCC ];
	cubeQuadsDiv2[ 2 ][ 1 ] = vecs[ FLC ];
	cubeQuadsDiv2[ 2 ][ 2 ] = vecs[ FLU ];
	cubeQuadsDiv2[ 2 ][ 3 ] = vecs[ FCU ];
	// top-right corner
	cubeQuadsDiv2[ 3 ][ 0 ] = vecs[ FRC ];
	cubeQuadsDiv2[ 3 ][ 1 ] = vecs[ FCC ];
	cubeQuadsDiv2[ 3 ][ 2 ] = vecs[ FCU ];
	cubeQuadsDiv2[ 3 ][ 3 ] = vecs[ FRU ];

	// back: bottom-left corner
	cubeQuadsDiv2[ 4 ][ 0 ] = vecs[ BCD ];
	cubeQuadsDiv2[ 4 ][ 1 ] = vecs[ BRD ];
	cubeQuadsDiv2[ 4 ][ 2 ] = vecs[ BRC ];
	cubeQuadsDiv2[ 4 ][ 3 ] = vecs[ BCC ];
	// bottom-right corner
	cubeQuadsDiv2[ 5 ][ 0 ] = vecs[ BLD ];
	cubeQuadsDiv2[ 5 ][ 1 ] = vecs[ BCD ];
	cubeQuadsDiv2[ 5 ][ 2 ] = vecs[ BCC ];
	cubeQuadsDiv2[ 5 ][ 3 ] = vecs[ BLC ];
	// top-left corner
	cubeQuadsDiv2[ 6 ][ 0 ] = vecs[ BCC ];
	cubeQuadsDiv2[ 6 ][ 1 ] = vecs[ BRC ];
	cubeQuadsDiv2[ 6 ][ 2 ] = vecs[ BRU ];
	cubeQuadsDiv2[ 6 ][ 3 ] = vecs[ BCU ];
	// top-right corner
	cubeQuadsDiv2[ 7 ][ 0 ] = vecs[ BLC ];
	cubeQuadsDiv2[ 7 ][ 1 ] = vecs[ BCC ];
	cubeQuadsDiv2[ 7 ][ 2 ] = vecs[ BCU ];
	cubeQuadsDiv2[ 7 ][ 3 ] = vecs[ BLU ];

	// right: bottom-left corner	
	cubeQuadsDiv2[ 8 ][ 0 ] = vecs[ CRD ];
	cubeQuadsDiv2[ 8 ][ 1 ] = vecs[ FRD ];
	cubeQuadsDiv2[ 8 ][ 2 ] = vecs[ FRC ];
	cubeQuadsDiv2[ 8 ][ 3 ] = vecs[ CRC ];
	// bottom-right corner
	cubeQuadsDiv2[ 9 ][ 0 ] = vecs[ BRD ];
	cubeQuadsDiv2[ 9 ][ 1 ] = vecs[ CRD ];
	cubeQuadsDiv2[ 9 ][ 2 ] = vecs[ CRC ];
	cubeQuadsDiv2[ 9 ][ 3 ] = vecs[ BRC ];
	// top-left corner		
	cubeQuadsDiv2[ 10 ][ 0 ] = vecs[ CRC ];
	cubeQuadsDiv2[ 10 ][ 1 ] = vecs[ FRC ];
	cubeQuadsDiv2[ 10 ][ 2 ] = vecs[ FRU ];
	cubeQuadsDiv2[ 10 ][ 3 ] = vecs[ CRU ];
	// top-right corner		
	cubeQuadsDiv2[ 11 ][ 0 ] = vecs[ BRC ];
	cubeQuadsDiv2[ 11 ][ 1 ] = vecs[ CRC ];
	cubeQuadsDiv2[ 11 ][ 2 ] = vecs[ CRU ];
	cubeQuadsDiv2[ 11 ][ 3 ] = vecs[ BRU ];

	// left: bottom-left corner
	cubeQuadsDiv2[ 12 ][ 0 ] = vecs[ CLD ];
	cubeQuadsDiv2[ 12 ][ 1 ] = vecs[ BLD ];
	cubeQuadsDiv2[ 12 ][ 2 ] = vecs[ BLC ];
	cubeQuadsDiv2[ 12 ][ 3 ] = vecs[ CLC ];
	// bottom-right corner					
	cubeQuadsDiv2[ 13 ][ 0 ] = vecs[ FLD ];
	cubeQuadsDiv2[ 13 ][ 1 ] = vecs[ CLD ];
	cubeQuadsDiv2[ 13 ][ 2 ] = vecs[ CLC ];
	cubeQuadsDiv2[ 13 ][ 3 ] = vecs[ FLC ];
	// top-left corner						
	cubeQuadsDiv2[ 14 ][ 0 ] = vecs[ CLC ];
	cubeQuadsDiv2[ 14 ][ 1 ] = vecs[ BLC ];
	cubeQuadsDiv2[ 14 ][ 2 ] = vecs[ BLU ];
	cubeQuadsDiv2[ 14 ][ 3 ] = vecs[ CLU ];
	// top-right corner					
	cubeQuadsDiv2[ 15 ][ 0 ] = vecs[ FLC ];
	cubeQuadsDiv2[ 15 ][ 1 ] = vecs[ CLC ];
	cubeQuadsDiv2[ 15 ][ 2 ] = vecs[ CLU ];
	cubeQuadsDiv2[ 15 ][ 3 ] = vecs[ FLU ];
						
	// up: bottom-left corner
	cubeQuadsDiv2[ 16 ][ 0 ] = vecs[ CLU ];
	cubeQuadsDiv2[ 16 ][ 1 ] = vecs[ BLU ];
	cubeQuadsDiv2[ 16 ][ 2 ] = vecs[ BCU ];
	cubeQuadsDiv2[ 16 ][ 3 ] = vecs[ CCU ];
	// bottom-right corner	
	cubeQuadsDiv2[ 17 ][ 0 ] = vecs[ FLU ];
	cubeQuadsDiv2[ 17 ][ 1 ] = vecs[ CLU ];
	cubeQuadsDiv2[ 17 ][ 2 ] = vecs[ CCU ];
	cubeQuadsDiv2[ 17 ][ 3 ] = vecs[ FCU ];
	// top-left corner
	cubeQuadsDiv2[ 18 ][ 0 ] = vecs[ CCU ];
	cubeQuadsDiv2[ 18 ][ 1 ] = vecs[ BCU ];
	cubeQuadsDiv2[ 18 ][ 2 ] = vecs[ BRU ];
	cubeQuadsDiv2[ 18 ][ 3 ] = vecs[ CRU ];
	// top-right corner	
	cubeQuadsDiv2[ 19 ][ 0 ] = vecs[ FCU ];
	cubeQuadsDiv2[ 19 ][ 1 ] = vecs[ CCU ];
	cubeQuadsDiv2[ 19 ][ 2 ] = vecs[ CRU ];
	cubeQuadsDiv2[ 19 ][ 3 ] = vecs[ FRU ];
							  
	// down: bottom-left corner
	cubeQuadsDiv2[ 20 ][ 0 ] = vecs[ CLD ];
	cubeQuadsDiv2[ 20 ][ 1 ] = vecs[ BLD ];
	cubeQuadsDiv2[ 20 ][ 2 ] = vecs[ BCD ];
	cubeQuadsDiv2[ 20 ][ 3 ] = vecs[ CCD ];
	// bottom-right corner
	cubeQuadsDiv2[ 21 ][ 0 ] = vecs[ FLD ];
	cubeQuadsDiv2[ 21 ][ 1 ] = vecs[ CLD ];
	cubeQuadsDiv2[ 21 ][ 2 ] = vecs[ CCD ];
	cubeQuadsDiv2[ 21 ][ 3 ] = vecs[ FCD ];
	// top-left corner	
	cubeQuadsDiv2[ 22 ][ 0 ] = vecs[ CCD ];
	cubeQuadsDiv2[ 22 ][ 1 ] = vecs[ BCD ];
	cubeQuadsDiv2[ 22 ][ 2 ] = vecs[ BRD ];
	cubeQuadsDiv2[ 22 ][ 3 ] = vecs[ CRD ];
	// top-right cor
	cubeQuadsDiv2[ 23 ][ 0 ] = vecs[ FCD ];
	cubeQuadsDiv2[ 23 ][ 1 ] = vecs[ CCD ];
	cubeQuadsDiv2[ 23 ][ 2 ] = vecs[ CRD ];
	cubeQuadsDiv2[ 23 ][ 3 ] = vecs[ FRD ];
}

void UpdateVerticesWithNewRadius( Vector vecs[ 27 ], float radius )
{
	for ( int i = 0; i < 27; i++ )
		vecs[ i ] = CubeVertsDiv2[ i ] * radius;
}

void UpdateSkyTextures( model_t *sprites[ 6 ], char *SkyName )
{
	for ( int i = 0; i < 6; i++ )
	{
		char szSkyName[ 64 ];
		sprintf( szSkyName, "gfx/env/%s%s.tga", SkyName, szSkyExt[ i ] );
		sprites[ i ] = gEngfuncs.LoadMapSprite( szSkyName );
	}
}

DECLARE_MESSAGE( m_clSky, SkyChange );

void CClientSkyRenderer::InitExtension()
{
	HOOK_MESSAGE( SkyChange );

	m_iFlags &= ~(HUD_ACTIVE);
}

int CClientSkyRenderer::VidInit()
{
	float radius = 8192.f;
	m_flSkyRadius = radius;
	m_skySize = 8;

	// initialise the cube
	m_rgCubeVerts[ cBLD ] = Vector( -m_flSkyRadius, -m_flSkyRadius, -m_flSkyRadius );
	m_rgCubeVerts[ cBRD ] = Vector( m_flSkyRadius,	-m_flSkyRadius, -m_flSkyRadius );
	m_rgCubeVerts[ cFLD ] = Vector( -m_flSkyRadius, m_flSkyRadius,	-m_flSkyRadius );
	m_rgCubeVerts[ cFRD ] = Vector( m_flSkyRadius,	m_flSkyRadius,	-m_flSkyRadius );

	m_rgCubeVerts[ cBLU ] = Vector( -m_flSkyRadius, -m_flSkyRadius, m_flSkyRadius );
	m_rgCubeVerts[ cBRU ] = Vector( m_flSkyRadius,	-m_flSkyRadius, m_flSkyRadius );
	m_rgCubeVerts[ cFLU ] = Vector( -m_flSkyRadius, m_flSkyRadius,	m_flSkyRadius );
	m_rgCubeVerts[ cFRU ] = Vector( m_flSkyRadius,	m_flSkyRadius,	m_flSkyRadius );

	for ( int i = 0; i < 27; i++ )
		m_rgCubeVertsDiv2[ i ] = CubeVertsDiv2[ i ] * m_flSkyRadius;

	// initialise cube angles
	m_rgCubeAngles[ cBLD ] = Vector2D( -135, -45 );
	m_rgCubeAngles[ cBRD ] = Vector2D( -45,	-45 );
	m_rgCubeAngles[ cFLD ] = Vector2D( 135,	-45 );
	m_rgCubeAngles[ cFRD ] = Vector2D( 45,	-45 );

	m_rgCubeAngles[ cBLU ] = Vector2D( -135, 45 );
	m_rgCubeAngles[ cBRU ] = Vector2D( -45,	45 );
	m_rgCubeAngles[ cFLU ] = Vector2D( 135,	45 );
	m_rgCubeAngles[ cFRU ] = Vector2D( 45,	45 );

	//for ( int i = 0; i < 6; i++ )
	//{
	//	char szSkyName[ 64 ];
	//	sprintf( szSkyName, "gfx/env/2desert%s.tga", szSkyExt[ i ] );
	//	m_skySprites[ i ] = gEngfuncs.LoadMapSprite( szSkyName );
	//}

	for ( int i = 0; i < 8; i++ )
	{
		m_rgCubeAnglesTotal[ i ] = m_rgCubeAngles[ i ];
		m_rgCubeAnglesAdded[ i ] = Vector2D( 0, 0 );
	}

	return 1;
}

#define deg2rad 0.0174533

void CClientSkyRenderer::Think()
{
	static float flSkyTimer = 0.0f;

	static float flWobbleSky = 0.0f;
	static float flIncrementor = 0.004;
	
	flWobbleSky += flIncrementor;
	flSkyTimer += 0.02;

	if ( flWobbleSky > 3.14 )
		flWobbleSky = 0;

	for ( int i = 0; i < 8; i++ )
	{
		m_rgCubeAnglesAdded[ i ].x = flWobbleSky * 2 / deg2rad;
		m_rgCubeAnglesTotal[ i ] = m_rgCubeAngles[ i ] + m_rgCubeAnglesAdded[ i ];

	//	m_rgCubeVerts[ i ] = v_origin;
	//	m_rgCubeVerts[ i ].x += m_flSkyRadius * cos( deg2rad * m_rgCubeAnglesTotal[ i ].x ) * cos( deg2rad * m_rgCubeAnglesTotal[ i ].y );
	//	m_rgCubeVerts[ i ].y += m_flSkyRadius * sin( deg2rad * m_rgCubeAnglesTotal[ i ].x ) * cos( deg2rad * m_rgCubeAnglesTotal[ i ].y );
	//	m_rgCubeVerts[ i ].z += (m_flSkyRadius/1.41) * sin( deg2rad * m_rgCubeAnglesTotal[ i ].y );
	}
}

#undef deg2rad

int CClientSkyRenderer::Draw( float flTime )
{
	return 1;
}

int CClientSkyRenderer::Render( triangleapi_s *pRender )
{
	if ( !(m_iFlags & HUD_ACTIVE) )
		return 0;

	float *quads[ 6 ][ 4 ] =
	{
		// front
		{
			m_rgCubeVerts[ cFLD ],
			m_rgCubeVerts[ cFRD ],
			m_rgCubeVerts[ cFRU ],
			m_rgCubeVerts[ cFLU ]
		},

		// back
		{
			m_rgCubeVerts[ cBRD ],
			m_rgCubeVerts[ cBLD ],
			m_rgCubeVerts[ cBLU ],
			m_rgCubeVerts[ cBRU ]
		},

		// left
		{
			m_rgCubeVerts[ cBLD ],
			m_rgCubeVerts[ cFLD ],
			m_rgCubeVerts[ cFLU ],
			m_rgCubeVerts[ cBLU ],
		},

		// right
		{
			m_rgCubeVerts[ cFRD ],
			m_rgCubeVerts[ cBRD ],
			m_rgCubeVerts[ cBRU ],
			m_rgCubeVerts[ cFRU ]
		},

		// top
		{
			m_rgCubeVerts[ cFRU ],
			m_rgCubeVerts[ cBRU ],
			m_rgCubeVerts[ cBLU ],
			m_rgCubeVerts[ cFLU ],
		},

		// bottom
		{
			m_rgCubeVerts[ cFRD ],
			m_rgCubeVerts[ cBRD ],
			m_rgCubeVerts[ cBLD ],
			m_rgCubeVerts[ cFLD ],
		}
	};

	if ( m_skySize < 2 )
	{
		for ( int i = 0; i < 6; i++ )
		{
			pRender->SpriteTexture( m_skySprites[ i ], 0 );
			pRender->CullFace( TRI_NONE );
			pRender->RenderMode( kRenderNormal );

			pRender->Begin( TRI_QUADS );
			pRender->Color4ub( 255, 255, 255, 255 );

			// bottom-right
			pRender->TexCoord2f( texCoords[ BottomRight ][ 0 ], texCoords[ BottomRight ][ 1 ] );
			pRender->Vertex3fv( quads[ i ][ BottomRight ] );

			// bottom-left
			pRender->TexCoord2f( texCoords[ BottomLeft ][ 0 ], texCoords[ BottomLeft ][ 1 ] );
			pRender->Vertex3fv( quads[ i ][ BottomLeft ] );

			// top-left
			pRender->TexCoord2f( texCoords[ TopLeft ][ 0 ], texCoords[ TopLeft ][ 1 ] );
			pRender->Vertex3fv( quads[ i ][ TopLeft ] );

			// top-right
			pRender->TexCoord2f( texCoords[ TopRight ][ 0 ], texCoords[ TopRight ][ 1 ] );
			pRender->Vertex3fv( quads[ i ][ TopRight ] );

			pRender->End();
		}
	}

	else if ( m_skySize == 2 )
	{
		UpdateVerticesWithNewRadius( m_rgCubeVertsDiv2, m_flSkyRadius );
		UpdateQuadsDiv2( m_rgCubeVertsDiv2 );

		// bottom-right corner
			
		for ( int i = 0; i < 6; i++ )
		{
			pRender->SpriteTexture( m_skySprites[ i ], 2 );
			pRender->CullFace( TRI_NONE );
			pRender->RenderMode( kRenderNormal );

			pRender->Begin( TRI_QUADS );
			pRender->Color4ub( 255, 255, 255, 255 );
			// bottom-right
			pRender->TexCoord2f( texCoords[ BottomRight ][ 0 ], texCoords[ BottomRight ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 ][ BottomRight ] );

			// bottom-left
			pRender->TexCoord2f( texCoords[ BottomLeft ][ 0 ], texCoords[ BottomLeft ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 ][ BottomLeft ] );

			// top-left
			pRender->TexCoord2f( texCoords[ TopLeft ][ 0 ], texCoords[ TopLeft ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 ][ TopLeft ] );

			// top-right
			pRender->TexCoord2f( texCoords[ TopRight ][ 0 ], texCoords[ TopRight ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 ][ TopRight ] );
			pRender->End();
		}

		// bottom-left corner

		for ( int i = 0; i < 6; i++ )
		{
			pRender->SpriteTexture( m_skySprites[ i ], 3 );
			pRender->CullFace( TRI_NONE );
			pRender->RenderMode( kRenderNormal );

			pRender->Begin( TRI_QUADS );
			pRender->Color4ub( 255, 255, 255, 255 );
			// bottom-right
			pRender->TexCoord2f( texCoords[ BottomRight ][ 0 ], texCoords[ BottomRight ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 1 ][ BottomRight ] );

			// bottom-left
			pRender->TexCoord2f( texCoords[ BottomLeft ][ 0 ], texCoords[ BottomLeft ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 1 ][ BottomLeft ] );

			// top-left
			pRender->TexCoord2f( texCoords[ TopLeft ][ 0 ], texCoords[ TopLeft ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 1 ][ TopLeft ] );

			// top-right
			pRender->TexCoord2f( texCoords[ TopRight ][ 0 ], texCoords[ TopRight ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 1 ][ TopRight ] );
			pRender->End();
		}

		// top-right corner

		for ( int i = 0; i < 6; i++ )
		{
			pRender->SpriteTexture( m_skySprites[ i ], 0 );
			pRender->CullFace( TRI_NONE );
			pRender->RenderMode( kRenderNormal );

			pRender->Begin( TRI_QUADS );
			pRender->Color4ub( 255, 255, 255, 255 );
			// bottom-right
			pRender->TexCoord2f( texCoords[ BottomRight ][ 0 ], texCoords[ BottomRight ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 2 ][ BottomRight ] );

			// bottom-left
			pRender->TexCoord2f( texCoords[ BottomLeft ][ 0 ], texCoords[ BottomLeft ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 2 ][ BottomLeft ] );

			// top-left
			pRender->TexCoord2f( texCoords[ TopLeft ][ 0 ], texCoords[ TopLeft ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 2 ][ TopLeft ] );

			// top-right
			pRender->TexCoord2f( texCoords[ TopRight ][ 0 ], texCoords[ TopRight ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 2 ][ TopRight ] );
			pRender->End();
		}

		// top-left corner
		
		for ( int i = 0; i < 6; i++ )
		{
			pRender->SpriteTexture( m_skySprites[ i ], 1 );
			pRender->CullFace( TRI_NONE );
			pRender->RenderMode( kRenderNormal );

			pRender->Begin( TRI_QUADS );
			pRender->Color4ub( 255, 255, 255, 255 );
			// bottom-right
			pRender->TexCoord2f( texCoords[ BottomRight ][ 0 ], texCoords[ BottomRight ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 3 ][ BottomRight ] );

			// bottom-left
			pRender->TexCoord2f( texCoords[ BottomLeft ][ 0 ], texCoords[ BottomLeft ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 3 ][ BottomLeft ] );

			// top-left
			pRender->TexCoord2f( texCoords[ TopLeft ][ 0 ], texCoords[ TopLeft ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 3 ][ TopLeft ] );

			// top-right
			pRender->TexCoord2f( texCoords[ TopRight ][ 0 ], texCoords[ TopRight ][ 1 ] );
			pRender->Vertex3fv( cubeQuadsDiv2[ i * 4 + 3 ][ TopRight ] );
			pRender->End();
		}
	}

	return 1;
}

void CClientSkyRenderer::Reset()
{

}

int CClientSkyRenderer::MsgFunc_SkyChange( const char* pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	// TGA can be loaded in powers of two:
	// 6 -> 64
	// 7 -> 128 // 1 plane per side
	// 8 -> 256 // 4 planes per side
	// 9 -> 512 // 8 planes per side
	// If you go over 128x128, then the model_t sent by MapLoadSprite will get 
	// divided into chunks, where each frame represents a quarter of the image.

	// Unpack the 2-in-1 variable
	unsigned int iPackedMessage = READ_BYTE();
	m_flSkyRadius = (1 << (iPackedMessage / 4)) * 8;
	m_skySize = iPackedMessage & 7;

	// Just the sky name, e.g. 2desert, to save up on bandwidth
	m_szSkyName = READ_STRING();

	UpdateSky();

	return 1;
}

void CClientSkyRenderer::UpdateSky()
{
	if ( !(m_iFlags & HUD_ACTIVE) )
		m_iFlags |= HUD_ACTIVE;

	for ( int i = 0; i < 6; i++ )
	{
		char szSkyName[ 64 ];
		sprintf( szSkyName, "gfx/env/%s%s.tga", m_szSkyName, szSkyExt[ i ] );
		m_skySprites[ i ] = gEngfuncs.LoadMapSprite( szSkyName );
	}
}