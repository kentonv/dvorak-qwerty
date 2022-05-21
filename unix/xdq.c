// "Dvorak-Qwerty" keyboard layout for the X-Window system
// Copyright 2010 Google Inc.  All rights reserved.
// http://dvorak-qwerty.googlecode.com
// Author:  Kenton Varda (temporal@gmail.com; formerly kenton@google.com)
//          Alberto Leiva (ydahhrk@gmail.com)
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

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
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
  66, 77, 78,    // lock (caps, num, scroll)
  133,           // Super
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

static void InitKeycodeMapping() {
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

#ifndef XDQ_MODIFIER_LIMIT
#define XDQ_MODIFIER_LIMIT 16
#endif

// These are the enabled modifiers, defined by the user as program arguments.
static unsigned int modifiers[XDQ_MODIFIER_LIMIT];
static int modifier_count;

static int WhineOverModifierCount(unsigned int count) {
   fprintf(stderr, "Error: Too many modifiers. (current limit is %u)\n",
       XDQ_MODIFIER_LIMIT);
   fprintf(stderr, "Please increase the modifier limit:\n");
   fprintf(stderr, "gcc xdq.c -o xdq -std=gnu99 -O2 -lX11 -DXDQ_MODIFIER_LIMIT=%u\n",
       count);
   return 1;
}

static int InitializeDefaultModifiers(void) {
  modifier_count = 4;
  if (modifier_count > XDQ_MODIFIER_LIMIT) {
    return WhineOverModifierCount(modifier_count);
  }

  modifiers[0] = ControlMask;
  modifiers[1] = ControlMask | ShiftMask;
  modifiers[2] = Mod1Mask;
  modifiers[3] = Mod1Mask | ShiftMask;

  printf("Using default modifiers: Ctrl, Ctrl+Shift, Mod1 and Mod1+Shift.\n");
  return 0;
}

static int ParseArgs(int argc, char* argv[]) {
  // Waste the first argument, since it's just the program name.
  argv++;
  argc--;

  if (argc == 0) {
    return InitializeDefaultModifiers();
  }
  if (argc > XDQ_MODIFIER_LIMIT) {
    return WhineOverModifierCount(argc);
  }

  for (int i = 0; i < argc; i++) {
    modifiers[i] = 0;
    char* token = strtok(argv[i], "+");
    Bool first_done = False;

    printf("Using modifier '");
    while (token) {
      if (first_done) {
        printf("+");
      }

      if (strcasecmp(token, "Shift") == 0) {
        modifiers[i] |= ShiftMask;
      } else if (strcasecmp(token, "CapsLock") == 0) {
        modifiers[i] |= LockMask;
      } else if (strcasecmp(token, "Control") == 0) {
        modifiers[i] |= ControlMask;
      } else if (strcasecmp(token, "Mod1") == 0) {
        modifiers[i] |= Mod1Mask;
      } else if (strcasecmp(token, "Mod2") == 0) {
        modifiers[i] |= Mod2Mask;
      } else if (strcasecmp(token, "Mod3") == 0) {
        modifiers[i] |= Mod3Mask;
      } else if (strcasecmp(token, "Mod4") == 0) {
        modifiers[i] |= Mod4Mask;
      } else if (strcasecmp(token, "Mod5") == 0) {
        modifiers[i] |= Mod5Mask;
      } else {
        printf("\n");
        fprintf(stderr, "I don't know what '%s' is; time to panic.\n", token);
        return 1;
      }

      printf("%s", token);
      first_done = True;
      token = strtok(NULL, "+");
    }
    printf("'.\n");
  }

  modifier_count = argc;
  return 0;
}

static Bool IsLayoutDvorak(Display* display) {
  // If the alphabetic characters of the "main" row are Dvorak, we will assume
  // the whole layout is Dvorak.
  // This is because other variants such as Svorak and british Dvorak are
  // similar enough to original Dvorak that this program might also be used to
  // map their keys. It'll be rough around the edges, but at least the
  // alphabetic characters will work.
  // (I don't seem to have access to the name of the layout...)
  // I'm hoping the middle row is the most representative, even if I have often
  // been tempted to create a custom layout that swaps "e" and "o" :/
  const char mainCharas[] = "aoeuidhtns";
  const KeySym mainKeycodes[] = {38, 39, 40, 41, 42, 43, 44, 45, 46, 47};
  Bool isDvorak = True;

  // Notice: arraysize(mainKeycodes) is better than arraysize(mainCharas)
  // because of the null chara.
  for (unsigned int c = 0; c < arraysize(mainKeycodes); c++) {
    KeySym sym = XkbKeycodeToKeysym(display, mainKeycodes[c], 0, 0);
    /* printf("   Testing keycode %lu (index %u). Dvorak:%u (%c) Current:%lu\n",
        mainKeycodes[c], c, mainCharas[c], mainCharas[c], sym); */
    if (sym != mainCharas[c]) {
      isDvorak = False;
      break;
    }
  }

  return isDvorak;
}

static void WaitUntilLayoutIsDvorak(Display* display) {
  printf("Layout is not Dvorak. Waiting...\n");
  Bool done = False;

  do {
    XEvent event;
    XNextEvent(display, &event);

    if (event.type == MappingNotify) {
      XMappingEvent *e = (XMappingEvent *) &event;
      if (e->request == MappingKeyboard && IsLayoutDvorak(display)) {
        done = True;
      }
      XRefreshKeyboardMapping(e);
    } else {
      fprintf(stderr, "Unknown event: %d\n", event.type);
    }
  } while (!done);
}

// We receive X errors if we grab keys that are already grabbed.  This is not
// really fatal so we catch them.
static int failed_grab_count;
static int (*original_error_handler)(Display* display, XErrorEvent* error);

static int HandleError(Display* display, XErrorEvent* error) {
  if (error->error_code == BadAccess) {
    ++failed_grab_count;
    return 0;
  } else {
    return original_error_handler(display, error);
  }
}

#define GRAB_METHOD 2

static void GrabKeys(Display* display, Window window) {
  printf("Layout is Dvorak; grabbing keyboard.\n");
  failed_grab_count = 0;

  // Establish grabs to intercept the events we want.
  if (GRAB_METHOD == 1) {
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
    //
    // Note(ydahhrk): I commented this out because it doesn't work anymore after
    //   the modifier refactor.

    // int keycodes[] = {37, 64, 109, 115};
    // for (int i = 0; i < arraysize(keycodes); i++) {
    //   XGrabKey(display, keycodes[i], 0, window, True,
    //            GrabModeAsync, GrabModeAsync);
    // }
  } else if (GRAB_METHOD == 2) {
    // Method 2:  Grab each individual key combination.
    //
    // This solves the cursor-disappearing problem with method 1.  We can also
    // avoid interfering with system hotkeys by letting the user decide which
    // modifier combinations are grabbed and which are not.

    // We will try to grab all of these modifier combinations.
    for (int i = 0; i < modifier_count; i++) {
      for (int j = 0; j < arraysize(kKeycodes); j++) {
        XGrabKey(display, kKeycodes[j], modifiers[i], window, True,
            GrabModeAsync, GrabModeAsync);
      }
    }
  } else {
    fprintf(stderr, "Please fix GRAB_METHOD...\n");
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
}

static void UngrabKeys(Display* display, Window window) {
  printf("Ungrabbing keyboard.\n");

  if (GRAB_METHOD == 1) {
    // int keycodes[] = {37, 64, 109, 115};
    // for (int i = 0; i < arraysize(keycodes); i++) {
    //   XUngrabKey(display, keycodes[i], 0, window);
    // }
  } else if (GRAB_METHOD == 2) {
    for (int i = 0; i < modifier_count; i++) {
      for (int j = 0; j < arraysize(kKeycodes); j++) {
        XUngrabKey(display, kKeycodes[j], modifiers[i], window);
      }
    }
  } else {
    fprintf(stderr, "Please fix GRAB_METHOD...\n");
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
  // We might never get a MappingNotify event if the
  // modifier and keymap information was never cached in Xlib.
  // The next line makes sure that this happens initially.
  // http://stackoverflow.com/questions/35569562 :>
  XKeysymToKeycode(display, XK_F1);

  if (ParseArgs(argc, argv)) {
    return 1;
  }

  if (!IsLayoutDvorak(display)) {
    WaitUntilLayoutIsDvorak(display);
  }
  // Often, some keys are already grabbed, e.g. by the desktop environment.
  // Set an error handler so that we can ignore those.
  original_error_handler = XSetErrorHandler(&HandleError);
  GrabKeys(display, window);

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
      case MappingNotify: {
        // This is what happens when the user switches keyboard layout.
        XMappingEvent *e = (XMappingEvent *) &event;
        if (e->request == MappingKeyboard && !IsLayoutDvorak(display)) {
          UngrabKeys(display, window);
          WaitUntilLayoutIsDvorak(display);
          GrabKeys(display, window);
        }
        XRefreshKeyboardMapping(e);
        break;
      }
      default:
        fprintf(stderr, "Unknown event: %d\n", event.type);
        break;
    }
  }
}
