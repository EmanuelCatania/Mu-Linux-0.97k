#include "stdafx.h"
#include "CriticalSection.h"

CCriticalSection::CCriticalSection()
{
#ifdef _WIN32
	InitializeCriticalSection(&this->m_critical);
#endif
}

CCriticalSection::~CCriticalSection()
{
#ifdef _WIN32
	DeleteCriticalSection(&this->m_critical);
#endif
}

void CCriticalSection::lock()
{
#ifdef _WIN32
	EnterCriticalSection(&this->m_critical);
#else
	this->m_critical.lock();
#endif
}

void CCriticalSection::unlock()
{
#ifdef _WIN32
	LeaveCriticalSection(&this->m_critical);
#else
	this->m_critical.unlock();
#endif
}
