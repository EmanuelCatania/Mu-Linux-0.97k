#include "stdafx.h"
#include "Util.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"

bool CheckTextSyntax(char* text, int size)
{
	for (int n = 0; n < size; n++)
	{
		if (text[n] == 0x20 || text[n] == 0x22 || text[n] == 0x27)
		{
			return false;
		}
	}

	return true;
}

LONG CheckAccountCaseSensitive(int value)
{
	if (CaseSensitive == 0)
	{
		return tolower(value);
	}
	else
	{
		return value;
	}
}

DWORD MakeAccountKey(char* account)
{
	int size = strlen(account);

	DWORD key = 0;

	for (int n = 0; n < size; n++)
	{
		key += account[n] + 17;
	}

	return ((key + ((10 - size) * 17)) % 256);
}

