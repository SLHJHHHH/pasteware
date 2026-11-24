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

static float GetMoveFrameTime(const usercmd_s* cmd)
{
	if (pmove && pmove->frametime > 0.f)
		return pmove->frametime;

	if (cmd && cmd->msec > 0)
		return static_cast<float>(cmd->msec) * 0.001f;

	return 0.f;
}

static float NormalizeRadians(const float& angle)
{
	return remainderf(angle, 6.28318530717958647692f);
}

CKreedz::CKreedz()
	: m_bSlowWalkKey(false)
	, m_bFastRunKey(false)
	, m_bDeGenKey(false)
	, m_bDDRunKey(false)
	, m_bIdealStrafeKey(false)
	, m_bRageStrafeKey(false)
	, m_bLegitStrafeKey(false)
	, m_bDopplerStrafeKey(false)
	, m_bLadderStrafeKey(false)
	, m_bWallAhead(false)
	, m_bGsDidDuck(false)
	, m_iDDRunStep(0)
	, m_bEdgeJumpKey(false)
	, m_iDopplerAirTicks(0)
	, m_iDopplerCyclePos(0)
	, m_flDopplerSideSign(1.f)
	, m_flIdealStrafeSide(1.f)
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
	if (!cvars::kreedz.active)
		return;

	if (!g_Local->m_bIsConnected || g_Local->m_bIsDead)
		return;

	const bool ideal_strafe = cvars::kreedz.strafe_ideal && (!cvars::kreedz.strafe_ideal_key.keynum || m_bIdealStrafeKey);
	const bool doppler_strafe = cvars::kreedz.doppler_strafe && (!cvars::kreedz.doppler_strafe_key.keynum || m_bDopplerStrafeKey);
	const bool rage_strafe = cvars::kreedz.rage_strafe && (!cvars::kreedz.rage_strafe_key.keynum || m_bRageStrafeKey);
	const bool legit_strafe = cvars::kreedz.legit_strafe && (!cvars::kreedz.legit_strafe_key.keynum || m_bLegitStrafeKey);
	const bool ladder_strafe = cvars::kreedz.ladder_strafe && (!cvars::kreedz.ladder_strafe_key.keynum || m_bLadderStrafeKey);

	if (ladder_strafe && g_Local->m_bIsOnLadder)
	{
		LadderStrafe(cmd);
		return;
	}

	if (!IsActive())
		return;

	BunnyHop(cmd);
	JumpBug(cmd);
	EdgeBug(cmd);
	WallBug(cmd);
	SlowWalk(cmd);
	FastRun(cmd);
	GroundStrafe(cmd);
	DDRun(cmd);

	if (cvars::kreedz.auto_jof)
		AutoJOF(cmd);

	if (ideal_strafe)
		IdealStrafe(cmd);
	else if (doppler_strafe)
		DopplerStrafe(cmd);
	else if (rage_strafe)
		Strafe(cmd, false);
	else if (legit_strafe)
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

	const bool on_ground = pmove ? pmove->onground != -1 : g_Local->m_bIsOnGround;

	if (on_ground)
	{
		cmd->buttons |= IN_JUMP;
		return;
	}

	if (cvars::kreedz.bunnyhop_no_slowdown)
		cmd->buttons &= ~IN_JUMP;
}

void CKreedz::DDRun(usercmd_s* cmd)
{
	if (!cvars::kreedz.ddrun || (cvars::kreedz.ddrun_key.keynum && !m_bDDRunKey))
	{
		m_iDDRunStep = 0;
		return;
	}

	if (m_iDDRunStep == 0 || m_iDDRunStep == 1 || m_iDDRunStep >= 4)
		cmd->buttons |= IN_DUCK;
	else
		cmd->buttons &= ~IN_DUCK;

	if (m_iDDRunStep < 4)
		++m_iDDRunStep;
}

void CKreedz::GroundStrafe(usercmd_s* cmd)
{
	if (!cvars::kreedz.groundstrafe)
		return;

	if (g_Local->m_bIsOnLadder || g_Local->m_bIsInWater || g_Local->m_flGroundAngle > 0.f)
		return;

	if (std::clamp(cvars::kreedz.gs_mode, 0, 1) == 1)
	{
		RageGroundStrafe(cmd);
		return;
	}

	if (m_bGsDidDuck)
		ReleaseGsDuck(cmd);
	else
		LegitGroundStrafe(cmd);
}

