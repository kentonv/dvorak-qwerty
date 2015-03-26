On Mac OSX, there is a keyboard layout known as "Dvorak-Qwerty ⌘".  This layout normally acts like Dvorak, but when the command key is held, it becomes Qwerty.  I find this layout has two significant advantages over straight Dvorak:
  * The common copy/paste hotkeys X, C, and V remain on the left hand, and so can be used while the right hand is on the mouse.
  * Holding the control key with my pinky tends to make it hard for me to remember where many keys are located, because my hands are no longer positioned as they would be when touch-typing.  Meanwhile, the labels on my keyboard are Qwerty, because I no longer bother reconfiguring them physically.  With the Dvorak-Qwerty layout, I can look at the keyboard to find the key I want.

Unfortunately, the layout is not shipped with Windows or most Unix variants.  Worse, while it's technically possible to implement DQ as a standard XKB layout on Unix systems, the layout appears to expose bugs in many Unix apps making it unusable.  (~~I have no idea if DQ can be implemented as a regular keyboard layout on Windows; I didn't try.~~  DQ apparently can be implemented as a Windows keyboard layout; see https://github.com/chid/dvorak-querty/tree/master/dverty)

This project provides (separate) programs for Windows and X which implement the Dvorak-Qwerty layout by intercepting keyboard events and re-mapping them.

**xdq.c** - Implements DQ on X (Unix/Linux).  See comments at top of file for instructions.

**dq-windows.zip** - Implements DQ on Windows.  Source code and compiled binaries included.