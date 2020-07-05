/*

======= adm_vehicle.cpp ===========================

	Vehicle API.

	The biggest beast that might've struck my mod.
	Vehicles are composed of 3 parts: a body (or 
	more), at least one seat, and optionally wheels.

	A body is required for the sake of physical properties,
	or else the vehicle wouldn't know how to behave.
	Essentially, the gravity pulls down the body. The wheels
	are absolutely necessary if it's a car, or a bike type.

	Otherwise, it doesn't need wheels. One driver seat, and
	one body is all that is needed for a vehicle to be physical
	and to be enterable. Boats, planes etc. won't need wheels,
	for example.
*/

#include "Base/ExtDLL.h"
#include "Util.h"
#include "Base/CBase.h"
#include "Player.h"
#include "Base/SaveRestore.h"
#include "AdmVehicle.h"
#include "Base/Animation.h"

// Welcome to the beginning of this monstrosity!
// Scroll all the way down until line 469 if you wanna take a look at a vehicle class template.
// CBaseVehicle handles all the work for you, and all you gotta do
// is to create a new vehicle class, and just define a VehicleInit() function.
// It's as easy as that. ;)

// CONTENTS
/*
	BASE CLASSES:
		CBaseVehicle
		- CBaseCar

	VEHICLE CLASSES:
		CVehiclePrototype (CBaseVehicle)
		CVehiclePrototypeTwoseat (CBaseVehicle)
		CVehicleCarTest (CBaseCar)
*/

// TO-DO: REFACTOR the HELL out of this thing

void CBaseVehicle::Spawn()
{
	SET_MODEL( ENT( pev ), (char*)STRING( pev->model ) );

	SetThink( NULL );

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BBOX;
	pev->friction = 0.01;
	pev->v_angle = pev->angles;

	VehicleInit(); // all the basic stuff goes here - wheels, engine, seats, weapons, sounds to use
	Precache(); // here we precache that stuff
	VehicleBaseInit(); // and here we apply the model, hence I had to precache between Init and BaseInit

	SetUse( &CBaseVehicle::VehicleUse );
	SetThink( &CBaseVehicle::VehicleThink );

	pev->nextthink = gpGlobals->time + 1.5; // They said this is a magic number. :o
}

void CBaseVehicle::Precache()
{
	if ( v_Body.m_iszModel )
		PRECACHE_MODEL( (char*)STRING( v_Body.m_iszModel ) );
}

void CBaseVehicle::KeyValue( KeyValueData *pkvd )
{
	if ( KeyvalueToken( bodymodel ) )
	{
		KeyvalueToString( v_Body.m_iszModel );
	}

	else if ( KeyvalueToken( bone ) )
	{
		KeyvalueToInt( m_iBoneOffset );
	}

	else KeyvaluesFromBase( CBaseEntity );
}

void CBaseVehicle::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	if ( ((CBasePlayer*)pActivator)->m_InVehicle )
	{
		return;
	}

	SetUse( &CBaseVehicle::VehicleUse );

	VehicleUse( pActivator, pCaller, useType, value );
}

void CBaseVehicle::SeatPlayerLocking()
{
	// This function will keep the players in sync with the respected seat positions.
	
	for ( int i = 0; i < m_iSeats; i++ )
	{
		v_Seats[i].SeatPlayer();
		if ( v_Seats[i].pSessilis )
			v_Seats[i].pSessilis->pev->angles = pev->angles;
	}
}

void CBaseVehicle::ListenToCommands()
{
	// Here, the vehicle listens to whatever custom commands we are sending

	if ( m_iSeats )
	{
		for ( int i = 0; i < m_iSeats; i++ )
			v_Seats[i].ListenToCommands();
	}
}

void CBaseVehicle::SeatPositionLocking()
{
	if ( v_Body.m_iszModel )
	{
		for ( int i = 0; i < m_iSeats; i++ )
		{
			v_Seats[i].AttachToPos( m_iBoneOffset );
		}
	}

	else
	{
		for ( int i = 0; i < m_iSeats; i++ )
		{
			v_Seats[i].AttachToPos( v_Body.origin, 0, 0, 0 );
		}
	}
}

