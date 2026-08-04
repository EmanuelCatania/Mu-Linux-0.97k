#include "stdafx.h"
#include "PartyDisplay.h"
#include "BuffDisplay.h"
#include "Font.h"
#include "MapManager.h"
#include "Offsets.h"

CPartyDisplay gPartyDisplay;

#pragma pack(push, 1)
struct PARTY_DISPLAY_KICK_SEND
{
	PBMSG_HEAD header;
	BYTE number;
};
#pragma pack(pop)

static_assert(sizeof(PARTY_DISPLAY_KICK_SEND) == 4, "Invalid native party kick packet size");

static void ForcePartyDisplayTexture(int Texture)
{
	MainTextureCache = -1;
	BindTexture(Texture);
}

struct PARTY_DISPLAY_RENDER_STATE
{
	int TextureCache;
	BYTE TextureEnabled;
	BYTE AlphaTestEnabled;
	DWORD BlendMode;

	PARTY_DISPLAY_RENDER_STATE()
	{
		this->TextureCache = MainTextureCache;
		this->TextureEnabled = MainTextureEnabled;
		this->AlphaTestEnabled = MainAlphaTestEnabled;
		this->BlendMode = MainBlendMode;
		glPushAttrib(GL_ALL_ATTRIB_BITS);
	}

	~PARTY_DISPLAY_RENDER_STATE()
	{
		glPopAttrib();
		MainTextureCache = this->TextureCache;
		MainTextureEnabled = this->TextureEnabled;
		MainAlphaTestEnabled = this->AlphaTestEnabled;
		MainBlendMode = this->BlendMode;
	}
};

static bool IsPartyDisplayEffect(BYTE Effect)
{
	return (Effect == 1 || Effect == 2 || Effect == 4 || Effect == 8);
}

static bool ParsePartyDisplayMembers(const BYTE* Data, int Size, BYTE Count, int EffectStride,
	PARTY_DISPLAY_MEMBER_ENTRY* Output)
{
	if (Data == 0 || Output == 0 || Count > PARTY_DISPLAY_MAX_MEMBERS ||
		(EffectStride != 1 && EffectStride != (int)sizeof(PMSG_PARTY_DISPLAY_EFFECT_RECV) && EffectStride != 8))
	{
		return false;
	}

	PARTY_DISPLAY_MEMBER_ENTRY Temp[PARTY_DISPLAY_MAX_MEMBERS] = {};
	bool Seen[PARTY_DISPLAY_MAX_MEMBERS] = {};
	const BYTE* Cursor = Data + sizeof(PMSG_PARTY_DISPLAY_RECV);
	const BYTE* End = Data + Size;

	for (int n = 0; n < Count; n++)
	{
		if ((End - Cursor) < (int)sizeof(PMSG_PARTY_DISPLAY_MEMBER_RECV))
		{
			return false;
		}

		const PMSG_PARTY_DISPLAY_MEMBER_RECV* Info = (const PMSG_PARTY_DISPLAY_MEMBER_RECV*)Cursor;
		if (Info->number >= PARTY_DISPLAY_MAX_MEMBERS || Seen[Info->number] != false ||
			Info->effectCount > PARTY_DISPLAY_MAX_EFFECTS)
		{
			return false;
		}

		Cursor += sizeof(PMSG_PARTY_DISPLAY_MEMBER_RECV);
		if ((End - Cursor) < (int)(Info->effectCount * EffectStride))
		{
			return false;
		}

		PARTY_DISPLAY_MEMBER_ENTRY* Member = &Temp[Info->number];
		Member->Active = true;
		Member->Number = Info->number;
		Member->Level = Info->level;
		Member->Class = Info->Class;
		Member->ChangeUp = Info->ChangeUp;
		Member->EffectCount = 0;
		Seen[Info->number] = true;

		for (int k = 0; k < Info->effectCount; k++)
		{
			BYTE Effect = Cursor[0];
			if (IsPartyDisplayEffect(Effect))
			{
				bool Duplicate = false;
				for (int e = 0; e < Member->EffectCount; e++)
				{
					if (Member->Effect[e].Effect == Effect)
					{
						Duplicate = true;
						break;
					}
				}

				if (Duplicate == false && Member->EffectCount < PARTY_DISPLAY_MAX_EFFECTS)
				{
					Member->Effect[Member->EffectCount++].Effect = Effect;
				}
			}

			Cursor += EffectStride;
		}

		for (int a = 0; a < Member->EffectCount; a++)
		{
			for (int b = a + 1; b < Member->EffectCount; b++)
			{
				if (Member->Effect[b].Effect < Member->Effect[a].Effect)
				{
					PARTY_DISPLAY_EFFECT_ENTRY Swap = Member->Effect[a];
					Member->Effect[a] = Member->Effect[b];
					Member->Effect[b] = Swap;
				}
			}
		}
	}

	if (Cursor != End)
	{
		return false;
	}

	memcpy(Output, Temp, sizeof(Temp));
	return true;
}

