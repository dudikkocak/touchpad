# Touchpad Toggle

A tiny, dependency-free Windows utility that toggles your Precision Touchpad on/off with a hotkey, and hides/shows the mouse cursor in sync.

Built as a single background executable with no window, no tray icon, and no runtime dependencies — the compiled binary is only a few tens of KB.

---

## Features

- **Win + 0** — Toggle the touchpad on/off
  - When disabling: the mouse cursor is hidden
  - When enabling: the mouse cursor is shown
- **Shift + Win + 0** — Restore the cursor (if hidden by this app) and exit gracefully
- **Single instance** — Only one copy can run at a time (enforced with a global named mutex)
- **State-aware** — Reads the actual touchpad state from the registry, so the cursor stays in sync even if the touchpad was toggled by other means
- **Lightweight** — Pure Win32 C++, statically linked CRT, no frameworks, no installer

---

## Requirements

- **Windows 10 or Windows 11**
- A laptop with a **Windows Precision Touchpad**
  - Legacy vendor drivers (older Synaptics / Elan) may not respond to the toggle hotkey
- **No administrator privileges required**

---

## Hotkeys

| Shortcut | Action |
|---|---|
| `Win + 0` | Toggle touchpad; hide cursor when disabling, show when enabling |
| `Shift + Win + 0` | Restore cursor (if hidden) and exit the application |

> Both shortcuts are intercepted by a low-level keyboard hook, so the `0` keypress never reaches other applications.

---

## Build

### Prerequisites

- **Visual Studio Build Tools 2022** (or the full Visual Studio IDE) with the **"Desktop development with C++"** workload installed

### Steps

1. Open a **Developer Command Prompt for VS 2022**.
2. Navigate to the project folder (containing `main.cpp` and `makefile`).
3. Run:

   ```cmd
   nmake
   ```

4. The output is `touchpad.exe`.

To clean the build artifacts:

```cmd
nmake clean
```

### Manual build (without `nmake`)

```cmd
cl /O1 /MT /GS- /DNDEBUG /c main.cpp
link main.obj user32.lib advapi32.lib /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup /OUT:touchpad.exe
```

### Build flags explained

| Flag | Purpose |
|---|---|
| `/O1` | Optimize for size |
| `/MT` | Statically link the C runtime (no `VCRUNTIME140.dll` dependency) |
| `/GS-` | Disable buffer security checks (smaller binary) |
| `/DNDEBUG` | Disable debug assertions |
| `/SUBSYSTEM:WINDOWS` | Window subsystem — no console window |
| `/ENTRY:wWinMainCRTStartup` | Use `wWinMain` as the entry point |

---

## Usage

1. Run `touchpad.exe`. It starts silently in the background (no window, no tray icon).
2. Press **Win + 0** to toggle the touchpad.
3. To quit, press **Shift + Win + 0** — this restores the cursor and exits cleanly.

### Starting automatically on login (optional)

1. Press `Win + R`, type `shell:startup`, and press Enter.
2. Place a shortcut to `touchpad.exe` in the folder that opens.

---

## How It Works

| Component | Mechanism |
|---|---|
| **Hotkey interception** | `SetWindowsHookEx(WH_KEYBOARD_LL, ...)` installs a low-level keyboard hook; the callback blocks `Win+0` and dispatches to the toggle or exit handler |
| **Touchpad toggle** | Sends the Microsoft-documented hotkey `Ctrl + Win + F24` via `SendInput` |
| **State detection** | Reads `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\PrecisionTouchPad\Status\Enabled` (non-zero = enabled) |
| **Cursor sync** | Calls `ShowCursor(FALSE)` when disabling, `ShowCursor(TRUE)` when enabling |
| **Single instance** | `CreateMutexW` with the name `Global\TouchpadToggleApp_SingleInstance`; a second launch detects `ERROR_ALREADY_EXISTS` and exits |
| **Graceful exit** | `PostQuitMessage(0)` breaks the message loop, then the hook is removed and the mutex released |

---

## Known Limitations

- **Precision Touchpad only** — The `Ctrl+Win+F24` toggle is a Microsoft feature; laptops using legacy vendor touchpad drivers may not respond.
- **Cursor counter** — `ShowCursor` uses a global display counter. If the process is force-killed (e.g., Task Manager → End task) while the cursor is hidden, the cursor may remain hidden. Use **Shift + Win + 0** to exit cleanly instead.
- **No keyboard/touchscreen toggling** — Disabling those devices requires administrator privileges and, for the built-in keyboard, has no clean reversible API. Out of scope for this lightweight tool.
- **No `Win` key side effects** — The hook only intercepts `Win+0`; standalone `Win`, `Win+R`, `Win+E`, etc. continue to work normally.

---

## File Overview

| File | Description |
|---|---|
| `main.cpp` | Complete application source |
| `makefile` | `nmake` build script |
| `README.md` | This document |

---

## License

Use, modify, and distribute freely. No warranty — use at your own risk.

Visit [Dudik Store](https://dudik.store) to support my work.

powered by

![deepseek powered](./deepseek.svg)

