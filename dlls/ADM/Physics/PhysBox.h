class CPhysBox : public CBaseEntity
{
public:
	void			Spawn();
	void			KeyValue( KeyValueData* pkvd );

	void EXPORT		PhysThink();
	void EXPORT		PhysSpawnThink();
	void			Touch( CBaseEntity* other );

protected:
	CPhysBody_Box	physBody;
	PhysParameters	params;
};
