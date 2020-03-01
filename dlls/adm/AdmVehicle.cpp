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

void CBaseVehicle::Spawn(void)
{
	SET_MODEL(ENT(pev), (char*)STRING(pev->model));

	SetThink(NULL);

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BBOX;
	pev->friction = 0.01;
	pev->v_angle = pev->angles;

	VehicleInit(); // all the basic stuff goes here - wheels, engine, seats, weapons, sounds to use
	Precache(); // here we precache that stuff
	VehicleBaseInit(); // and here we apply the model, hence I had to precache between Init and BaseInit

	SetUse(&CBaseVehicle::VehicleUse);
	SetThink(&CBaseVehicle::VehicleThink);

	pev->nextthink = 1.5; // They said this is a magic number. :o
}

void CBaseVehicle::Precache(void)
{
	// Precache sounds and models here
	if (v_Body.m_iszModel)
		PRECACHE_MODEL((char*)STRING(v_Body.m_iszModel));
}

void CBaseVehicle::KeyValue(KeyValueData *pkvd)
{
	if (KeyvalueToken(bodymodel))
	{
		KeyvalueToString(v_Body.m_iszModel);
	}

	else if (KeyvalueToken(bone))
	{
		KeyvalueToInt(m_iBoneOffset);
	}

	else KeyvaluesFromBase(CBaseEntity);
}

void CBaseVehicle::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	ALERT(at_console, "Vehicle Use\n");

	if (((CBasePlayer*)pActivator)->m_InVehicle)
	{
		ALERT(at_console, "VehicleUse unsuccessful, player already in vehicle\n");
		return;

	}

	SetUse(&CBaseVehicle::VehicleUse);

	VehicleUse(pActivator, pCaller, useType, value);
}

void EXPORT CBaseVehicle::VehicleUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if ( !pActivator || !pActivator->IsPlayer() || ((CBasePlayer*)pActivator)->m_InVehicle )
	{
		return;
	}
	else
	{
		static_cast<CBasePlayer*>(pActivator)->m_InVehicle = static_cast<InVehicleType>(v_Type + 1);
		for (int i = 0; i < m_iSeats; i++)
		{
			if (v_Seats[i].pSessilis == NULL && v_Seats[i].fExists == true)
			{
				v_Seats[i].pSessilis = (CBasePlayer *)pActivator;
				v_Seats[i].pSessilis->pev->movetype = MOVETYPE_NOCLIP;
				v_Seats[i].pSessilis->pev->solid = SOLID_NOT;
				SeatPlayerLocking();

				break;
			}

			else
			{
				ALERT(at_console, "\nSeat%d not free", i);
			}
		}
	}
}

void EXPORT CBaseVehicle::VehicleThink()
{
	v_Body.pos = pev->origin;
	pev->oldorigin = pev->origin;

	SeatPlayerLocking();
	SeatPositionLocking();
	ListenToCommands();

	VehicleMovement();

	for (int i = 0; i < m_iSeats; i++)
	{
		if (v_Seats[i].fCommands[bi_unuse] == true)
		{
			VehicleExit(&v_Seats[i]);
		}

		if (v_Seats[i].fCommands[bi_seatswitch] == true)
		{
			ALERT(at_console, "\nVehicleThink() - someone wants to switch seats");
			SeatSwitch(v_Seats[i]);
		}

		if (v_Seats[i].pSessilis)
		{
			FBitSet(v_Seats[i].pSessilis->pev->flags, FL_ONGROUND);
		}
	}

	pev->nextthink = 0.5;
}

