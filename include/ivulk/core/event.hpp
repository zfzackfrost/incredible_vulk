/**
 * @file event.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Event handling types and functions.
 */

#pragma once

#include <ivulk/glm.hpp>

#include <SDL2/SDL.h>
#include <cstdint>

#include <functional>
#include <optional>
#include <variant>

namespace ivulk {
    enum class E_EventType : uint32_t
    {
        KeyUp,
        KeyDown,
        MouseUp,
        MouseDown,
        MouseMove,
    };
    namespace E_KeyCode {
        using Type = SDL_Keycode;

        // Number row
        constexpr Type Key0 = SDLK_0;
        constexpr Type Key1 = SDLK_1;
        constexpr Type Key2 = SDLK_2;
        constexpr Type Key3 = SDLK_3;
        constexpr Type Key4 = SDLK_4;
        constexpr Type Key5 = SDLK_5;
        constexpr Type Key6 = SDLK_6;
        constexpr Type Key7 = SDLK_7;
        constexpr Type Key8 = SDLK_8;
        constexpr Type Key9 = SDLK_9;

        // Number pad
        constexpr Type KeyNumpad0      = SDLK_KP_0;
        constexpr Type KeyNumpad1      = SDLK_KP_1;
        constexpr Type KeyNumpad2      = SDLK_KP_2;
        constexpr Type KeyNumpad3      = SDLK_KP_3;
        constexpr Type KeyNumpad4      = SDLK_KP_4;
        constexpr Type KeyNumpad5      = SDLK_KP_5;
        constexpr Type KeyNumpad6      = SDLK_KP_6;
        constexpr Type KeyNumpad7      = SDLK_KP_7;
        constexpr Type KeyNumpad8      = SDLK_KP_8;
        constexpr Type KeyNumpad9      = SDLK_KP_9;
        constexpr Type KeyNumpadPlus   = SDLK_KP_PLUS;
        constexpr Type KeyNumpadMinus  = SDLK_KP_MINUS;
        constexpr Type KeyNumpadMult   = SDLK_KP_MULTIPLY;
        constexpr Type KeyNumpadDivide = SDLK_KP_DIVIDE;
        constexpr Type KeyNumpadEnter  = SDLK_KP_ENTER;
        constexpr Type KeyNumpadDot    = SDLK_KP_PERIOD;

        // Latin letters
        constexpr Type KeyA = SDLK_a;
        constexpr Type KeyB = SDLK_b;
        constexpr Type KeyC = SDLK_c;
        constexpr Type KeyD = SDLK_d;
        constexpr Type KeyE = SDLK_e;
        constexpr Type KeyF = SDLK_f;
        constexpr Type KeyG = SDLK_g;
        constexpr Type KeyH = SDLK_h;
        constexpr Type KeyI = SDLK_i;
        constexpr Type KeyJ = SDLK_j;
        constexpr Type KeyK = SDLK_k;
        constexpr Type KeyL = SDLK_l;
        constexpr Type KeyM = SDLK_m;
        constexpr Type KeyN = SDLK_n;
        constexpr Type KeyO = SDLK_o;
        constexpr Type KeyP = SDLK_p;
        constexpr Type KeyQ = SDLK_q;
        constexpr Type KeyR = SDLK_r;
        constexpr Type KeyS = SDLK_s;
        constexpr Type KeyT = SDLK_t;
        constexpr Type KeyU = SDLK_u;
        constexpr Type KeyV = SDLK_v;
        constexpr Type KeyW = SDLK_w;
        constexpr Type KeyX = SDLK_x;
        constexpr Type KeyY = SDLK_y;
        constexpr Type KeyZ = SDLK_z;

        // Punctuation
        constexpr Type KeyBackslash    = SDLK_BACKSLASH;
        constexpr Type KeyBackquote    = SDLK_BACKQUOTE;
        constexpr Type KeyComma        = SDLK_COMMA;
        constexpr Type KeyEquals       = SDLK_EQUALS;
        constexpr Type KeyMinus        = SDLK_MINUS;
        constexpr Type KeyLeftBracket  = SDLK_LEFTBRACKET;
        constexpr Type KeyRightBracket = SDLK_RIGHTBRACKET;
        constexpr Type KeyPeriod       = SDLK_PERIOD;
        constexpr Type KeySlash        = SDLK_SLASH;
        constexpr Type KeySemicolon    = SDLK_SEMICOLON;

