#include "stdafx.h"
#include "ItemOption.h"
#include "ItemManager.h"
#include "ReadScript.h"
#include "Util.h"

CItemOption gItemOption;

CItemOption::CItemOption()
{
	this->m_ItemOptionInfo.clear();
}

CItemOption::~CItemOption()
{

}

void CItemOption::Load(const char* path)
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

	this->m_ItemOptionInfo.clear();

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

			ITEM_OPTION_INFO info;

			info.Index = lpReadScript->GetNumber();

			info.OptionIndex = lpReadScript->GetAsNumber();

			info.OptionValue = lpReadScript->GetAsNumber();

			// El orden de evaluación de los argumentos no está definido (GCC los
			// evalúa al revés): sección e índice se leen por separado.
			int ItemMinIndexSection = lpReadScript->GetAsNumber();

			info.ItemMinIndex = SafeGetItem(GET_ITEM(ItemMinIndexSection, lpReadScript->GetAsNumber()));

			int ItemMaxIndexSection = lpReadScript->GetAsNumber();

			info.ItemMaxIndex = SafeGetItem(GET_ITEM(ItemMaxIndexSection, lpReadScript->GetAsNumber()));

			info.ItemSkillOption = lpReadScript->GetAsNumber();

			info.ItemLuckOption = lpReadScript->GetAsNumber();

			info.ItemAddOption = lpReadScript->GetAsNumber();

			info.ItemExceOption = lpReadScript->GetAsNumber();

			std::map<int, std::vector<ITEM_OPTION_INFO>>::iterator it = this->m_ItemOptionInfo.find(info.Index);

			if (it == this->m_ItemOptionInfo.end())
			{
				this->m_ItemOptionInfo.insert(std::pair<int, std::vector<ITEM_OPTION_INFO>>(info.Index, std::vector<ITEM_OPTION_INFO>(1, info)));
			}
			else
			{
				it->second.push_back(info);
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpReadScript->GetError());
	}

	delete lpReadScript;
}

bool CItemOption::GetItemOption(int index, CItem* lpItem)
{
	bool result = false;

	if (lpItem->m_OptionsQuant < MAX_SPECIAL_OPTION)
	{
		std::map<int, std::vector<ITEM_OPTION_INFO>>::iterator ItemOptionInfo = this->m_ItemOptionInfo.find(index);

		if (ItemOptionInfo != this->m_ItemOptionInfo.end())
		{
			for (std::vector<ITEM_OPTION_INFO>::iterator it = ItemOptionInfo->second.begin(); it != ItemOptionInfo->second.end(); it++)
			{
				if (it->Index != index)
				{
					continue;
				}

				if (it->ItemMinIndex != -1 && it->ItemMinIndex > lpItem->GetKind())
				{
					continue;
				}

				if (it->ItemMaxIndex != -1 && it->ItemMaxIndex < lpItem->GetKind())
				{
					continue;
				}

				if (it->ItemSkillOption != -1 && it->ItemSkillOption > lpItem->m_SkillOption)
				{
					continue;
				}

				if (it->ItemLuckOption != -1 && it->ItemLuckOption > lpItem->m_LuckOption)
				{
					continue;
				}

				if (it->ItemAddOption != -1 && it->ItemAddOption > lpItem->m_AddOption)
				{
					continue;
				}

				if (it->ItemExceOption != -1 && (lpItem->m_ExceOption & it->ItemExceOption) == 0)
				{
					continue;
				}

				result = true;

				lpItem->m_SpecialIndex[index] = it->OptionIndex;

				lpItem->m_SpecialValue[index] = it->OptionValue;
			}
		}
	}

	return result;
}

