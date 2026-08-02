#include "stdafx.h"
#include "BuffDisplay.h"
#include "Font.h"
#include "Language.h"
#include "Offsets.h"

static const int BUFF_DISPLAY_SLOT_SIZE = 22;
static const int BUFF_DISPLAY_SLOT_GAP = 0;
static const int BUFF_DISPLAY_DEFAULT_RIGHT = 112;
static const int BUFF_DISPLAY_TOP = 8;
static const int BUFF_DISPLAY_ROW_GAP = 4;
static const int BUFF_DISPLAY_ATLAS_SIZE = 256;
static const int BUFF_DISPLAY_ATLAS_CELL_WIDTH = 32;
static const int BUFF_DISPLAY_ATLAS_CELL_HEIGHT = 36;
static const int BUFF_DISPLAY_ATLAS_COLUMNS = 8;
static const int BUFF_DISPLAY_ICON_LEFT = 5;
static const int BUFF_DISPLAY_ICON_TOP = 4;
static const int BUFF_DISPLAY_ICON_WIDTH = 16;
static const int BUFF_DISPLAY_ICON_HEIGHT = 19;
static const int BUFF_DISPLAY_ICON_SOURCE_WIDTH = 22;
static const int BUFF_DISPLAY_ICON_SOURCE_HEIGHT = 25;
static const int BUFF_DISPLAY_TIMER_TOP = 11;
static const int BUFF_DISPLAY_TEXTURE = 0x12A;
static const DWORD BUFF_DISPLAY_PERMANENT = 0xFFFFFFFF;

CBuffDisplay gBuffDisplay;

CBuffDisplay::CBuffDisplay()
{
	memset(this->m_Entry, 0, sizeof(this->m_Entry));
	this->m_Count = 0;
	this->m_Initialized = false;
	this->m_ArrowHudScreenWidth = 0;
	this->m_ArrowHudLeft = 0;
	this->m_ArrowHudVisible = false;
	this->m_TimerFont = NULL;
}

CBuffDisplay::~CBuffDisplay()
{
	if (this->m_TimerFont != NULL && this->m_TimerFont != g_hFont && this->m_TimerFont != g_hFontBold)
	{
		DeleteObject((HGDIOBJ)this->m_TimerFont);
	}
}

void CBuffDisplay::Init()
{
	if (this->m_Initialized != false)
	{
		return;
	}

	int TimerFontHeight = FontHeight - 3;

	if (TimerFontHeight < 8)
	{
		TimerFontHeight = 8;
	}

	this->m_TimerFont = CreateFont(
		TimerFontHeight,
		gFont.Width,
		0,
		0,
		FW_NORMAL,
		gFont.Italic,
		gFont.UnderLine,
		gFont.StrikeOut,
		gFont.Charset,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		gFont.Quality,
		DEFAULT_PITCH,
		gFont.MyFontFaceName);

	if (this->m_TimerFont == NULL)
	{
		this->m_TimerFont = g_hFont;
	}

	SetCompleteHook(
		0xE8,
		BuffDisplayRenderCall,
		&CBuffDisplay::RenderBeforeMainWindows);

	this->m_Initialized = true;
}

void CBuffDisplay::Reset()
{
	memset(this->m_Entry, 0, sizeof(this->m_Entry));
	this->m_Count = 0;
	this->m_ArrowHudVisible = false;
}

void CBuffDisplay::SetArrowHud(int ScreenWidth, int Left, bool Visible)
{
	if (Visible == false || ScreenWidth <= 0 || Left < 0 || Left > ScreenWidth)
	{
		this->m_ArrowHudScreenWidth = 0;
		this->m_ArrowHudLeft = 0;
		this->m_ArrowHudVisible = false;
		return;
	}

	this->m_ArrowHudScreenWidth = ScreenWidth;
	this->m_ArrowHudLeft = Left;
	this->m_ArrowHudVisible = true;
}

