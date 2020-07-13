#pragma once

#include <vector>
#include "BulletPhysics.h"

// Physical construction parameters
// Used for initialisation of phys bodies
struct PhysParameters
{
	char*						meshPath{nullptr};	// in case we use a mesh

	Vector						offsetOrigin{Vector(0,0,0)};
	Vector						offsetAngles{Vector(0,0,0)};

	float						scale{1.0f};
	Vector						dimensions;
	bool						calculateDimensionsFromBrush{false};

	float						mass{0.f};			// in kg; if it's 0 it's static
	float						density{1000.f};	// in kg/m^3
	bool						calculateMassFromDensity{false};
};

class CBaseEntity;

/*
========================================================

	IPhysBody

	Base interface for physics bodies

========================================================
*/
class IPhysBody
{
public:
	IPhysBody() = default;
	~IPhysBody() = default;

	virtual void				Init( CBaseEntity* parent, const PhysParameters& params ) = 0;

	// Each phys body will return its own collision shape
	virtual btCollisionShape*	GetCollisionShape() = 0;

	// Each phys body contains a rigid body
	virtual btRigidBody*		GetRigidBody() = 0;

	// The entity that this phys body is part of
	virtual CBaseEntity*		GetParentEntity() = 0;

	virtual float				GetMass() = 0;
	virtual bool				IsDynamic() = 0;

	virtual void				SetOrigin( const btVector3& origin ) = 0;
	virtual void				SetAngles( const btVector3& angles ) = 0;
};
