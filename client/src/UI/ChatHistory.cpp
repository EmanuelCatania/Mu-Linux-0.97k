#include "stdafx.h"
#include "UI/ChatHistory.h"
#include <cstring>
#include <string>

namespace {
constexpr int kMaxEntries = 5;
std::string s_entries[kMaxEntries];   // [0] = la más reciente
int s_count = 0;
int s_browse = -1;                    // -1 = editando el texto propio
std::string s_draft;

// Campo 0 del input del chat (InputText[0] / InputLength[0]).
char* MessageField() { return DAT_07db8710[0]; }
void SetMessageField(const std::string& text)
{
    char* field = MessageField();
    memset(field, 0, sizeof(DAT_07db8710[0]));
    const size_t length = text.size() < sizeof(DAT_07db8710[0]) - 1 ? text.size() : sizeof(DAT_07db8710[0]) - 1;
    memcpy(field, text.data(), length);
    reinterpret_cast<DWORD*>(DAT_07d780a8)[0] = (DWORD)length;
}
}

namespace ChatHistory {
void Push(const char* line)
{
    ResetBrowse();
    if (!line || !*line || (s_count > 0 && s_entries[0] == line)) return;
    for (int i = kMaxEntries - 1; i > 0; --i) s_entries[i] = std::move(s_entries[i - 1]);
    s_entries[0] = line;
    if (s_count < kMaxEntries) ++s_count;
}

void ResetBrowse()
{
    s_browse = -1;
    s_draft.clear();
}

bool Browse(bool older)
{
    const int next = s_browse + (older ? 1 : -1);
    if (next < -1 || next >= s_count) return false;
    if (s_browse == -1) s_draft = MessageField();
    s_browse = next;
    SetMessageField(s_browse == -1 ? s_draft : s_entries[s_browse]);
    return true;
}
}
