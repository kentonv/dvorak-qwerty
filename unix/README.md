# xdq

## How to Use

Compile with:

	gcc xdq.c -o xdq -std=gnu99 -O2 -lX11

If you don't have GCC, and the C compiler you do have is not C99-compliant,
try compiling with a C++ compiler instead.

Once compiled, make sure your keyboard layout is set to Dvorak, all locks
are disabled (NumLock, CapsLock, etc) and then run the `xdq` binary.
While running, keys you press while holding control or alt (but not both
together) will be remapped to Qwerty.  To stop, just kill `xdq`.

If you want to remap other key combinations, see the "ARGUMENTS" section
below.

## Background

This file implements the "Dvorak-Qwerty" keyboard layout, in which the layout
is normally Dvorak but switches to Qwerty when control or alt is held.  There
are two reasons why I prefer this layout over straight Dvorak:

- The common copy/paste hotkeys X, C, and V remain on the left hand, and so
  can be used while the right hand is on the mouse.
- Holding the control key with my pinky tends to make it hard for me to
  remember where many keys are located, because my hands are no longer
  positioned as they would be when touch-typing.  Meanwhile, the labels on
  my keyboard are Qwerty, because I no longer bother reconfiguring them
  physically.  With the Dvorak-Qwerty layout, I can look at the keyboard to
  find the key I want.

The layout is available by default on Mac OSX.  Unfortunately, it is not
typically shipped with Linux distributions.  Even more unfortunately,
although it is possible to define an XKB layout which implements
Dvorak-Qwerty, doing so exposes a depressing number of bugs across the board
in X apps.  Since it is the responsibility of each X app to interpret the
keyboard layout itself, rather than having the X server do the work,
different GUI frameworks actually tend to have different bugs that kick in
when using such a layout.  Fixing them all would be infeasible.

This program instead works by passively grabbing (with XGrabKey()) all
relevant combinations, rewriting the event, and then using XSendEvent() to
send it to the focused window.

## Arguments

	./xdq [Modifiers]

`[Modifiers]` expands to a list of modifier combinations. These are the valid
modifiers:

| Name     | Usual binding             |
|----------|---------------------------|
| Shift    | Both Shifts (L and R)     |
| CapsLock | Caps Lock                 |
| Control  | Both Controls (L and R)   |
| Mod1     | Alt (Alt Gr not included) |
| Mod2     | Num Lock                  |
| Mod3     | Scroll Lock               |
| Mod4     | Super ("Windows")         |
| Mod5     | ?                         |

(I computed the second column by trial and error. There is a small chance that
you will get different bindings, so you might need to experiment.)

So if you want to remap only Control, run

	./xdq Control

If you want to remap Control and Alt, but *not* Control+Alt:

	./xdq Control Mod1

If you want to remap Control, Alt and Control+Alt:

	./xdq Control Mod1 Control+Mod1

et cetera.

Notice that locks are considered modifiers, so if you want Control to be
remapped regardless of NumLock, you have to bind both combinations:

	./xdq Control Control+Mod2

For backwards compatibility reasons, by default `xdq` remaps Control,
Control+Shift, Alt and Alt+Shift if you include no `[Modifiers]`.

`xdq` can only remap program-level hotkeys, not system-level hotkeys, as
system-level hotkeys are typically themselves implemented using XGrabKey().
Keep this in mind if you want to remap combinations such as Super and/or
Control+Alt; you might need additional, system-specific configuration to get
them to work.

## If you like it

If you find this useful, consider sending me a note at temporal@gmail.com to
say so.  Otherwise people only contact me when things don't work and that's
depressing.  :)
