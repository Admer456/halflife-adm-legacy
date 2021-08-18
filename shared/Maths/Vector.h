/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef VECTOR_H
#define VECTOR_H

//=========================================================
// 2DVector - used for many pathfinding and many other 
// operations that are treated as planar rather than 3d.
//=========================================================
class Vector2D
{
public:
	inline Vector2D()
		: x(0.0f), y(0.0f)
	{ 
	
	}
	
	inline Vector2D(float X, float Y)
		: x(X), y(Y)
	{ 
	
	}
	
	inline Vector2D operator+(const Vector2D& v) const 
	{ 
		return Vector2D(x+v.x, y+v.y);	
	}
	
	inline Vector2D operator-(const Vector2D& v) const 
	{ 
		return Vector2D(x-v.x, y-v.y);	
	}
	
	inline Vector2D operator*(float fl)	const 
	{ 
		return Vector2D(x*fl, y*fl);
	}
	
	inline Vector2D operator/(float fl)	const 
	{ 
		return Vector2D(x/fl, y/fl);	
	}
	
	inline float Length() const 
	{ 
		return sqrt(x*x + y*y );
	}

	inline Vector2D Normalized() const
	{
		Vector2D vec2;

		float flLen = Length();
		if ( flLen == 0 )
		{
			return Vector2D( 0, 0 );
		}
		else
		{
			flLen = 1 / flLen;
			return Vector2D( x * flLen, y * flLen );
		}
	}

	vec_t x, y;
};

inline float DotProduct( const Vector2D& a, const Vector2D& b ) 
{ 
	return( a.x*b.x + a.y*b.y ); 
}

inline Vector2D operator*( float fl, const Vector2D& v )
{ 
	return v * fl; 
}

// A bit of forward declaration because float * const Vector& is used inside Vector
class Vector;
inline Vector operator* ( float fl, const Vector& v );

//=========================================================
// 3D Vector
//=========================================================
class Vector						// same data-layout as engine's vec3_t,
{								//		which is a vec_t[3]
public:
	// Construction/destruction
	inline Vector()
		: x(0.0f), y(0.0f), z(0.0f)
	{ 
	
	}

	inline Vector( float X, float Y, float Z )
		: x(X), y(Y), z(Z)
	{

	}

	inline Vector( const Vector& v )
		: x(v.x), y(v.y), z(v.z)
	{
	
	}
	
	inline Vector( float rgfl[3] )		
		: x(rgfl[0]), y(rgfl[1]), z(rgfl[2])
	{ 

	}

	// ====================================================
	// Operators
	// ====================================================
	inline Vector operator- () const				
	{ 
		return Vector(-x,-y,-z);				
	}
	
	inline int operator== ( const Vector& v ) const	
	{ 
		return x==v.x && y==v.y && z==v.z;	
	}

	inline int operator!= ( const Vector& v ) const	
	{
		return !(*this==v);					
	}

	inline Vector operator+ ( const Vector& v ) const	
	{ 
		return Vector(x+v.x, y+v.y, z+v.z);	
	}

	inline Vector operator- ( const Vector& v ) const	
	{ 
		return Vector(x-v.x, y-v.y, z-v.z);	
	}

	inline Vector operator* ( float fl ) const			
	{ 
		return Vector(x*fl, y*fl, z*fl);		
	}

	inline Vector operator/ ( float fl ) const			
	{ 
		return Vector(x/fl, y/fl, z/fl);		
	}

