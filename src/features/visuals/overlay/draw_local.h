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
};

extern ToggleStatusData g_ToggleStatus;
extern std::deque<BulletTracerData> g_BulletTracers;