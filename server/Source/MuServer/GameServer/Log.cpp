#include "stdafx.h"
#include "Log.h"
#include <cstdio>
#include <filesystem>

CLog gLog;

CLog::CLog()
{
	this->m_count = 0;
}

CLog::~CLog()
{
	for (int n = 0; n < this->m_count; n++)
	{
		if (this->m_LogInfo[n].Active != 0 && this->m_LogInfo[n].File != nullptr)
		{
			std::fclose(this->m_LogInfo[n].File);
			this->m_LogInfo[n].File = nullptr;
		}
	}
}

void CLog::AddLog(int active, const char* directory)
{
	if (this->m_count < 0 || this->m_count >= MAX_LOG)
	{
		return;
	}

	LOG_INFO* lpInfo = &this->m_LogInfo[this->m_count++];

	lpInfo->Active = active;
	lpInfo->File = nullptr;

	strcpy_s(lpInfo->Directory, directory);

	if (lpInfo->Active != 0)
	{
		std::filesystem::create_directories(lpInfo->Directory);

		SYSTEMTIME time;

		GetLocalTime(&time);

		lpInfo->Day = time.wDay;

		lpInfo->Month = time.wMonth;

		lpInfo->Year = time.wYear;

		std::snprintf(lpInfo->Filename, sizeof(lpInfo->Filename), "./%s/%04d-%02d-%02d.txt", lpInfo->Directory, lpInfo->Year, lpInfo->Month, lpInfo->Day);

		lpInfo->File = std::fopen(lpInfo->Filename, "a");

		if (lpInfo->File == nullptr)
		{
			lpInfo->Active = 0;

			return;
		}
	}
}

void CLog::Output(eLogType type, const char* text, ...)
{
	if (type < 0 || type >= this->m_count)
	{
		return;
	}

	LOG_INFO* lpInfo = &this->m_LogInfo[type];

	if (lpInfo->Active == 0)
	{
		return;
	}

	SYSTEMTIME time;

	GetLocalTime(&time);

	if (time.wDay != lpInfo->Day || time.wMonth != lpInfo->Month || time.wYear != lpInfo->Year)
	{
		if (lpInfo->File != nullptr)
		{
			std::fclose(lpInfo->File);
			lpInfo->File = nullptr;
		}

		lpInfo->Day = time.wDay;

		lpInfo->Month = time.wMonth;

		lpInfo->Year = time.wYear;

		std::snprintf(lpInfo->Filename, sizeof(lpInfo->Filename), "./%s/%04d-%02d-%02d.txt", lpInfo->Directory, lpInfo->Year, lpInfo->Month, lpInfo->Day);

		lpInfo->File = std::fopen(lpInfo->Filename, "a");

		if (lpInfo->File == nullptr)
		{
			lpInfo->Active = 0;

			return;
		}
	}

	char temp[1024] = { 0 };

	va_list arg;

	va_start(arg, text);

	vsprintf_s(temp, text, arg);

	va_end(arg);

	char buff[1024] = { 0 };

	std::snprintf(buff, sizeof(buff), "%02d:%02d:%02d %s\r\n", time.wHour, time.wMinute, time.wSecond, temp);

	if (lpInfo->File != nullptr)
	{
		std::fputs(buff, lpInfo->File);
		std::fflush(lpInfo->File);
	}
}

