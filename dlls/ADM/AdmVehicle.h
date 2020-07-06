/*

======= adm_vehicle.h ===========================

	Vehicle API header

*/

#pragma once

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
	float torque;
	float rpm;
};

enum EngineOperationFlags
{
	EngineRunning = 0,
	EngineClutchHeld,
	EngineDestroyed,
	EngineNoFuel,
	EngineGasHeld,
	EngineBrakeHeld,
	EngineHandbrakeHeld
};

enum EngineDrivingstates
{
	DrivingReverse = -1,
	DrivingOff,
	DrivingLowRpm,
	DrivingHighRpm
};

struct VehicleEngine
{
	int					HorsePower;

	float				Pedal; // to the metal!!! Goes from 0 to +1
	float				BrakePedal;
	float				HandbrakeLever;
	
	float				torque;
	float				WheelTorque;
	float				rpm, minRpm, maxRpm;
	float				Output;

	int					Driving; // -1 = reverse; 0 = off; 1 = lowrpm; 2 = hirpm;
	uint8_t				flags;
	
	float				MaxHealth;
	float				Health;
	bool				MadeInGermany;
	
	float				GearRatios[9]; // Maximum of 7 gears + 1st neutral 0th reverse
	int					CurrentGear;
	int					Gears;
	VehicleTorqueCurve	TorqueCurve[5]; // 5 precalculated points, to be interpolated

	float				Efficiency;
	float				SlowDown;
	VehicleDrive		Drive;

	CBaseVehicle*		parent;

	void				Damage(float hp); // Obv. you can now apply negative damage to this, with a repair tool; gotta code such a thing in the future
	int					InstaRepair();

	// SetGearRatios HAS to be called before Init
	void				SetGearRatios( float reverse, float first, float second, float third, float fourth, float fifth, float sixth, float seventh );
	void				SetTorqueCurve( VehicleTorqueCurve tc1, VehicleTorqueCurve tc2, VehicleTorqueCurve tc3, VehicleTorqueCurve tc4, VehicleTorqueCurve tc5 );
	int					GetGears();
	float				CalcTorque();

	void				Accelerate();
	void				Brake();
	void				Handbrake();

	void				Init( CBaseVehicle* pParent, VehicleDrive Drive_XWD = Drive_NoWheels, float maxHealth = 1000, int horsepower = 4000, float Efficiency = 0.5f, bool invincible = false, float minrpm = 600, float maxrpm = 4000 );
	void				Update();
};

struct VehicleSeat
{ 
	VehicleSeatType		type;
	Vector				origin;
	Vector				angles;
	CBasePlayer*		pSessilis; // Never thought you'd see Latin in the HL SDK, did ya?
	bool				fExists = false;
	int					iSitdex; // Cool name for a seat index ;)
	int					commands;

	CBaseVehicle*		parent;

	int					SeatPlayer();
	void				SeatSwitch(VehicleSeat &nextSeat);

	void				FlushSeatCommands();
	void				ListenToCommands();
	inline bool			CheckCommandBit( int& offset );
	void				SetCommandBit( const int& offset, const bool& state );

	void				Exit();
	void				Init( CBaseVehicle* pParent, VehicleSeatType intype, Vector inpos, int sitdex );

	void				AttachToPos( Vector targetpos, float radius, float pitch, float yaw );
	void				AttachToPos( Vector targetpos, Vector offset, float radius, float pitch, float yaw );
	void				AttachToPos( int iBoneOffset );
};

enum WheelTypeFlags
{
	Wheel_Steerable = 0,
	Wheel_Front,
	Wheel_Back,
	Wheel_Brake,
	Wheel_Handbrake
};

struct VehicleWheel
{
	Vector				origin;
	Vector				oldOrigin;
	Vector				angles; 
	Vector				steerangles;
	Vector				groundangles;
//	Vector				force;
	Vector				tractionForce;

	bool				onGround;
	float				traction;
	float				originalTraction;
	float				tractionFactorBlend;
	
	float				radius;
	float				wear; // One could also say damage
	float				width;
	
	float				rollAngularVelocity;
	float				rollAngle;
	float				steerAngle;
	
	int					flags = 0;
	RubberType			type;
	bool				fExists = false;
	
	CBaseEntity*		m_pWheel; // actual, physical representations of this wheel, visible by the game
	string_t			m_iszModel;

	CBaseVehicle*		parent;

	void				SteerLeft( float flSpeed );
	void				SteerRight( float flSpeed );

	void				Init( RubberType rubbertype, CBaseVehicle* pParent, string_t iszWheel, int flags, float radius = 12.0, float width = 8.0 );
	void				Update( float flSpeed, int arrindex );
	void				AttachToPos( int arrayindex );
};

