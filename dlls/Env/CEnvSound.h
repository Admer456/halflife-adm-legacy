// =================== ROOM SOUND FX ==========================================

class CEnvSound : public CPointEntity
{
public:
	void KeyValue( KeyValueData* pkvd );
	void Spawn( void );

	void Think( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flRadius;
	float m_flRoomtype;
};
