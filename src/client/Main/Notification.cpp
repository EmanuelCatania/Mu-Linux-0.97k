#include "stdafx.h"
#include "Notification.h"
#include "Language.h"
#include "Protect.h"
#include "Window.h"

CNotification gNotification;

CNotification::CNotification()
{
	InitializeCriticalSection(&this->m_CriticalSection);

	this->m_Initialized = false;

	this->m_LastSessionActive = false;

	this->m_Enabled = (GetPrivateProfileInt("Notifications", "Enabled", 1, ".\\Config.ini") != 0);

	this->m_WarningMinutes = GetPrivateProfileInt("Notifications", "WarningMinutes", 5, ".\\Config.ini");

	if (this->m_WarningMinutes != 0 && this->m_WarningMinutes != 1 && this->m_WarningMinutes != 5 && this->m_WarningMinutes != 10)
	{
		this->m_WarningMinutes = 5;
	}

	this->m_NotifyOpen = (GetPrivateProfileInt("Notifications", "NotifyOpen", 1, ".\\Config.ini") != 0);

	this->m_NotifyDeath = (GetPrivateProfileInt("Notifications", "NotifyDeath", 1, ".\\Config.ini") != 0);

	this->m_EventCount = 0;

	memset(this->m_Events, 0, sizeof(this->m_Events));
}

CNotification::~CNotification()
{
	DeleteCriticalSection(&this->m_CriticalSection);
}

void CNotification::Update()
{
	bool Active = this->IsSessionActive();

	EnterCriticalSection(&this->m_CriticalSection);

	if (!Active && this->m_LastSessionActive)
	{
		this->m_Initialized = false;

		this->m_EventCount = 0;
	}

	this->m_LastSessionActive = Active;

	LeaveCriticalSection(&this->m_CriticalSection);
}

void CNotification::ResetSession()
{
	EnterCriticalSection(&this->m_CriticalSection);

	this->m_Initialized = false;

	this->m_EventCount = 0;

	memset(this->m_Events, 0, sizeof(this->m_Events));

	this->m_LastSessionActive = false;

	LeaveCriticalSection(&this->m_CriticalSection);

	gWindow.ClearNotificationQueue();
}

bool CNotification::FindEvent(const EVENT_STATE* List, int Count, const char* Name, bool* Used, int* Index)
{
	for (int i = 0; i < Count; i++)
	{
		if (!Used[i] && strcmp(List[i].Name, Name) == 0)
		{
			Used[i] = true;

			*Index = i;

			return true;
		}
	}

	return false;
}

void CNotification::AppendMessage(char* Dest, int Capacity, const char* Format, const char* Name, DWORD Time, int* Length)
{
	char Text[96] = { 0 };

	int TextLength = sprintf_s(Text, sizeof(Text), Format, Name, (int)(Time / 60));

	if (TextLength <= 0 || Dest == NULL || Length == NULL || Capacity <= 1)
	{
		return;
	}

	int Prefix = (*Length > 0) ? 2 : 0;

	if ((*Length + Prefix + TextLength) < Capacity)
	{
		if (Prefix != 0)
		{
			strcat_s(Dest, Capacity, "; ");
		}

		strcat_s(Dest, Capacity, Text);

		*Length = (int)strlen(Dest);

		return;
	}

	if (Capacity < 4 || *Length >= Capacity - 1)
	{
		return;
	}

	int KeepLength = Capacity - 4;

	if (KeepLength > *Length)
	{
		KeepLength = *Length;
	}

	if (KeepLength >= 3 && strcmp(&Dest[KeepLength - 3], "...") == 0)
	{
		return;
	}

	Dest[KeepLength] = '.';
	Dest[KeepLength + 1] = '.';
	Dest[KeepLength + 2] = '.';
	Dest[KeepLength + 3] = 0;
	*Length = KeepLength + 3;
}