void CBaseVehicle::SeatSwitch( VehicleSeat &seatFrom )
{
	if ( seatFrom.iSitdex == (m_iSeats - 1) )
	{
		for ( int i = 0; i < m_iSeats; i++ )
		{
			if ( v_Seats[i].pSessilis != nullptr )
			{
				continue;
			}

			else
			{
				seatFrom.SeatSwitch( v_Seats[i] );
				break;
			}
		}
	}

	else
	{
		for ( int i = seatFrom.iSitdex; i < m_iSeats; i++ )
		{
			if ( v_Seats[i].pSessilis != nullptr )
			{
				continue;
			}

			else
			{
				seatFrom.SeatSwitch( v_Seats[i] );
				break;
			}
		}
	}
}

void EXPORT CBaseVehicle::VehicleUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator || !pActivator->IsPlayer() || ((CBasePlayer*)pActivator)->m_InVehicle )
	{
		return;
	}

	else
	{
		static_cast<CBasePlayer*>(pActivator)->m_InVehicle = static_cast<InVehicleType>(v_Type + 1);
		for ( int i = 0; i < m_iSeats; i++ )
		{
			if ( v_Seats[i].pSessilis == NULL && v_Seats[i].fExists == true )
			{
				v_Seats[i].pSessilis = (CBasePlayer *)pActivator;
				v_Seats[i].pSessilis->pev->movetype = MOVETYPE_NOCLIP;
				v_Seats[i].pSessilis->pev->solid = SOLID_NOT;
				SeatPlayerLocking();

				break;
			}
		}
	}
}

void EXPORT CBaseVehicle::VehicleThink()
{
	v_Body.origin = pev->origin;
	pev->oldorigin = pev->origin;

	SeatPlayerLocking();
	SeatPositionLocking();
	ListenToCommands();

	VehicleMovement();

	for ( int i = 0; i < m_iSeats; i++ )
	{
		if ( v_Seats[i].commands & BIT( VehDismount ) )
		{
			v_Seats[i].Exit();
		}

		if ( v_Seats[i].commands & BIT( VehSwitchSeats ) )
		{
			SeatSwitch( v_Seats[i] );
		}

		if ( v_Seats[i].pSessilis )
		{
			FBitSet( v_Seats[i].pSessilis->pev->flags, FL_ONGROUND );
		}
	}

	pev->nextthink = 0.5;
}

void CBaseVehicle::VehicleBaseInit()
{
	if ( v_Body.m_iszModel )
		pev->model = v_Body.m_iszModel;

	SET_MODEL( ENT( pev ), STRING( pev->model ) );

	pev->max_health = v_Engine.MaxHealth;
	pev->health = pev->max_health;

	v_Body.origin = pev->origin;
}

void CBaseVehicle::VehicleInit()
{
	// All the properties are initialised here.
	// Engine properties like HP, the drive, efficiency,
	// seats and their properties, and the body properties.

	// Heaven:
	// v_Body.mass = Script_Read( "v_Body.mass" );

	v_Body.mass = 30; // kg
	v_Body.density = 670; // kg/m^3, Birch wood, European

	v_Seats[0].Init( this, Driver, v_Body.origin, 0 );

	v_Engine.Efficiency = 1.0f;
	v_Engine.GearRatios[0] = -4.2f;
	v_Engine.GearRatios[1] = 0.0f;
	v_Engine.GearRatios[2] = 4.2f;
	v_Engine.GearRatios[3] = 2.8f;
	v_Engine.GearRatios[4] = 1.8f;
	v_Engine.GearRatios[5] = 1.2f;
	v_Engine.GearRatios[6] = 1.0f;
	
	v_Engine.Drive = Drive_NoWheels;

	v_Type = VehicleSpecialTest;
}

