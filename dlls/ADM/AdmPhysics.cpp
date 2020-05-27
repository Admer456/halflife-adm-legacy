/*

======= adm_physics.cpp ===========================

	17th January 2019
	Physics API.

	The 2nd largest beast that might've struck my mod.
	GoldSrc itself has okay physics which date back to
	the Quake engine, but, I clearly need more than that.

	Here are the planned physical entities:
	physics_box
	physics_cylinder
	physics_sphere
	physics_vertex

	Based on these, we will be able to create more complex
	physical structures, and hopefully, something that won't
	interfere with GoldSrc's physics.

	18th January 2019
	I might use ReactPhysics3D for this. Awaiting results...

|||||||||||||||||||||||||||||||||||||||||||||||||||
	________________________
	19th January 2019
	________________________
	AdmSrc Physics Interface
	________________________
	Source file
	________________________
|||||||||||||||||||||||||||||||||||||||||||||||||||

	20th January 2019

	Bullet has a BSP importer. Quake III BSP importer.
	But still. :D

	I wonder if this thing is even gonna get built, LOL.
	Especially considering the fact that this is a v141_xp
	toolset, LOOOOOL.

	edit: HOLY SHIT, it works!
	________________________
	12th June 2019
	________________________

	I just realised, I need Bullet physics for my vehicle system. Gonna do it riiiight away. :3

*/

#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "AdmPhysics.h"
//#include "../utils/obj/OBJ_Loader.h"

string_t g_iszWorldModel; // e.g. mapname.bsp

// CAdmPhysicsBase a.k.a. AdmPhys
// class implementation

CPhysicsWorld::~CPhysicsWorld()
{
	delete physCollisionConfig;
	delete physDispatcher;
	delete overlappingPairCache;
	delete physSolver;
	delete world;

//	delete serializer;

	delete colonVertexArrays;
	delete physWorldShape;

	delete groundMS;
	delete groundBody;
}

void CPhysicsWorld::Init()
{
	physCollisionConfig = new btDefaultCollisionConfiguration();
	physDispatcher = new btCollisionDispatcher( physCollisionConfig );
	overlappingPairCache = new btDbvtBroadphase();
	physSolver = new btSequentialImpulseConstraintSolver();

	world = new btDiscreteDynamicsWorld(physDispatcher, overlappingPairCache, physSolver, physCollisionConfig);

	if (physCollisionConfig)
		ALERT(at_console, "AdmPhys: Collision config initialised\n");

	if (physDispatcher)
		ALERT(at_console, "AdmPhys: Dispatcher initialised\n");

	if (overlappingPairCache)
		ALERT(at_console, "AdmPhys: Broadphase initialised\n");

	if (physSolver)
		ALERT(at_console, "AdmPhys: Solver initialised\n");

	if (world)
	{
		ALERT(at_console, "AdmPhys: Dynamic world initialised\n");
		world->setGravity( btVector3( 0, -9.81, 0 ) );
	}
}

void CPhysicsWorld::AddRigidBody( btRigidBody* body )
{
	world->addRigidBody( body );
}

void CPhysicsWorld::AddCollisionShape( btCollisionShape* colshape )
{
	collisionShapes.push_back( colshape );
}

void CPhysicsWorld::StepSimulation( float deltaTime, int steps )
{
	world->stepSimulation( btScalar(deltaTime), steps );
}

void CPhysicsWorld::SetGravity(float x, float y, float z)
{
	world->setGravity( btVector3(x, z, y) ); // Just a quick reminder, Bullet uses a right-handed 3D coordinate system, meaning it's just like Minecraft. <-<
}

void CPhysicsWorld::SetGravity(float z)
{
	world->setGravity( btVector3( 0, z, 0 ) );
}

