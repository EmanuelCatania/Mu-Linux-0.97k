#include "StdAfx.h"
#include "Window.h"
#include "Controller.h"
#include "Input.h"
#include "LoginCredentials.h"
#include "ItemLink.h"
#include "Notification.h"
#include "Font.h"
#include "PingSystem.h"
#include "PrintPlayer.h"
#include "Protect.h"
#include "Protocol.h"
#include "resource.h"

#ifndef NIIF_RESPECT_QUIET_TIME
#define NIIF_RESPECT_QUIET_TIME 0x00000080
#endif

#ifndef NIF_SHOWTIP
#define NIF_SHOWTIP 0x00000080
#endif

CWindow	gWindow;

static UINT TaskbarCreatedMessage = 0;

#ifdef _DEBUG
static void WindowDebug(const char* Format, ...)
{
	char Buffer[512] = { 0 };
	va_list Args;
	va_start(Args, Format);
	vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
	va_end(Args);

	OutputDebugStringA(Buffer);
	OutputDebugStringA("\n");
}
#endif

static bool CopyAnsiToWide(const char* Source, wchar_t* Destination, int Capacity)
{
	if (Source == NULL || Destination == NULL || Capacity <= 0)
	{
		return false;
	}

	Destination[0] = 0;

	int Length = MultiByteToWideChar(1252, MB_PRECOMPOSED, Source, -1, Destination, Capacity);

	if (Length <= 0)
	{
		Destination[0] = 0;
		return false;
	}

	Destination[Capacity - 1] = 0;

	return true;
}

static bool TickIsBefore(DWORD Current, DWORD Deadline)
{
	return (Deadline != 0 && (LONG)(Current - Deadline) < 0);
}

CWindow::CWindow()
{
	InitializeCriticalSection(&this->m_NotificationCriticalSection);

	this->m_WindowIcon = NULL;
	this->m_TrayMode = TRAY_MODE_NONE;
	this->m_TrayIconVisible = false;
	this->m_WindowReady = false;
	this->m_WindowActive = false;
	this->m_WindowMinimized = false;
	this->m_NotificationQueueHead = 0;
	this->m_NotificationQueueTail = 0;
	this->m_NotificationQueueCount = 0;
	this->m_NextNotificationTick = 0;
	this->m_LastTrayToggleTick = 0;

	this->iResolutionValues[R640x480] = std::make_pair<WORD, WORD>(640, 480);
	this->iResolutionValues[R800x600] = std::make_pair<WORD, WORD>(800, 600);
	this->iResolutionValues[R1024x768] = std::make_pair<WORD, WORD>(1024, 768);
	this->iResolutionValues[R1280x1024] = std::make_pair<WORD, WORD>(1280, 1024);
	this->iResolutionValues[R1280x720] = std::make_pair<WORD, WORD>(1280, 720);
	this->iResolutionValues[R1366x768] = std::make_pair<WORD, WORD>(1366, 768);
	this->iResolutionValues[R1600x900] = std::make_pair<WORD, WORD>(1600, 900);
	this->iResolutionValues[R1920x1080] = std::make_pair<WORD, WORD>(1920, 1080);

	sprintf_s(this->m_WindowName, sizeof(this->m_WindowName), "%s", gProtect.m_MainInfo.WindowName);

	this->m_CharacterName[0] = 0;

	this->m_WindowMode = WINDOW_MODE;

	this->m_Borderless = false;

	m_Resolution = R1024x768;

	WindowWidth = this->iResolutionValues[R1024x768].first;

	WindowHeight = this->iResolutionValues[R1024x768].second;

	g_fScreenRate_x = (float)WindowWidth / 640.0f;

	g_fScreenRate_y = (float)WindowHeight / 480.0f;
}

