
# Basic functionality

* Render a cursor
* Scroll to bottom on input
* Implement [ASCII character codes](http://en.wikipedia.org/wiki/Control_character)

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

# Line Wrap

* Act like Terminal.app -- store the string without line wraps, but wrap the
  line dynamically when the window is resized.

# Copy / Paste

* Ability to highlight text with the mouse cursor
* Hotkeys for copying and pasting

# Instance Spinup

* Create another instance of the process on Ctrl + N or something

# Color support

* Support control character codes for xterm-256 coloring
* Support theming
* Hardcode an attractive default theme

