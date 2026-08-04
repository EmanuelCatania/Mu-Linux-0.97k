#include "stdafx.h"
#include "Input.h"
#include "LoginCredentials.h"

CInput gInput;

CInput::CInput()
{
	memset(this->m_CaretPosition, 0, sizeof(this->m_CaretPosition));

	memset(this->m_SelectionAnchor, 0, sizeof(this->m_SelectionAnchor));

	memset(this->m_RenderWidth, 0, sizeof(this->m_RenderWidth));

	memset(this->m_ViewStart, 0, sizeof(this->m_ViewStart));

	memset(this->m_History, 0, sizeof(this->m_History));

	memset(this->m_HistoryDraft, 0, sizeof(this->m_HistoryDraft));

	for (int n = 0; n < MAX_INPUT_TEXTS; n++)
	{
		this->m_LastLength[n] = -1;
	}

	this->m_ActiveContext = INPUT_CONTEXT_NONE;

	this->m_LastFocusedContext = INPUT_CONTEXT_NONE;

	this->m_LastFocusedIndex = -1;

	this->m_LastRenderTick = 0;

	this->m_HistoryCount = 0;

	this->m_HistoryPosition = 0;

	this->m_HistoryBrowsing = false;

	this->m_TokenActive = false;

	this->m_TokenStart = 0;

	this->m_TokenLength = 0;

	this->m_TokenValue = 0;

	memset(this->m_TokenText, 0, sizeof(this->m_TokenText));

	this->m_LoginPlaceholder = false;
}

CInput::~CInput()
{

}

void CInput::Init()
{
	SetCompleteHook(
		0xE8,
		ChatInputTextCall,
		&CInput::RenderChatInputTextHook);

	SetCompleteHook(
		0xE8,
		ChatInputWhisperCall,
		&CInput::RenderChatInputTextHook);

	SetCompleteHook(
		0xE8,
		LoginInputAccountCall,
		&CInput::RenderLoginInputTextHook);

	SetCompleteHook(
		0xE8,
		LoginInputPasswordCall,
		&CInput::RenderLoginInputTextHook);

}

void CInput::SetLoginInputText(int Index, const char* Text)
{
	if (Index < 0 || Index > 1 || Text == NULL)
	{
		return;
	}

	this->SetInputText(Index, Text);
}

void CInput::SetLoginPlaceholder(bool Enabled)
{
	this->m_LoginPlaceholder = Enabled;
}

void CInput::RenderChatInputTextHook(
	int X,
	int Y,
	int Index)
{
	gInput.RenderInputTextManaged(
		INPUT_CONTEXT_CHAT,
		X,
		Y,
		Index);
}

void CInput::RenderLoginInputTextHook(
	int X,
	int Y,
	int Index)
{
	gInput.RenderInputTextManaged(
		INPUT_CONTEXT_LOGIN,
		X,
		Y,
		Index);

	// The native login routine draws its controls after the text fields. Keep
	// the custom options in that same pass so they remain visible on builds
	// where the outer scene hook is followed by another native clear.
	if (Index == 1)
	{
		gLoginCredentials.RenderLoginOptions();
	}
}

bool CInput::GetActiveContext(eInputContext* Context, int* Index)
{
	if (Context == NULL || Index == NULL)
	{
		return false;
	}

	if ((GetTickCount() - this->m_LastRenderTick) > 500)
	{
		return false;
	}

	if (this->IsActiveInput(
		this->m_ActiveContext,
		InputIndex) == false)
	{
		return false;
	}

	*Context = this->m_ActiveContext;

	*Index = InputIndex;

	return true;
}

bool CInput::IsActiveInput(eInputContext Context, int Index)
{
	if (this->IsValidInputIndex(Index) == false || Index > 1)
	{
		return false;
	}

	if (Index != InputIndex)
	{
		return false;
	}

	if (Context == INPUT_CONTEXT_CHAT)
	{
		return (SceneFlag == MAIN_SCENE &&
			g_bGameServerConnected != FALSE &&
			(InputEnable != false || TabInputEnable != false));
	}

	if (Context == INPUT_CONTEXT_LOGIN)
	{
		return (SceneFlag == LOG_IN_SCENE &&
			InputEnable != false);
	}

	return false;
}

bool CInput::IsValidInputIndex(int Index)
{
	return (Index >= 0 && Index < MAX_INPUT_TEXTS);
}

int CInput::GetInputLength(int Index)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return 0;
	}

	InputText[Index][INPUT_TEXT_SIZE - 1] = '\0';

	return (int)strnlen_s(
		InputText[Index],
		INPUT_TEXT_SIZE);
}

int CInput::GetInputLimit(int Index)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return 0;
	}

	int Limit = InputTextMax[Index];

	if (Limit <= 0 || Limit >= INPUT_TEXT_SIZE)
	{
		Limit = INPUT_TEXT_SIZE - 1;
	}

	return Limit;
}

