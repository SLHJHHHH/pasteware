class CHitRegister
{
public:
	void AllocateMemory();
	void FreeMemory();

	bool RegisterHit(int index, char* pszSoundFile);

	void Update(usercmd_s* cmd);
	void Clear();

	CBaseHitInfo* operator->();

	int m_iResolverTarget;

private:
	std::unique_ptr<CBaseHitInfo> pBaseInfo;

	bool m_bIsInAttack;
	bool m_bIsInAttack2;

	int m_iMissedShots;

	int m_iMissStreak[MAX_CLIENTS] = { 0 };

	int m_iImpactIndex;
	int m_iImpactWeaponID;
	int m_iImpactHitboxNum;
	int m_iImpactAttackType;
	float m_flImpactTimeStamp;

	void UpdateAttackInfo(usercmd_s* cmd);

	void ClearAttackInfo();
	void ClearImpactInfo();
};

extern CHitRegister g_HitRegister;