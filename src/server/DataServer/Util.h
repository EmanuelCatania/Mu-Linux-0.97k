#pragma once

#include "ServerDisplayer.h"
#include "ServerManager.h"

void ErrorMessageBox(const char* message, ...);

void LogAdd(eLogColor color, const char* text, ...);

void ConsoleProtocolLog(int type, BYTE* lpMsg, int size);

bool GetCharacterSlot(char CharacterName[5][11], const char* name, BYTE* slot);

bool CheckTextSyntax(char* text, int size);

bool CheckSpecialText(char* Text);

int GetFreeServerIndex();

int SearchFreeServerIndex(int* index, int MinIndex, int MaxIndex, DWORD MinTime);

WORD GetServerCodeByName(char* name);

CServerManager* FindServerByCode(WORD ServerCode);

extern int gServerCount;