void CInput::SyncCaret(eInputContext Context, int Index)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return;
	}

	int Length = this->GetInputLength(Index);

	if (Index != InputIndex)
	{
		if (this->m_LastLength[Index] != Length)
		{
			this->m_CaretPosition[Index] = Length;

			this->m_SelectionAnchor[Index] = Length;

			this->m_ViewStart[Index] = 0;

			this->m_LastLength[Index] = Length;
		}

		this->SetCaretPosition(
			Index,
			this->m_CaretPosition[Index]);

		return;
	}

	if (this->m_LastFocusedContext != Context ||
		this->m_LastFocusedIndex != Index ||
		this->m_LastLength[Index] != Length)
	{
		this->m_CaretPosition[Index] = Length;

		this->m_SelectionAnchor[Index] = Length;

		this->m_ViewStart[Index] = 0;

		this->m_LastLength[Index] = Length;

		this->m_LastFocusedContext = Context;

		this->m_LastFocusedIndex = Index;
	}

	this->SetCaretPosition(
		Index,
		this->m_CaretPosition[Index]);
}

void CInput::SetCaretPosition(int Index, int Position)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return;
	}

	int Length = this->GetInputLength(Index);

	if (Position < 0)
	{
		Position = 0;
	}

	if (Position > Length)
	{
		Position = Length;
	}

	this->m_CaretPosition[Index] = Position;

	if (this->m_SelectionAnchor[Index] < 0)
	{
		this->m_SelectionAnchor[Index] = 0;
	}

	if (this->m_SelectionAnchor[Index] > Length)
	{
		this->m_SelectionAnchor[Index] = Length;
	}

	this->UpdateViewStart(Index);
}

void CInput::MoveCaret(
	int Index,
	int Position,
	bool ExtendSelection)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return;
	}

	this->SetCaretPosition(Index, Position);

	if (ExtendSelection == false)
	{
		this->ClearSelection(Index);
	}
}

void CInput::ClearSelection(int Index)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return;
	}

	this->m_SelectionAnchor[Index] =
		this->m_CaretPosition[Index];
}

bool CInput::HasSelection(int Index)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return false;
	}

	return (this->m_SelectionAnchor[Index] !=
		this->m_CaretPosition[Index]);
}

int CInput::GetSelectionStart(int Index)
{
	if (this->HasSelection(Index) == false)
	{
		return this->m_CaretPosition[Index];
	}

	return min(
		this->m_SelectionAnchor[Index],
		this->m_CaretPosition[Index]);
}

int CInput::GetSelectionEnd(int Index)
{
	if (this->HasSelection(Index) == false)
	{
		return this->m_CaretPosition[Index];
	}

	return max(
		this->m_SelectionAnchor[Index],
		this->m_CaretPosition[Index]);
}

void CInput::UpdateInputLength(int Index)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return;
	}

	int Length = this->GetInputLength(Index);

	InputLength[Index] = Length;

	this->m_LastLength[Index] = Length;
}

void CInput::UpdateViewStart(int Index)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return;
	}

	int CaretPosition = this->m_CaretPosition[Index];

	int ViewStart = this->m_ViewStart[Index];

	if (ViewStart < 0 || ViewStart > CaretPosition)
	{
		ViewStart = CaretPosition;
	}

	int AvailableWidth = this->m_RenderWidth[Index];

	char CaretText[] = "_";

	AvailableWidth -= GetTextWidth(CaretText);

	if (AvailableWidth <= 0)
	{
		this->m_ViewStart[Index] = 0;

		return;
	}

	char VisibleText[INPUT_TEXT_SIZE];

	while (ViewStart < CaretPosition)
	{
		this->BuildVisibleText(
			Index,
			ViewStart,
			CaretPosition,
			VisibleText,
			sizeof(VisibleText));

		if (GetTextWidth(VisibleText) < AvailableWidth)
		{
			break;
		}

		ViewStart++;
	}

	while (ViewStart > 0)
	{
		this->BuildVisibleText(
			Index,
			ViewStart - 1,
			CaretPosition,
			VisibleText,
			sizeof(VisibleText));

		if (GetTextWidth(VisibleText) >= AvailableWidth)
		{
			break;
		}

		ViewStart--;
	}

	this->m_ViewStart[Index] = ViewStart;
}

void CInput::BuildVisibleText(
	int Index,
	int Start,
	int End,
	char* Output,
	int OutputSize)
{
	if (Output == NULL || OutputSize <= 0)
	{
		return;
	}

	Output[0] = '\0';

	if (this->IsValidInputIndex(Index) == false)
	{
		return;
	}

	int Length = this->GetInputLength(Index);

	if (Start < 0)
	{
		Start = 0;
	}

	if (End < Start || End > Length)
	{
		End = Length;
	}

	int OutputIndex = 0;

	for (int n = Start; n < End && OutputIndex < (OutputSize - 1); n++)
	{
		if (InputTextHide[Index] == 1)
		{
			Output[OutputIndex++] = '*';
		}
		else if (InputTextHide[Index] == 2 && n >= 7)
		{
			Output[OutputIndex++] = '*';
		}
		else
		{
			Output[OutputIndex++] = InputText[Index][n];
		}
	}

	Output[OutputIndex] = '\0';
}

