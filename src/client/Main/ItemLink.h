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

	enum
	{
		MAX_CHAT_LINES = 120,
		CHAT_ENTRY_SIZE = 0x118,
		CHAT_NAME_SIZE = 11,
		CHAT_TEXT_SIZE = 257
	};

	struct CHAT_IDENTITY
	{
		char name[CHAT_NAME_SIZE];
		char message[CHAT_TEXT_SIZE];
		BYTE type;
	};

	struct CHAT_LINK
	{
		struct HIT_BOX
		{
			int x;
			int y;
			int width;
			int height;
			DWORD owner;
			DWORD frame;
		};

		bool active;
		CHAT_IDENTITY identity;
		BYTE linkStart;
		BYTE linkLength;
		ITEM item;
		int hitX;
		int hitY;
		int hitWidth;
		int hitHeight;
		DWORD renderedAt;
		DWORD lineNode;
		HIT_BOX uiHitBoxes[2];
	};

	static int RenderChatTextHook(
		int X,
		int Y,
		const char* Text,
		int Width,
		int Sort,
		SIZE* TextSize);

	static int __fastcall RenderChatLineHook(
		int This,
		int Unused,
		int LineIndex);

	static void __cdecl RenderTooltipTextListHook(
		void* TextListPointer,
		int Y,
		int TextCount,
		int Width,
		int Arg5,
		int Arg6);

	static void __fastcall RenderTooltipLineHook(
		void* This,
		void* Unused,
		int X,
		int Y,
		const char* Text,
		int Arg4,
		int Arg5,
		int Arg6,
		int Arg7,
		int Arg8);

	static void __cdecl RenderTooltipTopHook(float X, float Y, float Width, float Height);

	static void __cdecl RenderTooltipLeftHook(float X, float Y, float Width, float Height);

	static void __cdecl RenderTooltipRightHook(float X, float Y, float Width, float Height);

	static void __cdecl RenderTooltipBottomHook(float X, float Y, float Width, float Height);

	static void __cdecl RenderTooltipFillHook(float X, float Y, float Width, float Height);

	static DWORD GetItemLinkTextColor(const ITEM* Item);

	static int GetTooltipLineHeight();

	static int GetTooltipModelSize(const ITEM* Item);

	static bool IsTooltipSeparator(const char* Text);

	static int CalculateTooltipTextHeight(int TextCount);

	static void PrepareTooltipRectangle(float Y, float Height);

	static void RenderTooltipBorder(float X, float Y, float Width, float Height, int Part);

	bool IsSupportedClient();

	bool TryInsertPointedItem();

	bool GetPointedItem(ITEM* Item, BYTE* Slot, bool* Equipment);

	ITEM* GetItemBySlot(BYTE Slot);

	void ShowMessage(const char* Message);

	void ClearLink(CHAT_LINK* Link);

	void ReadIdentity(int Index, CHAT_IDENTITY* Identity);

	bool IdentityEquals(
		const CHAT_IDENTITY* Left,
		const CHAT_IDENTITY* Right);

	int SynchronizeMetadata(int MinimumShift);

	int GetMessageLineCount(
		const char* Name,
		const char* Message);

	void AttachReceivedLink(
		const char* Message,
		BYTE LinkStart,
		BYTE LinkLength,
		const ITEM* Item,
		int FirstNewLine,
		int NewCount);

	void DecodeItem(const BYTE* ItemInfo, ITEM* Item);

	void ReceiveItemLinkMessage(
		const char* Name,
		const char* Message,
		BYTE LinkStart,
		BYTE LinkLength,
		const BYTE* ItemInfo,
		BYTE Channel);

	CHAT_LINK* FindHoveredLink();

	void AttachUiLink(
		const char* Name,
		const char* Message,
		BYTE LinkStart,
		BYTE LinkLength,
		const ITEM* Item);

	void RenderUiLink(
		int This,
		int LineIndex);

	void RenderTooltipModel(const ITEM* Item);

	bool IsInsideHitBox(const CHAT_LINK* Link);

	CHAT_LINK m_Links[MAX_CHAT_LINES];

	CHAT_LINK m_UiLinks[MAX_CHAT_LINES];

	int m_UiLinkCursor;

	DWORD m_LastUiOwner;

	CHAT_IDENTITY m_Identities[MAX_CHAT_LINES];

	CHAT_LINK m_WorkLinks[MAX_CHAT_LINES];

	CHAT_IDENTITY m_WorkIdentities[MAX_CHAT_LINES];

	int m_IdentityCount;

	bool m_IdentityInitialized;

	DWORD m_LastRenderFrame;

	bool m_Pinned;

	bool m_ConsumeChatReturn;

	ITEM m_PinnedItem;

	int m_PinnedX;

	int m_PinnedY;

	struct TOOLTIP_LAYOUT
	{
		bool active;
		bool rectangleValid;
		bool titleCaptured;
		bool bodyCaptured;
		int extraHeight;
		int modelSize;
		int offsetY;
		int x;
		int y;
		int width;
		int height;
		int nativeTextHeight;
		int titleY;
		int bodyY;
		int bodyDirection;
	};

	static TOOLTIP_LAYOUT m_TooltipLayout;

	bool m_TooltipHooksInstalled;
};

extern CItemLink gItemLink;
