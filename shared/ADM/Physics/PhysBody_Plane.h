#pragma once

/*
========================================================

	CPhysBody_Plane

	Default physical plane

========================================================
*/
class CPhysBody_Plane : public CPhysBody
{
public:
	CPhysBody_Plane() = default;
	~CPhysBody_Plane() = default;

	void						InitShape( const PhysParameters& params ) override;
};