void CBaseVehicle::VehicleMovement()
{
	for ( int i = 0; i < m_iSeats; i++ )
	{
		if ( v_Seats[i].commands & BIT( VehDeceleration )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			pev->velocity.x = cos( (180 + pev->angles.y) * 0.017453292 ) * 20;
			pev->velocity.y = sin( (180 + pev->angles.y) * 0.017453292 ) * 20;

			if ( pev->flags & FL_ONGROUND )
			{
				pev->velocity.z += 100;
			}

			pev->friction = 0.01;
		}

		if ( v_Seats[i].commands & BIT( VehLeft )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			if ( pev->velocity.Length() > 50 )
				pev->angles.y += 500 / pev->velocity.Length();
			else
				pev->angles.y += 10;
		}

		if ( v_Seats[i].commands & BIT( VehRight )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			if ( pev->velocity.Length() > 50 )
				pev->angles.y -= 500 / pev->velocity.Length();
			else
				pev->angles.y -= 10;
		}

		if ( v_Seats[i].commands & BIT( VehJump )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			if ( pev->flags & FL_ONGROUND )
			{
				pev->velocity.z += 200;
			}
		}

		if ( v_Seats[i].commands & BIT( VehLeanLeft )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			pev->angles[1] += 2;
		}

		if ( v_Seats[i].commands & BIT( VehLeanRight )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			pev->angles[1] -= 2;
		}

		if ( v_Seats[i].commands & BIT( VehUp )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			pev->angles[0] += 2;
		}

		if ( v_Seats[i].commands & BIT( VehDown )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			pev->angles[0] -= 2;
		}

		if ( v_Seats[i].commands & BIT( VehAcceleration )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			pev->velocity.x += cos( pev->angles.y * 0.017453292 ) * 20;
			pev->velocity.y += sin( pev->angles.y * 0.017453292 ) * 20;

			if ( pev->flags & FL_ONGROUND )
			{
				pev->velocity.z += 100;
			}

			pev->friction = 0.01;
		}
	}

	oldangles = pev->angles;
	float currentYaw = pev->angles.y;

	if ( pev->velocity.Length() == 0 || pev->velocity.Length2D() < 0.001 )
	{
		pev->angles = oldangles;
	}
	else
	{
		pev->angles = UTIL_VecToAngles( pev->velocity );
		pev->angles[0] *= (-1);
		pev->angles.y = currentYaw;
	}

	if ( pev->velocity.Length2D() > 480 )
	{
		pev->velocity.x = pev->velocity.x * 0.9;
		pev->velocity.y = pev->velocity.y * 0.9;
	}
}

int CBaseVehicle::GetBoneId( int& bone, VehicleType type )
{
	switch ( type )
	{
	case VehicleCar:
		bone += 1; // 1 bone for the steering wheel + m_iWheels usually 4
		break;

	case VehicleBike:
		bone += 1; // engine/pedal bone + m_iWheels usually 2
		break;

	case VehiclePlane:
		bone += 4; // 2 propeller bones + 2 gear

	case VehicleHelicopter:
		bone += 2; // 2 propeller bones
		break;

	case VehicleBoat:
		bone += 1; // 1 propeller bone
		break;

	case VehicleChair:
		// It is already at root, so
	case VehicleShip:
		// space ships shouldn't really have any bones other than the root and seats
	case VehicleNPC:
		// NO
	case VehicleSpecialTest:
		// No, we cannot predict which bone is which :v
		// Instead, let the mapper define the bone offset
		break;
	}

	return bone;
}

/*
	VEHICLES [to be moved to the Wiki]

	Initialisers:

	v_Type: VehicleChair, VehicleCar, VehicleBike,
			VehiclePlane, VehicleHelicopter, VehicleShip,
			VehicleBoat, VehicleNPC, VehicleSpecialTest (VehicleType)

	m_iSeats (int, should match the number of seats initialised)
	m_iWheels (int, should match the number of wheels initialised)
	m_iBoneOffset (int, potentially different for each studio model)

	v_Body (VehicleBody)
	-	mass (float)
	-	density (float, use Density_ constants, e.g. Density_Iron)
	-	m_iszModel (string_t, if you are using a hardcoded model for this entity)
	
	v_Wheels[4] (VehicleWheel)
	-	traction (float)
	-	radius (float)
	-	wear (float)
	-	width (float)
	-	type: Stock, Comfort, Performance, Rally
	-	m_iszModel
	! Initialise these with their Init() functions !

	v_Seats[16] (VehicleSeat)
	-	type: Driver, DriverGunner, Gunner, Passenger (VehicleSeatType)
	-	iSitdex (int, should generally match the index)
	! Initialise these with their Init() functions !

	v_Engine (VehicleEngine)
	-	HorsePower (int)
	-	MaxHealth (float)
	-	MadeInGermany (bool) - a.k.a. indestructible
	-	GearRatios, indices 0 to 8 (float)
		! Maximum of 7 gears, 1st neutral, 0th reverse !
	-	Efficiency (float) 
	-	SlowDown (float)
	-	Drive: Drive_FWD, Drive_RWD, Drive_AWD, Drive_NoWheels (VehicleDrive)
*/

