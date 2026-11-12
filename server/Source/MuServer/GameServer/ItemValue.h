#pragma once

#include "Item.h"
#include "ProtocolDefines.h"
#include "ItemExtras.h"

// También es el registro de la sección F3:F9 del catálogo (antes, lista F3:E4
// del DLL).
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

	// Las filas que viajan en el catálogo (sección F3:F9).
	const std::vector<ITEM_VALUE_INFO>& GetCatalog() const { return this->m_ItemValueInfo; }

private:

	std::vector<ITEM_VALUE_INFO> m_ItemValueInfo;
};

extern CItemValue gItemValue;