bool CItemOption::GetItemOption(int index, WORD ItemIndex, BYTE ItemSkill, BYTE ItemLuck, BYTE ItemOption, BYTE ItemExcellent, BYTE* pItemOption, BYTE* pItemValue)
{
	bool result = false;

	std::map<int, std::vector<ITEM_OPTION_INFO>>::iterator ItemOptionInfo = this->m_ItemOptionInfo.find(index);

	if (ItemOptionInfo != this->m_ItemOptionInfo.end())
	{
		for (std::vector<ITEM_OPTION_INFO>::iterator it = ItemOptionInfo->second.begin(); it != ItemOptionInfo->second.end(); it++)
		{
			if (it->Index != index)
			{
				continue;
			}

			if (it->ItemMinIndex != -1 && it->ItemMinIndex > gItemManager.GetBehavior(ItemIndex))
			{
				continue;
			}

			if (it->ItemMaxIndex != -1 && it->ItemMaxIndex < gItemManager.GetBehavior(ItemIndex))
			{
				continue;
			}

			if (it->ItemSkillOption != -1 && it->ItemSkillOption > ItemSkill)
			{
				continue;
			}

			if (it->ItemLuckOption != -1 && it->ItemLuckOption > ItemLuck)
			{
				continue;
			}

			if (it->ItemAddOption != -1 && it->ItemAddOption > ItemOption)
			{
				continue;
			}

			if (it->ItemExceOption != -1 && (ItemExcellent & it->ItemExceOption) == 0)
			{
				continue;
			}

			result = true;

			(*pItemOption) = (BYTE)it->OptionIndex;

			(*pItemValue) = (BYTE)it->OptionValue;
		}
	}

	return result;
}

void CItemOption::CalcItemCommonOption(LPOBJ lpObj)
{
	for (int n = 0; n < INVENTORY_WEAR_SIZE; n++)
	{
		if (lpObj->Inventory[n].IsItem() != 0 && lpObj->Inventory[n].m_IsValidItem != 0 && lpObj->Inventory[n].m_Durability != 0)
		{
			this->InsertOption(lpObj, &lpObj->Inventory[n]);
		}
	}
}