void CNotification::OnEventSnapshot(const PMSG_EVENT_TIME* EventList, int Count)
{
	if (EventList == NULL || Count < 0 || Count > MAX_EVENTS || !this->IsSessionActive())
	{
		return;
	}

	EVENT_STATE Current[MAX_EVENTS];

	memset(Current, 0, sizeof(Current));

	for (int i = 0; i < Count; i++)
	{
		memcpy(Current[i].Name, EventList[i].name, sizeof(Current[i].Name));

		Current[i].Name[sizeof(Current[i].Name) - 1] = 0;

		Current[i].Status = EventList[i].status;

		Current[i].Time = EventList[i].time;
	}

	EnterCriticalSection(&this->m_CriticalSection);

	bool WasInitialized = this->m_Initialized;

	EVENT_STATE Previous[MAX_EVENTS];

	int PreviousCount = this->m_EventCount;

	memcpy(Previous, this->m_Events, sizeof(Previous));

	bool Used[MAX_EVENTS] = { false };

	char Message[256] = { 0 };

	int MessageLength = 0;

	const char* OpenFormat = (gLanguage.LangNum == LANGUAGE_PORTUGUESE) ? "Aberto: %s" : ((gLanguage.LangNum == LANGUAGE_SPANISH) ? "Abierto: %s" : "Open: %s");
	const char* WarningFormat = "%s: %d min";

	for (int i = 0; i < Count && WasInitialized; i++)
	{
		int PreviousIndex = -1;

		if (!this->FindEvent(Previous, PreviousCount, Current[i].Name, Used, &PreviousIndex))
		{
			if (Current[i].Status == EVENT_STATE_OPEN && this->m_NotifyOpen)
			{
				this->AppendMessage(Message, sizeof(Message), OpenFormat, Current[i].Name, Current[i].Time, &MessageLength);
			}
			continue;
		}

		if (this->m_WarningMinutes > 0 && Previous[PreviousIndex].Status == EVENT_STATE_STAND && Current[i].Status == EVENT_STATE_STAND &&
			Previous[PreviousIndex].Time > (DWORD)(this->m_WarningMinutes * 60) && Current[i].Time <= (DWORD)(this->m_WarningMinutes * 60) &&
			Previous[PreviousIndex].Time >= Current[i].Time)
		{
			this->AppendMessage(Message, sizeof(Message), WarningFormat, Current[i].Name, Current[i].Time, &MessageLength);
		}

		if (Current[i].Status == EVENT_STATE_OPEN && Previous[PreviousIndex].Status != EVENT_STATE_OPEN && this->m_NotifyOpen)
		{
			this->AppendMessage(Message, sizeof(Message), OpenFormat, Current[i].Name, Current[i].Time, &MessageLength);
		}

	}

	memcpy(this->m_Events, Current, sizeof(this->m_Events));

	this->m_EventCount = Count;

	this->m_Initialized = true;

	this->m_LastSessionActive = true;

	bool ShouldQueue = (WasInitialized && this->m_Enabled && MessageLength > 0 && gWindow.IsInactive());

	LeaveCriticalSection(&this->m_CriticalSection);

	if (ShouldQueue)
	{
		this->QueueEvents(Message);
	}
}

void CNotification::BuildTitle(char* Title, int Capacity)
{
	const char* CharacterName = gWindow.GetCharacterName();

	if (CharacterName != NULL && CharacterName[0] != 0)
	{
		sprintf_s(Title, Capacity, "%s - %s", gProtect.m_MainInfo.WindowName, CharacterName);
	}
	else
	{
		sprintf_s(Title, Capacity, "%s", gProtect.m_MainInfo.WindowName);
	}
}

void CNotification::QueueEvents(const char* Message)
{
	char Title[128] = { 0 };

	this->BuildTitle(Title, sizeof(Title));

	gWindow.QueueNotification(Title, Message);
}

void CNotification::BuildDeathMessage(char* Message, int Capacity)
{
	if (gLanguage.LangNum == LANGUAGE_PORTUGUESE)
	{
		sprintf_s(Message, Capacity, "Seu personagem morreu.");
	}
	else if (gLanguage.LangNum == LANGUAGE_SPANISH)
	{
		sprintf_s(Message, Capacity, "Tu personaje ha muerto.");
	}
	else
	{
		sprintf_s(Message, Capacity, "Your character died.");
	}
}

void CNotification::NotifyDeath()
{
	EnterCriticalSection(&this->m_CriticalSection);

	bool ShouldQueue = (this->m_Enabled && this->m_NotifyDeath && gWindow.IsInactive());

	LeaveCriticalSection(&this->m_CriticalSection);

	if (!ShouldQueue)
	{
		return;
	}

	char Message[128] = { 0 };

	this->BuildDeathMessage(Message, sizeof(Message));

	this->QueueEvents(Message);
}

bool CNotification::IsSessionActive()
{
	return (g_bGameServerConnected && SceneFlag == MAIN_SCENE);
}

void CNotification::SaveValue(const char* Key, int Value)
{
	char Text[16] = { 0 };

	sprintf_s(Text, sizeof(Text), "%d", Value);

	WritePrivateProfileString("Notifications", Key, Text, ".\\Config.ini");
}

bool CNotification::GetEnabled() const { return this->m_Enabled; }
int CNotification::GetWarningMinutes() const { return this->m_WarningMinutes; }
bool CNotification::GetNotifyOpen() const { return this->m_NotifyOpen; }
bool CNotification::GetNotifyDeath() const { return this->m_NotifyDeath; }

void CNotification::SetEnabled(bool Value)
{
	this->m_Enabled = Value;
	this->SaveValue("Enabled", Value ? 1 : 0);
}

void CNotification::SetWarningMinutes(int Value)
{
	if (Value != 0 && Value != 1 && Value != 5 && Value != 10)
	{
		return;
	}

	this->m_WarningMinutes = Value;

	this->SaveValue("WarningMinutes", Value);
}

void CNotification::SetNotifyOpen(bool Value)
{
	this->m_NotifyOpen = Value;

	this->SaveValue("NotifyOpen", Value ? 1 : 0);
}

void CNotification::SetNotifyDeath(bool Value)
{
	this->m_NotifyDeath = Value;

	this->SaveValue("NotifyDeath", Value ? 1 : 0);
}
