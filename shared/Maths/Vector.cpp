#include <cmath>
#include "Base/ExtDLL.h"
#include "Util.h"
#include "Vector.h"

// ====================================================
// Vector::AngleVectors
// 
// Copied from pm_math.cpp
// ====================================================
void Vector::AngleVectors( Vector* forward, Vector* right, Vector* up )
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = y * (M_PI * 2 / 360);
	sy = sin( angle );
	cy = cos( angle );
	angle = x * (M_PI * 2 / 360);
	sp = sin( angle );
	cp = cos( angle );
	angle = z * (M_PI * 2 / 360);
	sr = sin( angle );
	cr = cos( angle );

	if ( forward )
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if ( right )
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}
	
	if ( up )
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}
