class CLegitBot
{
public:
	CLegitBot();
	~CLegitBot();

	bool					m_bAimState;
	bool					m_bAimPerfectSilentState;
	bool					m_bTriggerState;
	bool					m_bFiredFirstBullet;
	bool					m_bPerfectSilentShot;
	float m_flFlashAlpha;

	int						m_iAimPlayer;
	int						m_iAimHitbox;

	int						m_iTriggerAttempts;
	int						m_iTriggerMaxAttempts;

	float					m_flCurrentFOV;
	int						m_iStickyPlayer;

	void					Run(usercmd_s* cmd);

	bool					IsFlashed();
	bool					IsSmokeBlocked(const Vector& vecSrc, const Vector& vecEnd);
	void					OnTempEntity(const TEMPENTITY* pTemp);

private:
	float					m_flMinAngleDemoChecker;

	bool					IsValidTarget(const int& i);
	bool					GetAdjustedOrigin(cl_entity_s* pGameEntity, const int& i, Vector& vecOut);
	float					GetSmoothing(usercmd_s* cmd, const bool& bIsRCS);
	bool					CheckVisibility(const int& i);
	void					WriteMouseMovement(usercmd_s* cmd, const QAngle& QAngles, const QAngle& QNewAngles);

	void					Aimbot(usercmd_s* cmd);
	void					Trigger(usercmd_s* cmd);
	void					StandaloneRecoilControl(usercmd_s* cmd);
	void					CorrectPhysentSolid(const int& nPlayerID);
	void					SmoothAimAngles(const QAngle& QAngles, const QAngle& QAimAngles, QAngle& QNewAngles, const float& flSmoothing);
	void					DesyncHelper(usercmd_s* cmd);

	bool					DemoChecker(const QAngle& a_QPreviousAngles, const QAngle& a_QNewAngles, QAngle& a_QCorrectedAngles);
};

extern std::unique_ptr<CLegitBot> g_pLegitBot;