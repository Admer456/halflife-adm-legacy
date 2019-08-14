/*#include <vector>

/*

======= adm_control.h ===========================

	Control API header
	
*/

/*
class AdmControlAPI
{
public:
	void ApplyControl(char *szControl);
	void OnButtonUp(char *szControl); // Called directly from input.cpp via KeyUp()
	void OnButtonDown(char *szControl); // Called from KeyDown()

private:
	int iIndex;

	std::vector<char*>szKeyList =
	{
		"adm_k1",	"adm_k2",	"adm_k3", // quickly going to reserve some keys here and there

/*		"car_acc",	"car_dec",	"car_lt",	"car_rt",	"car_hdbk",

		"bike_acc",	"bike_dec", "bike_lt",	"bike_rt",	"bike_hdbk",

		"boat_acc", "boat_dec", "boat_lt",	"boat_rt", // boats don't have handbrakes yo

		"plane_acc","plane_dec","plane_lt",	"plane_rt", "plane_up",	"plane_dn", "plane_llt","plane_lrt",// but that's why planes can look up and down ;)

		"ship_acc",	"ship_dec",	"ship_lt",	"ship_rt",	"ship_up",	"ship_dn",	"ship_llt",	"ship_lrt",

		"heli_acc",	"heli_dec",	"heli_lt",	"heli_rt",	"heli_up",	"heil_dn",	"heli_llt",	"heli_lrt",

		"vehicle_flashlight",	"vehicle_unuse",	"vehicle_startstop",	"vehicle_seatswitch"
		"gunner_shoot",			"gunner_shoot2", */
		// More will be added here eventually.
/*	};

	bool bitKeyList[] =
	{
		false, false, false, // adm_k
	/*	false, false, false, false, false, // car
		false, false, false, false, false, // bike
		false, false, false, false, // boat
		false, false, false, false, false, false, false, false, // plane
		false, false, false, false, false, false, false, false, // ship
		false, false, false, false, false, false, false, false, // heli

		false, false, false, false, // vehicle
		false, false, // gunner */ /*
	};
};

//AdmControlAPI ControlAPI; */ 
// This got scrapped as soon as I saw the EngineClientCmd command.
// It would've been too much of an overhead to do the whole API
// the way I imagined it, so it's better this way.

#pragma once

typedef enum CtrlApi_key
{
	adm_k1 = 0,
	adm_k2, 
	adm_k3,
	sprint, // 3

	vehicle_acc,  // accelerate // 4
	vehicle_dec,  // decelerate
	vehicle_lt,	  // steer left
	vehicle_rt,	  // steer right
	vehicle_hdbk, // handbrake
	vehicle_cltc, // clutch     // 9

	vehicle_up, // UPARROW		// 10
	vehicle_dn, // DOWNARROW
	vehicle_llt, // LEFTARROW
	vehicle_lrt, // RIGHTARROW	// 13

	vehicle_jump,				// 14 - 15
	vehicle_attack,

	vehicle_flashlight,			// 16
	vehicle_unuse, // Q
	vehicle_startstop, // I
	vehicle_seatswitch,	// O	// 19

	gunner_shoot,				// 20 - 21
	gunner_shoot2			

	// to be added, gotta get to 64 somehow. :P
} CtrlApi_key_t;
