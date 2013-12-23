// "Dvorak-Qwerty" keyboard layout for the X-Window system
// Copyright 2010 Google Inc.  All rights reserved.
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

// HOW TO USE
//
// Compile with:
//
//   gcc xdq.c -o xdq -std=gnu99 -O2 -lX11
//
// If you don't have GCC, and the C compiler you do have is not C99-compliant,
// try compiling with a C++ compiler instead.
//
// Once compiled, make sure your keyboard layout is set to Dvorak and then run
// the "xdq" binary.  While running, keys you press while holding control or
// alt (but not both together) will be remapped to Qwerty.  To stop, just kill
// xdq.

// BACKGROUND
//
// This file implements the "Dvorak-Qwerty" keyboard layout, in which the layout
// is normally Dvorak but switches to Qwerty when control or alt is held.  There
// are two reasons why I prefer this layout over straight Dvorak:
// - The common copy/paste hotkeys X, C, and V remain on the left hand, and so
//   can be used while the right hand is on the mouse.
// - Holding the control key with my pinky tends to make it hard for me to
//   remember where many keys are located, because my hands are no longer
//   positioned as they would be when touch-typing.  Meanwhile, the labels on
//   my keyboard are Qwerty, because I no longer bother reconfiguring them
//   physically.  With the Dvorak-Qwerty layout, I can look at the keyboard to
//   find the key I want.
//
// The layout is available by default on Mac OSX.  Unfortunately, it is not
// typically shipped with Linux distributions.  Even more unfortunately,
// although it is possible to define an XKB layout which implements
// Dvorak-Qwerty, doing so exposes a depressing number of bugs across the board
// in X apps.  Since it is the responsibility of each X app to interpret the
// keyboard layout itself, rather than having the X server do the work,
// different GUI frameworks actually tend to have different bugs that kick in
// when using such a layout.  Fixing them all would be infeasible.
//
// This program instead works by passively grabbing (with XGrabKey()) all
// relevant combinations, rewriting the event, and then using XSendEvent() to
// send it to the focused window.
//
// xdq can only remap program-level hotkeys, not system-level hotkeys, as
// system-level hotkeys are typically themselves implemented using XGrabKey().
// To avoid conflicts with system-level hotkeys, xdq only grabs key combinations
// involving holding control *or* alt, not both together.  xdq also does NOT
// try to grab anything involving the "windows" key.  If you would like xdq to
// grab all these keys as well, system hotkeys be damned, then compile with
// -DXDQ_GREEDY.

// IF YOU LIKE IT
//
// If you find this useful, consider sending me a note at temporal@gmail.com to
// say so.  Otherwise people only contact me when things don't work and that's
// depressing.  :)

#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#define arraysize(array) (sizeof(array) / sizeof((array)[0]))
#define bit_is_set(arr, i) ((arr[i/8] & (1 << (i % 8))) != 0)

unsigned int kModifierKeycodes[] = {
  37, 105,       // ctrl (L, R)
  64, 108,       // alt (L, R)
  50, 62,        // shift (L, R)
};

// X keycodes corresponding to keys, regardless of layout.
const int kKeycodes[] = {
                                        20, 21,
  24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
   38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
     52, 53, 54, 55, 56, 57, 58, 59, 60, 61
};

const char kQwerty[] =
            "-="
  "qwertyuiop[]"
  "asdfghjkl;'"
   "zxcvbnm,./";

const char kDvorak[] =
            "[]"
  "',.pyfgcrl/="
  "aoeuidhtns-"
   ";qjkxbmwvz";

// The user has their keyboard layout set to Dvorak.  When we get a keycode, we
// map it to a letter acconding to Qwerty, then figure out which keycode would
// map to the same letter in Dvorak.  This tells us what keycode to send to the
// focus window.  For efficiency, we build a lookup table in keycode_mapping.
//
// keycode --qwerty--> letter --reverse-dvorak--> new keycode
int keycode_mapping[256];