void CInput::RenderInputTextManaged(
	eInputContext Context,
	int X,
	int Y,
	int Index)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		RenderInputText(X, Y, Index);

		return;
	}

	this->m_ActiveContext = Context;

	this->m_LastRenderTick = GetTickCount();

	bool ActiveInput = this->IsActiveInput(Context, Index);

	if (Context == INPUT_CONTEXT_CHAT && WindowWidth > 0)
	{
		this->m_RenderWidth[Index] =
			(640 * InputTextWidth) / WindowWidth;
	}
	else
	{
		this->m_RenderWidth[Index] = InputTextWidth;
	}

	this->SyncCaret(Context, Index);

	this->UpdateViewStart(Index);

	if (ActiveInput != false &&
		(Context != INPUT_CONTEXT_LOGIN || this->m_LoginPlaceholder == false))
	{
		this->RenderSelection(X, Y, Index);
	}

	char OriginalText[INPUT_TEXT_SIZE];

	char VisibleText[INPUT_TEXT_SIZE];

	memcpy(
		OriginalText,
		InputText[Index],
		sizeof(OriginalText));

	if (Context == INPUT_CONTEXT_LOGIN && this->m_LoginPlaceholder != false)
	{
		strncpy_s(
			VisibleText,
			sizeof(VisibleText),
			gLoginCredentials.GetSavedLoginPlaceholderText(),
			_TRUNCATE);
	}
	else
	{
		this->BuildVisibleText(
			Index,
			this->m_ViewStart[Index],
			this->GetInputLength(Index),
			VisibleText,
			sizeof(VisibleText));
	}

	BYTE OriginalHide = InputTextHide[Index];

	int OriginalInputIndex = InputIndex;
	int OriginalInputLength = InputLength[Index];

	strncpy_s(
		InputText[Index],
		INPUT_TEXT_SIZE,
		VisibleText,
		_TRUNCATE);

	InputTextHide[Index] = 0;

	InputIndex = -1;

	if (Context == INPUT_CONTEXT_LOGIN && this->m_LoginPlaceholder != false)
	{
		// The native input renderer applies a horizontal view offset intended
		// for editable text. That offset clips the leading '[' of the visual
		// placeholder on some client revisions. Draw this read-only label
		// directly while keeping the real field untouched.
		RenderText(X, Y, VisibleText, 0, RT3_SORT_LEFT, NULL);
	}
	else
	{
		RenderInputText(X, Y, Index);
	}

	InputIndex = OriginalInputIndex;
	InputLength[Index] = OriginalInputLength;

	InputTextHide[Index] = OriginalHide;

	memcpy(
		InputText[Index],
		OriginalText,
		sizeof(OriginalText));

	if (ActiveInput != false &&
		(Context != INPUT_CONTEXT_LOGIN || this->m_LoginPlaceholder == false))
	{
		this->RenderCaret(X, Y, Index);
	}

	SecureZeroMemory(VisibleText, sizeof(VisibleText));

	SecureZeroMemory(OriginalText, sizeof(OriginalText));
}

void CInput::RenderCaret(
	int X,
	int Y,
	int Index)
{
	if (Index != InputIndex)
	{
		return;
	}

	if (((GetTickCount() / 500) % 2) != 0)
	{
		return;
	}

	char TextBeforeCaret[INPUT_TEXT_SIZE];

	this->BuildVisibleText(
		Index,
		this->m_ViewStart[Index],
		this->m_CaretPosition[Index],
		TextBeforeCaret,
		sizeof(TextBeforeCaret));

	int TextWidth = GetTextWidth(TextBeforeCaret);

	RenderText(
		X + TextWidth,
		Y,
		"_",
		0,
		RT3_SORT_LEFT,
		NULL);

	SecureZeroMemory(
		TextBeforeCaret,
		sizeof(TextBeforeCaret));
}

