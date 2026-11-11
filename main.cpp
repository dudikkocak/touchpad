#include <windows.h>

HHOOK g_hHook = NULL;
bool g_winPressed = false;
bool g_shiftPressed = false;
bool g_cursorHidden = false;   // tracks whether *we* hid the cursor
HANDLE g_hMutex = NULL;

// Send a single key event via SendInput
void SendKey(WORD vk, bool down) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

// Query the Precision Touchpad enabled state from the registry.
// Non-zero = enabled, zero = disabled. Defaults to enabled (1) on failure.
bool IsTouchpadEnabled() {
    DWORD value = 1;
    DWORD size = sizeof(value);
    RegGetValueW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PrecisionTouchPad\\Status",
        L"Enabled",
        RRF_RT_REG_DWORD,
        NULL,
        &value,
        &size
    );
    return value != 0;
}

// Toggle touchpad state and synchronise cursor visibility.
void ToggleTouchpad() {
    bool currentlyEnabled = IsTouchpadEnabled();

    if (currentlyEnabled) {
        // Touchpad is about to be disabled -> hide cursor
        ShowCursor(FALSE);
        g_cursorHidden = true;
    } else {
        // Touchpad is about to be enabled -> show cursor
        ShowCursor(TRUE);
        g_cursorHidden = false;
    }

    // Send the Microsoft-documented hotkey to toggle the Precision Touchpad
    SendKey(VK_CONTROL, true);
    SendKey(VK_LWIN, true);
    SendKey(VK_F24, true);
    SendKey(VK_F24, false);
    SendKey(VK_LWIN, false);
    SendKey(VK_CONTROL, false);
}

// Restore cursor if we hid it, then signal the message loop to exit.
void ShowCursorAndExit() {
    if (g_cursorHidden) {
        ShowCursor(TRUE);
        g_cursorHidden = false;
    }
    PostQuitMessage(0);   // GetMessage returns 0 -> cleanup runs -> process exits
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        // Track Win key state
        if (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN) {
            if (wParam == WM_KEYDOWN) g_winPressed = true;
            else if (wParam == WM_KEYUP) g_winPressed = false;
        }

        // Track Shift key state
        if (p->vkCode == VK_LSHIFT || p->vkCode == VK_RSHIFT) {
            if (wParam == WM_KEYDOWN) g_shiftPressed = true;
            else if (wParam == WM_KEYUP) g_shiftPressed = false;
        }

        // Intercept Win + 0 (key down only)
        if (wParam == WM_KEYDOWN && g_winPressed && p->vkCode == '0') {
            if (g_shiftPressed) {
                ShowCursorAndExit();   // Shift + Win + 0 -> show cursor + exit
            } else {
                ToggleTouchpad();      // Win + 0       -> toggle touchpad
            }
            return 1; // Block the key from reaching the system
        }
    }
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow) {
    // --- Single-instance guard ---
    g_hMutex = CreateMutexW(NULL, TRUE, L"Global\\TouchpadToggleApp_SingleInstance");
    if (g_hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (g_hMutex != NULL) CloseHandle(g_hMutex);
        MessageBoxW(NULL, L"Application is already running.", L"Notice",
                    MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    // --- Install global keyboard hook ---
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,
                               GetModuleHandle(NULL), 0);
    if (!g_hHook) {
        MessageBoxW(NULL, L"Failed to install keyboard hook", L"Error", MB_OK);
        ReleaseMutex(g_hMutex);
        CloseHandle(g_hMutex);
        return 1;
    }

    // --- Message loop ---
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // --- Cleanup (reached on graceful exit, i.e. Shift+Win+0) ---
    UnhookWindowsHookEx(g_hHook);
    ReleaseMutex(g_hMutex);
    CloseHandle(g_hMutex);
    return 0;
}