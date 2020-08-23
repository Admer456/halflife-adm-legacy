// view/refresh setup functions

#include "HUD/hud.h"
#include "cl_util.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"

#include "entity_state.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "Input/IN_Defs.h" // PITCH YAW ROLL
#include "pm_movevars.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "event_api.h"
#include "pmtrace.h"
#include "bench.h"
#include "screenfade.h"
#include "shake.h"
#include "hltv.h"
#include "Exports.h"

#include "SDL2/SDL.h"
#include "ADM/System/SDLWrapper.h"

// Since ADM should not affect HL gameplay,
// I've decided to disable this feature as it makes a significant change in aiming,
// as well as bringing back the old view bobbing.
#define ADM_CustomClientPunch 0

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

	int		CL_IsThirdPerson( void );
	void	CL_CameraOffset( float *ofs );

	void DLLEXPORT V_CalcRefdef( struct ref_params_s *pparams );

	void	PM_ParticleLine( float *start, float *end, int pcolor, float life, float vert);
	int		PM_GetVisEntInfo( int ent );
	int		PM_GetPhysEntInfo( int ent );
	void	InterpolateAngles(  float * start, float * end, float * output, float frac );
	void	NormalizeAngles( float* angles );
	float	Distance(const float * v1, const float * v2);
	float	AngleBetweenVectors(  const float * v1,  const float * v2 );

	extern float	vJumpOrigin[3];
	extern float	vJumpAngles[3];


void V_DropPunchAngle ( float frametime, float *ev_punchangle );
void VectorAngles( const float *forward, float *angles );

#include "r_studioint.h"
#include "com_model.h"
#include "Input/KButton.h"

extern engine_studio_api_t IEngineStudio;

extern kbutton_t	in_mlook;

/*
The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.
*/

extern cvar_t	*cl_forwardspeed;
extern cvar_t	*chase_active;
extern cvar_t	*scr_ofsx, *scr_ofsy, *scr_ofsz;
extern cvar_t	*cl_vsmoothing;

#define	CAM_MODE_RELAX		1
#define CAM_MODE_FOCUS		2

vec3_t		v_origin, v_angles, v_cl_angles, v_sim_org, v_lastAngles, v_velocity;
float		v_frametime, v_lastDistance;	
float		v_cameraRelaxAngle	= 5.0f;
float		v_cameraFocusAngle	= 35.0f;
int			v_cameraMode = CAM_MODE_FOCUS;
qboolean	v_resetCamera = 1;

vec3_t ev_punchangle;

cvar_t	*scr_ofsx;
cvar_t	*scr_ofsy;
cvar_t	*scr_ofsz;

cvar_t	*v_centermove;
cvar_t	*v_centerspeed;

cvar_t	*cl_bobcycle;
cvar_t	*cl_bob;
cvar_t	*cl_bobup;
cvar_t	*cl_waterdist;
cvar_t	*cl_chasedist;

// These cvars are not registered (so users can't cheat), so set the ->value field directly
// Register these cvars in V_Init() if needed for easy tweaking
cvar_t	v_iyaw_cycle		= {"v_iyaw_cycle", "2", 0, 2};
cvar_t	v_iroll_cycle		= {"v_iroll_cycle", "0.5", 0, 0.5};
cvar_t	v_ipitch_cycle		= {"v_ipitch_cycle", "1", 0, 1};
cvar_t	v_iyaw_level		= {"v_iyaw_level", "0.3", 0, 0.3};
cvar_t	v_iroll_level		= {"v_iroll_level", "0.1", 0, 0.1};
cvar_t	v_ipitch_level		= {"v_ipitch_level", "0.3", 0, 0.3};

float	v_idlescale;  // used by TFC for concussion grenade effect

//=============================================================================
/*
void V_NormalizeAngles( vec3_t angles )
{
	int i;
	// Normalize angles
	for ( i = 0; i < 3; i++ )
	{
		if ( angles[i] > 180.0 )
		{
			angles[i] -= 360.0;
		}
		else if ( angles[i] < -180.0 )
		{
			angles[i] += 360.0;
		}
	}
}

/*
===================
V_InterpolateAngles

Interpolate Euler angles.
FIXME:  Use Quaternions to avoid discontinuities
Frac is 0.0 to 1.0 ( i.e., should probably be clamped, but doesn't have to be )
===================

void V_InterpolateAngles( float *start, float *end, float *output, float frac )
{
	int i;
	float ang1, ang2;
	float d;
	
	V_NormalizeAngles( start );
	V_NormalizeAngles( end );

	for ( i = 0 ; i < 3 ; i++ )
	{
		ang1 = start[i];
		ang2 = end[i];

		d = ang2 - ang1;
		if ( d > 180 )
		{
			d -= 360;
		}
		else if ( d < -180 )
		{	
			d += 360;
		}

		output[i] = ang1 + d * frac;
	}

	V_NormalizeAngles( output );
} */

enum calcBobMode_t
{
	VB_COS,
	VB_SIN,
	VB_COS2,
	VB_SIN2
};

// Quakeworld bob code, this fixes jitters in the mutliplayer since the clock (pparams->time) isn't quite linear
void V_CalcBob( struct ref_params_s *pparams, float freqmod, calcBobMode_t mode, double &bobtime, float &bob, float &lasttime )
{
	float	cycle;
	vec3_t	vel;

	if ( pparams->onground == -1 ||
		 pparams->time == lasttime )
	{
		// just use old value
		return;// bob;	
	}

	lasttime = pparams->time;

	bobtime += pparams->frametime * freqmod;
	cycle = bobtime - (int)(bobtime / cl_bobcycle->value) * cl_bobcycle->value;
	cycle /= cl_bobcycle->value;

	if ( cycle < cl_bobup->value )
	{
		cycle = M_PI * cycle / cl_bobup->value;
	}
	else
	{
		cycle = M_PI + M_PI * (cycle - cl_bobup->value) / (1.0 - cl_bobup->value);
	}

	// bob is proportional to simulated velocity in the xy plane
	// (don't count Z, or jumping messes it up)
	VectorCopy( pparams->simvel, vel );
	vel[ 2 ] = 0;

	bob = sqrt( vel[ 0 ] * vel[ 0 ] + vel[ 1 ] * vel[ 1 ] ) * cl_bob->value;

	if ( mode == VB_SIN )
		bob = bob * 0.3 + bob * 0.7 * sin( cycle );
	else if ( mode == VB_COS )
		bob = bob * 0.3 + bob * 0.7 * cos( cycle );
	else if ( mode == VB_SIN2 )
		bob = bob * 0.3 + bob * 0.7 * sin( cycle ) * sin( cycle );
	else if ( mode == VB_COS2 )
		bob = bob * 0.3 + bob * 0.7 * cos( cycle ) * cos( cycle );

	bob = V_min( bob, 4 );
	bob = V_max( bob, -7 );
	//return bob;
}

/*
===============
V_CalcRoll
Used by view and sv_user
===============
*/
float V_CalcRoll (vec3_t angles, vec3_t velocity, float rollangle, float rollspeed, int direction = 1 ) // directions: 0 = forward, 1 = right, 2 = up, 3 = exclusive up
{
    float   sign;
    float   side;
    float   value;
	vec3_t  forward, right, up;
    
	AngleVectors ( angles, forward, right, up );
    
	switch ( direction )
	{
		default:
		case 0: side = DotProduct( velocity, forward ); break;
		case 1: side = DotProduct( velocity, right ); break;
		case 2: side = DotProduct( velocity, up ); break;
		case 3: side = velocity[2] * up[2]; break;
	}
    sign = side < 0 ? -1 : 1;
    side = fabs( side );
    
	value = rollangle;
    if (side < rollspeed)
	{
		side = side * value / rollspeed;
	}
    else
	{
		side = value;
	}
	return side * sign;
}

typedef struct pitchdrift_s
{
	float		pitchvel;
	int			nodrift;
	float		driftmove;
	double		laststop;
} pitchdrift_t;

static pitchdrift_t pd;

void V_StartPitchDrift( void )
{
	if ( pd.laststop == gEngfuncs.GetClientTime() )
	{
		return;		// something else is keeping it from drifting
	}

	if ( pd.nodrift || !pd.pitchvel )
	{
		pd.pitchvel = v_centerspeed->value;
		pd.nodrift = 0;
		pd.driftmove = 0;
	}
}

void V_StopPitchDrift ( void )
{
	pd.laststop = gEngfuncs.GetClientTime();
	pd.nodrift = 1;
	pd.pitchvel = 0;
}

/*
===============
V_DriftPitch

Moves the client pitch angle towards idealpitch sent by the server.

If the user is adjusting pitch manually, either with lookup/lookdown,
mlook and mouse, or klook and keyboard, pitch drifting is constantly stopped.
===============
*/
void V_DriftPitch ( struct ref_params_s *pparams )
{
	float		delta, move;

	if ( gEngfuncs.IsNoClipping() || !pparams->onground || pparams->demoplayback || pparams->spectator )
	{
		pd.driftmove = 0;
		pd.pitchvel = 0;
		return;
	}

	// don't count small mouse motion
	if ( pd.nodrift)
	{
		if ( v_centermove->value > 0 && !(in_mlook.state & 1) )
		{		
			// this is for lazy players. if they stopped, looked around and then continued
			// to move the view will be centered automatically if they move more than
			// v_centermove units. 

			if ( fabs( pparams->cmd->forwardmove ) < cl_forwardspeed->value )
				pd.driftmove = 0;
			else
				pd.driftmove += pparams->frametime;
		
			if ( pd.driftmove > v_centermove->value)
			{
				V_StartPitchDrift ();
			}
			else
			{
				return;	// player didn't move enough
			}
		}

		return;	// don't drift view
	}
	
	delta = pparams->idealpitch - pparams->cl_viewangles[PITCH];

	if (!delta)
	{
		pd.pitchvel = 0;
		return;
	}

	move = pparams->frametime * pd.pitchvel;
	
	pd.pitchvel *= (1.0f+(pparams->frametime*0.25f)); // get faster by time
	
	if (delta > 0)
	{
		if (move > delta)
		{
			pd.pitchvel = 0;
			move = delta;
		}
		pparams->cl_viewangles[PITCH] += move;
	}
	else if (delta < 0)
	{
		if (move > -delta)
		{
			pd.pitchvel = 0;
			move = -delta;
		}
		pparams->cl_viewangles[PITCH] -= move;
	}
}

