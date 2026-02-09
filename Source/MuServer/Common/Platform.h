#pragma once

#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

typedef unsigned __int64 QWORD;
#else

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstdarg>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <clocale>
#include <filesystem>
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <new>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

using BYTE = uint8_t;
using WORD = uint16_t;
using DWORD = uint32_t;
using QWORD = uint64_t;
using ULONGLONG = unsigned long long;
using LONG = int32_t;
using BOOL = int;
using UINT = unsigned int;
using ATOM = WORD;
using WPARAM = uintptr_t;
using LPARAM = intptr_t;
using LRESULT = intptr_t;

using SOCKET = int;
using HANDLE = void*;
using HINSTANCE = void*;
using HWND = void*;
using HBRUSH = void*;
using HFONT = void*;
using WSAEVENT = void*;
using PVOID = void*;
using BOOLEAN = unsigned char;
using GROUP = uint32_t;
using SOCKADDR = sockaddr;
using SOCKADDR_IN = sockaddr_in;
using __int64 = long long;

using WAITORTIMERCALLBACK = void (*)(PVOID, BOOLEAN);

struct WSAOVERLAPPED
{
	int dummy;
};

struct WSABUF
{
	unsigned long len;
	char* buf;
};

struct WSADATA
{
	unsigned short wVersion;
	unsigned short wHighVersion;
	char szDescription[257];
	char szSystemStatus[129];
	unsigned short iMaxSockets;
	unsigned short iMaxUdpDg;
	char* lpVendorInfo;
};

struct SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
};

struct RECT
{
	long left;
	long top;
	long right;
	long bottom;
};

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#endif

#ifndef FILE_ATTRIBUTE_ARCHIVE
#define FILE_ATTRIBUTE_ARCHIVE 0x20
#endif

#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL 0x80
#endif

#ifndef GENERIC_READ
#define GENERIC_READ 0x80000000
#endif

#ifndef GENERIC_WRITE
#define GENERIC_WRITE 0x40000000
#endif

#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ 0x00000001
#endif

#ifndef FILE_SHARE_WRITE
#define FILE_SHARE_WRITE 0x00000002
#endif

#ifndef OPEN_EXISTING
#define OPEN_EXISTING 3
#endif

#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS 2
#endif

#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif

using LPWSABUF = WSABUF*;
struct QOS
{
	int dummy;
};
using LPQOS = QOS*;

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

struct WIN32_FIND_DATA
{
	DWORD dwFileAttributes;
	char cFileName[MAX_PATH];
};

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef WINAPI
#define WINAPI
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef FAR
#define FAR
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#endif

#ifndef WSA_INVALID_EVENT
#define WSA_INVALID_EVENT nullptr
#endif

#ifndef WSA_IO_PENDING
#define WSA_IO_PENDING 997L
#endif

#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif

#ifndef WSAENOTSOCK
#define WSAENOTSOCK ENOTSOCK
#endif

#define MAKEWORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b) ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOBYTE(w) ((BYTE)((DWORD)(w) & 0xFF))
#define HIBYTE(w) ((BYTE)(((DWORD)(w) >> 8) & 0xFF))
#define LOWORD(l) ((WORD)((DWORD)(l) & 0xFFFF))
#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))

inline DWORD GetTickCount()
{
	using namespace std::chrono;
	return static_cast<DWORD>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

inline void Sleep(DWORD millis)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}

inline void GetLocalTime(SYSTEMTIME* st)
{
	if (!st)
	{
		return;
	}

	auto now = std::chrono::system_clock::now();
	auto tt = std::chrono::system_clock::to_time_t(now);
	std::tm local_tm {};
	localtime_r(&tt, &local_tm);

	st->wYear = static_cast<WORD>(local_tm.tm_year + 1900);
	st->wMonth = static_cast<WORD>(local_tm.tm_mon + 1);
	st->wDayOfWeek = static_cast<WORD>(local_tm.tm_wday);
	st->wDay = static_cast<WORD>(local_tm.tm_mday);
	st->wHour = static_cast<WORD>(local_tm.tm_hour);
	st->wMinute = static_cast<WORD>(local_tm.tm_min);
	st->wSecond = static_cast<WORD>(local_tm.tm_sec);
	st->wMilliseconds = 0;
}

inline DWORD GetLastError()
{
	return static_cast<DWORD>(errno);
}

inline int WSAGetLastError()
{
	return errno;
}

inline int WSAStartup(WORD, void*)
{
	return 0;
}

inline int WSACleanup()
{
	return 0;
}

inline int closesocket(SOCKET s)
{
	return close(s);
}

inline int localtime_s(std::tm* result, const time_t* timep)
{
	return (localtime_r(timep, result) == nullptr) ? 1 : 0;
}

