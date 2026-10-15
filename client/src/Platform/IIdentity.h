#pragma once
namespace Platform {
class IIdentity {
public:
    virtual ~IIdentity() = default;
    // 35 caracteres + NUL. En fallo sin identidad, output queda vacío y error explica la causa.
    virtual bool Read(char (&output)[36], const char*& error) = 0;
};
IIdentity& GetIdentity();
}
