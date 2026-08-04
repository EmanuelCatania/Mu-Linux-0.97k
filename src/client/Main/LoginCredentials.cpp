#include "stdafx.h"
#include "LoginCredentials.h"
#include "Input.h"
#include "Language.h"
#include "Protect.h"
#include "ServerList.h"

// Prefer the SDK declaration when available. Some legacy build environments
// for this client do not ship wincred.h, while the APIs themselves are part of
// advapi32 (already linked by the target), so keep a local ABI fallback.
#if defined(__has_include)
#if __has_include(<wincred.h>)
#include <wincred.h>
#define LOGIN_CREDENTIAL_HEADER_AVAILABLE 1
#endif
#endif

#ifndef LOGIN_CREDENTIAL_HEADER_AVAILABLE
#define CRED_TYPE_GENERIC 1
#define CRED_PERSIST_LOCAL_MACHINE 2

typedef struct _CREDENTIAL_ATTRIBUTEW
{
	LPWSTR Keyword;
	DWORD Flags;
	DWORD ValueSize;
	LPBYTE Value;
} CREDENTIAL_ATTRIBUTEW, *PCREDENTIAL_ATTRIBUTEW;

typedef struct _CREDENTIALW
{
	DWORD Flags;
	DWORD Type;
	LPWSTR TargetName;
	LPWSTR Comment;
	FILETIME LastWritten;
	DWORD CredentialBlobSize;
	LPBYTE CredentialBlob;
	DWORD Persist;
	DWORD AttributeCount;
	PCREDENTIAL_ATTRIBUTEW Attributes;
	LPWSTR TargetAlias;
	LPWSTR UserName;
} CREDENTIALW, *PCREDENTIALW;

extern "C" BOOL WINAPI CredReadW(
	LPCWSTR TargetName,
	DWORD Type,
	DWORD Flags,
	PCREDENTIALW* Credential);

extern "C" BOOL WINAPI CredWriteW(
	PCREDENTIALW Credential,
	DWORD Flags);

extern "C" BOOL WINAPI CredDeleteW(
	LPCWSTR TargetName,
	DWORD Type,
	DWORD Flags);

extern "C" VOID WINAPI CredFree(PVOID Buffer);
#endif

CLoginCredentials gLoginCredentials;

static const char* LOGIN_CONFIG_PATH = ".\\Config.ini";

CLoginCredentials::CLoginCredentials()
{
	this->m_Initialized = false;
	this->m_LoginSceneInitialized = false;
	this->m_HideUsername = false;
	this->m_SaveRequested = false;
	this->m_HasSavedCredential = false;
	this->m_PlaceholderActive = false;
	this->m_SubmitPending = false;
	memset(&this->m_SubmitSnapshot, 0, sizeof(this->m_SubmitSnapshot));
}

CLoginCredentials::~CLoginCredentials()
{
	this->ClearSubmitSnapshot();
}

void CLoginCredentials::Init()
{
	if (this->m_Initialized != false)
	{
		return;
	}

	this->LoadConfig();

	// Stop the legacy plaintext username persistence from being recreated.
	WritePrivateProfileStringA("User", "Username", NULL, LOGIN_CONFIG_PATH);

	this->m_Initialized = true;
}

void CLoginCredentials::LoadConfig()
{
	this->m_HideUsername =
		(GetPrivateProfileIntA(
			"Login",
			"HideUsername",
			0,
		LOGIN_CONFIG_PATH) != 0);
}

bool CLoginCredentials::IsHideUsernameEnabled() const
{
	return this->m_HideUsername;
}

void CLoginCredentials::SetHideUsername(bool Enabled)
{
	this->m_HideUsername = Enabled;
	InputTextHide[0] = (Enabled ? 1 : 0);

	WritePrivateProfileStringA(
		"Login",
		"HideUsername",
		(Enabled ? "1" : "0"),
		LOGIN_CONFIG_PATH);
}

const char* CLoginCredentials::GetHideUsernameLabel() const
{
	switch (gLanguage.LangNum)
	{
		case LANGUAGE_PORTUGUESE:
			return "Ocultar usu\xE1rio";

		case LANGUAGE_SPANISH:
			return "Ocultar usuario";

		default:
			return "Hide username";
	}
}

const char* CLoginCredentials::GetSaveLoginLabel() const
{
	switch (gLanguage.LangNum)
	{
		case LANGUAGE_PORTUGUESE:
			return "Salvar login";

		case LANGUAGE_SPANISH:
			return "Guardar login";

		default:
			return "Save login";
	}
}

