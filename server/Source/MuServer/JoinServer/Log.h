#pragma once

enum eLogType
{
	LOG_GENERAL = 0,
	LOG_ACCOUNT,
	LOG_QUERY,
	MAX_LOG
};

struct LOG_INFO
{
	bool Active;
	char Directory[256];
	int Day;
	int Month;
	int Year;
	char Filename[256];
	FILE* File;
};

class CLog
{
public:

	CLog();

	~CLog();

	void AddLog(bool active, const char* directory);

	void Output(eLogType type, const char* text, ...);

private:

	LOG_INFO m_LogInfo[MAX_LOG];

	int m_count;
};

extern CLog gLog;

