class CUtilServerCommand : public CBaseEntity
{
public:
	void			KeyValue( KeyValueData* pkvd );
	void			Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

protected:
	string_t		command = -1;
};