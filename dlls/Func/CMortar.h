
class CMortar : public CGrenade
{
public:
	void Spawn( void );
	void Precache( void );

	void EXPORT MortarExplode( void );

	int m_spriteTexture;
};