CPartyDisplay::CPartyDisplay()
{
	memset(this->m_Member, 0, sizeof(this->m_Member));
	memset(this->m_Layout, 0, sizeof(this->m_Layout));
	this->m_Initialized = false;
	this->m_LayoutX = 0;
	this->m_LayoutY = 0;
	this->m_LayoutCount = 0;
	this->m_PartyFont = NULL;
}

CPartyDisplay::~CPartyDisplay()
{
	if (this->m_PartyFont != NULL && this->m_PartyFont != g_hFont && this->m_PartyFont != g_hFontBold)
	{
		DeleteObject((HGDIOBJ)this->m_PartyFont);
	}
}

void CPartyDisplay::Init()
{
	if (this->m_Initialized)
	{
		return;
	}

	int PartyFontHeight = FontHeight + 1;
	if (PartyFontHeight < 8)
	{
		PartyFontHeight = 8;
	}

	this->m_PartyFont = CreateFont(
		PartyFontHeight,
		gFont.Width,
		0,
		0,
		FW_BOLD,
		gFont.Italic,
		gFont.UnderLine,
		gFont.StrikeOut,
		gFont.Charset,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		gFont.Quality,
		DEFAULT_PITCH,
		gFont.MyFontFaceName);

	if (this->m_PartyFont == NULL)
	{
		this->m_PartyFont = g_hFontBold;
	}

	SetCompleteHook(0xE8, PartyNativeRenderCall, &CPartyDisplay::RenderNativeHook);
	this->m_Initialized = true;
}

void __cdecl CPartyDisplay::RenderNativeHook(int X, int Y)
{
	if (PartyOpened == 0)
	{
		PartyNativeRender(X, Y);
		return;
	}

	if (gPartyDisplay.m_Initialized && SceneFlag == MAIN_SCENE && g_bGameServerConnected != FALSE)
	{
		gPartyDisplay.RenderNative(X, Y);
	}
}

void CPartyDisplay::Reset()
{
	memset(this->m_Member, 0, sizeof(this->m_Member));
	memset(this->m_Layout, 0, sizeof(this->m_Layout));
	this->m_LayoutX = 0;
	this->m_LayoutY = 0;
	this->m_LayoutCount = 0;
}

void CPartyDisplay::GCRecv(PMSG_PARTY_DISPLAY_RECV* lpMsg, int Size)
{
	if (lpMsg == 0 || Size < (int)sizeof(PMSG_PARTY_DISPLAY_RECV) ||
		lpMsg->header.type != 0xC2 || lpMsg->header.head != 0xF3 || lpMsg->header.subh != 0xEA)
	{
		return;
	}

	int PacketSize = MAKEWORD(lpMsg->header.size[1], lpMsg->header.size[0]);
	if (PacketSize != Size || lpMsg->count > PARTY_DISPLAY_MAX_MEMBERS)
	{
		return;
	}

	// Accept the current packed entry and older 1/8-byte variants while servers
	// are being rolled forward. The visual layer consumes only effect.
	if (ParsePartyDisplayMembers((const BYTE*)lpMsg, Size, lpMsg->count,
		(int)sizeof(PMSG_PARTY_DISPLAY_EFFECT_RECV), this->m_Member) == false)
	{
		if (ParsePartyDisplayMembers((const BYTE*)lpMsg, Size, lpMsg->count, 1, this->m_Member) == false)
		{
			ParsePartyDisplayMembers((const BYTE*)lpMsg, Size, lpMsg->count, 8, this->m_Member);
		}
	}
}

