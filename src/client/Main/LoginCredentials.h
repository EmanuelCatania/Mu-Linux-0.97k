#pragma once

class CLoginCredentials
{
public:

	CLoginCredentials();

	~CLoginCredentials();

	void Init();

	void Prepare();

	void Render();

	bool HandleKeyDown(WPARAM wParam);

	bool HandleChar(WPARAM wParam);

	bool HandleLeftButtonDown(HWND hwnd, LPARAM lParam);

	void OnConnectAccountResult(BYTE Result);

	void OnDisconnect();

	void RenderLoginOptions();

	void RenderSelectedServer();

	bool IsHideUsernameEnabled() const;

	void SetHideUsername(bool Enabled);

	const char* GetHideUsernameLabel() const;

	const char* GetSaveLoginLabel() const;

	const char* GetSavedLoginPlaceholderText() const;

	static void OnNativeLoginSubmit();

private:

	enum
	{
		LOGIN_TEXT_SIZE = 11,
		LOGIN_TARGET_SIZE = 256,
		LOGIN_BLOB_SIZE = 1 + LOGIN_TEXT_SIZE + LOGIN_TEXT_SIZE,
		LOGIN_CREDENTIAL_VERSION = 1
	};

	struct LOGIN_RECORD
	{
		char Account[LOGIN_TEXT_SIZE];
		char Password[LOGIN_TEXT_SIZE];
	};

	bool IsLoginInputActive() const;

	void LoadConfig();

	void LoadSavedCredentials();

	bool ReadCredential(LOGIN_RECORD* Record);

	bool WriteCredential(const LOGIN_RECORD* Record);

	void DeleteCredential();

	bool BuildTargetName(wchar_t* Target, DWORD TargetCapacity) const;

	bool CaptureFields(LOGIN_RECORD* Record) const;

	void ClearSubmitSnapshot();

	void BeginEdit();

	void SetSaveRequested(bool Enabled);

	void RenderCheckbox(int X, int Y, bool Checked, const char* Text);

	bool IsInsideCheckbox(int X, int Y, int* Checkbox) const;

	bool m_Initialized;

	bool m_LoginSceneInitialized;

	bool m_HideUsername;

	bool m_SaveRequested;

	bool m_HasSavedCredential;

	bool m_PlaceholderActive;

	bool m_SubmitPending;

	LOGIN_RECORD m_SubmitSnapshot;
};

extern CLoginCredentials gLoginCredentials;