const char* CLoginCredentials::GetSavedLoginPlaceholderText() const
{
	switch (gLanguage.LangNum)
	{
		case LANGUAGE_PORTUGUESE:
			return "[Detalhes de login salvos]";

		case LANGUAGE_SPANISH:
			return "[Detalles de inicio guardados]";

		default:
			return "[Saved Login Details]";
	}
}

void CLoginCredentials::Prepare()
{
	if (this->m_Initialized == false || SceneFlag != LOG_IN_SCENE)
	{
		return;
	}

	if (this->m_LoginSceneInitialized == false)
	{
		this->LoadSavedCredentials();
		this->m_LoginSceneInitialized = true;
	}

	InputTextHide[0] = (this->m_HideUsername ? 1 : 0);
	InputTextHide[1] = 1;
}

bool CLoginCredentials::IsLoginInputActive() const
{
	return (SceneFlag == LOG_IN_SCENE &&
		InputEnable != false &&
		InputNumber == 2 &&
		InputIndex >= 0 &&
		InputIndex <= 1);
}

void CLoginCredentials::LoadSavedCredentials()
{
	LOGIN_RECORD Record;
	memset(&Record, 0, sizeof(Record));

	this->m_HasSavedCredential = this->ReadCredential(&Record);

	if (this->m_HasSavedCredential == false)
	{
		// Treat malformed or stale records as absent instead of repeatedly
		// attempting to load them on every login scene.
		this->DeleteCredential();
	}

	this->m_SaveRequested = this->m_HasSavedCredential;
	this->m_PlaceholderActive = false;

	if (this->m_HasSavedCredential != false)
	{
		gInput.SetLoginInputText(0, Record.Account);
		gInput.SetLoginInputText(1, Record.Password);
		this->m_PlaceholderActive = true;
	}

	gInput.SetLoginPlaceholder(this->m_PlaceholderActive);

	SecureZeroMemory(&Record, sizeof(Record));
}

bool CLoginCredentials::BuildTargetName(
	wchar_t* Target,
	DWORD TargetCapacity) const
{
	if (Target == NULL || TargetCapacity == 0)
	{
		return false;
	}

	Target[0] = L'\0';

	int IpLength = (int)strnlen_s(
		gProtect.m_MainInfo.IpAddress,
		sizeof(gProtect.m_MainInfo.IpAddress));

	int SerialLength = (int)strnlen_s(
		gProtect.m_MainInfo.ClientSerial,
		sizeof(gProtect.m_MainInfo.ClientSerial));

	if (IpLength <= 0 || IpLength >= (int)sizeof(gProtect.m_MainInfo.IpAddress) ||
		SerialLength <= 0 || SerialLength >= (int)sizeof(gProtect.m_MainInfo.ClientSerial))
	{
		return false;
	}

	char TargetAnsi[LOGIN_TARGET_SIZE];
	memset(TargetAnsi, 0, sizeof(TargetAnsi));

	int Length = sprintf_s(
		TargetAnsi,
		sizeof(TargetAnsi),
		"MU097K.Login.v1|%.*s:%u|%.*s",
		IpLength,
		gProtect.m_MainInfo.IpAddress,
		(unsigned int)gProtect.m_MainInfo.IpAddressPort,
		SerialLength,
		gProtect.m_MainInfo.ClientSerial);

	if (Length <= 0)
	{
		SecureZeroMemory(TargetAnsi, sizeof(TargetAnsi));
		return false;
	}

	int Converted = MultiByteToWideChar(
		CP_ACP,
		MB_PRECOMPOSED,
		TargetAnsi,
		-1,
		Target,
		(int)TargetCapacity);

	SecureZeroMemory(TargetAnsi, sizeof(TargetAnsi));

	if (Converted <= 0)
	{
		Target[0] = L'\0';
		return false;
	}

	Target[TargetCapacity - 1] = L'\0';
	return true;
}

