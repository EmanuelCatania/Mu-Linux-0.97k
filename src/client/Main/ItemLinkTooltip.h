#pragma once

#include <windows.h>
#include "Util.h"

struct ITEM;

class CItemLinkTooltip
{
public:

	CItemLinkTooltip();

	void Init();

	void Pin(
		const ITEM* Item,
		int X,
		int Y);

	void ClearPinned();

	void Render(const ITEM* HoveredItem);

private:

	enum
	{
		ITEM_TOOLTIP_MODEL_SIZE = 64,
		ITEM_TOOLTIP_MODEL_GAP = 8
	};

	struct TOOLTIP_LAYOUT
	{
		bool active;
		bool rectangleValid;
		bool titleCaptured;
		bool bodyCaptured;
		int extraHeight;
		int modelSize;
		int offsetY;
		int x;
		int y;
		int width;
		int height;
		int nativeTextHeight;
		int titleY;
		int bodyY;
		int bodyDirection;
	};

	static void __cdecl RenderTextListHook(
		void* TextListPointer,
		int Y,
		int TextCount,
		int Width,
		int Arg5,
		int Arg6);

	static void __fastcall RenderLineHook(
		void* This,
		void* Unused,
		int X,
		int Y,
		const char* Text,
		int Arg4,
		int Arg5,
		int Arg6,
		int Arg7,
		int Arg8);

	static void __cdecl RenderTopHook(float X, float Y, float Width, float Height);

	static void __cdecl RenderLeftHook(float X, float Y, float Width, float Height);

	static void __cdecl RenderRightHook(float X, float Y, float Width, float Height);

	static void __cdecl RenderBottomHook(float X, float Y, float Width, float Height);

	static void __cdecl RenderFillHook(float X, float Y, float Width, float Height);

	static int GetTooltipLineHeight();

	static int GetTooltipModelSize(const ITEM* Item);

	static bool IsTooltipSeparator(const char* Text);

	static int CalculateTooltipTextHeight(int TextCount);

	void PrepareTooltipRectangle(float Y, float Height);

	void RenderTooltipBorder(
		float X,
		float Y,
		float Width,
		float Height,
		int Part);

	void RenderTooltipModel(const ITEM* Item);

	TOOLTIP_LAYOUT m_TooltipLayout;
	bool m_HooksInstalled;
	bool m_Pinned;
	ITEM m_PinnedItem;
	int m_PinnedX;
	int m_PinnedY;
};

extern CItemLinkTooltip gItemLinkTooltip;
