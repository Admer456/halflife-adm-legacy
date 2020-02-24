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

//#include <Eigen/Geometry>

#include "ExtDLL.h"
#include "Util.h"
#include "CBase.h"
#include "Player.h"
#include "SaveRestore.h"

#include "AdmVehicle.h"
#include "Animation.h"

void VehicleEngine::Damage(float hp)
{
	if (MadeInGermany)
		return; // engine was made in Germany, can't break

	Health -= hp;
	
	if (Health > MaxHealth)
		Health = MaxHealth;
}

int VehicleEngine::InstaRepair()
{
	if (Health == MaxHealth)
		return 0; // Already max

	Health = MaxHealth;
	return 1; // Repaired!
}

void VehicleEngine::SetGearRatios(float reverse, float first, float second, float third, float fourth, float fifth, float sixth, float seventh)
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

void VehicleEngine::SetTorqueCurve(VehicleTorqueCurve tc1, VehicleTorqueCurve tc2, VehicleTorqueCurve tc3, VehicleTorqueCurve tc4, VehicleTorqueCurve tc5)
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
	for (a = 0; a <= 9; a++)
	{
		if (GearRatios[a] > 0)
			result++;
	}

	return result;
}

float VehicleEngine::CalcTorque()
{
	int min = 0, max = 0;
	float retTorque, fraction = 0;

	for (min = 4; min >= 0; min--)
	{
		if (Rpm > TorqueCurve[min].Rpm)
		{
			break;
		}
	}

	if (min < 0)
		min = 0;

	if (min < 5)
		max = min + 1;

	float RpmMax = TorqueCurve[max].Rpm; // let's say 3200
	float RpmMin = TorqueCurve[min].Rpm; // let's say 1500, and Rpm is 2000

	// fraction = (2000 - 1500) / (3200 - 1500) = 500 / 1700 = about 0.3
	fraction = (Rpm - RpmMin) / (RpmMax - RpmMin);
	
	// Torque minimum = 250
	// Torque maximum = 270
	// retTorque = (250 * ( 1 - 0.3 )) + (270 * ( 0.3 ))
	// retTorque = (250 * 0.7) + (270 * 0.3)
	// retTorque = 175 + 81 = 256
	retTorque = (TorqueCurve[min].Torque * (1 - fraction)) + (TorqueCurve[max].Torque * fraction);
	
	// SOLVED DA BUG, it used to be fraction - 1 instead of 1 - fraction, HAHAHAAHAHAHHA

	// torque gets reset to 0 when you switch gears, 
	// so, let's actually accumulate them

	float sumTorque = 0;

	for ( int i = 0; i < CurrentGear; i++ )
	{
		sumTorque += TorqueCurve[ 4 ].Torque;
	}

	ALERT( at_console, "\nsumTorque %5.2f retTorque %5.2f gear %1d", sumTorque, retTorque, CurrentGear );

	return sumTorque + retTorque;
}

void VehicleEngine::Accelerate(CBaseVehicle &Vehicle)
{
//	Rpm += ((HorsePower * GearRatios[CurrentGear + 1]) / 20) * Efficiency;

	Pedal = (Pedal * 0.9) + (1.0 * 0.1);

	if (CurrentGear == 0)
	{
		CurrentGear++;
//		Rpm = minRpm;
	}

	if (CurrentGear == -1 && Rpm > -200)
	{
		CurrentGear++;
	}

	if(!FBitCheck(Flags, fEngine_GasHeld)) 
		FBitToggle(Flags, fEngine_GasHeld);
}

