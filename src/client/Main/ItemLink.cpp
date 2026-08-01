#include "stdafx.h"
#include "ItemLink.h"
#include "Input.h"
#include "Protocol.h"
#include "ItemManager.h"

static const DWORD CHAT_RENDER_TEXT_CALL = 0x00480BC8;

static const DWORD CHAT_WINDOW_VTABLE = 0x005525CC;
static const DWORD CHAT_WINDOW_LINE_RENDER_SLOT = 0x5C;
static const DWORD CHAT_WINDOW_LINE_RENDER_EXPECTED = 0x0040D610;

/* UIChatLogWindow_LineLayout applies this mode-specific scroll offset before
 * calculating the Y coordinate in the traditional chat view. */
static const DWORD CHAT_LOG_RENDER_MODE = 0x005590AC;

static const BYTE CHAT_RENDER_TEXT_BYTES[5] =
{
	0xE8, 0xD3, 0xEB, 0xFF, 0xFF
};

static const DWORD ITEM_LINK_EQUIPMENT_FLAG = 0x80000000;

/* Existing client code passes visual RGB values to the legacy Color4b macro;
 * this encodes the opaque #08090C token background. */
static const DWORD ITEM_LINK_BACKGROUND_COLOR = Color4b(8, 9, 12, 255);

static bool IsPostMessage(const char* Message)
{
	if (Message == NULL || _strnicmp(Message, "/post", 5) != 0)
	{
		return false;
	}

	return (Message[5] == 0 || Message[5] == ' ' || Message[5] == '\t');
}

static int (__thiscall* OriginalChatWindowLineRender)(
	int This,
	int LineIndex) = NULL;

static const int ITEM_TOOLTIP_MODEL_SIZE = 64;
static const int ITEM_TOOLTIP_MODEL_GAP = 8;

#if defined(_DEBUG)
static void ItemLinkDebug(const char* Format, ...);
#endif

struct PMSG_STANDARD_CHAT
{
	PBMSG_HEAD header;
	char name[10];
	char message[60];
};

CItemLink gItemLink;
CItemLink::TOOLTIP_LAYOUT CItemLink::m_TooltipLayout = {};

bool CItemLink::IsExpectedCall(DWORD Address, DWORD Target)
{
	return *(BYTE*)Address == 0xE8 &&
		(DWORD)(Address + 5 + *(int*)(Address + 1)) == Target;
}

int CItemLink::GetTooltipLineHeight()
{
	SIZE TextSize = { 0, 0 };
	const char* Text = "Ag";

	if (m_hFontDC != NULL)
	{
		GetTextExtentPointA(m_hFontDC, Text, 2, &TextSize);
	}

	return max(1, TextSize.cy);
}

bool CItemLink::IsTooltipSeparator(const char* Text)
{
	return Text != NULL &&
		(Text[0] == '\n' || (Text[0] == ' ' && Text[1] == '\0'));
}

int CItemLink::CalculateTooltipTextHeight(int TextCount)
{
	if (m_hFontDC == NULL || TextCount <= 0 || TextCount > 64)
	{
		return 0;
	}

	int Height = 0;
	SIZE TextSize = { 0, 0 };
	HGDIOBJ PreviousFont = GetCurrentObject(m_hFontDC, OBJ_FONT);

	for (int Index = 0; Index < TextCount; Index++)
	{
		const char* Text = TextList[Index];

		if (Text[0] == 0)
		{
			break;
		}

		SelectObject(
			m_hFontDC,
			(TextBold[Index] != 0) ? g_hFontBold : g_hFont);
		TextSize.cx = 0;
		TextSize.cy = 0;
		GetTextExtentPointA(
			m_hFontDC,
			Text,
			lstrlenA(Text),
			&TextSize);

		Height += (Text[0] == '\n') ?
			(TextSize.cy / 2) : TextSize.cy;
	}

	SelectObject(m_hFontDC, PreviousFont);

	if (g_fScreenRate_y <= 0.0f)
	{
		return Height;
	}

	return (int)ceilf(
		(float)Height / (g_fScreenRate_y * 0.9090909f));
}

void CItemLink::PrepareTooltipRectangle(float Y, float Height)
{
	if (CItemLink::m_TooltipLayout.active == false ||
		CItemLink::m_TooltipLayout.rectangleValid != false)
	{
		return;
	}

	const int NativeY = (int)floorf(Y + 1.0f);
	CItemLink::m_TooltipLayout.height =
		max(0, (int)ceilf(Height - 1.0f));

	const int ExpandedHeight =
		CItemLink::m_TooltipLayout.height +
		CItemLink::m_TooltipLayout.extraHeight;

	CItemLink::m_TooltipLayout.offsetY = 0;

	if (NativeY + ExpandedHeight > WindowHeight)
	{
		CItemLink::m_TooltipLayout.offsetY =
			WindowHeight - (NativeY + ExpandedHeight);
	}

	if (NativeY + CItemLink::m_TooltipLayout.offsetY < 0)
	{
		CItemLink::m_TooltipLayout.offsetY = -NativeY;
	}

	CItemLink::m_TooltipLayout.y =
		NativeY + CItemLink::m_TooltipLayout.offsetY;
	CItemLink::m_TooltipLayout.height = ExpandedHeight;
	CItemLink::m_TooltipLayout.rectangleValid = true;
}

void CItemLink::RenderTooltipBorder(
	float X,
	float Y,
	float Width,
	float Height,
	int Part)
{
	if (CItemLink::m_TooltipLayout.active == false)
	{
		RenderColor(X, Y, Width, Height);

		return;
	}

	if (Part == 1)
	{
		CItemLink::PrepareTooltipRectangle(Y, Height);
	}
	else if (Part == 0 &&
		CItemLink::m_TooltipLayout.rectangleValid == false)
	{
		CItemLink::m_TooltipLayout.x = (int)floorf(X + 1.0f);
		if (CItemLink::m_TooltipLayout.nativeTextHeight <= 0)
		{
			CItemLink::m_TooltipLayout.y = (int)floorf(Y + 1.0f);
		}
		CItemLink::m_TooltipLayout.width =
			max(0, (int)ceilf(Width - 1.0f));
	}

	float RenderY = Y + (float)CItemLink::m_TooltipLayout.offsetY;
	float RenderHeight = Height;

	switch (Part)
	{
		case 1:
		case 2:
			RenderHeight += (float)CItemLink::m_TooltipLayout.extraHeight;
			break;

		case 3:
			RenderY += (float)CItemLink::m_TooltipLayout.extraHeight;
			break;

		case 4:
			RenderHeight += (float)CItemLink::m_TooltipLayout.extraHeight;
			break;
	}

	RenderColor(X, RenderY, Width, RenderHeight);
}

void __cdecl CItemLink::RenderTooltipTextListHook(
	void* TextListPointer,
	int Y,
	int TextCount,
	int Width,
	int Arg5,
	int Arg6)
{
	if (CItemLink::m_TooltipLayout.active != false)
	{
		CItemLink::m_TooltipLayout.nativeTextHeight =
			CItemLink::CalculateTooltipTextHeight(TextCount);

		/* The border hook has the authoritative tooltip rectangle.  When
		 * RenderItemTextList runs first, keep a provisional offset only until
		 * the border arrives; otherwise do not undo the border's correction. */
		if (CItemLink::m_TooltipLayout.rectangleValid == false)
		{
			CItemLink::m_TooltipLayout.offsetY = 0;

			const int ExpandedHeight = max(1,
				CItemLink::m_TooltipLayout.nativeTextHeight) +
				CItemLink::m_TooltipLayout.extraHeight;

			if (Y + ExpandedHeight > WindowHeight)
			{
				CItemLink::m_TooltipLayout.offsetY =
					WindowHeight - (Y + ExpandedHeight);
			}

			if (Y + CItemLink::m_TooltipLayout.offsetY < 0)
			{
				CItemLink::m_TooltipLayout.offsetY = -Y;
			}
		}
	}

	RenderItemTextList(
		TextListPointer,
		Y,
		TextCount,
		Width,
		Arg5,
		Arg6);
}