void CItemOption::InsertOption(LPOBJ lpObj, CItem* lpItem)
{
	for (int n = 0; n < MAX_SPECIAL_OPTION; n++)
	{
		int index = lpItem->m_SpecialIndex[n];

		int value = lpItem->m_SpecialValue[n];

		switch (index)
		{
			case ITEM_OPTION_ADD_PHYSI_DAMAGE:
			{
				lpObj->PhysiDamageMinRight += value;

				lpObj->PhysiDamageMaxRight += value;

				lpObj->PhysiDamageMinLeft += value;

				lpObj->PhysiDamageMaxLeft += value;

				break;
			}

			case ITEM_OPTION_ADD_MAGIC_DAMAGE:
			{
				lpObj->MagicDamageMin += value;

				lpObj->MagicDamageMax += value;

				break;
			}

			case ITEM_OPTION_ADD_DEFENSE_SUCCESS_RATE:
			{
				lpObj->DefenseSuccessRate += value;

				break;
			}

			case ITEM_OPTION_ADD_DEFENSE:
			{
				lpObj->Defense += value;

				break;
			}

			case ITEM_OPTION_ADD_CRITICAL_DAMAGE_RATE:
			{
				lpObj->CriticalDamageRate += value;

				break;
			}

			case ITEM_OPTION_ADD_HP_RECOVERY_RATE:
			{
				lpObj->HPRecoveryRate += value;

				break;
			}

			case ITEM_OPTION_ADD_MONEY_AMOUNT_DROP_RATE:
			{
				lpObj->MoneyAmountDropRate += value;

				break;
			}

			case ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE:
			{
				lpObj->DefenseSuccessRate += (lpObj->DefenseSuccessRate * value) / 100;

				break;
			}

			case ITEM_OPTION_ADD_DAMAGE_REFLECT:
			{
				lpObj->DamageReflect += value;

				break;
			}

			case ITEM_OPTION_ADD_DAMAGE_REDUCTION:
			{
				lpObj->DamageReduction[DAMAGE_REDUCTION_EXCELLENT_ITEM] += value;

				break;
			}

			case ITEM_OPTION_MUL_MP:
			{
				lpObj->AddMana += (int)((lpObj->MaxMana * value) / 100);

				break;
			}

			case ITEM_OPTION_MUL_HP:
			{
				lpObj->AddLife += (int)((lpObj->MaxLife * value) / 100);

				break;
			}

			case ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE:
			{
				lpObj->ExcellentDamageRate += value;

				lpObj->ExcellentDamage += (value * 2);

				break;
			}

			case ITEM_OPTION_ADD_PHYSI_DAMAGE_BY_LEVEL:
			{
				int addVal = (value > 0) ? ((lpObj->Level) / value) : 0;

				addVal = ((addVal < 1 && lpObj->Level > 0) ? 1 : addVal);

				lpObj->PhysiDamageMinRight += addVal;

				lpObj->PhysiDamageMaxRight += addVal;

				lpObj->PhysiDamageMinLeft += addVal;

				lpObj->PhysiDamageMaxLeft += addVal;

				break;
			}

			case ITEM_OPTION_MUL_PHYSI_DAMAGE:
			{
				int addMinR = (lpObj->PhysiDamageMinRight * value) / 100;

				int addMaxR = (lpObj->PhysiDamageMaxRight * value) / 100;

				int addMinL = (lpObj->PhysiDamageMinLeft * value) / 100;

				int addMaxL = (lpObj->PhysiDamageMaxLeft * value) / 100;

				lpObj->PhysiDamageMinRight += ((addMinR < 1 && value > 0 && lpObj->PhysiDamageMinRight > 0) ? 1 : addMinR);

				lpObj->PhysiDamageMaxRight += ((addMaxR < 1 && value > 0 && lpObj->PhysiDamageMaxRight > 0) ? 1 : addMaxR);

				lpObj->PhysiDamageMinLeft += ((addMinL < 1 && value > 0 && lpObj->PhysiDamageMinLeft > 0) ? 1 : addMinL);

				lpObj->PhysiDamageMaxLeft += ((addMaxL < 1 && value > 0 && lpObj->PhysiDamageMaxLeft > 0) ? 1 : addMaxL);

				break;
			}

			case ITEM_OPTION_ADD_MAGIC_DAMAGE_BY_LEVEL:
			{
				int addVal = (value > 0) ? ((lpObj->Level) / value) : 0;

				addVal = ((addVal < 1 && lpObj->Level > 0) ? 1 : addVal);

				lpObj->MagicDamageMin += addVal;

				lpObj->MagicDamageMax += addVal;

				break;
			}

			case ITEM_OPTION_MUL_MAGIC_DAMAGE:
			{
				int addMin = (lpObj->MagicDamageMin * value) / 100;

				int addMax = (lpObj->MagicDamageMax * value) / 100;

				lpObj->MagicDamageMin += ((addMin < 1 && value > 0 && lpObj->MagicDamageMin > 0) ? 1 : addMin);

				lpObj->MagicDamageMax += ((addMax < 1 && value > 0 && lpObj->MagicDamageMax > 0) ? 1 : addMax);

				break;
			}

			case ITEM_OPTION_ADD_SPEED:
			{
				lpObj->PhysiSpeed += value;

				lpObj->MagicSpeed += value;

				break;
			}

			case ITEM_OPTION_ADD_HUNT_HP:
			{
				lpObj->HuntHP += value;

				break;
			}

			case ITEM_OPTION_ADD_HUNT_MP:
			{
				lpObj->HuntMP += value;

				break;
			}

			case ITEM_OPTION_ADD_WING_HP:
			{
				lpObj->AddLife += 50 + (value * lpItem->m_Level);

				break;
			}

			case ITEM_OPTION_ADD_WING_MP:
			{
				lpObj->AddMana += 50 + (value * lpItem->m_Level);

				break;
			}

			case ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE:
			{
				lpObj->IgnoreDefenseRate += value;

				break;
			}

			case ITEM_OPTION_ADD_BP:
			{
				lpObj->AddBP += value;

				break;
			}

			case ITEM_OPTION_MUL_BP:
			{
				lpObj->AddBP += (int)((lpObj->MaxBP * value) / 100);

				break;
			}
		}
	}
}
