#include "zzz.h"
#include "zzz_internal.h"

LRESULT CALLBACK _zWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

ZErr zInit(ZZZ* app, const ZZZInitInfo* info)
{
    if(!app || !info) {
        return ZERR_INVALID_ARGUMENTS;
    }
    zMemZero(app, sizeof(ZZZ));

    WNDCLASSA wc;
    u64 wndclass_size = sizeof(WNDCLASSA);
    const char* wndclass_name = "FluxWindowClass1";
    RECT wr;

    app->surface.hInstance = (HINSTANCE)GetModuleHandleA(NULL);
    if(!app->surface.hInstance) {
        return ZERR_FAILED_TO_GET_WIN32_INSTANCE;
    }
    zMemZero(&wc, wndclass_size);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = _zWindowProc;
    wc.hInstance = app->surface.hInstance;
    wc.hIcon = LoadIcon(app->surface.hInstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = wndclass_name;

    app->surface.mainWindowClass = RegisterClassA(&wc);
    if(!app->surface.mainWindowClass) {
        return ZERR_FAILED_TO_REGISTER_WIN32_WINDOW_CLASS;
    }

    zMemZero(&wr, sizeof(RECT));
    wr.right = info->surfaceWidth;
    wr.bottom = info->surfaceHeight;
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    HWND handle = CreateWindowExA(
            WS_EX_APPWINDOW,
            MAKEINTATOM(app->surface.mainWindowClass),
            info->name,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            wr.right - wr.left,
            wr.bottom - wr.top,
            NULL,
            NULL,
            app->surface.hInstance,
            NULL);

    if(!handle) {
        return ZERR_FAILED_TO_CREATE_WIN32_WINDOW;
    } else {
        SetPropA(handle, "ZZZ", &app->eq);
        app->surface.hWnd = handle;
    }

    return ZERR_NONE;
}

void zSetWindowVisibility(ZZZ* app, b32 should_visible)
{
    i32 show_window_command_flag = should_visible ? SW_SHOWNA : SW_HIDE;
    ShowWindow(app->surface.hWnd, show_window_command_flag);
}

void zPollEvents(ZZZ* app)
{
    (void)app;
    MSG msg;

    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) {
            _zNewEvent(&app->eq, ZEVENT_WINDOW_CLOSED);
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

static int _zWin32GetKeyMods(void);
static i32 _zWin32Scancode2Keycode(i32 scancode);

LRESULT CALLBACK _zWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ZEventQueue* eq = (ZEventQueue*)GetPropA(hWnd, "ZZZ");
    if(!eq) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    switch(uMsg) {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
        case WM_SIZE:
            {
                RECT r;
                GetClientRect(hWnd, &r);
                ZEvent* ev = _zNewEvent(eq, ZEVENT_WINDOW_RESIZED);
                if(!ev) {
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                }
                ev->window.width = r.right - r.left;
                ev->window.height = r.bottom - r.top;
            } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            {
                i32 key, scancode;
                const i32 action = (HIWORD(lParam) & KF_UP) ? ZEVENT_KEY_RELEASED : ZEVENT_KEY_PRESSED;
                const i32 mods = _zWin32GetKeyMods();

                scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
                if(!scancode) {
                    scancode = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_VSC);
                }

                // HACK: Alt+PrtSc has a different scancode than just PrtSc
                if (scancode == 0x54)
                    scancode = 0x137;

                // HACK: Ctrl+Pause has a different scancode than just Pause
                if (scancode == 0x146)
                    scancode = 0x45;

                // HACK: CJK IME sets the extended bit for right Shift
                if (scancode == 0x136)
                    scancode = 0x36;

                key = _zWin32Scancode2Keycode(scancode);

                // The Ctrl keys require special handling
                if (wParam == VK_CONTROL)
                {
                    if (HIWORD(lParam) & KF_EXTENDED)
                    {
                        // Right side keys have the extended key bit set
                        key = ZKEY_RIGHT_CONTROL;
                    }
                    else
                    {
                        // NOTE: Alt Gr sends Left Ctrl followed by Right Alt
                        // HACK: We only want one event for Alt Gr, so if we detect
                        //       this sequence we discard this Left Ctrl message now
                        //       and later report Right Alt normally
                        MSG next;
                        const DWORD time = GetMessageTime();

                        if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
                        {
                            if (next.message == WM_KEYDOWN ||
                                    next.message == WM_SYSKEYDOWN ||
                                    next.message == WM_KEYUP ||
                                    next.message == WM_SYSKEYUP)
                            {
                                if (next.wParam == VK_MENU &&
                                        (HIWORD(next.lParam) & KF_EXTENDED) &&
                                        next.time == time)
                                {
                                    // Next message is Right Alt down so discard this
                                    break;
                                }
                            }
                        }

                        // This is a regular Left Ctrl message
                        key = ZKEY_LEFT_CONTROL;
                    }
                }
                else if (wParam == VK_PROCESSKEY)
                {
                    // IME notifies that keys have been filtered by setting the
                    // virtual key-code to VK_PROCESSKEY
                    break;
                }

                if (action == ZEVENT_KEY_RELEASED && wParam == VK_SHIFT)
                {
                    // HACK: Release both Shift keys on Shift up event, as when both
                    //       are pressed the first release does not emit any event
                    // NOTE: The other half of this is in _glfwPollEventsWin32

                    _zInputKey(eq, ZKEY_LEFT_SHIFT, scancode, action, mods);
                    _zInputKey(eq, ZKEY_RIGHT_SHIFT, scancode, action, mods);
                }
                else if (wParam == VK_SNAPSHOT)
                {
                    // HACK: Key down is not reported for the Print Screen key
                    _zInputKey(eq, key, scancode, ZEVENT_KEY_PRESSED, mods);
                    _zInputKey(eq, key, scancode, ZEVENT_KEY_RELEASED, mods);
                }
                else
                    _zInputKey(eq, key, scancode, action, mods);

            } break;
        default:
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void* zMemReserve(u64 nbytes)
{
    void* res = VirtualAllocEx(
        GetCurrentProcess(), 
        NULL, 
        nbytes, 
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);
    if(res == NULL || res == INVALID_HANDLE_VALUE)
        return NULL;
    return res;
}

void zMemRelease(void* ptr)
{
    VirtualFreeEx(
        GetCurrentProcess(), 
        (LPVOID)ptr, 
        0,
        MEM_RELEASE);
}

void zMemSet(void* dst, i32 value, u64 nbytes)
{
    for(u64 i = 0; i < nbytes; ++i)
        ((i8*)dst)[i] = (i8)value;
}

void zMemZero(void* dst, u64 nbytes)
{
    return zMemSet(dst, 0, nbytes);
}

void zMemCopy(void* dst, const void* src, u64 nbytes)
{
    for(u64 i = 0; i < nbytes; ++i)
        ((i8*)dst)[i] = ((i8*)src)[i];
}

int _zWin32GetKeyMods(void)
{
    int mods = 0;

    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= ZKEY_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= ZKEY_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= ZKEY_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= ZKEY_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)
        mods |= ZKEY_MOD_CAPS_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)
        mods |= ZKEY_MOD_NUM_LOCK;

    return mods;
}