bool CLoginCredentials::ReadCredential(LOGIN_RECORD* Record)
{
	if (Record == NULL)
	{
		return false;
	}

	SecureZeroMemory(Record, sizeof(*Record));

	wchar_t Target[LOGIN_TARGET_SIZE];
	memset(Target, 0, sizeof(Target));

	if (this->BuildTargetName(Target, ARRAYSIZE(Target)) == false)
	{
		return false;
	}

	PCREDENTIALW Credential = NULL;
	bool Result = false;

	if (CredReadW(Target, CRED_TYPE_GENERIC, 0, &Credential) != FALSE &&
		Credential != NULL &&
		Credential->CredentialBlobSize == LOGIN_BLOB_SIZE &&
		Credential->CredentialBlob != NULL)
	{
		const BYTE* Blob = Credential->CredentialBlob;

		if (Blob[0] == LOGIN_CREDENTIAL_VERSION)
		{
			const void* AccountTerminator = memchr(
				Blob + 1,
				'\0',
				LOGIN_TEXT_SIZE);

			const void* PasswordTerminator = memchr(
				Blob + 1 + LOGIN_TEXT_SIZE,
				'\0',
				LOGIN_TEXT_SIZE);

			if (AccountTerminator != NULL && PasswordTerminator != NULL)
			{
				memcpy(Record->Account, Blob + 1, LOGIN_TEXT_SIZE);
				memcpy(Record->Password, Blob + 1 + LOGIN_TEXT_SIZE, LOGIN_TEXT_SIZE);

				int AccountLength = (int)strnlen_s(Record->Account, LOGIN_TEXT_SIZE);
				int PasswordLength = (int)strnlen_s(Record->Password, LOGIN_TEXT_SIZE);

				Result = (AccountLength > 0 && AccountLength < LOGIN_TEXT_SIZE &&
					PasswordLength > 0 && PasswordLength < LOGIN_TEXT_SIZE);
			}
		}
	}

	if (Credential != NULL)
	{
		CredFree(Credential);
	}

	SecureZeroMemory(Target, sizeof(Target));

	if (Result == false)
	{
		SecureZeroMemory(Record, sizeof(*Record));
	}

	return Result;
}

bool CLoginCredentials::WriteCredential(const LOGIN_RECORD* Record)
{
	if (Record == NULL ||
		strnlen_s(Record->Account, LOGIN_TEXT_SIZE) == 0 ||
		strnlen_s(Record->Account, LOGIN_TEXT_SIZE) >= LOGIN_TEXT_SIZE ||
		strnlen_s(Record->Password, LOGIN_TEXT_SIZE) == 0 ||
		strnlen_s(Record->Password, LOGIN_TEXT_SIZE) >= LOGIN_TEXT_SIZE)
	{
		return false;
	}

	wchar_t Target[LOGIN_TARGET_SIZE];
	memset(Target, 0, sizeof(Target));

	if (this->BuildTargetName(Target, ARRAYSIZE(Target)) == false)
	{
		return false;
	}

	BYTE Blob[LOGIN_BLOB_SIZE];
	memset(Blob, 0, sizeof(Blob));
	Blob[0] = LOGIN_CREDENTIAL_VERSION;
	memcpy(Blob + 1, Record->Account, LOGIN_TEXT_SIZE);
	memcpy(Blob + 1 + LOGIN_TEXT_SIZE, Record->Password, LOGIN_TEXT_SIZE);

	CREDENTIALW Credential;
	memset(&Credential, 0, sizeof(Credential));
	Credential.Type = CRED_TYPE_GENERIC;
	Credential.TargetName = Target;
	Credential.CredentialBlobSize = sizeof(Blob);
	Credential.CredentialBlob = Blob;
	Credential.Persist = CRED_PERSIST_LOCAL_MACHINE;

	bool Result = (CredWriteW(&Credential, 0) != FALSE);

	SecureZeroMemory(&Credential, sizeof(Credential));
	SecureZeroMemory(Blob, sizeof(Blob));
	SecureZeroMemory(Target, sizeof(Target));

	return Result;
}

void CLoginCredentials::DeleteCredential()
{
	wchar_t Target[LOGIN_TARGET_SIZE];
	memset(Target, 0, sizeof(Target));

	if (this->BuildTargetName(Target, ARRAYSIZE(Target)) != false)
	{
		CredDeleteW(Target, CRED_TYPE_GENERIC, 0);
	}

	SecureZeroMemory(Target, sizeof(Target));
	this->m_HasSavedCredential = false;
}

bool CLoginCredentials::CaptureFields(LOGIN_RECORD* Record) const
{
	if (Record == NULL)
	{
		return false;
	}

	SecureZeroMemory(Record, sizeof(*Record));

	for (int Index = 0; Index < 2; Index++)
	{
		int Length = (int)strnlen_s(InputText[Index], LOGIN_TEXT_SIZE);

		if (Length <= 0 || Length >= LOGIN_TEXT_SIZE)
		{
			SecureZeroMemory(Record, sizeof(*Record));
			return false;
		}

		char* Destination = ((Index == 0) ? Record->Account : Record->Password);
		memcpy(Destination, InputText[Index], Length);
		Destination[Length] = '\0';
	}

	return true;
}

void CLoginCredentials::ClearSubmitSnapshot()
{
	SecureZeroMemory(&this->m_SubmitSnapshot, sizeof(this->m_SubmitSnapshot));
	this->m_SubmitPending = false;
}

