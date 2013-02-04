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

// dqkeys_gui.cpp : Defines the entry point for the application.
//
// This file appears to be mostly auto-generated from some sort of MSVC
// template.  It was all so long ago.  I don't care to clean it up.

#include "stdafx.h"
#include "resource.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                // current instance
TCHAR szTitle[MAX_LOADSTRING];                // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];                // The title bar text
NOTIFYICONDATA gIconData;
HMENU gFileMenu;

// Foward declarations of functions included in this code module:
ATOM        MyRegisterClass(HINSTANCE hInstance);
BOOL        InitInstance(HINSTANCE, int);
LRESULT CALLBACK  WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  About(HWND, UINT, WPARAM, LPARAM);

typedef void (*tSetupHook)(HINSTANCE module);
typedef void (*tReleaseHook)();

tSetupHook SetupHook;
tReleaseHook ReleaseHook;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
   // TODO: Place code here.
  MSG msg;
  HACCEL hAccelTable;

  // Initialize global strings
  strcpy(szTitle, "blah");
  strcpy(szWindowClass, "dqkeys_gui");
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance (hInstance, SW_HIDE))
  {
    return FALSE;
  }

  hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_DQKEYS_GUI);

  // Main message loop:
  while (GetMessage(&msg, NULL, 0, 0))
  {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  ReleaseHook();
  Shell_NotifyIcon(NIM_DELETE, &gIconData);

  return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style      = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc  = (WNDPROC)WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance    = hInstance;
  wcex.hIcon      = LoadIcon(hInstance, (LPCTSTR)IDI_DQKEYS_GUI);
  wcex.hCursor    = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName  = (LPCSTR)IDC_DQKEYS_GUI;
  wcex.lpszClassName  = szWindowClass;
  wcex.hIconSm    = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_DQKEYS_GUI);

  return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  HINSTANCE lib;
  HWND hWnd;

  lib = LoadLibrary("dqkeys.dll");
  if(lib == NULL)
  {
    MessageBox(NULL, "Could not load required dll 'dqkeys.dll'", "DQ-Keys Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
    return FALSE;
  }

  SetupHook = (tSetupHook)GetProcAddress(lib, "SetupHook");
  ReleaseHook = (tReleaseHook)GetProcAddress(lib, "ReleaseHook");

  SetupHook(lib);

  hInst = hInstance; // Store instance handle in our global variable

  hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

  if (!hWnd)
  {
    return FALSE;
  }

  gIconData.cbSize = sizeof(NOTIFYICONDATA);
  gIconData.hWnd = hWnd;
  gIconData.uID = 12345;
  gIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  gIconData.uCallbackMessage = WM_USER;
  gIconData.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_DQKEYS_GUI);
  strcpy(gIconData.szTip, "DQ-Keys Dvorak-Qwerty keyboard tool.");

  Shell_NotifyIcon(NIM_ADD, &gIconData);

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  HMENU menu = LoadMenu(hInstance, (LPCTSTR)IDC_DQKEYS_GUI);
  gFileMenu = GetSubMenu(menu, 0);

  return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT  - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId, wmEvent;
  PAINTSTRUCT ps;
  HDC hdc;
  POINT mouse;

  switch (message)
  {
    case WM_COMMAND:
      wmId    = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      // Parse the menu selections:
      switch (wmId)
      {
        case IDM_ABOUT:
           DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
           break;
        case IDM_EXIT:
           DestroyWindow(hWnd);
           break;
        default:
           return DefWindowProc(hWnd, message, wParam, lParam);
      }
      break;
    case WM_PAINT:
      hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_USER:
      if(lParam != WM_LBUTTONUP && lParam != WM_RBUTTONUP)
        break;

      GetCursorPos(&mouse);
      SetForegroundWindow(hWnd);
      TrackPopupMenu(gFileMenu, 0, mouse.x, mouse.y, 0, hWnd, NULL);

      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
      {
        EndDialog(hDlg, LOWORD(wParam));
        return TRUE;
      }
      break;
  }
    return FALSE;
}
