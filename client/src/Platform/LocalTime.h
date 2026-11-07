#pragma once
#include "Platform/ITimer.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <chrono>
#include <ctime>
#include <stdexcept>
#endif
namespace Platform {
inline LocalDateTime ReadLocalTime() {
#ifdef _WIN32
    SYSTEMTIME t; GetLocalTime(&t);
    return {t.wYear,t.wMonth,t.wDayOfWeek,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds};
#else
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::system_clock::to_time_t(now);
    std::tm t{};
    if (!localtime_r(&seconds, &t)) throw std::runtime_error("No se pudo obtener la fecha local");
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return {static_cast<uint16_t>(t.tm_year+1900),static_cast<uint16_t>(t.tm_mon+1),
        static_cast<uint16_t>(t.tm_wday),static_cast<uint16_t>(t.tm_mday),
        static_cast<uint16_t>(t.tm_hour),static_cast<uint16_t>(t.tm_min),
        static_cast<uint16_t>(t.tm_sec),static_cast<uint16_t>((millis%1000+1000)%1000)};
#endif
}
}
