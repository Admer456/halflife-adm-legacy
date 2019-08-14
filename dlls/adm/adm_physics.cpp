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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "adm_physics.h"
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

//	Never mind, figured it out. :P
//	objl::Loader objLoader; // couldn't figure out Bullet's OBJ loader, so I just went ahead with this, lmao
	ConvexDecomposition::WavefrontObj objLoader;
	
	auto fPhysMeshExists = objLoader.loadObj(szPath.c_str());
	ALERT(at_console, "\nfPhysMeshExists = %i", fPhysMeshExists);

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

		PhysMeshShape = new btGImpactMeshShape(colonVertexArrays);
		PhysMeshShape->setLocalScaling(btVector3(mtou(1), mtou(1), mtou(1)));
		PhysMeshShape->setMargin(0.125f);
		PhysMeshShape->postUpdate();
//		PhysMeshShape->updateBound();
//		PhysMeshShape->postUpdate();

		groundShape = new btGImpactMeshShape(*PhysMeshShape);

		collisionShapes.push_back(groundShape);

		groundTrans.setIdentity();
		groundTrans.setOrigin(btVector3(0, 0, 0));

		btScalar mass(0.0f);
		btVector3 localInertia(0, 0, 0);

		groundMS = new btDefaultMotionState(groundTrans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, groundMS, groundShape, localInertia);

		groundBody = new btRigidBody(rbInfo);
	//	groundBody = localCreateRigidBody

		ALERT(at_console, "\nMOTHERFUCKER, YOU PISS ME OFF NIGGA\n");

		admp_World->addRigidBody(groundBody);

		ALERT(at_console, "\nAAAAAAAAAAAAA FUCK YOUUUUUUUUUUUUUU Holy shit that hurt.\n");
	}

//	objLoader.~WavefrontObj();
} 

/*btTriangleIndexVertexArray* colonVertexArrays = new btTriangleIndexVertexArray(
wobj.mTriCount,
wobj.mIndices,
                3*sizeof(int),
                wobj.mVertexCount,
                wobj.mVertices,
                3*sizeof(float)
                );

btGImpactMeshShape* bunnymesh = new btGImpactMeshShape(colonVertexArrays);*/

/*btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -56, 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);*/

// Utilities

// Function to convert from units to metres
float utom(float units)
{
	// 39.37 is how many inches there are in 1 metre
	// 1 unit is about 1 inch
	return units / 39.37007874;
}

// Function to convert from metres to units
float mtou(float metres)
{
	return metres * 39.37007874;
}

// AdmPhys special entities and classes

class CBasePhysical : public CBaseEntity
{
public:
	virtual void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	virtual void EXPORT Think(void);
	
	int ObjectCaps() override { return FCAP_ACROSS_TRANSITION | FCAP_CONTINUOUS_USE; }

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

void CBasePhysical::Spawn(void)
{
	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	m_CollisionShape = new btBoxShape(btVector3(btScalar(utom(16.0)), btScalar(utom(16.0)), btScalar(utom(16.0))));
	AdmPhysEngine.collisionShapes.push_back(m_CollisionShape);

	m_Trans.setIdentity();
	m_Trans.setOrigin(btVector3(utom(pev->origin.x), utom(pev->origin.z), utom(pev->origin.y)));

	btScalar mass(m_flMass);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	m_fDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);

	if (m_fDynamic)
		m_CollisionShape->calculateLocalInertia(mass, localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
	m_MotionState = new btDefaultMotionState(m_Trans);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, m_MotionState, m_CollisionShape, localInertia);
	m_Body = new btRigidBody(rbInfo);

	AdmPhysEngine.admp_World->addRigidBody(m_Body);

//	m_Body->applyCentralImpulse(btVector3(btScalar(0.1), btScalar(4), btScalar(0.1))); // DEbugging, just wanna launch them up a bit
//	m_Body->applyTorqueImpulse(btVector3(btScalar(1.6), btScalar(1.2), btScalar(0.6)));

	vecPos = m_Body->getWorldTransform().getOrigin();
	btScalar ang_yaw, ang_pitch, ang_roll;
	m_Body->getWorldTransform().getRotation().getEulerZYX(ang_yaw, ang_pitch, ang_roll);
	vecAngles = { ang_yaw, ang_pitch, ang_roll };