// 2-seated sofa, one gunner seat, one driver seat
// Contains an engine, to handle the speeds and whatnot
// Contains a body (loads a MDL file), no wheels
// Type: VehicleSpecialTest

class CVehiclePrototype : public CBaseVehicle
{
public:

	void VehicleInit()
	{
		CBaseVehicle::VehicleInit();
	}
};

LINK_ENTITY_TO_CLASS( vehicle_base, CVehiclePrototype );
LINK_ENTITY_TO_CLASS( vehicle_basep, CVehiclePrototype );

class CVehiclePrototypeTwoseat : public CBaseVehicle
{
public:

	void VehicleInit()
	{
		v_Body.mass = 30; // kg
		v_Body.density = 670; // kg/m^3, Birch wood, European

		m_iSeats = 2;

		v_Seats[0].Init( this, Driver, v_Body.origin, 0 );
		v_Seats[1].Init( this, Passenger, v_Body.origin, 1 );

		v_Type = VehicleSpecialTest;
	}
};

LINK_ENTITY_TO_CLASS( vehicle_proto_ms, CVehiclePrototypeTwoseat );

class CBaseCar : public CBaseVehicle
{
public:
	void				Spawn();
	void				KeyValue( KeyValueData *pkvd );

	void				VehicleBaseInit();
	void				VehicleMovement();
	void				VehicleThink();
	void				AlignToGround();
	void				WheelLocking();
//	void				ShootTraces();
//	void				HandleTraces();

//	int					TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	string_t			m_iszWheels[32];

private:
	VehiclePhysicsParams physParms;
};

void CBaseCar::Spawn()
{
	SET_MODEL( ENT( pev ), (char*)STRING( pev->model ) );

	SetThink( NULL );

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BBOX;
	pev->friction = 0.01;
	pev->v_angle = pev->angles;

	VehicleInit(); // all the basic stuff goes here - wheels, engine, seats, weapons, sounds to use
	Precache(); // here we precache that stuff
	VehicleBaseInit(); // and here we apply the model, hence I had to precache between Init and BaseInit

	SetUse( &CBaseVehicle::VehicleUse );
	SetThink( &CBaseVehicle::VehicleThink );

	pev->nextthink = 1.5; // They said this is a magic number. :o
}

void CBaseCar::KeyValue( KeyValueData *pkvd )
{
	if ( KeyvalueToken( wheel ) )
	{
		string_t path = ALLOC_STRING( pkvd->szValue );

		for ( int i = 0; i < 31; i++ )
		{
			v_Wheels[i].m_iszModel = path;
		}
	}

	else if ( KeyvalueToken( wheel0 ) )
	{
		KeyvalueToString( m_iszWheels[0] );
	}

	else if ( KeyvalueToken( wheel1 ) )
	{
		KeyvalueToString( m_iszWheels[1] );
	}

	else if ( KeyvalueToken( wheel2 ) )
	{
		KeyvalueToString( m_iszWheels[2] );
	}

	else if ( KeyvalueToken( wheel3 ) )
	{
		KeyvalueToString( m_iszWheels[3] );
	}

	else
	{
		KeyvaluesFromBase( CBaseVehicle );
	}
}

