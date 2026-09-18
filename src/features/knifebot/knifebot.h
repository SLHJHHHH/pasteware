class CKnifeBot
{
public:
	CKnifeBot();
	~CKnifeBot();

	void					Run(usercmd_s* cmd);

private:
	void					CorrectPhysentSolid(const int& nPlayerID);

	void					DebugPrint(const char* pszStage, const float& flFOV, const float& flDistance, const int& iPlayer = -1, const int& iTraceIndex = -1);
};

extern std::unique_ptr<CKnifeBot> g_pKnifeBot;