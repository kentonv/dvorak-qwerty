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

// This is a command-line tool for running DQ-Keys.  It is useful primarily
// for debugging.

#include <iostream>
#include <windows.h>

using namespace std;

typedef void (*tSetupHook)(HINSTANCE module);
typedef void (*tReleaseHook)();

tSetupHook SetupHook;
tReleaseHook ReleaseHook;

int main()
{
  HINSTANCE lib;
  lib = LoadLibrary("dqkeys.dll");
  SetupHook = (tSetupHook)GetProcAddress(lib, "SetupHook");
  ReleaseHook = (tReleaseHook)GetProcAddress(lib, "ReleaseHook");

  SetupHook(lib);
  cout << "DQ-Keys enabled" << endl;

  cout << "press enter to disable..." << endl;
  cin.get();

  ReleaseHook();

  return 0;
}