void CBuffDisplay::GCRecv(PMSG_BUFF_LIST_RECV* lpMsg, int Size)
{
	if (lpMsg == 0 || Size < (int)sizeof(PMSG_BUFF_LIST_RECV))
	{
		return;
	}

	if (lpMsg->header.type != 0xC2 || lpMsg->header.head != 0xF3 || lpMsg->header.subh != 0xE9)
	{
		return;
	}

	int PacketSize = MAKEWORD(lpMsg->header.size[1], lpMsg->header.size[0]);
	int ExpectedSize = sizeof(PMSG_BUFF_LIST_RECV) + (lpMsg->count * sizeof(PMSG_BUFF_INFO_RECV));

	if (PacketSize != Size || ExpectedSize != Size || lpMsg->count > MAX_BUFF_DISPLAY)
	{
		return;
	}

	BUFF_DISPLAY_ENTRY Temp[MAX_BUFF_DISPLAY] = {};
	bool Seen[MAX_BUFF_DISPLAY] = {};
	PMSG_BUFF_INFO_RECV* Info = (PMSG_BUFF_INFO_RECV*)((BYTE*)lpMsg + sizeof(PMSG_BUFF_LIST_RECV));
	int Count = 0;
	DWORD Tick = GetTickCount();

	for (int n = 0; n < lpMsg->count; n++)
	{
		if (Info[n].effect >= MAX_BUFF_DISPLAY || IsDisplayableEffect(Info[n].effect) == false)
		{
			continue;
		}

		if (Seen[Info[n].effect] != false)
		{
			return;
		}

		Seen[Info[n].effect] = true;
		Temp[Count].Effect = Info[n].effect;
		Temp[Count].Count = Info[n].count;
		memcpy(Temp[Count].Value, Info[n].value, sizeof(Temp[Count].Value));
		Temp[Count].ReceivedTick = Tick;
		Temp[Count].Active = true;
		Count++;
	}

	for (int n = 0; n < Count; n++)
	{
		for (int k = n + 1; k < Count; k++)
		{
			if (GetSkill(Temp[k].Effect) < GetSkill(Temp[n].Effect))
			{
				BUFF_DISPLAY_ENTRY Swap = Temp[n];
				Temp[n] = Temp[k];
				Temp[k] = Swap;
			}
		}
	}

	memset(this->m_Entry, 0, sizeof(this->m_Entry));
	memcpy(this->m_Entry, Temp, sizeof(Temp));
	this->m_Count = Count;
}

void __cdecl CBuffDisplay::RenderBeforeMainWindows()
{
	RenderGuildWarInfo();
	gBuffDisplay.Render();
}

void CBuffDisplay::Render()
{
	if (this->m_Initialized == false)
	{
		return;
	}

	if (SceneFlag != MAIN_SCENE || g_bGameServerConnected == FALSE)
	{
		this->Reset();
		return;
	}

	DWORD Tick = GetTickCount();
	int RightMargin = CBuffDisplay::GetRightMargin();
	int Visible = 0;
	int DisplayIndex = 0;

	for (int n = 0; n < this->m_Count; n++)
	{
		if (this->m_Entry[n].Active != false && this->GetRemaining(&this->m_Entry[n], Tick) != 0)
		{
			Visible++;
		}
	}

	for (int n = 0; n < this->m_Count; n++)
	{
		BUFF_DISPLAY_ENTRY* Entry = &this->m_Entry[n];

		if (Entry->Active == false || this->GetRemaining(Entry, Tick) == 0)
		{
			continue;
		}

		int Row = DisplayIndex / 8;
		int RowCount = min(Visible - (Row * 8), 8);
		int Width = (RowCount * BUFF_DISPLAY_SLOT_SIZE) + ((RowCount - 1) * BUFF_DISPLAY_SLOT_GAP);
		int Column = DisplayIndex % 8;

		Entry->PosX = 640 - RightMargin - Width + (Column * (BUFF_DISPLAY_SLOT_SIZE + BUFF_DISPLAY_SLOT_GAP));
		Entry->PosY = BUFF_DISPLAY_TOP + (Row * (BUFF_DISPLAY_SLOT_SIZE + BUFF_DISPLAY_ROW_GAP));

		this->RenderEntry(Entry, this->GetRemaining(Entry, Tick));
		DisplayIndex++;
	}
}

