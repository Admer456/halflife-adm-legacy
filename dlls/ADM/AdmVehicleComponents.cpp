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

	if ( !FBitCheck( flags, BIT(EngineGasHeld) ) )
		FBitToggle( flags, BIT(EngineGasHeld) );
}

void VehicleEngine::Brake()
{
	BrakePedal = (BrakePedal * 0.6) + (1.0 * 0.4);

	if ( !FBitCheck( flags, BIT(EngineBrakeHeld) ) )
		FBitToggle( flags, BIT(EngineBrakeHeld) );
}

void VehicleEngine::Handbrake()
{
	HandbrakeLever = (HandbrakeLever * 0.9) + (1.0 * 0.1);

	if ( !FBitCheck( flags, BIT(EngineHandbrakeHeld) ) )
		FBitToggle( flags, BIT(EngineHandbrakeHeld) );
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

	if ( !FBitCheck( flags, BIT(EngineGasHeld) ) )
		Pedal /= 1.05;
	if ( !FBitCheck( flags, BIT(EngineBrakeHeld) ) )
		BrakePedal /= 1.05;
	if ( !FBitCheck( flags, BIT(EngineHandbrakeHeld) ) )
		HandbrakeLever /= 1.05;

	UTIL_LimitBetween( SlowDown,		0.2, 1.0 );
	UTIL_LimitBetween( Pedal,			0.0, 1.0 );
	UTIL_LimitBetween( BrakePedal,		0.0, 1.0 );
	UTIL_LimitBetween( HandbrakeLever,	0.0, 1.0 );

	rpm += Pedal * (Efficiency * SlowDown * HorsePower * (GearRatios[CurrentGear+1])) * 0.016;

	// Gotta accelerate faster on lower rpms
	if ( rpm < (maxRpm / 2.0f) && FBitCheck( flags, BIT(EngineGasHeld) ) )
		rpm *= 1.3f;

	// Driving state
	if ( !FBitCheck( flags, BIT(EngineRunning) ) )
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
			if ( FBitCheck( flags, BIT(EngineGasHeld) ) )
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
				if ( !FBitCheck( flags, BIT(EngineGasHeld) ) )
				{
					CurrentGear--;

					if ( CurrentGear != 0 )
					{
						rpm = maxRpm;
					}
				}
			}
			else if ( !FBitCheck( flags, BIT(EngineGasHeld) ) )
			{
				CurrentGear--;
			}
		}
	}
	else
	{
		if ( rpm > -1.0f && FBitCheck( flags, BIT(EngineGasHeld) ) )
		{
			CurrentGear++;
		}
	}

	if ( !FBitCheck( flags, BIT(EngineGasHeld) ) )
		rpm /= 1.02;

	if ( FBitCheck( flags, BIT(EngineBrakeHeld) ) )
		rpm *= (0.3 * sqrt( 1 - BrakePedal )) + 0.7;

	torque = CalcTorque();
	torque -= torque * (1-Pedal) * 0.8;

	WheelTorque = torque * 5.0f;
	TrackTorque = TrackTorque * 0.995 + WheelTorque * 0.005;

	ALERT( at_aiconsole, "Gear: %i\n", CurrentGear );

	FBitClear( flags, BIT(EngineClutchHeld) );
	FBitClear( flags, BIT(EngineGasHeld) );
	FBitClear( flags, BIT(EngineBrakeHeld) );
	FBitClear( flags, BIT(EngineHandbrakeHeld) );

	if ( FBitCheck( flags, BIT(EngineGasHeld) ) && WheelTorque < TrackTorque && CurrentGear != -1 )
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
	steerAngle += (400 / (flSpeed + 100 - (10 * wear))) / width;
}

void VehicleWheel::SteerRight( float flSpeed )
{
	steerAngle -= (400 / (flSpeed + 100 + (10 * wear))) / width;
}

