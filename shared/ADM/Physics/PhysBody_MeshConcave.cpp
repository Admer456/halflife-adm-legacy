#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#include "IPhysBody.h"
#include "PhysBody.h"
#include "PhysBody_MeshConcave.h"

/*
============================

	CPhysObjLoader

============================
*/
void CPhysObjLoader::LoadObjMesh( const char* meshPath, float scale )
{
	// Load the data from file
	auto tris = loader.loadObj( meshPath );

	if ( tris )
	{
		printf( "Loaded %i tris successfully!\n", tris );
	}
	else
	{
		printf( "Failed to load tris!\n" );
		vertexArray = nullptr;

		return;
	}

	if ( scale != 1.0f )
	{
		for ( int i = 0; i < loader.mVertexCount * 3; i++ )
		{
			loader.mVertices[i] *= scale;
		}
	}

	vertexArray = new btTriangleIndexVertexArray(
		loader.mTriCount, loader.mIndices,
		3 * sizeof( int ),
		loader.mVertexCount, loader.mVertices,
		3 * sizeof( float )
	);
}


/*
============================

	CPhysBody_ConcaveMesh

============================
*/
void CPhysBody_ConcaveMesh::InitShape( const PhysParameters& params )
{
	// Load the .obj file
	LoadTriangleMeshShape( params.meshPath, params.scale );
}

void CPhysBody_ConcaveMesh::LoadTriangleMeshShape( const char* path, float scale )
{
	// Try loading data from a mesh
	objLoader.LoadObjMesh( path, scale );

	if ( objLoader.vertexArray == nullptr )
	{
		printf( "Error, while loading vertices in %s\n", path ); return;
	}

	// Construct the shape
	shape = new btGImpactMeshShape( objLoader.vertexArray );
	
	auto meshShape = static_cast<btGImpactMeshShape*>(shape);
	
	meshShape->postUpdate();
	meshShape->updateBound();
}