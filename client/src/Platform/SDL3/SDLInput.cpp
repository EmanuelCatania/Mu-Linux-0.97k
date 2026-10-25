#include "Platform/SDL3/SDLPlatform.h"
#include "Platform/IWindow.h"
#include <array>
#include <cmath>
namespace Platform {
void DeliverSDLInput(const InputEvent& event);
namespace {
std::array<bool, 256> held{};
std::array<bool, 256> pressed{};
bool composing = false;
float wheelRemainder = 0;
int VirtualKey(SDL_Scancode key) {
    if (key >= SDL_SCANCODE_A && key <= SDL_SCANCODE_Z) return 'A' + key - SDL_SCANCODE_A;
    if (key >= SDL_SCANCODE_1 && key <= SDL_SCANCODE_9) return '1' + key - SDL_SCANCODE_1;
    if (key >= SDL_SCANCODE_F1 && key <= SDL_SCANCODE_F12) return 0x70 + key - SDL_SCANCODE_F1;
    if (key >= SDL_SCANCODE_KP_1 && key <= SDL_SCANCODE_KP_9) return 0x61 + key - SDL_SCANCODE_KP_1;
    switch (key) {
    case SDL_SCANCODE_0: return '0';
    case SDL_SCANCODE_BACKSPACE: return 8;
    case SDL_SCANCODE_TAB: return 9;
    case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: return 13;
    case SDL_SCANCODE_ESCAPE: return 27;
    case SDL_SCANCODE_SPACE: return 32;
    case SDL_SCANCODE_LSHIFT: return 0xa0;
    case SDL_SCANCODE_RSHIFT: return 0xa1;
    case SDL_SCANCODE_LCTRL: return 0xa2;
    case SDL_SCANCODE_RCTRL: return 0xa3;
    case SDL_SCANCODE_LALT: return 0xa4;
    case SDL_SCANCODE_RALT: return 0xa5;
    case SDL_SCANCODE_CAPSLOCK: return 20;
    case SDL_SCANCODE_PAGEUP: return 33;
    case SDL_SCANCODE_PAGEDOWN: return 34;
    case SDL_SCANCODE_END: return 35;
    case SDL_SCANCODE_HOME: return 36;
    case SDL_SCANCODE_LEFT: return 37;
    case SDL_SCANCODE_UP: return 38;
    case SDL_SCANCODE_RIGHT: return 39;
    case SDL_SCANCODE_DOWN: return 40;
    case SDL_SCANCODE_PRINTSCREEN: return 44;
    case SDL_SCANCODE_INSERT: return 45;
    case SDL_SCANCODE_DELETE: return 46;
    case SDL_SCANCODE_KP_0: return 0x60;
    case SDL_SCANCODE_KP_MULTIPLY: return 0x6a;
    case SDL_SCANCODE_KP_PLUS: return 0x6b;
    case SDL_SCANCODE_KP_MINUS: return 0x6d;
    case SDL_SCANCODE_KP_PERIOD: return 0x6e;
    case SDL_SCANCODE_KP_DIVIDE: return 0x6f;
    default: return 0;
    }
}
void SetKey(int key, bool down) {
    if (!key) return;
    if (down && !held[key]) pressed[key] = true;
    held[key] = down;
}
void Composition(bool active) {
    if (composing == active) return;
    composing = active;
    DeliverSDLInput({active ? InputEventType::ImeStart : InputEventType::ImeEnd});
}
void Text(const char* utf8) {
    Composition(false);
    // El texto del cliente es UTF-8 (Fase 4): se entrega cada code point tal
    // cual y el campo de texto lo guarda en UTF-8. Lo inválido se descarta.
    const auto* p = reinterpret_cast<const unsigned char*>(utf8);
    while (*p) {
        uint32_t code = *p++;
        int rest = 0;
        if (code >= 0xf0) { code &= 7; rest = 3; }
        else if (code >= 0xe0) { code &= 15; rest = 2; }
        else if (code >= 0xc0) { code &= 31; rest = 1; }
        else if (code >= 0x80) continue;
        while (rest-- > 0) { if ((*p & 0xc0) != 0x80) return; code = (code << 6) | (*p++ & 63); }
        InputEvent event{InputEventType::Text}; event.character = code; DeliverSDLInput(event);
    }
}
bool SDLCALL Watch(void*, SDL_Event* event) {
    if (!SDL_IsMainThread()) return true;
    InputEvent input{};
    switch (event->type) {
    case SDL_EVENT_KEY_DOWN: case SDL_EVENT_KEY_UP: {
        const int key = VirtualKey(event->key.scancode);
        SetKey(key, event->key.down);
        SetKey(16, held[0xa0] || held[0xa1]);
        SetKey(17, held[0xa2] || held[0xa3]);
        SetKey(18, held[0xa4] || held[0xa5]);
        if (event->key.down && (key == 8 || key == 9 || key == 13 || key == 27)) {
            input.type = InputEventType::Text; input.character = key; DeliverSDLInput(input);
        }
        // Ctrl+V: SDL no genera texto para la combinación; se entrega el mismo
        // carácter de control que arma TranslateMessage en Win32.
        if (event->key.down && key == 'V' && held[17] && !held[18]) {
            input.type = InputEventType::Text; input.character = 0x16; DeliverSDLInput(input);
        }
        return true;
    }
    case SDL_EVENT_TEXT_INPUT: Text(event->text.text); return true;
    case SDL_EVENT_TEXT_EDITING: Composition(event->edit.text && *event->edit.text); return true;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        held.fill(false); pressed.fill(false); Composition(false);
        DeliverSDLInput({InputEventType::LeftUp}); DeliverSDLInput({InputEventType::RightUp});
        DeliverSDLInput({InputEventType::MiddleUp}); return true;
    case SDL_EVENT_MOUSE_MOTION:
        input.type = InputEventType::MouseMove; input.x = int(event->motion.x); input.y = int(event->motion.y); break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN: case SDL_EVENT_MOUSE_BUTTON_UP: {
        const bool down = event->button.down;
        input.x = int(event->button.x); input.y = int(event->button.y);
        switch (event->button.button) {
        case SDL_BUTTON_LEFT:
            SetKey(1, down); input.type = down ? (event->button.clicks == 2 ? InputEventType::LeftDoubleClick : InputEventType::LeftDown) : InputEventType::LeftUp; break;
        case SDL_BUTTON_RIGHT: SetKey(2, down); input.type = down ? InputEventType::RightDown : InputEventType::RightUp; break;
        case SDL_BUTTON_MIDDLE: SetKey(4, down); input.type = down ? InputEventType::MiddleDown : InputEventType::MiddleUp; break;
        default: return true;
        }
        break;
    }
    case SDL_EVENT_MOUSE_WHEEL:
        wheelRemainder += event->wheel.y * (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f);
        input.type = InputEventType::Wheel;
        input.wheel = int(wheelRemainder) * 120; wheelRemainder -= int(wheelRemainder);
        if (!input.wheel) return true;
        break;
    case SDL_EVENT_QUIT: case SDL_EVENT_WINDOW_CLOSE_REQUESTED: GetWindow().RequestCloseApplication(); return true;
    default: return true;
    }
    DeliverSDLInput(input);
    return true;
}
class SDLInput final : public IInput {
public:
    bool HasImeModeIndicator() const override {
#ifdef _WIN32
        return true;
#else
        // DESVIACION aprobada: Linux no muestra un modo IMM que SDL no proporciona.
        return false;
#endif
    }
    int16_t KeyState(int key) override {
        if (key < 0 || key >= 256) return 0;
        const auto result = static_cast<int16_t>((held[key] ? 0x8000 : 0) | (pressed[key] ? 1 : 0));
        pressed[key] = false;
        return result;
    }
    void CaptureMouse(bool capture) override { SDL_CaptureMouse(capture); }
    ImeState ReadImeState(bool clearActive) override {
        ImeState result{composing ? 1u : 0u, 0};
        // SDL expresa composición, no los bits de modo de idioma de IMM32.
        if (clearActive && composing) { SDL_ClearComposition(SDLWindow()); Composition(false); }
        return result;
    }
    void PositionIme(int x, int y, int width, int height) override {
        SDL_Rect area{x, y, width > x ? width - x : 1, 1};
        SDL_SetTextInputArea(SDLWindow(), &area, 0);
    }
    std::string ClipboardText() override {
        std::string result;
        if (char* text = SDL_GetClipboardText()) { result = text; SDL_free(text); }
        return result;
    }
};
}
IInput& GetInput() { static SDLInput input; return input; }
void InstallSDLInput() { held.fill(false); pressed.fill(false); composing = false; wheelRemainder = 0; SDL_AddEventWatch(Watch, nullptr); }
void RemoveSDLInput() { SDL_RemoveEventWatch(Watch, nullptr); }
void ServiceSDLEvents(bool pumpNative) {
    if (pumpNative) SDL_PumpEvents();
    // El watch ya entregó los eventos sin diferirlos respecto del pump del host.
    SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
}
}