void CPhysicsWorld::CreateWorldCollision(const char* path)
{
	ALERT(at_console, "\nCreating world collision...");

	char szGame[16];
	std::string szPath = path;
	std::string strGame;

	g_engfuncs.pfnGetGameDir(szGame);
	strGame = szGame;
	strGame += "/"; // "adm/"

	szPath = strGame + szPath; // "adm/" "maps/map.bsp"
	szPath += ".obj"; // "adm/maps/map.bsp.obj" This is gonna work ;)

	auto fPhysMeshExists = objLoader.loadObj( szPath.c_str() );

	if (fPhysMeshExists)
		ALERT(at_console, "\nYAAAAAY, PhysMesh does exist! %i triangles!", fPhysMeshExists);
	else
		ALERT(at_console, "\nWoof. ;w;\nPhysMesh doesn't exist or I can't load it... sowwy Master. >w<");
	
	if ( !fPhysMeshExists )
		return;

	colonVertexArrays = new btTriangleIndexVertexArray(
		objLoader.mTriCount, objLoader.mIndices,
		3 * sizeof(int),
		objLoader.mVertexCount, objLoader.mVertices,
		3 * sizeof(float)
	);

	physWorldShape = new btBvhTriangleMeshShape( colonVertexArrays, true, btVector3( -12000, -12000, -4000 ), btVector3( 12000, 12000, 12000 ) );
	physWorldShape->setLocalScaling( 39.37007874f * btVector3( 1, 1, 1 ) );
	physWorldShape->setMargin( 0.125f );
	physWorldShape->buildOptimizedBvh();

	// I dunno, maybe this can be used to write to files?
	//serializer = new btDefaultSerializer( maxSerializeSize );
	
	//serializer->startSerialization();
	//physWorldShape->serializeSingleBvh( serializer );
	//serializer->finishSerialization();

	// Rotate the world properly, OBJ uses a different orientation
	btVector3 angles( 0, 90 * M_PI/180.0f, 0 );
	btQuaternion q;
	q.setEuler( angles.x(), angles.y(), angles.z() );

	groundTrans.setIdentity();
	groundTrans.setOrigin( btVector3( 0, 0, 0 ) );
	groundTrans.setRotation( q );

	btScalar mass( 0.0f );
	btVector3 localInertia( 0, 0, 0 );

	groundMS = new btDefaultMotionState( groundTrans );
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, groundMS, physWorldShape, localInertia);

	groundBody = new btRigidBody( rbInfo );

	collisionShapes.push_back( physWorldShape );
	world->addRigidBody( groundBody ); // assumption is that treating a static body as a rigid body is not good

	ALERT( at_console, "\nCreated world PhysMesh!\n" );
} 

// Utilities

// 1 unit = 39th of a metre
float utom(float units)
{
	// 39.37 is how many inches there are in 1 metre
	// 1 unit is about 1 inch
	return units / 39.37007874;
}

// 1 metre = 39.37 units
float mtou(float metres)
{
	return metres * 39.37007874;
}

// AdmPhys special entities and classes

class CBasePhysical : public CBaseEntity
{
public:
	void				Spawn(void);
	void				KeyValue(KeyValueData *pkvd);
	virtual void EXPORT SpawnThink( void );
	void EXPORT			PhysicalThink( void );
	
	int					ObjectCaps() override { return (FCAP_ACROSS_TRANSITION | FCAP_CONTINUOUS_USE); }

protected:
	btCollisionObject*	m_CollisionObject;
	btCollisionShape*	m_CollisionShape;
	btRigidBody*		m_RigidBody;
	btTransform			m_Trans;
	btDefaultMotionState* m_MotionState;
	btRigidBody*		m_Body;

	float				m_flMass;
	bool				m_fDynamic;

	btVector3			vecPos; 
	btVector3			vecAngles;
};

LINK_ENTITY_TO_CLASS( phys_base, CBasePhysical );

void CBasePhysical::Spawn(void)
{
	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink( &CBasePhysical::SpawnThink );
}

void EXPORT CBasePhysical::SpawnThink( void )
{
	// Create new collision shape, add it to the global ones
	m_CollisionShape = new btBoxShape( btVector3( btScalar( utom( 16.0 ) ), btScalar( utom( 16.0 ) ), btScalar( utom( 16.0 ) ) ) );
	g_Physics.AddCollisionShape( m_CollisionShape );

	// Set up transform and set the origin
	m_Trans.setIdentity();
	m_Trans.setOrigin( btVector3( utom( pev->origin.x ), utom( pev->origin.z ), utom( pev->origin.y ) ) );

	// The rigid body is dynamic if the mass is not 0
	btScalar mass( m_flMass );
	m_fDynamic = (mass != 0.f);
	btVector3 localInertia( 0, 0, 0 );

	if ( m_fDynamic )
		m_CollisionShape->calculateLocalInertia( mass, localInertia );

	// Using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	m_MotionState = new btDefaultMotionState( m_Trans );
	btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, m_MotionState, m_CollisionShape, localInertia );
	m_Body = new btRigidBody( rbInfo );

	// Add the rigid body to the world
	g_Physics.AddRigidBody( m_Body );

	// Set the default orientation
	vecPos = m_Body->getWorldTransform().getOrigin();
	btScalar ang_yaw, ang_pitch, ang_roll;
	m_Body->getWorldTransform().getRotation().getEulerZYX( ang_yaw, ang_pitch, ang_roll );
	vecAngles = { ang_yaw, ang_pitch, ang_roll };

	if ( m_fDynamic )
		SetThink( &CBasePhysical::PhysicalThink );
	else
		SetThink( NULL );

	pev->nextthink = gpGlobals->time + 0.016;
}

