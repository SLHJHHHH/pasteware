#include "framework.h"

CKreedz g_Kreedz;

	static constexpr auto JUMPBUG_DISTANCE = 2.f;
static constexpr auto WALLBUG_DISTANCE = 4.f;
static constexpr auto STRAFE_ANGLE = 90.f;
static constexpr auto MAX_MOVE_SPEED = 250.f;
static constexpr auto FASTRUN_MAX_SPEED = 277.f;

static float AngleDifference(const float& a, const float& b)
{
	float difference = a - b;

	while (difference > 180.f)
		difference -= 360.f;

	while (difference < -180.f)
		difference += 360.f;

	return difference;
}

CKreedz::CKreedz()
	: m_bSlowWalkKey(false)
	, m_bFastRunKey(false)
	, m_bDeGenKey(false)
	, m_bWallAhead(false)
	, m_bEdgeJumpKey(false)
{
}

bool CKreedz::IsActive() const
{
	if (!cvars::kreedz.active)
		return false;

	if (!g_Local->m_bIsConnected || g_Local->m_bIsDead)
		return false;

	if (g_Local->m_bIsOnLadder || g_Local->m_bIsInWater)
		return false;

	return true;
}

bool CKreedz::WallTrace(const Vector& origin, const Vector& direction, const float& distance, pmtrace_t& tr)
{
	Vector start, end;

	start = origin;
	end = origin + direction * distance;

	g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
	g_Engine.pEventAPI->EV_PlayerTrace(start, end, PM_NORMAL, -1, &tr);

	return tr.fraction < 1.f;
}

void CKreedz::Frame()
{



	m_bWallAhead = false;

	if (!Game::IsConnected())
		return;

	if (!cvars::kreedz.wallbug)
		return;

	if (!g_Local->m_bIsConnected || g_Local->m_bIsDead || g_Local->m_bIsOnGround)
		return;

	Vector vecForward;

	g_Local->m_QAngles.AngleVectors(&vecForward, NULL, NULL);

	vecForward.z = 0.f;
	vecForward.Normalize();

	pmtrace_t tr;

	if (!WallTrace(g_Local->m_vecOrigin, vecForward, WALLBUG_DISTANCE, tr))
		return;


	if (g_Engine.pEventAPI->EV_IndexFromTrace(&tr) > 0)
		return;

	m_bWallAhead = true;
}

void CKreedz::Run(usercmd_s* cmd)
{
	if (!IsActive())
		return;

	BunnyHop(cmd);
	JumpBug(cmd);
	EdgeBug(cmd);
	WallBug(cmd);
	SlowWalk(cmd);
	FastRun(cmd);
	GroundStrafe(cmd);

	if (cvars::kreedz.auto_jof)
		AutoJOF(cmd);

	if (cvars::kreedz.rage_strafe)
		Strafe(cmd, false);
	else if (cvars::kreedz.legit_strafe)
		Strafe(cmd, true);

	if (cvars::kreedz.degen)
		DeGen(cmd);
}

void CKreedz::EdgeJump(usercmd_s* cmd)
{
	if (!cvars::kreedz.edgejump)
		return;

	if (!g_Local->m_bIsOnGround || g_Local->m_bIsOnLadder || g_Local->m_bIsInWater)
		return;

	if (cvars::kreedz.edgejump_key.keynum && !m_bEdgeJumpKey)
		return;

	Vector vecForward, vecRight;

	g_Local->m_QAngles.AngleVectors(&vecForward, &vecRight, NULL);

	vecForward.z = 0.f;
	vecForward.Normalize();
	vecRight.z = 0.f;
	vecRight.Normalize();

	Vector vecVelocity(g_Local->m_vecVelocity);

	vecVelocity.z = 0.f;

	float flMoveSpeed = vecVelocity.Length();

	if (flMoveSpeed <= 0.f)
		return;

	pmtrace_t tr;

	// check where the current velocity will land within the jump distance
	if (!WallTrace(g_Local->m_vecOrigin, vecVelocity / flMoveSpeed, cvars::kreedz.edgejump_distance, tr))
		return;

	if (g_Engine.pEventAPI->EV_IndexFromTrace(&tr) > 0)
		return;

	// the floor ends ahead: jump before running off the edge
	cmd->buttons |= IN_JUMP;
}