/* 
============================================================================== 
						VIEW RENDERING 
============================================================================== 
*/ 

/*
==================
V_CalcGunAngle
==================
*/
void V_CalcGunAngle ( struct ref_params_s *pparams )
{	
	cl_entity_t *viewent;
	static int iCount = 0;
	static Vector vecOldAngles;
	static Vector vecBlended;

	viewent = gEngfuncs.GetViewModel();
	if ( !viewent )
		return;

	if (!iCount && vecOldAngles.x == 0)
	{
		VectorCopy(viewent->angles, vecOldAngles);
		VectorCopy(viewent->angles, vecBlended);
		iCount = 1;
	}

	viewent->angles[YAW]   =  pparams->viewangles[YAW]   + pparams->crosshairangle[YAW];
	viewent->angles[PITCH] = -pparams->viewangles[PITCH] + pparams->crosshairangle[PITCH] * 0.25;
	
	// don't apply all of the v_ipitch to prevent normally unseen parts of viewmodel from coming into view.
	viewent->angles[PITCH] -= v_idlescale * sin(pparams->time*v_ipitch_cycle.value) * (v_ipitch_level.value * 0.5);
	viewent->angles[YAW]   -= v_idlescale * sin(pparams->time*v_iyaw_cycle.value) * v_iyaw_level.value;
	viewent->angles[ROLL] -= v_idlescale * sin(pparams->time*v_iroll_cycle.value) * v_iroll_level.value * 3;

	vecBlended = (vecOldAngles + viewent->angles) / 2;

	VectorCopy( vecBlended, viewent->curstate.angles );
	VectorCopy( vecBlended, viewent->latched.prevangles );

	iCount++;

	if (iCount == 100)
	{
		VectorCopy(viewent->angles, vecOldAngles);
		iCount = 0;
	}
}

/*
==============
V_AddIdle

Idle swaying
==============
*/
void V_AddIdle ( struct ref_params_s *pparams )
{
	pparams->viewangles[ROLL] += v_idlescale * sin(pparams->time*v_iroll_cycle.value) * v_iroll_level.value;
	pparams->viewangles[PITCH] += v_idlescale * sin(pparams->time*v_ipitch_cycle.value) * v_ipitch_level.value;
	pparams->viewangles[YAW] += v_idlescale * sin(pparams->time*v_iyaw_cycle.value) * v_iyaw_level.value;
}

/*
==============
V_CalcViewRoll

Roll is induced by movement and damage
==============
*/
void V_CalcViewRoll ( struct ref_params_s *pparams )
{
	float		side;
	float		additive_roll = 0;
	cl_entity_t *viewentity;
	
	viewentity = gEngfuncs.GetEntityByIndex( pparams->viewentity );
	if ( !viewentity )
		return;

	side = V_CalcRoll ( viewentity->angles, pparams->simvel, pparams->movevars->rollangle, pparams->movevars->rollspeed );

	pparams->viewangles[ROLL] += side;

	if ( pparams->health <= 0 && ( pparams->viewheight[2] != 0 ) )
	{
		// only roll the view if the player is dead and the viewheight[2] is nonzero 
		// this is so deadcam in multiplayer will work.
		pparams->viewangles[ROLL] = additive_roll;	// dead view angle

		additive_roll += 0.8;

		if (additive_roll > 80)
			additive_roll = 80;

		return;
	}
	else
		additive_roll = 0;
}

/*
==================
V_CalcIntermissionRefdef

==================
*/
void V_CalcIntermissionRefdef ( struct ref_params_s *pparams )
{
	cl_entity_t	*ent, *view;
	float		old;

	// ent is the player model ( visible when out of body )
	ent = gEngfuncs.GetLocalPlayer();
	
	// view is the weapon model (only visible from inside body )
	view = gEngfuncs.GetViewModel();

	VectorCopy ( pparams->simorg, pparams->vieworg );
	VectorCopy ( pparams->cl_viewangles, pparams->viewangles );

	view->model = NULL;

	// allways idle in intermission
	old = v_idlescale;
	v_idlescale = 1;

	V_AddIdle ( pparams );

	if ( gEngfuncs.IsSpectateOnly() )
	{
		// in HLTV we must go to 'intermission' position by ourself
		VectorCopy( gHUD.m_Spectator.m_cameraOrigin, pparams->vieworg );
		VectorCopy( gHUD.m_Spectator.m_cameraAngles, pparams->viewangles );
	}

	v_idlescale = old;

	v_cl_angles = pparams->cl_viewangles;
	v_origin = pparams->vieworg;
	v_angles = pparams->viewangles;
}

#define ORIGIN_BACKUP 64
#define ORIGIN_MASK ( ORIGIN_BACKUP - 1 )

typedef struct 
{
	float Origins[ ORIGIN_BACKUP ][3];
	float OriginTime[ ORIGIN_BACKUP ];

	float Angles[ ORIGIN_BACKUP ][3];
	float AngleTime[ ORIGIN_BACKUP ];

	int CurrentOrigin;
	int CurrentAngle;
} viewinterp_t;

void V_SmoothInterpolateAngles(float * startAngle, float * endAngle, float * finalAngle, float degreesPerSec)
{
	float absd, frac, d, threshhold;

	NormalizeAngles(startAngle);
	NormalizeAngles(endAngle);

	for (int i = 0; i < 3; i++)
	{
		d = endAngle[i] - startAngle[i];

		if (d > 180.0f)
		{
			d -= 360.0f;
		}
		else if (d < -180.0f)
		{
			d += 360.0f;
		}

		absd = fabs(d);

		if (absd > 0.01f)
		{
			frac = degreesPerSec * v_frametime;

			threshhold = degreesPerSec / 4;

			if (absd < threshhold)
			{
				float h = absd / threshhold;
				h *= h;
				frac *= h;  // slow down last degrees
			}

			if (frac > absd)
			{
				finalAngle[i] = endAngle[i];
			}
			else
			{
				if (d > 0)
					finalAngle[i] = startAngle[i] + frac;
				else
					finalAngle[i] = startAngle[i] - frac;
			}
		}
		else
		{
			finalAngle[i] = endAngle[i];
		}

	}

	NormalizeAngles(finalAngle);
}

void V_CalcViewSmoothing( struct ref_params_s* pparams, float &oldz, float &lasttime, cl_entity_t* ViewModel, viewinterp_t &ViewInterp )
{
	// smooth out stair step ups
#if 1
	if ( !pparams->smoothing && pparams->onground && pparams->simorg[ 2 ] - oldz > 0 )
	{
		float steptime;

		steptime = pparams->time - lasttime;
		if ( steptime < 0 )
	//FIXME		I_Error ("steptime < 0");
			steptime = 0;

		oldz += steptime * 150;
		if ( oldz > pparams->simorg[ 2 ] )
			oldz = pparams->simorg[ 2 ];
		if ( pparams->simorg[ 2 ] - oldz > 18 )
			oldz = pparams->simorg[ 2 ] - 18;
		pparams->vieworg[ 2 ] += oldz - pparams->simorg[ 2 ];
		ViewModel->origin[ 2 ] += oldz - pparams->simorg[ 2 ];
	}
	else
	{
		oldz = pparams->simorg[ 2 ];
	}
#endif

	{
		static float lastorg[ 3 ];
		vec3_t delta;

		VectorSubtract( pparams->simorg, lastorg, delta );

		if ( Length( delta ) != 0.0 )
		{
			VectorCopy( pparams->simorg, ViewInterp.Origins[ ViewInterp.CurrentOrigin & ORIGIN_MASK ] );
			ViewInterp.OriginTime[ ViewInterp.CurrentOrigin & ORIGIN_MASK ] = pparams->time;
			ViewInterp.CurrentOrigin++;

			VectorCopy( pparams->simorg, lastorg );
		}
	}

	// Smooth out whole view in multiplayer when on trains, lifts
	if ( cl_vsmoothing && cl_vsmoothing->value &&
		(pparams->smoothing && (pparams->maxclients > 1)) )
	{
		int foundidx;
		int i;
		float t;

		if ( cl_vsmoothing->value < 0.0 )
		{
			gEngfuncs.Cvar_SetValue( "cl_vsmoothing", 0.0 );
		}

		t = pparams->time - cl_vsmoothing->value;

		for ( i = 1; i < ORIGIN_MASK; i++ )
		{
			foundidx = ViewInterp.CurrentOrigin - 1 - i;
			if ( ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ] <= t )
				break;
		}

		if ( i < ORIGIN_MASK &&  ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ] != 0.0 )
		{
			// Interpolate
			vec3_t delta;
			double frac;
			double dt;
			vec3_t neworg;

			dt = ViewInterp.OriginTime[ (foundidx + 1) & ORIGIN_MASK ] - ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ];
			if ( dt > 0.0 )
			{
				frac = (t - ViewInterp.OriginTime[ foundidx & ORIGIN_MASK ]) / dt;
				frac = V_min( 1.0, frac );
				VectorSubtract( ViewInterp.Origins[ (foundidx + 1) & ORIGIN_MASK ], ViewInterp.Origins[ foundidx & ORIGIN_MASK ], delta );
				VectorMA( ViewInterp.Origins[ foundidx & ORIGIN_MASK ], frac, delta, neworg );

				// Dont interpolate large changes
				if ( Length( delta ) < 64 )
				{
					VectorSubtract( neworg, pparams->simorg, delta );

					VectorAdd( pparams->simorg, delta, pparams->simorg );
					VectorAdd( pparams->vieworg, delta, pparams->vieworg );
					VectorAdd( ViewModel->origin, delta, ViewModel->origin );

				}
			}
		}
	}
}

void V_ViewNonClient( struct ref_params_s* pparams )
{
	// override all previous settings if the viewent isn't the client
	if ( pparams->viewentity > pparams->maxclients )
	{
		cl_entity_t *viewentity;
		viewentity = gEngfuncs.GetEntityByIndex( pparams->viewentity );
		if ( viewentity )
		{
			VectorCopy( viewentity->origin, pparams->vieworg );
			VectorCopy( viewentity->angles, pparams->viewangles );

			// Store off overridden viewangles
			v_angles = pparams->viewangles;
		}
	}
}