PARTY_DISPLAY_MEMBER_ENTRY* CPartyDisplay::GetMember(BYTE Number)
{
	if (Number >= PARTY_DISPLAY_MAX_MEMBERS || this->m_Member[Number].Active == false)
	{
		return 0;
	}

	return &this->m_Member[Number];
}

const char* CPartyDisplay::GetClassName(BYTE Class, BYTE ChangeUp)
{
	switch (Class)
	{
		case 0: return ((ChangeUp != 0) ? "SM" : "DW");
		case 1: return ((ChangeUp != 0) ? "BK" : "DK");
		case 2: return ((ChangeUp != 0) ? "ME" : "FE");
		case 3: return "MG";
		default: return "??";
	}
}

void CPartyDisplay::RenderNativeFrame(int X, int Y)
{
	DisableAlphaBlend();
	EnableAlphaTest(true);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	ForcePartyDisplayTexture(0x104);
	RenderBitmap(0x104, (float)X, (float)Y, 190.0f, 256.0f, 0.0f, 0.0f,
		190.0f / 256.0f, 256.0f / 256.0f, true, true);
	ForcePartyDisplayTexture(0x105);
	RenderBitmap(0x105, (float)X, (float)(Y + 256), 190.0f, 177.0f, 0.0f, 0.0f,
		190.0f / 256.0f, 177.0f / 256.0f, true, true);

	DWORD OldTextColor = SetTextColor;
	DWORD OldBackgroundTextColor = SetBackgroundTextColor;
	SetBackgroundTextColor = 0;
	SetTextColor = Color4b(220, 220, 220, 255);
	const char* Title = "Grupo";
	RenderText(X + ((190 - GetTextWidth((char*)Title)) / 2), Y + 12, Title, 0, 0, NULL);
	SetBackgroundTextColor = OldBackgroundTextColor;
	SetTextColor = OldTextColor;
}