void CInput::RenderSelection(
	int X,
	int Y,
	int Index)
{
	if (Index != InputIndex ||
		this->HasSelection(Index) == false)
	{
		return;
	}

	int SelectionStart = max(
		this->GetSelectionStart(Index),
		this->m_ViewStart[Index]);

	int SelectionEnd = this->GetSelectionEnd(Index);

	if (SelectionEnd <= SelectionStart)
	{
		return;
	}

	char TextBeforeSelection[INPUT_TEXT_SIZE];

	char SelectedText[INPUT_TEXT_SIZE];

	this->BuildVisibleText(
		Index,
		this->m_ViewStart[Index],
		SelectionStart,
		TextBeforeSelection,
		sizeof(TextBeforeSelection));

	this->BuildVisibleText(
		Index,
		SelectionStart,
		SelectionEnd,
		SelectedText,
		sizeof(SelectedText));

	int SelectionX = GetTextWidth(TextBeforeSelection);

	int SelectionWidth = GetTextWidth(SelectedText);

	int MaximumWidth = this->m_RenderWidth[Index];

	if (SelectionX < MaximumWidth)
	{
		if ((SelectionX + SelectionWidth) > MaximumWidth)
		{
			SelectionWidth = MaximumWidth - SelectionX;
		}

		if (SelectionWidth > 0)
		{
			SIZE TextSize = { 0 };

			GetTextExtentPointA(
				m_hFontDC,
				"Ag",
				2,
				&TextSize);

			int SelectionHeight = TextSize.cy;

			if (SelectionHeight <= 0)
			{
				SelectionHeight = 10;
			}

			EnableAlphaTest(true);

			glColor4f(0.20f, 0.45f, 0.80f, 0.55f);

			RenderColor(
				(float)(X + SelectionX),
				(float)Y,
				(float)SelectionWidth,
				(float)SelectionHeight);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			EnableAlphaTest(true);
		}
	}

	SecureZeroMemory(
		SelectedText,
		sizeof(SelectedText));

	SecureZeroMemory(
		TextBeforeSelection,
		sizeof(TextBeforeSelection));
}

void CInput::InsertCharacter(int Index, char Character)
{
	char Text[2];

	Text[0] = Character;

	Text[1] = '\0';

	this->InsertText(Index, Text);
}

void CInput::InsertText(int Index, const char* Text)
{
	if (this->IsValidInputIndex(Index) == false ||
		Text == NULL || Text[0] == '\0')
	{
		return;
	}

	this->DeleteSelection(Index);

	if (Index == 0 && this->m_TokenActive != false)
	{
		this->m_CaretPosition[Index] = this->SkipToken(
			this->m_CaretPosition[Index],
			1);
	}

	int Length = this->GetInputLength(Index);

	int Available = min(
		this->GetInputLimit(Index),
		INPUT_TEXT_SIZE - 1) - Length;

	if (Available <= 0)
	{
		return;
	}

	int Position = this->m_CaretPosition[Index];

	int InsertLength = (int)strnlen_s(
		Text,
		INPUT_TEXT_SIZE);

	if (InsertLength > Available)
	{
		InsertLength = Available;
	}

	memmove(
		InputText[Index] + Position + InsertLength,
		InputText[Index] + Position,
		Length - Position + 1);

	memcpy(
		InputText[Index] + Position,
		Text,
		InsertLength);

	this->UpdateTokenAfterEdit(
		Position,
		0,
		InsertLength);

	this->m_CaretPosition[Index] += InsertLength;

	this->ClearSelection(Index);

	this->UpdateInputLength(Index);

	this->UpdateViewStart(Index);

	this->ResetHistoryNavigation();
}

void CInput::DeleteCharacterBeforeCaret(int Index)
{
	if (this->DeleteSelection(Index))
	{
		return;
	}

	int Position = this->m_CaretPosition[Index];

	if (Position <= 0)
	{
		return;
	}

	if (Index == 0 && this->m_TokenActive != false &&
		Position > this->m_TokenStart &&
		Position <= (this->m_TokenStart + this->m_TokenLength))
	{
		this->DeleteRange(
			Index,
			this->m_TokenStart,
			this->m_TokenStart + this->m_TokenLength);

		return;
	}

	int Length = this->GetInputLength(Index);

	memmove(
		InputText[Index] + Position - 1,
		InputText[Index] + Position,
		Length - Position + 1);

	this->UpdateTokenAfterEdit(Position - 1, 1, 0);

	this->m_CaretPosition[Index]--;

	this->ClearSelection(Index);

	this->UpdateInputLength(Index);

	this->UpdateViewStart(Index);

	this->ResetHistoryNavigation();
}

void CInput::DeleteCharacterAtCaret(int Index)
{
	if (this->DeleteSelection(Index))
	{
		return;
	}

	int Position = this->m_CaretPosition[Index];

	int Length = this->GetInputLength(Index);

	if (Position >= Length)
	{
		return;
	}

	if (Index == 0 && this->m_TokenActive != false &&
		Position >= this->m_TokenStart &&
		Position < (this->m_TokenStart + this->m_TokenLength))
	{
		this->DeleteRange(
			Index,
			this->m_TokenStart,
			this->m_TokenStart + this->m_TokenLength);

		return;
	}

	memmove(
		InputText[Index] + Position,
		InputText[Index] + Position + 1,
		Length - Position);

	this->UpdateTokenAfterEdit(Position, 1, 0);

	this->UpdateInputLength(Index);

	this->UpdateViewStart(Index);

	this->ResetHistoryNavigation();
}