void V_CalcWaterOffset( ref_params_t* pparams, float &waterOffset )
{
		// never let view origin sit exactly on a node line, because a water plane can
	// dissapear when viewed with the eye exactly on it.
	// FIXME, we send origin at 1/128 now, change this?
	// the server protocol only specifies to 1/16 pixel, so add 1/32 in each axis

	pparams->vieworg[ 0 ] += 1.0 / 32;
	pparams->vieworg[ 1 ] += 1.0 / 32;
	pparams->vieworg[ 2 ] += 1.0 / 32;

	// Check for problems around water, move the viewer artificially if necessary 
	// -- this prevents drawing errors in GL due to waves

	cl_entity_t* pwater;
	waterOffset = 0;
	if ( pparams->waterlevel >= 2 )
	{
		int		i, contents, waterDist, waterEntity;
		vec3_t	point;
		waterDist = cl_waterdist->value;

		if ( pparams->hardware )
		{
			waterEntity = gEngfuncs.PM_WaterEntity( pparams->simorg );
			if ( waterEntity >= 0 && waterEntity < pparams->max_entities )
			{
				pwater = gEngfuncs.GetEntityByIndex( waterEntity );
				if ( pwater && (pwater->model != NULL) )
				{
					waterDist += (pwater->curstate.scale * 16);	// Add in wave height
				}
			}
		}
		else
		{
			waterEntity = 0;	// Don't need this in software
		}

		VectorCopy( pparams->vieworg, point );

		// Eyes are above water, make sure we're above the waves
		if ( pparams->waterlevel == 2 )
		{
			point[ 2 ] -= waterDist / 3.0f;
			for ( i = 0; i < waterDist; i++ )
			{
				contents = gEngfuncs.PM_PointContents( point, NULL );
				if ( contents > CONTENTS_WATER )
					break;
				point[ 2 ] += 1 / 3.0f;
			}
			waterOffset = (point[ 2 ] + waterDist) - pparams->vieworg[ 2 ];
			waterOffset /= 4.0f;
		}
		else
		{
			// eyes are under water.  Make sure we're far enough under
			point[ 2 ] += waterDist / 3.0f;

			for ( i = 0; i < waterDist; i++ )
			{
				contents = gEngfuncs.PM_PointContents( point, NULL );
				if ( contents <= CONTENTS_WATER )
					break;
				point[ 2 ] -= 1 / 3.0f;
			}
			waterOffset = (point[ 2 ] - waterDist) - pparams->vieworg[ 2 ];
			waterOffset /= 4.0f;
		}
	}

	pparams->vieworg[ 2 ] += waterOffset;
}

void V_CalculatePunchCLView( ref_params_t* pparams )
{
	Vector totalPunch = gHUD.m_clPunch.GetTotalPunch();

	for ( int i = 0; i < 3; i++ )
		pparams->cl_viewangles[ i ] += totalPunch[ i ] * pparams->frametime;
}

/*
==================
V_CalcRefdef

==================
*/
void V_CalcRefdef_HL( struct ref_params_s *pparams )
{
	cl_entity_t		*ent, *ViewModel;
	int				i;
	vec3_t			angles;
	float			bob, waterOffset;
	static viewinterp_t		ViewInterp;

	static float oldz = 0;
	static float lasttime;
	static float bobLastTime = 0;
	static double bobTime = 0;

	vec3_t camAngles, camForward, camRight, camUp;

	V_DriftPitch( pparams );

	if ( gEngfuncs.IsSpectateOnly() )
	{
		ent = gEngfuncs.GetEntityByIndex( g_iUser2 );
	}
	else
	{
		// ent is the player model ( visible when out of body )
		ent = gEngfuncs.GetLocalPlayer();
	}

	// ViewModel is the weapon model (only visible from inside body )
	ViewModel = gEngfuncs.GetViewModel();

	// transform the ViewModel offset by the model's matrix to get the offset from
	// model origin for the ViewModel
	V_CalcBob( pparams, 1.0, VB_SIN, bobTime, bob, bobLastTime );

	// refresh position
	VectorCopy( pparams->simorg, pparams->vieworg );
	pparams->vieworg[ 2 ] += (bob);
	VectorAdd( pparams->vieworg, pparams->viewheight, pparams->vieworg );

	VectorCopy( pparams->cl_viewangles, pparams->viewangles );

	gEngfuncs.V_CalcShake();
	gEngfuncs.V_ApplyShake( pparams->vieworg, pparams->viewangles, 1.0 );

	V_CalcWaterOffset( pparams, waterOffset );
	V_CalcViewRoll( pparams );

	V_AddIdle( pparams );

	// offsets
	VectorCopy( pparams->cl_viewangles, angles );

	AngleVectors( angles, pparams->forward, pparams->right, pparams->up );

	// don't allow cheats in multiplayer
	if ( pparams->maxclients <= 1 )
	{
		for ( i = 0; i < 3; i++ )
		{
			pparams->vieworg[ i ] += scr_ofsx->value*pparams->forward[ i ] + scr_ofsy->value*pparams->right[ i ] + scr_ofsz->value*pparams->up[ i ];
		}
	}

	// Treating cam_ofs[2] as the distance
	if ( CL_IsThirdPerson() )
	{
		vec3_t ofs;

		ofs[ 0 ] = ofs[ 1 ] = ofs[ 2 ] = 0.0;

		CL_CameraOffset( (float *)&ofs );

		VectorCopy( ofs, camAngles );
		camAngles[ ROLL ] = 0;

		AngleVectors( camAngles, camForward, camRight, camUp );

		for ( i = 0; i < 3; i++ )
		{
			pparams->vieworg[ i ] += -ofs[ 2 ] * camForward[ i ];
		}
	}

	// Give gun our viewangles
	VectorCopy( pparams->cl_viewangles, ViewModel->angles );

	// set up gun position
	V_CalcGunAngle( pparams );

	// Use predicted origin as ViewModel origin.
	VectorCopy( pparams->simorg, ViewModel->origin );
	ViewModel->origin[ 2 ] += (waterOffset);
	VectorAdd( ViewModel->origin, pparams->viewheight, ViewModel->origin );

	// Let the viewmodel shake at about 10% of the amplitude
	gEngfuncs.V_ApplyShake( ViewModel->origin, ViewModel->angles, 0.9 );

	for ( i = 0; i < 3; i++ )
	{
		ViewModel->origin[ i ] += bob * 0.4 * pparams->forward[ i ];
	}

	ViewModel->origin[ 2 ] += bob;

	if ( v_wonbob->value )
	{
		// throw in a little tilt.
		ViewModel->angles[ YAW ] -= bob * 0.5;
		ViewModel->angles[ ROLL ] -= bob * 1;
		ViewModel->angles[ PITCH ] -= bob * 0.3;
	}

	if ( v_wontilt->value )
	{
		float side = V_CalcRoll( ViewModel->angles, pparams->simvel, v_rollangle->value, v_rollspeed->value );
		pparams->viewangles[ ROLL ] += side;
	}

	// pushing the ViewModel origin down off of the same X/Z plane as the ent's origin will give the
	// gun a very nice 'shifting' effect when the player looks up/down. If there is a problem
	// with ViewModel model distortion, this may be a cause. (SJB). 
	ViewModel->origin[ 2 ] -= 1;

	VectorCopy( ViewModel->angles, ViewModel->curstate.angles );

	// fudge position around to keep amount of weapon visible
	// roughly equal with different FOV
	if ( pparams->viewsize == 110 )
	{
		ViewModel->origin[ 2 ] += 1;
	}
	else if ( pparams->viewsize == 100 )
	{
		ViewModel->origin[ 2 ] += 2;
	}
	else if ( pparams->viewsize == 90 )
	{
		ViewModel->origin[ 2 ] += 1;
	}
	else if ( pparams->viewsize == 80 )
	{
		ViewModel->origin[ 2 ] += 0.5;
	}

	// Add in the punchangle, if any
	VectorAdd( pparams->viewangles, pparams->punchangle, pparams->viewangles );

	// Include client side punch, too
	VectorAdd( pparams->viewangles, (float *)&ev_punchangle, pparams->viewangles );

	V_DropPunchAngle( pparams->frametime, (float *)&ev_punchangle );

	V_CalculatePunchCLView( pparams );

	V_CalcViewSmoothing( pparams, oldz, lasttime, ViewModel, ViewInterp );

	// Store off v_angles before munging for third person
	v_angles = pparams->viewangles;
	v_lastAngles = pparams->viewangles;
//	v_cl_angles = pparams->cl_viewangles;	// keep old user mouse angles !
	if ( CL_IsThirdPerson() )
	{
		VectorCopy( camAngles, pparams->viewangles );
		float pitch = camAngles[ 0 ];

		// Normalize angles
		if ( pitch > 180 )
			pitch -= 360.0;
		else if ( pitch < -180 )
			pitch += 360;

		// Player pitch is inverted
		pitch /= -3.0;

		// Slam local player's pitch value
		ent->angles[ 0 ] = pitch;
		ent->curstate.angles[ 0 ] = pitch;
		ent->prevstate.angles[ 0 ] = pitch;
		ent->latched.prevangles[ 0 ] = pitch;
	}

	V_ViewNonClient( pparams );

	lasttime = pparams->time;
	v_origin = pparams->vieworg;
}

