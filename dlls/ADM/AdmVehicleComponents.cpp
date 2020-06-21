/*

======= AdmVehicleComponents.cpp ===========================

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

void VehicleEngine::Damage( float hp )
{
	if ( MadeInGermany )
		return; // engine was made in Germany, can't break

	Health -= hp;
	
	if (Health > MaxHealth)
		Health = MaxHealth;
}

int VehicleEngine::InstaRepair()
{
	if ( Health == MaxHealth )
		return 0; // Already max

	Health = MaxHealth;
	return 1; // Repaired!
}

void VehicleEngine::SetGearRatios( float reverse, float first, float second, float third, float fourth, float fifth, float sixth, float seventh )
{
	GearRatios[0] = reverse;
	GearRatios[1] = 1.000;
	GearRatios[2] = first;
	GearRatios[3] = second;
	GearRatios[4] = third;
	GearRatios[5] = fourth;
	GearRatios[6] = fifth;
	GearRatios[7] = sixth;
	GearRatios[8] = seventh;
}

void VehicleEngine::SetTorqueCurve( VehicleTorqueCurve tc1, VehicleTorqueCurve tc2, VehicleTorqueCurve tc3, VehicleTorqueCurve tc4, VehicleTorqueCurve tc5 )
{
	TorqueCurve[0] = tc1;
	TorqueCurve[1] = tc2;
	TorqueCurve[2] = tc3;
	TorqueCurve[3] = tc4;
	TorqueCurve[4] = tc5;
}

int VehicleEngine::GetGears()
{
	int a, result = 0;
	for ( a = 0; a <= 9; a++ )
	{
		if ( GearRatios[a] > 0 )
			result++;
	}

	return result;
}

float VehicleEngine::CalcTorque()
{
	int min = 0, max = 0;
	float retTorque, fraction = 0;

	for ( min = 4; min >= 0; min-- )
	{
		if ( rpm > TorqueCurve[min].rpm )
		{
			break;
		}
	}

	if ( min < 0 )
		min = 0;

	if ( min < 5 )
		max = min + 1;

	float RpmMax = TorqueCurve[max].rpm;
	float RpmMin = TorqueCurve[min].rpm;

	fraction = (rpm - RpmMin) / (RpmMax - RpmMin);
	
	retTorque = (TorqueCurve[min].torque * (1 - fraction)) + (TorqueCurve[max].torque * fraction);
	
	float sumTorque = 0;

	for ( int i = 0; i < CurrentGear; i++ )
	{
		sumTorque += TorqueCurve[ 4 ].torque;
	}

	return sumTorque + retTorque;
}

void VehicleEngine::Accelerate()
{
	Pedal = (Pedal * 0.9) + (1.0 * 0.1);

	if ( CurrentGear == 0 )
	{
		CurrentGear++;
	}

	if ( CurrentGear == -1 && rpm > -200 )
	{
		CurrentGear++;
	}

	if ( !FBitCheck( flags, EngineGasHeld ) )
		FBitToggle( flags, EngineGasHeld );
}

void VehicleEngine::Brake()
{
	BrakePedal = (BrakePedal * 0.6) + (1.0 * 0.4);

	if ( !FBitCheck( flags, EngineBrakeHeld ) )
		FBitToggle( flags, EngineBrakeHeld );
}

void VehicleEngine::Handbrake()
{
	HandbrakeLever = (HandbrakeLever * 0.9) + (1.0 * 0.1);

	if ( !FBitCheck( flags, EngineHandbrakeHeld ) )
		FBitToggle( flags, EngineHandbrakeHeld );
 }

void VehicleEngine::Init( CBaseVehicle* pParent, VehicleDrive Drive_XWD, float maxHealth, int horsepower, float efficiency, bool invincible, float minrpm, float maxrpm )
{
	parent = pParent;
	Drive = Drive_XWD;
	MaxHealth = maxHealth;
	Health = maxHealth;
	HorsePower = horsepower;
	Efficiency = efficiency;
	MadeInGermany = invincible;

	Driving = 0;
	rpm = 0;
	minRpm = minrpm;
	maxRpm = maxrpm;
	CurrentGear = 0;
	SlowDown = 0;
	flags = 0;
	Pedal = 0;
	BrakePedal = 0;
	HandbrakeLever = 0;
	Gears = GetGears();
}

void VehicleEngine::Update()
{
	static float TrackTorque = 0.0;

	SlowDown = Health / MaxHealth;

	if (SlowDown < 0.2)
		SlowDown = 0.2;

	if ( !FBitCheck( flags, EngineGasHeld ) )
		Pedal /= 1.05;
	if ( !FBitCheck( flags, EngineBrakeHeld ) )
		BrakePedal /= 1.05;
	if ( !FBitCheck( flags, EngineHandbrakeHeld ) )
		HandbrakeLever /= 1.05;

	UTIL_LimitBetween( SlowDown,		0.2, 1.0 );
	UTIL_LimitBetween( Pedal,			0.0, 1.0 );
	UTIL_LimitBetween( BrakePedal,		0.0, 1.0 );
	UTIL_LimitBetween( HandbrakeLever,	0.0, 1.0 );

	rpm += Pedal * (Efficiency * SlowDown * HorsePower * (GearRatios[CurrentGear+1])) * 0.065;

	// Gotta accelerate faster on lower rpms
	if ( rpm < (maxRpm / 2.0f) && FBitCheck( flags, EngineGasHeld ) )
		rpm *= 1.3f;

	// Driving state
	if ( !FBitCheck( flags, EngineRunning ) )
		Driving = DrivingOff;

	else if ( rpm < ((minRpm + maxRpm) / 2) )
		Driving = DrivingLowRpm;

	else if ( rpm > ( (minRpm + maxRpm) / 2 ) )
		Driving = DrivingHighRpm;

	else if ( CurrentGear == -1 )
		Driving = DrivingReverse;

	if ( CurrentGear != -1 )
	{
		if ( rpm > maxRpm && CurrentGear )
		{
			if ( FBitCheck( flags, EngineGasHeld ) )
			{
				if ( CurrentGear < Gears - 1 )
				{
					CurrentGear++;
					rpm = 0;
				}
				else
				{
					rpm = maxRpm;
				}
			}
		}

		else if ( rpm < minRpm )
		{
			if ( CurrentGear > 1 )
			{
				if ( !FBitCheck( flags, EngineGasHeld ) )
				{
					CurrentGear--;

					if ( CurrentGear != 0 )
					{
						rpm = maxRpm;
					}
				}
			}
			else if ( !FBitCheck( flags, EngineGasHeld ) )
			{
				CurrentGear--;
			}
		}
	}
	else
	{
		if ( rpm > -1.0f && FBitCheck( flags, EngineGasHeld ) )
		{
			CurrentGear++;
		}
	}

	if ( !FBitCheck( flags, EngineGasHeld ) )
		rpm /= 1.02;

	if ( FBitCheck( flags, EngineBrakeHeld ) )
		rpm *= (0.3 * sqrt( 1 - BrakePedal )) + 0.7;

	torque = CalcTorque();
	torque -= torque * (1-Pedal) * 0.8;

	WheelTorque = torque * 5.0f;
	TrackTorque = TrackTorque * 0.995 + WheelTorque * 0.005;

	FBitClear( flags, EngineClutchHeld );
	FBitClear( flags, EngineGasHeld );
	FBitClear( flags, EngineBrakeHeld );
	FBitClear( flags, EngineHandbrakeHeld );

	if ( FBitCheck( flags, EngineGasHeld ) && WheelTorque < TrackTorque && CurrentGear != -1 )
	{
		Output += TrackTorque * 0.005;
		TrackTorque *= 1.005;
		rpm *= 1.5f;
	}

	else
	{
		Output += WheelTorque * 0.005;
	}

	Output /= 1.02;
}

int VehicleSeat::SeatPlayer()
{
	if ( pSessilis == nullptr )
		return 0; // Fail, no sitting player

	UTIL_SetOrigin( pSessilis->pev, origin );
	pSessilis->pev->angles = angles;

	return 1; // Success
}

void VehicleSeat::SeatSwitch(VehicleSeat& nextSeat)
{
	if ( pSessilis == nullptr )
	{
		return;
	}

	nextSeat.pSessilis = pSessilis;
	pSessilis = nullptr;

	FlushSeatCommands();
}

void VehicleSeat::FlushSeatCommands() // NOT to be associated with TOILET FLUSHING, bastards - toilets are a separate entity
{
	commands = 0;
}

void VehicleSeat::ListenToCommands()
{
	if ( pSessilis == nullptr )
	{
		// I'm deaf to all the commands. I ain't gonna listen.
		return;
	}

	// Standard for all seats - always listen to unuse and seatswitch
	SetCommandBit( VehDismount,			pSessilis->GetKeyButton( vehicle_unuse ) );
	SetCommandBit( VehSwitchSeats,		pSessilis->GetKeyButton( vehicle_seatswitch ) );

	if ( type == Passenger )
	{
		// Don't listen to anything else and don't bother for the rest of the ifs
		return;
	}

	if ( type == Driver || type == DriverGunner )
	{
		SetCommandBit( VehAcceleration, pSessilis->pev->button & IN_FORWARD );
		SetCommandBit( VehDeceleration, pSessilis->pev->button & IN_BACK );
		SetCommandBit( VehLeft,			pSessilis->pev->button & IN_MOVELEFT );
		SetCommandBit( VehRight,		pSessilis->pev->button & IN_MOVERIGHT );
		SetCommandBit( VehUp,			pSessilis->GetKeyButton( vehicle_up ) );
		SetCommandBit( VehDown,			pSessilis->GetKeyButton( vehicle_dn ) );
		SetCommandBit( VehLeanLeft,		pSessilis->GetKeyButton( vehicle_llt ) );
		SetCommandBit( VehLeanRight,	pSessilis->GetKeyButton( vehicle_lrt ) );
		SetCommandBit( VehHandbrake,	pSessilis->pev->button & IN_JUMP );
		SetCommandBit( VehClutch,		pSessilis->GetKeyButton( vehicle_cltc ) );
		SetCommandBit( VehJump,			pSessilis->pev->button & IN_JUMP );
		SetCommandBit( VehAttack,		pSessilis->pev->button & IN_ATTACK );
		SetCommandBit( VehFlashlight,	pSessilis->GetKeyButton( vehicle_flashlight ) );
		SetCommandBit( VehToggleEngine, pSessilis->GetKeyButton( vehicle_startstop ) );
	}

	if (type == Gunner || type == DriverGunner)
	{
		SetCommandBit( VehShoot,		pSessilis->pev->button & IN_ATTACK );
		SetCommandBit( VehShootAlternate, pSessilis->pev->button & IN_ATTACK2 );
	}
}

bool VehicleSeat::CheckCommandBit( int& offset )
{
	return commands & BIT( offset );
}

void VehicleSeat::SetCommandBit( const int& offset, const bool& state )
{
	if ( state )
		commands |= BIT( offset );
	else
		commands &= ~BIT( offset );
}

void VehicleSeat::Exit()
{
	if ( pSessilis == nullptr )
		return;

	FlushSeatCommands();
	pSessilis->pev->origin.z += 64;				// YEET!
	pSessilis->pev->movetype = MOVETYPE_WALK;
	pSessilis->pev->solid = SOLID_BBOX;
	pSessilis->m_InVehicle = InWalking;

	pSessilis = nullptr;
}

void VehicleSeat::Init(CBaseVehicle* pParent, VehicleSeatType intype, Vector inpos, int sitdex)
{
	parent = pParent;
	type = intype;
	fExists = true;
	pSessilis = nullptr;
	origin = inpos;
	origin.z -= 48;
	iSitdex = sitdex;

	FlushSeatCommands();
}

// This is crap
// This must be rewritten xd
#define Deg2Rad (M_PI / 180)
#define Rad2Deg (180 / M_PI)
void VehicleSeat::AttachToPos( Vector targetpos, float radius, float pitch, float yaw )
{
	origin = targetpos;

	if ( !radius )
		return;

	origin.x += radius * cos(pitch * Deg2Rad) * cos(yaw * Deg2Rad);
	origin.y += radius * cos(pitch * Deg2Rad) * sin(yaw * Deg2Rad);
	origin.z += radius * sin(pitch * Deg2Rad);
}

void VehicleSeat::AttachToPos( Vector targetpos, Vector offset, float radius, float pitch, float yaw )
{
	origin = targetpos;
	static bool fTurn = true;

	float r = offset.x; // Radius
	float pitch2 = offset.y; // Pitch
	float yaw2 = offset.z; // Yaw

	if ( radius < 1.0f )
		return;

	origin.x += radius * cos(pitch * Deg2Rad) * cos(yaw * Deg2Rad);
	origin.y += radius * cos(pitch * Deg2Rad) * sin(yaw * Deg2Rad);
	origin.z += radius * sin(pitch * Deg2Rad);

/*	origin.x += (offset.Length() * cos((yaw + atan2(offset.x, offset.y)) * Deg2Rad) * cos((pitch + atan2(offset.x, offset.z)) * Deg2Rad));
	origin.y += (offset.Length() * sin((yaw + atan2(offset.x, offset.y)) * Deg2Rad) * cos((pitch + atan2(offset.z, offset.y)) * Deg2Rad));
	origin.z += (offset.Length() * cos((pitch + atan2(offset.x, offset.z)) * Deg2Rad)); */

	if ( fTurn )
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, targetpos );
		WRITE_BYTE( TE_LINE );
		WRITE_COORD( targetpos.x ); // coords
		WRITE_COORD( targetpos.y );
		WRITE_COORD( targetpos.z );
		WRITE_COORD( origin.x ); // coords
		WRITE_COORD( origin.y );
		WRITE_COORD( origin.z );
		WRITE_SHORT( 0.5 ); // life
		WRITE_BYTE( 255 ); // R
		WRITE_BYTE( 255 ); // G
		WRITE_BYTE( 0 ); // B
		MESSAGE_END();
		fTurn = false;
	}

	vec3_t pos2 = origin;

	origin.x += r * cos((pitch2 + pitch) * Deg2Rad) * cos((yaw2 + yaw) * Deg2Rad);
	origin.y += r * cos((pitch2 + pitch) * Deg2Rad) * sin((yaw2 + yaw) * Deg2Rad);
	origin.z += r * sin((pitch2 + pitch) * Deg2Rad);

	if ( !fTurn )
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, targetpos );
		WRITE_BYTE( TE_LINE );
		WRITE_COORD( origin.x ); // coords
		WRITE_COORD( origin.y );
		WRITE_COORD( origin.z );
		WRITE_COORD( pos2.x ); // coords
		WRITE_COORD( pos2.y );
		WRITE_COORD( pos2.z );
		WRITE_SHORT( 0.5 ); // life
		WRITE_BYTE( 255 ); // R
		WRITE_BYTE( 255 ); // G
		WRITE_BYTE( 0 ); // B
		MESSAGE_END();
		fTurn = true;
	}
}
#undef Deg2Rad