void CKreedz::BunnyHop(usercmd_s* cmd)
{
	if (!cvars::kreedz.bunnyhop)
		return;



	if (g_Local->m_bIsOnGround)
		cmd->buttons |= IN_JUMP;
}

void CKreedz::GroundStrafe(usercmd_s* cmd)
{
	if (!cvars::kreedz.groundstrafe)
		return;


	if (!g_Local->m_bIsOnGround)
		return;

	Vector vecVelocity = g_Local->m_vecVelocity;

	const float flSpeed = vecVelocity.ToVec2D().Length();


	if (flSpeed < 20.f)
		return;

	const float flMaxspeed = (pmove) ? pmove->maxspeed : MAX_MOVE_SPEED;















	const float flGain = std::clamp(cvars::kreedz.groundstrafe_gain, 0.f, 1.f);












	const float flVelYaw = RAD2DEG(atan2(vecVelocity.y, vecVelocity.x));
	const float flDeltaDeg = AngleDifference(cmd->viewangles.y, flVelYaw);
	const float flTheta = DEG2RAD(flDeltaDeg);

	const float flAbsDelta = (flDeltaDeg >= 0.f) ? flDeltaDeg : -flDeltaDeg;

	static bool bFlip = false;
	static float s_flSideSign = 1.f;

	if (flAbsDelta < 15.f)
	{
		bFlip = !bFlip;

		if (flDeltaDeg >= 0.f)
			s_flSideSign = bFlip ? 1.f : -1.f;
		else
			s_flSideSign = bFlip ? -1.f : 1.f;
	}

	const float flDt = (g_Local->m_flFrameTime > 0.f) ? g_Local->m_flFrameTime : 0.01f;
	const float flBudget = 5.5f * flDt * flMaxspeed;
	const float flLost = 4.f * flDt * flSpeed;

	float flAlong = flLost;

	if (flAlong > flBudget)
		flAlong = flBudget;

	flAlong += flGain * (flBudget - flAlong);

	const float flSide = s_flSideSign * flBudget;

	const float flSinTheta = sin(flTheta);
	const float flCosTheta = cos(flTheta);







	const float flLen = std::sqrt(flAlong * flAlong + flSide * flSide);

	const float flDirAlong = (flLen > 0.f) ? (flAlong / flLen) : 1.f;
	const float flDirSide = (flLen > 0.f) ? (flSide / flLen) : 0.f;

	cmd->forwardmove = flMaxspeed * (flDirAlong * flCosTheta + flDirSide * flSinTheta);
	cmd->sidemove = flMaxspeed * (flDirAlong * flSinTheta - flDirSide * flCosTheta);
}