void __fastcall CItemLink::RenderTooltipLineHook(
	void* This,
	void*,
	int X,
	int Y,
	const char* Text,
	int Arg4,
	int Arg5,
	int Arg6,
	int Arg7,
	int Arg8)
{
	bool IsTitle = false;
	int RenderY = Y;

	if (CItemLink::m_TooltipLayout.active != false)
	{
		RenderY += CItemLink::m_TooltipLayout.offsetY;

		if (Text != NULL && Text[0] != 0 &&
			CItemLink::IsTooltipSeparator(Text) == false)
		{
			if (CItemLink::m_TooltipLayout.titleCaptured == false)
			{
				CItemLink::m_TooltipLayout.titleY = RenderY;
				CItemLink::m_TooltipLayout.titleCaptured = true;
				IsTitle = true;
			}
			else if (CItemLink::m_TooltipLayout.bodyCaptured == false)
			{
				CItemLink::m_TooltipLayout.bodyY = RenderY;
				CItemLink::m_TooltipLayout.bodyDirection =
					(RenderY >= CItemLink::m_TooltipLayout.titleY) ? 1 : -1;
				CItemLink::m_TooltipLayout.bodyCaptured = true;
			}

			if (IsTitle == false)
			{
				RenderY +=
					(CItemLink::m_TooltipLayout.bodyDirection >= 0) ?
					CItemLink::m_TooltipLayout.extraHeight :
					-CItemLink::m_TooltipLayout.extraHeight;
			}
		}
	}

	RenderItemTextLine(
		This,
		X,
		RenderY,
		Text,
		Arg4,
		Arg5,
		Arg6,
		Arg7,
		Arg8);
}

void __cdecl CItemLink::RenderTooltipTopHook(float X, float Y, float Width, float Height)
{
	CItemLink::RenderTooltipBorder(X, Y, Width, Height, 0);
}

void __cdecl CItemLink::RenderTooltipLeftHook(float X, float Y, float Width, float Height)
{
	CItemLink::RenderTooltipBorder(X, Y, Width, Height, 1);
}

void __cdecl CItemLink::RenderTooltipRightHook(float X, float Y, float Width, float Height)
{
	CItemLink::RenderTooltipBorder(X, Y, Width, Height, 2);
}

void __cdecl CItemLink::RenderTooltipBottomHook(float X, float Y, float Width, float Height)
{
	CItemLink::RenderTooltipBorder(X, Y, Width, Height, 3);
}

void __cdecl CItemLink::RenderTooltipFillHook(float X, float Y, float Width, float Height)
{
	CItemLink::RenderTooltipBorder(X, Y, Width, Height, 4);
}

DWORD CItemLink::GetItemLinkTextColor(const ITEM* Item)
{
	if (Item == NULL || Item->Type < 0 || Item->Type >= MAX_ITEM)
	{
		return Color4b(255, 255, 255, 255);
	}

	const int Type = Item->Type;
	const int Level = GET_ITEM_OPT_LEVEL(Item->Level);
	const BYTE Excellent = GET_ITEM_OPT_EXC(Item->Option1);
	const BYTE SpecialCount = Item->SpecialNum;

	int ColorType = 0;

	/* This is the title-color selection performed by RenderItemInfo at
	 * 0x004C4650. Keep the item-specific overrides before the generic option
	 * rules, including the special wing range. */
	if (Type == 0x1CD || Type == 0x1CE || Type == 0x18F ||
		Type == 0x1D0 || Type == 0x1D6 || Type == 0x1D1 ||
		Type == 0x1D2 || Type == 0x1D3 || Type == 0x1B0 ||
		Type == 0x1B1)
	{
		ColorType = 3;
	}
	else if (Type == 0xAA || Type == 0x13 || Type == 0x92)
	{
		ColorType = 6;
	}
	else if (SpecialCount != 0 && Excellent != 0)
	{
		ColorType = 4;
	}
	else if (Level > 6)
	{
		ColorType = 3;
	}
	else if (SpecialCount != 0)
	{
		ColorType = 1;
	}

	if (Type > 0x182 && Type < 0x187)
	{
		ColorType = (Level < 7) ?
			((SpecialCount != 0) ? 1 : 0) :
			3;
	}

	switch (ColorType)
	{
		case 1: /* native blue: #7FB2FF */
			return Color4b(127, 178, 255, 255);

		case 3: /* native yellow: #FFCC19 */
			return Color4b(255, 204, 25, 255);

		case 4: /* native excellent green: #19FF7F */
			return Color4b(25, 255, 127, 255);

		case 6: /* native special magenta: #FF19FF */
			return Color4b(255, 25, 255, 255);

		default: /* native white: #FFFFFF */
			return Color4b(255, 255, 255, 255);
	}
}

#if defined(_DEBUG)
static void ItemLinkDebug(const char* Format, ...)
{
	char Buffer[256] = { 0 };
	va_list Args;

	va_start(Args, Format);
	vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
	va_end(Args);

	OutputDebugStringA(Buffer);
	OutputDebugStringA("\n");

	FILE* File = NULL;

	if (fopen_s(&File, "ItemLink-debug.log", "a") == 0 &&
		File != NULL)
	{
		fprintf(File, "%s\n", Buffer);
		fclose(File);
	}
}
#endif

CItemLink::CItemLink()
{
	for (int n = 0; n < MAX_CHAT_LINES; n++)
	{
		this->ClearLink(&this->m_Links[n]);
		this->ClearLink(&this->m_UiLinks[n]);

		memset(
			&this->m_Identities[n],
			0,
			sizeof(this->m_Identities[n]));

		this->ClearLink(&this->m_WorkLinks[n]);

		memset(
			&this->m_WorkIdentities[n],
			0,
			sizeof(this->m_WorkIdentities[n]));
	}

	this->m_IdentityCount = 0;
	this->m_UiLinkCursor = 0;
	this->m_LastUiRenderFrame = MAXDWORD;
	this->m_LastUiOwner = 0;

	this->m_IdentityInitialized = false;

	this->m_LastRenderFrame = MAXDWORD;

	this->m_Pinned = false;

	this->m_ConsumeChatReturn = false;

	memset(&this->m_PinnedItem, 0, sizeof(this->m_PinnedItem));

	this->m_PinnedX = 0;

	this->m_PinnedY = 0;

	this->m_TooltipHooksInstalled = false;
}

int CItemLink::GetTooltipModelSize(const ITEM* Item)
{
	if (Item == NULL || Item->Type < 0 || Item->Type >= MAX_ITEM)
	{
		return ITEM_TOOLTIP_MODEL_SIZE;
	}

	/* Jewels and consumables have a naturally small native preview.  Keeping
	 * their reserved area compact prevents a large empty gap in the tooltip. */
	if (Item->Type >= GET_ITEM(14, 0) &&
		Item->Type < GET_ITEM(15, 0))
	{
		return 32;
	}

	return ITEM_TOOLTIP_MODEL_SIZE;
}

