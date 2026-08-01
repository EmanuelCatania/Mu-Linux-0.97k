#pragma once

#include <windows.h>
#include "Util.h"

class CItemLinkChat
{
public:

	enum
	{
		MAX_CHAT_LINES = 120
	};

	CItemLinkChat();

	bool Init();

	int GetIdentityCount() const;

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

	void AttachUiLink(
		const char* Name,
		const char* Message,
		BYTE LinkStart,
		BYTE LinkLength,
		const ITEM* Item);

	bool GetHoveredItem(ITEM* Item);

private:

	enum
	{
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

	static int RenderTextHook(
		int X,
		int Y,
		const char* Text,
		int Width,
		int Sort,
		SIZE* TextSize);

	static int __fastcall RenderLineHook(
		int This,
		int Unused,
		int LineIndex);

	static DWORD GetItemLinkTextColor(const ITEM* Item);

	static bool IdentityEquals(
		const CHAT_IDENTITY* Left,
		const CHAT_IDENTITY* Right);

	void ClearLink(CHAT_LINK* Link);

	void ReadIdentity(
		int Index,
		CHAT_IDENTITY* Identity);

	void RenderUiLink(
		int This,
		int LineIndex);

	CHAT_LINK* FindHoveredLink();

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
	RELATIVE_CALL_HOOK m_TextHook;
	VTABLE_HOOK m_LineHook;
};

extern CItemLinkChat gItemLinkChat;
