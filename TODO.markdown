
# History Object

Thought about this in the shower yesterday and wrote a note on my phone :)

The basic idea is to have four pieces of data:

* A vector of QStrings called `lines`. Each item in `lines` is a line of text
  (without the trailing newline!) received from the shell, unmodified.

* A vector of structs called `vlines`. Each vline corresponds to an actual line
  of text as it will be drawn to the screen (i.e. with word wrap precomputed).
  Each item in the list is just a substring within one of the items in `lines`.

* A vector of structs called `colors`. Each item in this list contains an index
  into some `lines` item and the color palette index for that character. This
  list is sparse (i.e. items only exist when the color is changed).

* A cursor index `cursor` within `lines` somewhere.

When the window is resized, we compute the number of columns the window can
show. If the number of columns has changed, we throw away `vlines` and
recompute it from scratch. This should be a relatively quick operation
(basically just creating structs based on each lines precomputed `.length()`
property). However, if it's too slow for reasonably large histories, we may 
have to get clever about lazily re-evaluating sections of `vlines`.

To draw the contents of the window, compute the pixel coordinates of the top
and bottom of the viewport. Divide those by the font's line spacing to obtain
the row indices. Loop over those row indices in `vlines` and render that text.
Simple! :)

For color support, we will additionally need to binary search through `colors`
to find the color section for the first character we want to draw. Afterwards
we just need to look at subsequent items in the colors list.

When we insert a character, we change the `lines` item containing the cursor
and recompute the items in `vlines` that come after that `lines` item. Usually
this is constant time since usually the cursor is at the last line.

# Fix Escape Sequences

A number of escape sequences currently don't work correctly because they're
stubs waiting for the history object

# Line Wrap

Act like Terminal.app -- store the string without line wraps, but wrap the
line dynamically when the window is resized. 

# Color support

* Support control character codes for xterm-256 coloring
* Support theming
* Hardcode an attractive default theme

# Input Lag

It takes about half a second for the terminal to print a character that was
just typed. What's going on? Probably lags due to Qt's event loops.

* There could be latency between the process writing text and the signal making
  it through the event queue
* There could be latency between receiving text and the update() event making
  it through the event queue

See if there's some way we can poll the QProcess manually on a watchdog thread

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

