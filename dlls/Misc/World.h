class CPhysBody_ConcaveMesh;

// this moved here from world.cpp, to allow classes to be derived from it
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================
class CWorld : public CBaseEntity
{
public:
	void					Spawn( void );
	void					Precache( void );
	
	void					KeyValue( KeyValueData *pkvd );
	void EXPORT				PhysicsSpawnThink( void );

	void					GetKeyValueCustom( char &szTheValue, char szTargetKeyName[64] );
	void					SetKeyValueCustom( char szTargetKeyName[64], char szTheValue[64] );

private:
	BOOL					m_fHasPhysics;

	CPhysBody_ConcaveMesh	physWorld;
	PhysParameters			physParams;
	CPhysManager*			physManager;
};
