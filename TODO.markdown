
# Fix Escape Sequences

A number of escape sequences currently don't work correctly because they're
stubs waiting for the history object

# Color support

* Support control character codes for xterm-256 coloring
* Support theming
* Hardcode an attractive default theme

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

# Input Race Condition

This could be related to input lag, but: it seems it's possible for the shell
to write data to the screen before the input is echoed. Or something? 

If you type "help break" very quickly and type enter quickly, the first line
sometimes appears on the line buffer, which shouldn't happen ...

# Signals

On unix machines, detect special key sequences (Ctrl+C, Ctrl+Z) and send the
appropriate signal to the shell process. 

Figure out what happens on Windows. It's not as simple as killing the process,
because you can Ctrl+C in MinGW bash to cancel a command

# Pseudo-terminals

For example, try ssh localhost -- it'll complain that ssh is running as a child
process without a ptty. Figure out how ptty's work, and how MinGW deals with
them.

# Shell Driver

* Abstract system with at least three implementations:
    * A unix implementation that allocates a ptty and spawns a shell
    * A windows implementation that does hacky stuff with cmd.exe
      See the source for http://console.sf.net
    * An implementation that just spins up a shell binary

# Shell Options

* Create an options dialog
* Create options for picking your shell driver
* Create options for the shell drivers

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

# Shell / Terminal Communication

For things like mouse events. Figure out how this works on UNIX. On Windows,
we'd need to forward mouse events to the hidden cmd.exe window or something.

# Smarter Scroll Behavior

The user should be able to scroll up somewhere and read the text while the
shell is spewing text below

