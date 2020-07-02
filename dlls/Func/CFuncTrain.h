
//
//====================== TRAIN code ==================================================
//

class CFuncTrain : public CBasePlatTrain
{
public:
	void Spawn( void );
	void Precache( void );
	void Activate( void );
	void OverrideReset( void );

	void Blocked( CBaseEntity *pOther );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData *pkvd );

	void EXPORT Wait( void );
	void EXPORT Next( void );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	entvars_t	*m_pevCurrentTarget;
	int			m_sounds;
	BOOL		m_activated;
};