bool CItemLink::Init()
{
	if (this->IsSupportedClient() == false)
	{
		return false;
	}

	if (IsExpectedCall(ItemTooltipTextListCall, RenderItemTextListAddress) != false &&
		IsExpectedCall(ItemTooltipLineTextCall, RenderItemTextLineAddress) != false &&
		IsExpectedCall(ItemTooltipBorderTopCall, (DWORD)RenderColor) != false &&
		IsExpectedCall(ItemTooltipBorderLeftCall, (DWORD)RenderColor) != false &&
		IsExpectedCall(ItemTooltipBorderRightCall, (DWORD)RenderColor) != false &&
		IsExpectedCall(ItemTooltipBorderBottomCall, (DWORD)RenderColor) != false &&
		IsExpectedCall(ItemTooltipFillCall, (DWORD)RenderColor) != false)
	{
		SetCompleteHook(0xE8, ItemTooltipTextListCall, &CItemLink::RenderTooltipTextListHook);
		SetCompleteHook(0xE8, ItemTooltipLineTextCall, &CItemLink::RenderTooltipLineHook);
		SetCompleteHook(0xE8, ItemTooltipBorderTopCall, &CItemLink::RenderTooltipTopHook);
		SetCompleteHook(0xE8, ItemTooltipBorderLeftCall, &CItemLink::RenderTooltipLeftHook);
		SetCompleteHook(0xE8, ItemTooltipBorderRightCall, &CItemLink::RenderTooltipRightHook);
		SetCompleteHook(0xE8, ItemTooltipBorderBottomCall, &CItemLink::RenderTooltipBottomHook);
		SetCompleteHook(0xE8, ItemTooltipFillCall, &CItemLink::RenderTooltipFillHook);
		this->m_TooltipHooksInstalled = true;

#if defined(_DEBUG)
		ItemLinkDebug("[ItemLink] 3D tooltip layout hooks installed");
#endif
	}
	else
	{
#if defined(_DEBUG)
		ItemLinkDebug("[ItemLink] 3D tooltip layout unavailable; textual tooltip only");
#endif
	}

	SetCompleteHook(
		0xE8,
		CHAT_RENDER_TEXT_CALL,
		&CItemLink::RenderChatTextHook);

	DWORD* LineRenderEntry = (DWORD*)
		(CHAT_WINDOW_VTABLE + CHAT_WINDOW_LINE_RENDER_SLOT);

	if (*LineRenderEntry != CHAT_WINDOW_LINE_RENDER_EXPECTED)
	{
		return false;
	}

	DWORD PreviousProtection = 0;

	if (VirtualProtect(
		LineRenderEntry,
		sizeof(DWORD),
		PAGE_EXECUTE_READWRITE,
		&PreviousProtection) == FALSE)
	{
		return false;
	}

	OriginalChatWindowLineRender =
		(int (__thiscall*)(int, int))*LineRenderEntry;

	*LineRenderEntry = (DWORD)&CItemLink::RenderChatLineHook;

	FlushInstructionCache(
		GetCurrentProcess(),
		LineRenderEntry,
		sizeof(DWORD));

	DWORD IgnoredProtection = 0;

	VirtualProtect(
		LineRenderEntry,
		sizeof(DWORD),
		PreviousProtection,
		&IgnoredProtection);

#if defined(_DEBUG)
	ItemLinkDebug(
		"[ItemLink] render hook installed at %08X",
		CHAT_RENDER_TEXT_CALL);
	ItemLinkDebug(
		"[ItemLink] UI line hook installed vtable=%08X slot=%02X original=%08X",
		CHAT_WINDOW_VTABLE,
		CHAT_WINDOW_LINE_RENDER_SLOT,
		(DWORD)OriginalChatWindowLineRender);
#endif

	return true;
}

bool CItemLink::IsSupportedClient()
{
	return CheckBytes(
		CHAT_RENDER_TEXT_CALL,
		CHAT_RENDER_TEXT_BYTES,
		sizeof(CHAT_RENDER_TEXT_BYTES));
}

bool CItemLink::HandleLeftButtonDown()
{
	CHAT_LINK* Hovered = this->FindHoveredLink();

	if (Hovered != NULL)
	{
		this->m_Pinned = true;

		this->m_PinnedItem = Hovered->item;

		this->m_PinnedX = MouseX;

		this->m_PinnedY = MouseY;

		return true;
	}

	if (this->m_Pinned != false)
	{
		this->m_Pinned = false;
	}

	if ((GetKeyState(VK_SHIFT) & 0x8000) == 0)
	{
		return false;
	}

	return this->TryInsertPointedItem();
}

bool CItemLink::HandleKeyDown(WPARAM wParam)
{
	if (wParam == VK_ESCAPE)
	{
		this->m_Pinned = false;

		return false;
	}

	/*
	 * The original client sends chat from its keyboard state machine, not
	 * through the call site used by the login/UI packet loop.  Intercept
	 * Enter here, after the input manager has built the complete line, so a
	 * token can be replaced by the validated item-link packet.
	 */
	if (wParam == VK_RETURN &&
		gInput.IsChatInputActive() != false)
	{
		if (this->HandleMainChatSend() != false)
		{
			/* Keep the original history behavior, but consume Enter. */
			gInput.HandleKeyDown(wParam);
			ClearInput(FALSE);
			gInput.ClearChatInput();
			InputEnable = false;
			TabInputEnable = false;
			GuildInputEnable = false;
			this->m_ConsumeChatReturn = true;

			return true;
		}
	}

	return false;
}

bool CItemLink::HandleChar(WPARAM wParam)
{
	if (wParam == VK_RETURN && this->m_ConsumeChatReturn != false)
	{
		this->m_ConsumeChatReturn = false;

		return true;
	}

	return false;
}

bool CItemLink::TryInsertPointedItem()
{
	if (SceneFlag != MAIN_SCENE || InventoryOpened == 0 ||
		gInput.IsChatInputActive() == false)
	{
		return false;
	}

	ITEM Item;

	BYTE Slot = 0;
	bool Equipment = false;

	if (this->GetPointedItem(&Item, &Slot, &Equipment) == false)
	{
#if defined(_DEBUG)
		ItemLinkDebug("[ItemLink] click rejected: no pointed slot");
#endif
		return false;
	}

	if (ItemAttribute == 0 || Item.Type < 0 ||
		Item.Type >= MAX_ITEM)
	{
		return false;
	}

	ITEM_ATTRIBUTE* lpInfo =
		(ITEM_ATTRIBUTE*)(ItemAttribute +
		(Item.Type * sizeof(ITEM_ATTRIBUTE)));

	char Token[48];

	int Level = GET_ITEM_OPT_LEVEL(Item.Level);

	int TokenLength = 0;

	if (Level > 0)
	{
		TokenLength = sprintf_s(
			Token,
			sizeof(Token),
			"[%s +%d]",
			lpInfo->Name,
			Level);
	}
	else
	{
		TokenLength = sprintf_s(
			Token,
			sizeof(Token),
			"[%s]",
			lpInfo->Name);
	}

	if (TokenLength <= 0 || TokenLength >= sizeof(Token))
	{
		return true;
	}

#if defined(_DEBUG)
		ItemLinkDebug(
			"[ItemLink] click accepted slot=%d type=%d level=%d x=%d y=%d token='%s'",
			Slot,
			Item.Type,
			Level,
			Item.x,
			Item.y,
			Token);
#endif

	DWORD Value = Slot |
		(((DWORD)(WORD)Item.Type) << 8) |
		(((DWORD)(BYTE)Level) << 24);

	if (Equipment != false)
	{
		Value |= ITEM_LINK_EQUIPMENT_FLAG;
	}

	if (gInput.InsertAtomicToken(Token, Value) == false)
	{
		this->ShowMessage("Sem espaço para linkar o item.");
	}

	return true;
}

