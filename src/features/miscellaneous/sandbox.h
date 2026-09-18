class CSandbox
{
public:
	CSandbox();
	~CSandbox();

	void SetupCvar(std::string name, std::string value);
	std::string GetCvar(std::string name);

	// Returns clean value the server expects to see (or empty = answer honestly).
	// Lifetime: returns pointer to internal static buffer, valid until next call.
	static const char* GetSpoofedValue(cvar_t* cvar);
	static void RememberServerValue(const char* name, const char* value);

private:
	std::map<std::string, std::string> cvars;
};

extern std::unique_ptr<CSandbox> g_pSandbox;