void VehicleEngine::Brake(CBaseVehicle &Vehicle)
{
	BrakePedal = (BrakePedal * 0.6) + (1.0 * 0.4);

	if (CurrentGear < 0)
	{
		if (Vehicle.v_Wheels[0].onGround)
		{
//			Rpm += (Vehicle.v_Wheels[0].Traction + 1.001);
//			Rpm += 0.01;
		}

		if (Vehicle.v_Wheels[1].onGround)
		{
//			Rpm += (Vehicle.v_Wheels[1].Traction + 1.001);
//			Rpm += 0.01;
		}

		ALERT(at_console, "WERE BRAKIN");
	}
	else
	{
		if (Vehicle.v_Wheels[0].onGround)
		{
//			Rpm -= (Vehicle.v_Wheels[0].Traction + 1.001) * 5;
//			Rpm -= 1.01;
		}

		if (Vehicle.v_Wheels[1].onGround)
		{
//			Rpm -= (Vehicle.v_Wheels[1].Traction + 1.001) * 5;
//			Rpm -= 1.01;
		}
	}

	if (!FBitCheck(Flags, fEngine_BrakeHeld))
		FBitToggle(Flags, fEngine_BrakeHeld);
}

void VehicleEngine::Handbrake(CBaseVehicle &Vehicle)
{
	HandbrakeLever = (HandbrakeLever * 0.9) + (1.0 * 0.1);

	if (!FBitCheck(Flags, fEngine_HBHeld))
		FBitToggle(Flags, fEngine_HBHeld);
 }

void VehicleEngine::Init(VehicleDrive Drive_XWD, float maxHealth, int horsepower, float efficiency, bool invincible, float minrpm, float maxrpm)
{
	Drive = Drive_XWD;
	MaxHealth = maxHealth;
	Health = maxHealth;
	HorsePower = horsepower;
	Efficiency = efficiency;
	MadeInGermany = invincible;

	Driving = 0;
	Rpm = 0;
	minRpm = minrpm;
	maxRpm = maxrpm;
	CurrentGear = 0;
	SlowDown = 0;
	Flags = 0;
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

	if (!FBitCheck(Flags, fEngine_GasHeld))
		Pedal /= 1.05;
	if (!FBitCheck(Flags, fEngine_BrakeHeld))
		BrakePedal /= 1.05;
	if (!FBitCheck(Flags, fEngine_HBHeld))
		HandbrakeLever /= 1.05;

	UTIL_LimitBetween(SlowDown,			0.2, 1.0);
	UTIL_LimitBetween(Pedal,			0.0, 1.0);
	UTIL_LimitBetween(BrakePedal,		0.0, 1.0);
	UTIL_LimitBetween(HandbrakeLever,	0.0, 1.0);

	Rpm += Pedal * (Efficiency * SlowDown * HorsePower * (GearRatios[CurrentGear+1])) * 0.065;
//	Rpm = Rpm * (1 - (BrakePedal * 0.98));

	// Gotta accelerate faster on lower rpms
	if ( Rpm < (maxRpm / 2.0f) && FBitCheck( Flags, fEngine_GasHeld ) )
		Rpm *= 1.3f;

	// Driving state
	if (!FBitCheck(Flags, fEngine_Running))
		Driving = Driving_Off;

	else if (Rpm < ((minRpm + maxRpm) / 2))
		Driving = Driving_Lowrpm;

	else if (Rpm > ((minRpm + maxRpm) / 2))
		Driving = Driving_Hirpm;

	else if (CurrentGear == -1)
		Driving = Driving_Reverse;

//	if (SlowDown)
//		Rpm *= SlowDown;

	if (CurrentGear != -1)
	{
		if ( Rpm > maxRpm && CurrentGear )
		{
			if ( FBitCheck( Flags, fEngine_GasHeld ) )
			{
				if ( CurrentGear < Gears - 1 )
				{
					CurrentGear++;
					Rpm = 0;
	//				Rpm = minRpm * 1.05;
				}
				else
				{
					Rpm = maxRpm;
				}
			}
 		}

		else if ( Rpm < minRpm )
		{
			if ( CurrentGear > 1 )
			{
				if ( !FBitCheck( Flags, fEngine_GasHeld ) )
				{
					CurrentGear--;

					if ( CurrentGear != 0 )
					{
						Rpm = maxRpm;
					}
				}
			}
			else if ( !FBitCheck( Flags, fEngine_GasHeld ) )
			{
				CurrentGear--;
			}
		}
	}
	else
	{
		if ( Rpm > -1.0f && FBitCheck( Flags, fEngine_GasHeld ) )
		{
			CurrentGear++;
		}
	}

	if ( !FBitCheck( Flags, fEngine_GasHeld ) )
		Rpm /= 1.02;

	if ( FBitCheck( Flags, fEngine_BrakeHeld ) )
		Rpm *= (0.3 * sqrt(1 - BrakePedal)) + 0.7; 

	Torque = CalcTorque();
	Torque -= Torque * (1-Pedal) * 0.8;

	WheelTorque = Torque /** GearRatios[CurrentGear+1]*/ * 5.0f;
	TrackTorque = TrackTorque * 0.995 + WheelTorque * 0.005;

	FBitClear(Flags, fEngine_ClutchHeld);
	FBitClear(Flags, fEngine_GasHeld);
	FBitClear(Flags, fEngine_BrakeHeld);
	FBitClear(Flags, fEngine_HBHeld);

	if ( FBitCheck( Flags, fEngine_GasHeld ) && WheelTorque < TrackTorque && CurrentGear != -1 )
	{
		Output += TrackTorque * 0.005;
		TrackTorque *= 1.005;
		Rpm *= 1.5f;
	}

	else
	{
		Output += WheelTorque * 0.005;
	}

//	Output *= Pedal;
	Output /= 1.02;

//	ALERT(at_console, "\nWheelTorque = %f\tOutput = %f\tPedal = %f\tRpm = %f", WheelTorque, Output, Pedal, Rpm);
	ALERT( at_console, "\nWheelTorque %5.2f TrackTorque %5.2f", WheelTorque, TrackTorque );

//	ALERT( at_console, "\nTorque %f \t Rpm %f \t Gear %d",
//		   Torque, Rpm, CurrentGear );

//	else if (Rpm < minRpm && !FBitCheck(Flags, fEngine_ClutchHeld))
	//	FBitClear(Flags, fEngine_Running);
}