void VehicleSeat::AttachToPos( int iBoneOffset )
{
	int iBone = iSitdex + iBoneOffset + parent->GetWheels();

	auto v_Type = parent->GetVehType();

	if (iBone)
	{
		GetBoneId( iBone, v_Type );
		GET_BONE_POSITION( ENT(parent->pev), iBone, origin, angles );
	}
}

void VehicleWheel::SteerLeft( float flSpeed )
{
	steerAngle += (1200 / (flSpeed + 100 - (10 * wear))) / width;
}

void VehicleWheel::SteerRight( float flSpeed )
{
	steerAngle -= (1200 / (flSpeed + 100 + (10 * wear))) / width;
}

void VehicleWheel::Init(RubberType rubbertype, CBaseVehicle* pParent, string_t iszWheel, int flags, float radius, float width)
{
	type = rubbertype;
	parent = pParent;
	flags = flags;
	width = width;
	radius = radius;

	force = g_vecZero;
	tractionForce = g_vecZero;

	switch (type)
	{
	case Stock:
		originalTraction = width / (1.5 * radius);
		break;
	
	case Comfort:
		originalTraction = width / radius;
		break;
	
	case Performance:
		originalTraction = (width * 1.5) / radius;
		break;

	case Rally:
		originalTraction = (width * 2.25) / radius;
		break;
	}

	originalTraction *= 3;

	onGround = true; // DEBUGGING until we add actual support for onground checking
	fExists = true;
	rollAngle = 0;
	steerAngle = 0;
	wear = 0;

	m_pWheel = UTIL_FindEntityByTargetname( NULL, STRING( iszWheel ) );
}