void CLoginCredentials::OnNativeLoginSubmit()
{
	LOGIN_RECORD Record;
	memset(&Record, 0, sizeof(Record));

	gLoginCredentials.ClearSubmitSnapshot();
	gLoginCredentials.m_SubmitPending = gLoginCredentials.CaptureFields(&Record);

	if (gLoginCredentials.m_SubmitPending != false)
	{
		memcpy(&gLoginCredentials.m_SubmitSnapshot, &Record, sizeof(Record));
	}

	SecureZeroMemory(&Record, sizeof(Record));
}

void CLoginCredentials::OnConnectAccountResult(BYTE Result)
{
	if (Result == 1)
	{
		if (this->m_SaveRequested != false &&
			this->m_SubmitPending != false)
		{
			if (this->WriteCredential(&this->m_SubmitSnapshot) != false)
			{
				this->m_HasSavedCredential = true;
			}
		}
	}
	else if ((Result == 0 || Result == 2) &&
		this->m_SubmitPending != false)
	{
		// A reconnect response has no pending foreground submit and must not
		// alter the user's saved-login record.
		this->DeleteCredential();
		this->m_SaveRequested = false;
		this->m_PlaceholderActive = false;
		gInput.SetLoginPlaceholder(false);
	}

	this->ClearSubmitSnapshot();
}

void CLoginCredentials::OnDisconnect()
{
	this->ClearSubmitSnapshot();
}

void CLoginCredentials::BeginEdit()
{
	if (this->m_PlaceholderActive == false)
	{
		return;
	}

	this->m_PlaceholderActive = false;
	gInput.SetLoginPlaceholder(false);
	gInput.SetLoginInputText(0, "");
	gInput.SetLoginInputText(1, "");
}

bool CLoginCredentials::HandleKeyDown(WPARAM wParam)
{
	if (wParam == VK_RETURN && this->IsLoginInputActive() != false)
	{
		CLoginCredentials::OnNativeLoginSubmit();
	}

	if (this->m_PlaceholderActive == false ||
		this->IsLoginInputActive() == false)
	{
		return false;
	}

	if (wParam == VK_RETURN || wParam == VK_TAB ||
		wParam == VK_LEFT || wParam == VK_RIGHT ||
		wParam == VK_HOME || wParam == VK_END ||
		wParam == VK_UP || wParam == VK_DOWN ||
		wParam == VK_ESCAPE)
	{
		return false;
	}

	if ((GetKeyState(VK_CONTROL) & 0x8000) != 0 &&
		(wParam == 'A' || wParam == 'C'))
	{
		// Do not allow selection/copy to expose the real loaded credentials
		// while the placeholder is active.
		return true;
	}

	if (wParam == VK_BACK || wParam == VK_DELETE ||
		wParam == VK_INSERT ||
		((GetKeyState(VK_CONTROL) & 0x8000) != 0 &&
		 (wParam == 'V' || wParam == 'X')))
	{
		this->BeginEdit();
	}

	return false;
}

bool CLoginCredentials::HandleChar(WPARAM wParam)
{
	if (this->m_PlaceholderActive == false ||
		this->IsLoginInputActive() == false)
	{
		return false;
	}

	if (wParam == VK_BACK || (wParam >= 0x20 && wParam <= 0xFF))
	{
		this->BeginEdit();
	}

	return false;
}

void CLoginCredentials::SetSaveRequested(bool Enabled)
{
	this->m_SaveRequested = Enabled;

	if (Enabled == false)
	{
		this->DeleteCredential();
	}
}

bool CLoginCredentials::IsInsideCheckbox(
	int X,
	int Y,
	int* Checkbox) const
{
	if (Checkbox == NULL || SceneFlag != LOG_IN_SCENE)
	{
		return false;
	}

	int PanelY = LoginPanelY;

	if (Y < PanelY + 110 || Y >= PanelY + 141)
	{
		return false;
	}

	if (X >= 245 && X < 322)
	{
		*Checkbox = 0;
		return true;
	}

	if (X >= 325 && X < 405)
	{
		*Checkbox = 1;
		return true;
	}

	return false;
}