void CPartyDisplay::RenderNativeMember(const PARTY_t* Native, PARTY_DISPLAY_MEMBER_ENTRY* Member, PARTY_DISPLAY_ROW_LAYOUT* Layout)
{
	if (Native == 0 || Layout == 0 || Layout->Active == false)
	{
		return;
	}

	EnableAlphaBlend();
	glColor4f(0.015f, 0.015f, 0.015f, 0.98f);
	RenderColor((float)Layout->RowRect.X, (float)Layout->RowRect.Y,
		(float)Layout->RowRect.Width, (float)Layout->RowRect.Height);
	DisableAlphaBlend();
	EnableAlphaTest(true);
	glColor3f(1.0f, 1.0f, 1.0f);

	DWORD OldTextColor = SetTextColor;
	DWORD OldBackgroundTextColor = SetBackgroundTextColor;
	SetBackgroundTextColor = 0;
	HGDIOBJ OldFont = GetCurrentObject(m_hFontDC, OBJ_FONT);
	SelectObject(m_hFontDC, this->m_PartyFont != NULL ? this->m_PartyFont : g_hFontBold);

	char Text[64] = { 0 };
	sprintf_s(Text, sizeof(Text), "%s %u", GetClassName(Member != 0 ? Member->Class : 0, Member != 0 ? Member->ChangeUp : 0),
		(Member != 0) ? Member->Level : 0);
	SetTextColor = Color4b(232, 232, 232, 255);
	RenderText(Layout->ClassRect.X + Layout->ClassRect.Width - GetTextWidth(Text), Layout->ClassRect.Y, Text, 0, 0, NULL);

	SetTextColor = Color4b(138, 216, 255, 255);
	RenderText(Layout->NameRect.X, Layout->NameRect.Y, Native->Name, 0, 0, NULL);

	char MapText[64] = { 0 };
	char* MapName = gMapManager.GetMapName(Native->map);
	sprintf_s(MapText, sizeof(MapText), "%s %u,%u", MapName != 0 ? MapName : "Unknown", Native->x, Native->y);
	SetTextColor = Color4b(255, 228, 168, 255);
	RenderText(Layout->LocationRect.X, Layout->LocationRect.Y, MapText, 0, 0, NULL);

	char LifeText[48] = { 0 };
	sprintf_s(LifeText, sizeof(LifeText), "%u/%u", Native->CurLife, Native->MaxLife);
	SetTextColor = Color4b(232, 232, 232, 255);
	RenderText(Layout->LifeTextRect.X + Layout->LifeTextRect.Width - GetTextWidth(LifeText), Layout->LifeTextRect.Y, LifeText, 0, 0, NULL);

	SelectObject(m_hFontDC, OldFont);

	if (Layout->PartySlot == 0)
	{
		glColor4f(1.0f, 0.72f, 0.08f, 1.0f);
		RenderColor((float)Layout->NameRect.X - 8.0f, (float)Layout->NameRect.Y + 4.0f, 7.0f, 3.0f);
		RenderColor((float)Layout->NameRect.X - 6.0f, (float)Layout->NameRect.Y + 2.0f, 3.0f, 7.0f);
	}

	float Life = Native->MaxLife != 0 ? ((float)Native->CurLife / (float)Native->MaxLife) : 0.0f;
	if (Life < 0.0f) Life = 0.0f;
	if (Life > 1.0f) Life = 1.0f;
	glColor3f(0.03f, 0.01f, 0.01f);
	RenderColor((float)Layout->LifeBarRect.X, (float)Layout->LifeBarRect.Y,
		(float)Layout->LifeBarRect.Width, (float)Layout->LifeBarRect.Height);
	if (Life > 0.0f)
	{
		glColor3f(0.90f, 0.04f, 0.01f);
		RenderColor((float)Layout->LifeBarRect.X + 1.0f, (float)Layout->LifeBarRect.Y + 1.0f,
			(float)(Layout->LifeBarRect.Width - 2) * Life, (float)(Layout->LifeBarRect.Height - 2));
	}

	for (int n = 0; Member != 0 && n < Member->EffectCount && n < 4; n++)
	{
		gBuffDisplay.RenderPartyEntry(Member->Effect[n].Effect, Layout->BuffRect[n].X,
			Layout->BuffRect[n].Y, Layout->BuffRect[n].Width);
	}

	EnableAlphaTest(true);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	const bool Hover = Layout->KickRect.Contains(MouseX, MouseY);
	const int KickTexture = Hover ? 0x119 : 0x118;
	ForcePartyDisplayTexture(KickTexture);
	RenderBitmap(KickTexture, (float)Layout->KickRect.X, (float)Layout->KickRect.Y,
		(float)Layout->KickRect.Width, (float)Layout->KickRect.Height, 0.0f, 0.0f,
		24.0f / 32.0f, 24.0f / 32.0f, true, true);

	SetBackgroundTextColor = OldBackgroundTextColor;
	SetTextColor = OldTextColor;
}

