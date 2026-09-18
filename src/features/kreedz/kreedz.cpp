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
	// NOTE (crash rule, build 4554 + Protector): EV_PlayerTrace is only safe in
	// the render/draw context. Run() is called from CL_CreateMove and must not
	// trace, so all wall checks are done here once per frame and cached.
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

	// Only world geometry: landing on a player has nothing to do with the bug.
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

void CKreedz::BunnyHop(usercmd_s* cmd)
{
	if (!cvars::kreedz.bunnyhop)
		return;

	// Jump on the exact frame we touch the ground so no friction is ever
	// applied to the horizontal velocity.
	if (g_Local->m_bIsOnGround)
		cmd->buttons |= IN_JUMP;
}

void CKreedz::GroundStrafe(usercmd_s* cmd)
{
	if (!cvars::kreedz.groundstrafe)
		return;

	// air is DeGen/Strafe's job - groundstrafe is strictly a ground feature
	if (!g_Local->m_bIsOnGround)
		return;

	Vector vecVelocity = g_Local->m_vecVelocity;

	const float flSpeed = vecVelocity.ToVec2D().Length();

	// too slow - nothing to tilt around, plain walking is faster
	if (flSpeed < 20.f)
		return;

	const float flMaxspeed = (pmove) ? pmove->maxspeed : MAX_MOVE_SPEED;

	// NOTE: ground wishdir - accel budget model. The server grants
	// accel*dt*maxspeed (= 13.75 on the ground at 100fps) along the wishdir.
	// Pay the friction loss (4*dt*|v|) along v, spend the REST on the
	// perpendicular. gain (0-1) moves budget from hold to turn. Perpendicular
	// goes FULL raw: alternated flank (bFlip), view only picks the starting
	// flank so aiming into the turn takes the short way.
	//   Hard engine facts (verified by sim, do not "fix"):
	//   - addspeed is ALSO capped by maxspeed - dot(v,wh): the tilt itself
	//     eats the grant, so only ~7 u/s per tick survives at 250 u/s and
	//     equilibrium is ~270-300, NOT 400+;
	//   - the leftover grind 250 -> ~290 takes ~10-15 s of straight running,
	//     a bhop landing at 400+ holds much longer before decaying;
	//   - turns ALWAYS cost speed on the ground (friction 4x). Engine
	//     physics, not a bug in this function.
	const float flGain = std::clamp(cvars::kreedz.groundstrafe_gain, 0.f, 1.f);

	// Goal heading: the VIEW yaw. kept as an angle (not velocity yaw), so it
	// survives the low-speed phases where atan2 jitters.
	// NOTE on the steering: flip the flank ONLY when the velocity settles
	// onto the view (crosses the deadzone edge INWARD). While outside the
	// zone the side is CONTINUOUS - no every-tick negation (that kills the
	// heading, verified by sim), and no forcing to sign(delta) either (that
	// zeroes the perpendicular exactly at theta = 0 and the yaw sticks at a
	// ~30 deg offset). In-zone behaviour: alternate every tick for the
	// classic zigzag cancel. Outside: hold the flank and let the circle pull
	// the velocity around. The view sets the initial flank at zone entry,
	// so aiming into the turn takes the short way around.
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

	// accel-component basis (same GoldSrc expansion as DeGen):
	//   fm = A_along*cos(theta) + A_side*sin(theta)
	//   sm = A_along*sin(theta) - A_side*cos(theta)
	// The accel magnitude is already exactly what the server grants
	// (<= 30), so the cmd only carries the DIRECTION at full maxspeed
	// amplitude - PM_Accelerate scales it back to the 30-cap itself.
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

	// both keys of an axis held - no stable direction to boost
	if ((cmd->buttons & IN_MOVELEFT && cmd->buttons & IN_MOVERIGHT) || (cmd->buttons & IN_FORWARD && cmd->buttons & IN_BACK))
		return;

	// oxware/Tkz: fastwalk turns into a slowdown above ~277 u/s
	if (g_Local->m_flVelocity > FASTRUN_MAX_SPEED)
		return;

	// NOTE: oxware/Tkz formula ported to the QAngle yaw of the user command.
	// |forward| = 267.88, |side| = 135.06 rotated so the wishdir points along
	// the velocity direction (+90 if we steer left of it).
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

	// falling or not above a ledge (edge distance is cached per frame in CBaseLocal)
	if (g_Local->m_flFallVelocity < 0.f || g_Local->m_flEdgeDistance <= cvars::kreedz.auto_jof_min_distance)
		return;

	// jump when the distance to the ledge will be crossed this tick
	if (g_Local->m_flEdgeDistance <= g_Local->m_flVelocity * g_Local->m_flFrameTime)
		cmd->buttons |= IN_JUMP;
}

