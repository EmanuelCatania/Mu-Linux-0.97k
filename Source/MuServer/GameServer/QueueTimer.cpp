#include "stdafx.h"
#include "QueueTimer.h"

CQueueTimer gQueueTimer;

#ifdef _WIN32

CQueueTimer::CQueueTimer()
{
	this->m_QueueTimer = CreateTimerQueue();

	this->m_QueueTimerInfo.clear();
}

CQueueTimer::~CQueueTimer()
{
	DeleteTimerQueue(this->m_QueueTimer);

	this->m_QueueTimerInfo.clear();
}

void CQueueTimer::CreateTimer(int TimerIndex, int TimerDelay, WAITORTIMERCALLBACK CallbackFunction)
{
	QUEUE_TIMER_INFO QueueTimerInfo;

	QueueTimerInfo.TimerIndex = TimerIndex;

	CreateTimerQueueTimer(&QueueTimerInfo.QueueTimerTimer, this->m_QueueTimer, CallbackFunction, (PVOID)TimerIndex, 1000, TimerDelay, WT_EXECUTEINTIMERTHREAD);

	this->m_QueueTimerInfo.insert(std::pair<int, QUEUE_TIMER_INFO>(QueueTimerInfo.TimerIndex, QueueTimerInfo));
}

void CQueueTimer::DeleteTimer(int TimerIndex)
{
	std::map<int, QUEUE_TIMER_INFO>::iterator it = this->m_QueueTimerInfo.find(TimerIndex);

	if (it != this->m_QueueTimerInfo.end())
	{
		DeleteTimerQueueTimer(this->m_QueueTimer, it->second.QueueTimerTimer, 0);

		this->m_QueueTimerInfo.erase(it);
	}
}

#else

CQueueTimer::CQueueTimer()
{
	this->m_QueueTimerInfo.clear();
}

CQueueTimer::~CQueueTimer()
{
	for (auto& entry : this->m_QueueTimerInfo)
	{
		if (entry.second.StopFlag)
		{
			entry.second.StopFlag->store(true);
		}

		if (entry.second.Thread.joinable())
		{
			entry.second.Thread.join();
		}
	}

	this->m_QueueTimerInfo.clear();
}

void CQueueTimer::CreateTimer(int TimerIndex, int TimerDelay, WAITORTIMERCALLBACK CallbackFunction)
{
	this->DeleteTimer(TimerIndex);

	QUEUE_TIMER_INFO info;
	info.TimerIndex = TimerIndex;
	info.TimerDelay = TimerDelay;
	info.Callback = CallbackFunction;
	info.StopFlag = std::make_shared<std::atomic<bool>>(false);

	auto stopFlag = info.StopFlag;
	int delay = TimerDelay;
	int index = TimerIndex;
	WAITORTIMERCALLBACK callback = CallbackFunction;

	info.Thread = std::thread([stopFlag, delay, index, callback]()
	{
		Sleep(1000);
		while (!stopFlag->load())
		{
			callback(reinterpret_cast<PVOID>(static_cast<intptr_t>(index)), 0);
			Sleep(static_cast<DWORD>(delay));
		}
	});

	this->m_QueueTimerInfo.emplace(TimerIndex, std::move(info));
}

void CQueueTimer::DeleteTimer(int TimerIndex)
{
	auto it = this->m_QueueTimerInfo.find(TimerIndex);
	if (it == this->m_QueueTimerInfo.end())
	{
		return;
	}

	if (it->second.StopFlag)
	{
		it->second.StopFlag->store(true);
	}

	if (it->second.Thread.joinable())
	{
		it->second.Thread.join();
	}

	this->m_QueueTimerInfo.erase(it);
}

#endif
