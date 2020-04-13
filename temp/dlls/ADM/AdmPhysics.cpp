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

void CAdmPhysicsBase::Init()
{
	admp_CollisionConfiguration = new btDefaultCollisionConfiguration();
	admp_Dispatcher = new btCollisionDispatcher(admp_CollisionConfiguration);
	admp_overlappingPairCache = new btDbvtBroadphase();
	admp_Solver = new btSequentialImpulseConstraintSolver();

	admp_World = new btDiscreteDynamicsWorld(admp_Dispatcher, admp_overlappingPairCache, admp_Solver, admp_CollisionConfiguration);

	if (admp_CollisionConfiguration)
		ALERT(at_console, "AdmPhys: Collision config initialised\n");

	if (admp_Dispatcher)
		ALERT(at_console, "AdmPhys: Dispatcher initialised\n");

	if (admp_overlappingPairCache)
		ALERT(at_console, "AdmPhys: Broadphase initialised\n");

	if (admp_Solver)
		ALERT(at_console, "AdmPhys: Solver initialised\n");

	if (admp_World)
	{
		ALERT(at_console, "AdmPhys: Dynamic world initialised\n");
		admp_World->setGravity( btVector3( 0, -9.81, 0 ) );
	}
}

void CAdmPhysicsBase::SetGravity(float x, float y, float z)
{
	admp_World->setGravity( btVector3(x, z, y) ); // Just a quick reminder, Bullet uses a right-handed 3D coordinate system, meaning it's just like Minecraft. <-<
}

void CAdmPhysicsBase::SetGravity(float z)
{
	admp_World->setGravity( btVector3( 0, z, 0 ) );
}

void CAdmPhysicsBase::CreateWorldCollision(const char* path)
{
	ALERT(at_console, "\nCreating world collision...");

	std::string szPath = path;

	char szGame[16];
	g_engfuncs.pfnGetGameDir(szGame);
	std::string strGame = szGame;
	strGame += "/"; // "adm/"

	szPath = strGame + szPath; // "adm/" "maps/map.bsp"
	szPath += ".obj"; // "adm/maps/map.bsp.obj" This is gonna work ;)

	ConvexDecomposition::WavefrontObj objLoader;
	auto fPhysMeshExists = objLoader.loadObj(szPath.c_str());
//	ALERT(at_console, "\nfPhysMeshExists = %i", fPhysMeshExists);

	{ // Dev message
		if (fPhysMeshExists)
			ALERT(at_console, "\nYAAAAAY, PhysMesh does exist! %i triangles!", fPhysMeshExists);
		else
			ALERT(at_console, "\nWoof. ;w;\nPhysMesh doesn't exist or I can't load it... sowwy Master. >w<");
	}

	if (fPhysMeshExists)
	{
		colonVertexArrays = new btTriangleIndexVertexArray(
			objLoader.mTriCount, objLoader.mIndices,
			3 * sizeof(int),
			objLoader.mVertexCount, objLoader.mVertices,
			3 * sizeof(float)
		);

		PhysMeshShape = new btBvhTriangleMeshShape( colonVertexArrays, true, btVector3( -1000, -1000, -1000 ), btVector3( 1000, 1000, 1000 ) );
//		PhysMeshShape->setLocalScaling( btVector3( utom( 1 ), utom( 1 ), utom( 1 ) ) );
//		PhysMeshShape->setLocalScaling( btVector3( mtou( 1 ), mtou( 1 ), mtou( 1 ) ) );
		PhysMeshShape->setLocalScaling( btVector3( 1, 1, 1 ) );
//		PhysMeshShape->setMargin(0.125f);

		groundShape = new btBvhTriangleMeshShape(*PhysMeshShape);

		collisionShapes.push_back(groundShape);

		groundTrans.setIdentity();
		groundTrans.setOrigin(btVector3(0, 0, 0));

		btScalar mass(0.0f);
		btVector3 localInertia(0, 0, 0);

		groundMS = new btDefaultMotionState(groundTrans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, groundMS, groundShape, localInertia);

		groundBody = new btRigidBody(rbInfo);

		admp_World->addRigidBody(groundBody); // assumption is that treating a static body as a rigid body is not good

//		delete colonVertexArrays;

		ALERT(at_console, "\nCreated world PhysMesh!\n");
		ALERT(at_console, "\nCreated world PhysMesh!\n");
	}
} 

/*
	// the array of all collision shapes
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	// stuff for map geometry loading
	btTriangleIndexVertexArray *colonVertexArrays;
	btBvhTriangleMeshShape *PhysMeshShape;
	btCollisionShape *groundShape;
	btTransform groundTrans;

	btDefaultMotionState *groundMS;
	btRigidBody *groundBody;
*/

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
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	virtual void EXPORT SpawnThink( void );
	void EXPORT PhysicalThink( void );
	
	int ObjectCaps() override { return (FCAP_ACROSS_TRANSITION | FCAP_CONTINUOUS_USE); }

//private:
	btCollisionObject *m_CollisionObject;
	btCollisionShape *m_CollisionShape;
	btRigidBody *m_RigidBody;
	btTransform m_Trans;
	btDefaultMotionState *m_MotionState;
	btRigidBody* m_Body;
//	btBoxShape m_bbox;

	float m_flMass;
	bool m_fDynamic;

	btVector3 vecPos, vecAngles;
};

LINK_ENTITY_TO_CLASS( phys_base, CBasePhysical );

void CBasePhysical::Spawn(void)
{
	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->nextthink = 0.1;
	SetThink( &CBasePhysical::SpawnThink );

//	ALERT( at_console, "\nCBasePhysical::Spawn() time %f", gpGlobals->time );
}