	if (m_fDynamic)
	{
		SetThink(&CBasePhysical::Think);
		pev->nextthink = 1.0;
	}
}

LINK_ENTITY_TO_CLASS(phys_base, CBasePhysical);

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

void EXPORT CBasePhysical::Think(void)
{
//	m_Body->applyTorqueImpulse(btVector3(btScalar(0.005), btScalar(0.005), btScalar(0.005)));

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

//	ALERT(at_console, "\norigin %f %f %f\nangles pitch %f yaw %f roll %f", 
//		pev->origin.x, pev->origin.y, pev->origin.z,
//		pev->angles.x, pev->angles.y, pev->angles.z);

/*	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_LINE);
	WRITE_COORD(pev->origin.x); // coords
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_COORD((pev->origin.x + 64)); // coords
	WRITE_COORD((pev->origin.y + 64));
	WRITE_COORD((pev->origin.z + 64));
	WRITE_SHORT(5);
	WRITE_BYTE(255);
	WRITE_BYTE(128);
	WRITE_BYTE(0);
	MESSAGE_END(); */

	pev->nextthink = pev->ltime + (1.0 / 60.0);
}

class CPhysicsStatic : public CBasePhysical
{
	void Spawn();
};

void CPhysicsStatic::Spawn()
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
//	SET_MODEL(ENT(pev), STRING(pev->model));

	m_CollisionShape = new btBoxShape(btVector3(btScalar(utom(256.0)), btScalar(utom(0.5)), btScalar(utom(256.0))));
	AdmPhysEngine.collisionShapes.push_back(m_CollisionShape);

	m_Trans.setIdentity();
	m_Trans.setOrigin(btVector3(utom(pev->origin.x), utom(pev->origin.z), utom(pev->origin.y)));

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
}

LINK_ENTITY_TO_CLASS(phys_staticplane, CPhysicsStatic);

/*class CPhysicsBox : public CBasePhysical
{

};*/

class CPhysicsManager : public CPointEntity
{
	void Spawn();
	void EXPORT PM_Think();
	void EXPORT PM_SpawnThink();
};

void CPhysicsManager::Spawn()
{
	AdmPhysEngine.CreateWorldCollision(STRING(g_iszWorldModel));
	SetThink(&CPhysicsManager::PM_SpawnThink); 
	pev->nextthink = 1.0;
}

void EXPORT CPhysicsManager::PM_Think()
{
	ALERT(at_console, "\nCPhysicsManager::PM_Think()");

//	AdmPhysEngine.PhysMeshShape->postUpdate();
//	ALERT(at_console, "\nPhysMeshShape->postUpdate()");

//	AdmPhysEngine.PhysMeshShape->updateBound();
//	ALERT(at_console, "\nPhysMeshShape->updateBound()");

	AdmPhysEngine.PhysMeshShape->postUpdate();
	ALERT(at_console, "\nPhysMeshShape->postUpdate()");
	
	AdmPhysEngine.admp_World->stepSimulation(1.0f / 60.0f, 12);
	ALERT(at_console, "\nWorld->stepSimulation()");

	//print positions of all objects
/*	for (int j = AdmPhysEngine.admp_World->getNumCollisionObjects() - 1; j >= 0; j--)
	{
		btCollisionObject* obj = AdmPhysEngine.admp_World->getCollisionObjectArray()[j];
		btRigidBody* body = btRigidBody::upcast(obj);
		btTransform trans;
		if (body && body->getMotionState())
		{
			body->getMotionState()->getWorldTransform(trans);
		}
		else
		{
			trans = obj->getWorldTransform();
		}
	//	ALERT(at_console, "\nworld pos object %d = %f,%f,%f\n", j, float(mtou((trans.getOrigin().getX()))), float(mtou((trans.getOrigin().getZ()))), float(mtou((trans.getOrigin().getY()))));
	} */

	pev->nextthink = pev->ltime + (1.0 / 60.0);
	ALERT(at_console, "\npev->nextthink");
}

void EXPORT CPhysicsManager::PM_SpawnThink()
{
	pev->nextthink = pev->ltime + (1.0f / 60.0f);

	SetThink(&CPhysicsManager::PM_Think);
}

LINK_ENTITY_TO_CLASS(phys_manager, CPhysicsManager);
