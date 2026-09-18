class CKreedz
{
public:
	CKreedz();

	bool m_bSlowWalkKey;
	bool m_bFastRunKey;
	bool m_bDeGenKey;

	void Run(usercmd_s* cmd);

	
	
	void Frame();

private:
	void BunnyHop(usercmd_s* cmd);
	void GroundStrafe(usercmd_s* cmd);
	void JumpBug(usercmd_s* cmd);
	void EdgeBug(usercmd_s* cmd);
	void WallBug(usercmd_s* cmd);
	void SlowWalk(usercmd_s* cmd);
	void FastRun(usercmd_s* cmd);
	void AutoJOF(usercmd_s* cmd);
	void DeGen(usercmd_s* cmd);
	void Strafe(usercmd_s* cmd, const bool& legit);

	static bool WallTrace(const Vector& origin, const Vector& direction, const float& distance, pmtrace_t& tr);

	bool m_bWallAhead;

	bool IsActive() const;
};

extern CKreedz g_Kreedz;