CWindow::~CWindow()
{
	this->RemoveTrayIcon();

	if (this->m_WindowIcon != NULL)
	{
		DestroyIcon(this->m_WindowIcon);
		this->m_WindowIcon = NULL;
	}

	DeleteCriticalSection(&this->m_NotificationCriticalSection);

	char Text[33] = { 0 };

	wsprintf(Text, "%d", this->m_WindowMode);

	WritePrivateProfileString("Window", "WindowMode", Text, ".\\Config.ini");

	wsprintf(Text, "%d", this->m_Borderless);

	WritePrivateProfileString("Window", "Borderless", Text, ".\\Config.ini");

	wsprintf(Text, "%d", m_Resolution);

	WritePrivateProfileString("Window", "Resolution", Text, ".\\Config.ini");
}

void CWindow::Init(HINSTANCE hins)
{
	this->Instance = hins;

	TaskbarCreatedMessage = RegisterWindowMessage("TaskbarCreated");

	this->m_WindowIcon = (HICON)LoadImage(hins, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

	this->m_WindowMode = (GetPrivateProfileInt("Window", "WindowMode", WINDOW_MODE, ".\\Config.ini") != 0);

	this->m_Borderless = (GetPrivateProfileInt("Window", "Borderless", 0, ".\\Config.ini") != 0);

	this->SetResolution(GetPrivateProfileInt("Window", "Resolution", R1024x768, ".\\Config.ini"));

	SetCompleteHook(0xE8, 0x00412BC4, &this->FixDisplaySettingsOnClose);
	SetByte(0x00412BC4 + 5, 0x90);

	SetCompleteHook(0xE9, 0x0041ED79, 0x0041EEC6);

	SetCompleteHook(0xE9, 0x0041DFF0, &this->StartWindow);

	SetCompleteHook(0xE9, 0x0041DE30, &this->CreateOpenglWindow);

	SetCompleteHook(0xE9, 0x0041F617, 0x00421B0B);
}

LONG WINAPI CWindow::FixDisplaySettingsOnClose(DEVMODEA* lpDevMode, DWORD dwFlags)
{
	if (!gWindow.m_WindowMode)
	{
		return ChangeDisplaySettings(NULL, 0);
	}

	return 0;
}

LRESULT WINAPI CWindow::MyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (TaskbarCreatedMessage != 0 && msg == TaskbarCreatedMessage)
	{
		gWindow.HandleTaskbarCreated();

		return 0;
	}

	switch (msg)
	{
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				gWindow.m_WindowActive = false;
			}
			else
			{
				gWindow.HandleWindowActivated();
			}

			break;
		}

		case WM_ACTIVATEAPP:
		{
			if (wParam == FALSE)
			{
				gWindow.m_WindowActive = false;
			}
			else if (GetForegroundWindow() == hwnd && !IsIconic(hwnd))
			{
				gWindow.HandleWindowActivated();
			}

			break;
		}

		case WM_DESTROY:
		{
			gWindow.m_WindowReady = false;
			gWindow.m_TrayMode = TRAY_MODE_NONE;
			gWindow.RemoveTrayIcon();

			break;
		}
		case WM_KEYDOWN:
		{
			if (gLoginCredentials.HandleKeyDown(wParam))
			{
				return 0;
			}

			if (gItemLink.HandleKeyDown(wParam))
			{
				return 0;
			}

			if (gInput.HandleKeyDown(wParam))
			{
				return 0;
			}

			break;
		}

		case WM_CHAR:
		{
			if (gLoginCredentials.HandleChar(wParam))
			{
				return 0;
			}

			if (gItemLink.HandleChar(wParam))
			{
				return 0;
			}

			if (gInput.HandleChar(wParam))
			{
				return 0;
			}

			break;
		}
		case WM_LBUTTONDOWN:
		{
			if (gLoginCredentials.HandleLeftButtonDown(hwnd, lParam))
			{
				return 0;
			}

			if (gItemLink.HandleLeftButtonDown())
			{
				return 0;
			}

			if (MouseRButton || MouseRButtonPush)
			{
				return 0;
			}

			MouseLButtonPop = false;

			if (!MouseLButton)
			{
				MouseLButtonPush = true;
			}

			MouseLButton = true;

			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		case WM_RBUTTONDOWN:
		{
			if (MouseLButton || MouseLButtonPush)
			{
				return 0;
			}

			MouseRButtonPop = false;

			if (!MouseRButton)
			{
				MouseRButtonPush = true;
			}

			MouseRButton = true;

			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		case WM_NPROTECT_EXIT_TWO: // Fix disconnect when minimize
		{
			return 0;
		}

		case WM_SIZE: // Fix disconnect when minimize
		{
			if (wParam == SIZE_MINIMIZED)
			{
				gWindow.m_WindowMinimized = true;
			}
			else if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
			{
				gWindow.m_WindowMinimized = false;
			}

			return 0;
		}

		case WM_TIMER:
		{
			switch (wParam)
			{
				case WM_NOTIFICATION_TIMER:
				{
					KillTimer(hwnd, WM_NOTIFICATION_TIMER);

					gWindow.ProcessNotificationQueue();

					return 0;
				}

				case WM_AUTOCLICKTIMER:
				{
					gController.AutoClickState ^= 1;

					MouseRButtonPush = gController.AutoClickState;

					MouseRButton = gController.AutoClickState;

					return 0;
				}

				case WINDOWMINIMIZED_TIMER:
				{
					return 0;
				}

				case HACK_TIMER:
				{
					if (g_bGameServerConnected)
					{
						gProtocol.CGLiveClientSend();
					}

					return 0;
				}
			}

			break;
		}

		case WM_TRAY_MODE_MESSAGE:
		{
			if ((UINT)HIWORD(lParam) != WM_TRAY_MODE_ICON)
			{
				break;
			}

			UINT TrayMessage = (UINT)LOWORD(lParam);

			if (TrayMessage == NIN_BALLOONUSERCLICK)
			{
				gWindow.RestoreFromNotification();
			}
			else if (TrayMessage == WM_LBUTTONUP || TrayMessage == NIN_SELECT
			#ifdef NIN_KEYSELECT
				|| TrayMessage == NIN_KEYSELECT
			#endif
				)
			{
				DWORD Tick = GetTickCount();

				if (gWindow.m_LastTrayToggleTick == 0 || (Tick - gWindow.m_LastTrayToggleTick) >= 250)
				{
					gWindow.m_LastTrayToggleTick = Tick;
					gWindow.RestoreFromNotification();
				}
			}

			break;
		}

		case WM_NOTIFICATION_MESSAGE:
		{
			gWindow.ProcessNotificationQueue();

			return 0;
		}
	}

	return CallWindowProc(WndProc, hwnd, msg, wParam, lParam);
}