void CBaseVehicle::VehicleMovement()
{
	for (int i = 0; i < m_iSeats; i++)
	{
		if (v_Seats[i].fCommands[bi_dec] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			pev->velocity.x = cos((180 + pev->angles.y) * 0.017453292) * 20;
			pev->velocity.y = sin((180 + pev->angles.y) * 0.017453292) * 20;

			if (pev->flags & FL_ONGROUND)
			{
				pev->velocity.z += 100;
			}

			pev->friction = 0.01;
		}

		if (v_Seats[i].fCommands[bi_lt] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			if (pev->velocity.Length() > 50)
				pev->angles.y += 500 / pev->velocity.Length();
			else
				pev->angles.y += 10;
		}

		if (v_Seats[i].fCommands[bi_rt] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			if (pev->velocity.Length() > 50)
				pev->angles.y -= 500 / pev->velocity.Length();
			else
				pev->angles.y -= 10;
		}

		if (v_Seats[i].fCommands[bi_jump] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			if (pev->flags & FL_ONGROUND)
			{
				pev->velocity.z += 200;
			}
		}

		if (v_Seats[i].fCommands[bi_llt] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			pev->angles[1] += 2;
		}

		if (v_Seats[i].fCommands[bi_lrt] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			pev->angles[1] -= 2;
		}

		if (v_Seats[i].fCommands[bi_up] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			pev->angles[0] += 2;
		}

		if (v_Seats[i].fCommands[bi_dn] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			pev->angles[0] -= 2;
		}

		if (v_Seats[i].fCommands[bi_acc] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner)/* == true && pev->flags & FL_ONGROUND*/)
		{
			pev->velocity.x += cos(pev->angles.y * 0.017453292) * 20;
			pev->velocity.y += sin(pev->angles.y * 0.017453292) * 20;
			
			if (pev->flags & FL_ONGROUND)
			{
				pev->velocity.z += 100;
			}

			pev->friction = 0.01;
		}

	}

	oldangles = pev->angles;
	float currentYaw = pev->angles.y;

	if (pev->velocity.Length() == 0
		|| pev->velocity.Length2D() < 0.001)
	{
		pev->angles = oldangles;
	}
	else
	{
		pev->angles = UTIL_VecToAngles(pev->velocity);
		pev->angles[0] *= (-1);
		pev->angles.y = currentYaw;
	}

	if (pev->velocity.Length2D() > 480)
	{
		pev->velocity.x = pev->velocity.x * 0.9;
		pev->velocity.y = pev->velocity.y * 0.9;
	}

}

void CBaseVehicle::ListenToCommands()
{
	// Here, the vehicle listens to whatever custom commands we are sending
	// with the ControlAPI.
	// In adm_control.h, you have an enum that lists all custom controls.
	// A part of them are vehicle controls: car_acc, car_dec etc.

	// Basically, a player sends a key stroke or hold, and one of the bools
	// get turned to true. Whatever one of those bools become, true or false,
	// it will get copied to a seat's local controls record.

	// So, the seat is checking what controls the player activated. That's all.
	// In the think loop, the vehicle will do things depending on each of these
	// local seat bools. Hence, only one seat, or only one type of seat may control
	// the vehicle, i.e. the Driver seat type.

	if (m_iSeats)
	{
		for (int i = 0; i < m_iSeats; i++)
			v_Seats[i].ListenToCommands();
	}
}

void CBaseVehicle::SeatPlayerLocking()
{
	// This function will keep the players in sync with the respected seat positions.
	// Also, it will rotate the players according to our vehicle.
	
	for (int i = 0; i < m_iSeats; i++)
	{
		v_Seats[i].SeatPlayer();
		if (v_Seats[i].pSessilis)
			v_Seats[i].pSessilis->pev->angles = pev->angles;
	}
	//	v_Seat2.SeatPlayer(); etc.
}

void CBaseVehicle::SeatPositionLocking()
{
	if (v_Body.m_iszModel)
		for (int i = 0; i < m_iSeats; i++)
			v_Seats[i].AttachToPos(*this, m_iBoneOffset);

	else
		for (int i = 0; i < m_iSeats; i++)
			v_Seats[i].AttachToPos(v_Body.pos, 0, 0, 0);
}

void CBaseVehicle::SeatSwitch(VehicleSeat &seatFrom)
{
	if (seatFrom.iSitdex == (m_iSeats - 1))
	{
		for (int i = 0; i < m_iSeats; i++)
		{
			if (v_Seats[i].pSessilis != nullptr)
			{
				continue;
			}

			else
			{
				seatFrom.SeatSwitch(v_Seats[i]);
				break;
			}
		}
	}

	else
	{
		for (int i = seatFrom.iSitdex; i < m_iSeats; i++)
		{
			if (v_Seats[i].pSessilis != nullptr)
			{
				continue;
			}

			else
			{
				seatFrom.SeatSwitch(v_Seats[i]);
				break;
			}
		}
	}
}