bool CItemLink::GetPointedItem(
	ITEM* Item,
	BYTE* Slot,
	bool* Equipment)
{
	if (Item == NULL || Slot == NULL || Equipment == NULL ||
		PointedInventoryItem == NULL)
	{
		return false;
	}

	ITEM* Pointed = PointedInventoryItem;
	*Equipment = false;

	/*
	 * Equipped items are exposed by the original tooltip code as a pointer
	 * into CharacterMachine.  Resolve that address first; comparing the full
	 * ITEM contents is unreliable while the client is decrypting/updating the
	 * character structure.
	 */
	if (CharacterMachine != 0)
	{
		DWORD EquipmentBegin =
			(DWORD)(CharacterMachine + 536);

		DWORD PointedAddress = (DWORD)Pointed;

		DWORD EquipmentBytes =
			INVENTORY_WEAR_SIZE * sizeof(ITEM);

		if (PointedAddress >= EquipmentBegin &&
			PointedAddress < (EquipmentBegin + EquipmentBytes) &&
			((PointedAddress - EquipmentBegin) % sizeof(ITEM)) == 0)
		{
			BYTE EquipmentSlot = (BYTE)(
				(PointedAddress - EquipmentBegin) / sizeof(ITEM));

			ITEM* Candidate = NULL;
			bool Valid = false;

			/* CharacterMachine is encrypted outside the original item code. */
			STRUCT_DECRYPT;

			Candidate = this->GetItemBySlot(EquipmentSlot);

			if (Candidate != NULL &&
				Candidate->Type >= 0 &&
				Candidate->Type < MAX_ITEM)
			{
				*Item = *Candidate;
				*Slot = EquipmentSlot;
				*Equipment = true;
				Valid = true;
			}

			STRUCT_ENCRYPT;

			return Valid;
		}
	}

	if (Pointed->Type < 0 || Pointed->Type >= MAX_ITEM)
	{
		return false;
	}

	for (int n = 0; n < INVENTORY_MAX_SIZE; n++)
	{
		ITEM* Candidate = GetInventoryItem((BYTE)n);

		if (Candidate == Pointed)
		{
			*Item = *Candidate;

			/* Multi-cell items can expose any occupied cell as the pointed
			 * address. The server, however, indexes the authoritative anchor. */
			int AnchorSlot = INVENTORY_WEAR_SIZE +
				Item->x + (Item->y * 8);

			if (AnchorSlot < INVENTORY_WEAR_SIZE ||
				AnchorSlot >= INVENTORY_MAX_SIZE)
			{
				return false;
			}

			*Slot = (BYTE)AnchorSlot;

			return true;
		}
	}

	/*
	 * Some client builds expose a temporary copy while the equipment
	 * tooltip is being updated. Match that copy against the real slots so
	 * Shift+click still resolves to the authoritative slot.
	 */
	for (int n = 0; n < INVENTORY_MAX_SIZE; n++)
	{
		ITEM* Candidate = GetInventoryItem((BYTE)n);

		if (Candidate != NULL &&
			Candidate->Type == Pointed->Type &&
			memcmp(Candidate, Pointed, sizeof(ITEM)) == 0)
		{
			*Item = *Candidate;

			int AnchorSlot = INVENTORY_WEAR_SIZE +
				Item->x + (Item->y * 8);

			if (AnchorSlot < INVENTORY_WEAR_SIZE ||
				AnchorSlot >= INVENTORY_MAX_SIZE)
			{
				return false;
			}

			*Slot = (BYTE)AnchorSlot;

			return true;
		}
	}

	return false;
}

ITEM* CItemLink::GetItemBySlot(BYTE Slot)
{
	if (INVENTORY_MAX_RANGE(Slot) == 0)
	{
		return NULL;
	}

	if (INVENTORY_WEAR_RANGE(Slot) != 0)
	{
		return (ITEM*)(CharacterMachine +
			536 +
			(sizeof(ITEM) * Slot));
	}

	return GetInventoryItem(Slot);
}

void CItemLink::ShowMessage(const char* Message)
{
	if (Message != NULL && Message[0] != 0)
	{
		UIChatLogWindow_AddText("", Message, 2);
	}
}

bool CItemLink::HandleOutgoingChat(BYTE* lpMsg, DWORD size)
{
	if (lpMsg == NULL || size < sizeof(PBMSG_HEAD) + 10 + 1 ||
		size > sizeof(PMSG_STANDARD_CHAT) ||
		lpMsg[0] != 0xC1 || lpMsg[2] != 0x00)
	{
		return false;
	}

	int LinkStart = 0;

	int LinkLength = 0;

	DWORD Value = 0;

	if (gInput.GetAtomicToken(
		&LinkStart,
		&LinkLength,
		&Value) == false)
	{
		return false;
	}

	const size_t MessageOffset = offsetof(PMSG_STANDARD_CHAT, message);

	const size_t MessageCapacity =
		sizeof(PMSG_STANDARD_CHAT) - MessageOffset;

	const size_t MessageBytes = min(
		MessageCapacity,
		size - MessageOffset);

	char Message[60] = { 0 };

	memcpy(Message, lpMsg + MessageOffset, MessageBytes);

	int MessageLength = (int)strnlen_s(
		Message,
		MessageCapacity);

	BYTE Slot = (BYTE)(Value & 0xFF);
	bool Equipment = (Value & ITEM_LINK_EQUIPMENT_FLAG) != 0;

	WORD Type = (WORD)((Value >> 8) & 0xFFFF);

	BYTE Level = (BYTE)((Value >> 24) & 0x7F);

	ITEM Item;
	memset(&Item, 0, sizeof(Item));

	ITEM* lpItem = NULL;
	bool ValidItem = false;

	if (Equipment != false)
	{
		STRUCT_DECRYPT;
		lpItem = this->GetItemBySlot(Slot);

		if (lpItem != NULL)
		{
			Item = *lpItem;
			ValidItem = true;
		}

		STRUCT_ENCRYPT;
	}
	else
	{
		if (Slot >= INVENTORY_WEAR_SIZE)
		{
			lpItem = GetInventoryItem(
				(BYTE)(Slot - INVENTORY_WEAR_SIZE));
		}

		if (lpItem != NULL)
		{
			Item = *lpItem;
			ValidItem = true;
		}
	}

	bool Valid = (ValidItem &&
		Item.Type == Type &&
		GET_ITEM_OPT_LEVEL(Item.Level) == Level &&
		LinkStart >= 0 && LinkLength > 0 &&
		(LinkStart + LinkLength) <= MessageLength);

	gInput.ClearAtomicToken();

	if (Valid == false)
	{
		this->ShowMessage("O item linkado foi movido ou removido.");

		return true;
	}

	if (IsPostMessage(Message) != false)
	{
		PMSG_ITEM_POST_LINK_RECV pMsg;

		pMsg.header.set(0xF3, 0xE8, sizeof(pMsg));

		pMsg.itemType[0] = HIBYTE(Type);
		pMsg.itemType[1] = LOBYTE(Type);
		pMsg.itemLevel = Level;
		pMsg.slot = Slot;
		pMsg.linkStart = (BYTE)LinkStart;
		pMsg.linkLength = (BYTE)LinkLength;
		memset(pMsg.message, 0, sizeof(pMsg.message));
		memcpy(pMsg.message, Message, MessageLength);

		gProtocol.DataSend((BYTE*)&pMsg, sizeof(pMsg));
	}
	else
	{
		PMSG_ITEM_LINK_SEND pMsg;

		pMsg.header.set(0xF3, 0xE7, sizeof(pMsg));

		pMsg.itemType[0] = HIBYTE(Type);
		pMsg.itemType[1] = LOBYTE(Type);
		pMsg.itemLevel = Level;
		pMsg.slot = Slot;
		pMsg.linkStart = (BYTE)LinkStart;
		pMsg.linkLength = (BYTE)LinkLength;
		memset(pMsg.message, 0, sizeof(pMsg.message));
		memcpy(pMsg.message, Message, MessageLength);

		gProtocol.DataSend((BYTE*)&pMsg, sizeof(pMsg));
	}

#if defined(_DEBUG)
	gConsole.Write(
		"[ItemLink] send slot=%d type=%d level=%d start=%d length=%d",
		Slot,
		Type,
		Level,
		LinkStart,
		LinkLength);
#endif

	return true;
}

