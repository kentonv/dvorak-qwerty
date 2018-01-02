# Dvorak with Qwerty hotkeys

On Mac OSX, there is a keyboard layout known as "Dvorak-Qwerty ⌘".  This layout normally acts like Dvorak, but when the command key is held, it becomes Qwerty.  I find this layout has two significant advantages over straight Dvorak:
  * The common copy/paste hotkeys X, C, and V remain on the left hand, and so can be used while the right hand is on the mouse.
  * Holding the control key with my pinky tends to make it hard for me to remember where many keys are located, because my hands are no longer positioned as they would be when touch-typing.  Meanwhile, the labels on my keyboard are Qwerty, because I no longer bother reconfiguring them physically.  With the Dvorak-Qwerty layout, I can look at the keyboard to find the key I want.

Unfortunately, the layout is not shipped with Windows or most Unix variants.  Worse, while it's technically possible to implement DQ as a standard XKB layout on Unix systems, the layout appears to expose bugs in many Unix apps making it unusable. Instead, we're forced to write code that MITMs and rewrites keystrokes.

This project provides several different implementations of the Dvorak-Qwerty layout on various operating systems.

### Unix

By me:

* [xdq.c](unix/xdq.c) - Implements DQ on X (Unix/Linux) using X Grabs. See comments at top of file for instructions. **WARNING:** This doesn't work on Wayland, which many distros are moving to as of late 2017. It's also pretty hacky due to the nature of X grabs.
* [dvorak-qwerty.stp](unix/dvorak-qwerty.stp) - Implements DQ on Linux by [monkey-patching the kernel using SystemTap](https://blog.cloudflare.com/how-to-monkey-patch-the-linux-kernel/). This is totally nuts but it works and ultimately is way cleaner than xdq.c.

By others (*I have not tested these*):

* [Thomas Bocek's implementation](https://github.com/tbocek/dvorak) using Linux's `/dev/uinput` and `EVIOCGRAB`. This looks similar to my SystemTap-based implementation, without injecting code into kernelspace.

### Windows

By me:

* [dq-windows.zip](https://github.com/kentonv/dvorak-qwerty/releases/download/ancient/dq-windows.zip) - Implements DQ on Windows.  Source code and [compiled binaries](https://github.com/kentonv/dvorak-qwerty/releases) included. **WARNING:** This code was originally written literally half my lifetime ago and I have not used it in nearly a decade since I don't use Windows anymore. I hear it still works, though.

By others (*I have not tested these*):

* [Jeffrey Min's implementation](https://github.com/chid/dvorak-querty/tree/master/dverty). I haven't tested this.