bool CLoginCredentials::HandleLeftButtonDown(HWND hwnd, LPARAM lParam)
{
	if (hwnd == NULL)
	{
		return false;
	}

	RECT ClientRect;
	GetClientRect(hwnd, &ClientRect);

	int ClientWidth = ClientRect.right - ClientRect.left;
	int ClientHeight = ClientRect.bottom - ClientRect.top;

	if (ClientWidth <= 0 || ClientHeight <= 0)
	{
		return false;
	}

	int X = MulDiv((short)LOWORD(lParam), 640, ClientWidth);
	int Y = MulDiv((short)HIWORD(lParam), 480, ClientHeight);
	int Checkbox = -1;

	if (this->IsInsideCheckbox(X, Y, &Checkbox) == false)
	{
		if (SceneFlag == LOG_IN_SCENE)
		{
			CLoginCredentials::OnNativeLoginSubmit();
		}

		return false;
	}

	if (Checkbox == 0)
	{
		this->SetHideUsername(!this->m_HideUsername);
	}
	else
	{
		this->SetSaveRequested(!this->m_SaveRequested);
	}

	return true;
}

void CLoginCredentials::RenderCheckbox(
	int X,
	int Y,
	bool Checked,
	const char* Text)
{
	char Label[128];
	memset(Label, 0, sizeof(Label));

	sprintf_s(
		Label,
		sizeof(Label),
		"[%c] %s",
		(Checked ? 'X' : ' '),
		Text);

	RenderText(X, Y, Label, 0, RT3_SORT_LEFT, NULL);
	SecureZeroMemory(Label, sizeof(Label));
}

void CLoginCredentials::RenderSelectedServer()
{
	char ServerName[32] = { 0 };
	BYTE Percent = 0;

	if (gServerList.GetSelectedServerInfo(
		ServerName,
		(int)sizeof(ServerName),
		&Percent) == false)
	{
		return;
	}

	char Text[96] = { 0 };

	if (gLanguage.LangNum == LANGUAGE_PORTUGUESE)
	{
		sprintf_s(Text, "%s: %s (%u%%)", "Servidor", ServerName, (unsigned int)Percent);
	}
	else if (gLanguage.LangNum == LANGUAGE_SPANISH)
	{
		sprintf_s(Text, "%s: %s (%u%%)", "Servidor", ServerName, (unsigned int)Percent);
	}
	else
	{
		sprintf_s(Text, "%s: %s (%u%%)", "Server", ServerName, (unsigned int)Percent);
	}

	DWORD BackupTextColor = SetTextColor;
	DWORD BackupBackgroundTextColor = SetBackgroundTextColor;

	EnableAlphaTest(true);
	SetBackgroundTextColor = Color4b(255, 255, 255, 0);
	SetTextColor = Color4b(255, 230, 210, 255);

	int PanelY = LoginPanelY;
	RenderText(220, PanelY - 28, Text, REAL_WIDTH(200), RT3_SORT_CENTER, NULL);

	SetTextColor = BackupTextColor;
	SetBackgroundTextColor = BackupBackgroundTextColor;

	SecureZeroMemory(Text, sizeof(Text));
	SecureZeroMemory(ServerName, sizeof(ServerName));
}

void CLoginCredentials::RenderLoginOptions()
{
	if (this->m_Initialized == false || SceneFlag != LOG_IN_SCENE)
	{
		return;
	}

	DWORD BackupTextColor = SetTextColor;
	DWORD BackupBackgroundTextColor = SetBackgroundTextColor;

	EnableAlphaTest(true);
	SetBackgroundTextColor = Color4b(255, 255, 255, 0);
	SetTextColor = Color4b(255, 230, 210, 255);

	this->RenderSelectedServer();

	int PanelY = LoginPanelY;
	const int CheckboxY = PanelY + 122;
	this->RenderCheckbox(250, CheckboxY, this->m_HideUsername, this->GetHideUsernameLabel());
	this->RenderCheckbox(328, CheckboxY, this->m_SaveRequested, this->GetSaveLoginLabel());

	SetTextColor = BackupTextColor;
	SetBackgroundTextColor = BackupBackgroundTextColor;
}

void CLoginCredentials::Render()
{
	if (this->m_Initialized == false)
	{
		return;
	}

	if (SceneFlag != LOG_IN_SCENE)
	{
		if (this->m_LoginSceneInitialized != false)
		{
			// The native fields are no longer needed after leaving the login
			// scene. Clear them so the transient plaintext does not survive
			// into character/game scenes; the saved record remains in the
			// Credential Manager and will be loaded on the next login scene.
			gInput.SetLoginInputText(0, "");
			gInput.SetLoginInputText(1, "");
		}

		this->m_LoginSceneInitialized = false;
		this->m_PlaceholderActive = false;
		gInput.SetLoginPlaceholder(false);
		InputTextHide[0] = 0;
		InputTextHide[1] = 0;
		this->ClearSubmitSnapshot();
		return;
	}

	// Login text and options are rendered by the native input hooks. Avoid
	// issuing an additional RenderText call from the outer scene hook; some
	// revisions of the main executable do not tolerate that extra render pass
	// while transitioning from the Webzen scene.
}
