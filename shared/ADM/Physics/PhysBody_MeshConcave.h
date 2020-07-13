#pragma once

/*
========================================================

	CPhysObjLoader

	Loads model data from an .obj

========================================================
*/
struct CPhysObjLoader
{
	btTriangleIndexVertexArray* vertexArray;
	ConvexDecomposition::WavefrontObj loader;

	void						LoadObjMesh( const char* meshPath, float scale );
};

class IPhysBody;

/*
========================================================

	CPhysBody_ConcaveMesh

	Most expensive physics class

========================================================
*/
class CPhysBody_ConcaveMesh : public CPhysBody
{
public:
	CPhysBody_ConcaveMesh() = default;
	~CPhysBody_ConcaveMesh() = default;
	
	void						InitShape( const PhysParameters& params ) override;

protected:
	void						LoadTriangleMeshShape( const char* meshPath, float scale );
	CPhysObjLoader				objLoader;
};