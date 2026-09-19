struct ToggleStatusData
{
	std::string name;
	bool status;
	double timestamp;
};

struct BulletTracerData
{
	Vector start;
	Vector end;
	double time;
};

struct HitMarkerData
{
	bool headshot;
	float timestamp;
};

struct DamageLogData
{
	std::string name;
	int damage;
	int health;
	bool headshot;
	double timestamp;
};

void RegisterHitEvent(int index, int damage, bool headshot);

extern ToggleStatusData g_ToggleStatus;
extern HitMarkerData g_HitMarker;
extern std::deque<BulletTracerData> g_BulletTracers;
extern std::deque<DamageLogData> g_DamageLog;

class CDrawLocal
{
public:
	CDrawLocal();

	void Overlay();

private:
	std::unique_ptr<CDefaultCrosshair> m_pDefaultCrosshair;

	void SniperCrosshair();
	void AimFOV();
	void RecoilPoint();
	void SpreadPoint();
	void SpreadCircle();
	void ToggleStatus();
	void Watermark();
	void CustomCrosshair();
	void FunctionStatus();
	void HotkeyList();
	void Statistics();
	void BulletTracers();
	void Indicators();
	void HitMarker();
	void DamageLog();
	void SpectatorList();
};