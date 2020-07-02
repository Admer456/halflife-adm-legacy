
class CFuncWallToggle : public CFuncWall
{
public:
	void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void	TurnOff( void );
	void	TurnOn( void );
	BOOL	IsOn( void );
};