HWND CWindow::StartWindow(HINSTANCE hCurrentInst, int nCmdShow)
{
	char* windowName = "MU ONLINE";

	WNDCLASS wndClass = { 0 };

	wndClass.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;

	wndClass.lpfnWndProc = gWindow.MyWndProc;

	wndClass.cbClsExtra = 0;

	wndClass.cbWndExtra = 0;

	wndClass.hInstance = gWindow.Instance;

	wndClass.hIcon = gWindow.m_WindowIcon;

	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	wndClass.lpszMenuName = NULL;

	wndClass.lpszClassName = windowName;

	RegisterClass(&wndClass);

	HWND hWnd;

	if (gWindow.m_WindowMode)
	{
		RECT rc = { 0, 0, WindowWidth, WindowHeight };

		LONG STYLE = (gWindow.m_Borderless)
			? WS_POPUP | WS_VISIBLE
			: WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;

		AdjustWindowRect(&rc, STYLE, NULL);

		hWnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, windowName, gProtect.m_MainInfo.WindowName, STYLE, (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2, (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2, rc.right, rc.bottom + 26, NULL, NULL, gWindow.Instance, NULL);
	}
	else
	{
		hWnd = CreateWindowEx(WS_EX_APPWINDOW, windowName, gProtect.m_MainInfo.WindowName, WS_POPUP | WS_VISIBLE, 0, 0, WindowWidth, WindowHeight, NULL, NULL, gWindow.Instance, NULL);

		gWindow.ChangeDisplaySettingsFunction();
	}

	return hWnd;
}

void CWindow::ChangeDisplaySettingsFunction()
{
	std::vector<DEVMODE> displayModes;

	DEVMODE devMode = {};

	int modeIndex = 0;

	DWORD preferredBitsPerPel = 0;

	while (EnumDisplaySettings(NULL, modeIndex, &devMode))
	{
		displayModes.push_back(devMode);

		if (devMode.dmBitsPerPel > preferredBitsPerPel)
		{
			preferredBitsPerPel = devMode.dmBitsPerPel;
		}

		modeIndex++;
	}

	for (auto& mode : displayModes)
	{
		if (mode.dmPelsWidth == WindowWidth &&
		    mode.dmPelsHeight == WindowHeight &&
		    mode.dmBitsPerPel == preferredBitsPerPel)
		{
			ChangeDisplaySettings(&mode, 0);

			return;
		}
	}

	MessageBox(NULL, "It was not possible to find any compatible configuration with the selected resolution.", "Display Settings Error.", MB_OK | MB_ICONEXCLAMATION);
}

bool CWindow::CreateOpenglWindow()
{
	PIXELFORMATDESCRIPTOR pfd;

	memset(&pfd, 0, sizeof(pfd));

	pfd.nSize = sizeof(pfd);

	pfd.nVersion = 1;

	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	pfd.iPixelType = PFD_TYPE_RGBA;

	pfd.cColorBits = 32;

	pfd.cDepthBits = 32;

	if (!(g_hDC = GetDC(g_hWnd))) // Get the device context.
	{
		KillGLWindow(); // reset the display

		MessageBox(NULL, GlobalText[4], "OpenGL Get DC Error.", MB_OK | MB_ICONEXCLAMATION);

		return false; // failure
	}

	GLuint PixelFormat; // Find and remember the appropriate pixel format.

	if (!(PixelFormat = ChoosePixelFormat(g_hDC, &pfd))) // Select the pixel format closest to the one specified by pdf.
	{
		KillGLWindow(); // reset the display

		MessageBox(NULL, GlobalText[4], "OpenGL Choose Pixel Format Error.", MB_OK | MB_ICONEXCLAMATION);

		return false; // failure
	}

	if (!SetPixelFormat(g_hDC, PixelFormat, &pfd)) // Set the pixel format of the device context.
	{
		KillGLWindow(); // reset the display

		MessageBox(NULL, GlobalText[4], "OpenGL Set Pixel Format Error.", MB_OK | MB_ICONEXCLAMATION);

		return false; // failure
	}

	if (!(g_hRC = wglCreateContext(g_hDC))) // Create an appropriate rendering context with the device context.
	{
		KillGLWindow(); // reset the display

		MessageBox(NULL, GlobalText[4], "OpenGL Create Context Error.", MB_OK | MB_ICONEXCLAMATION);

		return false; // failure
	}

	if (!wglMakeCurrent(g_hDC, g_hRC)) // Activate the rendering context and associate it with the device context.
	{
		KillGLWindow(); // reset the display

		MessageBox(NULL, GlobalText[4], "OpenGL Make Current Error.", MB_OK | MB_ICONEXCLAMATION);

		return false; // failure
	}

	ShowWindow(g_hWnd, SW_SHOW); // show the window

	SetForegroundWindow(g_hWnd); // bring the window to the top

	SetFocus(g_hWnd); // Give the window keyboard focus.

	gWindow.m_WindowReady = true;
	gWindow.m_WindowActive = true;
	gWindow.m_WindowMinimized = false;
	gWindow.m_TrayMode = TRAY_MODE_NONE;
	gWindow.RemoveTrayIcon();

	return true;
}

void CWindow::ToggleTrayMode()
{
	if (this->m_TrayMode == TRAY_MODE_F12)
	{
		this->RestoreFromNotification();
	}
	else if (this->m_TrayMode == TRAY_MODE_NOTIFICATION)
	{
		this->m_TrayMode = TRAY_MODE_F12;

		ShowWindow(g_hWnd, SW_HIDE);

		if (!this->EnsureTrayIcon(TRAY_MODE_F12))
		{
			this->m_TrayMode = TRAY_MODE_NONE;
			ShowWindow(g_hWnd, SW_SHOW);
		}
	}
	else
	{
		this->m_TrayMode = TRAY_MODE_F12;
		this->m_WindowActive = false;

		ShowWindow(g_hWnd, SW_HIDE);

		if (!this->EnsureTrayIcon(TRAY_MODE_F12))
		{
			this->m_TrayMode = TRAY_MODE_NONE;
			ShowWindow(g_hWnd, SW_SHOW);
		}
	}
}

bool CWindow::IsInactive() const
{
	return (!this->m_WindowReady || this->m_TrayMode != TRAY_MODE_NONE || !this->m_WindowActive || this->m_WindowMinimized || GetForegroundWindow() != g_hWnd || IsWindowVisible(g_hWnd) == FALSE);
}

void CWindow::HandleWindowActivated()
{
	this->m_WindowActive = true;

	if (this->m_TrayMode != TRAY_MODE_F12)
	{
		this->m_TrayMode = TRAY_MODE_NONE;
		this->RemoveTrayIcon();
		this->ClearNotificationQueue();
	}

	gNotification.Update();
}

const char* CWindow::GetWindowName() const
{
	return this->m_WindowName;
}

const char* CWindow::GetCharacterName() const
{
	return this->m_CharacterName;
}

bool CWindow::EnsureTrayIcon(eTrayMode Mode)
{
	if (Mode == TRAY_MODE_NONE || g_hWnd == NULL || this->m_WindowIcon == NULL)
	{
		return false;
	}

	if (this->m_TrayMode == TRAY_MODE_F12 && Mode == TRAY_MODE_NOTIFICATION)
	{
		Mode = TRAY_MODE_F12;
	}

	if (this->m_TrayIconVisible)
	{
		this->m_TrayMode = Mode;
		return true;
	}

	NOTIFYICONDATAW Icon;

	memset(&Icon, 0, sizeof(Icon));

	Icon.cbSize = sizeof(Icon);
	Icon.hWnd = g_hWnd;
	Icon.uID = WM_TRAY_MODE_ICON;
	Icon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP;
	Icon.uCallbackMessage = WM_TRAY_MODE_MESSAGE;
	Icon.hIcon = this->m_WindowIcon;

	if (!CopyAnsiToWide(this->m_WindowName, Icon.szTip, ARRAYSIZE(Icon.szTip)))
	{
		CopyAnsiToWide("MU Online", Icon.szTip, ARRAYSIZE(Icon.szTip));
	}

	if (!Shell_NotifyIconW(NIM_ADD, &Icon))
	{
#ifdef _DEBUG
		WindowDebug("Shell_NotifyIconW(NIM_ADD) failed: %lu", GetLastError());
#endif
		return false;
	}

	Icon.uVersion = NOTIFYICON_VERSION_4;

	if (!Shell_NotifyIconW(NIM_SETVERSION, &Icon))
	{
#ifdef _DEBUG
		WindowDebug("Shell_NotifyIconW(NIM_SETVERSION) failed: %lu", GetLastError());
#endif
		if (!Shell_NotifyIconW(NIM_DELETE, &Icon))
		{
#ifdef _DEBUG
			WindowDebug("Shell_NotifyIconW(NIM_DELETE) cleanup failed: %lu", GetLastError());
#endif
		}
		return false;
	}

	this->m_TrayIconVisible = true;
	this->m_TrayMode = Mode;

	return true;
}

void CWindow::RemoveTrayIcon()
{
	if (!this->m_TrayIconVisible || g_hWnd == NULL)
	{
		this->m_TrayIconVisible = false;
		return;
	}

	NOTIFYICONDATAW Icon;

	memset(&Icon, 0, sizeof(Icon));

	Icon.cbSize = sizeof(Icon);
	Icon.hWnd = g_hWnd;
	Icon.uID = WM_TRAY_MODE_ICON;

	if (!Shell_NotifyIconW(NIM_DELETE, &Icon))
	{
#ifdef _DEBUG
		WindowDebug("Shell_NotifyIconW(NIM_DELETE) failed: %lu", GetLastError());
#endif
	}

	this->m_TrayIconVisible = false;
}

bool CWindow::IsNotificationAllowed() const
{
	if (!this->IsInactive())
	{
		return false;
	}

	QUERY_USER_NOTIFICATION_STATE State;

	if (SUCCEEDED(SHQueryUserNotificationState(&State)) && State != QUNS_ACCEPTS_NOTIFICATIONS)
	{
		return false;
	}

	return true;
}

void CWindow::QueueNotification(const char* Title, const char* Message)
{
	if (Title == NULL || Message == NULL || !this->IsInactive() || g_hWnd == NULL)
	{
		return;
	}

	EnterCriticalSection(&this->m_NotificationCriticalSection);

	if (this->m_NotificationQueueCount >= NOTIFICATION_QUEUE_SIZE)
	{
		LeaveCriticalSection(&this->m_NotificationCriticalSection);

#ifdef _DEBUG
		WindowDebug("Notification queue full; dropping newest message.");
#endif

		return;
	}

	NOTIFICATION_MESSAGE* Item = &this->m_NotificationQueue[this->m_NotificationQueueTail];

	strncpy_s(Item->Title, sizeof(Item->Title), Title, _TRUNCATE);
	strncpy_s(Item->Message, sizeof(Item->Message), Message, _TRUNCATE);

	this->m_NotificationQueueTail = (this->m_NotificationQueueTail + 1) % NOTIFICATION_QUEUE_SIZE;
	this->m_NotificationQueueCount++;

	LeaveCriticalSection(&this->m_NotificationCriticalSection);

	if (!PostMessage(g_hWnd, WM_NOTIFICATION_MESSAGE, 0, 0))
	{
	#ifdef _DEBUG
		WindowDebug("PostMessage(WM_NOTIFICATION_MESSAGE) failed: %lu", GetLastError());
	#endif
		this->ClearNotificationQueue();
	}
}

void CWindow::ClearNotificationQueue()
{
	EnterCriticalSection(&this->m_NotificationCriticalSection);

	this->m_NotificationQueueHead = 0;
	this->m_NotificationQueueTail = 0;
	this->m_NotificationQueueCount = 0;
	this->m_NextNotificationTick = 0;

	LeaveCriticalSection(&this->m_NotificationCriticalSection);

	if (g_hWnd != NULL)
	{
		KillTimer(g_hWnd, WM_NOTIFICATION_TIMER);
	}
}

void CWindow::ProcessNotificationQueue()
{
	if (!this->IsNotificationAllowed())
	{
		this->ClearNotificationQueue();
		return;
	}

	DWORD Tick = GetTickCount();

	if (TickIsBefore(Tick, this->m_NextNotificationTick))
	{
		DWORD Delay = this->m_NextNotificationTick - Tick;

		SetTimer(g_hWnd, WM_NOTIFICATION_TIMER, (Delay == 0) ? 1 : Delay, NULL);
		return;
	}

	NOTIFICATION_MESSAGE Item;

	EnterCriticalSection(&this->m_NotificationCriticalSection);

	if (this->m_NotificationQueueCount == 0)
	{
		LeaveCriticalSection(&this->m_NotificationCriticalSection);
		return;
	}

	memcpy(&Item, &this->m_NotificationQueue[this->m_NotificationQueueHead], sizeof(Item));

	this->m_NotificationQueueHead = (this->m_NotificationQueueHead + 1) % NOTIFICATION_QUEUE_SIZE;
	this->m_NotificationQueueCount--;

	LeaveCriticalSection(&this->m_NotificationCriticalSection);

	if (!this->EnsureTrayIcon(TRAY_MODE_NOTIFICATION))
	{
		return;
	}

	if (!this->ShowTrayMessage(Item.Title, Item.Message))
	{
		return;
	}
	this->m_NextNotificationTick = GetTickCount() + 1200;

	EnterCriticalSection(&this->m_NotificationCriticalSection);
	bool More = (this->m_NotificationQueueCount != 0);
	LeaveCriticalSection(&this->m_NotificationCriticalSection);

	if (More)
	{
		SetTimer(g_hWnd, WM_NOTIFICATION_TIMER, 1200, NULL);
	}
}

void CWindow::RestoreFromNotification()
{
	this->m_TrayMode = TRAY_MODE_NONE;
	this->m_WindowMinimized = false;

	ShowWindow(g_hWnd, SW_SHOW);
	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);
	this->m_WindowActive = true;
	this->RemoveTrayIcon();
	this->ClearNotificationQueue();
}