bool CInput::DeleteSelection(int Index)
{
	if (this->HasSelection(Index) == false)
	{
		return false;
	}

	int SelectionStart = this->GetSelectionStart(Index);

	int SelectionEnd = this->GetSelectionEnd(Index);

	if (Index == 0)
	{
		this->ExpandRangeForToken(
			&SelectionStart,
			&SelectionEnd);
	}

	int Length = this->GetInputLength(Index);

	memmove(
		InputText[Index] + SelectionStart,
		InputText[Index] + SelectionEnd,
		Length - SelectionEnd + 1);

	this->UpdateTokenAfterEdit(
		SelectionStart,
		SelectionEnd - SelectionStart,
		0);

	this->m_CaretPosition[Index] = SelectionStart;

	this->ClearSelection(Index);

	this->UpdateInputLength(Index);

	this->UpdateViewStart(Index);

	this->ResetHistoryNavigation();

	return true;
}

void CInput::ResetHistoryNavigation()
{
	this->m_HistoryPosition = this->m_HistoryCount;

	this->m_HistoryBrowsing = false;

	this->m_HistoryDraft[0] = '\0';
}

void CInput::PushHistory(const char* Text)
{
	if (Text == NULL || Text[0] == '\0')
	{
		this->ResetHistoryNavigation();

		return;
	}

	char Value[INPUT_TEXT_SIZE];

	strncpy_s(
		Value,
		sizeof(Value),
		Text,
		_TRUNCATE);

	int Length = (int)strnlen_s(Value, sizeof(Value));

	while (Length > 0 && Value[Length - 1] == ' ')
	{
		Value[--Length] = '\0';
	}

	if (Length == 0)
	{
		SecureZeroMemory(Value, sizeof(Value));

		this->ResetHistoryNavigation();

		return;
	}

	if (this->m_HistoryCount > 0 &&
		strcmp(this->m_History[this->m_HistoryCount - 1], Value) == 0)
	{
		SecureZeroMemory(Value, sizeof(Value));

		this->ResetHistoryNavigation();

		return;
	}

	if (this->m_HistoryCount == MAX_INPUT_HISTORY)
	{
		memmove(
			this->m_History[0],
			this->m_History[1],
			(MAX_INPUT_HISTORY - 1) * INPUT_TEXT_SIZE);

		this->m_HistoryCount--;
	}

	strncpy_s(
		this->m_History[this->m_HistoryCount],
		INPUT_TEXT_SIZE,
		Value,
		_TRUNCATE);

	this->m_HistoryCount++;

	this->ResetHistoryNavigation();

	SecureZeroMemory(Value, sizeof(Value));
}

void CInput::BrowseHistory(int Direction, int Index)
{
	if (this->m_HistoryCount == 0)
	{
		return;
	}

	if (this->m_HistoryBrowsing == false)
	{
		strncpy_s(
			this->m_HistoryDraft,
			sizeof(this->m_HistoryDraft),
			InputText[Index],
			_TRUNCATE);

		this->m_HistoryPosition = this->m_HistoryCount;

		this->m_HistoryBrowsing = true;
	}

	if (Direction < 0)
	{
		if (this->m_HistoryPosition > 0)
		{
			this->m_HistoryPosition--;
		}

		this->SetInputText(
			Index,
			this->m_History[this->m_HistoryPosition]);
	}
	else if (this->m_HistoryPosition < (this->m_HistoryCount - 1))
	{
		this->m_HistoryPosition++;

		this->SetInputText(
			Index,
			this->m_History[this->m_HistoryPosition]);
	}
	else
	{
		this->m_HistoryPosition = this->m_HistoryCount;

		this->SetInputText(
			Index,
			this->m_HistoryDraft);
	}
}

void CInput::SetInputText(int Index, const char* Text)
{
	if (this->IsValidInputIndex(Index) == false || Text == NULL)
	{
		return;
	}

	strncpy_s(
		InputText[Index],
		INPUT_TEXT_SIZE,
		Text,
		_TRUNCATE);

	if (Index == 0)
	{
		this->ClearAtomicToken();
	}

	this->UpdateInputLength(Index);

	this->m_CaretPosition[Index] = this->GetInputLength(Index);

	this->ClearSelection(Index);

	this->m_ViewStart[Index] = 0;

	this->UpdateViewStart(Index);
}

bool CInput::CopySelection(int Index)
{
	if (this->HasSelection(Index) == false)
	{
		return false;
	}

	int SelectionStart = this->GetSelectionStart(Index);

	int SelectionLength =
		this->GetSelectionEnd(Index) - SelectionStart;

	char SelectedText[INPUT_TEXT_SIZE];

	memcpy(
		SelectedText,
		InputText[Index] + SelectionStart,
		SelectionLength);

	SelectedText[SelectionLength] = '\0';

	bool Result = this->SetClipboardText(
		SelectedText,
		SelectionLength);

	SecureZeroMemory(
		SelectedText,
		sizeof(SelectedText));

	return Result;
}

