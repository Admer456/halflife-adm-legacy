//=========================================================
// Soda can
//=========================================================
class CItemSoda : public CBaseEntity
{
public:
	void	Spawn( void );
	void	Precache( void );
	void	EXPORT CanThink( void );
	void	EXPORT CanTouch( CBaseEntity *pOther );
};
