#pragma once

enum eInputContext
{
	INPUT_CONTEXT_NONE = 0,
	INPUT_CONTEXT_CHAT,
	INPUT_CONTEXT_LOGIN
};

class CInput
{
public:

	CInput();

	~CInput();

	bool Init();

	bool HandleKeyDown(WPARAM wParam);

	bool HandleChar(WPARAM wParam);

private:

	enum
	{
		MAX_INPUT_TEXTS = 10,
		INPUT_TEXT_SIZE = 256,
		MAX_INPUT_HISTORY = 50,
		INPUT_CODE_PAGE = 1252
	};

	static void RenderChatInputTextHook(
		int X,
		int Y,
		int Index);

	static void RenderLoginInputTextHook(
		int X,
		int Y,
		int Index);

	bool IsSupportedClient();

	bool GetActiveContext(eInputContext* Context, int* Index);

	bool IsActiveInput(eInputContext Context, int Index);

	bool IsValidInputIndex(int Index);

	int GetInputLength(int Index);

	int GetInputLimit(int Index);

	void SyncCaret(eInputContext Context, int Index);

	void SetCaretPosition(int Index, int Position);

	void MoveCaret(int Index, int Position, bool ExtendSelection);

	void ClearSelection(int Index);

	bool HasSelection(int Index);

	int GetSelectionStart(int Index);

	int GetSelectionEnd(int Index);

	void UpdateInputLength(int Index);

	void UpdateViewStart(int Index);

	void BuildVisibleText(
		int Index,
		int Start,
		int End,
		char* Output,
		int OutputSize);

	void RenderInputTextManaged(
		eInputContext Context,
		int X,
		int Y,
		int Index);

	void RenderCaret(
		int X,
		int Y,
		int Index);

	void RenderSelection(
		int X,
		int Y,
		int Index);

	void InsertCharacter(int Index, char Character);

	void InsertText(int Index, const char* Text);

	bool DeleteSelection(int Index);

	void DeleteCharacterBeforeCaret(int Index);

	void DeleteCharacterAtCaret(int Index);

	void ResetHistoryNavigation();

	void PushHistory(const char* Text);

	void BrowseHistory(int Direction, int Index);

	void SetInputText(int Index, const char* Text);

	bool CopySelection(int Index);

	bool SetClipboardText(const char* Text, int Length);

	bool GetClipboardText(char* Text, int TextSize);

	bool CanCopySelection(eInputContext Context, int Index);

	int m_CaretPosition[MAX_INPUT_TEXTS];

	int m_SelectionAnchor[MAX_INPUT_TEXTS];

	int m_LastLength[MAX_INPUT_TEXTS];

	int m_RenderWidth[MAX_INPUT_TEXTS];

	int m_ViewStart[MAX_INPUT_TEXTS];

	eInputContext m_ActiveContext;

	eInputContext m_LastFocusedContext;

	int m_LastFocusedIndex;

	DWORD m_LastRenderTick;

	char m_History[MAX_INPUT_HISTORY][INPUT_TEXT_SIZE];

	int m_HistoryCount;

	int m_HistoryPosition;

	bool m_HistoryBrowsing;

	char m_HistoryDraft[INPUT_TEXT_SIZE];
};

extern CInput gInput;
