#pragma once
namespace Platform {
using DiagnosticLog = void (*)(const char*);
class IDiagnostics {
public:
    virtual ~IDiagnostics() = default;
    virtual bool Install(DiagnosticLog log) = 0;
};
IDiagnostics& GetDiagnostics();
}