struct VehicleBody
{
	vec3_t				origin, angles;
	float				mass;
	float				density;
	string_t			m_iszModel;
};

struct VehicleTrace
{
	// TO-DO: Fill this
	TraceResult trace;
};

// The mother of all vehicles. Contains a body and a seat.
// All vehicles must inherit from this class.
class CBaseVehicle : public CBaseEntity
{
public:
	void				Spawn( void );
	void				Precache( void );
	void				KeyValue( KeyValueData* pkvd );

	void				Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );
	int					ObjectCaps() override { return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE; }
	
	// Seat-related business
	void				SeatPlayerLocking();	// Attaches players to their seats
	void				ListenToCommands();		// Polls for any player input
	void				SeatPositionLocking();	// Attaches all seats to the vehicle body and synchronises them
	void				SeatSwitch( VehicleSeat& seatFrom ); // Picks a free seat and switches the player from seatFrom to the next free seat

	// Vehicle general
	void EXPORT			VehicleUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );
	virtual void EXPORT VehicleThink();

	virtual void		VehicleBaseInit();		// New vehicle bases override this
	virtual void		VehicleInit();			// Each vehicle class defines its own init parameters
	virtual void		VehicleMovement();		// Each vehicle base and vehicle class can override this

	VehicleEngine		GetEngine()		{ return v_Engine;	}
	VehicleType			GetVehType()	{ return v_Type;	}
	int					GetSeats()		{ return m_iSeats;	}
	int					GetWheels()		{ return m_iWheels;	}

	// Vehicle utilities
public:
	static int			GetBoneId( int& bone, VehicleType type );

protected:
	VehicleType			v_Type;
	VehicleBody			v_Body;
	VehicleSeat			v_Seats[16];			// Vehicles can have a maximum of 16 seats
	VehicleWheel		v_Wheels[32];			// Vehicles can have a maximum of 32 wheels
	VehicleEngine		v_Engine;
	int					m_iBoneOffset = 0;		// [to be deprecated] Offset for bone IDs, for the purpose of seat attaching, e.g. seat 3 will attach to boneOffset + 3
	int					m_iSeats = 1;
	int					m_iWheels = 0;

	Vector				oldangles;
	Vector				m_vecFinalForce;		// TO-DO: create VehiclePhysicsParams struct
};

// Some vehicle utilities
Vector AlignToGround( Vector origin, Vector dirangles, float radius, edict_t* pentIgnore );
void GetBoneId( int& iBone, VehicleType vType );

// BI - bool index for vehicle commands
enum VehicleCommands
{	// Basic controls
	VehAcceleration = 0,
	VehDeceleration,
	VehLeft,
	VehRight,

	// Gunner controls
	VehShoot,
	VehShootAlternate,
	
	// Airplane controls
	VehUp,
	VehDown,
	VehLeanLeft,
	VehLeanRight,

	// Car-specific controls
	VehHandbrake,
	VehClutch,

	// Special vehicle commands
	VehJump,		// used for NPC vehicles, e.g. a horse jumps over a fence
	VehAttack,		// used for NPC vehicles
	VehFlashlight,
	VehDismount,
	VehToggleEngine,
	VehSwitchSeats
};

struct LocalVectors
{
	Vector forward, right, up;
};

struct VehiclePhysicsParams;

// Represents some physical properties
// of an end of a car, e.g. front part of the car
// For now, it supports axles with 2 wheels only
struct AxlePhysicsParams
{
	Vector direction;	// Direction of the axle, calculated by positions of the 2 wheels
	Vector position;	// Position of the axle relative to the world
	Vector force;		// Calculated force from the 2 wheels
	float traction;		// Traction from wheels; 1.0 = grippy, 0.0 = slippery

	float offset;		// Local offset from the car's centre (only along the front axis for now)
	Vector momentum;	// Almost like an external force caused by the car going on its own, e.g. downhill 

	VehicleWheel* leftWheel;
	VehicleWheel* rightWheel;
	VehiclePhysicsParams* baseParams;

	void Init( VehicleWheel* left, VehicleWheel* right, VehiclePhysicsParams* params, float offset );
	void Update();
};

// Represents total physical properties of a car
// Meant to be handled in VehicleMovement()
struct VehiclePhysicsParams
{
	// Final vectors
	Vector finalVelocity;	// Where the car will ultimately go to
	Vector finalAngles;		// The final direction of the car
	Vector momentum;		// Momentum (last frame's velocity) from the car 

	AxlePhysicsParams frontAxle;
	AxlePhysicsParams rearAxle;

	CBaseVehicle* parent;

	LocalVectors lv;

	void Init( CBaseVehicle* parent );
	void Update();
};


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
