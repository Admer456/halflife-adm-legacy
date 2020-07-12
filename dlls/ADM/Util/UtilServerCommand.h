class CUtilServerCommand : public CBaseEntity
{
public:
	void			KeyValue( KeyValueData* pkvd );
	void			Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value );

protected:
	string_t		command = -1;
};