bool CInput::SetClipboardText(
	const char* Text,
	int Length)
{
	if (Text == NULL || Length <= 0 ||
		Length >= INPUT_TEXT_SIZE ||
		GetForegroundWindow() != g_hWnd)
	{
		return false;
	}

	wchar_t WideText[INPUT_TEXT_SIZE];

	int WideLength = MultiByteToWideChar(
		INPUT_CODE_PAGE,
		0,
		Text,
		Length,
		WideText,
		INPUT_TEXT_SIZE - 1);

	if (WideLength <= 0)
	{
		SecureZeroMemory(WideText, sizeof(WideText));

		return false;
	}

	WideText[WideLength] = L'\0';

	SIZE_T MemorySize =
		(WideLength + 1) * sizeof(wchar_t);

	HGLOBAL ClipboardMemory = GlobalAlloc(
		GMEM_MOVEABLE,
		MemorySize);

	if (ClipboardMemory == NULL)
	{
		SecureZeroMemory(WideText, sizeof(WideText));

		return false;
	}

	void* ClipboardData = GlobalLock(ClipboardMemory);

	if (ClipboardData == NULL)
	{
		GlobalFree(ClipboardMemory);

		SecureZeroMemory(WideText, sizeof(WideText));

		return false;
	}

	memcpy(ClipboardData, WideText, MemorySize);

	GlobalUnlock(ClipboardMemory);

	SecureZeroMemory(WideText, sizeof(WideText));

	if (OpenClipboard(g_hWnd) == FALSE)
	{
		GlobalFree(ClipboardMemory);

		return false;
	}

	bool Result = false;

	if (EmptyClipboard() != FALSE &&
		SetClipboardData(
			CF_UNICODETEXT,
			ClipboardMemory) != NULL)
	{
		ClipboardMemory = NULL;

		Result = true;
	}

	CloseClipboard();

	if (ClipboardMemory != NULL)
	{
		GlobalFree(ClipboardMemory);
	}

	return Result;
}

bool CInput::GetClipboardText(
	char* Text,
	int TextSize)
{
	if (Text == NULL || TextSize <= 1)
	{
		return false;
	}

	Text[0] = '\0';

	if (GetForegroundWindow() != g_hWnd ||
		IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE ||
		OpenClipboard(g_hWnd) == FALSE)
	{
		return false;
	}

	HANDLE ClipboardHandle = GetClipboardData(CF_UNICODETEXT);

	if (ClipboardHandle == NULL)
	{
		CloseClipboard();

		return false;
	}

	SIZE_T ClipboardSize = GlobalSize(ClipboardHandle);

	const wchar_t* WideText =
		(const wchar_t*)GlobalLock(ClipboardHandle);

	if (ClipboardSize < sizeof(wchar_t) || WideText == NULL)
	{
		if (WideText != NULL)
		{
			GlobalUnlock(ClipboardHandle);
		}

		CloseClipboard();

		return false;
	}

	SIZE_T WideCapacity = ClipboardSize / sizeof(wchar_t);

	int OutputIndex = 0;

	for (SIZE_T n = 0;
		n < WideCapacity && OutputIndex < (TextSize - 1);
		n++)
	{
		wchar_t Character = WideText[n];

		if (Character == L'\0')
		{
			break;
		}

		if (Character == L'\r' ||
			Character == L'\n' ||
			Character == L'\t')
		{
			if (OutputIndex > 0 &&
				Text[OutputIndex - 1] != ' ')
			{
				Text[OutputIndex++] = ' ';
			}

			continue;
		}

		if (Character < 0x20 ||
			(Character >= 0x7F && Character <= 0x9F))
		{
			continue;
		}

		char ConvertedCharacter = '\0';

		BOOL UsedDefaultCharacter = FALSE;

		int ConvertedLength = WideCharToMultiByte(
			INPUT_CODE_PAGE,
			WC_NO_BEST_FIT_CHARS,
			&Character,
			1,
			&ConvertedCharacter,
			1,
			NULL,
			&UsedDefaultCharacter);

		if (ConvertedLength == 1 &&
			UsedDefaultCharacter == FALSE &&
			ConvertedCharacter != '\0')
		{
			Text[OutputIndex++] = ConvertedCharacter;
		}
	}

	Text[OutputIndex] = '\0';

	GlobalUnlock(ClipboardHandle);

	CloseClipboard();

	return (OutputIndex > 0);
}

bool CInput::CanCopySelection(
	eInputContext Context,
	int Index)
{
	return (Context != INPUT_CONTEXT_LOGIN || Index != 1);
}