void CBaseVehicle::VehicleExit(VehicleSeat *pSeat)
{ 
	// This is the vehicle exit function.
	// It works by accessing a seat, and setting the
	// pSessilis to NULL.
	// But, before setting to NULL, it also frees the
	// player in question, brings him back the control,
	// and throws him about 20 units outside of the 
	// vehicle, and flushes the commands.

	ALERT(at_console, "VehicleExit\n");
	pSeat->Exit();
}

void CBaseVehicle::VehicleBaseInit()
{
	if (v_Body.m_iszModel)
		pev->model = v_Body.m_iszModel;

	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->max_health = v_Engine.MaxHealth;
	pev->health = pev->max_health;

	v_Body.pos = pev->origin;
}

void CBaseVehicle::VehicleInit()
{
	// All the properties are initialised here.
	// Engine properties like HP, the drive, efficiency,
	// seats and their properties, and the body properties.

	// Heaven:
	// v_Body.Mass = Script_Read( "v_Body.mass" );

	v_Body.Mass = 30; // kg
	v_Body.Density = 670; // kg/m^3, Birch wood, European

	v_Seats[0].Init(Driver, v_Body.pos, 0);

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

/*
	VEHICLES

	Initialisers:

	v_Type: VehicleChair, VehicleCar, VehicleBike,
			VehiclePlane, VehicleHelicopter, VehicleShip,
			VehicleBoat, VehicleNPC, VehicleSpecialTest (VehicleType)

	m_iSeats (int, should match the number of seats initialised)
	m_iWheels (int, should match the number of wheels initialised)
	m_iBoneOffset (int, potentially different for each studio model)

	v_Body (VehicleBody)
	-	Mass (float)
	-	Density (float, use Density_ constants, e.g. Density_Iron)
	-	m_iszModel (string_t, if you are using a hardcoded model for this entity)
	
	v_Wheels[4] (VehicleWheel)
	-	Traction (float)
	-	Radius (float)
	-	Wear (float)
	-	Width (float)
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

LINK_ENTITY_TO_CLASS(vehicle_base, CVehiclePrototype);
LINK_ENTITY_TO_CLASS(vehicle_basep, CVehiclePrototype);

class CVehiclePrototypeTwoseat : public CBaseVehicle
{
public:

	void VehicleInit()
	{
		v_Body.Mass = 30; // kg
		v_Body.Density = 670; // kg/m^3, Birch wood, European

		m_iSeats = 2;

		v_Seats[0].Init(Driver, v_Body.pos, 0);
		v_Seats[1].Init(Passenger, v_Body.pos, 1);

		v_Type = VehicleSpecialTest;
	}
};

LINK_ENTITY_TO_CLASS(vehicle_proto_ms, CVehiclePrototypeTwoseat);

// This is literally ***EVERYTHING*** needed for the couch.
// The base takes care of ***EVERYTHING*** else. :DDDD

// BASE CAR CLASS

class CBaseCar : public CBaseVehicle
{
public:
	void	Spawn();
	void	KeyValue(KeyValueData *pkvd);

	void	VehicleBaseInit();
	void	VehicleMovement();
	void	VehicleThink();
	void	AlignToGround();
	void	WheelLocking();
//	void	ShootTraces();
//	void	HandleTraces();

//	int		TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	string_t m_iszWheels[32];

private:
	vec3_t	vecDesiredAng; // the angles we want to move to
	vec3_t	vecPushedAng; // if you hit a wall, this is where this variable comes into play, as it can potentially rotate you a bit
	vec3_t	vecCurrentAng; // the angle we are currently facing

	Vector vecLocalForward, vecLocalRight, vecLocalUp;
	TraceResult trUp, trDown, trFront, trBack, trLeft, trRight, trDiagonalFL, trDiagonalFR, trDiagonalBL, trDiagonalBR; // Essentially, we'll have 10 trace calls to determine proximity to stuff; up-down, left-right, front-back and 4 diagonals
	
};

void CBaseCar::Spawn()
{
	SET_MODEL(ENT(pev), (char*)STRING(pev->model));

	SetThink(NULL);

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BBOX;
	pev->friction = 0.01;
	pev->v_angle = pev->angles;

	VehicleInit(); // all the basic stuff goes here - wheels, engine, seats, weapons, sounds to use
	Precache(); // here we precache that stuff
	VehicleBaseInit(); // and here we apply the model, hence I had to precache between Init and BaseInit

	SetUse(&CBaseVehicle::VehicleUse);
	SetThink(&CBaseVehicle::VehicleThink);

	pev->nextthink = 1.5; // They said this is a magic number. :o
}

void CBaseCar::KeyValue(KeyValueData *pkvd)
{
	if (KeyvalueToken(wheel))
	{
		ALERT(at_console, "LOADIN");

		string_t path = ALLOC_STRING(pkvd->szValue);

		for (int i = 0; i < 31; i++)
		{
			v_Wheels[i].m_iszModel = path;
		}
	}

	else if (KeyvalueToken(wheel0))
	{
		KeyvalueToString(m_iszWheels[0]);
	}

	else if (KeyvalueToken(wheel1))
	{
		KeyvalueToString(m_iszWheels[1]);
	}

	else if (KeyvalueToken(wheel2))
	{
		KeyvalueToString(m_iszWheels[2]);
	}

	else if (KeyvalueToken(wheel3))
	{
		KeyvalueToString(m_iszWheels[3]);
	}

	else
	{
		KeyvaluesFromBase(CBaseVehicle);
	}
}

void CBaseCar::VehicleBaseInit()
{
	if (v_Body.m_iszModel)
		pev->model = v_Body.m_iszModel;

	SET_MODEL(ENT(pev), STRING(pev->model));

	pev->max_health = v_Engine.MaxHealth;
	pev->health = pev->max_health;

	v_Body.pos = pev->origin;
	UTIL_SetSizeAuto(pev);
}

void CBaseCar::VehicleThink()
{
	UTIL_MakeVectorsPrivate(pev->angles, vecLocalForward, vecLocalRight, vecLocalUp);

	float flVelocity = pev->velocity.Length();
	static Vector vecang = { 0, 0, 0 };
	static int cnt = 5; 

	v_Body.pos = pev->origin;
	pev->oldorigin = pev->origin;

	ListenToCommands(); // check which keys are being held by players, in each seat

	v_Engine.Update(); // update other components, synchronise their local variables
	v_Wheels[0].Update(flVelocity, *this, 0);
	v_Wheels[1].Update(flVelocity, *this, 1);
	v_Wheels[2].Update(flVelocity, *this, 2);
	v_Wheels[3].Update(flVelocity, *this, 3);

//	ShootTraces(); // trace collision, distance from ground etc.
//	HandleTraces(); // calculate that traced stuff
	AlignToGround(); // align the vehicle to the ground
	VehicleMovement(); // calculate vehicle movement
	SeatPositionLocking(); // "attach" the seats to the body
	SeatPlayerLocking(); // "attach" the players to the seats

	if (cnt == 0)
	{
		Vector vecMin, vecMax;

		for (int i = 0; i < m_iWheels; i++)
		{
			vecMin.x = -v_Wheels[i].Width;
			vecMin.y = -v_Wheels[i].Radius;
			vecMin.z = vecMin.y;

			vecMax.x = -vecMin.x;
			vecMax.y = -vecMin.y;
			vecMax.z = -vecMin.z;

			UTIL_SetSizeAuto(v_Wheels[i].m_pWheel->pev);
		}

		cnt--;
	}
	
	else if (cnt > 0)
	{
		cnt--;
	}

	for (int i = 0; i < m_iSeats; i++)
	{
		if (v_Seats[i].fCommands[bi_unuse] == true)
		{
			VehicleExit(&v_Seats[i]);
		}

		if (v_Seats[i].fCommands[bi_seatswitch] == true)
		{
			ALERT(at_console, "\nVehicleThink() - someone wants to switch seats");
			SeatSwitch(v_Seats[i]);
		}

		if (v_Seats[i].pSessilis)
		{
			FBitSet(v_Seats[i].pSessilis->pev->flags, FL_ONGROUND);
		}
	}

	for (int i = 0; i < m_iWheels; i++)
	{
		if (!v_Wheels[i].m_pWheel)
		{
			v_Wheels[i].m_pWheel = UTIL_FindEntityByTargetname(NULL, (char*)STRING(m_iszWheels[i]));
		}
	}
	
	vecang.y += (v_Wheels[0].SteerAngle + v_Wheels[1].SteerAngle) / 60;

//	ALERT(at_console, "\nvecang.y %f\tangles.y %f", vecang.y, pev->angles.y);

	pev->nextthink = gpGlobals->time + 0.016;
}

void CBaseCar::VehicleMovement()
{
	static Vector vecResultForce = { 0, 0, 0 };
	Vector vecForwardForce = { 0, 0, 0 };
	Vector vecDragForce = { 0, 0, 0 };
	Vector vecWeight = { 0, 0, 0 };
	Vector vecTraction = { 0, 0, 0 };
	Vector vecAngularVel = { 0, 0, 0 };

	Vector vecMoveDelta = pev->origin - pev->oldorigin;
	float flVelocity = vecMoveDelta.Length();
	float flTraction = v_Wheels[0].Traction + v_Wheels[1].Traction + v_Wheels[2].Traction + v_Wheels[3].Traction;
	flTraction /= 4;
	float flTractionFraction = flTraction / v_Wheels[0].originalTraction;

	static Vector vecDir;
	static Vector vecWheelSteer = { 0, 0, 0 };
	static Vector vecDeltaDir = { 0, 0, 0 };
	static Vector vecOldAngles;
	static Vector vecActualAngles = { 0, 0, 0 };
	static Vector vecWheelAngles = { 0, 0, 0 };

	vecDir = pev->angles;
	vecOldAngles = vecActualAngles;

	for (int i = 0; i < m_iSeats; i++)
	{
		if (v_Seats[i].fCommands[bi_acc] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			if (v_Engine.CurrentGear == -1 && v_Engine.Rpm < -100)
			{
				v_Engine.Brake(*this);
			}

			else
			{
				v_Engine.Accelerate(*this);
			}
		}

		if (v_Seats[i].fCommands[bi_dec] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			if (v_Engine.CurrentGear > 0)
			{
				v_Engine.Brake(*this);
			}

			else
			{
				v_Engine.CurrentGear = -1;
				v_Engine.Accelerate(*this);
			}
		}

		if (v_Seats[i].fCommands[bi_lt] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			v_Wheels[0].SteerLeft(flVelocity, *this);
			v_Wheels[1].SteerLeft(flVelocity, *this);
		}

		if (v_Seats[i].fCommands[bi_rt] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			v_Wheels[0].SteerRight(flVelocity, *this);
			v_Wheels[1].SteerRight(flVelocity, *this);
		}

		if (v_Seats[i].fCommands[bi_jump] && (v_Seats[i].type == Driver || v_Seats[i].type == DriverGunner))
		{
			v_Engine.Handbrake(*this);
		}
	}

	// STEERING ANGLES
	vecWheelSteer = (v_Wheels[0].steerangles + v_Wheels[1].steerangles) / 4;

	if (v_Engine.Torque > 0)
		vecDir = vecDir + vecWheelSteer;
	else if (v_Engine.Torque < 0)
		vecDir = vecDir - vecWheelSteer;
		
	pev->angles = vecDir;

//	vecDeltaDir = vecDir - vecOldAngles;
//	vecDeltaDir = vecDeltaDir * flTraction;
//	vecDir = vecDir + (vecDeltaDir * flTraction);
//	pev->angles = pev->angles + vecAngularVel;

//	vecActualAngles = (vecOldAngles * (1 - flTractionFraction)) + (vecDir * flTractionFraction);

	pev->angles = v_Wheels[0].angles;
	pev->angles = pev->angles + v_Wheels[1].angles;
	pev->angles = pev->angles + v_Wheels[2].angles;
	pev->angles = pev->angles + v_Wheels[3].angles;
	pev->angles = pev->angles / 4;

	vecForwardForce = vecForwardForce + v_Wheels[0].Force;
	vecForwardForce = vecForwardForce + v_Wheels[1].Force;
	vecForwardForce = vecForwardForce + v_Wheels[2].Force;
	vecForwardForce = vecForwardForce + v_Wheels[3].Force;
	vecForwardForce = vecForwardForce / 4;

	vecTraction = v_Wheels[0].TractionForce;
	vecTraction = vecTraction + v_Wheels[1].TractionForce;
	vecTraction = vecTraction + v_Wheels[2].TractionForce;
	vecTraction = vecTraction + v_Wheels[3].TractionForce;
	vecTraction = vecTraction / 4;

	vecForwardForce /= 1 + (v_Engine.BrakePedal * 0.1);
	vecDragForce = -0.12 * vecResultForce * flVelocity;

	vecResultForce = vecResultForce + vecForwardForce + vecDragForce;
	vecWeight.z = -v_Body.Mass * 9.81 / 30;

	pev->velocity = pev->velocity + ((vecResultForce + vecTraction + vecWeight) / (v_Body.Mass));
	vecResultForce = vecResultForce / 1.02;
	m_vecFinalForce = vecResultForce;

	pev->oldorigin = pev->origin;

/*	ALERT(at_console, "\n%s = %f\t%s = %f\t%s = %f\t%s = %f",
		"PEV", pev->angles.y,
		"vWA", vecWheelAngles.y,
		"vW.A", v_Wheels[0].angles.y,
		"W4T", v_Wheels[3].Traction / v_Wheels[3].originalTraction); */
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
			v_Wheels[i].AttachToPos(*this, i);
		}
	}
}

