#pragma once

// Temporary class for osprey dust effect
class COspreyDustManager : public CPointEntity
{
public:

	static COspreyDustManager* CreateManager();

	void			Spawn( void );
	void			Think( void );
	void			DustEffect();

	TraceResult		traceSprite;
	TraceResult		traceOsprey;
	CBaseEntity*	m_pParent;
	CSprite*		m_prgParticles[ 32 ];
};