void CBaseCar::VehicleBaseInit()
{
	if ( v_Body.m_iszModel )
		pev->model = v_Body.m_iszModel;

	SET_MODEL( ENT( pev ), STRING( pev->model ) );

	pev->max_health = v_Engine.MaxHealth;
	pev->health = pev->max_health;

	physParms.frontAxle.Init( &v_Wheels[0], &v_Wheels[1], &physParms, 48.0f );
	physParms.rearAxle.Init( &v_Wheels[2], &v_Wheels[3], &physParms, -48.0f );
	physParms.Init( this );

	v_Body.origin = pev->origin;
	UTIL_SetSizeAuto( pev );
}

void CBaseCar::VehicleMovement()
{
	static Vector oldVelocity = pev->velocity;
	float speed = oldVelocity.Length();

	for ( int i = 0; i < m_iSeats; i++ )
	{
		if ( v_Seats[i].commands & BIT( VehAcceleration )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			if ( v_Engine.CurrentGear == -1 && v_Engine.rpm < -100 )
			{
				v_Engine.Brake();
			}

			else
			{
				v_Engine.Accelerate();
			}
		}

		if ( v_Seats[i].commands & BIT( VehDeceleration )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			if ( v_Engine.CurrentGear > 0 )
			{
				v_Engine.Brake();
			}

			else
			{
				v_Engine.CurrentGear = -1;
				v_Engine.Accelerate();
			}
		}

		if ( v_Seats[i].commands & BIT( VehLeft )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			v_Wheels[0].SteerLeft( speed );
			v_Wheels[1].SteerLeft( speed );
		}

		if ( v_Seats[i].commands & BIT( VehRight )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			v_Wheels[0].SteerRight( speed );
			v_Wheels[1].SteerRight( speed );
		}

		if ( v_Seats[i].commands & BIT( VehJump )
			&& (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner) )
		{
			v_Engine.Handbrake();
		}
	}

	// Get physics values from our local vehicle phys params
	pev->angles = physParms.finalAngles;
	pev->angles.z = pev->angles.z*0.95 + (speed * (v_Wheels[0].steerAngle / 30.f))*0.05;
	pev->velocity = physParms.finalVelocity;
	pev->velocity.z -= 300.0f;
	
	physParms.frontAxle.Update();
	physParms.rearAxle.Update();
	physParms.Update();

	oldVelocity = pev->velocity;
}

void CBaseCar::VehicleThink()
{
	float flVelocity = pev->velocity.Length();
	static int cnt = 5; 

	v_Body.origin = pev->origin;
	pev->oldorigin = pev->origin;

	ListenToCommands();		// check which keys are being held by players, in each seat
//	ShootTraces();			// trace collision, distance from ground etc.
//	HandleTraces();			// calculate that traced stuff
//	AlignToGround();		// align the vehicle to the ground

	v_Engine.Update();		// update other components, synchronise their local variables
	v_Wheels[0].Update( flVelocity, 0 );
	v_Wheels[1].Update( flVelocity, 1 );
	v_Wheels[2].Update( flVelocity, 2 );
	v_Wheels[3].Update( flVelocity, 3 );
	
	VehicleMovement();		// calculate vehicle movement
	SeatPositionLocking();	// "attach" the seats to the vehicle, keep them in sync
	SeatPlayerLocking();	// "attach" the players to the seats
	
	for ( int i = 0; i < m_iSeats; i++ )
	{
		if ( v_Seats[i].commands & BIT( VehDismount ) )
		{
			v_Seats[i].Exit();
		}

		if ( v_Seats[i].commands & BIT( VehSwitchSeats ) )
		{
			SeatSwitch( v_Seats[i] );
		}

		if ( v_Seats[i].pSessilis )
		{
			v_Seats[i].pSessilis->pev->flags |= FL_ONGROUND;
		}
	}

	for ( int i = 0; i < m_iWheels; i++ )
	{
		if ( !v_Wheels[i].m_pWheel )
		{
			v_Wheels[i].m_pWheel = UTIL_FindEntityByTargetname( NULL, (char*)STRING( m_iszWheels[i] ) );
		}
	}

	pev->nextthink = gpGlobals->time + 0.001;
}

