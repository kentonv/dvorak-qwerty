# Copilot Custom Instructions for Dvorak-Qwerty Project

## Project Overview

This is the **Dvorak-Qwerty** keyboard layout implementation project that provides cross-platform solutions for a hybrid keyboard layout that:

- Uses Dvorak layout by default for normal typing
- Switches to Qwerty layout when modifier keys (Ctrl/Alt/Windows) are held
- Maintains familiar hotkey positions (Ctrl+C, Ctrl+V, etc.) on the left hand for mouse+keyboard workflows
- Only activates when the current system keyboard layout is set to Dvorak

## Project Structure and Components

### Windows Implementation (`windows/` directory)
- **Primary Language**: C++ (Win32 API)
- **Architecture**: DLL hook system using SetWindowsHookEx
- **Main Components**:
  - `dqkeys.dll` - Core keyboard hook implementation
  - `dqkeys_gui.exe` - System tray GUI application
  - `dqkeys_starter.cpp` - Launcher utility
- **Build System**: Visual Studio project files (.vcproj, .sln)
- **Key Files**:
  - `dqkeys.cpp` - Main hook logic and key remapping
  - `dqkeys_gui.cpp` - GUI wrapper with system tray icon
  - `resource.h`, `*.rc` - Windows resource definitions

### Unix/Linux Implementation (`unix/` directory)  
- **Primary Language**: C (X11/SystemTap)
- **Multiple Approaches**:
  - `xdq.c` - X11 passive grab implementation (works on X, not Wayland)
  - `dvorak-qwerty.stp` - SystemTap kernel-level implementation
- **Dependencies**: X11 libraries, SystemTap (for .stp version)
- **Compilation**: Standard GCC with X11 linking

## Technical Context and Constraints

### Core Algorithm
- Intercepts keyboard events at system level
- Detects current keyboard layout and only activates when Dvorak is in use
- Detects modifier key states (Ctrl, Alt, Shift, and Windows key for Windows implementation)
- Performs real-time Dvorak↔Qwerty translation based on modifier state
- Handles edge cases like preventing infinite loops from synthetic events

### Platform-Specific Challenges
- **Windows**: Uses global hooks, may conflict with some applications
- **Linux X11**: Uses passive grabs, limited by X server architecture  
- **Linux Wayland**: Not supported by X11 implementation
- **SystemTap**: Requires kernel debug symbols and root privileges

### Key Mapping Logic
The project implements bidirectional character mapping between Dvorak and Qwerty layouts, focusing on:
- Alphabetic keys (preserving case)
- Common punctuation marks
- Maintaining number row unchanged
- Preserving modifier key combinations

### Windows-Specific Features
- **Windows Key Support**: For the Windows implementation, the Windows key (VK_LWIN/VK_RWIN) should also trigger Qwerty mode
- **Modifier Key Tracking**: Use volatile shared variables in the DLL to track modifier states:
  - `gControlDown`, `gAltDown` for current functionality
  - Add `gWinDown` for Windows key tracking
- **Key Code Constants**: 
  - VK_LWIN (0x5B) - Left Windows key
  - VK_RWIN (0x5C) - Right Windows key
  - VK_CONTROL (0x11) - Control key
  - VK_MENU (0x12) - Alt key

## Development Guidelines

### When Working on This Project:

1. **Respect Platform Conventions**:
   - Windows: Follow Win32 API patterns, use TCHAR for strings
   - Unix: Follow POSIX conventions, use standard C99
   - Handle platform-specific include paths and linking

2. **Windows Key Implementation Guidelines**:
   - Add Windows key detection in `DoKeyboardEvent()` function
   - Track Windows key state with `gWinDown` volatile variable
   - Support both left (VK_LWIN) and right (VK_RWIN) Windows keys
   - Ensure Windows key combinations trigger Qwerty mode like Ctrl/Alt
   - Update modifier detection logic: `(gControlDown || gAltDown || gWinDown)`
   - Handle key release properly to avoid stuck modifier states
   - **Keyboard Layout Detection**: Only activate remapping when Dvorak layout is detected using `GetKeyboardLayout()` API

3. **Memory Management**:
   - Windows: Use appropriate Windows memory functions
   - Unix: Standard malloc/free with proper cleanup
   - Be especially careful with shared memory segments in Windows DLL

4. **Error Handling**:
   - Graceful degradation when system features unavailable
   - Proper cleanup of system resources (hooks, grabs, etc.)
   - User-friendly error messages for common setup issues

5. **Security Considerations**:
   - This is system-level input handling code
   - Minimize attack surface in hook procedures
   - Validate all input parameters
   - Avoid buffer overflows in key event processing

6. **Compatibility**:
   - Test across Windows versions (the code is old, from ~2010 era)
   - Consider both 32-bit and 64-bit architectures
   - Unix code should work across major distributions

### Code Style and Patterns:

- **Naming**: Use descriptive names for keyboard-related constants and functions
- **Comments**: Explain the keyboard layout mapping logic clearly
- **Constants**: Define key codes and modifier masks as named constants
- **Modularity**: Separate platform abstraction from core remapping logic

### Testing Considerations:

- Test with various applications (text editors, IDEs, games, etc.)
- Verify hotkey functionality (Ctrl+C, Ctrl+V, Alt+Tab, Win+R, etc.)
- Test modifier key combinations and edge cases
- Ensure no interference with system-level shortcuts
- Validate proper cleanup when application exits
- **Windows-specific**: Test Windows key combinations (Win+L, Win+D, Win+R)
- **Windows-specific**: Verify Windows key doesn't get stuck in pressed state

### Building and Distribution:

- **Windows**: Maintain Visual Studio compatibility, provide both debug and release configurations
- **Unix**: Ensure clean compilation with standard build tools (make, gcc)
- **Documentation**: Keep installation instructions updated for modern systems

## Historical Context

- Original implementation by Kenton Varda (Google)
- Inspired by Mac OS X built-in "Dvorak-Qwerty ⌘" layout
- Code base is mature but may need updates for modern systems
- Active community with third-party implementations (referenced in README)

## When Suggesting Improvements:

- Consider cross-platform compatibility
- Respect the minimalist design philosophy
- Maintain backward compatibility where possible
- Be mindful of the low-level system integration requirements
- Consider modern security practices for system-level software
