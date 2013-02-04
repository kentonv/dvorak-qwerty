// "Dvorak-Qwerty" keyboard layout for Windows
// Copyright 2010 Kenton Varda and Google Inc.  All rights reserved.
// http://dvorak-qwerty.googlecode.com
// Author:  Kenton Varda (temporal@gmail.com; formerly kenton@google.com)
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#define _WIN32_WINNT 0x0500

#include <windows.h>

// Shared DATA
#pragma data_seg(".SHARDATA")
static volatile HANDLE gStdOutHandle = NULL;
static volatile HHOOK  gHook = NULL;
static volatile HHOOK  gLlHook = NULL;

static volatile bool gControlDown = false;
static volatile bool gAltDown = false;

static volatile bool gExpectedKeys[256];
static volatile bool gFakeDownKeys[256];
#pragma data_seg()

extern "C"
{
  __declspec(dllexport) void SetupHook(HINSTANCE module);
  __declspec(dllexport) void ReleaseHook();
  __declspec(dllexport) bool ToggleHook(HINSTANCE module);

  __declspec(dllexport) LRESULT CALLBACK KeyboardProc(int code, WPARAM wparam, LPARAM lparam);
  __declspec(dllexport) LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wparam, LPARAM lparam);
}

const unsigned char kDvorakToQwerty[256] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x41,0x4e,0x49,0x48,0x44,0x59,0x55,0x4a,0x47,0x43,0x56,0x50,0x4d,0x4c,0x53,
  0x52,0x58,0x4f,0xba,0x4b,0x46,0xbe,0xbc,0x42,0x54,0xbf,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5a,0xdd,0x57,0xde,0x45,0xdb,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xbd,0x00,0xbb,0x51,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

const unsigned char kDvorakToScancode[256] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x1e,0x31,0x17,0x23,0x20,0x15,0x16,0x24,0x22,0x2e,0x2f,0x19,0x32,0x26,0x1f,
  0x13,0x2d,0x18,0x27,0x25,0x21,0x34,0x33,0x30,0x14,0x35,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2c,0x1b,0x11,0x28,0x12,0x1a,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x00,0x0d,0x10,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

#if 0
// Above tables generated by:
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void MakeTable(unsigned char table[],
               const unsigned char from[],
               const unsigned char to[]) {
  memset(table, 0, 256);
  for (int i = 0; from[i] != '\0'; i++) {
    table[from[i]] = to[i];
  }
}

void PrintTable(const char* name, const unsigned char* table) {
  printf("const unsigned char %s[256] = {", name);
  for (int i = 0; i < 256; i++) {
    if (i % 16 == 0) printf("\n\t");
    printf("0x%02x,", table[i]);
  }
  printf("\n};\n\n");
}

int main() {
  const unsigned char dvorak[] = "\333\335\336\274\276PYFGCRL\277\273AOEUIDHTNS\275\272QJKXBMWVZ";
  const unsigned char qwerty[] = "\275\273QWERTYUIOP\333\335ASDFGHJKL\272\336ZXCVBNM\274\276\277";
  const unsigned char scancodes[] = "\014\015\020\021\022\023\024\025\026\027\030\031\032\033\036\037\040\041\042\043\044\045\046\047\050\054\055\056\057\060\061\062\063\064\065";
  unsigned char dvorak2qwerty[256];
  unsigned char dvorak2scancode[256];

  MakeTable(dvorak2qwerty, dvorak, qwerty);
  MakeTable(dvorak2scancode, dvorak, scancodes);

  PrintTable("kDvorakToQwerty", dvorak2qwerty);
  PrintTable("kDvorakToScancode", dvorak2scancode);

  return 0;
}
#endif

void SetupHook(HINSTANCE module)
{
  gStdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

  gHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, module, 0);
//  gLlHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, module, 0);
}

void ReleaseHook()
{
  gStdOutHandle = NULL;
  if(gHook != NULL) {
    UnhookWindowsHookEx(gHook);
    gHook = NULL;
  }
  if(gLlHook != NULL) {
    UnhookWindowsHookEx(gLlHook);
    gLlHook = NULL;
  }
}

bool ToggleHook(HINSTANCE module)
{
  if(gHook == NULL)
  {
    SetupHook(module);
    return true;
  }
  else
  {
    ReleaseHook();
    return false;
  }
}

