#include "stdafx.h"
#include "ItemLinkTooltip.h"
#include "ItemManager.h"

CItemLinkTooltip gItemLinkTooltip;

CItemLinkTooltip::CItemLinkTooltip()
{
	memset(this, 0, sizeof(*this));
}

void CItemLinkTooltip::Init()
{
	if (this->m_HooksInstalled != false)
	{
		return;
	}

	SetCompleteHook(
		0xE8,
		ItemTooltipTextListCall,
		&CItemLinkTooltip::RenderTextListHook);

	SetCompleteHook(
		0xE8,
		ItemTooltipLineTextCall,
		&CItemLinkTooltip::RenderLineHook);

	SetCompleteHook(
		0xE8,
		ItemTooltipBorderTopCall,
		&CItemLinkTooltip::RenderTopHook);

	SetCompleteHook(
		0xE8,
		ItemTooltipBorderLeftCall,
		&CItemLinkTooltip::RenderLeftHook);

	SetCompleteHook(
		0xE8,
		ItemTooltipBorderRightCall,
		&CItemLinkTooltip::RenderRightHook);

	SetCompleteHook(
		0xE8,
		ItemTooltipBorderBottomCall,
		&CItemLinkTooltip::RenderBottomHook);

	SetCompleteHook(
		0xE8,
		ItemTooltipFillCall,
		&CItemLinkTooltip::RenderFillHook);

	this->m_HooksInstalled = true;
}

void CItemLinkTooltip::Pin(const ITEM* Item, int X, int Y)
{
	if (Item == NULL)
	{
		return;
	}

	this->m_PinnedItem = *Item;
	this->m_PinnedX = X;
	this->m_PinnedY = Y;
	this->m_Pinned = true;
}

void CItemLinkTooltip::ClearPinned()
{
	this->m_Pinned = false;
}
int CItemLinkTooltip::GetTooltipLineHeight()
{
	SIZE TextSize = { 0, 0 };
	const char* Text = "Ag";

	if (m_hFontDC != NULL)
	{
		GetTextExtentPointA(m_hFontDC, Text, 2, &TextSize);
	}

	return max(1, TextSize.cy);
}

bool CItemLinkTooltip::IsTooltipSeparator(const char* Text)
{
	return Text != NULL &&
		(Text[0] == '\n' || (Text[0] == ' ' && Text[1] == '\0'));
}

int CItemLinkTooltip::CalculateTooltipTextHeight(int TextCount)
{
	if (m_hFontDC == NULL || TextCount <= 0 || TextCount > 64)
	{
		return 0;
	}

	int Height = 0;
	SIZE TextSize = { 0, 0 };
	HGDIOBJ PreviousFont = GetCurrentObject(m_hFontDC, OBJ_FONT);

	for (int Index = 0; Index < TextCount; Index++)
	{
		const char* Text = TextList[Index];

		if (Text[0] == 0)
		{
			break;
		}

		SelectObject(
			m_hFontDC,
			(TextBold[Index] != 0) ? g_hFontBold : g_hFont);
		TextSize.cx = 0;
		TextSize.cy = 0;
		GetTextExtentPointA(
			m_hFontDC,
			Text,
			lstrlenA(Text),
			&TextSize);

		Height += (Text[0] == '\n') ?
			(TextSize.cy / 2) : TextSize.cy;
	}

	SelectObject(m_hFontDC, PreviousFont);

	if (g_fScreenRate_y <= 0.0f)
	{
		return Height;
	}

	return (int)ceilf(
		(float)Height / (g_fScreenRate_y * 0.9090909f));
}

void CItemLinkTooltip::PrepareTooltipRectangle(float Y, float Height)
{
	if (gItemLinkTooltip.m_TooltipLayout.active == false ||
		gItemLinkTooltip.m_TooltipLayout.rectangleValid != false)
	{
		return;
	}

	const int NativeY = (int)floorf(Y + 1.0f);
	gItemLinkTooltip.m_TooltipLayout.height =
		max(0, (int)ceilf(Height - 1.0f));

	const int ExpandedHeight =
		gItemLinkTooltip.m_TooltipLayout.height +
		gItemLinkTooltip.m_TooltipLayout.extraHeight;

	gItemLinkTooltip.m_TooltipLayout.offsetY = 0;

	if (NativeY + ExpandedHeight > WindowHeight)
	{
		gItemLinkTooltip.m_TooltipLayout.offsetY =
			WindowHeight - (NativeY + ExpandedHeight);
	}

	if (NativeY + gItemLinkTooltip.m_TooltipLayout.offsetY < 0)
	{
		gItemLinkTooltip.m_TooltipLayout.offsetY = -NativeY;
	}

	gItemLinkTooltip.m_TooltipLayout.y =
		NativeY + gItemLinkTooltip.m_TooltipLayout.offsetY;
	gItemLinkTooltip.m_TooltipLayout.height = ExpandedHeight;
	gItemLinkTooltip.m_TooltipLayout.rectangleValid = true;
}

