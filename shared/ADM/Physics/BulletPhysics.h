/*
*	Basic and essential Bullet API headers
*	These are required for basically anything to do with
*	the physics system
*/

#pragma once

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "ConvexDecomposition/ConvexDecomposition.h"
#include "ConvexDecomposition/cd_wavefront.h"

// Basic Bullet-to-GoldSrc unit conversion
// To-do: move to Util.h or something
namespace PhysUtils
{
	// 1u = 0.0254m
	template< typename T >
	T utom( T vec )
	{
		return vec / 39.37007874f;
	}

	// 1m = 39.37u
	template< typename T >
	T mtou( T vec )
	{
		return vec * 39.37007874f;
	}
}
