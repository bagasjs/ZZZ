/*
Windowing Library
zzz - v0.1.00 - 2023-09-25

Bagas Jonathan Sitanggang - bagasjssoftware@gmail.com
*/

#ifndef ZZZ_H
#define ZZZ_H


#ifdef __cplusplus
extern "C" {
#endif

/** 
 * Macros 
 */
#if defined(_WIN32) || defined(__WIN32__)
    #include <windows.h>
    #include <windowsx.h>
    #define ZZZ_PLATFORM_WINDOWS 1
    #define ZZZ_PLATFORM_DESKTOP 1
    #ifndef _WIN64
        #error "Only supporting 64-bit windows"
    #endif
#elif defined(__ANDROID__)
    #define ZZZ_PLATFORM_ANDROID 1
    #define ZZZ_PLATFORM_LINUX 1
    #define ZZZ_PLATFORM_MOBILE 1
#elif defined(__gnu_linux__) || defined(__linux__)
    #define ZZZ_PLATFORM_LINUX 1
    #define ZZZ_PLATFORM_DESKTOP 1
#elif __APPLE__
    #define ZZZ_PLATFORM_APPLE 1
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #define ZZZ_PLATFORM_MOBILE 1
    #elif TARGET_OS_MAC
        #define ZZZ_PLATFORM_DESKTOP 1
    #else
        #warn "Unknown apple platform defaulting to desktop environment"
        #define ZZZ_PLATFORM_DESKTOP 1
    #endif
    #error "Apple platform is unsupported currently"
#elif defined(__unix__)
    #define ZZZ_PLATFORM_UNIX 1
    #error "Unix platform is unsupported"
#elif defined(_POSIX_VERSION)
    #define ZZZ_PLATFORM_POSIX 1
    #error "Unknown posix platform, only supporting linux, android and apple platforms"
#else
    #error "Unsupported platform"
#endif

#ifndef ZZZ_PLATFORM_DESKTOP
    #define ZZZ_PLATFORM_DESKTOP 0
#endif
#ifndef ZZZ_PLATFORM_MOBILE
    #define ZZZ_PLATFORM_MOBILE 0
#endif
#ifndef ZZZ_PLATFORM_WINDOWS
    #define ZZZ_PLATFORM_WINDOWS 0
#endif
#ifndef ZZZ_PLATFORM_ANDROID
    #define ZZZ_PLATFORM_ANDROID 0
#endif
#ifndef ZZZ_PLATFORM_APPLE
    #define ZZZ_PLATFORM_APPLE 0
#endif
#ifndef ZZZ_PLATFORM_LINUX
    #define ZZZ_PLATFORM_LINUX 0
#endif
#ifndef ZZZ_PLATFORM_UNIX
    #define ZZZ_PLATFORM_UNIX 0
#endif
#ifndef ZZZ_PLATFORM_POSIX
    #define ZZZ_PLATFORM_POSIX 0
#endif

#if defined(__clang__)
    #define ZZZ_CC_CLANG 1
#elif defined(_MSC_VER)
    #if !ZZZ_PLATFORM_WINDOWS
        #error "Unreachable: MSVC should only be in Windows"
    #endif
    #define ZZZ_CC_MSVC 1
#elif defined(__GNUC__) || defined(__GNUG__)
    #define ZZZ_CC_GCC 1
#else
    #error "Unsupported compilers"
#endif

#ifndef ZZZ_CC_CLANG
    #define ZZZ_CC_CLANG 0
#endif
#ifndef ZZZ_CC_MSVC
    #define ZZZ_CC_MSVC 0
#endif
#ifndef ZZZ_CC_GCC
    #define ZZZ_CC_GCC 0
#endif

#ifndef NULL
    #define NULL ((void*)0)
#endif
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

/** 
 * Configurations
 */
#define ZZZ_EVENT_QUEUE_CAPACITY 64

#ifdef ZZZ_RELEASE
    #define NDEBUG 1
#endif

/** 
 * Type Aliases
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float f32;
typedef double f64;
typedef u32 b32; // bool32

/** 
 * Core Types
 */
typedef i32 ZErr;

typedef struct {
    i32 type;
    struct { i32 width, height; } size;
    struct { f32 x, y; } scroll;
    struct { i32 key, scancode, mods; } keyboard;
    struct { i32 x, y, width, height; } window;
    struct { i32 button, mods; } mouse;
    struct { f32 x, y; } cursor;
    struct { char** paths; i32 count; } file;
    struct { f32 x, y; } scale;
} ZEvent;

typedef struct {
    u64 tail, head;
    ZEvent events[ZZZ_EVENT_QUEUE_CAPACITY];
} ZEventQueue;

typedef struct {
#if ZZZ_PLATFORM_DESKTOP && ZZZ_PLATFORM_WINDOWS
    HINSTANCE hInstance;
    HWND hWnd;
    ATOM mainWindowClass;
#endif
} ZSurface;

typedef struct {
    ZEventQueue eq;
    ZSurface surface;
} ZZZ;

typedef struct {
    const char* name;
#ifdef ZZZ_PLATFORM_DESKTOP
    u32 surfaceWidth, surfaceHeight;
#endif
} ZZZInitInfo;

/** 
 * Functions
 */

void* zMemReserve(u64 size);
void zMemRelease(void* ptr);
void zMemSet(void* dst, i32 value, u64 nbytes);
void zMemZero(void* dst, u64 nbytes);
void zMemCopy(void* dst, const void* src, u64 nbytes);

ZErr zInit(ZZZ* app, const ZZZInitInfo* info);
void zSetWindowVisibility(ZZZ* app, b32 should_visible);
void zTerminate(ZZZ* app);
void zPollEvents(ZZZ* app);
b32 zNextEvent(ZZZ* app, ZEvent* event);

/** 
 * Enums
 */
enum {
    ZERR_NONE = 0,
    ZERR_INVALID_ARGUMENTS,
    ZERR_FAILED_TO_GET_WIN32_INSTANCE,
    ZERR_FAILED_TO_REGISTER_WIN32_WINDOW_CLASS,
    ZERR_FAILED_TO_CREATE_WIN32_WINDOW,
};

enum {
    ZEVENT_UNKNOWN = 0,
    ZEVENT_WINDOW_MOVED,
    ZEVENT_WINDOW_RESIZED,
    ZEVENT_WINDOW_CLOSED,
    ZEVENT_WINDOW_REFRESH,
    ZEVENT_WINDOW_GAIN_FOCUS,
    ZEVENT_WINDOW_LOST_FOCUS,
    ZEVENT_WINDOW_ICONIFIED,
    ZEVENT_WINDOW_UNICONIFIED,
    ZEVENT_FRAMEBUFFER_RESIZED,
    ZEVENT_BUTTON_PRESSED,
    ZEVENT_BUTTON_RELEASED,
    ZEVENT_CURSOR_MOVED,
    ZEVENT_CURSOR_ENTERED,
    ZEVENT_CURSOR_LEFT,
    ZEVENT_SCROLLED,
    ZEVENT_KEY_PRESSED,
    ZEVENT_KEY_REPEATED,
    ZEVENT_KEY_RELEASED,
    ZEVENT_CODEPOINT_INPUT,
    ZEVENT_MONITOR_CONNECTED,
    ZEVENT_MONITOR_DISCONNECTED,
    ZEVENT_FILE_DROPPED,
    ZEVENT_JOYSTICK_CONNECTED,
    ZEVENT_JOYSTICK_DISCONNECTED,
    ZEVENT_WINDOW_MAXIMIZED,
    ZEVENT_WINDOW_UNMAXIMIZED,
    ZEVENT_SCALE_CHANGED,
};

enum {
    ZKEY_UNKNOWN = -1,
    ZKEY_SPACE = 32,
    ZKEY_APOSTROPHE = 39, /* ' */
    ZKEY_COMMA = 44, /* , */
    ZKEY_MINUS = 45, /* - */
    ZKEY_PERIOD = 46, /* . */
    ZKEY_SLASH = 47, /* / */
    ZKEY_0 = 48,
    ZKEY_1 = 49,
    ZKEY_2 = 50,
    ZKEY_3 = 51,
    ZKEY_4 = 52,
    ZKEY_5 = 53,
    ZKEY_6 = 54,
    ZKEY_7 = 55,
    ZKEY_8 = 56,
    ZKEY_9 = 57,
    ZKEY_SEMICOLON = 59, /* ; */
    ZKEY_EQUAL = 61, /* = */
    ZKEY_A = 65,
    ZKEY_B = 66,
    ZKEY_C = 67,
    ZKEY_D = 68,
    ZKEY_E = 69,
    ZKEY_F = 70,
    ZKEY_G = 71,
    ZKEY_H = 72,
    ZKEY_I = 73,
    ZKEY_J = 74,
    ZKEY_K = 75,
    ZKEY_L = 76,
    ZKEY_M = 77,
    ZKEY_N = 78,
    ZKEY_O = 79,
    ZKEY_P = 80,
    ZKEY_Q = 81,
    ZKEY_R = 82,
    ZKEY_S = 83,
    ZKEY_T = 84,
    ZKEY_U = 85,
    ZKEY_V = 86,
    ZKEY_W = 87,
    ZKEY_X = 88,
    ZKEY_Y = 89,
    ZKEY_Z = 90,
    ZKEY_LEFT_BRACKET = 91, /* [ */
    ZKEY_BACKSLASH = 92, /* \ */
    ZKEY_RIGHT_BRACKET = 93, /* ] */
    ZKEY_GRAVE_ACCENT = 96, /* ` */
    ZKEY_WORLD_1 = 161, /* non-US #1 */
    ZKEY_WORLD_2 = 162, /* non-US #2 */
    ZKEY_ESCAPE = 256,
    ZKEY_ENTER = 257,
    ZKEY_TAB = 258,
    ZKEY_BACKSPACE = 259,
    ZKEY_INSERT = 260,
    ZKEY_DELETE = 261,
    ZKEY_RIGHT = 262,
    ZKEY_LEFT = 263,
    ZKEY_DOWN = 264,
    ZKEY_UP = 265,
    ZKEY_PAGE_UP = 266,
    ZKEY_PAGE_DOWN = 267,
    ZKEY_HOME = 268,
    ZKEY_END = 269,
    ZKEY_CAPS_LOCK = 280,
    ZKEY_SCROLL_LOCK = 281,
    ZKEY_NUM_LOCK = 282,
    ZKEY_PRINT_SCREEN = 283,
    ZKEY_PAUSE = 284,
    ZKEY_F1 = 290,
    ZKEY_F2 = 291,
    ZKEY_F3 = 292,
    ZKEY_F4 = 293,
    ZKEY_F5 = 294,
    ZKEY_F6 = 295,
    ZKEY_F7 = 296,
    ZKEY_F8 = 297,
    ZKEY_F9 = 298,
    ZKEY_F10 = 299,
    ZKEY_F11 = 300,
    ZKEY_F12 = 301,
    ZKEY_F13 = 302,
    ZKEY_F14 = 303,
    ZKEY_F15 = 304,
    ZKEY_F16 = 305,
    ZKEY_F17 = 306,
    ZKEY_F18 = 307,
    ZKEY_F19 = 308,
    ZKEY_F20 = 309,
    ZKEY_F21 = 310,
    ZKEY_F22 = 311,
    ZKEY_F23 = 312,
    ZKEY_F24 = 313,
    ZKEY_F25 = 314,
    ZKEY_KP_0 = 320,
    ZKEY_KP_1 = 321,
    ZKEY_KP_2 = 322,
    ZKEY_KP_3 = 323,
    ZKEY_KP_4 = 324,
    ZKEY_KP_5 = 325,
    ZKEY_KP_6 = 326,
    ZKEY_KP_7 = 327,
    ZKEY_KP_8 = 328,
    ZKEY_KP_9 = 329,
    ZKEY_KP_DECIMAL = 330,
    ZKEY_KP_DIVIDE = 331,
    ZKEY_KP_MULTIPLY = 332,
    ZKEY_KP_SUBTRACT = 333,
    ZKEY_KP_ADD = 334,
    ZKEY_KP_ENTER = 335,
    ZKEY_KP_EQUAL = 336,
    ZKEY_LEFT_SHIFT = 340,
    ZKEY_LEFT_CONTROL = 341,
    ZKEY_LEFT_ALT = 342,
    ZKEY_LEFT_SUPER = 343,
    ZKEY_RIGHT_SHIFT = 344,
    ZKEY_RIGHT_CONTROL = 345,
    ZKEY_RIGHT_ALT = 346,
    ZKEY_RIGHT_SUPER = 347,
    ZKEY_MENU = 348,
};

enum {
    ZKEY_MOD_SHIFT = 0x0001,
    ZKEY_MOD_CONTROL = 0x0002,
    ZKEY_MOD_ALT = 0x0004,
    ZKEY_MOD_SUPER = 0x0008,
    ZKEY_MOD_CAPS_LOCK = 0x0010,
    ZKEY_MOD_NUM_LOCK = 0x0020,
};


#ifdef __cplusplus
}
#endif

#endif // ZZZ_H

/*

MIT License

Copyright (c) 2023 Bagas Jonathan Sitanggang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

**/