void VehicleWheel::Update( float flSpeed, int arrindex )
{
	static float angle = 0;
	static Vector oldpos = g_vecZero;

	Vector vecDeltaMove = origin - oldpos;
	oldpos = origin;

	Vector vecForward, vecRight, vecUp;
	UTIL_MakeVectorsPrivate(angles, vecForward, vecRight, vecUp);

	angle += flSpeed / 60;

	if (FBitCheck(flags, Wheel_Steerable))
		steerAngle /= ((flSpeed / 6) * width / 2000) + 1;

	if (m_pWheel != nullptr)
	{
		UTIL_SetOrigin(m_pWheel->pev, origin);
		m_pWheel->pev->angles = angles;

		void *pmodel = GET_MODEL_PTR(ENT(m_pWheel->pev));
		SetController(pmodel, m_pWheel->pev, 0, steerAngle);
		SetController(pmodel, m_pWheel->pev, 1, angle);
	}

	AttachToPos( arrindex );

	if (FBitCheck(flags, Wheel_Handbrake))
		traction /= 1 + ((parent->GetEngine().HandbrakeLever * 0.6) * (flSpeed * 0.01));

	traction = (traction * 0.97) + (originalTraction * 0.03); // Climb back slowly to original value

	traction = V_max( traction, 0.04 ); // minimum value for the traction is 0.04, else weird shit happens while steering

	steerangles.y = steerAngle;

	force = (parent->GetEngine().Output + 0.0001) * vecForward / radius;
	force *= (traction / originalTraction) * (traction / originalTraction);
	force /= 2;

	float flFraction = DotProduct(force, vecDeltaMove);
	float flDiffAngle = flFraction / (force.Length() * vecDeltaMove.Length());
	flDiffAngle = std::acos(flDiffAngle);
	
	tractionForce = (0 + cos(flDiffAngle)) * force.Length() * vecDeltaMove * traction * 3.5;

	angles = parent->pev->angles;
	angles = angles + steerangles;
	groundangles = AlignToGround(origin, angles, 256, parent->edict());
}