void V_CalcRefdef_ADM( struct ref_params_s* pparams )
{
	cl_entity_t		*ent, *ViewModel;
	int				i;
	vec3_t			angles;
	float			v_positionbob, v_rollbob_y, v_rollbob_p, v_rollbob_r, v_positionbob_sway, waterOffset;
	float			rollBobTimeDiv, rollBobFrequency, RBPitch, RBYaw, RBRoll, bobPos, bobPosSway, bobHeight;
//	float			developer, adm_classicbob, adm_cam_sway, adm_cam_roll, adm_cam_falleffect;
	
	// double &bobtime, float &bob, float &lasttime
	static double bobtimes[ 5 ] = { 0,0,0,0,0 };
	static float lasttimes[ 5 ] = { 0,0,0,0,0 };

	static viewinterp_t		ViewInterp;

	// View angles
	static Vector vecDelta;			// Delta - difference between old angles and current angles, used for interpolation
	static Vector vecBlendAngles;	// BlendAngles - they are the ones which get interpolated
	static Vector vecResultAngles;	// ResultAngles - the resulting viewmodel angle, after BlendAngles, Old and Delta do their thing
	static Vector vecOld;			// Old - used to get the old angles, so we can interpolate between old and blend
	static Vector vecAdditive;		// Additive - we add extra sways on top of everything - used by the jump punch

	// "Camera" view angles
	static Vector vecClientPunch;	// ClientPunch - this vector modifies pparams->viewangles in the end
	static Vector vecFinalAngles;	// FinalAngles - ClientPunch + some other stuff

	// View origins
	static Vector vecOrgDelta;		// difference between the old view origin, and the current one, used to determine movement speed

	// Angle vectors
	static Vector vecOriginalFront;
	static Vector vecOriginalRight;
	static Vector vecOriginalUp;

	// Other floats
	static float oldz = 0;
	static float lasttime;
	static float nexttime = 0;
	static float flAngSway = 0;		// 0 to 360 angle, used for sine waves in view bobbin'
	static float flAngFallSway = 0;		// 0 to 60 angle, used to elevate the view a bit when falling
	static float flAngSwim = 0;		// another 0 to 360 angle, used for separate sine waves when swimming
	static float flAngJump = 1.57;	// this angle punches the view a bit, the moment you jump
	
	static float flOldSideMove = 0;
	static float flOldForwardMove = 0;
	static float flOldUpMove = 0;

	vec3_t camAngles, camForward, camRight, camUp;

	V_DriftPitch(pparams);

	if (gEngfuncs.IsSpectateOnly())
	{
		ent = gEngfuncs.GetEntityByIndex(g_iUser2);
	}
	else
	{
		// ent is the player model ( visible when out of body )
		ent = gEngfuncs.GetLocalPlayer();
	}

	// ViewModel is the weapon model (only visible from inside body)
	ViewModel = gEngfuncs.GetViewModel();
	vecFinalAngles = { 0, 0, 0 };

	rollBobTimeDiv			= adm_bob_roll_timediv->value;
	rollBobFrequency		= adm_bob_roll_freq->value;
	RBPitch					= adm_bob_roll_p->value;
	RBYaw					= adm_bob_roll_y->value;
	RBRoll					= adm_bob_roll_r->value;
	bobPos					= adm_bob_pos->value;
	bobPosSway				= adm_bob_pos2->value;
	bobHeight				= adm_bob_height->value;

	float classicBobEnable	= adm_classicbob_enable->value;
	float camSway			= adm_cam_sway->value;
	float camRoll			= adm_cam_roll->value;
	float camFallEffect		= adm_cam_falleffect->value;
	float camTurnBack		= adm_cam_turnback->value;
	float viewLerp			= adm_view_lerp1->value;
	float viewWeaponLerp	= adm_view_lerp2->value;
	float camOfsSide		= adm_cam_ofs_side->value;

	float flSideMove		= V_CalcRoll(ViewModel->angles, pparams->simvel, 60, 6000);
	float flForwardMove		= V_CalcRoll( ViewModel->angles, pparams->simvel, 8, 600, 0 ) * 0.5;
	float flUpMove			= V_CalcRoll( ViewModel->angles, pparams->simvel, 6, 300, 2 ) * 1.333;

	if ( flUpMove < 0 )
		flUpMove *= 0.36;

	// This constant works well lmao
	static float bobFrequencyModifier = 1.33;

	// If we're not on ground, slow stuff down
	if ( !pparams->onground )
		bobFrequencyModifier = bobFrequencyModifier * 0.9 + 0.25*0.1;
	else
		bobFrequencyModifier = bobFrequencyModifier * 0.9 + 1.33*0.1;

	// transform the view offset by the model's matrix to get the offset from
	// model origin for the view
	V_CalcBob( pparams, 1.60 * bobFrequencyModifier,				VB_SIN, bobtimes[0], v_positionbob, lasttimes[0] );
	V_CalcBob( pparams, 0.8 * bobFrequencyModifier,					VB_SIN, bobtimes[1], v_positionbob_sway, lasttimes[1] );
	V_CalcBob( pparams, rollBobFrequency * bobFrequencyModifier,	VB_SIN, bobtimes[2], v_rollbob_y, lasttimes[2] );
	V_CalcBob( pparams, rollBobFrequency * 2 * bobFrequencyModifier, VB_SIN, bobtimes[3], v_rollbob_p, lasttimes[3] );
	V_CalcBob( pparams, rollBobFrequency * 2 * bobFrequencyModifier, VB_SIN, bobtimes[4], v_rollbob_r, lasttimes[4] );

	v_positionbob *= bobPos;
	v_positionbob_sway *= bobPosSway;

	v_rollbob_y *= flForwardMove * RBYaw;
	v_rollbob_p *= flForwardMove * RBPitch;
	v_rollbob_r *= flForwardMove * RBRoll;

	// refresh position
	VectorCopy(pparams->simorg, pparams->vieworg);
	pparams->vieworg[2] += (v_positionbob / bobHeight);
	VectorAdd(pparams->vieworg, pparams->viewheight, pparams->vieworg);
	pparams->vieworg[2] += (v_positionbob * bobHeight);

	pparams->cl_viewangles[ ROLL ] = 0;
	VectorCopy(pparams->cl_viewangles, pparams->viewangles);

	gEngfuncs.V_CalcShake();
	gEngfuncs.V_ApplyShake(pparams->vieworg, pparams->viewangles, 1.0);

	V_CalcWaterOffset( pparams, waterOffset );
	V_CalcViewRoll(pparams);
	V_AddIdle(pparams);

	// offsets
	VectorCopy(pparams->cl_viewangles, angles);
	AngleVectors(angles, pparams->forward, pparams->right, pparams->up);
	AngleVectors( ViewModel->curstate.angles, vecOriginalFront, vecOriginalRight, vecOriginalUp );

	// don't allow cheats in multiplayer
	if (pparams->maxclients <= 1)
	{
		for (i = 0; i < 3; i++)
		{
			pparams->vieworg[i] += scr_ofsx->value*pparams->forward[i] + scr_ofsy->value*pparams->right[i] + scr_ofsz->value*pparams->up[i];
		}
	}

	// Treating cam_ofs[2] as the distance
	if (CL_IsThirdPerson())
	{
		vec3_t ofs;

		ofs[0] = ofs[1] = ofs[2] = 0.0;

		CL_CameraOffset((float *)&ofs);

		VectorCopy(ofs, camAngles);
		camAngles[ROLL] = 0;

		AngleVectors(camAngles, camForward, camRight, camUp);

		for (i = 0; i < 3; i++)
		{
			pparams->vieworg[i] += -ofs[2] * camForward[i];
			pparams->vieworg[i] += camOfsSide * camRight[i];
		}
	}

	// Give gun our viewangles
	VectorCopy(pparams->cl_viewangles, ViewModel->angles);

	// set up gun position
	V_CalcGunAngle(pparams);

	// Use predicted origin as view origin.
	VectorCopy(pparams->simorg, ViewModel->origin);
	ViewModel->origin[2] += (waterOffset);
	VectorAdd(ViewModel->origin, pparams->viewheight, ViewModel->origin);

	// Let the viewmodel shake at about 10% of the amplitude
	gEngfuncs.V_ApplyShake(ViewModel->origin, ViewModel->angles, 0.5);

	float flBlendForwardMove =  (0.90 * flOldForwardMove)	+ (0.10 * flForwardMove);
	float flBlendSideMove =		(0.90 * flOldSideMove)		+ (0.10 * flSideMove);
	float flBlendUpMove =		(0.93 * flOldUpMove)		+ (0.07 * flUpMove);

	flOldForwardMove = flBlendForwardMove;
	flOldSideMove = flBlendSideMove;
	flOldUpMove = flBlendUpMove;

	for (i = 0; i < 2; i++)
	{
		ViewModel->origin[ i ] += 2.00 * v_positionbob_sway	* pparams->right[i];
		ViewModel->origin[ i ] += 0.65 * flBlendForwardMove	* pparams->forward[i];
		ViewModel->origin[ i ] += 1.00 * flBlendSideMove	* pparams->right[i];
		ViewModel->origin[ i ] += 0.25 * flBlendUpMove		* pparams->up[i];
	}

	ViewModel->origin[ 2 ] -= 0.65 * flBlendUpMove;

	// NOTE TO NEW PROGRAMMERS
	// BUGGY SHIT STARTS HERE
	// DO NOT TOUCH UNLESS YOU KNOW VECTOR MATHS

	if (camSway)
	{
		vecOrgDelta[0] = v_origin[0] - pparams->vieworg[0];
		vecOrgDelta[1] = v_origin[1] - pparams->vieworg[1];
		vecOrgDelta[2] = v_origin[2] - pparams->vieworg[2];

		float movingSpeedXY = vecOrgDelta.Length2D();
		float movingSpeed = vecOrgDelta.Length();

		if ( movingSpeedXY > 0.01)
		{
			vecClientPunch[PITCH] += pparams->frametime * 30.0 * sin(flAngSway) + 2 * sin(4 * flAngSway) * (movingSpeedXY * 0.125);
			vecClientPunch[YAW]	  += pparams->frametime * 30.0 * sin(flAngSway) + 3 * sin(2 * flAngSway) * (movingSpeedXY * 0.125);

			vecClientPunch = vecClientPunch / 50;
			vecClientPunch = vecClientPunch * movingSpeedXY;
			vecClientPunch = vecClientPunch * camSway;
		}

		else
		{
			vecClientPunch[PITCH] += sin(flAngSway) + 4 * sin(2 * flAngSway);
			vecClientPunch[YAW]	  += sin(flAngSway) + (1 * sin(4 * flAngSway));

			vecClientPunch = vecClientPunch / 100;
			vecClientPunch = vecClientPunch * camSway;

			flAngSway += pparams->frametime * 60.0 * M_PI / 3600;
		}

		if (camFallEffect && !pparams->waterlevel)
		{

			if (!pparams->onground)
			{
				flAngFallSway += 1.0;
				flAngSway	  += flAngFallSway * (M_PI / 7200);
				flAngJump	  += 0.11 * pparams->frametime;

				if (flAngFallSway > 60)
				{
					flAngFallSway -= 1.0;
				}
				
				if (flAngJump >= M_PI)
				{
					flAngJump = M_PI;
				}
			}

			else
			{
				flAngFallSway /= 1.05f;
				flAngJump	  /= 1.02f;
			}

			if (camTurnBack)
			{
				vecClientPunch[ROLL] += (camTurnBack * vecDelta.z) / 10;
				vecClientPunch[ROLL] *= camFallEffect * (((flAngFallSway + 1) / 12) + vecOrgDelta.Length() * 0.125);
			}

			vecClientPunch[ROLL] /= 1.19;
		}
		
		if (pparams->waterlevel > 2) // If we're strictly under water!!!
		{
			flAngSwim += pparams->frametime * 60.0 * M_PI / 180;
			flAngJump /= 1.015;
		}

		else
		{
			if (pparams->waterlevel == 2)
			{
				flAngFallSway += 0.25;

				if (flAngFallSway >= 10)
				{
					flAngFallSway -= 0.30;
				}
			}

			else
			{
				flAngFallSway /= 1.01;
			}

			flAngSwim /= 1.05;
			flAngJump /= 1.015;
		}

		if (flAngFallSway)
		{
			vecClientPunch[PITCH] += pparams->frametime * 30.0 * camFallEffect * min(3.0, movingSpeed*0.05) * ((flAngFallSway * sin(flAngSway*2.0) / 2.0) + 1.0);
			vecClientPunch[ROLL]  += pparams->frametime * 30.0 * camFallEffect * min(3.0, movingSpeed*0.05) *  (flAngFallSway * pow(sin(flAngSway*2.0),3) / 1.5);
		}

		if (flAngSwim)
		{
			vecClientPunch[PITCH] += pparams->frametime * 15.0 * 4   * sin(2 * flAngSwim);
			vecClientPunch[YAW]   += pparams->frametime * 15.0 * 2   * sin(4 * flAngSwim);
			vecClientPunch[ROLL]  += pparams->frametime * 15.0 * 3   * pow(sin(2 * flAngSwim), 3);
			vecAdditive[YAW]	  -= pparams->frametime * 15.0 * 0.2 * sin(2 * flAngSwim);
			vecAdditive[ROLL]	  -= pparams->frametime * 15.0 * 0.6 * sin(4 * flAngSwim);
		}

		if (flAngJump)
		{
			vecClientPunch[PITCH] += pparams->frametime * 50.0 * 1.6 * sin(flAngJump);
			vecClientPunch[ROLL]  += pparams->frametime * 50.0 * 1.6 * sin(flAngJump);
			ViewModel->origin.z   -= pparams->frametime * 30.0 * 0.4 * sin(flAngJump * 1.75);
			vecAdditive[PITCH]	  -= pparams->frametime * 40.0 * 1.7 * pow(sin(flAngJump * 2.0), 3);
			vecAdditive[YAW]	  += pparams->frametime * 15.0 * 0.9 * sin(flAngJump * 2.0) * flBlendSideMove;
		}

		vecClientPunch = vecClientPunch / 1.05;
		vecAdditive = vecAdditive / 1.05;

		flAngSway += pparams->frametime * 60.0 * (1 + vecOrgDelta.Length2D() * 0.1) * M_PI / 90;
		flAngSway += pparams->frametime * 60.0 * M_PI / 120;

		if (flAngSway >= M_PI)
			flAngSway *= -1;

		if (flAngSwim >= M_PI)
			flAngSwim *= -1;
	}

	if (lasttime < 0.017)
		vecOld = v_lastAngles;

	if (nexttime < lasttime)
	{
		nexttime = lasttime + 0.1;
	}

	if (vecOld.y >= 180 && ViewModel->angles.y <= 90)
	{
		ViewModel->angles.y += 360;

		vecDelta = vecOld - ViewModel->angles;

		ViewModel->angles.y -= 360;
	}

	else if (vecOld.y <= 90 && ViewModel->angles.y >= 180)
	{
		vecOld.y += 360;

		vecDelta = vecOld - ViewModel->angles;

		if (vecDelta.y < 0)
		{
			vecOld.y -= 360;
		}
	}

	else
	{
		vecDelta = vecOld - ViewModel->angles;

		if (vecOld.y < -90 && ViewModel->angles.y > 90)
		{
			vecDelta.y += 360;
		}
	}

	vecDelta = vecDelta / 4.5f;
	vecDelta.z = vecDelta.z + (vecDelta.y / 2.333f);

	float horizontalDelta = 0.5 + fabs(vecDelta.x / 0.25f);

	float oldZDelta = vecDelta.z;

	vecDelta.z = vecDelta.z*0.95 + (vecDelta.z * (horizontalDelta))*0.05;

	if (camRoll)
	{
		if (CL_IsThirdPerson())
			vecFinalAngles[ROLL] += (vecDelta.z * camRoll) / 3.5f;
		else
			vecFinalAngles[ROLL] -= (vecDelta.z * camRoll) / 3.5f;
		
		vecFinalAngles[ROLL]	+= pparams->frametime * 60.0 * (flSideMove * camRoll) * 2;
		vecFinalAngles[ PITCH ] -= pparams->frametime * 60.0 * (flForwardMove * camRoll) * 0.5;
	}

	if (camSway)
	{
		vecFinalAngles[PITCH] += vecClientPunch[PITCH];
		vecFinalAngles[YAW]	  += vecClientPunch[YAW];
		vecFinalAngles[ROLL]  += vecClientPunch[ROLL];

		// gun rotating around its pivot
		vecAdditive[YAW]	  -= vecDelta[YAW]*0.15;
		vecAdditive[PITCH]    -= vecDelta[PITCH]*0.15;

		for ( int i = 0; i < 3; i++ )
		{
			ViewModel->origin[ i ] -= pparams->right[ i ] * vecDelta[ YAW ] * 0.35;
			ViewModel->origin[ i ] += pparams->up[ i ] * vecDelta[ PITCH ] * 0.35;
		}
	}

	vecBlendAngles = ViewModel->angles + vecDelta;
	InterpolateAngles(vecBlendAngles, ViewModel->angles, vecResultAngles, viewLerp); // adm_view_lerp1

	vecOld = vecOld + vecDelta;
	InterpolateAngles(vecOld, vecBlendAngles, vecOld, viewWeaponLerp); // adm_view_lerp2

	for (int i = 0; i < 3; i++)
	{
		if (vecFinalAngles[i] <= -360)
			vecFinalAngles[i] = 0;

		if (vecFinalAngles[i] >= 360)
			vecFinalAngles[i] = 0;
	}

	for (int i = 0; i < 3; i++)
	{
		if (ViewModel->angles[i] <= -360)
			ViewModel->angles[i] = 0;

		if (ViewModel->angles[i] >= 360)
			ViewModel->angles[i] = 0;
	}

	vecFinalAngles = vecFinalAngles + ViewModel->angles;
	vecFinalAngles[0] *= -1;

	vecResultAngles = vecResultAngles + vecAdditive;

	if ( rollBobFrequency > 0.0 )
	{
		vecResultAngles[ PITCH ] += v_rollbob_p;
		vecResultAngles[ ROLL ] += v_rollbob_r;
		vecResultAngles[ YAW ] += v_rollbob_y;
	}

	// Thank you Shepard for telling me about that one line <3
	if (classicBobEnable && !gEngfuncs.IsNoClipping())
	{
		VectorCopy( vecResultAngles, ViewModel->curstate.angles );
		VectorCopy( vecFinalAngles, pparams->viewangles );
		//VectorCopy(vecFinalAngles, pparams->cl_viewangles);
		//pparams->viewangles[ ROLL ] = vecFinalAngles[ ROLL ];
	}
	else
	{
		VectorCopy(ViewModel->angles, ViewModel->curstate.angles);
	}

	if (developer->value == 4)
	{
		// Debugging, gotta display all the angles here...
		gEngfuncs.Con_Printf("View\nAngles: old %d delta %d blend %d vm %d result %d final %d\nposbob %d posbobsw %d rbobp %d rboby %d last %f \n",
			(int)vecOld.y,
			(int)vecDelta.y,
			(int)vecBlendAngles[YAW],
			(int)ViewModel->angles[YAW],
			(int)vecResultAngles[YAW],
			(int)vecFinalAngles[YAW],
			(int)v_positionbob, (int)v_positionbob_sway, (int)v_rollbob_p, (int)v_rollbob_y,
			lasttime);
		//ConsolePrintf("View angles: p %f y %f r %f", view->curstate.angles[PITCH], view->curstate.angles[YAW], view->curstate.angles[ROLL]);
	}

	// pushing the view origin down off of the same X/Z plane as the ent's origin will give the
	// gun a very nice 'shifting' effect when the player looks up/down. If there is a problem
	// with view model distortion, this may be a cause. (SJB). 
	//ViewModel->origin[2] -= 1;

	// HL2-style, viewmodel goes up when we look up or down, but stays still when we look straight
	ViewModel->origin[ 2 ] += 1.0;

	for ( int i = 0; i < 2; i++ )
		ViewModel->origin[ i ] += 2.0 * pparams->up[i] * (pparams->cl_viewangles[ PITCH ] / 90.0);

	if ( !pparams->onground )
	{
		flAngJump += pparams->frametime * 60.0 * 0.128;

		if ( flAngJump >= M_PI )
			flAngJump = M_PI;
	}
	else
	{
		flAngJump /= 1.1f;
	}

	ViewModel->origin.z -= pparams->frametime * 60.0 * 1.5 * sin( flAngJump * 1.75 );

	// Mathematically determine the position according to FOV
	// to keep amount of weapon visible roughly equal with 
	// different FOV
	for ( int i = 0; i < 3; i++ )
		ViewModel->origin[ i ] += (pparams->up[ i ] * (pparams->viewsize * pparams->viewsize) / 8000.0) -Vector( 0, 0, 2 )[ i ];

	// Add in the punchangle, if any
	VectorAdd ( pparams->viewangles, pparams->punchangle, pparams->viewangles );

	// Include client side punch, too
	VectorAdd ( pparams->viewangles, (float *)&ev_punchangle, pparams->viewangles);

	V_DropPunchAngle ( pparams->frametime, (float *)&ev_punchangle );

	V_CalcViewSmoothing( pparams, oldz, lasttime, ViewModel, ViewInterp );

	// Store off v_angles before munging for third person
	v_angles = pparams->viewangles;
	v_lastAngles = pparams->viewangles;
//	v_cl_angles = pparams->cl_viewangles;	// keep old user mouse angles !
	if ( CL_IsThirdPerson() )
	{
		camAngles[PITCH] = vecFinalAngles[PITCH];
//		camAngles[YAW] = vecFinalAngles[YAW];
		camAngles[ROLL] = vecFinalAngles[ROLL];

		VectorCopy( camAngles, pparams->viewangles);
		float pitch = camAngles[ 0 ];
		float roll =  camAngles[ 2 ] * 5;

		// Normalize angles
		if ( pitch > 180 ) 
			pitch -= 360.0;
		else if ( pitch < -180 )
			pitch += 360.0;

		if (roll > 180)
			roll -= 360.0;
		else if (roll < -180)
			roll += 360.0;

		// Player pitch is inverted
		pitch /= -3.0;

		// Slam local player's pitch value
		ent->angles[ 0 ] = pitch;
		ent->curstate.angles[ 0 ] = pitch;
		ent->prevstate.angles[ 0 ] = pitch;
		ent->latched.prevangles[ 0 ] = pitch;

		ent->angles[ 2 ] = roll;
		ent->curstate.angles[2] = roll;
		ent->prevstate.angles[2] = roll;
		ent->latched.prevangles[2] = roll;
	}

	lasttime = pparams->time;
	v_origin = pparams->vieworg;
}

