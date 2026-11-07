#pragma once
#include <cstdint>
namespace Platform {
using DiagnosticLog = void (*)(const char*);
class IDiagnostics {
public:
    virtual ~IDiagnostics() = default;
    virtual bool Install(DiagnosticLog log) = 0;
    virtual std::int64_t MeasureClockRate(std::uint32_t milliseconds) = 0;
};
IDiagnostics& GetDiagnostics();
}