void CBuffDisplay::RenderTooltip()
{
	if (this->m_Initialized == false || SceneFlag != MAIN_SCENE || g_bGameServerConnected == FALSE)
	{
		return;
	}

	BUFF_DISPLAY_ENTRY* Entry = this->GetEntryAtMouse();

	if (Entry == 0)
	{
		return;
	}

	DWORD Remaining = this->GetRemaining(Entry, GetTickCount());

	if (Remaining == 0)
	{
		return;
	}

	this->RenderTooltipEntry(Entry, Remaining);
}

BUFF_DISPLAY_ENTRY* CBuffDisplay::GetEntryAtMouse()
{
	for (int n = 0; n < this->m_Count; n++)
	{
		BUFF_DISPLAY_ENTRY* Entry = &this->m_Entry[n];

		if (Entry->Active != false && IsWorkZone(Entry->PosX, Entry->PosY, BUFF_DISPLAY_SLOT_SIZE, BUFF_DISPLAY_SLOT_SIZE))
		{
			return Entry;
		}
	}

	return 0;
}

DWORD CBuffDisplay::GetRemaining(const BUFF_DISPLAY_ENTRY* Entry, DWORD Tick) const
{
	if (Entry->Count == BUFF_DISPLAY_PERMANENT)
	{
		return BUFF_DISPLAY_PERMANENT;
	}

	DWORD Elapsed = (Tick - Entry->ReceivedTick) / 1000;

	if (Elapsed >= Entry->Count)
	{
		return 0;
	}

	return Entry->Count - Elapsed;
}

void CBuffDisplay::RenderEntry(BUFF_DISPLAY_ENTRY* Entry, DWORD Remaining)
{
	int Cell = this->GetSkill(Entry->Effect) - 1;

	if (Cell >= 0)
	{
		int Column = Cell % BUFF_DISPLAY_ATLAS_COLUMNS;
		int Row = Cell / BUFF_DISPLAY_ATLAS_COLUMNS;
		int SourceX = (Column * BUFF_DISPLAY_ATLAS_CELL_WIDTH) + BUFF_DISPLAY_ICON_LEFT;
		int SourceY = (Row * BUFF_DISPLAY_ATLAS_CELL_HEIGHT) + BUFF_DISPLAY_ICON_TOP;

		glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		EnableAlphaTest(true);
		RenderBitmap(
			BUFF_DISPLAY_TEXTURE,
			(float)Entry->PosX + ((BUFF_DISPLAY_SLOT_SIZE - BUFF_DISPLAY_ICON_WIDTH) / 2.0f),
			(float)Entry->PosY + ((BUFF_DISPLAY_SLOT_SIZE - BUFF_DISPLAY_ICON_HEIGHT) / 2.0f),
			(float)BUFF_DISPLAY_ICON_WIDTH,
			(float)BUFF_DISPLAY_ICON_HEIGHT,
			(SourceX + 0.5f) / BUFF_DISPLAY_ATLAS_SIZE,
			(SourceY + 0.5f) / BUFF_DISPLAY_ATLAS_SIZE,
			(BUFF_DISPLAY_ICON_SOURCE_WIDTH - 1.0f) / BUFF_DISPLAY_ATLAS_SIZE,
			(BUFF_DISPLAY_ICON_SOURCE_HEIGHT - 1.0f) / BUFF_DISPLAY_ATLAS_SIZE,
			true,
			true);
		glPopAttrib();
	}

	char Text[16] = { 0 };
	this->FormatShortTime(Remaining, Text, sizeof(Text));

	DWORD BackupTextColor = SetTextColor;
	DWORD BackupBackgroundColor = SetBackgroundTextColor;
	SetTextColor = (Remaining != BUFF_DISPLAY_PERMANENT && Remaining <= 10) ? Color4b(255, 204, 25, 255) : Color4b(255, 255, 255, 255);
	SetBackgroundTextColor = Color4b(0, 0, 0, 160);
	HGDIOBJ PreviousFont = GetCurrentObject(m_hFontDC, OBJ_FONT);
	SelectObject(m_hFontDC, this->m_TimerFont);
	int TextWidth = GetTextWidth(Text);
	RenderText(Entry->PosX + ((BUFF_DISPLAY_SLOT_SIZE - TextWidth) / 2), Entry->PosY + BUFF_DISPLAY_TIMER_TOP, Text, 0, RT3_SORT_LEFT, NULL);
	SelectObject(m_hFontDC, PreviousFont);
	SetBackgroundTextColor = BackupBackgroundColor;
	SetTextColor = BackupTextColor;
}