int VehicleSeat::SeatPlayer()
{
	if (pSessilis == nullptr)
		return 0; // Fail, no sitting player

	UTIL_SetOrigin(pSessilis->pev, pos);
	pSessilis->pev->angles = angles;

	return 1; // Success
}

void VehicleSeat::SeatSwitch(VehicleSeat &nextSeat)
{
	if (pSessilis == nullptr)
	{
		ALERT(at_console, "\nSeatSwitch() fail at Seat%d - no sitting player", iSitdex);
		return; // crashy crashy gamey if this check don't worky
	}
	nextSeat.pSessilis = pSessilis;
	pSessilis = nullptr;

	FlushSeatCommands();

	ALERT(at_console, "\nSeat switched to Seat%d!", nextSeat.iSitdex);
}

void VehicleSeat::FlushSeatCommands() // NOT to be associated with TOILET FLUSHING, bastards - toilets are a separate entity
{
	fCommands[0] = false; // TBH I feel like it's a
	fCommands[1] = false; // microsecond faster to
	fCommands[2] = false; // execute through all of
	fCommands[3] = false; // these instead of actually
	fCommands[4] = false; // doing a loop thru the index
	fCommands[5] = false; // every time this happens.
	fCommands[6] = false;
	fCommands[7] = false; // :v
	fCommands[8] = false;
	fCommands[9] = false;
	fCommands[10] = false;
	fCommands[11] = false;
	fCommands[12] = false;
	fCommands[13] = false;
	fCommands[14] = false;
	fCommands[15] = false;
}