bool CItemLink::HandleMainChatSend()
{
	int LinkStart = 0;
	int LinkLength = 0;
	DWORD Value = 0;

	if (gInput.GetAtomicToken(
		&LinkStart,
		&LinkLength,
		&Value) == false)
	{
		return false;
	}

	PMSG_STANDARD_CHAT Chat;

	memset(&Chat, 0, sizeof(Chat));

	Chat.header.set(0x00, sizeof(Chat));

	int MessageLength = min(
		(int)sizeof(Chat.message) - 1,
		(int)strnlen_s(InputText[0], sizeof(InputText[0])));

	if (MessageLength <= 0)
	{
		return false;
	}

	memcpy(
		Chat.message,
		InputText[0],
		MessageLength);

	return this->HandleOutgoingChat(
		(BYTE*)&Chat,
		sizeof(Chat));
}

void CItemLink::ReceiveItemLinkMessage(
	const char* Name,
	const char* Message,
	BYTE LinkStart,
	BYTE LinkLength,
	const BYTE* ItemInfo,
	BYTE Channel)
{
	if (Name == NULL || Message == NULL || ItemInfo == NULL)
	{
		return;
	}

	int MessageLength = (int)strnlen_s(
		Message,
		60);

	if (MessageLength <= 0 || LinkLength == 0 ||
		LinkStart >= MessageLength ||
		(LinkStart + LinkLength) > MessageLength)
	{
		return;
	}

	bool StructuredPost = (Channel <= 2);

	if (StructuredPost != false)
	{
		if ((Channel == 0 && (Message[0] == '~' || Message[0] == '@')) ||
			(Channel == 1 && Message[0] != '~') ||
			(Channel == 2 && Message[0] != '@'))
		{
			return;
		}

#if defined(_DEBUG)
		ItemLinkDebug(
			"[ItemLink] post recv style=%u name='%s' message='%s' start=%u length=%u",
			(unsigned int)Channel,
			Name,
			Message,
			(unsigned int)LinkStart,
			(unsigned int)LinkLength);
#endif
	}

	int OldCount = 0;
	int AddedLines = 0;

	if (StructuredPost == false)
	{
		this->SynchronizeMetadata(0);

		OldCount = this->m_IdentityCount;

		AddedLines = this->GetMessageLineCount(
			Name,
			Message);
	}

	PMSG_STANDARD_CHAT Chat;

	Chat.header.set(0x00, sizeof(Chat));

	memset(Chat.name, 0, sizeof(Chat.name));

	memcpy(Chat.name, Name, sizeof(Chat.name));

	memset(Chat.message, 0, sizeof(Chat.message));

	memcpy(Chat.message, Message, MessageLength);

	if (StructuredPost == false)
	{
		ReceiveChatMessage(&Chat);
	}
	else if (Channel == 0)
	{
		/* Match the original C1:02 post path.  Type 0 is the gold post
		 * style and avoids the normal-chat overhead/bubble processing. */
		RegisterWhisperName(10, Name);

		if (m_bWhisperSound != false)
		{
			PlayBuffer(38, 0, FALSE);
		}

		UIChatLogWindow_AddText(Name, Message, 0);

#if defined(_DEBUG)
		ItemLinkDebug("[ItemLink] post gold path type=0");
#endif
	}
	else
	{
		/* The server includes '~' or '@'; ReceiveChatMessage selects the
		 * original blue/green post render path and strips the marker. */
		ReceiveChatMessage(&Chat);

#if defined(_DEBUG)
		ItemLinkDebug(
			"[ItemLink] post colored path marker=%c",
			Message[0]);
#endif
	}

	ITEM Item;

	this->DecodeItem(ItemInfo, &Item);

	this->AttachUiLink(
		Name,
		Message,
		LinkStart,
		LinkLength,
		&Item);

	if (StructuredPost == false)
	{
		int MinimumShift = max(
			0,
			(OldCount + AddedLines) - MAX_CHAT_LINES);

		int FirstNewLine = this->SynchronizeMetadata(MinimumShift);

		int NewCount = this->m_IdentityCount;

		this->AttachReceivedLink(
			Message,
			LinkStart,
			LinkLength,
			&Item,
			FirstNewLine,
			NewCount);
	}

}

void CItemLink::GCItemLinkRecv(PMSG_ITEM_LINK_RECV* lpMsg)
{
	if (lpMsg == NULL || lpMsg->header.size != sizeof(PMSG_ITEM_LINK_RECV))
	{
		return;
	}

	lpMsg->name[sizeof(lpMsg->name) - 1] = 0;
	lpMsg->message[sizeof(lpMsg->message) - 1] = 0;

	this->ReceiveItemLinkMessage(
		lpMsg->name,
		lpMsg->message,
		lpMsg->linkStart,
		lpMsg->linkLength,
		lpMsg->ItemInfo,
		0xFF);
}

void CItemLink::GCItemPostLinkRecv(PMSG_ITEM_POST_LINK_SEND* lpMsg)
{
	if (lpMsg == NULL ||
		lpMsg->header.size != sizeof(PMSG_ITEM_POST_LINK_SEND) ||
		lpMsg->style > 2)
	{
		return;
	}

	lpMsg->name[sizeof(lpMsg->name) - 1] = 0;
	lpMsg->message[sizeof(lpMsg->message) - 1] = 0;

	this->ReceiveItemLinkMessage(
		lpMsg->name,
		lpMsg->message,
		lpMsg->linkStart,
		lpMsg->linkLength,
		lpMsg->ItemInfo,
		lpMsg->style);
}

void CItemLink::ClearLink(CHAT_LINK* Link)
{
	if (Link != NULL)
	{
		memset(Link, 0, sizeof(CHAT_LINK));
	}
}

void CItemLink::ReadIdentity(
	int Index,
	CHAT_IDENTITY* Identity)
{
	if (Identity == NULL || Index < 0 || Index >= MAX_CHAT_LINES)
	{
		return;
	}

	BYTE* Entry = (BYTE*)(ChatLogBuffer +
		(Index * CHAT_ENTRY_SIZE));

	memset(Identity, 0, sizeof(CHAT_IDENTITY));

	strncpy_s(
		Identity->name,
		sizeof(Identity->name),
		(char*)Entry,
		_TRUNCATE);

	strncpy_s(
		Identity->message,
		sizeof(Identity->message),
		(char*)(Entry + 0x0B),
		_TRUNCATE);

	Identity->type = *(BYTE*)(Entry + 0x10C);
}

bool CItemLink::IdentityEquals(
	const CHAT_IDENTITY* Left,
	const CHAT_IDENTITY* Right)
{
	return (Left != NULL && Right != NULL &&
		Left->type == Right->type &&
		strcmp(Left->name, Right->name) == 0 &&
		strcmp(Left->message, Right->message) == 0);
}