void CWindow::HandleTaskbarCreated()
{
	this->m_TrayIconVisible = false;

	if (this->m_TrayMode != TRAY_MODE_NONE)
	{
		this->EnsureTrayIcon(this->m_TrayMode);
	}
}

bool CWindow::ShowTrayMessage(const char* Title, const char* Message)
{
	if (!this->IsNotificationAllowed())
	{
		return false;
	}

	NOTIFYICONDATAW Icon;

	memset(&Icon, 0, sizeof(Icon));

	Icon.cbSize = sizeof(Icon);
	Icon.hWnd = g_hWnd;
	Icon.uID = WM_TRAY_MODE_ICON;
	Icon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_INFO;
	Icon.hIcon = this->m_WindowIcon;
	Icon.uCallbackMessage = WM_TRAY_MODE_MESSAGE;
	Icon.dwInfoFlags = NIIF_INFO | NIIF_RESPECT_QUIET_TIME;
	Icon.uTimeout = 5000;

	CopyAnsiToWide(Message, Icon.szInfo, ARRAYSIZE(Icon.szInfo));
	CopyAnsiToWide(Title, Icon.szInfoTitle, ARRAYSIZE(Icon.szInfoTitle));

	if (!Shell_NotifyIconW(NIM_MODIFY, &Icon))
	{
#ifdef _DEBUG
		WindowDebug("Shell_NotifyIconW(NIM_MODIFY) failed: %lu", GetLastError());
#endif
		this->m_TrayIconVisible = false;
		return false;
	}

	return true;
}

