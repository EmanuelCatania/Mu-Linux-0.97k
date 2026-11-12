#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

#define GAMESERVER_EXTRA 1

#define MAX_LANGUAGE 3

#define ENCRYPT_STATE 1

#include "../Common/Platform.h"
#include "../Common/TextField.h"
#include "../Common/ProtocolVersion.h"
#include "../Common/RejectReason.h"
#include "../Common/TextName.h"

// System includes
#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <random>
#include <time.h>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <dbghelp.h>
#ifdef _MSC_VER
#include <atltime.h>
#else
#include "../Common/PortableTime.h"
#endif
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"dbghelp.lib")
#endif

// General includes
#include "Console.h"

#define DWORD_MAX 4294967295