bool CInput::HandleKeyDown(WPARAM wParam)
{
	eInputContext Context;

	int Index;

	if (this->GetActiveContext(&Context, &Index) == false)
	{
		return false;
	}

	this->SyncCaret(Context, Index);

	bool ControlPressed =
		(GetKeyState(VK_CONTROL) & 0x8000) != 0;

	bool AltPressed =
		(GetKeyState(VK_MENU) & 0x8000) != 0;

	bool ShiftPressed =
		(GetKeyState(VK_SHIFT) & 0x8000) != 0;

	if (ControlPressed != false && AltPressed == false)
	{
		switch (wParam)
		{
			case 'A':
			{
				this->m_SelectionAnchor[Index] = 0;

				this->SetCaretPosition(
					Index,
					this->GetInputLength(Index));

				return true;
			}

			case 'C':
			{
				if (this->CanCopySelection(Context, Index))
				{
					this->CopySelection(Index);
				}

				return true;
			}

			case 'X':
			{
				if (this->CanCopySelection(Context, Index) &&
					this->CopySelection(Index))
				{
					this->DeleteSelection(Index);
				}

				return true;
			}

			case 'V':
			{
				char ClipboardText[INPUT_TEXT_SIZE];

				if (this->GetClipboardText(
					ClipboardText,
					sizeof(ClipboardText)))
				{
					this->InsertText(Index, ClipboardText);
				}

				SecureZeroMemory(
					ClipboardText,
					sizeof(ClipboardText));

				return true;
			}
		}
	}

	if (Context == INPUT_CONTEXT_CHAT && Index == 0)
	{
		if (wParam == VK_UP)
		{
			this->BrowseHistory(-1, Index);

			return true;
		}

		if (wParam == VK_DOWN)
		{
			this->BrowseHistory(1, Index);

			return true;
		}

		if (wParam == VK_RETURN)
		{
			this->PushHistory(InputText[Index]);

			this->ClearSelection(Index);

			return false;
		}
	}

	if (wParam == VK_ESCAPE)
	{
		this->ClearSelection(Index);

		return false;
	}

	switch (wParam)
	{
		case VK_LEFT:
		{
			int Position = this->m_CaretPosition[Index] - 1;

			if (ShiftPressed == false &&
				this->HasSelection(Index))
			{
				Position = this->GetSelectionStart(Index);
			}

			this->MoveCaret(
				Index,
				this->SkipToken(Position, -1),
				ShiftPressed);

			return true;
		}

		case VK_RIGHT:
		{
			int Position = this->m_CaretPosition[Index] + 1;

			if (ShiftPressed == false &&
				this->HasSelection(Index))
			{
				Position = this->GetSelectionEnd(Index);
			}

			this->MoveCaret(
				Index,
				this->SkipToken(Position, 1),
				ShiftPressed);

			return true;
		}

		case VK_HOME:
		{
			this->MoveCaret(Index, 0, ShiftPressed);

			return true;
		}

		case VK_END:
		{
			this->MoveCaret(
				Index,
				this->GetInputLength(Index),
				ShiftPressed);

			return true;
		}

		case VK_DELETE:
		{
			this->DeleteCharacterAtCaret(Index);

			return true;
		}
	}

	return false;
}

bool CInput::HandleChar(WPARAM wParam)
{
	eInputContext Context;

	int Index;

	if (this->GetActiveContext(&Context, &Index) == false)
	{
		return false;
	}

	this->SyncCaret(Context, Index);

	if (wParam == VK_BACK)
	{
		this->DeleteCharacterBeforeCaret(Index);

		return true;
	}

	if (wParam == VK_RETURN || wParam == VK_TAB)
	{
		this->ClearSelection(Index);

		return false;
	}

	if (wParam < 0x20)
	{
		return true;
	}

	if (wParam == 0x7F || wParam > 0xFF)
	{
		return false;
	}

	this->InsertCharacter(
		Index,
		(char)wParam);

	return true;
}

bool CInput::IsChatInputActive()
{
	eInputContext Context;

	int Index;

	return (this->GetActiveContext(&Context, &Index) != false &&
		Context == INPUT_CONTEXT_CHAT &&
		Index == 0);
}

void CInput::ClearChatInput()
{
	this->SetInputText(0, "");
	this->ResetHistoryNavigation();
}