void CWindow::ChangeWindowText()
{
	if (SceneFlag != MAIN_SCENE)
	{
		this->m_CharacterName[0] = 0;
		sprintf_s(this->m_WindowName, sizeof(this->m_WindowName), "%s", gProtect.m_MainInfo.WindowName);
	}
	else
	{
		STRUCT_DECRYPT;

		sprintf_s(this->m_CharacterName, sizeof(this->m_CharacterName), "%s", (char*)(CharacterAttribute + 0x00));
		sprintf_s(this->m_WindowName, sizeof(this->m_WindowName), "%s", this->m_CharacterName);

		if (!gProtect.m_MainInfo.DisableResets)
		{
			char Resets[64];
			sprintf_s(Resets, sizeof(Resets), " || Resets: %d", gPrintPlayer.ViewReset);

			strcat_s(this->m_WindowName, Resets);
		}

		if (!gProtect.m_MainInfo.DisableGrandResets)
		{
			char GrandResets[64];
			sprintf_s(GrandResets, sizeof(GrandResets), " || GrandResets: %d", gPrintPlayer.ViewGrandReset);

			strcat_s(this->m_WindowName, GrandResets);
		}

		char Text[128];
		sprintf_s(Text, sizeof(Text), " || Level: %d || PING: %u ms || FPS: %.0f", *(WORD*)(CharacterAttribute + 0x0E), gPing.m_Ping, FPS);

		strcat_s(this->m_WindowName, Text);

		STRUCT_ENCRYPT;
	}

	SetWindowText(g_hWnd, this->m_WindowName);
}