	inline Vector operator+= ( const Vector& v ) 
	{ 
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline Vector operator-= ( const Vector& v ) 
	{ 
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;						
	}

	inline Vector operator*= ( float fl )		
	{ 
		x *= fl;
		y *= fl;
		z *= fl;
		return *this;
	}

	inline Vector operator/= ( float fl )		
	{ 
		x /= fl;
		y /= fl;
		z /= fl;
		return *this;					
	}
	
	inline float operator* ( const Vector& v ) const
	{
		return Dot( v );
	}

	// Vectors automatically convert to float * when needed
	operator float *() 
	{ 
		return &x; 
	}
	
	operator const float *() const 
	{ 
		return &x; 
	}
	
	// ====================================================
	// Methods
	// ====================================================
	inline void CopyToArray( float* rgfl ) const		
	{ 
		rgfl[0] = x; 
		rgfl[1] = y; 
		rgfl[2] = z;
	}

	inline float Length() const
	{ 
		return sqrt(x*x + y*y + z*z); 
	}
	
	inline float Dot( const Vector& v ) const
	{
		return (x * v.x + y * v.y + z * v.z);
	}

	inline Vector Cross( const Vector& v ) const
	{
		return Vector( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
	}

	// @param mod: Bias. Lower the value, greater the angle between the incoming
	// vector and the reflected vector. Higher the value, greater the influence of the surface normal
	// @returns The reflection of this vector off a surface normal
	inline Vector Reflected( const Vector& normal, const float& mod = 2.0f ) const
	{
		const float dot = Dot( normal );
		const Vector inverseNormal = normal * mod * dot;

		return *this - inverseNormal;
	}

	// Reflects this vector off a plane
	inline void Reflect( const Vector& normal, const float& mod = 2.0f )
	{
		*this = Reflected( normal, mod );
	}

	// "Flattens" this vector on a plane
	// This can be useful when you wanna glide over a surface nicely
	inline Vector ProjectedOnPlane( const Vector& normal ) const
	{
		// The original formula divided the dot product
		// by the square of the length of the normal
		// But normals are normalized, so that's unnecessary
		return *this - (Dot( normal ) * normal);
	}

	// Projects *this* vector on a plane
	inline void ProjectOnPlane( const Vector& normal )
	{
		*this = ProjectedOnPlane( normal );
	}

	// NOTE: This works with directional vectors, not Euler angle vectors
	// It would be better to use quaternions for this 
	// @returns A vector that was rotated around an axis
	inline Vector Rotated( const Vector& axis, const float& angle ) const
	{
		const float degrees = angle * 180.0f / 3.14159f;

		// How much of this will remain after the rotation
		Vector cosV = *this * std::cos( degrees );
		// Dunno what this really means
		Vector K = (1.0f - std::cos( degrees )) * (Dot( axis )) * axis;
		// How much remains of the rotated vector
		Vector cross = std::sin( degrees ) * Cross( axis );

		return cosV + K + cross;
	}

	// Rotates this vector around an axis
	inline void Rotate( const Vector& axis, const float& angle )
	{
		*this = Rotated( axis, angle );
	}

	// Checks if two vectors are roughly equal
	// @param v: The vector to check against
	// @param epsilon: Tolerance value, default 0.01
	inline bool Equals( const Vector& v, const float& epsilon = 0.01f ) const
	{
		bool equalX = std::fabs( v.x - x ) < epsilon;
		bool equalY = std::fabs( v.y - y ) < epsilon;
		bool equalZ = std::fabs( v.z - z ) < epsilon;

		return equalX && equalY && equalZ;
	}

	// Calculates forward, right and up vectors from an angle vector
	// Assumes that this vector contains Euler angles in degrees
	void AngleVectors( Vector* forward = nullptr, Vector* right = nullptr, Vector* up = nullptr ) const;

	// Treats this vector as a direction and converts it to Euler angles (pitch and yaw only)
	Vector ToAngles() const;

	// Normalizes this vector
	inline void Normalize()
	{
		*this = this->Normalized();
	}

	// @returns A normalized version of this vector
	inline Vector Normalized() const
	{
		float flLen = Length();
		if ( flLen == 0 ) 
			return Vector( 0, 0, 1 ); // ????
		flLen = 1.0f / flLen;
		return Vector( x * flLen, y * flLen, z * flLen );
	}

	// @returns A 2D version of this vector
	inline Vector2D To2D() const
	{
		return Vector2D( x, y );
	}

	inline float Length2D() const
	{ 
		return sqrt(x*x + y*y); 
	}

	// Members
	vec_t x, y, z;
};

inline Vector operator* ( float fl, const Vector& v )	
{ 
	return v * fl; 
}

inline float DotProduct( const Vector& a, const Vector& b ) 
{ 
	return a * b;
}

inline Vector CrossProduct( const Vector& a, const Vector& b ) 
{ 
	return a.Cross( b );
}

#endif
