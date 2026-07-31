#include "stdafx.h"
#include "ChatInput.h"

static const DWORD CHAT_INPUT_TEXT_CALL = 0x004BE5DF;

static const DWORD CHAT_INPUT_WHISPER_CALL = 0x004BE60F;

static const BYTE CHAT_INPUT_TEXT_BYTES[5] =
{
	0xE8, 0xCC, 0x0A, 0xFC, 0xFF
};

static const BYTE CHAT_INPUT_WHISPER_BYTES[5] =
{
	0xE8, 0x9C, 0x0A, 0xFC, 0xFF
};

static bool CheckBytes(DWORD Address, const BYTE* Expected, SIZE_T Size)
{
	MEMORY_BASIC_INFORMATION MemoryInfo;

	if (VirtualQuery((LPCVOID)Address, &MemoryInfo, sizeof(MemoryInfo)) == 0)
	{
		return false;
	}

	if (MemoryInfo.State != MEM_COMMIT)
	{
		return false;
	}

	if ((MemoryInfo.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0)
	{
		return false;
	}

	return (memcmp((const void*)Address, Expected, Size) == 0);
}

CChatInput gChatInput;

CChatInput::CChatInput()
{

}

CChatInput::~CChatInput()
{

}

bool CChatInput::Init()
{
	if (!this->IsSupportedClient())
	{
		return false;
	}

	SetCompleteHook(
		0xE8,
		CHAT_INPUT_TEXT_CALL,
		&this->RenderInputTextHook);

	SetCompleteHook(
		0xE8,
		CHAT_INPUT_WHISPER_CALL,
		&this->RenderInputTextHook);

	return true;
}

bool CChatInput::IsSupportedClient()
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

	return true;
}

void CChatInput::RenderInputTextHook(
	int X,
	int Y,
	int Index)
{
	RenderInputText(X, Y, Index);
}