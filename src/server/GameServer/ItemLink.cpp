#include "stdafx.h"
#include "ItemLink.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "Filter.h"
#include "GameMain.h"
#include "Guild.h"
#include "ItemManager.h"
#include "Log.h"
#include "Message.h"
#include "Notice.h"
#include "Party.h"
#include "Protocol.h"
#include "ServerInfo.h"
#include "Util.h"

CItemLink gItemLink;

void CItemLink::CGItemPostLinkRecv(
	PMSG_ITEM_POST_LINK_RECV* lpMsg,
	int size,
	int aIndex)
{
	if (lpMsg == NULL || size != sizeof(PMSG_ITEM_POST_LINK_RECV) ||
		lpMsg->header.size != sizeof(PMSG_ITEM_POST_LINK_RECV) ||
		lpMsg->message[sizeof(lpMsg->message) - 1] != 0 ||
		gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	char source[60] = { 0 };
	memcpy(source, lpMsg->message, sizeof(source) - 1);

	if (_strnicmp(source, "/post", 5) != 0 ||
		(source[5] != ' ' && source[5] != '\t'))
	{
		return;
	}

	int ArgumentStart = 5;

	while (source[ArgumentStart] == ' ' || source[ArgumentStart] == '\t')
	{
		ArgumentStart++;
	}

	int SourceLength = (int)strnlen(source, sizeof(source));

	if (source[ArgumentStart] == 0 ||
		lpMsg->linkLength == 0 ||
		lpMsg->linkStart < ArgumentStart ||
		lpMsg->linkStart + lpMsg->linkLength > SourceLength)
	{
		return;
	}

	PMSG_ITEM_LINK_RECV Argument;
	memset(&Argument, 0, sizeof(Argument));
	Argument.itemType[0] = lpMsg->itemType[0];
	Argument.itemType[1] = lpMsg->itemType[1];
	Argument.itemLevel = lpMsg->itemLevel;
	Argument.slot = lpMsg->slot;
	Argument.linkStart = (BYTE)(lpMsg->linkStart - ArgumentStart);
	Argument.linkLength = lpMsg->linkLength;
	memcpy(Argument.message, source + ArgumentStart, SourceLength - ArgumentStart);

	char message[60] = { 0 };
	BYTE linkStart = 0;
	BYTE linkLength = 0;
	BYTE itemInfo[MAX_ITEM_INFO] = { 0 };

	if (this->BuildCanonicalMessage(
		&Argument,
		message,
		sizeof(message),
		&linkStart,
		&linkLength,
		itemInfo,
		aIndex) == false)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->ChatLimitTime > 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, gMessage.GetTextMessage(63, lpObj->Lang), lpObj->ChatLimitTime);
		return;
	}

	if ((lpObj->Penalty & 2) != 0)
	{
		return;
	}

	gCommandManager.ManagementItemPost(
		lpObj,
		message,
		linkStart,
		linkLength,
		itemInfo);
}