void CKreedz::RageGroundStrafe(usercmd_s* cmd)
{
	if (!g_Local->m_bIsOnGround)
	{
		ReleaseGsDuck(cmd);
		return;
	}

	if (g_Local->m_vecVelocity.ToVec2D().Length() < 20.f)
	{
		ReleaseGsDuck(cmd);
		return;
	}

	cmd->buttons |= IN_DUCK;
	m_bGsDidDuck = true;

	ApplyGsNoSlowdown();
}

void CKreedz::LegitGroundStrafe(usercmd_s* cmd)
{
	if (!g_Local->m_bIsOnGround)
	{
		ReleaseGsDuck(cmd);
		return;
	}

	if (g_Local->m_vecVelocity.ToVec2D().Length() < 20.f)
	{
		ReleaseGsDuck(cmd);
		return;
	}

	const float ft = GetMoveFrameTime(cmd);

	const float fall_speed = -g_Local->m_vecVelocity.z;
	const float ground_dist = g_Local->m_flHeightGround;

	const float min_dist = std::clamp(cvars::kreedz.gs_min_dist, 1.f, 20.f);
	const float max_dist = std::clamp(cvars::kreedz.gs_max_dist, 2.f, 32.f);

	const bool landing = fall_speed > 0.f && ground_dist > 0.f && ((fall_speed * ft) * 2.f > ground_dist);
	const bool touch_window = ground_dist > min_dist && ground_dist < max_dist;

	if (landing || touch_window)
	{
		cmd->buttons |= IN_DUCK;
		m_bGsDidDuck = true;

		ApplyGsNoSlowdown();
	}
	else
	{
		ReleaseGsDuck(cmd);
	}
}

void CKreedz::ReleaseGsDuck(usercmd_s* cmd)
{
	if (m_bGsDidDuck)
	{
		cmd->buttons &= ~IN_DUCK;
		m_bGsDidDuck = false;
	}
}