inline int asctime_s(char* buffer, size_t bufferSize, const std::tm* timeptr)
{
	if (!buffer || bufferSize == 0)
	{
		return 1;
	}

	if (asctime_r(timeptr, buffer) == nullptr)
	{
		buffer[0] = '\0';
		return 1;
	}

	buffer[bufferSize - 1] = '\0';
	return 0;
}

inline void ExitProcess(int code)
{
	std::exit(code);
}

inline HANDLE CreateMutex(void*, BOOL, const char*)
{
	return reinterpret_cast<HANDLE>(1);
}

#ifndef ERROR_ALREADY_EXISTS
#define ERROR_ALREADY_EXISTS 183L
#endif

using HGLOBAL = void*;

#ifndef GPTR
#define GPTR 0
#endif

inline HGLOBAL GlobalAlloc(int, size_t bytes)
{
	unsigned char* mem = new (std::nothrow) unsigned char[bytes];
	if (!mem)
	{
		return nullptr;
	}

	std::memset(mem, 0, bytes);
	return mem;
}

inline void GlobalFree(HGLOBAL mem)
{
	delete[] static_cast<unsigned char*>(mem);
}

inline void SetRect(RECT* rc, int left, int top, int right, int bottom)
{
	if (!rc)
	{
		return;
	}

	rc->left = left;
	rc->top = top;
	rc->right = right;
	rc->bottom = bottom;
}

inline int _stricmp(const char* a, const char* b)
{
	return strcasecmp(a, b);
}

inline int _strcmpi(const char* a, const char* b)
{
	return strcasecmp(a, b);
}

inline int _strnicmp(const char* a, const char* b, size_t n)
{
	return strncasecmp(a, b, n);
}

inline long long _atoi64(const char* s)
{
	if (!s)
	{
		return 0;
	}

	return std::strtoll(s, nullptr, 10);
}

inline int _mbclen(const unsigned char* s)
{
	if (!s)
	{
		return 0;
	}

	return std::mblen(reinterpret_cast<const char*>(s), MB_CUR_MAX);
}

template <size_t N>
inline int strcpy_s(char (&dest)[N], const char* src)
{
	if (!src)
	{
		dest[0] = '\0';
		return 0;
	}

	std::strncpy(dest, src, N);
	dest[N - 1] = '\0';
	return 0;
}

inline int strcpy_s(char* dest, size_t destsz, const char* src)
{
	if (!dest || destsz == 0)
	{
		return -1;
	}

	if (!src)
	{
		dest[0] = '\0';
		return 0;
	}

	std::strncpy(dest, src, destsz);
	dest[destsz - 1] = '\0';
	return 0;
}

template <size_t N>
inline int strncpy_s(char (&dest)[N], size_t destsz, const char* src, size_t count)
{
	if (destsz == 0 || destsz > N)
	{
		return -1;
	}

	if (!src)
	{
		dest[0] = '\0';
		return 0;
	}

	std::strncpy(dest, src, count);
	dest[destsz - 1] = '\0';
	return 0;
}

inline int strncpy_s(char* dest, size_t destsz, const char* src, size_t count)
{
	if (!dest || destsz == 0)
	{
		return -1;
	}

	if (!src)
	{
		dest[0] = '\0';
		return 0;
	}

	std::strncpy(dest, src, count);
	dest[destsz - 1] = '\0';
	return 0;
}

template <size_t N>
inline int vsprintf_s(char (&dest)[N], const char* fmt, va_list arg)
{
	return vsnprintf(dest, N, fmt, arg);
}

template <size_t N>
inline int sprintf_s(char (&dest)[N], const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = vsnprintf(dest, N, fmt, args);
	va_end(args);
	return result;
}

inline std::string NormalizePath(const char* path)
{
	if (!path)
	{
		return std::string();
	}

	std::string normalized(path);
	for (char& ch : normalized)
	{
		if (ch == '\\')
		{
			ch = '/';
		}
	}

	return normalized;
}

inline std::string TrimIni(std::string value)
{
	auto is_space = [](unsigned char ch) { return std::isspace(ch); };

	while (!value.empty() && is_space(value.front()))
	{
		value.erase(value.begin());
	}

	while (!value.empty() && is_space(value.back()))
	{
		value.pop_back();
	}

	return value;
}

inline std::string ToLowerIni(std::string value)
{
	for (char& ch : value)
	{
		ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
	}

	return value;
}