class CVehicleWheel : public CBaseEntity // it's actually a dummy entity
{
public:
	void Spawn()
	{
		SET_MODEL(ENT(pev), (char*)STRING(m_iszModel));
		SetThink(&CVehicleWheel::Think);
		return;
	}

	void KeyValue(KeyValueData *pkvd)
	{
		if (KeyvalueToken(mdl))
		{
			KeyvalueToString(m_iszModel);
		}

		else KeyvaluesFromBase(CBaseEntity);
	}

	void Use(CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value) { return; }
	void Think()
	{
		SUB_DoNothing();
		pev->nextthink = gpGlobals->time + 0.016;
	}

private:
	string_t m_iszModel;
};

LINK_ENTITY_TO_CLASS(vehicle_wheel, CVehicleWheel);

class CVehicleCarTest : public CBaseCar
{
public:
	void VehicleInit()
	{
		v_Body.Mass = 400;

		m_iSeats = 1;
		m_iWheels = 4;
		v_Seats[0].Init(Driver, v_Body.pos, 0);

		v_Engine.SetGearRatios
		(
			-4.5, 4.5, 
			4.0, 3.5, 
			3.0, 2.0, 
			0, 0
		);

		v_Engine.SetTorqueCurve
		(
			{ 40.0f,  000.0f  },
			{ 90.0f,  100.0f  },
			{ 170.0f, 1000.0f },
			{ 190.0f, 3200.0f },
			{ 250.0f,  5000.0f }
		);

		v_Engine.Init(Drive_AWD, 500, 70, 0.9);
		
		v_Type = VehicleCar;

		PRECACHE_MODEL((char*)STRING(v_Wheels[0].m_iszModel)); // DIRTY friggin hanck

		v_Wheels[0].Init(Stock, *this, m_iszWheels[0], (Wheel_Steerable | Wheel_Front));
		v_Wheels[1].Init(Stock, *this, m_iszWheels[1], (Wheel_Steerable | Wheel_Front));
		v_Wheels[2].Init(Stock, *this, m_iszWheels[2], (Wheel_Handbrake | Wheel_Back));
		v_Wheels[3].Init(Stock, *this, m_iszWheels[3], (Wheel_Handbrake | Wheel_Back));
	}
};

LINK_ENTITY_TO_CLASS(vehicle_bath, CVehicleCarTest);
