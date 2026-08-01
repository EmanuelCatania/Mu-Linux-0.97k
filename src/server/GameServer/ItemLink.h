#pragma once

#include "ProtocolDefines.h"
#include "User.h"

struct PMSG_ITEM_LINK_RECV
{
	PSBMSG_HEAD header; // C1:F3:E7
	BYTE itemType[2];
	BYTE itemLevel;
	BYTE slot;
	BYTE linkStart;
	BYTE linkLength;
	char message[60];
};

struct PMSG_ITEM_LINK_SEND
{
	PSBMSG_HEAD header; // C1:F3:E7
	char name[11];
	char message[60];
	BYTE linkStart;
	BYTE linkLength;
	BYTE ItemInfo[4];
};

static_assert(sizeof(PMSG_ITEM_LINK_RECV) == 70, "Invalid item link receive packet size");

static_assert(sizeof(PMSG_ITEM_LINK_SEND) == 81, "Invalid item link send packet size");

struct PMSG_ITEM_POST_LINK_RECV
{
	PSBMSG_HEAD header; // C1:F3:E8
	BYTE itemType[2];
	BYTE itemLevel;
	BYTE slot;
	BYTE linkStart;
	BYTE linkLength;
	char message[60];
};

static_assert(sizeof(PMSG_ITEM_POST_LINK_RECV) == 70, "Invalid item post link receive packet size");

struct PMSG_ITEM_POST_LINK_SEND
{
	PSBMSG_HEAD header; // C1:F3:E8
	char name[11];
	char message[60];
	BYTE linkStart;
	BYTE linkLength;
	BYTE style;
	BYTE ItemInfo[4];
};

static_assert(sizeof(PMSG_ITEM_POST_LINK_SEND) == 82, "Invalid item post link send packet size");

struct SDHP_GLOBAL_POST_LINK_RECV
{
	PSBMSG_HEAD header; // C1:05:06
	BYTE type;
	char name[11];
	char message[60];
	char serverName[60];
	BYTE linkStart;
	BYTE linkLength;
	BYTE ItemInfo[4];
};

struct SDHP_GLOBAL_POST_LINK_SEND
{
	PSBMSG_HEAD header; // C1:05:06
	BYTE type;
	char name[11];
	char message[60];
	char serverName[60];
	BYTE linkStart;
	BYTE linkLength;
	BYTE ItemInfo[4];
};

static_assert(sizeof(SDHP_GLOBAL_POST_LINK_RECV) == 142, "Invalid global post item link receive packet size");

static_assert(sizeof(SDHP_GLOBAL_POST_LINK_SEND) == 142, "Invalid global post item link send packet size");

class CItemLink
{
public:

	void CGItemLinkRecv(PMSG_ITEM_LINK_RECV* lpMsg, int size, int aIndex);

	void CGItemPostLinkRecv(PMSG_ITEM_POST_LINK_RECV* lpMsg, int size, int aIndex);

	bool ExecutePostLink(
		LPOBJ lpObj,
		char* message,
		BYTE linkStart,
		BYTE linkLength,
		const BYTE* itemInfo,
		BYTE type);

	void DGGlobalPostLinkRecv(SDHP_GLOBAL_POST_LINK_SEND* lpMsg, int size);

private:

	bool BuildCanonicalMessage(
		PMSG_ITEM_LINK_RECV* lpMsg,
		char* message,
		int messageSize,
		BYTE* linkStart,
		BYTE* linkLength,
		BYTE* itemInfo,
		int aIndex);

	void SendToClient(
		int aIndex,
		const char* name,
		const char* message,
		BYTE linkStart,
		BYTE linkLength,
		const BYTE* itemInfo);

	static bool BuildPostLinkMessage(
		BYTE type,
		char* serverName,
		char* text,
		BYTE linkStart,
		BYTE linkLength,
		int language,
		char* output,
		int outputSize,
		BYTE* outputLinkStart);

	void GCPostLink(
		BYTE type,
		char* name,
		char* serverName,
		char* text,
		BYTE linkStart,
		BYTE linkLength,
		const BYTE* itemInfo);

	void GDPostLink(
		BYTE type,
		char* name,
		char* message,
		BYTE linkStart,
		BYTE linkLength,
		const BYTE* itemInfo);

};

extern CItemLink gItemLink;
