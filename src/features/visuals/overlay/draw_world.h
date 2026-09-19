struct GrenadeTrailData
{
	std::deque<Vector> points;
	Vector origin;
	float detonate_time;
	int type;
};

class CDrawWorld
{
public:
	void Overlay();

private:
	void GrenadePreview();
	void SimulateTrajectory(const Vector& start, const Vector& velocity, std::deque<Vector>& points, Vector& impact);

	std::deque<GrenadeTrailData> m_grenades;
};