void CItemLink::CGItemLinkRecv(
	PMSG_ITEM_LINK_RECV* lpMsg,
	int size,
	int aIndex)
{
	if (lpMsg == NULL || size != sizeof(PMSG_ITEM_LINK_RECV) ||
		lpMsg->header.size != sizeof(PMSG_ITEM_LINK_RECV) ||
		lpMsg->message[sizeof(lpMsg->message) - 1] != 0 ||
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

bool CItemLink::ExecutePostLink(
	LPOBJ lpObj,
	char* message,
	BYTE linkStart,
	BYTE linkLength,
	const BYTE* itemInfo,
	BYTE type)
{
	if (lpObj == NULL || message == NULL || itemInfo == NULL || type > 5)
	{
		return false;
	}

	BYTE style = (type <= 2) ? type : (BYTE)(type - 3);
	char preview[60] = { 0 };
	BYTE previewLinkStart = 0;

	if (this->BuildPostLinkMessage(
		style,
		gServerInfo.m_ServerName,
		message,
		linkStart,
		linkLength,
		lpObj->Lang,
		preview,
		sizeof(preview),
		&previewLinkStart) == false)
	{
		gNotice.GCNoticeSend(
			lpObj->Index,
			1,
			"A mensagem do post excede o limite.");

		return false;
	}

	if (type <= 2)
	{
		this->GCPostLink(
			type,
			lpObj->Name,
			gServerInfo.m_ServerName,
			message,
			linkStart,
			linkLength,
			itemInfo);
	}
	else
	{
		this->GDPostLink(
			(BYTE)(type - 3),
			lpObj->Name,
			message,
			linkStart,
			linkLength,
			itemInfo);
	}

	return true;
}

bool CItemLink::BuildPostLinkMessage(
	BYTE type,
	char* serverName,
	char* text,
	BYTE linkStart,
	BYTE linkLength,
	int language,
	char* output,
	int outputSize,
	BYTE* outputLinkStart)
{
	if (serverName == NULL || text == NULL || output == NULL ||
		outputLinkStart == NULL || outputSize < 60 || type > 2)
	{
		return false;
	}

	char prefix[256] = { 0 };
	const char* format = gMessage.GetTextMessage(69, language);

	if (format == NULL)
	{
		return false;
	}

	int prefixLength = snprintf(
		prefix,
		sizeof(prefix),
		format,
		serverName,
		"");

	if (prefixLength < 0 || prefixLength >= (int)sizeof(prefix))
	{
		return false;
	}

	int markerLength = (type == 0) ? 0 : 1;
	int argumentOffset = markerLength + (int)strlen(prefix);
	int textLength = (int)strlen(text);
	int finalLength = argumentOffset + textLength;

	if (linkLength == 0 || linkStart + linkLength > textLength ||
		finalLength <= 0 || finalLength >= outputSize ||
		finalLength > MAX_CHAT_MESSAGE_SIZE)
	{
		return false;
	}

	memset(output, 0, outputSize);

	if (type == 1)
	{
		output[0] = '~';
	}
	else if (type == 2)
	{
		output[0] = '@';
	}

	int writtenLength = snprintf(
		output + markerLength,
		outputSize - markerLength,
		format,
		serverName,
		text);

	if (writtenLength < 0 || writtenLength >= (outputSize - markerLength))
	{
		return false;
	}

	int actualLength = (int)strlen(output);

	if (actualLength != finalLength ||
		argumentOffset + linkStart + linkLength > actualLength)
	{
		return false;
	}

	*outputLinkStart = (BYTE)(argumentOffset + linkStart);
	return true;
}

void CItemLink::GCPostLink(
	BYTE type,
	char* name,
	char* serverName,
	char* text,
	BYTE linkStart,
	BYTE linkLength,
	const BYTE* itemInfo)
{
	if (name == NULL || serverName == NULL || text == NULL ||
		itemInfo == NULL || type > 2)
	{
		return;
	}

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		char message[60] = { 0 };
		BYTE finalLinkStart = 0;

		if (this->BuildPostLinkMessage(
			type,
			serverName,
			text,
			linkStart,
			linkLength,
			gObj[n].Lang,
			message,
			sizeof(message),
			&finalLinkStart) == false)
		{
			continue;
		}

		PMSG_ITEM_POST_LINK_SEND pMsg;
		pMsg.header.set(0xF3, 0xE8, sizeof(pMsg));
		memset(pMsg.name, 0, sizeof(pMsg.name));
		strncpy(pMsg.name, name, sizeof(pMsg.name) - 1);
		memset(pMsg.message, 0, sizeof(pMsg.message));
		memcpy(pMsg.message, message, sizeof(pMsg.message));
		pMsg.linkStart = finalLinkStart;
		pMsg.linkLength = linkLength;
		pMsg.style = type;
		memcpy(pMsg.ItemInfo, itemInfo, sizeof(pMsg.ItemInfo));

		DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
	}
}

void CItemLink::GDPostLink(
	BYTE type,
	char* name,
	char* message,
	BYTE linkStart,
	BYTE linkLength,
	const BYTE* itemInfo)
{
	if (type > 2 || name == NULL || message == NULL || itemInfo == NULL)
	{
		return;
	}

	SDHP_GLOBAL_POST_LINK_RECV pMsg;
	pMsg.header.set(0x05, 0x06, sizeof(pMsg));
	pMsg.type = type;
	memset(pMsg.name, 0, sizeof(pMsg.name));
	strncpy(pMsg.name, name, sizeof(pMsg.name) - 1);
	memset(pMsg.message, 0, sizeof(pMsg.message));
	strncpy(pMsg.message, message, sizeof(pMsg.message) - 1);
	memset(pMsg.serverName, 0, sizeof(pMsg.serverName));
	strncpy(pMsg.serverName, gServerInfo.m_ServerName, sizeof(pMsg.serverName) - 1);
	pMsg.linkStart = linkStart;
	pMsg.linkLength = linkLength;
	memcpy(pMsg.ItemInfo, itemInfo, sizeof(pMsg.ItemInfo));

	gDataServerConnection.DataSend((BYTE*)&pMsg, sizeof(pMsg));
}

void CItemLink::DGGlobalPostLinkRecv(
	SDHP_GLOBAL_POST_LINK_SEND* lpMsg,
	int size)
{
	if (lpMsg == NULL || size != sizeof(SDHP_GLOBAL_POST_LINK_SEND) ||
		lpMsg->header.size != sizeof(SDHP_GLOBAL_POST_LINK_SEND) ||
		lpMsg->type > 2 || lpMsg->linkLength == 0)
	{
		return;
	}

	char name[sizeof(lpMsg->name)] = { 0 };
	char postMessage[sizeof(lpMsg->message)] = { 0 };
	char serverName[sizeof(lpMsg->serverName)] = { 0 };

	memcpy(name, lpMsg->name, sizeof(name) - 1);
	memcpy(postMessage, lpMsg->message, sizeof(postMessage) - 1);
	memcpy(serverName, lpMsg->serverName, sizeof(serverName) - 1);

	if (lpMsg->message[sizeof(lpMsg->message) - 1] != 0 ||
		lpMsg->name[sizeof(lpMsg->name) - 1] != 0 ||
		lpMsg->serverName[sizeof(lpMsg->serverName) - 1] != 0 ||
		lpMsg->linkStart + lpMsg->linkLength > strlen(postMessage))
	{
		return;
	}

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) == 0)
		{
			continue;
		}

		char formattedMessage[60] = { 0 };
		BYTE finalLinkStart = 0;

		if (this->BuildPostLinkMessage(
			lpMsg->type,
			serverName,
			postMessage,
			lpMsg->linkStart,
			lpMsg->linkLength,
			gObj[n].Lang,
			formattedMessage,
			sizeof(formattedMessage),
			&finalLinkStart) == false)
		{
			continue;
		}

		PMSG_ITEM_POST_LINK_SEND pMsg;
		pMsg.header.set(0xF3, 0xE8, sizeof(pMsg));
		memset(pMsg.name, 0, sizeof(pMsg.name));
		strncpy(pMsg.name, name, sizeof(pMsg.name) - 1);
		memcpy(pMsg.message, formattedMessage, sizeof(pMsg.message));
		pMsg.linkStart = finalLinkStart;
		pMsg.linkLength = lpMsg->linkLength;
		pMsg.style = lpMsg->type;
		memcpy(pMsg.ItemInfo, lpMsg->ItemInfo, sizeof(pMsg.ItemInfo));

		DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
	}
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
	if (lpMsg == NULL || message == NULL || messageSize < 60 ||
		linkStart == NULL || linkLength == NULL || itemInfo == NULL ||
		INVENTORY_RANGE(lpMsg->slot) == 0)
	{
		return false;
	}

	char source[60];

	memcpy(source, lpMsg->message, sizeof(source));

	source[sizeof(source) - 1] = 0;

	int sourceLength = (int)strnlen(source, sizeof(source));

	int LinkStart = lpMsg->linkStart;
	int LinkLength = lpMsg->linkLength;

	if (sourceLength <= 0 || sourceLength >= sizeof(source) ||
		LinkLength <= 0 ||
		LinkStart >= sourceLength ||
		(LinkStart + LinkLength) > sourceLength ||
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

	int suffixStart = LinkStart + LinkLength;

	int finalLength = LinkStart +
		tokenLength +
		(sourceLength - suffixStart);

	if (finalLength <= 0 || finalLength >= messageSize)
	{
		return false;
	}

	memcpy(message, source, LinkStart);

	memcpy(message + LinkStart, token, tokenLength);

	memcpy(
		message + LinkStart + tokenLength,
		source + suffixStart,
		sourceLength - suffixStart);

	message[finalLength] = 0;

	*linkStart = (BYTE)LinkStart;

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