void CBaseCar::AlignToGround()
{
	Vector vecAngles;
	vecAngles = v_Wheels[0].groundangles;
	vecAngles = vecAngles + v_Wheels[1].groundangles;
	vecAngles = vecAngles + v_Wheels[2].groundangles;
	vecAngles = vecAngles + v_Wheels[3].groundangles;

	vecAngles = vecAngles / 4;
	pev->angles = vecAngles;
}

/*
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,10), ignore_monsters, edict(), &tr );

	if ( tr.flFraction < 1.0 )
	{
		Vector angdir = Vector(
			cos(pev->angles.y * ang2rad) * cos(pev->angles.x * ang2rad),
			sin(pev->angles.y * ang2rad) * cos(pev->angles.x * ang2rad),
			-sin(pev->angles.x * ang2rad));

		Vector angdiry = Vector(
			sin(pev->angles.y * ang2rad) * cos(pev->angles.x * ang2rad),
			cos(pev->angles.y * ang2rad) * cos(pev->angles.x * ang2rad),
			-sin(pev->angles.x * ang2rad));

		pev->angles = UTIL_VecToAngles(angdir - DotProduct(angdir, tr.vecPlaneNormal) * tr.vecPlaneNormal);
		pev->angles.z = -UTIL_VecToAngles(angdiry - DotProduct(angdiry, tr.vecPlaneNormal) * tr.vecPlaneNormal).x;
	}

#undef ang2rad
*/

void CBaseCar::WheelLocking()
{
	for (int i = 0; i < 31; i++)
	{
		if (v_Wheels[i].fExists)
		{
			v_Wheels[i].AttachToPos( i );
		}
	}
}

class CVehicleWheel : public CBaseEntity // it's actually a dummy entity
{
public:
	void Spawn()
	{
		SET_MODEL( ENT( pev ), (char*)STRING( m_iszModel ) );
		SetThink( &CVehicleWheel::Think );
		return;
	}

	void KeyValue( KeyValueData* pkvd )
	{
		if ( KeyvalueToken( mdl ) )
		{
			KeyvalueToString( m_iszModel );
		}

		else KeyvaluesFromBase( CBaseEntity );
	}

	void Use( CBaseEntity* pActivator, CBaseEntity* pOther, USE_TYPE useType, float value ) { return; }
	void Think()
	{
		SUB_DoNothing();
		pev->nextthink = gpGlobals->time + 0.001;
	}

private:
	string_t m_iszModel;
};

LINK_ENTITY_TO_CLASS( vehicle_wheel, CVehicleWheel );

class CVehicleCarTest : public CBaseCar
{
public:
	void VehicleInit()
	{
		v_Body.mass = 400;

		m_iSeats = 1;
		m_iWheels = 4;
		v_Seats[0].Init(this, Driver, v_Body.origin, 0);

		v_Engine.SetGearRatios
		(
			-4.5, 4.5, 
			4.0, 3.5, 
			3.0, 2.0, 
			0, 0
		);

		v_Engine.SetTorqueCurve
		(
			{ 40.0f,     0.0f },
			{ 120.0f,  100.0f },
			{ 240.0f, 1000.0f },
			{ 290.0f, 3200.0f },
			{ 360.0f, 5000.0f }
		);

		v_Engine.Init( this, Drive_AWD, 500, 70, 0.9 );
		
		v_Type = VehicleCar;

		PRECACHE_MODEL( (char*)STRING( v_Wheels[0].m_iszModel ) ); // DIRTY friggin hanck

		v_Wheels[0].Init( Stock, this, m_iszWheels[0], ( BIT( Wheel_Steerable ) | BIT( Wheel_Front ) ) );
		v_Wheels[1].Init( Stock, this, m_iszWheels[1], ( BIT( Wheel_Steerable ) | BIT( Wheel_Front ) ) );
		v_Wheels[2].Init( Stock, this, m_iszWheels[2], ( BIT( Wheel_Handbrake ) | BIT( Wheel_Back ) ) );
		v_Wheels[3].Init( Stock, this, m_iszWheels[3], ( BIT( Wheel_Handbrake ) | BIT( Wheel_Back ) ) );
	}
};

LINK_ENTITY_TO_CLASS( vehicle_bath, CVehicleCarTest );