void EXPORT CBasePhysical::SpawnThink( void )
{
	m_CollisionShape = new btBoxShape( btVector3( btScalar( utom( 16.0 ) ), btScalar( utom( 16.0 ) ), btScalar( utom( 16.0 ) ) ) );
	AdmPhysEngine.collisionShapes.push_back( m_CollisionShape );

	m_Trans.setIdentity();
	m_Trans.setOrigin( btVector3( utom( pev->origin.x ), utom( pev->origin.z ), utom( pev->origin.y ) ) );

	btScalar mass( m_flMass );

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	m_fDynamic = (mass != 0.f);

	btVector3 localInertia( 0, 0, 0 );

	if ( m_fDynamic )
		m_CollisionShape->calculateLocalInertia( mass, localInertia );

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	m_MotionState = new btDefaultMotionState( m_Trans );
	btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, m_MotionState, m_CollisionShape, localInertia );
	m_Body = new btRigidBody( rbInfo );

	AdmPhysEngine.admp_World->addRigidBody( m_Body );

//	m_Body->applyCentralImpulse(btVector3(btScalar(0.1), btScalar(4), btScalar(0.1))); // DEbugging, just wanna launch them up a bit
//	m_Body->applyTorqueImpulse(btVector3(btScalar(1.6), btScalar(1.2), btScalar(0.6)));

	vecPos = m_Body->getWorldTransform().getOrigin();
	btScalar ang_yaw, ang_pitch, ang_roll;
	m_Body->getWorldTransform().getRotation().getEulerZYX( ang_yaw, ang_pitch, ang_roll );
	vecAngles = { ang_yaw, ang_pitch, ang_roll };

	if ( m_fDynamic )
	{
		SetThink( &CBasePhysical::PhysicalThink );

		ALERT( at_console, "Created dynamic CBasePhysical!\n" );
//		ALERT( at_console, "\nCBasePhysical::SpawnThink() time %f", gpGlobals->time );
	}
	else
	{
		SetThink( NULL );

		ALERT( at_console, "Created static CBasePhysical!\n" );
	}

	pev->nextthink = pev->ltime + 0.1;
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
//	m_Body->applyTorqueImpulse(btVector3(btScalar(0.275), btScalar(0.155), btScalar(0.135))); // TEST
//	m_Body->applyCentralForce( btVector3( btScalar( 0 ), btScalar( 0.5 ), btScalar( 0 ) ) ); // TEST
		
	vecPos = m_Body->getWorldTransform().getOrigin();

	btScalar ang_yaw, ang_pitch, ang_roll;

	m_Body->getWorldTransform().getRotation().getEulerZYX(ang_yaw, ang_pitch, ang_roll);

	vecAngles = { ang_yaw, ang_pitch, ang_roll };

	pev->origin.x = mtou(vecPos.getX());
	pev->origin.y = mtou(vecPos.getZ());
	pev->origin.z = mtou(vecPos.getY());

	pev->angles.x = vecAngles.getX();
	pev->angles.y = vecAngles.getY();
	pev->angles.z = -vecAngles.getZ();

	UTIL_Rad2Deg(pev->angles.x);
	UTIL_Rad2Deg(pev->angles.y);
	UTIL_Rad2Deg(pev->angles.z);

	UTIL_SetOrigin(pev, pev->origin);

	pev->nextthink = pev->ltime + (1.0f / 100.0f);
	
// 	ALERT( at_console, "\nCBasePhysical::PhysicalThink() time = %f\tourtime = %f", gpGlobals->time, pev->nextthink );
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
	AdmPhysEngine.collisionShapes.push_back(m_CollisionShape);

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

	AdmPhysEngine.admp_World->addRigidBody(m_Body);
	SetThink(NULL);
	pev->nextthink = pev->ltime + 0.1;
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
	if ( !AdmPhysEngine.PhysMeshShape )
		AdmPhysEngine.CreateWorldCollision(STRING(g_iszWorldModel));
	
	pev->nextthink = 1.0;
	SetThink(&CPhysicsManager::PM_SpawnThink); 

//	ALERT( at_console, "\nCPhysicsManager::Spawn() time %f", gpGlobals->time );
}

void EXPORT CPhysicsManager::PM_SpawnThink()
{
	pev->nextthink = gpGlobals->time + 0.8;
	SetThink( &CPhysicsManager::PM_Think );

//	ALERT( at_console, "\nCPhysicsManager::PM_SpawnThink() time %f", gpGlobals->time );
}

void EXPORT CPhysicsManager::PM_Think()
{
//	ALERT(at_console, "\nCPhysicsManager::PM_Think() time %f\tnextthink %f", gpGlobals->time, pev->nextthink);

//	AdmPhysEngine.PhysMeshShape->postUpdate();
//	ALERT(at_console, "\nPhysMeshShape->postUpdate()");

//	if ( AdmPhysEngine.PhysMeshShape )
//		AdmPhysEngine.PhysMeshShape->updateBound();
//	ALERT(at_console, "\nPhysMeshShape->updateBound()");
//	if ( AdmPhysEngine.PhysMeshShape )
//		AdmPhysEngine.PhysMeshShape->postUpdate();
//	ALERT(at_console, "\nPhysMeshShape->postUpdate()");
	
	AdmPhysEngine.admp_World->stepSimulation(1.0f / 60.0f, 12);
//	ALERT(at_console, "\nWorld->stepSimulation()");

	pev->nextthink = gpGlobals->time + (1.0f / 100.0f);
}

LINK_ENTITY_TO_CLASS(phys_manager, CPhysicsManager);