i32 _zWin32Scancode2Keycode(i32 scancode)
{
    switch(scancode) {
        case 0x00B: return ZKEY_0;
        case 0x002: return ZKEY_1;
        case 0x003: return ZKEY_2;
        case 0x004: return ZKEY_3;
        case 0x005: return ZKEY_4;
        case 0x006: return ZKEY_5;
        case 0x007: return ZKEY_6;
        case 0x008: return ZKEY_7;
        case 0x009: return ZKEY_8;
        case 0x00A: return ZKEY_9;
        case 0x01E: return ZKEY_A;
        case 0x030: return ZKEY_B;
        case 0x02E: return ZKEY_C;
        case 0x020: return ZKEY_D;
        case 0x012: return ZKEY_E;
        case 0x021: return ZKEY_F;
        case 0x022: return ZKEY_G;
        case 0x023: return ZKEY_H;
        case 0x017: return ZKEY_I;
        case 0x024: return ZKEY_J;
        case 0x025: return ZKEY_K;
        case 0x026: return ZKEY_L;
        case 0x032: return ZKEY_M;
        case 0x031: return ZKEY_N;
        case 0x018: return ZKEY_O;
        case 0x019: return ZKEY_P;
        case 0x010: return ZKEY_Q;
        case 0x013: return ZKEY_R;
        case 0x01F: return ZKEY_S;
        case 0x014: return ZKEY_T;
        case 0x016: return ZKEY_U;
        case 0x02F: return ZKEY_V;
        case 0x011: return ZKEY_W;
        case 0x02D: return ZKEY_X;
        case 0x015: return ZKEY_Y;
        case 0x02C: return ZKEY_Z;
        case 0x028: return ZKEY_APOSTROPHE;
        case 0x02B: return ZKEY_BACKSLASH;
        case 0x033: return ZKEY_COMMA;
        case 0x00D: return ZKEY_EQUAL;
        case 0x029: return ZKEY_GRAVE_ACCENT;
        case 0x01A: return ZKEY_LEFT_BRACKET;
        case 0x00C: return ZKEY_MINUS;
        case 0x034: return ZKEY_PERIOD;
        case 0x01B: return ZKEY_RIGHT_BRACKET;
        case 0x027: return ZKEY_SEMICOLON;
        case 0x035: return ZKEY_SLASH;
        case 0x056: return ZKEY_WORLD_2;
        case 0x00E: return ZKEY_BACKSPACE;
        case 0x153: return ZKEY_DELETE;
        case 0x14F: return ZKEY_END;
        case 0x01C: return ZKEY_ENTER;
        case 0x001: return ZKEY_ESCAPE;
        case 0x147: return ZKEY_HOME;
        case 0x152: return ZKEY_INSERT;
        case 0x15D: return ZKEY_MENU;
        case 0x151: return ZKEY_PAGE_DOWN;
        case 0x149: return ZKEY_PAGE_UP;
        case 0x045: return ZKEY_PAUSE;
        case 0x039: return ZKEY_SPACE;
        case 0x00F: return ZKEY_TAB;
        case 0x03A: return ZKEY_CAPS_LOCK;
        case 0x145: return ZKEY_NUM_LOCK;
        case 0x046: return ZKEY_SCROLL_LOCK;
        case 0x03B: return ZKEY_F1;
        case 0x03C: return ZKEY_F2;
        case 0x03D: return ZKEY_F3;
        case 0x03E: return ZKEY_F4;
        case 0x03F: return ZKEY_F5;
        case 0x040: return ZKEY_F6;
        case 0x041: return ZKEY_F7;
        case 0x042: return ZKEY_F8;
        case 0x043: return ZKEY_F9;
        case 0x044: return ZKEY_F10;
        case 0x057: return ZKEY_F11;
        case 0x058: return ZKEY_F12;
        case 0x064: return ZKEY_F13;
        case 0x065: return ZKEY_F14;
        case 0x066: return ZKEY_F15;
        case 0x067: return ZKEY_F16;
        case 0x068: return ZKEY_F17;
        case 0x069: return ZKEY_F18;
        case 0x06A: return ZKEY_F19;
        case 0x06B: return ZKEY_F20;
        case 0x06C: return ZKEY_F21;
        case 0x06D: return ZKEY_F22;
        case 0x06E: return ZKEY_F23;
        case 0x076: return ZKEY_F24;
        case 0x038: return ZKEY_LEFT_ALT;
        case 0x01D: return ZKEY_LEFT_CONTROL;
        case 0x02A: return ZKEY_LEFT_SHIFT;
        case 0x15B: return ZKEY_LEFT_SUPER;
        case 0x137: return ZKEY_PRINT_SCREEN;
        case 0x138: return ZKEY_RIGHT_ALT;
        case 0x11D: return ZKEY_RIGHT_CONTROL;
        case 0x036: return ZKEY_RIGHT_SHIFT;
        case 0x15C: return ZKEY_RIGHT_SUPER;
        case 0x150: return ZKEY_DOWN;
        case 0x14B: return ZKEY_LEFT;
        case 0x14D: return ZKEY_RIGHT;
        case 0x148: return ZKEY_UP;
        case 0x052: return ZKEY_KP_0;
        case 0x04F: return ZKEY_KP_1;
        case 0x050: return ZKEY_KP_2;
        case 0x051: return ZKEY_KP_3;
        case 0x04B: return ZKEY_KP_4;
        case 0x04C: return ZKEY_KP_5;
        case 0x04D: return ZKEY_KP_6;
        case 0x047: return ZKEY_KP_7;
        case 0x048: return ZKEY_KP_8;
        case 0x049: return ZKEY_KP_9;
        case 0x04E: return ZKEY_KP_ADD;
        case 0x053: return ZKEY_KP_DECIMAL;
        case 0x135: return ZKEY_KP_DIVIDE;
        case 0x11C: return ZKEY_KP_ENTER;
        case 0x059: return ZKEY_KP_EQUAL;
        case 0x037: return ZKEY_KP_MULTIPLY;
        case 0x04A: return ZKEY_KP_SUBTRACT;
        default: return ZKEY_UNKNOWN;
    }
    return ZKEY_UNKNOWN;
}
