#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#include "IPhysBody.h"
#include "PhysBody.h"
#include "PhysBody_Plane.h"

/*
========================================================

	CPhysBody_Plane

========================================================
*/
void CPhysBody_Plane::InitShape( const PhysParameters& params )
{
	btVector3 btExtents( params.dimensions.x, 2.0f, params.dimensions.y );
	shape = new btBoxShape( PhysUtils::utom( btExtents ) );
}
