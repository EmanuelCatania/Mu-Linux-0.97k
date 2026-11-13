#include "stdafx.h"
#include "CharacterManager.h"
#include "ServerManager.h"
#include "Util.h"

int EnableSpecialCharacters = GetPrivateProfileInt("Syntax", "EnableSpecialCharacters", 0, ".\\DataServer.ini");

bool GetCharacterSlot(char CharacterName[5][CHARACTER_NAME_BUFFER], const char* name, BYTE* slot)
{
	for (int n = 0; n < 5; n++)
	{
		if (TextName::Equals(CharacterName[n], name))
		{
			(*slot) = n;

			return true;
		}
	}

	return false;
}

WORD GetServerCodeByName(char* name)
{
	CHARACTER_INFO CharacterInfo;

	if (gCharacterManager.GetCharacterInfo(&CharacterInfo, name) == false)
	{
		return 0xFFFF;
	}
	else
	{
		return CharacterInfo.GameServerCode;
	}
}

