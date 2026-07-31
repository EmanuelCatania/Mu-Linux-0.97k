#include "stdafx.h"
#include "Input.h"

static const DWORD CHAT_INPUT_TEXT_CALL = 0x004BE5DF;

static const DWORD CHAT_INPUT_WHISPER_CALL = 0x004BE60F;

static const DWORD LOGIN_INPUT_ACCOUNT_CALL = 0x00521778;

static const DWORD LOGIN_INPUT_PASSWORD_CALL = 0x005217A0;

static const BYTE CHAT_INPUT_TEXT_BYTES[5] =
{
	0xE8, 0xCC, 0x0A, 0xFC, 0xFF
};

static const BYTE CHAT_INPUT_WHISPER_BYTES[5] =
{
	0xE8, 0x9C, 0x0A, 0xFC, 0xFF
};

static const BYTE LOGIN_INPUT_ACCOUNT_BYTES[5] =
{
	0xE8, 0x33, 0xD9, 0xF5, 0xFF
};

static const BYTE LOGIN_INPUT_PASSWORD_BYTES[5] =
{
	0xE8, 0x0B, 0xD9, 0xF5, 0xFF
};

CInput gInput;

CInput::CInput()
{
	memset(this->m_CaretPosition, 0, sizeof(this->m_CaretPosition));

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
}

CInput::~CInput()
{

}

bool CInput::Init()
{
	if (this->IsSupportedClient() == false)
	{
		return false;
	}

	SetCompleteHook(
		0xE8,
		CHAT_INPUT_TEXT_CALL,
		&CInput::RenderChatInputTextHook);

	SetCompleteHook(
		0xE8,
		CHAT_INPUT_WHISPER_CALL,
		&CInput::RenderChatInputTextHook);

	SetCompleteHook(
		0xE8,
		LOGIN_INPUT_ACCOUNT_CALL,
		&CInput::RenderLoginInputTextHook);

	SetCompleteHook(
		0xE8,
		LOGIN_INPUT_PASSWORD_CALL,
		&CInput::RenderLoginInputTextHook);

	return true;
}

bool CInput::IsSupportedClient()
{
	if (CheckBytes(
		CHAT_INPUT_TEXT_CALL,
		CHAT_INPUT_TEXT_BYTES,
		sizeof(CHAT_INPUT_TEXT_BYTES)) == false)
	{
		return false;
	}

	if (CheckBytes(
		CHAT_INPUT_WHISPER_CALL,
		CHAT_INPUT_WHISPER_BYTES,
		sizeof(CHAT_INPUT_WHISPER_BYTES)) == false)
	{
		return false;
	}

	if (CheckBytes(
		LOGIN_INPUT_ACCOUNT_CALL,
		LOGIN_INPUT_ACCOUNT_BYTES,
		sizeof(LOGIN_INPUT_ACCOUNT_BYTES)) == false)
	{
		return false;
	}

	if (CheckBytes(
		LOGIN_INPUT_PASSWORD_CALL,
		LOGIN_INPUT_PASSWORD_BYTES,
		sizeof(LOGIN_INPUT_PASSWORD_BYTES)) == false)
	{
		return false;
	}

	return true;
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

	if (this->IsValidInputIndex(InputIndex) == false)
	{
		return false;
	}

	if (InputIndex > 1)
	{
		return false;
	}

	if (this->m_ActiveContext == INPUT_CONTEXT_CHAT)
	{
		if (SceneFlag != MAIN_SCENE ||
			g_bGameServerConnected == FALSE ||
			(InputEnable == false && TabInputEnable == false))
		{
			return false;
		}
	}
	else if (this->m_ActiveContext == INPUT_CONTEXT_LOGIN)
	{
		if (SceneFlag != LOG_IN_SCENE || InputEnable == false)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	*Context = this->m_ActiveContext;

	*Index = InputIndex;

	return true;
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

	this->UpdateViewStart(Index);
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

	char OriginalText[INPUT_TEXT_SIZE];

	char VisibleText[INPUT_TEXT_SIZE];

	memcpy(
		OriginalText,
		InputText[Index],
		sizeof(OriginalText));

	this->BuildVisibleText(
		Index,
		this->m_ViewStart[Index],
		this->GetInputLength(Index),
		VisibleText,
		sizeof(VisibleText));

	BYTE OriginalHide = InputTextHide[Index];

	int OriginalInputIndex = InputIndex;

	strncpy_s(
		InputText[Index],
		INPUT_TEXT_SIZE,
		VisibleText,
		_TRUNCATE);

	InputTextHide[Index] = 0;

	InputIndex = -1;

	RenderInputText(X, Y, Index);

	InputIndex = OriginalInputIndex;

	InputTextHide[Index] = OriginalHide;

	memcpy(
		InputText[Index],
		OriginalText,
		sizeof(OriginalText));

	this->RenderCaret(X, Y, Index);

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

void CInput::InsertCharacter(int Index, char Character)
{
	int Length = this->GetInputLength(Index);

	if (Length >= this->GetInputLimit(Index) ||
		Length >= (INPUT_TEXT_SIZE - 1))
	{
		return;
	}

	int Position = this->m_CaretPosition[Index];

	memmove(
		InputText[Index] + Position + 1,
		InputText[Index] + Position,
		Length - Position + 1);

	InputText[Index][Position] = Character;

	this->m_CaretPosition[Index]++;

	this->UpdateInputLength(Index);

	this->UpdateViewStart(Index);

	this->ResetHistoryNavigation();
}

void CInput::DeleteCharacterBeforeCaret(int Index)
{
	int Position = this->m_CaretPosition[Index];

	if (Position <= 0)
	{
		return;
	}

	int Length = this->GetInputLength(Index);

	memmove(
		InputText[Index] + Position - 1,
		InputText[Index] + Position,
		Length - Position + 1);

	this->m_CaretPosition[Index]--;

	this->UpdateInputLength(Index);

	this->UpdateViewStart(Index);

	this->ResetHistoryNavigation();
}

void CInput::DeleteCharacterAtCaret(int Index)
{
	int Position = this->m_CaretPosition[Index];

	int Length = this->GetInputLength(Index);

	if (Position >= Length)
	{
		return;
	}

	memmove(
		InputText[Index] + Position,
		InputText[Index] + Position + 1,
		Length - Position);

	this->UpdateInputLength(Index);

	this->UpdateViewStart(Index);

	this->ResetHistoryNavigation();
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

	this->UpdateInputLength(Index);

	this->m_CaretPosition[Index] = this->GetInputLength(Index);

	this->m_ViewStart[Index] = 0;

	this->UpdateViewStart(Index);
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

			return false;
		}
	}

	switch (wParam)
	{
		case VK_LEFT:
		{
			this->SetCaretPosition(
				Index,
				this->m_CaretPosition[Index] - 1);

			return true;
		}

		case VK_RIGHT:
		{
			this->SetCaretPosition(
				Index,
				this->m_CaretPosition[Index] + 1);

			return true;
		}

		case VK_HOME:
		{
			this->SetCaretPosition(Index, 0);

			return true;
		}

		case VK_END:
		{
			this->SetCaretPosition(
				Index,
				this->GetInputLength(Index));

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
		return false;
	}

	if (wParam < 0x20)
	{
		return true;
	}

	if (wParam > 0xFF)
	{
		return false;
	}

	this->InsertCharacter(
		Index,
		(char)wParam);

	return true;
}