void CKreedz::FastRun(usercmd_s* cmd)
{
	if (!cvars::kreedz.fastrun)
		return;

	if (cvars::kreedz.fastrun_key.keynum && !m_bFastRunKey)
		return;

	if (!g_Local->m_bIsOnGround)
		return;

	if (!(cmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
		return;


	if ((cmd->buttons & IN_MOVELEFT && cmd->buttons & IN_MOVERIGHT) || (cmd->buttons & IN_FORWARD && cmd->buttons & IN_BACK))
		return;


	if (g_Local->m_flVelocity > FASTRUN_MAX_SPEED)
		return;




	const float flVelYaw = RAD2DEG(atan2(g_Local->m_vecVelocity.y, g_Local->m_vecVelocity.x));
	const float flAngleToMove = AngleDifference(cmd->viewangles.y, flVelYaw);

	const int nMoveYaw = (static_cast<int>(-flAngleToMove + 360.f)) % 360;
	const bool bRight = (nMoveYaw <= 180);

	float x = 135.063748f * (bRight ? -1.f : 1.f);
	float y = 267.876434f;

	const float flRadian = DEG2RAD(flAngleToMove);
	const float flSin = sin(flRadian);
	const float flCos = cos(flRadian);

	cmd->sidemove = x * flCos - y * flSin;
	cmd->forwardmove = y * flCos + x * flSin;

	x = cmd->sidemove;
	y = cmd->forwardmove;

	cmd->sidemove = x * flCos - y * flSin;
	cmd->forwardmove = y * flCos + x * flSin;
}

void CKreedz::AutoJOF(usercmd_s* cmd)
{
	if (g_Local->m_bIsOnGround || g_Local->m_bIsOnLadder || g_Local->m_bIsInWater)
		return;


	if (g_Local->m_flFallVelocity < 0.f || g_Local->m_flEdgeDistance <= cvars::kreedz.auto_jof_min_distance)
		return;


	if (g_Local->m_flEdgeDistance <= g_Local->m_flVelocity * g_Local->m_flFrameTime)
		cmd->buttons |= IN_JUMP;
}

void CKreedz::JumpBug(usercmd_s* cmd)
{
	if (!cvars::kreedz.jumpbug)
		return;

	if (g_Local->m_bIsOnGround || g_Local->m_flFallVelocity < 150.f)
		return;



	if (g_Local->m_flHeightGround > JUMPBUG_DISTANCE)
		return;



	cmd->buttons |= IN_JUMP | IN_DUCK;
}

void CKreedz::EdgeBug(usercmd_s* cmd)
{
	if (!cvars::kreedz.edgebug)
		return;

	if (g_Local->m_bIsOnGround || g_Local->m_flFallVelocity < 1.f)
		return;




	if (g_Local->m_flHeightInDuck > 1.f || g_Local->m_flHeightGround <= 1.f)
		return;

	cmd->buttons |= IN_DUCK;
}

void CKreedz::WallBug(usercmd_s* cmd)
{
	if (!cvars::kreedz.wallbug)
		return;

	if (g_Local->m_bIsOnGround)
		return;


	if (!m_bWallAhead)
		return;

	cmd->buttons |= IN_DUCK;
}

void CKreedz::SlowWalk(usercmd_s* cmd)
{
	if (!cvars::kreedz.slowwalk)
		return;

	if (cvars::kreedz.slowwalk_key.keynum && !m_bSlowWalkKey)
		return;

	const float flScale = std::clamp(cvars::kreedz.slowwalk_speed / MAX_MOVE_SPEED, 0.f, 1.f);

	cmd->forwardmove *= flScale;
	cmd->sidemove *= flScale;
}

void CKreedz::Strafe(usercmd_s* cmd, const bool& legit)
{
	if (g_Local->m_bIsOnGround)
		return;

	Vector vecVelocity = g_Local->m_vecVelocity;

	const float flSpeed = vecVelocity.ToVec2D().Length();

	if (flSpeed < 20.f)
		return;












	const float flVelYaw = RAD2DEG(atan2(vecVelocity.y, vecVelocity.x));
	const float flDeltaDeg = AngleDifference(cmd->viewangles.y, flVelYaw);
	const float flDelta = DEG2RAD(flDeltaDeg);

	const float flSide = (flDeltaDeg >= 0.f) ? 1.f : -1.f;

	const float flPower = 400.f;

	const float flSin = sin(flDelta);
	const float flCos = cos(flDelta);


	cmd->forwardmove = flPower * flSide * flSin;
	cmd->sidemove = flPower * -flSide * flCos;



	QAngle QAngles(cmd->viewangles);

	QAngles.Normalize();

	if (legit)
	{
		const float flSmoothing = std::clamp(cvars::kreedz.legit_strafe_speed / 100.f, 0.f, 1.f);

		QAngles.y += AngleDifference(flVelYaw, QAngles.y) * flSmoothing;
	}
	else
	{
		QAngles.y = flVelYaw;
	}

	QAngles.Normalize();




	Game::MakeAngle(QAngles, cmd);
}












































void CKreedz::DeGen(usercmd_s* cmd)
{
	if (cvars::kreedz.degen_key.keynum && !m_bDeGenKey)
		return;

	if (g_Local->m_bIsOnGround)
		return;

	Vector vecVelocity = g_Local->m_vecVelocity;

	const float flSpeed = vecVelocity.ToVec2D().Length();


	if (flSpeed < 20.f)
		return;

	const float flVelYaw = RAD2DEG(atan2(vecVelocity.y, vecVelocity.x));
	const float flDeltaDeg = AngleDifference(cmd->viewangles.y, flVelYaw);
	const float flDelta = DEG2RAD(flDeltaDeg);



	const float flSide = (flDeltaDeg >= 0.f) ? 1.f : -1.f;

	const float flPower = std::clamp(cvars::kreedz.degen_power, 1.f, 1000.f);
	const float flTilt = std::clamp(cvars::kreedz.degen_tilt, -0.9f, 0.9f);

	const float flSin = sin(flDelta);
	const float flCos = cos(flDelta);


	cmd->forwardmove = flPower * (flSide * flSin - flTilt * flCos);
	cmd->sidemove = flPower * (-flSide * flCos - flTilt * flSin);
}