void CItemLinkTooltip::RenderTooltipBorder(
	float X,
	float Y,
	float Width,
	float Height,
	int Part)
{
	if (gItemLinkTooltip.m_TooltipLayout.active == false)
	{
		RenderColor(X, Y, Width, Height);

		return;
	}

	if (Part == 1)
	{
		gItemLinkTooltip.PrepareTooltipRectangle(Y, Height);
	}
	else if (Part == 0 &&
		gItemLinkTooltip.m_TooltipLayout.rectangleValid == false)
	{
		gItemLinkTooltip.m_TooltipLayout.x = (int)floorf(X + 1.0f);
		if (gItemLinkTooltip.m_TooltipLayout.nativeTextHeight <= 0)
		{
			gItemLinkTooltip.m_TooltipLayout.y = (int)floorf(Y + 1.0f);
		}
		gItemLinkTooltip.m_TooltipLayout.width =
			max(0, (int)ceilf(Width - 1.0f));
	}

	float RenderY = Y + (float)gItemLinkTooltip.m_TooltipLayout.offsetY;
	float RenderHeight = Height;

	switch (Part)
	{
		case 1:
		case 2:
			RenderHeight += (float)gItemLinkTooltip.m_TooltipLayout.extraHeight;
			break;

		case 3:
			RenderY += (float)gItemLinkTooltip.m_TooltipLayout.extraHeight;
			break;

		case 4:
			RenderHeight += (float)gItemLinkTooltip.m_TooltipLayout.extraHeight;
			break;
	}

	RenderColor(X, RenderY, Width, RenderHeight);
}

void __cdecl CItemLinkTooltip::RenderTextListHook(
	void* TextListPointer,
	int Y,
	int TextCount,
	int Width,
	int Arg5,
	int Arg6)
{
	if (gItemLinkTooltip.m_TooltipLayout.active != false)
	{
		gItemLinkTooltip.m_TooltipLayout.nativeTextHeight =
			gItemLinkTooltip.CalculateTooltipTextHeight(TextCount);

		/* The border hook has the authoritative tooltip rectangle.  When
		 * RenderItemTextList runs first, keep a provisional offset only until
		 * the border arrives; otherwise do not undo the border's correction. */
		if (gItemLinkTooltip.m_TooltipLayout.rectangleValid == false)
		{
			gItemLinkTooltip.m_TooltipLayout.offsetY = 0;

			const int ExpandedHeight = max(1,
				gItemLinkTooltip.m_TooltipLayout.nativeTextHeight) +
				gItemLinkTooltip.m_TooltipLayout.extraHeight;

			if (Y + ExpandedHeight > WindowHeight)
			{
				gItemLinkTooltip.m_TooltipLayout.offsetY =
					WindowHeight - (Y + ExpandedHeight);
			}

			if (Y + gItemLinkTooltip.m_TooltipLayout.offsetY < 0)
			{
				gItemLinkTooltip.m_TooltipLayout.offsetY = -Y;
			}
		}
	}

	RenderItemTextList(
		TextListPointer,
		Y,
		TextCount,
		Width,
		Arg5,
		Arg6);
}

void __fastcall CItemLinkTooltip::RenderLineHook(
	void* This,
	void*,
	int X,
	int Y,
	const char* Text,
	int Arg4,
	int Arg5,
	int Arg6,
	int Arg7,
	int Arg8)
{
	bool IsTitle = false;
	int RenderY = Y;

	if (gItemLinkTooltip.m_TooltipLayout.active != false)
	{
		RenderY += gItemLinkTooltip.m_TooltipLayout.offsetY;

		if (Text != NULL && Text[0] != 0 &&
			gItemLinkTooltip.IsTooltipSeparator(Text) == false)
		{
			if (gItemLinkTooltip.m_TooltipLayout.titleCaptured == false)
			{
				gItemLinkTooltip.m_TooltipLayout.titleY = RenderY;
				gItemLinkTooltip.m_TooltipLayout.titleCaptured = true;
				IsTitle = true;
			}
			else if (gItemLinkTooltip.m_TooltipLayout.bodyCaptured == false)
			{
				gItemLinkTooltip.m_TooltipLayout.bodyY = RenderY;
				gItemLinkTooltip.m_TooltipLayout.bodyDirection =
					(RenderY >= gItemLinkTooltip.m_TooltipLayout.titleY) ? 1 : -1;
				gItemLinkTooltip.m_TooltipLayout.bodyCaptured = true;
			}

			if (IsTitle == false)
			{
				RenderY +=
					(gItemLinkTooltip.m_TooltipLayout.bodyDirection >= 0) ?
					gItemLinkTooltip.m_TooltipLayout.extraHeight :
					-gItemLinkTooltip.m_TooltipLayout.extraHeight;
			}
		}
	}

	RenderItemTextLine(
		This,
		X,
		RenderY,
		Text,
		Arg4,
		Arg5,
		Arg6,
		Arg7,
		Arg8);
}

