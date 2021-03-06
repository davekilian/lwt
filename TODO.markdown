
# Shell Driver

* Implement a Windows-specific driver using the Console API
* Activate that driver by default on Windows
* Add the ability to respond to a DSR to the shell driver
* Implement DSR for the existing shells
* Hook up the DSR call when initializing the shell driver
* Add the ability for the driver to send signals (at least SIGINT)

## Notes

Abstract system with three implementations:

1. A generic process-based implementation that doesn't do any kind of
   pseudo-terminal allocation. Cross-platform but limited. This is currently
   what the Shell object does.

2. A Windows-only implementation that uses the Console API to spin up the shell
   in a hidden console window, and do I/O with that. Implementing Ctrl+C might
   be tricky.

3. A unix-only implementation that allocates a pty and spawns the shell binary
   See [kpty](http://api.kde.org/4.x-api/kdelibs-apidocs/kpty/html/)

## Windows Console Driver

I've been reading a bit about windows consoles. It seems like we may be able to
do the following for the Windows console driver:

* Call `AllocConsole()` to create and associate a console with the lwt process.
  This will show an actual console window, but there may be some way to hide
  the window (either showing it way off screen or putting it in a "background
  desktop" like winpty)

* Disable all text processing on the console's output, so the shell driver (in
  lwt) can process the console's output in raw mode. That is, on the output,
  disable `ENABLE_PROCESSED_OUTPUT` and maybe `ENABLE_WRAP_AT_EOL_OUTPUT`.

* Spin up the shell binary. Since we spun it up from within lwt.exe, it'll
  inherit lwt.exe's console. Which will incidentally cause isatty() to return
  true

* Service I/O using native Windows I/O calls and the streams returned by 
  `GetStdHandle()`. Maybe we can `WaitForMultipleObjects` or something so that
  we can get events instead of regularly polling. Are Qt signals
  thread-friendly?

It's not really clear how to read from the output. It seems I can either poll
the screen buffer (which isn't ideal, because the screen buffer has a fixed
width and enforces wrapping) or call `ReadFile` / `ReadConsole` on the stdout
stream. Ideally I can do the latter to get raw character output with no
processing. 

Also, it's not clear how to send a SIGINT to a process on Windows. Apparently 
[this person](http://www.latenighthacking.com/projects/2003/sendSignal/)
figured it out already though. There's also `GenerateControlCtrlEvent()`, which
might work for us since we spawned the console the shell's running in.

Reference:

* [MSDN Console Docs](http://msdn.microsoft.com/en-us/library/windows/desktop/ms682010(v=vs.85\).aspx)
* [winpty](https://github.com/rprichard/winpty)
* [Someone else's notes on Windows consoles](https://code.soundsoftware.ac.uk/projects/easyhg/wiki/TalkingToSubprocess)

# Input Lag

It takes about half a second for the terminal to print a character that was
just typed. What's going on? Probably lags due to Qt's event loops.

* There could be latency between pressing a key and getting the event for it
  (before the key is sent to the shell)
* There could be latency between the process writing text and the signal making
  it through the event queue
* There could be latency between receiving text and the update() event making
  it through the event queue

See if there's some way we can poll the QProcess manually on a watchdog thread

Update: Removed the third item by calling repaint instead of update. Already a
small improvement :)

Note that this might not be a problem once we have the windows-specific shell
driver.

# Crash Bug

Not sure why, but `git lg -p` on the repo currently crashes the app. This might
be triggered by the input string being enormous, but I don't think we do
anything that could cause the app to crash if the input string were enormous.

# Shell Options

* Create an options dialog
* Create options for picking your shell driver
* Create options for the shell drivers

# Theme Options

* Load/save themes to disk
* Create options for changing the theme options

# Option to handle Ctrl+L / Ctrl+D natively

When enabled, lwt eats Ctrl+L and Ctrl+D events and does the following:

* For Ctrl+L events, the console computes what's visible on the current line,
  clears the screen, redraws the current line, and moves the cursor to where it
  should be. The shell never sees any input. This gives you bash-like Ctrl+L
  when you're using cmd.exe

* For Ctrl+D events, the console recalls whether or not input has been sent
  since the last newline. If yes, this just sends a newline to complete the
  command, regardless of cursor position. If no, this kills the shell and
  closes the terminal.

# Copy / Paste

* Ability to highlight text with the mouse cursor
* Hotkeys for copying and pasting

# Instance Spinup

* Create another instance of the process on Ctrl + N or something

# Smarter Scroll Behavior

The user should be able to scroll up somewhere and read the text while the
shell is spewing text below

# Word Wrap and Scrolling

Right now, when you change the size of the window, word-wrapping can cause you
to see something different from what you were seeing before. For example, if
you drag to make the window more narrow, you see older lines; if you make the
window wider, you see newer lines.

To fix this, we'd need to derive a formula for what the new scroll value should
be. We need information from the word-wrapper too, since which lines are
getting wrapped is important. 