void ClickButton(int button)
{
  INPUT inputs[2];

  inputs[0].type = INPUT_MOUSE;
  inputs[0].mi.dx = 0;
  inputs[0].mi.dy = 0;
  inputs[0].mi.mouseData = 0;
  if(button == 1)
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  else if(button == 2)
    inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
  else if(button == 3)
    inputs[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
  inputs[0].mi.time = 0;
  inputs[0].mi.dwExtraInfo = 0;

  inputs[1].type = INPUT_MOUSE;
  inputs[1].mi.dx = 0;
  inputs[1].mi.dy = 0;
  inputs[1].mi.mouseData = 0;
  if(button == 1)
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
  else if(button == 2)
    inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
  else if(button == 3)
    inputs[1].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
  inputs[1].mi.time = 0;
  inputs[1].mi.dwExtraInfo = 0;

  SendInput(2, inputs, sizeof(INPUT));
}

void PressHotkey(char key)
{
  INPUT inputs[4];

  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = VK_CONTROL;
  inputs[0].ki.wScan = 0;
  inputs[0].ki.dwFlags = 0;
  inputs[0].ki.time = 0;
  inputs[0].ki.dwExtraInfo = 0;

  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = key;
  inputs[1].ki.wScan = 0;
  inputs[1].ki.dwFlags = 0;
  inputs[1].ki.time = 0;
  inputs[1].ki.dwExtraInfo = 0;

  inputs[2].type = INPUT_KEYBOARD;
  inputs[2].ki.wVk = key;
  inputs[2].ki.wScan = 0;
  inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
  inputs[2].ki.time = 0;
  inputs[2].ki.dwExtraInfo = 0;

  inputs[3].type = INPUT_KEYBOARD;
  inputs[3].ki.wVk = VK_CONTROL;
  inputs[3].ki.wScan = 0;
  inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
  inputs[3].ki.time = 0;
  inputs[3].ki.dwExtraInfo = 0;

  SendInput(4, inputs, sizeof(INPUT));
}

void KeyDown(unsigned char key) {
  INPUT inputs[1];

  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = key;
  inputs[0].ki.wScan = kDvorakToScancode[key];
  inputs[0].ki.dwFlags = 0;
  inputs[0].ki.time = 0;
  inputs[0].ki.dwExtraInfo = 0;

  SendInput(1, inputs, sizeof(INPUT));
  gFakeDownKeys[key] = true;
}

void KeyUp(unsigned char key) {
  INPUT inputs[1];

  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = key;
  inputs[0].ki.wScan = kDvorakToScancode[key];
  inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
  inputs[0].ki.time = 0;
  inputs[0].ki.dwExtraInfo = 0;

  SendInput(1, inputs, sizeof(INPUT));
  gFakeDownKeys[key] = false;
}

void PressKey(unsigned char key) {
  INPUT inputs[2];

  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = key;
  inputs[0].ki.wScan = 0;
  inputs[0].ki.dwFlags = 0;
  inputs[0].ki.time = 0;
  inputs[0].ki.dwExtraInfo = 0;

  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = key;
  inputs[1].ki.wScan = 0;
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
  inputs[1].ki.time = 0;
  inputs[1].ki.dwExtraInfo = 0;

  SendInput(2, inputs, sizeof(INPUT));
}

void UpFakeKeys() {
  for (int i = 0; i < 256; i++) {
    if (gFakeDownKeys[i]) {
      KeyUp(i);
    }
  }
}

static const int kReleaseBit = 1 << 31;

bool DoKeyboardEvent(int code, WPARAM wparam, LPARAM lparam)
{
  bool down = (lparam & kReleaseBit) == 0;

  switch(wparam)
  {
  case VK_CONTROL:
    gControlDown = down;
    if (!gAltDown) UpFakeKeys();
    break;

  case VK_MENU:
    gAltDown = down;
    if (!gControlDown) UpFakeKeys();
    break;

  default:
    if (wparam <= 0xFF) {
      if (gExpectedKeys[wparam]) {
        if (code != HC_NOREMOVE) {
          gExpectedKeys[wparam] = false;
        }
      } else if ((gControlDown || gAltDown) &&
                 kDvorakToQwerty[wparam] != 0 &&
             kDvorakToQwerty[wparam] != wparam) {
        unsigned char mappedKey = kDvorakToQwerty[wparam];
        gExpectedKeys[mappedKey] = true;
        if (down) {
          KeyDown(mappedKey);
        } else {
          KeyUp(mappedKey);
        }
        return true;
      }
    }
    break;
  }

  return false;
}

LRESULT CALLBACK KeyboardProc(int code, WPARAM wparam, LPARAM lparam) {
  if (code >= 0) {
    if (DoKeyboardEvent(code, wparam, lparam)) return 1;
  }

  return CallNextHookEx(gHook, code, wparam, lparam);
}

LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wparam, LPARAM lparam) {
  if (code >= 0) {
    // Translation code borrowed from Synergy -- synergy2.sourceforge.net.

    // decode the message
    KBDLLHOOKSTRUCT* info = reinterpret_cast<KBDLLHOOKSTRUCT*>(lparam);
    WPARAM wParam = info->vkCode;
    LPARAM lParam = 1;              // repeat code
    lParam      |= (info->scanCode << 16);    // scan code
    if (info->flags & LLKHF_EXTENDED) {
      lParam  |= (1lu << 24);          // extended key
    }
    if (info->flags & LLKHF_ALTDOWN) {
      lParam  |= (1lu << 29);          // context code
    }
    if (info->flags & LLKHF_UP) {
      lParam  |= (1lu << 31);          // transition
    }
    // FIXME -- bit 30 should be set if key was already down but
    // we don't know that info.  as a result we'll never generate
    // key repeat events.

    if (DoKeyboardEvent(code, wParam, lParam)) return 1;
  }

  return CallNextHookEx(gHook, code, wparam, lparam);
}