void V_CalcNormalRefdef( struct ref_params_s* pparams )
{
	v_velocity = pparams->simvel;

	V_CalcRefdef_HL( pparams );
	//V_CalcRefdef_ADM( pparams );
}

// Get the origin of the Observer based around the target's position and angles
void V_GetChaseOrigin( float * angles, float * origin, float distance, float * returnvec )
{
	vec3_t	vecEnd;
	vec3_t	forward;
	vec3_t	vecStart;
	pmtrace_t * trace;
	int maxLoops = 8;

	int ignoreent = -1;	// first, ignore no entity
	
	cl_entity_t	 *	ent = NULL;
	
	// Trace back from the target using the player's view angles
	AngleVectors(angles, forward, NULL, NULL);
	
	VectorScale(forward,-1,forward);

	VectorCopy( origin, vecStart );

	VectorMA(vecStart, distance , forward, vecEnd);

	while ( maxLoops > 0)
	{
		trace = gEngfuncs.PM_TraceLine( vecStart, vecEnd, PM_TRACELINE_PHYSENTSONLY, 2, ignoreent );

		// WARNING! trace->ent is is the number in physent list not the normal entity number

		if ( trace->ent <= 0)
			break;	// we hit the world or nothing, stop trace

		ent = gEngfuncs.GetEntityByIndex( PM_GetPhysEntInfo( trace->ent ) );

		if ( ent == NULL )
			break;

		// hit non-player solid BSP , stop here
		if ( ent->curstate.solid == SOLID_BSP && !ent->player ) 
			break;

		// if close enought to end pos, stop, otherwise continue trace
		if( Distance(trace->endpos, vecEnd ) < 1.0f )
		{
			break;
		}
		else
		{
			ignoreent = trace->ent;	// ignore last hit entity
			VectorCopy( trace->endpos, vecStart);
		}

		maxLoops--;
	}  

/*	if ( ent )
	{
		gEngfuncs.Con_Printf("Trace loops %i , entity %i, model %s, solid %i\n",(8-maxLoops),ent->curstate.number, ent->model->name , ent->curstate.solid ); 
	} */

	VectorMA( trace->endpos, 4, trace->plane.normal, returnvec );

	v_lastDistance = Distance(trace->endpos, origin);	// real distance without offset
}

