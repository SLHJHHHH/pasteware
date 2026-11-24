class CKreedz
{
public:
	CKreedz();

	bool m_bSlowWalkKey;
	bool m_bFastRunKey;
	bool m_bDeGenKey;
	bool m_bEdgeJumpKey;
	bool m_bDDRunKey;
	bool m_bIdealStrafeKey;
	bool m_bRageStrafeKey;
	bool m_bLegitStrafeKey;
	bool m_bDopplerStrafeKey;
	bool m_bLadderStrafeKey;

	void Run(usercmd_s* cmd);

	void EdgeJump(usercmd_s* cmd);



	void Frame();

private:
	void BunnyHop(usercmd_s* cmd);
	void DDRun(usercmd_s* cmd);
	void GroundStrafe(usercmd_s* cmd);
	void RageGroundStrafe(usercmd_s* cmd);
	void LegitGroundStrafe(usercmd_s* cmd);
	void ReleaseGsDuck(usercmd_s* cmd);
	void ApplyGsNoSlowdown();
	void JumpBug(usercmd_s* cmd);
	void EdgeBug(usercmd_s* cmd);
	void WallBug(usercmd_s* cmd);
	void SlowWalk(usercmd_s* cmd);
	void FastRun(usercmd_s* cmd);
	void AutoJOF(usercmd_s* cmd);
	void DeGen(usercmd_s* cmd);
	void Strafe(usercmd_s* cmd, const bool& legit);
	void DopplerStrafe(usercmd_s* cmd);
	void LadderStrafe(usercmd_s* cmd);
	void IdealStrafe(usercmd_s* cmd);
	float ComputeOptimalPhi(const float& speed_2d, const float& wishspeed_budget, const float& accelspeed) const;
	float GetAirAccelerate() const;
	float GetPlayerFriction() const;
	float GetEffectiveMaxSpeed() const;
	Vector GetPMVelocity() const;
	float ComputeGoldSrcAirPhi(const float& speed, const float& wishspeed, const float& frametime) const;
	float SelectIdealStrafeSide(const usercmd_s* cmd, const float& velocity_yaw, const float& phi) const;
	void SteerMoves(usercmd_s* cmd, const float& vel_yaw, const float& view_yaw, const float& phi, const float& sign, const float& magnitude);

	static bool WallTrace(const Vector& origin, const Vector& direction, const float& distance, pmtrace_t& tr);

	bool m_bWallAhead;

	bool m_bGsDidDuck;
	int m_iDDRunStep;

	int		m_iDopplerAirTicks;
	int		m_iDopplerCyclePos;
	float	m_flDopplerSideSign;
	float	m_flIdealStrafeSide;

	bool IsActive() const;
};

extern CKreedz g_Kreedz;
