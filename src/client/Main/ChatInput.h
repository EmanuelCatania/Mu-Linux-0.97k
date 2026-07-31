#pragma once

class CChatInput
{
public:

	CChatInput();

	~CChatInput();

	bool Init();

	static void RenderInputTextHook(
		int X,
		int Y,
		int Index);

private:

	bool IsSupportedClient();
};

extern CChatInput gChatInput;