void VehicleSeat::ListenToCommands()
{
	if (pSessilis == nullptr)
	{
		// I'm deaf to all the commands. I ain't gonna listen.
		return;
	}

	// Standard for all seats - always listen to unuse and seatswitch
	fCommands[bi_unuse]			= pSessilis->GetKeyButton(vehicle_unuse);
	fCommands[bi_seatswitch]	= pSessilis->GetKeyButton(vehicle_seatswitch);

	if (type == Passenger)
	{
		// Don't listen to anything else and don't bother for the rest of the ifs
		return;
	}

	if (type == Driver || type == DriverGunner)
	{
		fCommands[bi_acc]			= pSessilis->pev->button & IN_FORWARD;
		fCommands[bi_dec]			= pSessilis->pev->button & IN_BACK;
		fCommands[bi_lt]			= pSessilis->pev->button & IN_MOVELEFT;
		fCommands[bi_rt]			= pSessilis->pev->button & IN_MOVERIGHT;

		fCommands[bi_up]			= pSessilis->GetKeyButton(vehicle_up);
		fCommands[bi_dn]			= pSessilis->GetKeyButton(vehicle_dn);
		fCommands[bi_llt]			= pSessilis->GetKeyButton(vehicle_llt);
		fCommands[bi_lrt]			= pSessilis->GetKeyButton(vehicle_lrt);

		fCommands[bi_hdbk]			= pSessilis->pev->button & IN_JUMP;
		fCommands[bi_cltc]			= pSessilis->GetKeyButton(vehicle_cltc);

		fCommands[bi_jump]			= pSessilis->pev->button & IN_JUMP;
		fCommands[bi_attack]		= pSessilis->pev->button & IN_ATTACK;

		fCommands[bi_flashlight]	= pSessilis->GetKeyButton(vehicle_flashlight);
		fCommands[bi_startstop]		= pSessilis->GetKeyButton(vehicle_startstop);
	}

	if (type == Gunner || type == DriverGunner)
	{
		fCommands[bi_shoot]		= pSessilis->pev->button & IN_ATTACK;
		fCommands[bi_shoot2]	= pSessilis->pev->button & IN_ATTACK2;
	}
}

void VehicleSeat::Exit()
{
	if (pSessilis == nullptr)
		return;

	FlushSeatCommands();
	pSessilis->pev->origin.z += 64;				// YEET!
	pSessilis->pev->movetype = MOVETYPE_WALK;
	pSessilis->pev->solid = SOLID_BBOX;
	pSessilis->m_InVehicle = InWalking;

	pSessilis = nullptr;
}

void VehicleSeat::Init(VehicleSeatType intype, vec3_t inpos, int sitdex)
{
	type = intype;
	fExists = true;
	pSessilis = nullptr;
	pos = inpos;
	pos.z -= 48;
	iSitdex = sitdex;

	FlushSeatCommands();
}

#define Deg2Rad (M_PI / 180)
#define Rad2Deg (180 / M_PI)
void VehicleSeat::AttachToPos(vec3_t targetpos, float radius, float pitch, float yaw)
{
	pos = targetpos;

	if (!radius)
		return;

	pos.x += radius * cos(pitch * Deg2Rad) * cos(yaw * Deg2Rad);
	pos.y += radius * cos(pitch * Deg2Rad) * sin(yaw * Deg2Rad);
	pos.z += radius * sin(pitch * Deg2Rad);
}

void VehicleSeat::AttachToPos(vec3_t targetpos, vec3_t offset, float radius, float pitch, float yaw)
{
	pos = targetpos;
	static bool fTurn = true;

	float r = offset.x; // Radius
	float pitch2 = offset.y; // Pitch
	float yaw2 = offset.z; // Yaw

	if (radius < 1.0f)
		return;

	pos.x += radius * cos(pitch * Deg2Rad) * cos(yaw * Deg2Rad);
	pos.y += radius * cos(pitch * Deg2Rad) * sin(yaw * Deg2Rad);
	pos.z += radius * sin(pitch * Deg2Rad);

/*	pos.x += (offset.Length() * cos((yaw + atan2(offset.x, offset.y)) * Deg2Rad) * cos((pitch + atan2(offset.x, offset.z)) * Deg2Rad));
	pos.y += (offset.Length() * sin((yaw + atan2(offset.x, offset.y)) * Deg2Rad) * cos((pitch + atan2(offset.z, offset.y)) * Deg2Rad));
	pos.z += (offset.Length() * cos((pitch + atan2(offset.x, offset.z)) * Deg2Rad)); */

	if (fTurn)
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, targetpos);
		WRITE_BYTE(TE_LINE);
		WRITE_COORD(targetpos.x); // coords
		WRITE_COORD(targetpos.y);
		WRITE_COORD(targetpos.z);
		WRITE_COORD(pos.x); // coords
		WRITE_COORD(pos.y);
		WRITE_COORD(pos.z);
		WRITE_SHORT(0.5); // life
		WRITE_BYTE(255); // R
		WRITE_BYTE(255); // G
		WRITE_BYTE(0); // B
		MESSAGE_END();
		fTurn = false;
	}

