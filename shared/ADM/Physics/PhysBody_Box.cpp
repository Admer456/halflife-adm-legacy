#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"

#include "IPhysBody.h"
#include "PhysBody.h"
#include "PhysBody_Box.h"

/*
========================================================

	CPhysBody_Box

========================================================
*/
void CPhysBody_Box::InitShape( const PhysParameters& params )
{
	btVector3 btExtents( params.dimensions.x, params.dimensions.z, params.dimensions.y );
	shape = new btBoxShape( PhysUtils::utom(btExtents) );
}
