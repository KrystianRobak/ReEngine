#pragma once

#include <bitset>
#include <cstdint>
#include <functional>

// ECS
using Entity = std::uint32_t;

const Entity APPLICATION = -1;
const Entity GUI = -2;
const Entity WINDOW = -3;

const Entity MAX_ENTITIES = 110;
const Entity MAX_LIGHT_ENTITIES = 10;
const Entity MAX_OBJECT_ENTITIES = 100;
using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


// Input
enum class InputButtons {
    NUL = 0,  // Null character
    SOH = 1,  // Start of Header
    STX = 2,  // Start of Text
    ETX = 3,  // End of Text
    EOT = 4,  // End of Transmission
    ENQ = 5,  // Enquiry
    ACK = 6,  // Acknowledge
    BEL = 7,  // Bell
    BS  = 8,  // Backspace
    TAB = 9,  // Horizontal Tab
    LF  = 10, // Line Feed
    VT  = 11, // Vertical Tab
    FF  = 12, // Form Feed
    CR  = 13, // Carriage Return
    SO  = 14, // Shift Out
    SI  = 15, // Shift In
    DLE = 16, // Data Link Escape
    DC1 = 17, // Device Control 1
    DC2 = 18, // Device Control 2
    DC3 = 19, // Device Control 3
    DC4 = 20, // Device Control 4
    NAK = 21, // Negative Acknowledge
    SYN = 22, // Synchronous Idle
    ETB = 23, // End of Transmission Block
    CAN = 24, // Cancel
    EM  = 25, // End of Medium
    SUB = 26, // Substitute
    ESC = 27, // Escape
    FS  = 28, // File Separator
    GS  = 29, // Group Separator
    RS  = 30, // Record Separator
    US  = 31, // Unit Separator
    SPACE = 32,
    EXCLAMATION_MARK = 33,
    DOUBLE_QUOTE = 34,
    HASH = 35,
    DOLLAR = 36,
    PERCENT = 37,
    AMPERSAND = 38,
    SINGLE_QUOTE = 39,
    LEFT_PARENTHESIS = 40,
    RIGHT_PARENTHESIS = 41,
    ASTERISK = 42,
    PLUS = 43,
    COMMA = 44,
    MINUS = 45,
    PERIOD = 46,
    SLASH = 47,
    ZERO = 48,
    ONE = 49,
    TWO = 50,
    THREE = 51,
    FOUR = 52,
    FIVE = 53,
    SIX = 54,
    SEVEN = 55,
    EIGHT = 56,
    NINE = 57,
    COLON = 58,
    SEMICOLON = 59,
    LESS_THAN = 60,
    EQUAL = 61,
    GREATER_THAN = 62,
    QUESTION_MARK = 63,
    AT = 64,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LEFT_SQUARE_BRACKET = 91,
    BACKSLASH = 92,
    RIGHT_SQUARE_BRACKET = 93,
    CARET = 94,
    UNDERSCORE = 95,
    GRAVE_ACCENT = 96,
    a = 97,
    b = 98,
    c = 99,
    d = 100,
    e = 101,
    f = 102,
    g = 103,
    h = 104,
    i = 105,
    j = 106,
    k = 107,
    l = 108,
    m = 109,
    n = 110,
    o = 111,
    p = 112,
    q = 113,
    r = 114,
    s = 115,
    t = 116,
    u = 117,
    v = 118,
    w = 119,
    x = 120,
    y = 121,
    z = 122,
    LEFT_CURLY_BRACE = 123,
    PIPE = 124,
    RIGHT_CURLY_BRACE = 125,
    TILDE = 126,
    DEL = 127 // Delete
};

enum MenuType
{
	BaseMenu,
	AnimationMenu
};

// Events
using EventType = std::string;
using ParamId = std::string;

#define METHOD_LISTENER_NO_PARAM(EventType, Listener) EventType, std::bind(&Listener, this)
#define METHOD_LISTENER_ONE_PARAM(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1)
#define METHOD_LISTENER_TWO_PARAM(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1, std::placeholders::_2)
#define METHOD_LISTENER_THREE_PARAM(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define METHOD_LISTENER_FOUR_PARAM(EventType, Listener) EventType, std::bind(&Listener, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)

#define FUNCTION_LISTENER_ONE_PARAM(EventType, Listener) EventType, std::bind(&Listener, std::placeholders::_1)


namespace Events::Window {
	const EventType QUIT = "Events::Window::QUIT";
	const EventType RESIZED = "Events::Window::RESIZED";
	const EventType INPUT = "Events::Window::INPUT";
}

namespace Events::Window::Input {
	const ParamId INPUT = "Events::Window::Input::INPUT";
}

namespace Events::Window::Resized {
	const ParamId WIDTH = "Events::Window::Resized::WIDTH";
	const ParamId HEIGHT = "Events::Window::Resized::HEIGHT";
}

namespace Events::Application {
	const EventType TOGGLE = "Events::Application::TOGGLE";
	const EventType RECOMPILE_SHADER = "Events::Application::RECOMPILE_SHADER";
	const EventType LIGHT_ENTITY_ADDED = "Events::Application::LIGHT_ENTITY_ADDED";
	const EventType MENU_CHANGED = "Events::Application::MENU_CHANGED";
}