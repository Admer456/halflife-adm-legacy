class CPhysBox : public CBaseEntity
{
public:
	void			Spawn();
	void			KeyValue( KeyValueData* pkvd );

	void			Think();
	void			Touch( CBaseEntity* other );

protected:
	CPhysBody_Box	physBody;
	PhysParameters	params;
};
