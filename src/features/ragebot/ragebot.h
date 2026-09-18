struct CRageBotTarget
{
	int				index;
	int				hitbox;
	int				point;
	int				damage;
	int				weight;

	float			fov;

	Vector			origin;
};

class CRageBot
{
public:
	CRageBot();
	~CRageBot();

	bool			m_bAimState;
	bool			m_bForceBody;
	bool			m_bForceMinDamage;

	QAngle			m_QAntiAimbotAngle;

	void			Run(usercmd_s* cmd);
	void			Aimbot(usercmd_s* cmd);
	void			AntiAimbot(usercmd_s* cmd);
	void			FakeLag(usercmd_s* cmd);
	
private:
	CRageBotTarget  m_TargetData;

	int				GetWeightOfHitbox(const int& nHitbox);

	float			GetScaleOfHitbox(const int& nHitbox);

	void			CorrectPhysentSolid(const int& nPlayerID);
};

extern std::unique_ptr<CRageBot> g_pRageBot;