/*

======= adm_clientside_world.cpp ===========================

	14th February 2019
	Worldspawn model manipulation

*/

#include "hud.h"
#include "cl_util.h"
#include "const.h"

#include "com_model.h"
#include "studio.h"

#include "entity_state.h"
#include "cl_entity.h"

#include "dlight.h"
#include "triangleapi.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

extern engine_studio_api_t IEngineStudio;

void CClientWorld::InitExtension()
{
	m_iFlags			|= HUD_ACTIVE;
	m_flWave			= 0;
	m_flWaveIterator	= 0.05;
}

int CClientWorld::VidInit()
{
	m_pWorld = gEngfuncs.GetEntityByIndex( 0 );
	return 1;
}

void CClientWorld::Think()
{
	//if ( gEngfuncs.GetClientTime() > 5.0 )
	//{
	//	model_t* &pModel = m_pWorld->model;
	//	medge_t currentEdge = pModel->edges[ 0 ];
	//
	//	pModel->vertexes[ currentEdge.v[0] ].position.z += m_flWave;
	//}

	m_flWave += m_flWaveIterator;

	if ( fabs(m_flWave) > 1.0 )
		m_flWaveIterator *= -1;
}

int CClientWorld::Render( triangleapi_s *pRender )
{
	model_t* pModel = IEngineStudio.GetModelByIndex( 1 );

	if ( pModel == NULL || 
		 pModel->type != modtype_t::mod_brush )
		return 0;

		// use hotglow, or any other sprite for the texture
//	gEngfuncs.pTriAPI->SpriteTexture( 
//		(struct model_s *)gEngfuncs.GetSpritePointer( SPR_Load( "sprites/hotglow.spr" ) ), 0 );

	pRender->Begin( TRI_LINES );
	pRender->CullFace( TRI_NONE );
	pRender->RenderMode( kRenderTransAdd );
	pRender->Color4ub( 255, 0, 0, 255 );

	if ( m_pWorld )
	{
		m_pWorld->model->vertexes[ pModel->edges[ 1 ].v[ 0 ] ].position.z += m_flWave;
	}
	else
	{
		m_pWorld = gEngfuncs.GetEntityByIndex( 0 );
	}

	if ( m_pWorld )
	{
		msurface_t* surf = m_pWorld->model->leafs[ 0 ].firstmarksurface[ 0 ];

		surf->polys[ 0 ].verts[ 0 ][ 2 ] += m_flWave;
	}

	mvertex_t vertices[2] =
	{
		pModel->vertexes[ pModel->edges[ 1 ].v[ 0 ] ],
		pModel->vertexes[ pModel->edges[ 1 ].v[ 1 ] ]
	};

	pRender->TexCoord2f( 0.4, 0.4 );
	pRender->Vertex3fv( vertices[ 0 ].position );
	pRender->Vertex3fv( vertices[ 1 ].position );

	pRender->End();

	return 1;
}

/*
void RenderWireFrame (void)
{
	// all valid model_s' stored in the engine start at 1 by this method, not 0,
	// and the world model is always the first model.  Thus, it's at index 1.
	model_s *pModel = IEngineStudio.GetModelByIndex (1);

	// make sure it's a valid model_s pointer and the model type is mod_brush
	if ((pModel == NULL) || (pModel->type != mod_brush))
		return;

	// now it shouldn't crash if we do this
	// we're simply drawing every single edge in the world
	gEngfuncs.pTriAPI->Begin (TRI_LINES);
	gEngfuncs.pTriAPI->CullFace (TRI_NONE); // don't cull
	gEngfuncs.pTriAPI->RenderMode (kRenderNormal);  // normal render mode
	gEngfuncs.pTriAPI->Color4ub (255, 255, 255, 255);   // white lines

	// for all edges in the map
	for (int i = 0; i < pModel->numedges; i++)
	{
		// get the current edge at index i
		medge_t currentEdge = pModel->edges[i];
		// the medge_t's simply store indexes into a master vertex (mvertex_t) list,
		// so get those two vertexes
		mvertex_t currentVertexes[2] =
		{
			pModel->vertexes[currentEdge.v[0]],
			pModel->vertexes[currentEdge.v[1]]
		};

		// now render this edge/line
		gEngfuncs.pTriAPI->Vertex3fv (currentVertexes[0].position);
		gEngfuncs.pTriAPI->Vertex3fv (currentVertexes[1].position);
	}

	gEngfuncs.pTriAPI->End ();  // we're done rendering
}

*/