void CKreedz::JumpBug(usercmd_s* cmd)
{
	if (!cvars::kreedz.jumpbug)
		return;

	if (g_Local->m_bIsOnGround || g_Local->m_flFallVelocity < 150.f)
		return;

	// m_flHeightGround is maintained every frame by CBaseLocal::Update, so no
	// extra trace is needed here (traces from CL_CreateMove are forbidden).
	if (g_Local->m_flHeightGround > JUMPBUG_DISTANCE)
		return;

	// Duck + jump on the landing frame: the ducked hull touches first so the
	// engine never applies the fall damage.
	cmd->buttons |= IN_JUMP | IN_DUCK;
}

void CKreedz::EdgeBug(usercmd_s* cmd)
{
	if (!cvars::kreedz.edgebug)
		return;

	if (g_Local->m_bIsOnGround || g_Local->m_flFallVelocity < 1.f)
		return;

	// Falling over a ledge: the normal hull already sees a long drop while the
	// ducked hull is still almost touching the ground, so ducking catches the
	// edge instead of letting us fall.
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

	// Wall check is cached once per frame by Frame() (render context).
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

	// NOTE: same failure the old code had, documented so nobody reverts it:
	// the old Strafe snapped the VIEW to velYaw +- 90 and held sidemove 400.
	// Through MakeAngle that yields a wishdir COLLINEAR with the velocity,
	// so addspeed = 30 - |v| <= 0 every tick - zero gain by design, and the
	// per-tick view overwrite kills the mouse (the "spinning in place" bug).
	// This version does the DeGen thing instead: solve fm/sm so the wishdir
	// IS the perpendicular every tick (maximal add, no sync error), and move
	// the VIEW toward the velocity yaw (rage = snap, legit = smoothed) so the
	// heading converges instead of orbiting. View motion is capped per tick
	// (no teleport), mouse stays alive - you can still steer mid-flight.
	// theta ~ 0 flips the solved side each tick by design (self-centering).
	const float flVelYaw = RAD2DEG(atan2(vecVelocity.y, vecVelocity.x));
	const float flDeltaDeg = AngleDifference(cmd->viewangles.y, flVelYaw);
	const float flDelta = DEG2RAD(flDeltaDeg);

	const float flSide = (flDeltaDeg >= 0.f) ? 1.f : -1.f;

	const float flPower = 400.f;

	const float flSin = sin(flDelta);
	const float flCos = cos(flDelta);

	// perpendicular part flips with the side (gain), no tilt (pure fixpoint)
	cmd->forwardmove = flPower * flSide * flSin;
	cmd->sidemove = flPower * -flSide * flCos;

	// bring the view onto the velocity: rage snaps, legit eases.
	// the smoothing reuses the legit strafe speed slider (1-100%).
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

	// MakeAngle re-expresses the SOLVED fm/sm for the new view (it rotates
	// the move, it does not reset it) - the perpendicular survives, the view
	// lands on the velocity. Order matters: fm/sm first, angle second.
	Game::MakeAngle(QAngles, cmd);
}

