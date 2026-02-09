#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

#include "../Common/Platform.h"

// System includes
#include <queue>
#include <map>
#include <time.h>
#include <iostream>
#include <string>
#include <sstream>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <dbghelp.h>
#include <sql.h>
#include <sqlext.h>
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"ws2_32.lib")
#endif

// General includes
#include "Console.h"

extern BOOL CaseSensitive;
extern int MD5Encryption;
extern char GlobalPassword[11];
