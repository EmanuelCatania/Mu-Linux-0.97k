#include "stdafx.h"
#include "ItemLink.h"
#include "Input.h"
#include "Protocol.h"
#include "ItemManager.h"
#include "ItemLinkChat.h"
#include "ItemLinkTooltip.h"

static const DWORD ITEM_LINK_EQUIPMENT_FLAG = 0x80000000;

static bool IsPostMessage(const char* Message)
{
	if (Message == NULL || _strnicmp(Message, "/post", 5) != 0)
	{
		return false;
	}

	return (Message[5] == 0 || Message[5] == ' ' || Message[5] == '\t');
}


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

CItemLink::CItemLink()
{
	this->m_ConsumeChatReturn = false;
}

bool CItemLink::Init()
{
	if (gItemLinkChat.Init() == false)
	{
		return false;
	}

#if defined(_DEBUG)
	ItemLinkDebug("[ItemLink] chat hooks installed");
#endif

	if (gItemLinkTooltip.Init() == false)
	{
#if defined(_DEBUG)
		ItemLinkDebug("[ItemLink] 3D tooltip layout unavailable; textual tooltip only");
#endif
	}

	return true;
}

bool CItemLink::HandleLeftButtonDown()
{
	ITEM HoveredItem;

	if (gItemLinkChat.GetHoveredItem(&HoveredItem) != false)
	{
		gItemLinkTooltip.Pin(&HoveredItem, MouseX, MouseY);

		return true;
	}

	gItemLinkTooltip.ClearPinned();

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
		gItemLinkTooltip.ClearPinned();

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

	for (int n = 0; n < INVENTORY_ITEM_SIZE; n++)
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
	for (int n = 0; n < INVENTORY_ITEM_SIZE; n++)
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
		PMSG_ITEM_POST_LINK_REQUEST pMsg;

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
		gItemLinkChat.SynchronizeMetadata(0);

		OldCount = gItemLinkChat.GetIdentityCount();

		AddedLines = gItemLinkChat.GetMessageLineCount(
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

	gItemLinkChat.AttachUiLink(
		Name,
		Message,
		LinkStart,
		LinkLength,
		&Item);

	if (StructuredPost == false)
	{
		int MinimumShift = max(
			0,
			(OldCount + AddedLines) - CItemLinkChat::MAX_CHAT_LINES);

		int FirstNewLine = gItemLinkChat.SynchronizeMetadata(
			MinimumShift);

		int NewCount = gItemLinkChat.GetIdentityCount();

		gItemLinkChat.AttachReceivedLink(
			Message,
			LinkStart,
			LinkLength,
			&Item,
			FirstNewLine,
			NewCount);
	}

}

void CItemLink::GCItemLinkRecv(PMSG_ITEM_LINK_RECV* lpMsg, DWORD size)
{
	int MessageLength = (lpMsg != NULL) ?
		(int)strnlen_s(lpMsg->message, sizeof(lpMsg->message)) : 0;

	if (lpMsg == NULL || size != sizeof(PMSG_ITEM_LINK_RECV) ||
		lpMsg->header.size != sizeof(PMSG_ITEM_LINK_RECV) ||
		lpMsg->name[sizeof(lpMsg->name) - 1] != 0 ||
		lpMsg->message[sizeof(lpMsg->message) - 1] != 0 ||
		lpMsg->linkLength == 0 ||
		lpMsg->linkStart + lpMsg->linkLength > MessageLength)
	{
		return;
	}

	char Name[sizeof(lpMsg->name)] = { 0 };
	char Message[sizeof(lpMsg->message)] = { 0 };

	memcpy(Name, lpMsg->name, sizeof(Name) - 1);
	memcpy(Message, lpMsg->message, sizeof(Message) - 1);

	this->ReceiveItemLinkMessage(
		Name,
		Message,
		lpMsg->linkStart,
		lpMsg->linkLength,
		lpMsg->ItemInfo,
		0xFF);
}

void CItemLink::GCItemPostLinkRecv(PMSG_ITEM_POST_LINK_RESPONSE* lpMsg, DWORD size)
{
	int MessageLength = (lpMsg != NULL) ?
		(int)strnlen_s(lpMsg->message, sizeof(lpMsg->message)) : 0;

	if (lpMsg == NULL ||
		size != sizeof(PMSG_ITEM_POST_LINK_RESPONSE) ||
		lpMsg->header.size != sizeof(PMSG_ITEM_POST_LINK_RESPONSE) ||
		lpMsg->style > 2 ||
		lpMsg->name[sizeof(lpMsg->name) - 1] != 0 ||
		lpMsg->message[sizeof(lpMsg->message) - 1] != 0 ||
		lpMsg->linkLength == 0 ||
		lpMsg->linkStart + lpMsg->linkLength > MessageLength)
	{
		return;
	}

	char Name[sizeof(lpMsg->name)] = { 0 };
	char Message[sizeof(lpMsg->message)] = { 0 };

	memcpy(Name, lpMsg->name, sizeof(Name) - 1);
	memcpy(Message, lpMsg->message, sizeof(Message) - 1);

	this->ReceiveItemLinkMessage(
		Name,
		Message,
		lpMsg->linkStart,
		lpMsg->linkLength,
		lpMsg->ItemInfo,
		lpMsg->style);
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

void CItemLink::RenderTooltip()
{
	ITEM HoveredItem;
	const ITEM* Hovered = NULL;

	if (gItemLinkChat.GetHoveredItem(&HoveredItem) != false)
	{
		Hovered = &HoveredItem;
	}

	gItemLinkTooltip.Render(Hovered);
}