int CItemLink::SynchronizeMetadata(int MinimumShift)
{
	int NewCount = min(ChatLogCount, MAX_CHAT_LINES);

	bool Changed = (MinimumShift > 0 ||
		this->m_IdentityInitialized == false ||
		this->m_IdentityCount != NewCount);

	for (int n = 0; n < NewCount; n++)
	{
		this->ReadIdentity(n, &this->m_WorkIdentities[n]);

		if (Changed == false &&
			this->IdentityEquals(
				&this->m_Identities[n],
				&this->m_WorkIdentities[n]) == false)
		{
			Changed = true;
		}
	}

	if (Changed == false)
	{
		return NewCount;
	}

	int Overlap = 0;

	if (this->m_IdentityInitialized == false)
	{
		for (int n = 0; n < MAX_CHAT_LINES; n++)
		{
			this->ClearLink(&this->m_Links[n]);
		}
	}
	else
	{
		memcpy(
			this->m_WorkLinks,
			this->m_Links,
			sizeof(this->m_WorkLinks));

		int Shift = this->m_IdentityCount;

		MinimumShift = max(
			0,
			min(MinimumShift, this->m_IdentityCount));

		for (int Candidate = MinimumShift;
			Candidate <= this->m_IdentityCount;
			Candidate++)
		{
			int CandidateOverlap = min(
				this->m_IdentityCount - Candidate,
				NewCount);

			bool Matches = true;

			for (int n = 0; n < CandidateOverlap; n++)
			{
				if (this->IdentityEquals(
					&this->m_Identities[Candidate + n],
					&this->m_WorkIdentities[n]) == false)
				{
					Matches = false;

					break;
				}
			}

			if (Matches != false)
			{
				Shift = Candidate;

				Overlap = CandidateOverlap;

				break;
			}
		}

		for (int n = 0; n < MAX_CHAT_LINES; n++)
		{
			this->ClearLink(&this->m_Links[n]);
		}

		for (int n = 0; n < Overlap; n++)
		{
			this->m_Links[n] =
				this->m_WorkLinks[Shift + n];

			this->m_Links[n].identity =
				this->m_WorkIdentities[n];
		}
	}

	for (int n = 0; n < NewCount; n++)
	{
		this->m_Identities[n] = this->m_WorkIdentities[n];
	}

	for (int n = NewCount; n < MAX_CHAT_LINES; n++)
	{
		memset(
			&this->m_Identities[n],
			0,
			sizeof(this->m_Identities[n]));
	}

	this->m_IdentityCount = NewCount;

	this->m_IdentityInitialized = true;

	return Overlap;
}

int CItemLink::GetMessageLineCount(
	const char* Name,
	const char* Message)
{
	if (Name == NULL || Message == NULL)
	{
		return 1;
	}

	if (Message[0] == '~' || Message[0] == '@')
	{
		Message++;
	}

	char Text[320];

	int Length = sprintf_s(
		Text,
		sizeof(Text),
		"%s : %s",
		Name,
		Message);

	if (Length <= 0)
	{
		return 1;
	}

	SIZE TextSize = { 0 };

	if (GetTextExtentPointA(
		m_hFontDC,
		Text,
		Length,
		&TextSize) == FALSE)
	{
		return 1;
	}

	return (TextSize.cx < 256) ? 1 : 2;
}

void CItemLink::AttachReceivedLink(
	const char* Message,
	BYTE LinkStart,
	BYTE LinkLength,
	const ITEM* Item,
	int FirstNewLine,
	int NewCount)
{
	if (Message == NULL || Item == NULL)
	{
		return;
	}

	const char* DisplayMessage = Message;

	int DisplayLinkStart = LinkStart;

	if (Message[0] == '~' || Message[0] == '@')
	{
		DisplayMessage++;

		DisplayLinkStart--;
	}

	if (DisplayLinkStart < 0)
	{
		return;
	}

	int LinkEnd = DisplayLinkStart + LinkLength;

	/* ReceiveChatMessage may compact or scroll the fixed-size chat array.
	 * The returned overlap is only a hint, so locate the newest line that
	 * contains the complete canonical message. */
	int Start = min(NewCount - 1, MAX_CHAT_LINES - 1);
	int End = max(0, min(FirstNewLine, Start));
	int FoundLine = -1;
	int FoundStart = 0;

	for (int n = Start; n >= End; n--)
	{
		const char* Line = this->m_Links[n].identity.message;

		if (Line[0] == 0)
		{
			CHAT_IDENTITY Identity;
			this->ReadIdentity(n, &Identity);
			this->m_Links[n].identity = Identity;
			Line = this->m_Links[n].identity.message;
		}

		const char* LineText = Line;

		if (LineText[0] == '~' || LineText[0] == '@')
		{
			LineText++;
		}

		const char* Found = strstr(LineText, DisplayMessage);
		if (Found != NULL)
		{
			FoundLine = n;
			FoundStart = (int)(Found - LineText);
			break;
		}
	}

	if (FoundLine < 0 && End > 0)
	{
		for (int n = End - 1; n >= 0; n--)
		{
			const char* Line = this->m_Links[n].identity.message;
			const char* LineText = Line;

			if (LineText[0] == '~' || LineText[0] == '@')
			{
				LineText++;
			}

			const char* Found = strstr(LineText, DisplayMessage);

			if (Found != NULL)
			{
				FoundLine = n;
				FoundStart = (int)(Found - LineText);
				break;
			}
		}
	}

	if (FoundLine >= 0 &&
		DisplayLinkStart >= 0 &&
		LinkEnd <= (int)strlen(DisplayMessage))
	{
		this->m_Links[FoundLine].active = true;
		this->m_Links[FoundLine].linkStart =
			(BYTE)(FoundStart + DisplayLinkStart);
		this->m_Links[FoundLine].linkLength = LinkLength;
		this->m_Links[FoundLine].item = *Item;
	}
}

void CItemLink::DecodeItem(
	const BYTE* ItemInfo,
	ITEM* Item)
{
	memset(Item, 0, sizeof(ITEM));

	BYTE Data[MAX_ITEM_INFO];

	memcpy(Data, ItemInfo, sizeof(Data));

	Item->Type = (short)ConvertItemType(Data);

	DecodeItemInfo(Item, Data[1], Data[3]);

	Item->Durability = Data[2];
}

void CItemLink::AttachUiLink(
	const char* Name,
	const char* Message,
	BYTE LinkStart,
	BYTE LinkLength,
	const ITEM* Item)
{
	if (Name == NULL || Message == NULL || Item == NULL ||
		LinkLength == 0)
	{
		return;
	}

	int MessageLength = (int)strnlen_s(Message, CHAT_TEXT_SIZE);

	if (MessageLength <= 0 || LinkStart >= MessageLength ||
		(LinkStart + LinkLength) > MessageLength)
	{
		return;
	}

	int DisplayLinkStart = LinkStart;

	if (Message[0] == '~' || Message[0] == '@')
	{
		DisplayLinkStart--;
	}

	if (DisplayLinkStart < 0 ||
		(DisplayLinkStart + LinkLength) > MessageLength)
	{
		return;
	}

	CHAT_LINK* Link = &this->m_UiLinks[
		this->m_UiLinkCursor % MAX_CHAT_LINES];

	this->m_UiLinkCursor =
		(this->m_UiLinkCursor + 1) % MAX_CHAT_LINES;

	this->ClearLink(Link);

	Link->active = true;
	Link->linkStart = (BYTE)DisplayLinkStart;
	Link->linkLength = LinkLength;
	Link->item = *Item;

	strncpy_s(
		Link->identity.name,
		sizeof(Link->identity.name),
		Name,
		_TRUNCATE);

	strncpy_s(
		Link->identity.message,
		sizeof(Link->identity.message),
		Message + ((Message[0] == '~' || Message[0] == '@') ? 1 : 0),
		_TRUNCATE);

#if defined(_DEBUG)
	ItemLinkDebug(
		"[ItemLink] ui link queued name='%s' message='%s' start=%d length=%d",
		Link->identity.name,
		Link->identity.message,
		Link->linkStart,
		Link->linkLength);
#endif
}

int __fastcall CItemLink::RenderChatLineHook(
	int This,
	int Unused,
	int LineIndex)
{
	UNREFERENCED_PARAMETER(Unused);

	int Result = 0;

	if (OriginalChatWindowLineRender != NULL)
	{
		Result = OriginalChatWindowLineRender(This, LineIndex);
	}

	gItemLink.RenderUiLink(This, LineIndex);

	return Result;
}

