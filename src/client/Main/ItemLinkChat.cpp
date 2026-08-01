#include "stdafx.h"
#include "ItemLinkChat.h"

static const DWORD CHAT_LINE_NODE_OFFSET = 0x64;
static const DWORD CHAT_LINE_MESSAGE_OFFSET = 0x13;
static const DWORD CHAT_LINE_SCROLL_OFFSET = 0x11C;
static const DWORD CHAT_WINDOW_STATE_OFFSET = 0x88;
static const DWORD CHAT_WINDOW_SCROLL_OFFSET = 0x8C;
static const DWORD CHAT_WINDOW_POS_X_OFFSET = 0x2C;
static const DWORD CHAT_WINDOW_POS_Y_OFFSET = 0x30;
static const DWORD CHAT_ENTRY_MESSAGE_OFFSET = 0x0B;
static const DWORD CHAT_ENTRY_TYPE_OFFSET = 0x10C;
static const DWORD ITEM_LINK_BACKGROUND_COLOR = Color4b(8, 9, 12, 255);

typedef int (__thiscall* CHAT_LINE_RENDER)(int This, int LineIndex);
static CHAT_LINE_RENDER OriginalChatWindowLineRender = NULL;

CItemLinkChat gItemLinkChat;

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
	gConsole.Write((char*)"%s", Buffer);
}
#endif

CItemLinkChat::CItemLinkChat()
{
	memset(this, 0, sizeof(*this));
	this->m_LastRenderFrame = MAXDWORD;
}

bool CItemLinkChat::Init()
{
	if (this->m_TextHook.Installed != false &&
		this->m_LineHook.Installed != false)
	{
		return true;
	}

	memset(&this->m_TextHook, 0, sizeof(this->m_TextHook));
	memset(&this->m_LineHook, 0, sizeof(this->m_LineHook));

	this->m_TextHook.Address = ItemLinkChatRenderTextCall;
	this->m_TextHook.Target = RenderChatTextAddress;
	this->m_TextHook.Hook = (DWORD)&CItemLinkChat::RenderTextHook;

	this->m_LineHook.Address =
		ItemLinkChatWindowVTable + ItemLinkChatWindowLineRenderSlot;
	this->m_LineHook.Expected = ItemLinkChatWindowLineRenderExpected;
	this->m_LineHook.Hook = (DWORD)&CItemLinkChat::RenderLineHook;

	if (CheckRelativeCall(
		this->m_TextHook.Address,
		this->m_TextHook.Target) == false ||
		CheckBytes(
			this->m_LineHook.Address,
			(const BYTE*)&this->m_LineHook.Expected,
			sizeof(this->m_LineHook.Expected)) == false)
	{
		return false;
	}

	if (InstallRelativeCallHook(&this->m_TextHook) == false)
	{
		return false;
	}

	if (InstallVtableHook(&this->m_LineHook) == false)
	{
		RestoreRelativeCallHook(&this->m_TextHook);
		return false;
	}

	OriginalChatWindowLineRender =
		(CHAT_LINE_RENDER)this->m_LineHook.Original;

	return true;
}

int CItemLinkChat::GetIdentityCount() const
{
	return this->m_IdentityCount;
}

bool CItemLinkChat::GetHoveredItem(ITEM* Item)
{
	if (Item == NULL)
	{
		return false;
	}

	CHAT_LINK* Link = this->FindHoveredLink();

	if (Link == NULL)
	{
		return false;
	}

	*Item = Link->item;
	return true;
}


void CItemLinkChat::ClearLink(CHAT_LINK* Link)
{
	if (Link != NULL)
	{
		memset(Link, 0, sizeof(CHAT_LINK));
	}
}

void CItemLinkChat::ReadIdentity(
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
		(char*)(Entry + CHAT_ENTRY_MESSAGE_OFFSET),
		_TRUNCATE);

	Identity->type = *(BYTE*)(Entry + CHAT_ENTRY_TYPE_OFFSET);
}

bool CItemLinkChat::IdentityEquals(
	const CHAT_IDENTITY* Left,
	const CHAT_IDENTITY* Right)
{
	return (Left != NULL && Right != NULL &&
		Left->type == Right->type &&
		strcmp(Left->name, Right->name) == 0 &&
		strcmp(Left->message, Right->message) == 0);
}