/*void V_GetDeathCam(cl_entity_t * ent1, cl_entity_t * ent2, float * angle, float * origin)
{
	float newAngle[3]; float newOrigin[3]; 

	float distance = 168.0f;

	v_lastDistance+= v_frametime * 96.0f;	// move unit per seconds back

	if ( v_resetCamera )
		v_lastDistance = 64.0f;

	if ( distance > v_lastDistance )
		distance = v_lastDistance;

	VectorCopy(ent1->origin, newOrigin);

	if ( ent1->player )
		newOrigin[2]+= 17; // head level of living player

	// get new angle towards second target
	if ( ent2 )
	{
		VectorSubtract( ent2->origin, ent1->origin, newAngle );
		VectorAngles( newAngle, newAngle );
		newAngle[0] = -newAngle[0];
	}
	else
	{
		// if no second target is given, look down to dead player
		newAngle[0] = 90.0f;
		newAngle[1] = 0.0f;
		newAngle[2] = 0;
	}

	// and smooth view
	V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 120.0f );
			
	V_GetChaseOrigin( angle, newOrigin, distance, origin );

	VectorCopy(angle, v_lastAngles);
}*/

void V_GetSingleTargetCam(cl_entity_t * ent1, float * angle, float * origin)
{
	float newAngle[3]; float newOrigin[3]; 
	
	int flags 	   = gHUD.m_Spectator.m_iObserverFlags;

	// see is target is a dead player
	qboolean deadPlayer = ent1->player && (ent1->curstate.solid == SOLID_NOT);
	
	float dfactor   = ( flags & DRC_FLAG_DRAMATIC )? -1.0f : 1.0f;

	float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;
	
	// go away in final scenes or if player just died
	if ( flags & DRC_FLAG_FINAL )
		distance*=2.0f;	
	else if ( deadPlayer )
		distance*=1.5f;	

	// let v_lastDistance float smoothly away
	v_lastDistance+= v_frametime * 32.0f;	// move unit per seconds back

	if ( distance > v_lastDistance )
		distance = v_lastDistance;
	
	VectorCopy(ent1->origin, newOrigin);

	if ( ent1->player )
	{
		if ( deadPlayer )  
			newOrigin[2]+= 2;	//laying on ground
		else
			newOrigin[2]+= 17; // head level of living player
			
	}
	else
		newOrigin[2]+= 8;	// object, tricky, must be above bomb in CS

	// we have no second target, choose view direction based on
	// show front of primary target
	VectorCopy(ent1->angles, newAngle);

	// show dead players from front, normal players back
	if ( flags & DRC_FLAG_FACEPLAYER )
		newAngle[1]+= 180.0f;


	newAngle[0]+= 12.5f * dfactor; // lower angle if dramatic

	// if final scene (bomb), show from real high pos
	if ( flags & DRC_FLAG_FINAL )
		newAngle[0] = 22.5f; 

	// choose side of object/player			
	if ( flags & DRC_FLAG_SIDE )
		newAngle[1]+=22.5f;
	else
		newAngle[1]-=22.5f;

	V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 120.0f );

	// HACK, if player is dead don't clip against his dead body, can't check this
	V_GetChaseOrigin( angle, newOrigin, distance, origin );
}

float MaxAngleBetweenAngles(  float * a1, float * a2 )
{
	float d, maxd = 0.0f;

	NormalizeAngles( a1 );
	NormalizeAngles( a2 );

	for ( int i = 0 ; i < 3 ; i++ )
	{
		d = a2[i] - a1[i];
		if ( d > 180 )
		{
			d -= 360;
		}
		else if ( d < -180 )
		{	
			d += 360;
		}

		d = fabs(d);

		if ( d > maxd )
			maxd=d;
	}

	return maxd;
}

void V_GetDoubleTargetsCam(cl_entity_t	 * ent1, cl_entity_t * ent2,float * angle, float * origin)
{
	float newAngle[3]; float newOrigin[3]; float tempVec[3];

	int flags 	   = gHUD.m_Spectator.m_iObserverFlags;

	float dfactor   = ( flags & DRC_FLAG_DRAMATIC )? -1.0f : 1.0f;

	float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;
	
	// go away in final scenes or if player just died
	if ( flags & DRC_FLAG_FINAL )
		distance*=2.0f;	
	
	// let v_lastDistance float smoothly away
	v_lastDistance+= v_frametime * 32.0f;	// move unit per seconds back

	if ( distance > v_lastDistance )
		distance = v_lastDistance;

	VectorCopy(ent1->origin, newOrigin);

	if ( ent1->player )
		newOrigin[2]+= 17; // head level of living player
	else
		newOrigin[2]+= 8;	// object, tricky, must be above bomb in CS

	// get new angle towards second target
	VectorSubtract( ent2->origin, ent1->origin, newAngle );

	VectorAngles( newAngle, newAngle );
	newAngle[0] = -newAngle[0];

	// set angle diffrent in Dramtaic scenes
	newAngle[0]+= 12.5f * dfactor; // lower angle if dramatic
			
	if ( flags & DRC_FLAG_SIDE )
		newAngle[1]+=22.5f;
	else
		newAngle[1]-=22.5f;

	float d = MaxAngleBetweenAngles( v_lastAngles, newAngle );

	if ( ( d < v_cameraFocusAngle) && ( v_cameraMode == CAM_MODE_RELAX ) )
	{
		// difference is to small and we are in relax camera mode, keep viewangles
		VectorCopy(v_lastAngles, newAngle );
	}
	else if ( (d < v_cameraRelaxAngle) && (v_cameraMode == CAM_MODE_FOCUS) )
	{
		// we catched up with our target, relax again
		v_cameraMode = CAM_MODE_RELAX;
	}
	else
	{
		// target move too far away, focus camera again
		v_cameraMode = CAM_MODE_FOCUS;
	}

	// and smooth view, if not a scene cut
	if ( v_resetCamera || (v_cameraMode == CAM_MODE_RELAX) )
	{
		VectorCopy( newAngle, angle );
	}
	else
	{
		V_SmoothInterpolateAngles( v_lastAngles, newAngle, angle, 180.0f );
	}

	V_GetChaseOrigin( newAngle, newOrigin, distance, origin );

	// move position up, if very close at target
	if ( v_lastDistance < 64.0f )
		origin[2]+= 16.0f*( 1.0f - (v_lastDistance / 64.0f ) );

	// calculate angle to second target
	VectorSubtract( ent2->origin, origin, tempVec );
	VectorAngles( tempVec, tempVec );
	tempVec[0] = -tempVec[0];

	/* take middle between two viewangles
	InterpolateAngles( newAngle, tempVec, newAngle, 0.5f); */
	
	

}

void V_GetDirectedChasePosition(cl_entity_t	 * ent1, cl_entity_t * ent2,float * angle, float * origin)
{

	if ( v_resetCamera )
	{
		v_lastDistance = 4096.0f;
		// v_cameraMode = CAM_MODE_FOCUS;
	}

	if ( ( ent2 == (cl_entity_t*)0xFFFFFFFF ) || ( ent1->player && (ent1->curstate.solid == SOLID_NOT) ) )
	{
		// we have no second target or player just died
		V_GetSingleTargetCam(ent1, angle, origin);
	}
	else if ( ent2 )
	{
		// keep both target in view
		V_GetDoubleTargetsCam( ent1, ent2, angle, origin );
	}
	else
	{
		// second target disappeard somehow (dead)

		// keep last good viewangle
		float newOrigin[3];

		int flags 	   = gHUD.m_Spectator.m_iObserverFlags;

		float dfactor   = ( flags & DRC_FLAG_DRAMATIC )? -1.0f : 1.0f;

		float distance = 112.0f + ( 16.0f * dfactor ); // get close if dramatic;
	
		// go away in final scenes or if player just died
		if ( flags & DRC_FLAG_FINAL )
			distance*=2.0f;	
	
		// let v_lastDistance float smoothly away
		v_lastDistance+= v_frametime * 32.0f;	// move unit per seconds back

		if ( distance > v_lastDistance )
			distance = v_lastDistance;
		
		VectorCopy(ent1->origin, newOrigin);

		if ( ent1->player )
			newOrigin[2]+= 17; // head level of living player
		else
			newOrigin[2]+= 8;	// object, tricky, must be above bomb in CS

		V_GetChaseOrigin( angle, newOrigin, distance, origin );
	}

	VectorCopy(angle, v_lastAngles);
}

