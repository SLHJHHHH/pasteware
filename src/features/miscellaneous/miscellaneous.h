class CMiscellaneous
{
public:
	CMiscellaneous();
	~CMiscellaneous();

	int				m_iChokedCommands;
	int				m_iHudCommands;

	bool			m_bFakeLatencyActive;
	bool			m_bPositionAdjustmentActive;

	
	int				m_iDoubleTapCharge;
	bool			m_bDoubleTapShifting;
	double			m_flLastDoubleTapTime;

	
	bool			m_bAirStuckActive;
	bool			m_bFakeLatencyHoldKey;
	bool			m_bAutoBlockKey;
	bool			m_bAutoBoostKey;
	bool			m_bSpeedKey;
	bool			m_bAirStuckKey;
	bool			m_bDoubleTapKey;

	int				m_iFrameSkipCounter;
	float			m_flGameSpeedMultiplier;

	float			m_flPositionAdjustmentInterpAmount;
	float			GetInterpAmount(const int &lerp);

	void			ChokedCommandsCounter();
	void			AutoReload(usercmd_s* cmd);
	void			AutoPistol(usercmd_s* cmd);
	void			NameStealer();
	void			FakeLatency();
	void			RecordHUDCommands(usercmd_s* cmd);

	
	void			AutoBlock(usercmd_s* cmd);
	void			AutoBoost(usercmd_s* cmd);
	void			PredictPlayers();
	void			FrameSkip();
	bool			ShouldSkipFrame() const;
	void			MaximizeOnRespawn();
	void			ApplyClientWeapons();
	void			GameSpeed();
	float			GetGameSpeedMultiplier() const;
	void			AirStuck(usercmd_s* cmd);
	void			DoubleTap(usercmd_s* cmd);
	bool			IsFakeLatencyHoldActive() const;
	void			SetFakeLatencyHoldKey(bool down);
	void			SetAutoBlockKey(bool down);
	void			SetAutoBoostKey(bool down);
	void			SetSpeedKey(bool down);
	void			SetAirStuckKey(bool down);
	void			SetDoubleTapKey(bool down);
};

extern std::unique_ptr<CMiscellaneous> g_pMiscellaneous;