class CLightning : public CBeam
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	KeyValue( KeyValueData *pkvd );
	void	Activate( void );

	void	EXPORT StrikeThink( void );
	void	EXPORT DamageThink( void );
	void	RandomArea( void );
	void	RandomPoint( Vector &vecSrc );
	void	Zap( const Vector &vecSrc, const Vector &vecDest );
	void	EXPORT StrikeUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT ToggleUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	inline BOOL ServerSide( void )
	{
		if ( m_life == 0 && !(pev->spawnflags & SF_BEAM_RING) )
			return TRUE;
		return FALSE;
	}

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void	BeamUpdateVars( void );

	int		m_active;
	int		m_iszStartEntity;
	int		m_iszEndEntity;
	float	m_life;
	int		m_boltWidth;
	int		m_noiseAmplitude;
	int		m_brightness;
	int		m_speed;
	float	m_restrike;
	int		m_spriteTexture;
	int		m_iszSpriteName;
	int		m_frameStart;

	float	m_radius;
};
