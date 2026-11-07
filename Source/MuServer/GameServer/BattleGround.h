#pragma once

class CBattleGround
{
public:

	CBattleGround();

	~CBattleGround();

	void SetGroundRect(int map, int x, int y, int xl, int yl);

	void SetBattleType(int type);

	void Enable(bool enable);

	bool GetUsed();

	void SetTeamMaster(int n, const char* szId);

	void SetTeamName(int n, const char* szName);

	const char* GetTeamName(int n);

	void SetTimer(int g_time);

	int GetTimer();

	int GetScore(int team);

	void SetScore(int team, int score);

public:

	char m_szTeamName1[GUILD_NAME_BUFFER];

	char m_szTeamName2[GUILD_NAME_BUFFER];

	int m_Score1;

	int m_Score2;

	char m_szID1[CHARACTER_NAME_BUFFER];

	char m_szID2[CHARACTER_NAME_BUFFER];

	RECT m_rect;

	bool m_Set;

	int m_Type;

	bool m_Used;

	int m_MapNumber;

	int m_nTimer;

	DWORD m_dwCurTimer;
};