void __cdecl CItemLinkTooltip::RenderTopHook(float X, float Y, float Width, float Height)
{
	gItemLinkTooltip.RenderTooltipBorder(X, Y, Width, Height, 0);
}

void __cdecl CItemLinkTooltip::RenderLeftHook(float X, float Y, float Width, float Height)
{
	gItemLinkTooltip.RenderTooltipBorder(X, Y, Width, Height, 1);
}

void __cdecl CItemLinkTooltip::RenderRightHook(float X, float Y, float Width, float Height)
{
	gItemLinkTooltip.RenderTooltipBorder(X, Y, Width, Height, 2);
}

void __cdecl CItemLinkTooltip::RenderBottomHook(float X, float Y, float Width, float Height)
{
	gItemLinkTooltip.RenderTooltipBorder(X, Y, Width, Height, 3);
}

void __cdecl CItemLinkTooltip::RenderFillHook(float X, float Y, float Width, float Height)
{
	gItemLinkTooltip.RenderTooltipBorder(X, Y, Width, Height, 4);
}

int CItemLinkTooltip::GetTooltipModelSize(const ITEM* Item)
{
	if (Item == NULL || Item->Type < 0 || Item->Type >= MAX_ITEM)
	{
		return ITEM_TOOLTIP_MODEL_SIZE;
	}

	/* Jewels and consumables have a naturally small native preview.  Keeping
	 * their reserved area compact prevents a large empty gap in the tooltip. */
	if (Item->Type >= GET_ITEM(14, 0) &&
		Item->Type < GET_ITEM(15, 0))
	{
		return 32;
	}

	return ITEM_TOOLTIP_MODEL_SIZE;
}

void CItemLinkTooltip::Render(const ITEM* HoveredItem)
{
	if (SceneFlag != MAIN_SCENE)
	{
		this->m_Pinned = false;
		return;
	}

	const ITEM* Item = HoveredItem;
	int X = MouseX;
	int Y = MouseY;

	if (Item == NULL && this->m_Pinned != false)
	{
		Item = &this->m_PinnedItem;
		X = this->m_PinnedX;
		Y = this->m_PinnedY;
	}

	if (Item == NULL)
	{
		return;
	}

	memset(&this->m_TooltipLayout, 0, sizeof(this->m_TooltipLayout));
	this->m_TooltipLayout.active = this->m_HooksInstalled;
	this->m_TooltipLayout.modelSize =
		this->GetTooltipModelSize(Item);
	this->m_TooltipLayout.extraHeight = this->m_HooksInstalled ?
		this->m_TooltipLayout.modelSize + ITEM_TOOLTIP_MODEL_GAP : 0;

	RenderItemInfo(X, Y, (ITEM*)Item, false);

	this->m_TooltipLayout.active = false;
	this->RenderTooltipModel(Item);
}

void CItemLinkTooltip::RenderTooltipModel(const ITEM* Item)
{
	if (this->m_HooksInstalled == false ||
		Item == NULL || Item->Type < 0 || Item->Type >= MAX_ITEM ||
		gItemLinkTooltip.m_TooltipLayout.rectangleValid == false ||
		gItemLinkTooltip.m_TooltipLayout.titleCaptured == false)
	{
		return;
	}

	const int LineStep =
		(gItemLinkTooltip.m_TooltipLayout.bodyCaptured != false) ?
		abs(gItemLinkTooltip.m_TooltipLayout.bodyY -
			gItemLinkTooltip.m_TooltipLayout.titleY) :
		this->GetTooltipLineHeight();

	const int ModelSize = (gItemLinkTooltip.m_TooltipLayout.modelSize > 0) ?
		gItemLinkTooltip.m_TooltipLayout.modelSize : ITEM_TOOLTIP_MODEL_SIZE;

	int X = gItemLinkTooltip.m_TooltipLayout.x +
		(gItemLinkTooltip.m_TooltipLayout.width - ModelSize) / 2;
	int Y = gItemLinkTooltip.m_TooltipLayout.titleY;

	if (gItemLinkTooltip.m_TooltipLayout.bodyDirection >= 0)
	{
		Y += LineStep;
	}
	else
	{
		Y -= ModelSize + LineStep;
	}

	const int MinimumX = gItemLinkTooltip.m_TooltipLayout.x;
	const int MaximumX =
		gItemLinkTooltip.m_TooltipLayout.x +
		gItemLinkTooltip.m_TooltipLayout.width - ModelSize;
	const int MinimumY = gItemLinkTooltip.m_TooltipLayout.y;
	const int MaximumY =
		gItemLinkTooltip.m_TooltipLayout.y +
		gItemLinkTooltip.m_TooltipLayout.height - ModelSize;

	X = max(MinimumX, min(X, MaximumX));
	Y = max(MinimumY, min(Y, MaximumY));

	gItemManager.RenderItemPreview3D(
		(float)X,
		(float)Y,
		(float)ModelSize,
		(float)ModelSize,
		(ITEM*)Item);
}