void CPartyDisplay::BuildLayout(int X, int Y)
{
	this->m_LayoutX = X;
	this->m_LayoutY = Y;
	this->m_LayoutCount = (BYTE)min(max(PartyNumber, 0), PARTY_DISPLAY_MAX_MEMBERS);

	for (int n = 0; n < PARTY_DISPLAY_MAX_MEMBERS; n++)
	{
		PARTY_DISPLAY_ROW_LAYOUT* Layout = &this->m_Layout[n];
		memset(Layout, 0, sizeof(*Layout));
		if (n >= this->m_LayoutCount)
		{
			continue;
		}

		int RowY = Y + 48 + (n * 60);
		Layout->Active = true;
		Layout->PartySlot = Party[n].number;
		Layout->RowRect = { X + 12, RowY, 166, 56 };
		Layout->NameRect = { X + 25, RowY + 3, 82, 13 };
		Layout->ClassRect = { X + 112, RowY + 3, 53, 13 };
		Layout->LocationRect = { X + 25, RowY + 17, 88, 12 };
		Layout->LifeTextRect = { X + 113, RowY + 17, 45, 12 };
		Layout->LifeBarRect = { X + 25, RowY + 29, 118, 5 };
		for (int k = 0; k < 4; k++)
		{
			Layout->BuffRect[k] = { X + 25 + (k * 23), RowY + 34, 22, 22 };
		}
		Layout->KickRect = { X + 145, RowY + 31, 24, 24 };
	}
}

void CPartyDisplay::SendPartyKick(BYTE Number)
{
	PARTY_DISPLAY_KICK_SEND pMsg = {};
	pMsg.header.set(0x43, sizeof(pMsg));
	pMsg.number = Number;
	gProtocol.DataSend((BYTE*)&pMsg, sizeof(pMsg));
}

void CPartyDisplay::RenderNative(int X, int Y)
{
	if (this->m_Initialized == false || PartyOpened == 0)
	{
		return;
	}

	PartyWindowX = X;
	PartyWindowY = Y;
	PARTY_DISPLAY_RENDER_STATE RenderState;
	this->BuildLayout(X, Y);
	this->RenderNativeFrame(X, Y);

	for (int n = 0; n < this->m_LayoutCount; n++)
	{
		PARTY_DISPLAY_MEMBER_ENTRY* Member = this->GetMember(this->m_Layout[n].PartySlot);
		this->RenderNativeMember(&Party[n], Member, &this->m_Layout[n]);
	}

	PARTY_DISPLAY_RECT CloseRect = { X + 25, Y + 395, 24, 24 };
	const bool Hover = CloseRect.Contains(MouseX, MouseY);
	EnableAlphaTest(true);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	const int CloseTexture = Hover ? 0x119 : 0x118;
	ForcePartyDisplayTexture(CloseTexture);
	RenderBitmap(CloseTexture, (float)CloseRect.X, (float)CloseRect.Y,
		(float)CloseRect.Width, (float)CloseRect.Height, 0.0f, 0.0f, 24.0f / 32.0f, 24.0f / 32.0f, true, true);
}

void CPartyDisplay::UpdateMouse()
{
	if (this->m_Initialized == false || SceneFlag != MAIN_SCENE || g_bGameServerConnected == FALSE || PartyOpened == 0)
	{
		return;
	}

	int PartyX = PartyWindowX;
	int PartyY = PartyWindowY;
	if (this->m_LayoutCount == 0 || this->m_LayoutX != PartyX || this->m_LayoutY != PartyY)
	{
		this->BuildLayout(PartyX, PartyY);
	}

	if (MouseX >= PartyX && MouseX < PartyX + 190 && MouseY >= PartyY && MouseY < PartyY + 433)
	{
		MouseOnWindow = true;
	}

	if (MouseLButton == false || MouseLButtonPush == false)
	{
		return;
	}

	PARTY_DISPLAY_RECT CloseRect = { PartyX + 25, PartyY + 395, 24, 24 };
	if (CloseRect.Contains(MouseX, MouseY))
	{
		PartyOpened = 0;
		MouseLButtonPush = false;
		return;
	}

	for (int n = 0; n < this->m_LayoutCount; n++)
	{
		const PARTY_DISPLAY_ROW_LAYOUT* Layout = &this->m_Layout[n];
		if (Layout->KickRect.Contains(MouseX, MouseY))
		{
			this->SendPartyKick(Layout->PartySlot);
			MouseLButtonPush = false;
			return;
		}

		if (Layout->RowRect.Contains(MouseX, MouseY) && Party[n].index >= 0)
		{
			SelectedCharacter = (DWORD)Party[n].index;
			MouseLButtonPush = false;
			return;
		}
	}
}
