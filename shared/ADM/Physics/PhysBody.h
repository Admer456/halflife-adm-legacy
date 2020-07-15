#pragma once

class IPhysBody;
class Vector;

/*
========================================================

	CPhysBody

	Base implementation of IPhysBody
	Not actually usable as a physics body, because
	it does not have a collision shape!

	Used phys parameters: none

========================================================
*/
class CPhysBody : public IPhysBody
{
public: 
	CPhysBody() = default;
	~CPhysBody() = default;

	virtual void				Init( CBaseEntity* parent, const PhysParameters& params ) override;
	virtual void				InitShape( const PhysParameters& params ) = 0;

	virtual btCollisionShape*	GetCollisionShape() override { return shape; }
	virtual btRigidBody*		GetRigidBody() override { return rigidBody; }
	virtual CBaseEntity*		GetParentEntity() override { return parent; }

	virtual float				GetMass() override { return mass; }
	virtual bool				IsDynamic() override { return isDynamic; }

	virtual void				ResetTransform();

	virtual void				SetOrigin( const btVector3& origin ) override;
	virtual void				SetAngles( const btVector3& angles ) override;

	virtual void				SetOrigin( const Vector& origin );
	virtual void				SetAngles( const Vector& angles );

	virtual Vector				GetOrigin();
	virtual Vector				GetAngles();

public:
	// Utility to construct rigid bodies
	static btRigidBody*			ConstructRigidBody( btScalar& mass, btMotionState* motionState, btCollisionShape* shape )
	{
		if ( !shape || !motionState )
			return nullptr;

		btVector3 localInertia;
		shape->calculateLocalInertia( mass, localInertia );
		return new btRigidBody( btRigidBody::btRigidBodyConstructionInfo( mass, motionState, shape, localInertia ) );
	}

protected:
	btRigidBody*				rigidBody;	// The result of combining a collision shape, mass etc.
	btCollisionShape*			shape;

	btTransform					transform;	// The transform of this body
	btDefaultMotionState*		motionState;// The motion state of this body

	btScalar					mass;		// Mass of this physics body
	bool						isDynamic;	// If mass is 0, then isDynamic is false

	CBaseEntity*				parent;		// Entity that "owns" this phys body
};