void CWindow::SetWindowMode(bool windowMode, bool borderless)
{
	this->m_WindowMode = windowMode;

	this->m_Borderless = borderless;
}

void CWindow::SetResolution(int res)
{
	if (res >= R640x480 && res < MAX_RESOLUTION_VALUE)
	{
		m_Resolution = res;

		WindowWidth = this->iResolutionValues[res].first;

		WindowHeight = this->iResolutionValues[res].second;

		g_fScreenRate_x = (float)WindowWidth / 640.0f;

		g_fScreenRate_y = (float)WindowHeight / 480.0f;
	}
}

void CWindow::ChangeWindowState(bool windowMode, bool borderless, int resolution)
{
	if (windowMode != this->m_WindowMode || borderless != this->m_Borderless)
	{
		if (!windowMode)
		{
			this->ChangeDisplaySettingsFunction();
		}
		else if (windowMode != this->m_WindowMode)
		{
			ChangeDisplaySettings(NULL, 0);
		}

		this->SetWindowMode(windowMode, borderless);
	}
	else if (resolution != m_Resolution)
	{
		this->SetResolution(resolution);

		if (!this->m_WindowMode)
		{
			this->ChangeDisplaySettingsFunction();
		}
	}

	if (!this->m_WindowMode || this->m_Borderless)
	{
		// Delete the window icon after changing styles in window mode
		SendMessage(g_hWnd, WM_SETICON, ICON_BIG, (LPARAM)NULL);

		SendMessage(g_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)NULL);
	}

	RECT rc = { 0, 0, WindowWidth, WindowHeight };

	LONG PosX = ((GetSystemMetrics(SM_CXSCREEN)) / 2) - (WindowWidth / 2);

	LONG PosY = ((GetSystemMetrics(SM_CYSCREEN)) / 2) - (WindowHeight / 2);

	if (this->m_WindowMode)
	{
		LONG STYLE = (this->m_Borderless)
			? WS_POPUP | WS_VISIBLE
			: WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;

		LONG EXSTYLE = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

		SetWindowLongPtr(g_hWnd, GWL_STYLE, STYLE); // Set the Window Style

		SetWindowLongPtr(g_hWnd, GWL_EXSTYLE, EXSTYLE); // Set the Window Extra Style

		AdjustWindowRect(&rc, STYLE, FALSE); // Adjust the rectangle inside

		if (!this->m_Borderless)
		{
			// Restore the window icon after changing styles in window mode
			SendMessage(g_hWnd, WM_SETICON, ICON_BIG, (LPARAM)this->m_WindowIcon);

			SendMessage(g_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)this->m_WindowIcon);
		}
	}
	else
	{
		LONG STYLE = WS_POPUP | WS_VISIBLE;

		LONG EXSTYLE = WS_EX_APPWINDOW;

		SetWindowLongPtr(g_hWnd, GWL_STYLE, STYLE); // Set the Window Style

		SetWindowLongPtr(g_hWnd, GWL_EXSTYLE, EXSTYLE); // Set the Window Extra Style

		AdjustWindowRect(&rc, STYLE, FALSE); // Adjust the rectangle inside
	}

	SetWindowPos(g_hWnd, NULL, PosX, PosY, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_FRAMECHANGED);

	MoveWindow(g_hWnd, PosX, PosY, rc.right - rc.left, rc.bottom - rc.top, TRUE); // Change the size

	gFont.ReloadFont();
}
