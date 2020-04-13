/*

======= adm_vehicle.h ===========================

	Vehicle API header

*/

#pragma once

//#include "adm/shared/adm_control.h"
//#include <../Eigen/src/Geometry/Quaternion.h> // We'll use this custom library later on, when we try implementing vehicle physics ;)

class CBaseVehicle;

enum InVehicleType
{
	InWalking,
	InChair,
	InCar,
	InBike,
	InPlane,
	InHelicopter,
	InShip,
	InBoat,
	InNPC, // Ahem, ON the NPC, unless it vored you, YOU FURRY PRICK
	InSpecialTest,
};

enum VehicleType
{
	VehicleChair = 0,
	VehicleCar, // automobiles, trucks
	VehicleBike, // bicycles, motorbikes
	VehiclePlane, // airplanes, jets
	VehicleHelicopter, // helicopters, drones
	VehicleShip, // spaceships, UFOs
	VehicleBoat, // boats, bath tubs, sleds
	VehicleNPC, // special, rideable NPCs. Think of horses, or no... GARGANTUAS >:D
	VehicleSpecialTest // chair that is like a car, needs no wheels
};

enum VehicleSeatType
{
	Driver = 0,
	Gunner,
	Passenger,
	DriverGunner
};

enum RubberType
{
	Stock = 0,
	Comfort,
	Performance,
	Rally
};

enum VehicleDrive
{
	Drive_FWD = 0,
	Drive_RWD,
	Drive_AWD,
	Drive_NoWheels
};

struct VehicleTorqueCurve // What torque in Nm do we have, at which RPM
{
	float Torque;
	float Rpm;
};

constexpr uint8_t fEngine_Running		= 1 << 0;
constexpr uint8_t fEngine_ClutchHeld	= 1 << 1;
constexpr uint8_t fEngine_Destroyed		= 1 << 2;
constexpr uint8_t fEngine_NoFuel		= 1 << 3;
constexpr uint8_t fEngine_GasHeld		= 1 << 4;
constexpr uint8_t fEngine_BrakeHeld		= 1 << 5;
constexpr uint8_t fEngine_HBHeld		= 1 << 6;
//constexpr uint8_t fEngine_GasHeld		= 1 << 7;

constexpr int Driving_Reverse = -1;
constexpr int Driving_Off = 0;
constexpr int Driving_Lowrpm = 1;
constexpr int Driving_Hirpm = 2;


struct VehicleEngine
{
	int		HorsePower;

	float	Pedal; // to the metal!!! Goes from 0 to +1
	float	BrakePedal;
	float	HandbrakeLever;
	
	float	Torque;
	float	WheelTorque;
	float	Rpm, minRpm, maxRpm;
	float	Output;

	int		Driving; // -1 = reverse; 0 = off; 1 = lowrpm; 2 = hirpm;
	uint8_t Flags;
	
	float	MaxHealth;
	float	Health;
	bool	MadeInGermany;
	
	float	GearRatios[9]; // Maximum of 7 gears + 1st neutral 0th reverse
	int		CurrentGear;
	int		Gears;
	VehicleTorqueCurve TorqueCurve[5]; // 5 precalculated points, to be interpolated

	float	Efficiency;
	float	SlowDown;

	VehicleDrive Drive;

	void Damage(float hp); // Obv. you can now apply negative damage to this, with a repair tool; gotta code such a thing in the future
	int  InstaRepair();

	// SetGearRatios HAS to be called before Init
	void  SetGearRatios(float reverse, float first, float second, float third, float fourth, float fifth, float sixth, float seventh);
	void  SetTorqueCurve(VehicleTorqueCurve tc1, VehicleTorqueCurve tc2, VehicleTorqueCurve tc3, VehicleTorqueCurve tc4, VehicleTorqueCurve tc5);
	int   GetGears();
	float CalcTorque();

	void Accelerate(CBaseVehicle &Vehicle);
	void Brake(CBaseVehicle &Vehicle);
	void Handbrake(CBaseVehicle &Vehicle);

	void Init(VehicleDrive Drive_XWD, float maxHealth, int horsepower, float Efficiency, bool invincible = false, float minrpm = 600, float maxrpm = 4000);
	void Update();
};

