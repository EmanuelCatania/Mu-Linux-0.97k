#include "stdafx.h"
#include "ItemStack.h"
#include "ItemManager.h"
#include "ReadScript.h"
#include "Util.h"

CItemStack gItemStack;

CItemStack::CItemStack()
{
	this->m_ItemStackInfo.clear();
}

CItemStack::~CItemStack()
{

}

void CItemStack::SetInfo(const std::vector<ItemExtraDef>& items)
{
	this->m_ItemStackInfo.clear();

	for (const ItemExtraDef& d : items)
	{
		for (const ItemExtraStack& s : d.Stack)
		{
			ITEM_STACK_INFO info;

			info.ItemIndex = SafeGetItem(GET_ITEM(d.Section, d.Index));

			info.Level = s.Level;

			info.MaxStack = s.MaxStack;

			info.CreateItemIndex = (s.CreateSection < 0) ? -1 : SafeGetItem(GET_ITEM(s.CreateSection, s.CreateIndex));

			this->m_ItemStackInfo.push_back(info);
		}
	}
}

void CItemStack::Load(const char* path)
{
	CReadScript* lpReadScript = new CReadScript;

	if (lpReadScript == NULL)
	{
		ErrorMessageBox(READ_SCRIPT_ALLOC_ERROR, path);

		return;
	}

	if (!lpReadScript->Load(path))
	{
		ErrorMessageBox(READ_SCRIPT_FILE_ERROR, path);

		delete lpReadScript;

		return;
	}

	this->m_ItemStackInfo.clear();

	try
	{
		eTokenResult token;

		while (true)
		{
			token = lpReadScript->GetToken();

			if (token == TOKEN_END || token == TOKEN_END_SECTION)
			{
				break;
			}

			ITEM_STACK_INFO info;

			info.ItemIndex = lpReadScript->GetNumber();

			info.ItemIndex = SafeGetItem(GET_ITEM(info.ItemIndex, lpReadScript->GetAsNumber()));

			info.Level = lpReadScript->GetAsNumber();

			info.MaxStack = lpReadScript->GetAsNumber();

			info.CreateItemIndex = lpReadScript->GetAsNumber();

			if (info.CreateItemIndex != -1)
			{
				info.CreateItemIndex = SafeGetItem(GET_ITEM(info.CreateItemIndex, lpReadScript->GetAsNumber()));
			}

			this->m_ItemStackInfo.push_back(info);
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpReadScript->GetError());
	}

	delete lpReadScript;
}

int CItemStack::GetItemMaxStack(int index, int level)
{
	for (std::vector<ITEM_STACK_INFO>::iterator it = this->m_ItemStackInfo.begin(); it != this->m_ItemStackInfo.end(); it++)
	{
		if (it->ItemIndex != index)
		{
			continue;
		}

		if (it->Level != -1 && it->Level != level)
		{
			continue;
		}

		return it->MaxStack;
	}

	return 0;
}

int CItemStack::GetCreateItemIndex(int index)
{
	for (std::vector<ITEM_STACK_INFO>::iterator it = this->m_ItemStackInfo.begin(); it != this->m_ItemStackInfo.end(); it++)
	{
		if (it->ItemIndex != index)
		{
			continue;
		}

		return it->CreateItemIndex;
	}

	return -1;
}

void CItemStack::BuildCatalog(std::vector<ITEM_STACK>& out) const
{
	out.clear();

	for (const ITEM_STACK_INFO& it : this->m_ItemStackInfo)
	{
		ITEM_STACK info;

		info.ItemIndex = it.ItemIndex;

		info.Level = it.Level;

		info.MaxStack = it.MaxStack;

		out.push_back(info);
	}
}