// Function will be deprecated soon, we should attach to bones by name, not by ID
void VehicleWheel::AttachToPos( int arrayindex )
{
	arrayindex++;
	GET_BONE_POSITION( ENT( parent->pev ), arrayindex, origin, angles );
}

void GetBoneId( int &iBone, VehicleType vType )
{
	switch (vType)
	{
	case VehicleCar:
		iBone += 1; // 1 bone for the steering wheel + m_iWheels usually 4
		break;

	case VehicleBike:
		iBone += 1; // engine/pedal bone + m_iWheels usually 2
		break;

	case VehiclePlane:
		iBone += 4; // 2 propeller bones + 2 gear

	case VehicleHelicopter:
		iBone += 2; // 2 propeller bones
		break;
		
	case VehicleBoat:
		iBone += 1; // 1 propeller bone
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
}

Vector AlignToGround( Vector origin, Vector dirangles, float radius, edict_t *pentIgnore )
{
	TraceResult trDown;

	UTIL_TraceLine( origin, origin - Vector( 0, 0, 256 ), ignore_monsters, pentIgnore, &trDown );

	Vector vecAngles, vecFinalAngles, angdir, angdiry;

	vecAngles = dirangles;

	UTIL_Deg2Rad(vecAngles.x);
	UTIL_Deg2Rad(vecAngles.y);
	UTIL_Deg2Rad(vecAngles.z);

	angdir.x = cos(vecAngles.y) * cos(vecAngles.x);
	angdir.y = sin(vecAngles.y) * cos(vecAngles.x);
	angdir.z = -sin(vecAngles.z);

	angdiry.x = sin(vecAngles.y) * cos(vecAngles.x);
	angdiry.y = sin(vecAngles.y) * cos(vecAngles.x);
	angdiry.z = -sin(vecAngles.x);

	vecFinalAngles = UTIL_VecToAngles( angdir - DotProduct( angdir, trDown.vecPlaneNormal ) * trDown.vecPlaneNormal);
	vecFinalAngles.z = -UTIL_VecToAngles( angdiry - DotProduct( angdiry, trDown.vecPlaneNormal ) * trDown.vecPlaneNormal).x;

	return vecFinalAngles;
}
