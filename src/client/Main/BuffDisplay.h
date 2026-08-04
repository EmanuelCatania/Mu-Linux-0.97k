#pragma once

#include "Protocol.h"

#define MAX_BUFF_DISPLAY 16

struct BUFF_DISPLAY_ENTRY
{
	BYTE Effect;
	DWORD Count;
	WORD Value[4];
	DWORD ReceivedTick;
	int PosX;
	int PosY;
	bool Active;
};

class CBuffDisplay
{
public:

	CBuffDisplay();

	virtual ~CBuffDisplay();

	void Init();

	void Reset();

	void SetArrowHud(int ScreenWidth, int Left, bool Visible);

	void GCRecv(PMSG_BUFF_LIST_RECV* lpMsg, int Size);

	void Render();

	void RenderTooltip();

	// Renders only the native atlas icon for a party buff.
	void RenderPartyEntry(BYTE Effect, int PosX, int PosY, int SlotSize = 16);

private:

	static void __cdecl RenderBeforeMainWindows();

	static bool IsDisplayableEffect(BYTE Effect);

	static int GetSkill(BYTE Effect);

	static const char* GetEffectText(BYTE Effect, const WORD* Value, char* Text, int Size);

	static void GetSkillName(int Skill, char* Name, int Size);

	static void FormatShortTime(DWORD Seconds, char* Text, int Size);

	static void FormatLongTime(DWORD Seconds, char* Text, int Size);

	static const char* GetDurationLabel();

	int GetRightMargin() const;

	DWORD GetRemaining(const BUFF_DISPLAY_ENTRY* Entry, DWORD Tick) const;

	void RenderEntry(BUFF_DISPLAY_ENTRY* Entry, DWORD Remaining);

	void RenderTooltipEntry(const BUFF_DISPLAY_ENTRY* Entry, DWORD Remaining);

	BUFF_DISPLAY_ENTRY* GetEntryAtMouse();

private:

	BUFF_DISPLAY_ENTRY m_Entry[MAX_BUFF_DISPLAY];

	int m_Count;

	bool m_Initialized;

	int m_ArrowHudScreenWidth;

	int m_ArrowHudLeft;

	bool m_ArrowHudVisible;

	HFONT m_TimerFont;

};

extern CBuffDisplay gBuffDisplay;