void V_GetChasePos(int target, float * cl_angles, float * origin, float * angles)
{
	cl_entity_t	 *	ent = NULL;
	
	if ( target ) 
	{
		ent = gEngfuncs.GetEntityByIndex( target );
	};
	
	if (!ent)
	{
		// just copy a save in-map position
		VectorCopy ( vJumpAngles, angles );
		VectorCopy ( vJumpOrigin, origin );
		return;
	}
	
	
	
	if ( gHUD.m_Spectator.m_autoDirector->value )
	{
		if ( g_iUser3 )
			V_GetDirectedChasePosition( ent, gEngfuncs.GetEntityByIndex( g_iUser3 ),
				angles, origin );
		else
			V_GetDirectedChasePosition( ent, ( cl_entity_t*)0xFFFFFFFF,
				angles, origin );
	}
	else
	{
		if ( cl_angles == NULL )	// no mouse angles given, use entity angles ( locked mode )
		{
			VectorCopy ( ent->angles, angles);
			angles[0]*=-1;
		}
		else
			VectorCopy ( cl_angles, angles);


		VectorCopy ( ent->origin, origin);
		
		origin[2]+= 28; // DEFAULT_VIEWHEIGHT - some offset

		V_GetChaseOrigin( angles, origin, cl_chasedist->value, origin );
	}

	v_resetCamera = false;	
}

void V_ResetChaseCam()
{
	v_resetCamera = true;
}

void V_GetInEyePos(int target, float * origin, float * angles )
{
	if ( !target)
	{
		// just copy a save in-map position
		VectorCopy ( vJumpAngles, angles );
		VectorCopy ( vJumpOrigin, origin );
		return;
	};


	cl_entity_t	 * ent = gEngfuncs.GetEntityByIndex( target );

	if ( !ent )
		return;

	VectorCopy ( ent->origin, origin );
	VectorCopy ( ent->angles, angles );

	angles[PITCH]*=-3.0f;	// see CL_ProcessEntityUpdate()

	if ( ent->curstate.solid == SOLID_NOT )
	{
		angles[ROLL] = 80;	// dead view angle
		origin[2]+= -8 ; // PM_DEAD_VIEWHEIGHT
	}
	else if (ent->curstate.usehull == 1 )
		origin[2]+= 12; // VEC_DUCK_VIEW;
	else
		// exacty eye position can't be caluculated since it depends on
		// client values like cl_bobcycle, this offset matches the default values
		origin[2]+= 28; // DEFAULT_VIEWHEIGHT
}

void V_GetMapFreePosition( float * cl_angles, float * origin, float * angles )
{
	vec3_t forward;
	vec3_t zScaledTarget;

	VectorCopy(cl_angles, angles);

	// modify angles since we don't wanna see map's bottom
	angles[0] = 51.25f + 38.75f*(angles[0]/90.0f);

	zScaledTarget[0] = gHUD.m_Spectator.m_mapOrigin[0];
	zScaledTarget[1] = gHUD.m_Spectator.m_mapOrigin[1];
	zScaledTarget[2] = gHUD.m_Spectator.m_mapOrigin[2] * (( 90.0f - angles[0] ) / 90.0f );
	

	AngleVectors(angles, forward, NULL, NULL);

	VectorNormalize(forward);

	VectorMA(zScaledTarget, -( 4096.0f / gHUD.m_Spectator.m_mapZoom ), forward , origin);
}

void V_GetMapChasePosition(int target, float * cl_angles, float * origin, float * angles)
{
	vec3_t forward;

	if ( target )
	{
		cl_entity_t	 *	ent = gEngfuncs.GetEntityByIndex( target );

		if ( gHUD.m_Spectator.m_autoDirector->value )
		{
			// this is done to get the angles made by director mode
			V_GetChasePos(target, cl_angles, origin, angles);
			VectorCopy(ent->origin, origin);
			
			// keep fix chase angle horizontal
			angles[0] = 45.0f;
		}
		else
		{
			VectorCopy(cl_angles, angles);
			VectorCopy(ent->origin, origin);

			// modify angles since we don't wanna see map's bottom
			angles[0] = 51.25f + 38.75f*(angles[0]/90.0f);
		}
	}
	else
	{
		// keep out roaming position, but modify angles
		VectorCopy(cl_angles, angles);
		angles[0] = 51.25f + 38.75f*(angles[0]/90.0f);
	}

	origin[2] *= (( 90.0f - angles[0] ) / 90.0f );
	angles[2] = 0.0f;	// don't roll angle (if chased player is dead)

	AngleVectors(angles, forward, NULL, NULL);

	VectorNormalize(forward);

	VectorMA(origin, -1536, forward, origin); 
}

int V_FindViewModelByWeaponModel(int weaponindex)
{

	static char * modelmap[][2] =	{

# ifdef _TFC	// TFC models override HL models
		{ "models/p_mini.mdl",			"models/v_tfac.mdl"			},
		{ "models/p_sniper.mdl",		"models/v_tfc_sniper.mdl"	},
		{ "models/p_umbrella.mdl",		"models/v_umbrella.mdl"		},
		{ "models/p_crowbar.mdl",		"models/v_tfc_crowbar.mdl"	},
		{ "models/p_spanner.mdl",		"models/v_tfc_spanner.mdl"	},
		{ "models/p_knife.mdl",			"models/v_tfc_knife.mdl"	},
		{ "models/p_medkit.mdl",		"models/v_tfc_medkit.mdl"	},
		{ "models/p_egon.mdl",			"models/v_flame.mdl"		},
		{ "models/p_glauncher.mdl",		"models/v_tfgl.mdl"			},
		{ "models/p_rpg.mdl",			"models/v_tfc_rpg.mdl"		},
		{ "models/p_nailgun.mdl",		"models/v_tfc_nailgun.mdl"	},
		{ "models/p_snailgun.mdl",		"models/v_tfc_supernailgun.mdl" },
		{ "models/p_9mmhandgun.mdl",	"models/v_tfc_railgun.mdl"	},
		{ "models/p_srpg.mdl",			"models/v_tfc_rpg.mdl"		},
		{ "models/p_smallshotgun.mdl",	"models/v_tfc_12gauge.mdl"	},
		{ "models/p_shotgun.mdl",		"models/v_tfc_shotgun.mdl"	},
		{ "models/p_spygun.mdl",		"models/v_tfc_pistol.mdl"	},
#endif
		{ "models/p_crossbow.mdl",		"models/v_crossbow.mdl"		},
		{ "models/p_crowbar.mdl",		"models/v_crowbar.mdl"		},
		{ "models/p_egon.mdl",			"models/v_egon.mdl"			},
		{ "models/p_gauss.mdl",			"models/v_gauss.mdl"		},
		{ "models/p_9mmhandgun.mdl",	"models/v_9mmhandgun.mdl"	},
		{ "models/p_grenade.mdl",		"models/v_grenade.mdl"		},
		{ "models/p_hgun.mdl",			"models/v_hgun.mdl"			},
		{ "models/p_9mmAR.mdl",			"models/v_9mmAR.mdl"		},
		{ "models/p_357.mdl",			"models/v_357.mdl"			},
		{ "models/p_rpg.mdl",			"models/v_rpg.mdl"			},
		{ "models/p_shotgun.mdl",		"models/v_shotgun.mdl"		},
		{ "models/p_squeak.mdl",		"models/v_squeak.mdl"		},
		{ "models/p_tripmine.mdl",		"models/v_tripmine.mdl"		},
		{ "models/p_satchel_radio.mdl",	"models/v_satchel_radio.mdl"},
		{ "models/p_satchel.mdl",		"models/v_satchel.mdl"		},
		{ NULL, NULL } };

	struct model_s * weaponModel = IEngineStudio.GetModelByIndex( weaponindex );

	if ( weaponModel )
	{
		int len = strlen( weaponModel->name );
		int i = 0;

		while ( modelmap[i] != NULL )
		{
			if ( !strnicmp( weaponModel->name, modelmap[i][0], len ) )
			{
				return gEngfuncs.pEventAPI->EV_FindModelIndex( modelmap[i][1] );
			}
			i++;
		}

		return 0;
	}
	else
		return 0;

}