void CBuffDisplay::RenderTooltipEntry(const BUFF_DISPLAY_ENTRY* Entry, DWORD Remaining)
{
	int Width = 190;
	int Height = 54;
	int PosX = Entry->PosX + (BUFF_DISPLAY_SLOT_SIZE / 2) - (Width / 2);
	int PosY = Entry->PosY + BUFF_DISPLAY_SLOT_SIZE + 6;

	if (PosX < 2)
	{
		PosX = 2;
	}

	if (PosX + Width > 638)
	{
		PosX = 638 - Width;
	}

	if (PosY + Height > 478)
	{
		PosY = Entry->PosY - Height - 6;
	}

	EnableAlphaTest(true);
	glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
	RenderColor((float)PosX, (float)PosY, (float)Width, (float)Height);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);

	char Name[64] = { 0 };
	char Effect[128] = { 0 };
	char Duration[32] = { 0 };
	this->GetSkillName(GetSkill(Entry->Effect), Name, sizeof(Name));
	this->GetEffectText(Entry->Effect, Entry->Value, Effect, sizeof(Effect));
	this->FormatLongTime(Remaining, Duration, sizeof(Duration));

	DWORD BackupTextColor = SetTextColor;
	DWORD BackupBackgroundColor = SetBackgroundTextColor;
	SetBackgroundTextColor = Color4b(0, 0, 0, 0);
	SetTextColor = Color4b(255, 255, 255, 255);
	HGDIOBJ PreviousFont = GetCurrentObject(m_hFontDC, OBJ_FONT);
	int TextWidth;
	SelectObject(m_hFontDC, g_hFontBold);
	TextWidth = GetTextWidth(Name);
	RenderText(PosX + ((Width - TextWidth) / 2), PosY + 5, Name, 0, RT3_SORT_LEFT, NULL);
	SelectObject(m_hFontDC, g_hFont);
	TextWidth = GetTextWidth(Effect);
	RenderText(PosX + ((Width - TextWidth) / 2), PosY + 20, Effect, 0, RT3_SORT_LEFT, NULL);
	TextWidth = GetTextWidth(Duration);
	RenderText(PosX + ((Width - TextWidth) / 2), PosY + 35, Duration, 0, RT3_SORT_LEFT, NULL);
	SelectObject(m_hFontDC, PreviousFont);
	SetBackgroundTextColor = BackupBackgroundColor;
	SetTextColor = BackupTextColor;
}

int CBuffDisplay::GetRightMargin() const
{
	int RightMargin = BUFF_DISPLAY_DEFAULT_RIGHT;

	if (this->m_ArrowHudVisible == false || this->m_ArrowHudScreenWidth <= 0 || WindowWidth <= 0)
	{
		return RightMargin;
	}

	int ArrowLeftVirtual = (this->m_ArrowHudLeft * 640) / this->m_ArrowHudScreenWidth;
	int AvailableRight = ArrowLeftVirtual - 8;

	if (AvailableRight > 0)
	{
		int ArrowRightMargin = 640 - AvailableRight;

		if (ArrowRightMargin > RightMargin)
		{
			RightMargin = ArrowRightMargin;
		}
	}

	return RightMargin;
}