int CItemLinkChat::SynchronizeMetadata(int MinimumShift)
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

int CItemLinkChat::GetMessageLineCount(
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

void CItemLinkChat::AttachReceivedLink(
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

void CItemLinkChat::AttachUiLink(
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

int __fastcall CItemLinkChat::RenderLineHook(
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

	gItemLinkChat.RenderUiLink(This, LineIndex);

	return Result;
}

void CItemLinkChat::RenderUiLink(
	int This,
	int LineIndex)
{
	if (This == 0 || LineIndex < 0 || LineIndex >= MAX_CHAT_LINES)
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
	}

	DWORD LineNode = *(DWORD*)(This + CHAT_LINE_NODE_OFFSET);

	if (LineNode == 0)
	{
		return;
	}

	char* Message = (char*)(LineNode + CHAT_LINE_MESSAGE_OFFSET);
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

	if (*(int*)ItemLinkChatLogRenderMode == 0)
	{
		int ScrollOffset = *(int*)(LineNode + CHAT_LINE_SCROLL_OFFSET);

		if (ScrollOffset != 0 && *(int*)(This + CHAT_WINDOW_STATE_OFFSET) == 0)
		{
			if (((*(int*)(This + CHAT_WINDOW_SCROLL_OFFSET) - ScrollOffset) -
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

	int X = *(int*)(This + CHAT_WINDOW_POS_X_OFFSET) + 10 +
		GetTextWidth(NamePrefix) + GetTextWidth(Prefix);

	int Y = *(int*)(This + CHAT_WINDOW_POS_Y_OFFSET) + (LayoutLineIndex * -13) - 16;

	DWORD OriginalColor = SetTextColor;
	DWORD OriginalBackgroundColor = SetBackgroundTextColor;
	SetTextColor = CItemLinkChat::GetItemLinkTextColor(&FoundLink->item);
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

int CItemLinkChat::RenderTextHook(
	int X,
	int Y,
	const char* Text,
	int Width,
	int Sort,
	SIZE* TextSize)
{
	if (gItemLinkChat.m_LastRenderFrame != ChatLogRenderFrame)
	{
		gItemLinkChat.SynchronizeMetadata(0);

		for (int n = 0; n < CItemLinkChat::MAX_CHAT_LINES; n++)
		{
			gItemLinkChat.m_Links[n].hitX = 0;
			gItemLinkChat.m_Links[n].hitY = 0;
			gItemLinkChat.m_Links[n].hitWidth = 0;
			gItemLinkChat.m_Links[n].hitHeight = 0;
			gItemLinkChat.m_Links[n].renderedAt = 0;
		}

		gItemLinkChat.m_LastRenderFrame = ChatLogRenderFrame;
	}

	int Index = ChatLogViewStart + (Y / 13);

	if (Text == NULL || Index < 0 || Index >= CItemLinkChat::MAX_CHAT_LINES ||
		gItemLinkChat.m_Links[Index].active == false)
	{
		return RenderChatText(
			X,
			Y,
			Text,
			Width,
			Sort,
			TextSize);
	}

	CHAT_LINK* Link = &gItemLinkChat.m_Links[Index];

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

	SetTextColor = CItemLinkChat::GetItemLinkTextColor(
		&gItemLinkChat.m_Links[Index].item);
	SetBackgroundTextColor = ITEM_LINK_BACKGROUND_COLOR;

	int Result = RenderChatText(
		CurrentX,
		Y,
		Token,
		0,
		Sort,
		(Suffix[0] == 0) ? TextSize : NULL);

	Link->hitX = CurrentX;

	Link->hitY = Y;

	Link->hitWidth = GetTextWidth(Token);

	Link->hitHeight = max(GetTextHeight(Token), 13);

	Link->renderedAt = ChatLogRenderFrame;

	CurrentX += Link->hitWidth;

	SetBackgroundTextColor = OriginalBackgroundColor;
	SetTextColor = OriginalColor;

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

bool CItemLinkChat::IsInsideHitBox(const CHAT_LINK* Link)
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

CItemLinkChat::CHAT_LINK* CItemLinkChat::FindHoveredLink()
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

DWORD CItemLinkChat::GetItemLinkTextColor(const ITEM* Item)
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
