#pragma once

#include "Protocol.h"

#define PARTY_DISPLAY_MAX_MEMBERS 5
#define PARTY_DISPLAY_MAX_EFFECTS 4

struct PARTY_DISPLAY_EFFECT_ENTRY
{
	BYTE Effect;
};

struct PARTY_DISPLAY_MEMBER_ENTRY
{
	bool Active;
	BYTE Number;
	WORD Level;
	BYTE Class;
	BYTE ChangeUp;
	BYTE EffectCount;
	PARTY_DISPLAY_EFFECT_ENTRY Effect[PARTY_DISPLAY_MAX_EFFECTS];
};

struct PARTY_DISPLAY_RECT
{
	int X;
	int Y;
	int Width;
	int Height;

	bool Contains(int PosX, int PosY) const
	{
		return (PosX >= this->X && PosX < this->X + this->Width && PosY >= this->Y && PosY < this->Y + this->Height);
	}
};

struct PARTY_DISPLAY_ROW_LAYOUT
{
	bool Active;
	BYTE PartySlot;
	PARTY_DISPLAY_RECT RowRect;
	PARTY_DISPLAY_RECT ClassRect;
	PARTY_DISPLAY_RECT NameRect;
	PARTY_DISPLAY_RECT LocationRect;
	PARTY_DISPLAY_RECT LifeBarRect;
	PARTY_DISPLAY_RECT LifeTextRect;
	PARTY_DISPLAY_RECT BuffRect[4];
	PARTY_DISPLAY_RECT KickRect;
};

class CPartyDisplay
{
public:
	CPartyDisplay();

	virtual ~CPartyDisplay();

	void Init();

	// Replaces only the call-site renderer used by the expanded (P) party
	// window.  The compact HUD still dispatches to the original main.exe
	// renderer; open-mode interaction is handled by the shared row layout.
	static void __cdecl RenderNativeHook(int X, int Y);
	void RenderNative(int X, int Y);

	void Reset();

	void GCRecv(PMSG_PARTY_DISPLAY_RECV* lpMsg, int Size);

	void UpdateMouse();

private:
	void RenderNativeFrame(int X, int Y);
	void BuildLayout(int X, int Y);
	void RenderNativeMember(const PARTY_t* Native, PARTY_DISPLAY_MEMBER_ENTRY* Member, PARTY_DISPLAY_ROW_LAYOUT* Layout);
	void SendPartyKick(BYTE Number);

	PARTY_DISPLAY_MEMBER_ENTRY* GetMember(BYTE Number);

	static const char* GetClassName(BYTE Class, BYTE ChangeUp);

	PARTY_DISPLAY_MEMBER_ENTRY m_Member[PARTY_DISPLAY_MAX_MEMBERS];

	bool m_Initialized;

	HFONT m_PartyFont;

	PARTY_DISPLAY_ROW_LAYOUT m_Layout[PARTY_DISPLAY_MAX_MEMBERS];

	int m_LayoutX;

	int m_LayoutY;

	BYTE m_LayoutCount;
};

extern CPartyDisplay gPartyDisplay;
