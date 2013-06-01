
# Basic functionality

* Implement [ASCII character codes](http://en.wikipedia.org/wiki/Control_character)
    * [see also](http://en.wikipedia.org/wiki/ASCII#ASCII_control_characters)
* Send non-textual input to the shell (e.g. arrow keys, signals)

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

# Infinite Scrollback

* Figure out how to store the input history in a way that makes scrolls though
  history efficient. For example, we might want to store lines separately so we
  can easily jump between lines

# Line Wrap

* Act like Terminal.app -- store the string without line wraps, but wrap the
  line dynamically when the window is resized. To make things easier in other
  places, it might make sense to store two buffers -- the canonical buffer that
  holds what came from the shell, and the view buffer that has line breaks
  based on the size of the current window. Then we can do draw calculations
  using the view buffer.

# Copy / Paste

* Ability to highlight text with the mouse cursor
* Hotkeys for copying and pasting

# Instance Spinup

* Create another instance of the process on Ctrl + N or something

# Color support

* Support control character codes for xterm-256 coloring
* Support theming
* Hardcode an attractive default theme

# Shell / Terminal Communication

For things like mouse events. Figure out how this works on UNIX. On Windows,
we'd need to forward mouse events to the hidden cmd.exe window or something.

# Smarter Scroll Behavior

The user should be able to scroll up somewhere and read the text while the
shell is spewing text below

