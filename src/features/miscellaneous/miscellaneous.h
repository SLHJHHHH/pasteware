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
	bool			m_bDuckHullKey;

	int				m_iTimeBankReleaseTicks;
	float			m_flTimeBankCredit;

	double			m_dbSpeedBoostUntil;
	float			m_flSpeedBoostValue;

	int				m_iFrameSkipCounter;
	float			m_flGameSpeedMultiplier;

	float			m_flPositionAdjustmentInterpAmount;
	float			GetInterpAmount(const int &lerp);

	void			ChokedCommandsCounter();
	void			AutoReload(usercmd_s* cmd);
	void			AutoPistol(usercmd_s* cmd);
	void			NameStealer();
	void			KillSay(int victim, bool headshot);
	void			FakeLatency();
	void			RecordHUDCommands(usercmd_s* cmd);


	void			AutoBlock(usercmd_s* cmd);
	void			AutoBoost(usercmd_s* cmd);
	void			FrameSkip();
	bool			ShouldSkipFrame() const;
	void			MaximizeOnRespawn();
	void			GameSpeed();
	float			GetGameSpeedMultiplier() const;
	void			RequestSpeedBoost(float value, double duration);
	void			AirStuck(usercmd_s* cmd);
	void			DoubleTap(usercmd_s* cmd);
	void			FakeDuckHull(usercmd_s* cmd);
	void			TimeBank(float frametime, usercmd_s* cmd);
	void			SetFakeLatencyHoldKey(bool down);
};

extern std::unique_ptr<CMiscellaneous> g_pMiscellaneous;