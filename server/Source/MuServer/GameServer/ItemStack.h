#pragma once

#include "ProtocolDefines.h"
#include "ItemExtras.h"

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

// Registro de la sección F3:F8 del catálogo (antes, lista F3:E3 del DLL).
struct ITEM_STACK
{
	int ItemIndex;
	int Level;
	int MaxStack;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct ITEM_STACK_INFO
{
	int ItemIndex;
	int Level;
	int MaxStack;
	int CreateItemIndex;
};

class CItemStack
{
public:

	CItemStack();

	~CItemStack();

	void Load(const char* path);

	// Fase 5: el apilado de Data/Items/NN-nombre.toml (`stack`).
	void SetInfo(const std::vector<ItemExtraDef>& items);

	int GetItemMaxStack(int index, int level);

	int GetCreateItemIndex(int index);

	// Las filas que viajan en el catálogo (sección F3:F8).
	void BuildCatalog(std::vector<ITEM_STACK>& out) const;

private:

	std::vector<ITEM_STACK_INFO> m_ItemStackInfo;
};

extern CItemStack gItemStack;