struct VehicleSeat
{ 
	VehicleSeatType type;
	vec3_t pos, angles;
	CBasePlayer *pSessilis; // Never thought you'd see Latin in the HL SDK, did ya?
	bool fExists = false;
	int iSitdex; // Cool name for a seat index ;)
	bool fCommands[16]
	{
		false, false, false, false, // acc, dec, lt, rt / shoot, shoot2, lt, rt
		false, false, false, false, // up, dn, llt, lrt
		false, false, false, false, // hdbk, cltc, jump, attack
		false, false, false, false  // flashlight, unuse, startstop, seatswitch
	};

	int SeatPlayer();
	void SeatSwitch(VehicleSeat &nextSeat);
	void FlushSeatCommands();
	void ListenToCommands();
	void Exit();
	void Init(VehicleSeatType intype, vec3_t inpos, int sitdex);
	void AttachToPos(vec3_t targetpos, float radius, float pitch, float yaw);
	void AttachToPos(vec3_t targetpos, vec3_t offset, float radius, float pitch, float yaw);
	void AttachToPos(CBaseVehicle &Vehicle, int iBoneOffset);
};

constexpr int Wheel_Steerable = 1 << 1;
constexpr int Wheel_Front = 1 << 2;
constexpr int Wheel_Back = 1 << 3;
constexpr int Wheel_Brake = 1 << 4;
constexpr int Wheel_Handbrake = 1 << 4;

struct VehicleWheel
{
	vec3_t pos, angles, steerangles, groundangles;
	Vector Force = { 0, 0, 0 }, TractionForce = { 0, 0, 0 };

	bool  onGround;
	float Traction;
	float originalTraction;
	
	float Radius;
	float Wear; // One could also say damage
	float Width;
	
	float RollAngle;
	float SteerAngle;
	
	int   Flags = 0;
	RubberType type;
	bool fExists = false;
	
	CBaseEntity* m_pWheel; // actual, physical representations of this wheel, visible by the game
	string_t m_iszModel;

	void SteerLeft(float flSpeed, CBaseVehicle &Vehicle);
	void SteerRight(float flSpeed, CBaseVehicle &Vehicle);

	void Init(RubberType rubbertype, CBaseVehicle &Vehicle, string_t iszWheel, int flags, float radius = 12.0, float width = 8.0);
	void Update(float flSpeed, CBaseVehicle &Vehicle, int arrindex);
	void AttachToPos(CBaseVehicle &Vehicle, int arrayindex);
};

struct VehicleBody
{
	vec3_t pos, angles;
	float Mass;
	float Density;
	string_t m_iszModel;
};

struct VehicleTrace
{
	
};

// The mother of all vehicles. Contains a body and a seat.
// Contains all the basic functions. The idea is to 
// derive other vehicles from this that have different combos
// of seats, engines, wheels and bodies, and all that is required is
// to set up the components and let them call their internal functions.
// Body uses a studio model!!! Brush vehicles are a separate class.
// Number of seats: 1
// Number of wheels: 0
// Number of bodies: 1
// Number of engines: 0
class CBaseVehicle : public CBaseEntity
{
public:

	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData *pkvd);

	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int ObjectCaps() override { return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;  }
	
	// Seat-related business
	void SeatPlayerLocking();
	void ListenToCommands();
	void SeatPositionLocking();
	void SeatSwitch(VehicleSeat &seatFrom);

	// Vehicle general
	void EXPORT VehicleUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual void EXPORT VehicleThink();
	void		VehicleExit(VehicleSeat *pSeat);

	virtual void VehicleBaseInit();
	virtual void VehicleInit();
	virtual void VehicleMovement();

	VehicleType GetVehType() { return v_Type; }
	int GetSeats() { return m_iSeats; }
	int GetWheels() { return m_iWheels;  }

//private:
	VehicleType		v_Type;
	VehicleBody		v_Body;
	VehicleSeat		v_Seats[16]; // Vehicles can have a maximum of 16 seats
	VehicleWheel	v_Wheels[32]; // Vehicles can have a maximum of 32 wheels
	VehicleEngine	v_Engine;
	int				m_iBoneOffset = 0;
	int				m_iSeats = 1;
	int				m_iWheels = 0;

	vec3_t			oldangles;
	vec3_t			m_vecFinalForce;
};

// Some vehicle utilities
Vector AlignToGround(Vector origin, Vector dirangles, float radius, edict_t *pentIgnore);
void GetBoneId(int &iBone, VehicleType vType);

// BI - bool index for vehicle commands
constexpr int bi_acc = 0;
constexpr int bi_dec = 1;
constexpr int bi_lt = 2;
constexpr int bi_rt = 3;

constexpr int bi_shoot = 0;
constexpr int bi_shoot2 = 1;

