#pragma once

#include "ServerDisplayer.h"
#include "UtilCommon.h"
#include "ServerManager.h"

void ConsoleProtocolLog(int type, BYTE* lpMsg, int size);

bool GetCharacterSlot(char CharacterName[5][CHARACTER_NAME_BUFFER], const char* name, BYTE* slot);

int GetFreeServerIndex();

int SearchFreeServerIndex(int* index, int MinIndex, int MaxIndex, DWORD MinTime);

WORD GetServerCodeByName(char* name);

CServerManager* FindServerByCode(WORD ServerCode);

extern int gServerCount;

extern int EnableSpecialCharacters;
