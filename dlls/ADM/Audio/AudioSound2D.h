#pragma once

class AudioSound2D : public CBaseEntity
{
public:
	void			Spawn() override;
	void			OnPlayerJoin( CBasePlayer* player ) override;
	void			KeyValue( KeyValueData* pkvd ) override;
	
	void			Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value ) override;
	void			Think() override;

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

private:
	string_t		soundName;
	float			volume;
	int				channelNumber;
};