//	Eigen::Quaternionf qOriginal;
//	Eigen::Vector3f e_pos;

//	e_pos.x = offset.x; // Eigen vector = offset
//	e_pos.y = offset.y;
//	e_pos.z = offset.z;
//	qOriginal = e_pos; // Quaternion = vector

//	qOriginal = Eigen::AngleAxisf(pitch * Deg2Rad, e_pos); // Rotate the quaternion
//	qOriginal = Eigen::AngleAxisf(yaw * Deg2Rad, e_pos); 

//	e_pos.x = qOriginal.x;
//	e_pos.y = qOriginal.y;
//	e_pos.z = qOriginal.z;

//	pos.x += e_pos.x;
//	pos.y += e_pos.y;
//	pos.z += e_pos.z;

	vec3_t pos2 = pos;

	pos.x += r * cos((pitch2 + pitch) * Deg2Rad) * cos((yaw2 + yaw) * Deg2Rad);
	pos.y += r * cos((pitch2 + pitch) * Deg2Rad) * sin((yaw2 + yaw) * Deg2Rad);
	pos.z += r * sin((pitch2 + pitch) * Deg2Rad);

	if (!fTurn)
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, targetpos);
		WRITE_BYTE(TE_LINE);
		WRITE_COORD(pos.x); // coords
		WRITE_COORD(pos.y);
		WRITE_COORD(pos.z);
		WRITE_COORD(pos2.x); // coords
		WRITE_COORD(pos2.y);
		WRITE_COORD(pos2.z);
		WRITE_SHORT(0.5); // life
		WRITE_BYTE(255); // R
		WRITE_BYTE(255); // G
		WRITE_BYTE(0); // B
		MESSAGE_END();
		fTurn = true;
	}
}
#undef Deg2Rad

void VehicleSeat::AttachToPos(CBaseVehicle &Vehicle, int iBoneOffset)
{
	int iBone = iSitdex + iBoneOffset + Vehicle.m_iWheels;

	auto v_Type = Vehicle.GetVehType();

	if (iBone)
	{
		GetBoneId(iBone, v_Type);
		GET_BONE_POSITION(ENT(Vehicle.pev), iBone, pos, angles);
	}
}

void VehicleWheel::SteerLeft(float flSpeed, CBaseVehicle &Vehicle)
{
	SteerAngle += (1200 / (flSpeed + 100 - (10 * Wear))) / Width;
//	Vehicle.pev->angles.y += SteerAngle / 2;
}

void VehicleWheel::SteerRight(float flSpeed, CBaseVehicle &Vehicle)
{
	SteerAngle -= (1200 / (flSpeed + 100 + (10 * Wear))) / Width;
//	Vehicle.pev->angles.y += SteerAngle / 2;
}

void VehicleWheel::Init(RubberType rubbertype, CBaseVehicle &Vehicle, string_t iszWheel, int flags, float radius, float width)
{
	type = rubbertype;
	Flags = flags;
	Width = width;
	Radius = radius;

	switch (type)
	{
	case Stock:
		originalTraction = Width / (1.5 * Radius);
		break;
	
	case Comfort:
		originalTraction = Width / Radius;
		break;
	
	case Performance:
		originalTraction = (Width * 1.5) / Radius;
		break;

	case Rally:
		originalTraction = (Width * 2.25) / Radius;
		break;
	}

	originalTraction *= 3;

	onGround = true; // DEBUGGING until we add actual support for onground checking
	fExists = true;
	RollAngle = 0;
	SteerAngle = 0;
	Wear = 0;

	m_pWheel = UTIL_FindEntityByTargetname(&Vehicle, STRING(iszWheel));
}