bool CInput::InsertAtomicToken(const char* Text, DWORD Value)
{
	if (Text == NULL || Text[0] == 0 ||
		this->IsChatInputActive() == false)
	{
		return false;
	}

	int TokenLength = (int)strnlen_s(Text, INPUT_TEXT_SIZE);

	if (TokenLength <= 0 || TokenLength >= INPUT_TEXT_SIZE)
	{
		return false;
	}

	int Index = 0;

	this->SyncCaret(INPUT_CONTEXT_CHAT, Index);

	if (this->m_TokenActive != false)
	{
		int ExistingStart = 0;

		int ExistingLength = 0;

		DWORD ExistingValue = 0;

		this->GetAtomicToken(
			&ExistingStart,
			&ExistingLength,
			&ExistingValue);
	}

	int CurrentLength = this->GetInputLength(Index);

	int RemovedLength = 0;

	if (this->m_TokenActive != false)
	{
		RemovedLength = this->m_TokenLength;
	}
	else if (this->HasSelection(Index) != false)
	{
		RemovedLength = this->GetSelectionEnd(Index) -
			this->GetSelectionStart(Index);
	}

	int Limit = min(
		this->GetInputLimit(Index),
		INPUT_TEXT_SIZE - 1);

	if ((CurrentLength - RemovedLength + TokenLength) > Limit)
	{
		return false;
	}

	int Position = this->m_CaretPosition[Index];

	if (this->m_TokenActive != false)
	{
		Position = this->m_TokenStart;

		this->DeleteRange(
			Index,
			this->m_TokenStart,
			this->m_TokenStart + this->m_TokenLength);
	}
	else if (this->HasSelection(Index) != false)
	{
		Position = this->GetSelectionStart(Index);

		this->DeleteSelection(Index);
	}

	int Length = this->GetInputLength(Index);

	memmove(
		InputText[Index] + Position + TokenLength,
		InputText[Index] + Position,
		Length - Position + 1);

	memcpy(
		InputText[Index] + Position,
		Text,
		TokenLength);

	this->m_TokenActive = true;

	this->m_TokenStart = Position;

	this->m_TokenLength = TokenLength;

	this->m_TokenValue = Value;

	memset(this->m_TokenText, 0, sizeof(this->m_TokenText));

	memcpy(this->m_TokenText, Text, TokenLength);

	this->m_CaretPosition[Index] = Position + TokenLength;

	this->ClearSelection(Index);

	this->UpdateInputLength(Index);

	this->UpdateViewStart(Index);

	this->ResetHistoryNavigation();

	return true;
}

bool CInput::GetAtomicToken(
	int* Start,
	int* Length,
	DWORD* Value)
{
	if (this->m_TokenActive == false ||
		Start == NULL || Length == NULL || Value == NULL)
	{
		return false;
	}

	int InputLengthValue = this->GetInputLength(0);

	if (this->m_TokenStart < 0 || this->m_TokenLength <= 0 ||
		(this->m_TokenStart + this->m_TokenLength) > InputLengthValue ||
		memcmp(
			InputText[0] + this->m_TokenStart,
			this->m_TokenText,
			this->m_TokenLength) != 0)
	{
		this->ClearAtomicToken();

		return false;
	}

	*Start = this->m_TokenStart;

	*Length = this->m_TokenLength;

	*Value = this->m_TokenValue;

	return true;
}

void CInput::ClearAtomicToken()
{
	this->m_TokenActive = false;

	this->m_TokenStart = 0;

	this->m_TokenLength = 0;

	this->m_TokenValue = 0;

	SecureZeroMemory(
		this->m_TokenText,
		sizeof(this->m_TokenText));
}

bool CInput::TokenOverlaps(int Start, int End)
{
	if (this->m_TokenActive == false || End <= Start)
	{
		return false;
	}

	int TokenEnd = this->m_TokenStart + this->m_TokenLength;

	return (Start < TokenEnd && End > this->m_TokenStart);
}

void CInput::ExpandRangeForToken(int* Start, int* End)
{
	if (Start == NULL || End == NULL ||
		this->TokenOverlaps(*Start, *End) == false)
	{
		return;
	}

	*Start = min(*Start, this->m_TokenStart);

	*End = max(
		*End,
		this->m_TokenStart + this->m_TokenLength);
}

void CInput::UpdateTokenAfterEdit(
	int Start,
	int RemovedLength,
	int InsertedLength)
{
	if (this->m_TokenActive == false)
	{
		return;
	}

	int End = Start + RemovedLength;

	if (this->TokenOverlaps(Start, End) != false ||
		(RemovedLength == 0 && Start > this->m_TokenStart &&
		 Start < (this->m_TokenStart + this->m_TokenLength)))
	{
		this->ClearAtomicToken();

		return;
	}

	if (Start <= this->m_TokenStart)
	{
		this->m_TokenStart += InsertedLength - RemovedLength;
	}
}

void CInput::DeleteRange(int Index, int Start, int End)
{
	if (this->IsValidInputIndex(Index) == false)
	{
		return;
	}

	int Length = this->GetInputLength(Index);

	Start = max(0, min(Start, Length));

	End = max(Start, min(End, Length));

	if (End <= Start)
	{
		return;
	}

	this->ExpandRangeForToken(&Start, &End);

	memmove(
		InputText[Index] + Start,
		InputText[Index] + End,
		Length - End + 1);

	this->UpdateTokenAfterEdit(Start, End - Start, 0);

	this->m_CaretPosition[Index] = Start;

	this->ClearSelection(Index);

	this->UpdateInputLength(Index);

	this->UpdateViewStart(Index);

	this->ResetHistoryNavigation();
}

int CInput::SkipToken(int Position, int Direction)
{
	if (this->m_TokenActive == false)
	{
		return Position;
	}

	int TokenEnd = this->m_TokenStart + this->m_TokenLength;

	if (Direction < 0 &&
		Position >= this->m_TokenStart && Position < TokenEnd)
	{
		return this->m_TokenStart;
	}

	if (Direction > 0 &&
		Position > this->m_TokenStart && Position <= TokenEnd)
	{
		return TokenEnd;
	}

	return Position;
}
