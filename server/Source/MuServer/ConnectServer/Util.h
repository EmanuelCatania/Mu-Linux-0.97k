#pragma once

#include "ServerDisplayer.h"
#include "UtilCommon.h"

void ConsoleProtocolLog(int type, BYTE* lpMsg, int size);

void ConnectServerTimeoutProc();

int GetFreeClientIndex();

int SearchFreeClientIndex(int* index, int MinIndex, int MaxIndex, DWORD MinTime);

extern int gClientCount;