void InitKeycodeMapping() {
  int size = arraysize(kKeycodes);

  int dvorak_to_keycode[128];
  memset(dvorak_to_keycode, 0, sizeof(dvorak_to_keycode));

  for (int i = 0; i < size; i++) {
    dvorak_to_keycode[(int) kDvorak[i]] = kKeycodes[i];
  }

  memset(keycode_mapping, 0, sizeof(keycode_mapping));
  for (int i = 0; i < size; i++) {
    assert(dvorak_to_keycode[(int) kQwerty[i]] != 0);
    keycode_mapping[kKeycodes[i]] = dvorak_to_keycode[(int) kQwerty[i]];
  }
}

// We receive X errors if we grab keys that are already grabbed.  This is not
// really fatal so we catch them.
int failed_grab_count = 0;
int (*original_error_handler)(Display* display, XErrorEvent* error);

int HandleError(Display* display, XErrorEvent* error) {
  if (error->error_code == BadAccess) {
    ++failed_grab_count;
    return 0;
  } else {
    return original_error_handler(display, error);
  }
}

int main(int argc, char* argv[]) {
  InitKeycodeMapping();

  // Open the display and get the root window.
  Display* display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "Couldn't open display.\n");
    return 1;
  }

  Window window = DefaultRootWindow(display);

  // Establish grabs to intercept the events we want.
  if (0) {
    // Method 1:  Grab the actual modifier keys.
    //
    // The keycodes here are for left control, right control, left alt, and
    // command ("windows"), not necessarily in that order (I forget).  Right
    // alt doesn't seem to produce any event for me; maybe my keyboard is
    // broken.
    //
    // In any case, the way XGrabKey() works, while the grabbed key is held
    // down, *all* keyboard events are redirected to the grabber (us).  This
    // may seem perfect, but has some down-sides:
    // * While the grab is in effect, the focused window is apparently notified
    //   that it is no longer receiving keystrokes directly.  Many text editors
    //   respond by hiding the cursor.  This is particularly annoying when using
    //   ctrl+arrows to move the cursor a word at a time, because you cannot
    //   actually see what you're doing.
    // * I'm not sure how this interacts with other grabs in effect on specific
    //   key combinations.  E.g. what happens if the system is configured to
    //   open some particular program when ctrl+alt+shift+z is pressed?  Will
    //   it get the event, or will we?  If we get the event, we'll forward it
    //   to the focused window, which means whatever the system wanted to do
    //   with it won't happen, which would be bad.
    int keycodes[] = {37, 64, 109, 115};
    for (int i = 0; i < arraysize(keycodes); i++) {
      XGrabKey(display, keycodes[i], 0, window, True,
               GrabModeAsync, GrabModeAsync);
    }
  } else {
    // Method 2:  Grab each individual key combination.
    //
    // This solves the cursor-disappearing problem with method 1.  We can also
    // avoid interfering with system hotkeys by only grabbing ctrl and alt
    // individually but not when used together.  Compile with -DXDQ_GREEDY if
    // you really want to grab everything.

    // We will try to grab all of these modifier combinations.
    unsigned int modifiers[] = {
      // Control.
      ControlMask,
      ControlMask | ShiftMask,

      // Alt.
      Mod1Mask,
      Mod1Mask | ShiftMask,

#ifdef XQD_GREEDY
      // Command/"Windows" key.  This is usually used for system-level hotkeys,
      // so only grab it in greedy mode.
      Mod4Mask,
      Mod4Mask | ShiftMask,

      // Control + Alt.  Also typically used for system-level hotkeys.
      ControlMask | Mod1Mask,
      ControlMask | Mod1Mask | ShiftMask,
#endif

      // TODO(kenton):  Other combinations?
    };

    // Often, some keys are already grabbed, e.g. by the desktop environment.
    // Set an error handler so that we can ignore those.
    original_error_handler = XSetErrorHandler(&HandleError);

    for (int i = 0; i < arraysize(kKeycodes); i++) {
      for (int j = 0; j < arraysize(modifiers); j++) {
        XGrabKey(display, kKeycodes[i], modifiers[j], window, True,
                 GrabModeAsync, GrabModeAsync);
      }
    }
  }

  // Make sure all errors have been reported, then print how many errors we saw.
  XSync(display, False);
  if (failed_grab_count != 0) {
    fprintf(stderr, "Failed to grab %d key combinations.\n", failed_grab_count);
    fprintf(stderr,
      "This is probably because some hotkeys are already grabbed by the system.\n"
      "Unfortunately, these system-wide hotkeys cannot be automatically remapped by\n"
      "this tool.  However, you can usually configure them manually.\n");
  }

  // Event loop.
  XEvent down, up;
  int state = 0;
  char keysAtPress[32];
  for (;;) {
    XEvent event;
    XNextEvent(display, &event);

    switch (event.type) {
      case KeyPress:
      case KeyRelease: {
        if (event.xkey.send_event) {
          fprintf(stderr, "SendEvent loop?\n");
          break;
        }

        // Interpret the key event, remap it, and save for later.
        // NOTE(kenton):  I think this conditional is always true but better
        //   safe than sorry.
        if (event.xkey.keycode >= 0 &&
            event.xkey.keycode < arraysize(keycode_mapping)) {
          int new_keycode = keycode_mapping[event.xkey.keycode];
          if (new_keycode != 0) {
            event.xkey.keycode = new_keycode;
          }
          
          // We can't actually send the event yet because during a grab the
          // active window loses keyboard focus.  Many apps are fine with
          // receiving key events when not focused, but some get very confused.
          // The grab (and loss of focus) extends from the KeyPress to the
          // KeyRelease event, so we'll send both remapped events immediately
          // after KeyRelease.  Sadly this makes hotkeys feel laggy, but oh
          // well.
          
          if (event.type == KeyPress) {
            down = event;
            state = 1;
            XQueryKeymap(display, keysAtPress);
          } else if (state == 1 && event.type == KeyRelease &&
                     event.xkey.keycode == down.xkey.keycode) {
            up = event;
            state = 2;
          }
        }

        // If we have received both a KeyPress and a KeyRelease, send the
        // remapped events to the currently-focused window.
        if (state == 2) {
          // Find the focused window.
          int junk;
          XGetInputFocus(display, &down.xkey.window, &junk);
          up.xkey.window = down.xkey.window;
          
          // NX client forgets which modifier keys are down whenever it loses
          // focus, so check which are down and re-send keydown events for them.
          for (unsigned int i = 0; i < arraysize(kModifierKeycodes); i++) {
            unsigned int keycode = kModifierKeycodes[i];
            if (bit_is_set(keysAtPress, keycode)) {
              XEvent modifier = down;
              modifier.xkey.keycode = keycode;
              XSendEvent(display, modifier.xkey.window, True, 0, &modifier);
            }
          }
          
          // Send our remapped KeyPress followed by KeyRelease.
          XSendEvent(display, down.xkey.window, True, 0, &down);
          XSendEvent(display, up.xkey.window, True, 0, &up);
          state = 0;
          
          // If some modifier keys were released between when the keypress
          // happened and now, we should send release events.
          char keys[32];
          XQueryKeymap(display, keys);
          for (unsigned int i = 0; i < arraysize(kModifierKeycodes); i++) {
            unsigned int keycode = kModifierKeycodes[i];
            if (bit_is_set(keysAtPress, keycode) &&
                !bit_is_set(keys, keycode)) {
              XEvent modifier = up;
              modifier.xkey.keycode = keycode;
              XSendEvent(display, modifier.xkey.window, True, 0, &modifier);
            }
          }
        }

        break;
      }
      default:
        fprintf(stderr, "Unknown event: %d\n", event.type);
        break;
    }
  }
}