// ============================================================================
// DeGen - air wishdir generator (original feature, not a port).
//
// GoldSrc PM_AirAccelerate asymmetry:
//   currentspeed = DotProduct(velocity, wishdir);
//   addspeed     = 30 - currentspeed;               // hard 30 cap (wishspd)
//   accelspeed   = accel * frametime * wishspeed;   // wishspeed NOT capped by 30
//   velocity    += min(accelspeed, addspeed) * wishdir;
//
// With wishdir EXACTLY perpendicular to velocity (DotProduct = 0) the add is
// maximal every single tick:  |v'|^2 = |v|^2 + min(accel*ft*min(|fm,sm|,250), 30)^2.
// Any desync d turns it into |v'|^2 = |v|^2 + 900 - (x*sin(d))^2 - real sync
// errors (5..30 deg mid-turn) give addspeed <= 0 and the whole tick is wasted.
// Humans approximate the fixed point by rotating the view; this feature
// SOLVES fm/sm so the wishdir IS the fixed point every tick:
//   theta = view_yaw - velocity_yaw   (cmd basis angle)
//   side  = sign(theta)               (steer toward the crosshair)
//   fm = A*( side*sin(theta) - t*cos(theta) )
//   sm = A*( -side*cos(theta) - t*sin(theta) ),  A = degen_power, t = degen_tilt
// Identity proof (GoldSrc basis: f=(cos p, sin p) forward, r=(sin p, -cos p)
// right, i.e. right = forward x up; orthonormal, |W|=A*sqrt(1+t^2)):
//   W = fm*f + sm*r = side*A*(-sin phi, cos phi) - t*A*v_hat (phi = velocity yaw)
//   => W is exactly perpendicular to v (t=0); DotProduct(v, W) = -t*A*|v|
//   => addspeed = 30 + t*A*|v|  (t = 0: the exact fixed point, addspeed = 30).
// SIDE STEERING: perpendicular acceleration also ROTATES the velocity by
// ~atan(30/|v|) per tick. A fixed side orbits you around a circle of radius
// |v|^2/30 (~2k..12k u) - that is the "spinning in place" failure mode.
// Taking the perpendicular on the side of the view makes the velocity rotate
// TOWARD the crosshair each tick: aim where you fly and the flight is
// straight (micro-zigzag around the view yaw once converged, same mean
// direction), aim sideways to turn - gain stays maximal in both cases.
// theta ~ 0 makes side flip per tick by design: that is the self-centering.
// No MakeAngle, no view sync: works while aiming anywhere, no angle snap.
//
// Server side: wishspeed is capped at sv_maxspeed (250) for the accel call
// and the add is capped at 30 (hardcoded), so gain per tick is
// min(airaccel*ft*250, 30): default sv_airaccelerate 10 @100fps -> 25,
// KZ servers (airaccel 100) -> 30 every tick, forever. degen_power only has
// to exceed 250; wishspeed is capped server-side so bigger is free (but some
// AC flag |fm/sm| far above 400 - default 400 stays under the sentinel).
// degen_tilt != 0 is strictly worse on default configs (t-term in |v'|)
// and exists for exotic airaccel setups only.
// ============================================================================
void CKreedz::DeGen(usercmd_s* cmd)
{
	if (cvars::kreedz.degen_key.keynum && !m_bDeGenKey)
		return;

	if (g_Local->m_bIsOnGround)
		return;

	Vector vecVelocity = g_Local->m_vecVelocity;

	const float flSpeed = vecVelocity.ToVec2D().Length();

	// below walking speed there is no stable perpendicular to accelerate along
	if (flSpeed < 20.f)
		return;

	const float flVelYaw = RAD2DEG(atan2(vecVelocity.y, vecVelocity.x));
	const float flDeltaDeg = AngleDifference(cmd->viewangles.y, flVelYaw);
	const float flDelta = DEG2RAD(flDeltaDeg);

	// steer toward the crosshair: the perpendicular is taken on the side of
	// the view, so the velocity converges to the view yaw instead of orbiting
	const float flSide = (flDeltaDeg >= 0.f) ? 1.f : -1.f;

	const float flPower = std::clamp(cvars::kreedz.degen_power, 1.f, 1000.f);
	const float flTilt = std::clamp(cvars::kreedz.degen_tilt, -0.9f, 0.9f);

	const float flSin = sin(flDelta);
	const float flCos = cos(flDelta);

	// perpendicular part flips with the side, tilt part (along v) does not
	cmd->forwardmove = flPower * (flSide * flSin - flTilt * flCos);
	cmd->sidemove = flPower * (-flSide * flCos - flTilt * flSin);
}