constexpr int bi_up = 4;
constexpr int bi_dn = 5;
constexpr int bi_llt = 6;
constexpr int bi_lrt = 7;

constexpr int bi_hdbk = 8;
constexpr int bi_cltc = 9;
constexpr int bi_jump = 10;
constexpr int bi_attack = 11;

constexpr int bi_flashlight = 12;
constexpr int bi_unuse = 13;
constexpr int bi_startstop = 14;
constexpr int bi_seatswitch = 15;

// Bones

// Densities - kg / m^3
constexpr int Density_BirchWood = 670;
constexpr int Density_HiChromIron = 7400;
constexpr int Density_Iron = 7874;
constexpr int Density_Steel = 7900;
constexpr int Density_PVC = 1380;
constexpr int Density_Plastic = 920;
constexpr int Density_Tyre1 = 570;
constexpr int Density_Tyre2 = 600;
constexpr int Density_Tyre3 = 730;
constexpr int Density_Pillow = 11;

/*
Development of a car physics engine for games
Thesis: https://nccastaff.bournemouth.ac.uk/jmacey/MastersProjects/MSc12/Srisuchat/Thesis.pdf

The angular velocity of the engine in rad/s is obtained by multiplying
the engine turnover rate by 2π and dividing by 60.
ωe = 2π Ωe / 60

The wheel toque can
be obtained using the following equation.
Tw = Te * gk * G (4-2)
Where Te is the engine torque, gk is the gear ratio of whatever gear the car is
in and G is the final drive ratio.
The relationship between the engine turnover rate and the wheel angular
velocity is as follows.
ωw = 2π Ωe / (60*gk*G) (4-3)
If we assume that the tires roll on the ground without slipping, the
translational velocity of the car can be related to the engine turnover rate as
follows.
V = rwωw = rw * 2π Ωe / (60*gk*G)

1. Transform velocity in world reference frame to velocity in car
reference frame (Vx = Vlong, Vz = Vlong). Convention for car reference
frame: x – pointing to the front, z – pointing to the right

2. Compute the slip angles for front and rear wheels (equation 5.2)
αfront = arctan((Vlat + ω * b) / Vlong)) – σ * sgn(Vlong)
αfront = arctan((Vlat - ω * c) / Vlong))

3. Compute Flat = Ca * slip angle (do for both rear and front wheels)

4. Cap Flat to maximum normalized frictional force (do for both rear and
front wheels)

5. Multiply Flat by the load (do for both rear and front wheels) to obtain
the cornering forces.

6. Compute the engine turn over rate Ωe = Vx 60*gk*G / (2π * rw)

7. Clamp the engine turn over rate from 6 to the defined redline

8. If use automatic transmission call automaticTransmission() function
to shift the gear

9. Compute the constant that define the torque curve line from the
engine turn over rate

10. From 9, compute the maximum engine torque, Te

11. Compute the maximum torque applied to the wheel Tw = Te * gk * G

12. Multiply the maximum torque with the fraction of the throttle
position to get the actual torque applied to the wheel (Ftraction - The
traction force)

13. If the player is braking replace the traction force from 12 to a defined
braking force

14. If the car is in reverse gear replace the traction force from 12 to a
defined reverse force

15. Compute rolling resistance Frr, x = - Crr * Vx and Frr,z = - Crr * Vz

16. Compute drag resistance Fdrag, x = - Cdrag * Vx * |Vx| and Fdrag, z = -
Cdrag * Vz * |Vz|

17. Compute total resistance (Fresistance) = rolling resistance + drag
resistance

18. Sum the force on the car body
Fx = Ftraction + Flat, front * sin (σ) * Fresistance, x
Fz = Flat, rear + Flat, front * cos (σ) * Fresistance, z

19. Compute the torque on the car body
Torque = cos (σ) * Flat, front * b – Flat, rear * c

20. Compute the acceleration
a = F / M

21. Compute the angular acceleration
α = Torque/Inertia

22. Transform the acceleration from car reference frame to world
reference frame

23. Integrate the acceleration to get the velocity (in world reference
frame)
Vwc += dt * a

24. Integrate the velocity to get the new position in world coordinate
Pwc += dt * Vwc
25. Move the camera to follow the car.

26. Integrate the angular acceleration to get the angular velocity
ω += dt * α

27. Integrate the angular velocity to get the angular orientation
Yaw angle += dt * ω

28. Obtain the rotation rate of the wheels by dividing the car speed with
the wheel radius
Wheel rotation rate = car speed / wheel radius

29. Re-render the car
*/