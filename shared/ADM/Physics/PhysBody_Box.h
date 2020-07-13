#pragma once

/*
========================================================

	CPhysBody_Box

	Box physics body

	Used phys parameters:
	extents : the box's half-extents

========================================================
*/
class CPhysBody_Box : public CPhysBody
{
public:
	CPhysBody_Box() = default;
	~CPhysBody_Box() = default;

	void						InitShape( const PhysParameters& params ) override;
};