/*
==================
V_CalcSpectatorRefdef

==================
*/
void V_CalcSpectatorRefdef ( struct ref_params_s * pparams )
{
	static vec3_t			velocity ( 0.0f, 0.0f, 0.0f);

	static int lastWeaponModelIndex = 0;
	static int lastViewModelIndex = 0;
		
	cl_entity_t	 * ent = gEngfuncs.GetEntityByIndex( g_iUser2 );
	
	pparams->onlyClientDraw = false;

	// refresh position
	VectorCopy ( pparams->simorg, v_sim_org );

	// get old values
	VectorCopy ( pparams->cl_viewangles, v_cl_angles );
	VectorCopy ( pparams->viewangles, v_angles );
	VectorCopy ( pparams->vieworg, v_origin );

	if (  ( g_iUser1 == OBS_IN_EYE || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE ) && ent )
	{
		// calculate player velocity
		float timeDiff = ent->curstate.msg_time - ent->prevstate.msg_time;

		if ( timeDiff > 0 )
		{
			vec3_t distance;
			VectorSubtract(ent->prevstate.origin, ent->curstate.origin, distance);
			VectorScale(distance, 1/timeDiff, distance );

			velocity[0] = velocity[0]*0.9f + distance[0]*0.1f;
			velocity[1] = velocity[1]*0.9f + distance[1]*0.1f;
			velocity[2] = velocity[2]*0.9f + distance[2]*0.1f;
			
			VectorCopy(velocity, pparams->simvel);
		}

		// predict missing client data and set weapon model ( in HLTV mode or inset in eye mode )
#ifdef _TFC
		if ( gEngfuncs.IsSpectateOnly() || gHUD.m_Spectator.m_pip->value == INSET_IN_EYE )
#else
		if ( gEngfuncs.IsSpectateOnly() )
#endif
		{
			V_GetInEyePos( g_iUser2, pparams->simorg, pparams->cl_viewangles );

			pparams->health = 1;

			cl_entity_t	 * gunModel = gEngfuncs.GetViewModel();

			if ( lastWeaponModelIndex != ent->curstate.weaponmodel )
			{
				// weapon model changed

				lastWeaponModelIndex = ent->curstate.weaponmodel;
				lastViewModelIndex = V_FindViewModelByWeaponModel( lastWeaponModelIndex );
				if ( lastViewModelIndex )
				{
					gEngfuncs.pfnWeaponAnim(0,0);	// reset weapon animation
				}
				else
				{
					// model not found
					gunModel->model = NULL;	// disable weapon model
					lastWeaponModelIndex = lastViewModelIndex = 0;
				}
			}

			if ( lastViewModelIndex )
			{
				gunModel->model = IEngineStudio.GetModelByIndex( lastViewModelIndex );
				gunModel->curstate.modelindex = lastViewModelIndex;
				gunModel->curstate.frame = 0;
				gunModel->curstate.colormap = 0; 
				gunModel->index = g_iUser2;
			}
			else
			{
				gunModel->model = NULL;	// disable weaopn model
			}
		}
		else
		{
			// only get viewangles from entity
			VectorCopy ( ent->angles, pparams->cl_viewangles );
			pparams->cl_viewangles[PITCH]*=-3.0f;	// see CL_ProcessEntityUpdate()
		}
	}

	v_frametime = pparams->frametime;

	if ( pparams->nextView == 0 )
	{
		// first renderer cycle, full screen

		switch ( g_iUser1 )
		{
			case OBS_CHASE_LOCKED:	V_GetChasePos( g_iUser2, NULL, v_origin, v_angles );
									break;

			case OBS_CHASE_FREE:	V_GetChasePos( g_iUser2, v_cl_angles, v_origin, v_angles );
									break;

			case OBS_ROAMING	:	VectorCopy (v_cl_angles, v_angles);
									VectorCopy (v_sim_org, v_origin);
									
									// override values if director is active
									gHUD.m_Spectator.GetDirectorCamera(v_origin, v_angles);
									break;

			case OBS_IN_EYE		:   V_CalcNormalRefdef ( pparams );
									break;
				
			case OBS_MAP_FREE  :	pparams->onlyClientDraw = true;
									V_GetMapFreePosition( v_cl_angles, v_origin, v_angles );
									break;

			case OBS_MAP_CHASE  :	pparams->onlyClientDraw = true;
									V_GetMapChasePosition( g_iUser2, v_cl_angles, v_origin, v_angles );
									break;
		}

		if ( gHUD.m_Spectator.m_pip->value )
			pparams->nextView = 1;	// force a second renderer view

		gHUD.m_Spectator.m_iDrawCycle = 0;

	}
	else
	{
		// second renderer cycle, inset window

		// set inset parameters
		pparams->viewport[0] = XRES(gHUD.m_Spectator.m_OverviewData.insetWindowX);	// change viewport to inset window
		pparams->viewport[1] = YRES(gHUD.m_Spectator.m_OverviewData.insetWindowY);
		pparams->viewport[2] = XRES(gHUD.m_Spectator.m_OverviewData.insetWindowWidth);
		pparams->viewport[3] = YRES(gHUD.m_Spectator.m_OverviewData.insetWindowHeight);
		pparams->nextView	 = 0;	// on further view

		// override some settings in certain modes
		switch ( (int)gHUD.m_Spectator.m_pip->value )
		{
			case INSET_CHASE_FREE : V_GetChasePos( g_iUser2, v_cl_angles, v_origin, v_angles );
									break;	

			case INSET_IN_EYE	 :	V_CalcNormalRefdef ( pparams );
									break;

			case INSET_MAP_FREE  :	pparams->onlyClientDraw = true;
									V_GetMapFreePosition( v_cl_angles, v_origin, v_angles );
									break;

			case INSET_MAP_CHASE  :	pparams->onlyClientDraw = true;

									if ( g_iUser1 == OBS_ROAMING )
										V_GetMapChasePosition( 0, v_cl_angles, v_origin, v_angles );
									else
										V_GetMapChasePosition( g_iUser2, v_cl_angles, v_origin, v_angles );

									break;
		}

		gHUD.m_Spectator.m_iDrawCycle = 1;
	}

	// write back new values into pparams
	VectorCopy ( v_cl_angles, pparams->cl_viewangles );
	VectorCopy ( v_angles, pparams->viewangles )
	VectorCopy ( v_origin, pparams->vieworg );

}

void DLLEXPORT V_CalcRefdef( struct ref_params_s *pparams )
{
//	RecClCalcRefdef(pparams);

	// intermission / finale rendering
	if ( pparams->intermission )
	{	
		V_CalcIntermissionRefdef ( pparams );	
	}
	else if ( pparams->spectator || g_iUser1 )	// g_iUser true if in spectator mode
	{
		V_CalcSpectatorRefdef ( pparams );	
	}
	else if ( !pparams->paused )
	{
		V_CalcNormalRefdef ( pparams );
	}

	System::SetPausedMode( pparams->paused );

/*
// Example of how to overlay the whole screen with red at 50 % alpha
#define SF_TEST
#if defined SF_TEST
	{
		screenfade_t sf;
		gEngfuncs.pfnGetScreenFade( &sf );

		sf.fader = 255;
		sf.fadeg = 0;
		sf.fadeb = 0;
		sf.fadealpha = 128;
		sf.fadeFlags = FFADE_STAYOUT | FFADE_OUT;

		gEngfuncs.pfnSetScreenFade( &sf );
	}
#endif
*/
}

/*
=============
V_DropPunchAngle

=============
*/
void V_DropPunchAngle ( float frametime, float *ev_punchangle )
{
	float	len;
	
	len = VectorNormalize ( ev_punchangle );
	len -= (10.0 + len * 0.5) * frametime;
	len = V_max( len, 0.0 );
	VectorScale ( ev_punchangle, len, ev_punchangle );
}

void V_PunchCLView( Vector angles, float speed )
{
	gHUD.m_clPunch.AddPunch( angles, speed );
}

/*
=============
V_PunchAxis

Client side punch effect
=============
*/
void V_PunchAxis( int axis, float punch )
{
#if ADM_CustomClientPunch == 0

	ev_punchangle[ axis ] = punch;

#else 

	Vector angle( 0, 0, 0 );
	angle[ axis ] = punch * 4.0;

	V_PunchCLView( angle, 25.0f );

#endif
}

void V_PunchAxis( float pitch, float yaw, float roll )
{
	ev_punchangle[ PITCH ] = pitch;
	ev_punchangle[ YAW ] = yaw;
	ev_punchangle[ ROLL ] = roll;
}

/*
=============
V_Init
=============
*/
void V_Init (void)
{
	gEngfuncs.pfnAddCommand ("centerview", V_StartPitchDrift );

	scr_ofsx			= gEngfuncs.pfnRegisterVariable( "scr_ofsx","0", 0 );
	scr_ofsy			= gEngfuncs.pfnRegisterVariable( "scr_ofsy","0", 0 );
	scr_ofsz			= gEngfuncs.pfnRegisterVariable( "scr_ofsz","0", 0 );

	v_centermove		= gEngfuncs.pfnRegisterVariable( "v_centermove", "0.15", 0 );
	v_centerspeed		= gEngfuncs.pfnRegisterVariable( "v_centerspeed","500", 0 );

	cl_bobcycle			= gEngfuncs.pfnRegisterVariable( "cl_bobcycle","0.8", 0 );// best default for my experimental gun wag (sjb)
	cl_bob				= gEngfuncs.pfnRegisterVariable( "cl_bob","0.01", 0 );// best default for my experimental gun wag (sjb)
	cl_bobup			= gEngfuncs.pfnRegisterVariable( "cl_bobup","0.5", 0 );
	cl_waterdist		= gEngfuncs.pfnRegisterVariable( "cl_waterdist","4", 0 );
	cl_chasedist		= gEngfuncs.pfnRegisterVariable( "cl_chasedist","112", 0 );
}


//#define TRACE_TEST
#if defined( TRACE_TEST )

extern float in_fov;
/*
====================
CalcFov
====================
*/
float CalcFov (float fov_x, float width, float height)
{
	float	a;
	float	x;

	if (fov_x < 1 || fov_x > 179)
		fov_x = 90;	// error, set to 90

	x = width/tan(fov_x/360*M_PI);

	a = atan (height/x);

	a = a*360/M_PI;

	return a;
}

int hitent = -1;

void V_Move( int mx, int my )
{
	float fov;
	float fx, fy;
	float dx, dy;
	float c_x, c_y;
	float dX, dY;
	vec3_t forward, up, right;
	vec3_t newangles;

	vec3_t farpoint;
	pmtrace_t tr;

	fov = CalcFov( in_fov, (float)ScreenWidth, (float)ScreenHeight );

	c_x = (float)ScreenWidth / 2.0;
	c_y = (float)ScreenHeight / 2.0;

	dx = (float)mx - c_x;
	dy = (float)my - c_y;

	// Proportion we moved in each direction
	fx = dx / c_x;
	fy = dy / c_y;

	dX = fx * in_fov / 2.0 ;
	dY = fy * fov / 2.0;

	newangles = v_angles;

	newangles[ YAW ] -= dX;
	newangles[ PITCH ] += dY;

	// Now rotate v_forward around that point
	AngleVectors ( newangles, forward, right, up );

	farpoint = v_origin + 8192 * forward;

	// Trace
	tr = *(gEngfuncs.PM_TraceLine( (float *)&v_origin, (float *)&farpoint, PM_TRACELINE_PHYSENTSONLY, 2 /*point sized hull*/, -1 ));

	if ( tr.fraction != 1.0 && tr.ent != 0 )
	{
		hitent = PM_GetPhysEntInfo( tr.ent );
		PM_ParticleLine( (float *)&v_origin, (float *)&tr.endpos, 5, 1.0, 0.0 );
	}
	else
	{
		hitent = -1;
	}
}

#endif