inline bool ReadIniValue(const char* section, const char* key, const char* path, std::string& outValue)
{
	if (!section || !key || !path)
	{
		return false;
	}

	std::string normalizedPath = NormalizePath(path);
	std::ifstream file(normalizedPath);
	if (!file.is_open())
	{
		return false;
	}

	std::string currentSection;
	std::string line;
	const std::string sectionLower = ToLowerIni(section);
	const std::string keyLower = ToLowerIni(key);

	while (std::getline(file, line))
	{
		line = TrimIni(line);
		if (line.empty() || line[0] == ';' || line[0] == '#')
		{
			continue;
		}

		if (line.front() == '[' && line.back() == ']')
		{
			currentSection = ToLowerIni(TrimIni(line.substr(1, line.size() - 2)));
			continue;
		}

		if (currentSection != sectionLower)
		{
			continue;
		}

		auto pos = line.find('=');
		if (pos == std::string::npos)
		{
			continue;
		}

		std::string foundKey = ToLowerIni(TrimIni(line.substr(0, pos)));
		if (foundKey != keyLower)
		{
			continue;
		}

		outValue = TrimIni(line.substr(pos + 1));
		return true;
	}

	return false;
}

inline int GetPrivateProfileInt(const char* section, const char* key, int defaultValue, const char* path)
{
	std::string value;
	if (!ReadIniValue(section, key, path, value))
	{
		return defaultValue;
	}

	try
	{
		return std::stoi(value);
	}
	catch (...)
	{
		return defaultValue;
	}
}

inline unsigned long GetPrivateProfileString(const char* section, const char* key, const char* defaultValue, char* out,
	unsigned long outSize, const char* path)
{
	if (!out || outSize == 0)
	{
		return 0;
	}

	std::string value;
	if (!ReadIniValue(section, key, path, value))
	{
		value = defaultValue ? defaultValue : "";
	}

	std::strncpy(out, value.c_str(), outSize);
	out[outSize - 1] = '\0';
	return static_cast<unsigned long>(std::strlen(out));
}

class CTimeSpan
{
public:
	CTimeSpan(int days, int hours, int minutes, int seconds)
	{
		m_totalSeconds = static_cast<long long>(days) * 86400 +
			static_cast<long long>(hours) * 3600 +
			static_cast<long long>(minutes) * 60 +
			static_cast<long long>(seconds);
	}

	long long GetTotalSeconds() const
	{
		return m_totalSeconds;
	}

private:
	long long m_totalSeconds = 0;
};

class CTime
{
public:
	CTime() : m_time(0)
	{
	}

	CTime(int year, int month, int day, int hour, int minute, int second, int dst)
	{
		std::tm tm {};
		tm.tm_year = year - 1900;
		tm.tm_mon = month - 1;
		tm.tm_mday = day;
		tm.tm_hour = hour;
		tm.tm_min = minute;
		tm.tm_sec = second;
		tm.tm_isdst = dst;
		m_time = std::mktime(&tm);
	}

	static CTime GetTickCount()
	{
		CTime now;
		now.m_time = std::time(nullptr);
		return now;
	}

	int GetYear() const { return GetLocal().tm_year + 1900; }
	int GetMonth() const { return GetLocal().tm_mon + 1; }
	int GetDay() const { return GetLocal().tm_mday; }
	int GetHour() const { return GetLocal().tm_hour; }
	int GetMinute() const { return GetLocal().tm_min; }
	int GetSecond() const { return GetLocal().tm_sec; }
	int GetDayOfWeek() const { return GetLocal().tm_wday + 1; }
	std::time_t GetTime() const { return m_time; }

	CTime& operator+=(const CTimeSpan& span)
	{
		m_time += span.GetTotalSeconds();
		return *this;
	}

	friend CTime operator+(const CTime& time, const CTimeSpan& span)
	{
		CTime result = time;
		result += span;
		return result;
	}

	friend bool operator<(const CTime& lhs, const CTime& rhs)
	{
		return lhs.m_time < rhs.m_time;
	}

	friend bool operator>(const CTime& lhs, const CTime& rhs)
	{
		return lhs.m_time > rhs.m_time;
	}

	friend bool operator==(const CTime& lhs, const CTime& rhs)
	{
		return lhs.m_time == rhs.m_time;
	}

private:
	std::tm GetLocal() const
	{
		std::tm local_tm {};
		localtime_r(&m_time, &local_tm);
		return local_tm;
	}

	std::time_t m_time;
};

inline int wsprintf(char* buffer, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = std::vsprintf(buffer, fmt, args);
	va_end(args);
	return result;
}

template <size_t N>
inline int strcat_s(char (&dest)[N], const char* src)
{
	if (!src)
	{
		return 0;
	}

	size_t len = std::strlen(dest);
	if (len >= N)
	{
		return -1;
	}

	size_t remaining = N - len - 1;
	std::strncat(dest, src, remaining);
	dest[N - 1] = '\0';
	return 0;
}

