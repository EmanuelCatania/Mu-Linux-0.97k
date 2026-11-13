#pragma once

// Tipos de log de este server. Lo incluye Common/Log.h.

#define MAX_LOG 7

enum eLogType
{
	LOG_GENERAL = 0,
	LOG_CHAT = 1,
	LOG_COMMAND = 2,
	LOG_TRADE = 3,
	LOG_CONNECT = 4,
	LOG_HACK = 5,
	LOG_CHAOS_MIX = 6,
};