        // Special keys
        constexpr Type KeyBackspace = SDLK_BACKSPACE;
        constexpr Type KeyCapslock  = SDLK_CAPSLOCK;
        constexpr Type KeyDelete    = SDLK_DELETE;
        constexpr Type KeyEnd       = SDLK_END;
        constexpr Type KeyEsc       = SDLK_ESCAPE;
        constexpr Type KeyHome      = SDLK_HOME;
        constexpr Type KeyInsert    = SDLK_INSERT;
        constexpr Type KeyPageUp    = SDLK_PAGEUP;
        constexpr Type KeyPageDown  = SDLK_PAGEDOWN;
        constexpr Type KeyReturn    = SDLK_RETURN;
        constexpr Type KeySpace     = SDLK_SPACE;
        constexpr Type KeyTab       = SDLK_TAB;

        // Arrow keys
        constexpr Type KeyUpArrow    = SDLK_UP;
        constexpr Type KeyDownArrow  = SDLK_DOWN;
        constexpr Type KeyLeftArrow  = SDLK_LEFT;
        constexpr Type KeyRightArrow = SDLK_RIGHT;

        // Function keys
        constexpr Type KeyF1  = SDLK_F1;
        constexpr Type KeyF2  = SDLK_F2;
        constexpr Type KeyF3  = SDLK_F3;
        constexpr Type KeyF4  = SDLK_F4;
        constexpr Type KeyF5  = SDLK_F5;
        constexpr Type KeyF6  = SDLK_F6;
        constexpr Type KeyF7  = SDLK_F7;
        constexpr Type KeyF8  = SDLK_F8;
        constexpr Type KeyF9  = SDLK_F9;
        constexpr Type KeyF10 = SDLK_F10;
        constexpr Type KeyF11 = SDLK_F11;
        constexpr Type KeyF12 = SDLK_F12;

        // Modifiers
        constexpr Type KeyLeftAlt    = SDLK_LALT;
        constexpr Type KeyLeftCtrl   = SDLK_LCTRL;
        constexpr Type KeyLeftShift  = SDLK_LSHIFT;
        constexpr Type KeyRightAlt   = SDLK_RALT;
        constexpr Type KeyRightCtrl  = SDLK_RCTRL;
        constexpr Type KeyRightShift = SDLK_RSHIFT;

    }; // namespace E_KeyCode

    namespace E_MouseButton {
        using Type = Uint8;

        constexpr Type BtnLeft   = SDL_BUTTON_LEFT;
        constexpr Type BtnMiddle = SDL_BUTTON_MIDDLE;
        constexpr Type BtnRight  = SDL_BUTTON_RIGHT;
    } // namespace E_MouseButton

    struct KeyEvent
    {
        bool bRepeat : 1;
        bool bIsDown : 1;
        E_KeyCode::Type keycode;
    };

    struct MouseButtonEvent
    {
        bool bIsDown : 1;
        E_MouseButton::Type button;
    };
    struct MouseMoveEvent
    {
        glm::ivec2 mousePos;
        glm::ivec2 mouseDelta;
    };

    using AnyEventData = std::variant<KeyEvent, MouseButtonEvent, MouseMoveEvent>;

    struct Event
    {
        E_EventType type;
        AnyEventData data;

        KeyEvent assumeKeyEvent() { return std::get<KeyEvent>(data); }
        MouseButtonEvent assumeMouseBtnEvent() { return std::get<MouseButtonEvent>(data); }
        MouseMoveEvent assumeMouseMoveEvent() { return std::get<MouseMoveEvent>(data); }

        static inline Event makeKeyEvent(KeyEvent data)
        {
            return Event {
                .type = data.bIsDown ? E_EventType::KeyDown : E_EventType::KeyUp,
                .data = data,
            };
        }
        static inline Event makeMouseBtnEvent(MouseButtonEvent data)
        {
            return Event {
                .type = data.bIsDown ? E_EventType::MouseDown : E_EventType::MouseUp,
                .data = data,
            };
        }
        static inline Event makeMouseMoveEvent(MouseMoveEvent data)
        {
            return Event {
                .type = E_EventType::MouseMove,
                .data = data,
            };
        }

        static std::optional<Event> fromSDLEvent(SDL_Event evt);
    };

    class EventManager final
    {
    public:
        using Callback = std::function<void(Event)>;
        static void addCallback(E_EventType eventType, const Callback& cb);

        static void pushEvent(const Event evt);
        static void pushEvent(const SDL_Event sdlEvt);
        static void popEvent();
        static void processAllEvents();

    private:
        // Disable construction
        EventManager()                     = delete;
        EventManager(const EventManager&)  = delete;
        EventManager(const EventManager&&) = delete;
        ~EventManager()                    = delete;
    };
} // namespace ivulk
