#pragma once

#include "Item.h"
#include "ProtocolDefines.h"
#include "ItemExtras.h"

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_ITEM_VALUE_LIST_SEND
{
	PSWMSG_HEAD header; // C2:F3:E4
	BYTE count;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct ITEM_VALUE_INFO
{
	int Index;
	int Level;
	int BuyValue;
	int SellValue;
};

class CItemValue
{
public:

	CItemValue();

	~CItemValue();

	void Load(const char* path);

	// Fase 5: los precios de Data/Items/NN-nombre.toml (`value`).
	void SetInfo(const std::vector<ItemExtraDef>& items);

	bool GetItemBuyValue(CItem* lpItem, ULONGLONG* value);

	bool GetItemSellValue(CItem* lpItem, ULONGLONG* value);

	void GCItemValueListSend(int aIndex);

private:

	std::vector<ITEM_VALUE_INFO> m_ItemValueInfo;
};

extern CItemValue gItemValue;
