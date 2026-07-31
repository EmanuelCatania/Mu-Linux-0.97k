#include "stdafx.h"
#include "ItemLink.h"
#include "Filter.h"
#include "Guild.h"
#include "ItemManager.h"
#include "Log.h"
#include "Message.h"
#include "Notice.h"
#include "Party.h"
#include "Protocol.h"
#include "Util.h"

CItemLink gItemLink;

void CItemLink::CGItemLinkRecv(
	PMSG_ITEM_LINK_RECV* lpMsg,
	int size,
	int aIndex)
{
	if (lpMsg == NULL || size != sizeof(PMSG_ITEM_LINK_RECV) ||
		lpMsg->header.size != sizeof(PMSG_ITEM_LINK_RECV) ||
		gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	char message[60] = { 0 };

	BYTE linkStart = 0;

	BYTE linkLength = 0;

	BYTE itemInfo[MAX_ITEM_INFO] = { 0 };

	if (this->BuildCanonicalMessage(
		lpMsg,
		message,
		sizeof(message),
		&linkStart,
		&linkLength,
		itemInfo,
		aIndex) == false)
	{
		return;
	}

	if (lpObj->ChatLimitTime > 0)
	{
		gNotice.GCNoticeSend(
			lpObj->Index,
			1,
			gMessage.GetTextMessage(63, lpObj->Lang),
			lpObj->ChatLimitTime);

		return;
	}

	if ((lpObj->Penalty & 2) != 0)
	{
		return;
	}

	gLog.Output(
		LOG_CHAT,
		"[ItemLink][%s][%s] - (Message: %s)",
		lpObj->Account,
		lpObj->Name,
		message);

	if (message[0] == '~')
	{
		if (OBJECT_RANGE(lpObj->PartyNumber) == 0)
		{
			return;
		}

		for (int n = 0; n < MAX_PARTY_USER; n++)
		{
			int index = gParty.m_PartyInfo[lpObj->PartyNumber].Index[n];

			if (OBJECT_RANGE(index) != 0)
			{
				this->SendToClient(
					index,
					lpObj->Name,
					message,
					linkStart,
					linkLength,
					itemInfo);
			}
		}

		return;
	}

	if (message[0] == '@')
	{
		if (lpObj->Guild != 0 && lpObj->Guild->Number != 0)
		{
			gGuild.GDGuildItemLinkSend(
				lpObj->Guild->Name,
				lpObj->Name,
				message,
				linkStart,
				linkLength,
				itemInfo);
		}

		return;
	}

	PMSG_ITEM_LINK_SEND pMsg;

	pMsg.header.set(0xF3, 0xE7, sizeof(pMsg));

	memset(pMsg.name, 0, sizeof(pMsg.name));

	memcpy(pMsg.name, lpObj->Name, sizeof(lpObj->Name));

	memcpy(pMsg.message, message, sizeof(pMsg.message));

	pMsg.linkStart = linkStart;

	pMsg.linkLength = linkLength;

	memcpy(pMsg.ItemInfo, itemInfo, sizeof(pMsg.ItemInfo));

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);

	MsgSendV2(lpObj, (BYTE*)&pMsg, pMsg.header.size);
}

bool CItemLink::BuildCanonicalMessage(
	PMSG_ITEM_LINK_RECV* lpMsg,
	char* message,
	int messageSize,
	BYTE* linkStart,
	BYTE* linkLength,
	BYTE* itemInfo,
	int aIndex)
{
	if (message == NULL || messageSize < 60 ||
		linkStart == NULL || linkLength == NULL || itemInfo == NULL ||
		INVENTORY_RANGE(lpMsg->slot) == 0)
	{
		return false;
	}

	char source[60];

	memcpy(source, lpMsg->message, sizeof(source));

	source[sizeof(source) - 1] = 0;

	int sourceLength = (int)strnlen(source, sizeof(source));

	if (sourceLength <= 0 || sourceLength >= sizeof(source) ||
		lpMsg->linkLength == 0 ||
		lpMsg->linkStart >= sourceLength ||
		(lpMsg->linkStart + lpMsg->linkLength) > sourceLength ||
		source[0] == '/' || source[0] == '#' ||
		(source[0] == '@' && source[1] == '>'))
	{
		return false;
	}

	LPOBJ lpObj = &gObj[aIndex];

	CItem* lpItem = &lpObj->Inventory[lpMsg->slot];

	int expectedType = MAKE_NUMBERW(
		lpMsg->itemType[0],
		lpMsg->itemType[1]);

	if (lpItem->IsItem() == false ||
		lpItem->m_Index != expectedType ||
		lpItem->m_Level != lpMsg->itemLevel)
	{
		return false;
	}

	ITEM_INFO info;

	if (gItemManager.GetInfo(lpItem->m_Index, &info) == false)
	{
		return false;
	}

	gFilter.CheckSyntax(source);

	char token[48];

	int tokenLength = 0;

	if (lpItem->m_Level > 0)
	{
		tokenLength = snprintf(
			token,
			sizeof(token),
			"[%s +%d]",
			info.Name,
			lpItem->m_Level);
	}
	else
	{
		tokenLength = snprintf(
			token,
			sizeof(token),
			"[%s]",
			info.Name);
	}

	if (tokenLength <= 0 || tokenLength >= sizeof(token))
	{
		return false;
	}

	int suffixStart = lpMsg->linkStart + lpMsg->linkLength;

	int finalLength = lpMsg->linkStart +
		tokenLength +
		(sourceLength - suffixStart);

	if (finalLength <= 0 || finalLength >= messageSize)
	{
		return false;
	}

	memcpy(message, source, lpMsg->linkStart);

	memcpy(message + lpMsg->linkStart, token, tokenLength);

	memcpy(
		message + lpMsg->linkStart + tokenLength,
		source + suffixStart,
		sourceLength - suffixStart);

	message[finalLength] = 0;

	*linkStart = lpMsg->linkStart;

	*linkLength = (BYTE)tokenLength;

	gItemManager.ItemByteConvert(itemInfo, *lpItem);

	return true;
}

void CItemLink::SendToClient(
	int aIndex,
	const char* name,
	const char* message,
	BYTE linkStart,
	BYTE linkLength,
	const BYTE* itemInfo)
{
	if (name == NULL || message == NULL || itemInfo == NULL)
	{
		return;
	}

	PMSG_ITEM_LINK_SEND pMsg;

	pMsg.header.set(0xF3, 0xE7, sizeof(pMsg));

	memset(pMsg.name, 0, sizeof(pMsg.name));

	strncpy(pMsg.name, name, sizeof(pMsg.name) - 1);

	memset(pMsg.message, 0, sizeof(pMsg.message));

	strncpy(pMsg.message, message, sizeof(pMsg.message) - 1);

	pMsg.linkStart = linkStart;

	pMsg.linkLength = linkLength;

	memcpy(pMsg.ItemInfo, itemInfo, sizeof(pMsg.ItemInfo));

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
}