void CItemLink::RenderUiLink(
	int This,
	int LineIndex)
{
	if (This == 0 || LineIndex < 0 || LineIndex > MAX_CHAT_LINES)
	{
		return;
	}

	/* The line renderer is called once for every visible line.  A new pass
	 * starts at line zero; clear only the hitboxes belonging to this window so
	 * the compact (F4) and traditional chat windows can coexist. */
	if (LineIndex == 0 || this->m_LastUiOwner != (DWORD)This)
	{
		for (int n = 0; n < MAX_CHAT_LINES; n++)
		{
			for (int h = 0; h < 2; h++)
			{
				if (this->m_UiLinks[n].uiHitBoxes[h].owner == (DWORD)This)
				{
					memset(
						&this->m_UiLinks[n].uiHitBoxes[h],
						0,
						sizeof(this->m_UiLinks[n].uiHitBoxes[h]));
				}
			}

		}

		this->m_LastUiOwner = (DWORD)This;
		this->m_LastUiRenderFrame = ChatLogRenderFrame;
	}

	DWORD LineNode = *(DWORD*)(This + 0x64);

	if (LineNode == 0)
	{
		return;
	}

	char* Message = (char*)(LineNode + 0x13);
	int MessageLength = (int)strnlen_s(Message, CHAT_TEXT_SIZE);

	if (MessageLength <= 0 || MessageLength >= CHAT_TEXT_SIZE)
	{
		return;
	}

	/* In the compact/F4 view the line index is used as-is.  In the
	 * traditional view, newly inserted lines are animated by the offset stored
	 * at node + 0x11C.  The original renderer adds that offset before drawing;
	 * omitting it makes the cyan overlay stay behind while the name/message
	 * scrolls upward. */
	int LayoutLineIndex = LineIndex;

	if (*(int*)CHAT_LOG_RENDER_MODE == 0)
	{
		int ScrollOffset = *(int*)(LineNode + 0x11C);

		if (ScrollOffset != 0 && *(int*)(This + 0x88) == 0)
		{
			if (((*(int*)(This + 0x8C) - ScrollOffset) -
				LineIndex - 1) < 0)
			{
				return;
			}

			LayoutLineIndex += ScrollOffset;
		}
	}

	CHAT_LINK* FoundLink = NULL;
	CHAT_LINK* UnassignedLink = NULL;
	int LocalLinkStart = -1;

	for (int n = 0; n < MAX_CHAT_LINES; n++)
	{
		CHAT_LINK* Link = &this->m_UiLinks[n];

		if (Link->active == false)
		{
			continue;
		}

		/* A message can contain the same item text as another message.  Keep
		 * the association with the actual chat node so the second occurrence
		 * receives its own hitbox instead of reusing the first metadata entry. */
		if (Link->lineNode != 0 && Link->lineNode != LineNode)
		{
			continue;
		}

		const char* Target = Link->identity.message;

		if (Target[0] == '~' || Target[0] == '@')
		{
			Target++;
		}

		const char* Found = strstr(Message, Target);

		if (Found != NULL &&
			((Found - Message) + Link->linkStart + Link->linkLength) <= MessageLength)
		{
			if (Link->lineNode == LineNode)
			{
				FoundLink = Link;
				LocalLinkStart = (int)(Found - Message) + Link->linkStart;
				break;
			}

			if (UnassignedLink == NULL)
			{
				UnassignedLink = Link;
			}
		}

		if (Link->linkLength > 0 && Link->linkLength < 64)
		{
			char Token[64] = { 0 };
			int TokenStart = Link->linkStart;

			if (TokenStart < (int)strlen(Target))
			{
				strncpy_s(
					Token,
					sizeof(Token),
					Target + TokenStart,
					Link->linkLength);

				Found = strstr(Message, Token);

				if (Found != NULL)
				{
					if (Link->lineNode == LineNode)
					{
						FoundLink = Link;
						LocalLinkStart = (int)(Found - Message);
						break;
					}

					if (UnassignedLink == NULL)
					{
						UnassignedLink = Link;
					}
				}
			}
		}
	}

	if (FoundLink == NULL && UnassignedLink != NULL)
	{
		FoundLink = UnassignedLink;
		const char* Found = strstr(
			Message,
			FoundLink->identity.message[0] == '~' ||
			FoundLink->identity.message[0] == '@' ?
			FoundLink->identity.message + 1 :
			FoundLink->identity.message);

		if (Found != NULL)
		{
			LocalLinkStart = (int)(Found - Message) +
				FoundLink->linkStart;
		}
	}

	if (FoundLink == NULL || LocalLinkStart < 0)
	{
		return;
	}

	FoundLink->lineNode = LineNode;

	char Prefix[CHAT_TEXT_SIZE] = { 0 };
	strncpy_s(Prefix, sizeof(Prefix), Message, LocalLinkStart);

	char Token[64] = { 0 };
	strncpy_s(
		Token,
		sizeof(Token),
		Message + LocalLinkStart,
		FoundLink->linkLength);

	char NamePrefix[CHAT_NAME_SIZE + 3] = { 0 };
	char* Name = (char*)(LineNode + 8);

	if (Name[0] != 0)
	{
		sprintf_s(NamePrefix, sizeof(NamePrefix), "%s: ", Name);
	}

	int X = *(int*)(This + 0x2C) + 10 +
		GetTextWidth(NamePrefix) + GetTextWidth(Prefix);

	int Y = *(int*)(This + 0x30) + (LayoutLineIndex * -13) - 16;

	DWORD OriginalColor = SetTextColor;
	DWORD OriginalBackgroundColor = SetBackgroundTextColor;
	SetTextColor = CItemLink::GetItemLinkTextColor(&FoundLink->item);
	SetBackgroundTextColor = ITEM_LINK_BACKGROUND_COLOR;

	RenderText(X, Y, Token, 0, 0, NULL);

	SetBackgroundTextColor = OriginalBackgroundColor;
	SetTextColor = OriginalColor;

	int HitWidth = GetTextWidth(Token);
	int HitHeight = max(GetTextHeight(Token), 13);

	int HitBoxIndex = -1;

	for (int h = 0; h < 2; h++)
	{
		if (FoundLink->uiHitBoxes[h].owner == (DWORD)This)
		{
			HitBoxIndex = h;
			break;
		}
	}

	if (HitBoxIndex < 0)
	{
		for (int h = 0; h < 2; h++)
		{
			if (FoundLink->uiHitBoxes[h].owner == 0)
			{
				HitBoxIndex = h;
				break;
			}
		}
	}

	if (HitBoxIndex < 0)
	{
		HitBoxIndex = 0;
	}

	FoundLink->uiHitBoxes[HitBoxIndex].x = X;
	FoundLink->uiHitBoxes[HitBoxIndex].y = Y;
	FoundLink->uiHitBoxes[HitBoxIndex].width = HitWidth;
	FoundLink->uiHitBoxes[HitBoxIndex].height = HitHeight;
	FoundLink->uiHitBoxes[HitBoxIndex].owner = (DWORD)This;
	FoundLink->uiHitBoxes[HitBoxIndex].frame = ChatLogRenderFrame;

	/* Keep the legacy fields populated for diagnostics and for code that
	 * inspects the last rendered position.  Hit testing uses uiHitBoxes. */
	FoundLink->hitX = X;
	FoundLink->hitY = Y;
	FoundLink->hitWidth = HitWidth;
	FoundLink->hitHeight = HitHeight;
	FoundLink->renderedAt = ChatLogRenderFrame;
}

