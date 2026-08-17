#pragma once
// Input.h — Mouse/keyboard WndProc, key state, IME, text input, NumPad hit-test.
typedef unsigned int  uint;
typedef unsigned char byte;
uint Key_IsJustPressed(int vk);
bool IME_GetAndClearState(char clear_mode, byte save_mask);
int  NumPad_HitTest(void);
