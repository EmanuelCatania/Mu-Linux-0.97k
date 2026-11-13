#pragma once

#include <mutex>

class CCriticalSection
{
public:

	CCriticalSection();

	~CCriticalSection();

	void lock();

	void unlock();

private:

#ifdef _WIN32
	CRITICAL_SECTION m_critical;
#else
	std::recursive_mutex m_critical;
#endif
};
