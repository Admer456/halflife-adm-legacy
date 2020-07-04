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
#ifndef SHAKE_H
#define SHAKE_H

// Screen / View effects

// screen shake
extern int gmsgShake;

// This structure is sent over the net to describe a screen shake event
typedef struct
{
	unsigned short	amplitude;		// FIXED 4.12 amount of shake
	unsigned short 	duration;		// FIXED 4.12 seconds duration
	unsigned short	frequency;		// FIXED 8.8 noise frequency (low frequency is a jerk,high frequency is a rumble)
} ScreenShake;

extern void V_ApplyShake( float *origin, float *angles, float factor );
extern void V_CalcShake( void );
extern int V_ScreenShake( const char *pszName, int iSize, void *pbuf );
extern int V_ScreenFade( const char *pszName, int iSize, void *pbuf );

#endif		// SHAKE_H