int CItemLink::RenderChatTextHook(
	int X,
	int Y,
	const char* Text,
	int Width,
	int Sort,
	SIZE* TextSize)
{
	if (gItemLink.m_LastRenderFrame != ChatLogRenderFrame)
	{
		gItemLink.SynchronizeMetadata(0);

		for (int n = 0; n < MAX_CHAT_LINES; n++)
		{
			gItemLink.m_Links[n].hitX = 0;
			gItemLink.m_Links[n].hitY = 0;
			gItemLink.m_Links[n].hitWidth = 0;
			gItemLink.m_Links[n].hitHeight = 0;
			gItemLink.m_Links[n].renderedAt = 0;
		}

		gItemLink.m_LastRenderFrame = ChatLogRenderFrame;
	}

	int Index = ChatLogViewStart + (Y / 13);

	if (Text == NULL || Index < 0 || Index >= MAX_CHAT_LINES ||
		gItemLink.m_Links[Index].active == false)
	{
		return RenderChatText(
			X,
			Y,
			Text,
			Width,
			Sort,
			TextSize);
	}

	CHAT_LINK* Link = &gItemLink.m_Links[Index];

	const char* Message = Link->identity.message;

	const char* MessageInText = strstr(Text, Message);

	if (MessageInText == NULL ||
		(Link->linkStart + Link->linkLength) > (int)strlen(Message))
	{
		return RenderChatText(
			X,
			Y,
			Text,
			Width,
			Sort,
			TextSize);
	}

	int LinkStart = (int)(MessageInText - Text) +
		Link->linkStart;

	int LinkLength = Link->linkLength;

	char Prefix[320];

	char Token[64];

	char Suffix[320];

	strncpy_s(Prefix, sizeof(Prefix), Text, LinkStart);

	strncpy_s(
		Token,
		sizeof(Token),
		Text + LinkStart,
		LinkLength);

	strncpy_s(
		Suffix,
		sizeof(Suffix),
		Text + LinkStart + LinkLength,
		_TRUNCATE);

	DWORD OriginalColor = SetTextColor;
	DWORD OriginalBackgroundColor = SetBackgroundTextColor;

	int CurrentX = X;

	if (Prefix[0] != 0)
	{
		RenderChatText(CurrentX, Y, Prefix, 0, Sort, NULL);

		CurrentX += GetTextWidth(Prefix);
	}

	SetTextColor = CItemLink::GetItemLinkTextColor(
		&gItemLink.m_Links[Index].item);
	SetBackgroundTextColor = ITEM_LINK_BACKGROUND_COLOR;

	RenderChatText(CurrentX, Y, Token, 0, Sort, NULL);

	Link->hitX = CurrentX;

	Link->hitY = Y;

	Link->hitWidth = GetTextWidth(Token);

	Link->hitHeight = max(GetTextHeight(Token), 13);

	Link->renderedAt = ChatLogRenderFrame;

	CurrentX += Link->hitWidth;

	SetBackgroundTextColor = OriginalBackgroundColor;
	SetTextColor = OriginalColor;

	int Result = 0;

	if (Suffix[0] != 0)
	{
		Result = RenderChatText(
			CurrentX,
			Y,
			Suffix,
			0,
			Sort,
			TextSize);
	}

	return Result;
}

bool CItemLink::IsInsideHitBox(const CHAT_LINK* Link)
{
	if (Link == NULL || Link->active == false)
	{
		return false;
	}

	for (int h = 0; h < 2; h++)
	{
		const CHAT_LINK::HIT_BOX* Hit = &Link->uiHitBoxes[h];

		if (Hit->owner != this->m_LastUiOwner || Hit->owner == 0 ||
			Hit->frame == 0 || Hit->width <= 0 || Hit->height <= 0)
		{
			continue;
		}

		if (MouseX >= Hit->x && MouseX <= (Hit->x + Hit->width) &&
			MouseY >= Hit->y && MouseY <= (Hit->y + Hit->height))
		{
			return true;
		}
	}

	return (Link->renderedAt == ChatLogRenderFrame &&
		MouseX >= Link->hitX &&
		MouseX <= (Link->hitX + Link->hitWidth) &&
		MouseY >= Link->hitY &&
		MouseY <= (Link->hitY + Link->hitHeight));
}

 CItemLink::CHAT_LINK* CItemLink::FindHoveredLink()
{
	for (int n = 0; n < MAX_CHAT_LINES; n++)
	{
		if (this->IsInsideHitBox(&this->m_UiLinks[n]) != false)
		{
			return &this->m_UiLinks[n];
		}

		if (this->IsInsideHitBox(&this->m_Links[n]) != false)
		{
			return &this->m_Links[n];
		}
	}

	return NULL;
}

void CItemLink::RenderTooltip()
{
	if (SceneFlag != MAIN_SCENE)
	{
		this->m_Pinned = false;

		return;
	}

	CHAT_LINK* Hovered = this->FindHoveredLink();

	if (Hovered != NULL)
	{
		memset(&CItemLink::m_TooltipLayout, 0, sizeof(CItemLink::m_TooltipLayout));
		CItemLink::m_TooltipLayout.active = this->m_TooltipHooksInstalled;
		CItemLink::m_TooltipLayout.modelSize =
			CItemLink::GetTooltipModelSize(&Hovered->item);
		CItemLink::m_TooltipLayout.extraHeight =
			this->m_TooltipHooksInstalled ?
			CItemLink::m_TooltipLayout.modelSize + ITEM_TOOLTIP_MODEL_GAP : 0;

		RenderItemInfo(
			MouseX,
			MouseY,
			&Hovered->item,
			false);

		CItemLink::m_TooltipLayout.active = false;
		this->RenderTooltipModel(&Hovered->item);

		return;
	}

	if (this->m_Pinned != false)
	{
		memset(&CItemLink::m_TooltipLayout, 0, sizeof(CItemLink::m_TooltipLayout));
		CItemLink::m_TooltipLayout.active = this->m_TooltipHooksInstalled;
		CItemLink::m_TooltipLayout.modelSize =
			CItemLink::GetTooltipModelSize(&this->m_PinnedItem);
		CItemLink::m_TooltipLayout.extraHeight =
			this->m_TooltipHooksInstalled ?
			CItemLink::m_TooltipLayout.modelSize + ITEM_TOOLTIP_MODEL_GAP : 0;

		RenderItemInfo(
			this->m_PinnedX,
			this->m_PinnedY,
			&this->m_PinnedItem,
			false);

		CItemLink::m_TooltipLayout.active = false;
		this->RenderTooltipModel(&this->m_PinnedItem);
	}
}

void CItemLink::RenderTooltipModel(const ITEM* Item)
{
	if (this->m_TooltipHooksInstalled == false ||
		Item == NULL || Item->Type < 0 || Item->Type >= MAX_ITEM ||
		CItemLink::m_TooltipLayout.rectangleValid == false ||
		CItemLink::m_TooltipLayout.titleCaptured == false)
	{
		return;
	}

	const int LineStep =
		(CItemLink::m_TooltipLayout.bodyCaptured != false) ?
		abs(CItemLink::m_TooltipLayout.bodyY -
			CItemLink::m_TooltipLayout.titleY) :
		this->GetTooltipLineHeight();

	const int ModelSize = (CItemLink::m_TooltipLayout.modelSize > 0) ?
		CItemLink::m_TooltipLayout.modelSize : ITEM_TOOLTIP_MODEL_SIZE;

	int X = CItemLink::m_TooltipLayout.x +
		(CItemLink::m_TooltipLayout.width - ModelSize) / 2;
	int Y = CItemLink::m_TooltipLayout.titleY;

	if (CItemLink::m_TooltipLayout.bodyDirection >= 0)
	{
		Y += LineStep;
	}
	else
	{
		Y -= ModelSize + LineStep;
	}

	const int MinimumX = CItemLink::m_TooltipLayout.x;
	const int MaximumX =
		CItemLink::m_TooltipLayout.x +
		CItemLink::m_TooltipLayout.width - ModelSize;
	const int MinimumY = CItemLink::m_TooltipLayout.y;
	const int MaximumY =
		CItemLink::m_TooltipLayout.y +
		CItemLink::m_TooltipLayout.height - ModelSize;

	X = max(MinimumX, min(X, MaximumX));
	Y = max(MinimumY, min(Y, MaximumY));

	gItemManager.RenderItemLink3D(
		(float)X,
		(float)Y,
		(float)ModelSize,
		(float)ModelSize,
		(ITEM*)Item);
}
