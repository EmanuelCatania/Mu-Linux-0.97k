#pragma once
#include <cstdint>
namespace Platform {
enum class InputEventType { MouseMove, LeftDown, LeftUp, LeftDoubleClick, RightDown, RightUp, Wheel, MiddleDown, MiddleUp, ImeStart, ImeEnd, Text };
struct InputEvent { InputEventType type; int x = 0; int y = 0; int wheel = 0; uint32_t character = 0; };
struct ImeState { uint32_t conversion = 0; uint32_t sentence = 0; };
class IInput {
public:
    virtual ~IInput() = default;
    // Códigos lógicos heredados 0..255; conserva bits alto/bajo y consumo por consulta.
    virtual bool HasImeModeIndicator() const = 0;
    virtual int16_t KeyState(int key) = 0;
    virtual void CaptureMouse(bool capture) = 0;
    virtual ImeState ReadImeState(bool clearActive = false) = 0;
    virtual void PositionIme(int x, int y, int width, int height) = 0;
};
IInput& GetInput();
} // namespace Platform
