#pragma once

class CWindow
{
public:

	enum eTrayMode
	{
		TRAY_MODE_NONE = 0,
		TRAY_MODE_NOTIFICATION = 1,
		TRAY_MODE_F12 = 2,
	};

	CWindow();

	virtual ~CWindow();

	void Init(HINSTANCE hins);

	void ToggleTrayMode();

	bool ShowTrayMessage(const char* Title, const char* Message);

	void QueueNotification(const char* Title, const char* Message);

	void ProcessNotificationQueue();

	void ClearNotificationQueue();

	bool IsInactive() const;

	const char* GetWindowName() const;

	const char* GetCharacterName() const;

	void ChangeWindowText();

	void SetWindowMode(bool windowMode, bool borderless);

	void SetResolution(int res);

	void ChangeWindowState(bool windowMode, bool borderless, int resolution);

private:

	static LONG WINAPI FixDisplaySettingsOnClose(DEVMODEA* lpDevMode, DWORD dwFlags);

	static LONG WINAPI MyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static HWND StartWindow(HINSTANCE hCurrentInst, int nCmdShow);

	void ChangeDisplaySettingsFunction();

	static bool CreateOpenglWindow();

	void RemoveTrayIcon();

	void RestoreFromNotification();

	void HandleTaskbarCreated();

	void HandleWindowActivated();

	bool IsNotificationAllowed() const;

	bool EnsureTrayIcon(eTrayMode Mode);

private:

	HINSTANCE Instance;

	HICON m_WindowIcon;

	char m_WindowName[256];

	char m_CharacterName[64];

	eTrayMode m_TrayMode;

	bool m_TrayIconVisible;

	bool m_WindowReady;

	bool m_WindowActive;

	bool m_WindowMinimized;

	struct NOTIFICATION_MESSAGE
	{
		char Title[128];
		char Message[256];
	};

	static const int NOTIFICATION_QUEUE_SIZE = 8;

	NOTIFICATION_MESSAGE m_NotificationQueue[NOTIFICATION_QUEUE_SIZE];

	int m_NotificationQueueHead;

	int m_NotificationQueueTail;

	int m_NotificationQueueCount;

	DWORD m_NextNotificationTick;

	DWORD m_LastTrayToggleTick;

	CRITICAL_SECTION m_NotificationCriticalSection;

public:

	bool m_WindowMode;

	bool m_Borderless;

	std::pair<WORD, WORD> iResolutionValues[MAX_RESOLUTION_VALUE];
};

extern CWindow gWindow;
