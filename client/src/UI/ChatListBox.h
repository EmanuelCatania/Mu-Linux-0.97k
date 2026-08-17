// ChatListBox.h — engine chat-widget object behind dword_55C9FF0.
// See ChatListBox.cpp for the rationale and full vtable layout.
#pragma once

extern "C" {
// Allocate, default-construct, return the chat-widget object.  Caller stores
// the pointer into DAT_055c9ff0.  Returns NULL on OOM.
void* ChatListBox_Construct(void);

// Widget hover flag — set by ChatListBox per-frame input handling, consumed
// by cursor render.  (Storage in ChatListBox.cpp.)
extern int MouseOnWindow;
}
