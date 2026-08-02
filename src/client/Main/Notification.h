#pragma once

#include "EventTimer.h"

class CNotification
{
public:

	CNotification();

	virtual ~CNotification();

	void Update();

	void ResetSession();

	void OnEventSnapshot(const PMSG_EVENT_TIME* EventList, int Count);

	void NotifyDeath();

	bool GetEnabled() const;

	int GetWarningMinutes() const;

	bool GetNotifyOpen() const;

	bool GetNotifyDeath() const;

	void SetEnabled(bool Value);

	void SetWarningMinutes(int Value);

	void SetNotifyOpen(bool Value);

	void SetNotifyDeath(bool Value);

private:

	struct EVENT_STATE
	{
		char Name[32];
		BYTE Status;
		DWORD Time;
	};

	static const int MAX_EVENTS = 64;

	void SaveValue(const char* Key, int Value);

	bool FindEvent(const EVENT_STATE* List, int Count, const char* Name, bool* Used, int* Index);

	void AppendMessage(char* Dest, int Capacity, const char* Format, const char* Name, DWORD Time, int* Length);

	void QueueEvents(const char* Message);

	void BuildDeathMessage(char* Message, int Capacity);

	void BuildTitle(char* Title, int Capacity);

	bool IsSessionActive();

private:

	bool m_Initialized;

	bool m_LastSessionActive;

	bool m_Enabled;

	int m_WarningMinutes;

	bool m_NotifyOpen;

	bool m_NotifyDeath;

	int m_EventCount;

	EVENT_STATE m_Events[MAX_EVENTS];

	CRITICAL_SECTION m_CriticalSection;
};

extern CNotification gNotification;
