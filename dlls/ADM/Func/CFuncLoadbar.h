#define SF_LOADMOVE 1 << 0
#define SF_LOADTARG 1 << 1

class CFuncLoadbar : public CBaseEntity
{
public:
	void			Spawn();
	void			KeyValue( KeyValueData *pkvd );
	void			Use( CBaseEntity *pActivator, CBaseEntity *pOther, USE_TYPE useType, float value );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

private:
	float m_flNudge = 0.5;

	void TryPercentage1( CBaseEntity *pActivator, USE_TYPE USETYPE, float value );
	void TryPercentage2( CBaseEntity *pActivator, USE_TYPE USETYPE, float value );
	void TryPercentage3( CBaseEntity *pActivator, USE_TYPE USETYPE, float value );
	void TryPercentage4( CBaseEntity *pActivator, USE_TYPE USETYPE, float value );
	void TryPercentage5( CBaseEntity *pActivator, USE_TYPE USETYPE, float value );

	int m_iPercentage1; // percentages are from 1 to 100
	int m_iPercentage2;
	int m_iPercentage3;
	int m_iPercentage4;
	int m_iPercentage5;

	BOOL m_fPercentage1 = false;
	BOOL m_fPercentage2 = false;
	BOOL m_fPercentage3 = false;
	BOOL m_fPercentage4 = false;
	BOOL m_fPercentage5 = false;

	int m_iCompletion = 0; // completion goes from 0 to 255

	string_t m_iszTarget1;
	string_t m_iszTarget2;
	string_t m_iszTarget3;
	string_t m_iszTarget4;
	string_t m_iszTarget5;
};