void VehicleWheel::Update(float flSpeed, CBaseVehicle &Vehicle, int arrindex)
{
	static float angle = 0;
	static int dbg = 0;
	static Vector oldpos = { 0, 0, 0 };

	Vector vecDeltaMove = pos - oldpos;
	oldpos = pos;

	Vector vecForward, vecRight, vecUp;
	UTIL_MakeVectorsPrivate(angles, vecForward, vecRight, vecUp);

	angle += flSpeed / 60;

	if (FBitCheck(Flags, Wheel_Steerable))
		SteerAngle /= ((flSpeed / 6) * Width / 2000) + 1;

	if (m_pWheel != nullptr)
	{
		UTIL_SetOrigin(m_pWheel->pev, pos);
		m_pWheel->pev->angles = angles;

		void *pmodel = GET_MODEL_PTR(ENT(m_pWheel->pev));
		SetController(pmodel, m_pWheel->pev, 0, SteerAngle);
		SetController(pmodel, m_pWheel->pev, 1, angle);
	}

	AttachToPos(Vehicle, arrindex);

	if (FBitCheck(Flags, Wheel_Handbrake))
		Traction /= 1 + ((Vehicle.v_Engine.HandbrakeLever * 0.6) * (flSpeed * 0.01));

	Traction = (Traction * 0.97) + (originalTraction * 0.03); // Climb back slowly to original value

	if (Traction < 0.04)
	{
		Traction = 0.04; // Limit value
	}

	steerangles.y = SteerAngle;

	Force = (Vehicle.v_Engine.Output + 0.0001) * vecForward / Radius;
	Force *= (Traction / originalTraction) * (Traction / originalTraction);
	Force /= 2;

	float flFraction = DotProduct(Force, vecDeltaMove);
	float flDiffAngle = flFraction / (Force.Length() * vecDeltaMove.Length());
	flDiffAngle = std::acos(flDiffAngle);
	
	TractionForce = (0 + cos(flDiffAngle)) * Force.Length() * vecDeltaMove * Traction * 3.5;

	angles = Vehicle.pev->angles;
	angles = angles + steerangles;
	groundangles = AlignToGround(pos, angles, 256, Vehicle.edict());

//	if (!arrindex)
//		ALERT(at_console, "\nTraction powa: %f\nAngle between force and finalforce: %f", TractionForce.Length(), abs(cos(flDiffAngle)));


	switch (dbg)
	{
	case 0:
		UTIL_DbgDrawLine(Radius, pos, angles, 0);
		break;
	case 1:
		UTIL_DbgDrawLine((-Radius), pos, angles, 0);
		break; 
	case 2:
		UTIL_DbgDrawLine(Radius, pos, angles, 1);
		break;
	case 3:
		UTIL_DbgDrawLine((-Radius), pos, angles, 1);
		break;
	case 4:
		UTIL_DbgDrawLine(Radius, pos, angles, 2);
		break;
	case 5:
		UTIL_DbgDrawLine((-Radius), pos, angles, 2);
		break;
	}

	dbg++;
	if (dbg > 5)
		dbg = 0;
}

void VehicleWheel::AttachToPos(CBaseVehicle &Vehicle, int arrayindex)
{
	// Wheel 0 is at index 1, for example, so 0 + 1 is 1. Maths. uwu
	arrayindex++;

	GET_BONE_POSITION(ENT(Vehicle.pev), arrayindex, pos, angles);
}

void GetBoneId(int &iBone, VehicleType vType)
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

Vector AlignToGround(Vector origin, Vector dirangles, float radius, edict_t *pentIgnore)
{
	TraceResult trDown;

	UTIL_TraceLine(origin, origin - Vector(0, 0, 256), ignore_monsters, pentIgnore, &trDown);

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

	vecFinalAngles = UTIL_VecToAngles(angdir - DotProduct(angdir, trDown.vecPlaneNormal) * trDown.vecPlaneNormal);
	vecFinalAngles.z = -UTIL_VecToAngles(angdiry - DotProduct(angdiry, trDown.vecPlaneNormal) * trDown.vecPlaneNormal).x;

	return vecFinalAngles;
}
