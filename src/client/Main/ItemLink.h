#pragma once

#include "ProtocolDefines.h"

struct PMSG_ITEM_LINK_RECV
{
	PSBMSG_HEAD header; // C1:F3:E7
	char name[11];
	char message[60];
	BYTE linkStart;
	BYTE linkLength;
	BYTE ItemInfo[4];
};

struct PMSG_ITEM_LINK_SEND
{
	PSBMSG_HEAD header; // C1:F3:E7
	BYTE itemType[2];
	BYTE itemLevel;
	BYTE slot;
	BYTE linkStart;
	BYTE linkLength;
	char message[60];
};

static_assert(sizeof(PMSG_ITEM_LINK_RECV) == 81, "Invalid item link receive packet size");

static_assert(sizeof(PMSG_ITEM_LINK_SEND) == 70, "Invalid item link send packet size");

struct PMSG_ITEM_POST_LINK_REQUEST
{
	PSBMSG_HEAD header; // C1:F3:E8
	BYTE itemType[2];
	BYTE itemLevel;
	BYTE slot;
	BYTE linkStart;
	BYTE linkLength;
	char message[60];
};

struct PMSG_ITEM_POST_LINK_RESPONSE
{
	PSBMSG_HEAD header; // C1:F3:E8
	char name[11];
	char message[60];
	BYTE linkStart;
	BYTE linkLength;
	BYTE style;
	BYTE ItemInfo[4];
};

static_assert(sizeof(PMSG_ITEM_POST_LINK_REQUEST) == 70, "Invalid item post link request packet size");

static_assert(sizeof(PMSG_ITEM_POST_LINK_RESPONSE) == 82, "Invalid item post link response packet size");

class CItemLink
{
public:

	CItemLink();

	bool Init();

	bool HandleLeftButtonDown();

	bool HandleKeyDown(WPARAM wParam);

	bool HandleChar(WPARAM wParam);

	bool HandleOutgoingChat(BYTE* lpMsg, DWORD size);

	bool HandleMainChatSend();

	void GCItemLinkRecv(PMSG_ITEM_LINK_RECV* lpMsg, DWORD size);

	void GCItemPostLinkRecv(PMSG_ITEM_POST_LINK_RESPONSE* lpMsg, DWORD size);

	void RenderTooltip();

private:

	bool TryInsertPointedItem();

	bool GetPointedItem(ITEM* Item, BYTE* Slot, bool* Equipment);

	ITEM* GetItemBySlot(BYTE Slot);

	void ShowMessage(const char* Message);

	void DecodeItem(const BYTE* ItemInfo, ITEM* Item);

	void ReceiveItemLinkMessage(
		const char* Name,
		const char* Message,
		BYTE LinkStart,
		BYTE LinkLength,
		const BYTE* ItemInfo,
		BYTE Channel);

	bool m_ConsumeChatReturn;
};

extern CItemLink gItemLink;