inline int strcat_s(char* dest, size_t destsz, const char* src)
{
	if (!dest || destsz == 0)
	{
		return -1;
	}

	if (!src)
	{
		return 0;
	}

	size_t len = std::strlen(dest);
	if (len >= destsz)
	{
		return -1;
	}

	size_t remaining = destsz - len - 1;
	std::strncat(dest, src, remaining);
	dest[destsz - 1] = '\0';
	return 0;
}

inline int fopen_s(FILE** file, const char* path, const char* mode)
{
	if (!file)
	{
		return EINVAL;
	}

	std::string normalizedPath = NormalizePath(path);
	*file = std::fopen(normalizedPath.c_str(), mode);
	return (*file != nullptr) ? 0 : errno;
}

inline HANDLE CreateFile(const char* path, DWORD desiredAccess, DWORD, void*, DWORD creationDisposition, DWORD, HANDLE)
{
	std::string normalizedPath = NormalizePath(path);
	const char* mode = "rb";

	if ((desiredAccess & GENERIC_WRITE) != 0)
	{
		if (creationDisposition == CREATE_ALWAYS)
		{
			mode = "wb";
		}
		else
		{
			mode = "rb+";
		}
	}

	FILE* file = std::fopen(normalizedPath.c_str(), mode);
	if (!file)
	{
		return INVALID_HANDLE_VALUE;
	}

	return reinterpret_cast<HANDLE>(file);
}

inline BOOL ReadFile(HANDLE handle, void* buffer, DWORD bytesToRead, DWORD* bytesRead, void*)
{
	if (!handle || handle == INVALID_HANDLE_VALUE || !buffer)
	{
		return FALSE;
	}

	FILE* file = reinterpret_cast<FILE*>(handle);
	size_t readCount = std::fread(buffer, 1, bytesToRead, file);

	if (bytesRead)
	{
		*bytesRead = static_cast<DWORD>(readCount);
	}

	if (readCount < bytesToRead && std::ferror(file))
	{
		return FALSE;
	}

	return TRUE;
}

inline DWORD GetFileSize(HANDLE handle, DWORD*)
{
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		return static_cast<DWORD>(-1);
	}

	FILE* file = reinterpret_cast<FILE*>(handle);
	int fd = fileno(file);
	if (fd == -1)
	{
		return static_cast<DWORD>(-1);
	}

	struct stat st {};
	if (fstat(fd, &st) != 0)
	{
		return static_cast<DWORD>(-1);
	}

	if (st.st_size < 0)
	{
		return static_cast<DWORD>(-1);
	}

	return static_cast<DWORD>(st.st_size);
}

inline BOOL CloseHandle(HANDLE handle)
{
	if (!handle || handle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	FILE* file = reinterpret_cast<FILE*>(handle);
	return (std::fclose(file) == 0) ? TRUE : FALSE;
}

struct FindFileHandle
{
	std::vector<std::filesystem::directory_entry> entries;
	size_t index = 0;
};

inline void FillFindData(const std::filesystem::directory_entry& entry, WIN32_FIND_DATA* data)
{
	if (!data)
	{
		return;
	}

	data->dwFileAttributes = entry.is_directory() ? FILE_ATTRIBUTE_DIRECTORY : 0;
	std::string name = entry.path().filename().string();
	std::strncpy(data->cFileName, name.c_str(), MAX_PATH);
	data->cFileName[MAX_PATH - 1] = '\0';
}

inline HANDLE FindFirstFile(const char* pattern, WIN32_FIND_DATA* data)
{
	if (!pattern || !data)
	{
		return INVALID_HANDLE_VALUE;
	}

	std::string normalized = NormalizePath(pattern);
	std::filesystem::path patternPath(normalized);
	std::filesystem::path directory = patternPath;

	if (normalized.find('*') != std::string::npos)
	{
		directory = patternPath.parent_path();
	}

	if (directory.empty())
	{
		directory = ".";
	}

	std::error_code ec;
	if (!std::filesystem::exists(directory, ec))
	{
		return INVALID_HANDLE_VALUE;
	}

	auto* handle = new FindFileHandle();
	for (const auto& entry : std::filesystem::directory_iterator(directory, ec))
	{
		handle->entries.push_back(entry);
	}

	if (handle->entries.empty())
	{
		delete handle;
		return INVALID_HANDLE_VALUE;
	}

	handle->index = 0;
	FillFindData(handle->entries[0], data);

	return reinterpret_cast<HANDLE>(handle);
}

inline BOOL FindNextFile(HANDLE handle, WIN32_FIND_DATA* data)
{
	if (!handle || handle == INVALID_HANDLE_VALUE || !data)
	{
		return FALSE;
	}

	auto* state = reinterpret_cast<FindFileHandle*>(handle);
	state->index++;
	if (state->index >= state->entries.size())
	{
		delete state;
		return FALSE;
	}

	FillFindData(state->entries[state->index], data);
	return TRUE;
}

#endif
