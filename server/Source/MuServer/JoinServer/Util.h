#pragma once

#include "ServerDisplayer.h"
#include "UtilCommon.h"
#include "ServerManager.h"

void ConsoleProtocolLog(int type, BYTE* lpMsg, int size);

bool CheckTextSyntax(char* text, int size);

LONG CheckAccountCaseSensitive(int value);

int GetFreeServerIndex();

int SearchFreeServerIndex(int* index, int MinIndex, int MaxIndex, DWORD MinTime);

CServerManager* FindServerByCode(WORD ServerCode);

DWORD MakeAccountKey(char* account);

extern int gServerCount;