bool CBuffDisplay::IsDisplayableEffect(BYTE Effect)
{
	return (Effect == 1 || Effect == 2 || Effect == 4 || Effect == 8);
}

int CBuffDisplay::GetSkill(BYTE Effect)
{
	switch (Effect)
	{
		case 1: return 28;
		case 2: return 27;
		case 4: return 16;
		case 8: return 48;
	}

	return 0;
}

const char* CBuffDisplay::GetEffectText(BYTE Effect, const WORD* Value, char* Text, int Size)
{
	WORD Amount = (Value == 0) ? 0 : Value[0];

	switch (Effect)
	{
		case 1:
			if (gLanguage.LangNum == LANGUAGE_PORTUGUESE) sprintf_s(Text, Size, "Dano f\xEDsico/m\xE1gico +%u", Amount);
			else if (gLanguage.LangNum == LANGUAGE_SPANISH) sprintf_s(Text, Size, "Da\xF1o f\xEDsico/m\xE1gico +%u", Amount);
			else sprintf_s(Text, Size, "Physical/magic damage +%u", Amount);
			break;
		case 2:
			if (gLanguage.LangNum == LANGUAGE_PORTUGUESE) sprintf_s(Text, Size, "Defesa +%u", Amount);
			else if (gLanguage.LangNum == LANGUAGE_SPANISH) sprintf_s(Text, Size, "Defensa +%u", Amount);
			else sprintf_s(Text, Size, "Defense +%u", Amount);
			break;
		case 4:
			if (gLanguage.LangNum == LANGUAGE_PORTUGUESE) sprintf_s(Text, Size, "Redu\xE7\xE3o de dano %u%%", Amount);
			else if (gLanguage.LangNum == LANGUAGE_SPANISH) sprintf_s(Text, Size, "Reducci\xF3n de da\xF1o %u%%", Amount);
			else sprintf_s(Text, Size, "Damage reduction %u%%", Amount);
			break;
		case 8:
			if (gLanguage.LangNum == LANGUAGE_PORTUGUESE) sprintf_s(Text, Size, "Vida m\xE1xima +%u%%", Amount);
			else if (gLanguage.LangNum == LANGUAGE_SPANISH) sprintf_s(Text, Size, "Vida m\xE1xima +%u%%", Amount);
			else sprintf_s(Text, Size, "Maximum life +%u%%", Amount);
			break;
		default:
			Text[0] = 0;
			break;
	}

	return Text;
}

void CBuffDisplay::GetSkillName(int Skill, char* Name, int Size)
{
	if (Name == 0 || Size <= 0)
	{
		return;
	}

	Name[0] = 0;
	GetSkillInformation(Skill, 1, Name, NULL, NULL, NULL);
	Name[Size - 1] = 0;
}

void CBuffDisplay::FormatShortTime(DWORD Seconds, char* Text, int Size)
{
	if (Seconds == BUFF_DISPLAY_PERMANENT)
	{
		sprintf_s(Text, Size, "--:--");
	}
	else
	{
		sprintf_s(Text, Size, "%us", Seconds);
	}
}

void CBuffDisplay::FormatLongTime(DWORD Seconds, char* Text, int Size)
{
	if (Seconds == BUFF_DISPLAY_PERMANENT)
	{
		sprintf_s(Text, Size, "%s: --:--:--", CBuffDisplay::GetDurationLabel());
	}
	else
	{
		sprintf_s(Text, Size, "%s: %u:%02u:%02u", CBuffDisplay::GetDurationLabel(), Seconds / 3600, (Seconds / 60) % 60, Seconds % 60);
	}
}

const char* CBuffDisplay::GetDurationLabel()
{
	if (gLanguage.LangNum == LANGUAGE_PORTUGUESE)
	{
		return "Dura\xE7\xE3o";
	}

	if (gLanguage.LangNum == LANGUAGE_SPANISH)
	{
		return "Duraci\xF3n";
	}

	return "Duration";
}