void CKreedz::ApplyGsNoSlowdown()
{
	if (!cvars::kreedz.gs_noslowdown)
		return;

	const int factor = std::clamp(cvars::kreedz.gs_noslowdown_factor, 1, 10);

	if (!IS_NULLPTR(g_pMiscellaneous))
		g_pMiscellaneous->RequestSpeedBoost(1.f + 0.02f * static_cast<float>(factor), 0.06);
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

	Vector velocity = g_Local->m_vecVelocity;

	float speed = velocity.ToVec2D().Length();

	if (speed < 20.f)
		return;

	const float ft = GetMoveFrameTime(cmd);

	const float magnitude = legit ? std::clamp(cvars::kreedz.legit_strafe_speed * 4.f, 100.f, 400.f) : 400.f;

	float max_speed = (pmove) ? pmove->maxspeed : MAX_MOVE_SPEED;

	if (max_speed < 1.f)
		max_speed = MAX_MOVE_SPEED;

	const float wishspeed = (std::min)(magnitude, max_speed);

	const float phi = ComputeGoldSrcAirPhi(speed, wishspeed, ft);

	const float vel_yaw = atan2f(velocity.y, velocity.x);
	const float view_yaw = DEG2RAD(cmd->viewangles.y);

	const float yaw_delta = AngleDifference(cmd->viewangles.y, RAD2DEG(vel_yaw));

	const float sign = (yaw_delta >= 0.f) ? 1.f : -1.f;

	SteerMoves(cmd, vel_yaw, view_yaw, phi, sign, magnitude);
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

float CKreedz::ComputeOptimalPhi(const float& speed_2d, const float& wishspeed_budget, const float& accelspeed) const
{
	float cos_phi = (wishspeed_budget - accelspeed) / speed_2d;
	cos_phi = std::clamp(cos_phi, -1.0f, 1.0f);
	return acosf(cos_phi);
}

float CKreedz::GetAirAccelerate() const
{
	if (pmove && pmove->movevars)
		return pmove->movevars->airaccelerate;

	return 10.f;
}

float CKreedz::GetPlayerFriction() const
{
	if (pmove)
		return pmove->friction;

	return 1.f;
}

float CKreedz::GetEffectiveMaxSpeed() const
{
	if (!pmove)
		return MAX_MOVE_SPEED;

	float max_speed = pmove->maxspeed;

	if (pmove->clientmaxspeed > 0.f)
		max_speed = (std::min)(max_speed, pmove->clientmaxspeed);

	return (std::max)(max_speed, 0.f);
}

Vector CKreedz::GetPMVelocity() const
{
	if (pmove)
		return pmove->velocity;

	return g_Local->m_vecVelocity;
}

float CKreedz::ComputeGoldSrcAirPhi(const float& speed, const float& wishspeed, const float& frametime) const
{
	if (speed <= 0.f || wishspeed <= 0.f || frametime <= 0.f)
		return 0.f;

	const float air_accelerate = GetAirAccelerate();
	const float player_friction = GetPlayerFriction();

	if (air_accelerate <= 0.f || player_friction <= 0.f)
		return 0.f;

	const float capped_wishspeed = (std::min)(wishspeed, 30.f);
	const float raw_accel = air_accelerate * wishspeed * frametime * player_friction;

	const float target_current_speed = (std::max)(0.f, capped_wishspeed - raw_accel);

	const float cos_phi = std::clamp(target_current_speed / speed, 0.f, 1.f);

	return acosf(cos_phi);
}

float CKreedz::SelectIdealStrafeSide(const usercmd_s* cmd, const float& velocity_yaw, const float& phi) const
{
	if (!cmd)
		return m_flIdealStrafeSide;

	const float view_yaw = DEG2RAD(cmd->viewangles.y);
	float desired_yaw = view_yaw;

	if (fabsf(cmd->forwardmove) > 0.001f || fabsf(cmd->sidemove) > 0.001f)
		desired_yaw += atan2f(cmd->sidemove, cmd->forwardmove);

	const float left_error = fabsf(NormalizeRadians(velocity_yaw + phi - desired_yaw));
	const float right_error = fabsf(NormalizeRadians(velocity_yaw - phi - desired_yaw));

	return left_error <= right_error ? 1.f : -1.f;
}

void CKreedz::SteerMoves(usercmd_s* cmd, const float& vel_yaw, const float& view_yaw, const float& phi, const float& sign, const float& magnitude)
{
	const float target_yaw = vel_yaw + sign * phi;
	const float psi = target_yaw - view_yaw;

	cmd->forwardmove = magnitude * cosf(psi);
	cmd->sidemove = magnitude * sinf(psi);
}

void CKreedz::DopplerStrafe(usercmd_s* cmd)
{
	if (g_Local->m_bIsOnLadder || g_Local->m_flGroundAngle > 0.f)
		return;

	Vector vecVelocity = GetPMVelocity();

	float speed = vecVelocity.ToVec2D().Length();

	if (cvars::kreedz.doppler_limit_velocity && speed > static_cast<float>(cvars::kreedz.doppler_max_speed))
		return;

	if (speed < 1.f)
		speed = 1.f;

	const float ft = GetMoveFrameTime(cmd);
	const float vel_yaw = atan2f(vecVelocity.y, vecVelocity.x);
	const float view_yaw = DEG2RAD(cmd->viewangles.y);

	const int mode = std::clamp(cvars::kreedz.doppler_mode, 0, 2);
	const float magnitude = std::clamp(cvars::kreedz.doppler_move, 200.f, 400.f);

	if (!g_Local->m_bIsOnGround)
	{
		m_iDopplerAirTicks++;

		if (m_iDopplerAirTicks <= std::clamp(cvars::kreedz.doppler_min_air_ticks, 0, 10))
			return;

		if (mode == 1)
			return;

		const float wishspeed = (std::min)(magnitude, GetEffectiveMaxSpeed());

		if (wishspeed <= 0.f || ft <= 0.f)
			return;

		const float phi_air = ComputeGoldSrcAirPhi(speed, wishspeed, ft);
		const float sign = SelectIdealStrafeSide(cmd, vel_yaw, phi_air);

		m_flDopplerSideSign = sign;

		SteerMoves(cmd, vel_yaw, view_yaw, phi_air, sign, magnitude);
		return;
	}

	m_iDopplerAirTicks = 0;

	if (mode == 0)
		return;

	const int cadence = (std::max)(1, cvars::kreedz.doppler_pump_ticks);
	const int cycle = cadence + 1;
	const bool contact = (m_iDopplerCyclePos % cycle) < cadence;

	if ((m_iDopplerCyclePos % cycle) == 0)
		m_flDopplerSideSign = -m_flDopplerSideSign;

	m_iDopplerCyclePos++;

	if (!contact)
	{
		cmd->buttons |= IN_DUCK;

		if (cvars::kreedz.doppler_auto_jump)
			cmd->buttons |= IN_JUMP;

		cmd->sidemove = 0.f;
		cmd->forwardmove = 0.f;
		return;
	}

	if (speed < 5.f)
	{
		cmd->sidemove = 0.f;
		cmd->forwardmove = magnitude;
		return;
	}

	float maxspeed = (pmove) ? pmove->maxspeed : MAX_MOVE_SPEED;

	if (maxspeed < 1.f)
		maxspeed = MAX_MOVE_SPEED;

	const float accelerate = (pmove && pmove->movevars && pmove->movevars->accelerate > 0.f) ? pmove->movevars->accelerate : 10.f;

	const float accel_ground = (std::min)(accelerate * ft * maxspeed, maxspeed);
	const float phi_ground = ComputeOptimalPhi(speed, maxspeed, accel_ground);

	SteerMoves(cmd, vel_yaw, view_yaw, phi_ground, m_flDopplerSideSign, magnitude);
}

void CKreedz::LadderStrafe(usercmd_s* cmd)
{
	if (!g_Local->m_bIsOnLadder)
		return;

	Vector vecVelocity = g_Local->m_vecVelocity;

	float speed = vecVelocity.ToVec2D().Length();

	if (speed >= static_cast<float>(cvars::kreedz.ladder_max_speed))
	{
		cmd->buttons |= IN_JUMP;
		return;
	}

	if (speed < 5.f)
	{
		cmd->sidemove = 0.f;
		cmd->forwardmove = std::clamp(cvars::kreedz.ladder_move, 200.f, 400.f);
		return;
	}

	const float ft = std::clamp(g_Local->m_flFrameTime, 0.001f, 0.255f);

	float budget = (pmove) ? pmove->maxspeed : MAX_MOVE_SPEED;

	if (budget < 1.f)
		budget = MAX_MOVE_SPEED;

	const float accel = (std::min)(10.f * budget * ft, budget);
	const float phi = ComputeOptimalPhi(speed, budget, accel);

	const float vel_yaw = atan2f(vecVelocity.y, vecVelocity.x);
	const float view_yaw = DEG2RAD(cmd->viewangles.y);

	SteerMoves(cmd, vel_yaw, view_yaw, phi, 1.f, std::clamp(cvars::kreedz.ladder_move, 200.f, 400.f));
}

void CKreedz::IdealStrafe(usercmd_s* cmd)
{
	if (!cmd)
		return;

	const bool on_ground = pmove ? pmove->onground != -1 : g_Local->m_bIsOnGround;
	const bool on_ladder = pmove ? pmove->movetype != MOVETYPE_WALK : g_Local->m_bIsOnLadder;
	const bool in_water = pmove ? pmove->waterlevel > 1 : g_Local->m_bIsInWater;

	if (on_ground || on_ladder || in_water)
	{
		m_flIdealStrafeSide = 1.f;
		return;
	}

	Vector velocity = GetPMVelocity();

	float speed = velocity.ToVec2D().Length();

	if (speed < 15.f)
		return;

	const float ft = GetMoveFrameTime(cmd);

	const float wishspeed = GetEffectiveMaxSpeed();

	if (wishspeed <= 0.f || ft <= 0.f)
		return;

	const float phi = ComputeGoldSrcAirPhi(speed, wishspeed, ft);

	const float vel_yaw = atan2f(velocity.y, velocity.x);
	const float view_yaw = DEG2RAD(cmd->viewangles.y);

	m_flIdealStrafeSide = SelectIdealStrafeSide(cmd, vel_yaw, phi);

	SteerMoves(cmd, vel_yaw, view_yaw, phi, m_flIdealStrafeSide, wishspeed);
}
