"Dvorak-Qwerty" keyboard layout for Windows
Copyright 2010 Kenton Varda and Google Inc.  All rights reserved.
http://dvorak-qwerty.googlecode.com
Author:  Kenton Varda (temporal@gmail.com; formerly kenton@google.com)

INSTALLATION

Just put dqkeys_gui.exe and dqkeys.dll wherever you want.  They must be in
the same directory.

USE

1) Set your system's keyboard layout to Dvorak.

2) Run dqkeys_gui.  An icon will appear in your task bar.  While this icon
   is present, any key you press while holding ctrl or alt will be remapped
   to Qwerty.  To stop, click the icon and choose "exit".

BACKGROUND

This progam implements the "Dvorak-Qwerty" keyboard layout, in which the layout
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
found on Windows.  I am not sure how Windows keyboard layouts work.  Maybe
it would be possible to create a DQ layout -- if so, let me know.  Not wanting
to figure it out, I instead opted to adapt some old code I had that intercepts
and rewrites input events.

This program works by registering a "hook" using SetWindowsHookEx.  This
causes every keyboard event to be run through a filter function implemented
by dqkeys.dll.  The filter function filters out keys pressed while holding
control or alt and generates new, fake events as if the key were typed on
a Qwerty layout.  Unfortunately the new, fake events end up going back through
the filter function, so it must intelligently recognize and not filter them to
avoid an infinite loop.

The remapping does not work with all software.  Some software actually uses
hooks of its own to read input, and these hooks may be executed before the
DQ-Keys hook, thus preventing DQ-Keys from doing its remapping.  Not very
many programs do this, but I note that Synergy (used to control multiple
machines with one keyboard/mouse) is an annoying example.  As a result, when
using Synergy, you will have to find some other solution to implement
Dvorak-Qwerty on the "client" machines.  Luckily you can use my own XDQ on
Unix and the built-in DQ layout on Mac.

IF YOU LIKE IT

If you find this useful, consider sending me a note at temporal@gmail.com to
say so.  Otherwise people only contact me when things don't work and that's
depressing.  :)
