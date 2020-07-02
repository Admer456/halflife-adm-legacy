// =================== FUNC_ILLUSIONARY ==============================================

/*QUAKED func_illusionary (0 .5 .8) ?
A simple entity that looks solid but lets you walk through it.
*/
class CFuncIllusionary : public CBaseToggle
{
public:
	void Spawn( void );
	void EXPORT SloshTouch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	virtual int	ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};