void CBasePhysical::KeyValue(KeyValueData *pkvd)
{
	if (KeyvalueToken(mass))
	{
		KeyvalueToFloat(m_flMass);
	}
	else
	{
		KeyvaluesFromBase(CBaseEntity);
	}
}

void EXPORT CBasePhysical::PhysicalThink(void)
{
	vecPos = m_Body->getWorldTransform().getOrigin();

	btScalar ang_yaw, ang_pitch, ang_roll;

	m_Body->getWorldTransform().getRotation().getEulerZYX(ang_yaw, ang_pitch, ang_roll);

	vecAngles = { ang_yaw, ang_pitch, ang_roll };

	// This is so sad. .w.
	pev->origin.x = mtou(vecPos.getX());
	pev->origin.y = mtou(vecPos.getZ());
	pev->origin.z = mtou(vecPos.getY());

	pev->angles.x = -vecAngles.getX();
	pev->angles.y = vecAngles.getY();
	pev->angles.z = -vecAngles.getZ();

	UTIL_Rad2Deg(pev->angles.x);
	UTIL_Rad2Deg(pev->angles.y);
	UTIL_Rad2Deg(pev->angles.z);

	UTIL_SetOrigin(pev, pev->origin);

	pev->nextthink = gpGlobals->time + (1.0f / 60.0f);
}

class CPhysicsStatic : public CBasePhysical
{
public:
	void EXPORT SpawnThink( void ) override;
};

void EXPORT CPhysicsStatic::SpawnThink(void)
{
	ALERT( at_console, "\nCPhysicsStatic::SpawnThink" );

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
//	SET_MODEL(ENT(pev), STRING(pev->model));

	m_CollisionShape = new btBoxShape(btVector3(btScalar(utom(256.0)), btScalar(utom(-4)), btScalar(utom(256.0))));
	g_Physics.AddCollisionShape(m_CollisionShape);

	m_Trans.setIdentity();
	m_Trans.setOrigin(btVector3(utom(pev->origin.x), utom(pev->origin.z + 1), utom(pev->origin.y)));

	//rigidbody is static, mass zero
	btScalar mass(0.0f);
	m_fDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	m_MotionState = new btDefaultMotionState(m_Trans);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_MotionState, m_CollisionShape, localInertia);
	m_Body = new btRigidBody(rbInfo);

	g_Physics.AddRigidBody(m_Body);
	SetThink(NULL);
	pev->nextthink = gpGlobals->time + 0.1;
}

LINK_ENTITY_TO_CLASS(phys_staticplane, CPhysicsStatic);

/*class CPhysicsBox : public CBasePhysical
{

};*/

class CPhysicsManager : public CPointEntity
{
public:
	void Spawn();
	void EXPORT PM_Think();
	void EXPORT PM_SpawnThink();
};

void CPhysicsManager::Spawn()
{
	if ( !g_Physics.HasWorldCollision() )
		g_Physics.CreateWorldCollision(STRING(g_iszWorldModel));
	
	pev->nextthink = gpGlobals->time + 1.0;
	SetThink(&CPhysicsManager::PM_SpawnThink);
}

void EXPORT CPhysicsManager::PM_SpawnThink()
{
	pev->nextthink = gpGlobals->time + 0.8;
	SetThink( &CPhysicsManager::PM_Think );
}

void EXPORT CPhysicsManager::PM_Think()
{
//	ALERT(at_console, "\nCPhysicsManager::PM_Think() time %f\tnextthink %f", gpGlobals->time, pev->nextthink);

	g_Physics.PostUpdate();
//	ALERT(at_console, "\nPhysMeshShape->postUpdate()");

//	if ( AdmPhysEngine.PhysMeshShape )
//		AdmPhysEngine.PhysMeshShape->updateBound();
//	ALERT(at_console, "\nPhysMeshShape->updateBound()");
//	if ( AdmPhysEngine.PhysMeshShape )
//		AdmPhysEngine.PhysMeshShape->postUpdate();
//	ALERT(at_console, "\nPhysMeshShape->postUpdate()");
	
	g_Physics.StepSimulation(1.0f / 60.0f, 256);
	ALERT(at_console, "\nWorld->stepSimulation()");

	pev->nextthink = gpGlobals->time + (1.0f / 60.0f);
}

LINK_ENTITY_TO_CLASS(phys_manager, CPhysicsManager);