void VehicleWheel::Init(RubberType rubbertype, CBaseVehicle* pParent, string_t iszWheel, int flags, float radius, float width)
{
	origin = g_vecZero;
	oldOrigin = origin;

	type = rubbertype;
	parent = pParent;
	this->flags = flags;
	this->width = width;
	this->radius = radius;

	//force = g_vecZero;
	tractionForce = g_vecZero;

	traction = 1.0f;

	switch (type)
	{
	case Stock:
		originalTraction = width / (1.5f * radius);
		break;
	
	case Comfort:
		originalTraction = width / radius;
		break;
	
	case Performance:
		originalTraction = (width * 1.5f) / radius;
		break;

	case Rally:
		originalTraction = (width * 2.25f) / radius;
		break;
	}

	originalTraction *= 3;
	tractionFactorBlend = 1.0f;

	onGround = true; // DEBUGGING until we add actual support for onground checking
	fExists = true;
	rollAngle = 0;
	steerAngle = 0;
	wear = 0;

	m_pWheel = UTIL_FindEntityByTargetname( NULL, STRING( iszWheel ) );
}

void VehicleWheel::Update( float flSpeed, int arrindex )
{
	Vector vecDeltaMove = (origin - oldOrigin) * 10.f;
	oldOrigin = origin;

	//if ( arrindex == 0 )
	//	ALERT( at_console, "origin %3.2f oldpos %3.2f vecDeltaMove %3.2f\n", origin.x, oldOrigin.x, vecDeltaMove.x );
	
	Vector vecForward, vecRight, vecUp;
	UTIL_MakeVectorsPrivate(angles, vecForward, vecRight, vecUp);

	// Determine whether this wheel can accelerate
	bool canDrive = false;

	VehicleDrive drive = parent->GetEngine().Drive;
	if ( drive == Drive_FWD && flags & BIT( Wheel_Front ) )
	{
		canDrive = true;
	}
	else if ( drive == Drive_RWD && flags & BIT( Wheel_Back ) )
	{
		canDrive = true;
	}
	else if ( drive == Drive_AWD )
	{
		canDrive = true;
	}

	if ( canDrive )
	{
		// Calculate the force according to these rules:
		// Bigger engine output = more force
		// More radius = less force
		// More traction = more force squared
		rollAngularVelocity = parent->GetEngine().Output / radius;
		rollAngularVelocity *= parent->GetEngine().Pedal;
	}
	else
	{
		rollAngularVelocity = vecDeltaMove.Length() / 11.f;
	}

	// Decrease traction a lot if we pulled the handbrake
	if ( FBitCheck( flags, BIT( Wheel_Handbrake ) ) )
	{
		float handbrake = parent->GetEngine().HandbrakeLever;
		if ( handbrake > 0.4 )
		{
			rollAngularVelocity *= 1.0f - handbrake;
		}
	}

	// Decrease traction while braking
	if ( flags & BIT( Wheel_Brake ) )
	{
		float brake = parent->GetEngine().BrakePedal;
		if ( brake > 0.4 )
		{
			rollAngularVelocity /= 1.05f;
		}
	}

	// Calculate the friction between the tyre and the floor
	// 3.788 is the constant i.e. the relation between our "angular velocity" of the wheel
	// and the actual velocity of the vehicle
	float contactDifference = rollAngularVelocity/2.0f - DotProduct( vecForward, vecDeltaMove);
	contactDifference *= 10.f;
	//float contactDifference = DotProduct( vecForward*rollAngularVelocity/6.66f, vecForward*vecDeltaMove.Length() );
	//contactDifference = fabs( contactDifference );

	// If the difference is greater than the threshold, then start sliding
	float contactTraction = 1.f;
	float contactThreshold = flSpeed * 0.35f;
	if ( fabs( contactDifference ) > contactThreshold )
	{
		contactTraction = contactThreshold / fabs(contactDifference);
		contactTraction = sqrt( contactTraction );

		contactTraction = max( contactTraction, 0.20f );

		ALERT( at_aiconsole, "TRACTION LOST, SKIDDING\n" );
	}
	else
	{
		ALERT( at_aiconsole, "Traction is stable\n" );
	}

	//if ( arrindex == 3 )
	//	ALERT( at_console, "contactDifference %3.2f contactThreshold %3.2f\n", contactDifference, contactThreshold );

	// Calculate the side friction in case the tyre is moving sideways along the floor (e.g. rear wheels while drifting)
	// If sideContact is 1.0, then the wheel is going 100% right, if -1.0, then 100% left
	// If 0.0, then it's either forward or backward
	// The epsilon for |sideContact| is at least 0.1
	float sideContact = DotProduct( vecRight * vecDeltaMove.Length(), vecDeltaMove ) * 20.f;
	float sideSlide = 0.f;

	sideContact /= (tractionFactorBlend*9.0f) + 1.0f;
	sideContact /= flSpeed * 0.25f + 1.0f;

	if ( vecDeltaMove.Length() < 0.5f )
		sideContact = 0.0f;

	UTIL_LimitBetween( sideContact, -120.0f, 120.0f );

	if ( fabs( sideContact ) > 20.0f )
	{
		if ( sideContact < 0.0f )
			sideSlide = sideContact + 20.f;
		else
			sideSlide = sideContact - 20.f;

		sideSlide /= 10.f;
	}

	ALERT( at_aiconsole, "slide %f\n", sideSlide );

	//ALERT( at_console, "sideContact %3.2f\n", sideContact );

	// Calculate a traction factor
	float tractionFactor = traction / originalTraction;
	tractionFactor *= contactTraction;

	//if ( arrindex <= 2 )
	//	tractionFactor = 0.1f;

	// If the angle between the wheel force and current velocity is 90°
	// then there will hardly be any fraction
	// If the angle is 0°, we have 100% traction
	tractionForce = vecForward * rollAngularVelocity * tractionFactorBlend;
	//tractionForce = tractionForce + vecDeltaMove * (1.0f - tractionFactorBlend) * 0.15f;
	tractionForce = tractionForce + vecRight * sideSlide * 0.25f /** (1.0f - tractionFactorBlend)*/;

	if ( tractionForce.Length2D() < 0.1f )
		tractionForce = g_vecZero;

	// Climb back slowly to original value
	traction = (traction * 0.9) + (originalTraction * 0.1);
	tractionFactorBlend = tractionFactorBlend*0.92 + tractionFactor*0.08;

	// Minimum value for the traction is 0.07, else weird shit happens while steering
	traction = V_max( traction, 0.07 );

	// Calculate angle for the axle bone controller
	if ( fabs( rollAngularVelocity ) > 0.2f )
		rollAngle += (rollAngularVelocity / 10.f);
	
	if ( rollAngle > 360.f )
		rollAngle -= 360.f;

	// Calculate the steering angle
	if ( FBitCheck( flags, BIT( Wheel_Steerable ) ) )
	{
		// Less traction = easier to steer^2
		// Wider tyres = harder to steer
		float decrease = width / 40.f;
		decrease *= tractionFactorBlend * tractionFactorBlend;
		steerAngle /= decrease + 1;
	}

	// Update the visual model
	if ( m_pWheel != nullptr )
	{
		UTIL_SetOrigin( m_pWheel->pev, origin );
		m_pWheel->pev->angles = angles;

		void *pmodel = GET_MODEL_PTR( ENT( m_pWheel->pev ) );
		SetController( pmodel, m_pWheel->pev, 0, steerAngle * 24.0f );
		SetController( pmodel, m_pWheel->pev, 1, rollAngle );
	}

	steerangles.y = steerAngle;

	angles = parent->pev->angles;
	angles = angles + steerangles;
	groundangles = AlignToGround( origin, angles, 256, parent->edict() );

//	ALERT( at_console, "flSpeed %f\n", flSpeed );

	AttachToPos( arrindex );
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

void AxlePhysicsParams::Init( VehicleWheel* left, VehicleWheel* right, VehiclePhysicsParams* params, float offset )
{
	direction = g_vecZero;
	position = g_vecZero;
	force = g_vecZero;

	this->offset = offset;
	momentum = g_vecZero;

	leftWheel = left;
	rightWheel = right;
	baseParams = params;
}

void AxlePhysicsParams::Update()
{
	Vector forward;
	UTIL_MakeVectorsPrivate( baseParams->parent->pev->angles, forward, nullptr, nullptr );

	// Degradation of momentum due to air drag and traction
	momentum = momentum / 1.001f;

	traction =  leftWheel->traction  / leftWheel->originalTraction;
	traction += rightWheel->traction / rightWheel->originalTraction;
	traction /= 2.0f;

	UTIL_LimitBetween( traction, 0.5, 1.0 );
	traction = fabs( traction );

	// Physically, the traction force is what actually pushes the car
	force = leftWheel->tractionForce + rightWheel->tractionForce;

	// Multiply force and momentum by 0.016f otherwise we'll skyrocket
	position = baseParams->parent->pev->origin + (offset * forward) + (force + momentum);

	// Update momentum for next frame
	momentum = force;

	//ALERT( at_console, "force %3.2f momentum %3.2f\n", force.Length(), momentum.Length() );
}

void VehiclePhysicsParams::Init( CBaseVehicle* parent )
{
	finalVelocity = g_vecZero;
	finalAngles = g_vecZero;
	momentum = g_vecZero;

	// frontAxle and rearAxle will be set by the parent vehicle!!!
	this->parent = parent;

	lv.forward = g_vecZero;
	lv.right = g_vecZero;
	lv.up = g_vecZero;
}

void VehiclePhysicsParams::Update()
{
	Vector calculatedPosition = g_vecZero;

	// Get momentum from car
	momentum = parent->pev->velocity / 1.05f;

	// We average the calculated position between two axles according to offset lengths
	// We treat offsets as absolute values, otherwise there will be trouble
	float totalLength = fabs(frontAxle.offset) + fabs(rearAxle.offset);
	float frontLength = fabs(frontAxle.offset) / totalLength;
	float backLength = fabs(rearAxle.offset) / totalLength;

	//// Commented out! This code is buggy
	// While averaging the positions, we'll also take traction into account
	// For example, if the front axle has more traction, then it will have a 
	// stronger multiplier than the rear axle
	//float totalTraction = frontAxle.traction + rearAxle.traction;
	//float frontTraction = frontAxle.traction / totalTraction;
	//float backTraction = rearAxle.traction / totalTraction;
	//
	//float frontMultiplier = (frontLength + backTraction) / 2.0f;
	//float backMultiplier = (backLength + frontTraction) / 2.0f;
	
	// Front axle * back length and rear axle * front length
	// NOT front axle * front length etc.
	// If frontLength was 0.7 (which clearly means the front axle is FARTHER)
	// then front axle * front length would imply that the front axle is closer
	// and that would be a lie
	calculatedPosition = (frontAxle.position * backLength) + (rearAxle.position * frontLength);
	
	// Now that we have the calculated position, let's calculate the velocity needed to reach it
	finalVelocity = (calculatedPosition - parent->pev->origin); // might wanna divide by 0.016 later or something

	// Calculate final angles from resulting axle positions
	// Step 1: treat rearAxle.position as origin, and frontAxle.position relative to that
	Vector axleTransformed = frontAxle.position - rearAxle.position;
	Vector calculatedAngles = UTIL_VecToAngles( finalVelocity );

	// Step 2: Use atan2() on the YX coordinates of the resulting vector
	finalAngles.y = atan2( axleTransformed.y, axleTransformed.x ) * (180.f/M_PI);

	if ( finalVelocity.Length2D() > 0.1f )
		finalAngles.x = finalAngles.x*0.2 + calculatedAngles.x*0.8;
	
	finalAngles.x = 0;

	//ALERT( at_console, "x %3.2f y %3.2f z %3.2f\n", finalVelocity.x, finalVelocity.y, finalVelocity.z );

	finalAngles.z = 0;
}









