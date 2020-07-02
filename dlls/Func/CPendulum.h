class CPendulum : public CBaseEntity
{
public:
	void	Spawn( void );
	void	KeyValue( KeyValueData *pkvd );
	void	EXPORT Swing( void );
	void	EXPORT PendulumUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	EXPORT Stop( void );
	void	Touch( CBaseEntity *pOther );
	void	EXPORT RopeTouch( CBaseEntity *pOther );// this touch func makes the pendulum a rope
	virtual int	ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	void	Blocked( CBaseEntity *pOther );

	static	TYPEDESCRIPTION m_SaveData[];

	float	m_accel;			// Acceleration
	float	m_distance;			// 
	float	m_time;
	float	m_damp;
	float	m_maxSpeed;
	float	m_dampSpeed;
	vec3_t	m_center;
	vec3_t	m_start;
};
