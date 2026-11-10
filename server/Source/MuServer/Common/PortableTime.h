#pragma once

// CTime y CTimeSpan de ATL, con lo que usa el GameServer. MSVC usa los de ATL
// (atltime.h); Linux y MinGW, que no tienen ATL, usan estos.
#include <ctime>

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
#ifdef _WIN32
		localtime_s(&local_tm, &m_time);
#else
		localtime_r(&m_time, &local_tm);
#endif
		return local_tm;
	}

	std::time_t m_time;
};
