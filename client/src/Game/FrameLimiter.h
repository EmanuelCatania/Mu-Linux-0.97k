#pragma once
#include <windows.h>

class CFrameLimiter
{
public:
    DWORD Wait(DWORD renderStart) const;
};

extern CFrameLimiter gFrameLimiter;
