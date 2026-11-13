#include "stdafx.h"
#include "Util.h"
#include <cstdio>

void ErrorMessageBox(const char* message, ...)
{
	char buff[256];

	memset(buff, 0, sizeof(buff));

	va_list arg;

	va_start(arg, message);

	vsprintf_s(buff, message, arg);

	va_end(arg);

#ifdef _WIN32
	MessageBox(0, buff, "Error", MB_OK | MB_ICONERROR);
#else
	fprintf(stderr, "Error: %s\n", buff);
#endif

	ExitProcess(0);
}

void LogAdd(eLogColor color, const char* text, ...)
{
	tm today;

	time_t ltime;

	time(&ltime);

	if (localtime_s(&today, &ltime) != 0)
	{
		return;
	}

	char time[32];

	if (asctime_s(time, sizeof(time), &today) != 0)
	{
		return;
	}

	char temp[1024];

	va_list arg;

	va_start(arg, text);

	vsprintf_s(temp, text, arg);

	va_end(arg);

	char log[1024];

	wsprintf(log, "%.8s %s", &time[11], temp);

	gServerDisplayer.LogAddText(color, log, strlen(log));
}
