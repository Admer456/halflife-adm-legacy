#include "HUD/HUD.h"
#include "CL_Util.h"
#include "ParseMsg.h"
#include "PM_Shared.h"
#include "Const.h"

DECLARE_MESSAGE( m_clPunch, VPunch );

void CClientPunch::InitExtension()
{
	HOOK_MESSAGE( VPunch );
}

void CClientPunch::Think()
{
	for ( viewPunch &p : punchRegistry )
	{
		if ( p.punch.Length() < 0.1 )
		{
			p.speed = 0;
			p.wishSpeed = 0;
			p.active = false;
		}

		if ( !p.active )
			continue;

		p.speed = p.speed * 0.9 + p.wishSpeed * 0.1;
		p.punch = p.punch / (1.0 + (p.speed / 100));
	}
}

int CClientPunch::MsgFunc_VPunch( const char* pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	Vector receivePunch;
	float receiveSpeed = READ_COORD();
	receivePunch.x = READ_ANGLE();
	receivePunch.y = READ_ANGLE();
	receivePunch.z = READ_ANGLE();

	AddPunch( receivePunch, receiveSpeed );

	return 1;
}

void CClientPunch::AddPunch( Vector angles, float wishSpeed )
{
	for ( viewPunch &p : punchRegistry )
	{
		if ( p.active )
			continue;

		p.punch = angles;
		p.wishSpeed = wishSpeed;
		p.active = true;

		return;
	}
}

Vector CClientPunch::GetTotalPunch()
{
	Vector result = Vector(0,0,0);

	for ( viewPunch &p : punchRegistry )
	{
		if ( p.active )
			result = result + p.punch;
	}

	return result;
}