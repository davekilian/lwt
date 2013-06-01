
#include "specialchars.h"


// The ASCII Control Characters
// http://en.wikipedia.org/wiki/ASCII#ASCII_control_characters

#define ASCII_NUL       0       // Null
#define ASCII_SOH       1       // Start of Header
#define ASCII_STX       2       // Start of Text
#define ASCII_ETX       3       // End of Text
#define ASCII_EOT       4       // End of Transmission
#define ASCII_ENQ       5       // Enquiry
#define ASCII_ACK       6       // Acknowledgement
#define ASCII_BEL       7       // Bell
#define ASCII_BS        8       // Backspace
#define ASCII_HT        9       // Horizontal Tab
#define ASCII_LF        10      // Line Feed
#define ASCII_VT        11      // Vertical Tab
#define ASCII_FF        12      // Form Feed
#define ASCII_CR        13      // Carriage Return
#define ASCII_SO        14      // Shift Out
#define ASCII_SI        15      // Shift In
#define ASCII_DLE       16      // Data Link Escape
#define ASCII_DC1       17      // Device Control 1 (XON - Transmission On)
#define ASCII_DC2       18      // Device Control 2
#define ASCII_DC3       19      // Device Control 3 (XOFF - Transmission Off)
#define ASCII_DC4       20      // Device Control 4
#define ASCII_NAK       21      // Negative Acknowledgement
#define ASCII_SYN       22      // Synchronous Idle
#define ASCII_ETB       23      // End of Transmission Block
#define ASCII_CAN       24      // Cancel
#define ASCII_EM        25      // End of Medium
#define ASCII_SUB       26      // Substitute
#define ASCII_ESC       27      // Escape
#define ASCII_FS        28      // File Separator
#define ASCII_GS        29      // Group Separator
#define ASCII_RS        30      // Record Separator
#define ASCII_US        31      // Unit Separator
#define ASCII_DEL       127     // Delete


// The ANSI Escape Sequences
// http://en.wikipedia.org/wiki/ANSI_escape_code

#define ANSI_CSI        [ ASCII_ESC, '[' ]  // Control Sequence Introducer

#define ANSI_CUU        'A'     // Cursor Up
#define ANSI_CUD        'B'     // Cursor Down
#define ANSI_CUF        'C'     // Cursor Forward
#define ANSI_CUB        'D'     // Cursor Backward
#define ANSI_CNL        'E'     // Cursor Next Line
#define ANSI_CPL        'F'     // Cursor Previous Line
#define ANSI_CHA        'G'     // Cursor Horizontal Absolute
#define ANSI_CUP        'H'     // Cursor Position
#define ANSI_ED         'J'     // Erase Data
#define ANSI_EL         'K'     // Erase in Line
#define ANSI_SU         'S'     // Scroll Up
#define ANSI_ST         'T'     // Scroll Down
#define ANSI_HVP        'f'     // Horizontal and Vertical Position
#define ANSI_SGR        'm'     // Select Graphic Rendition
#define ANSI_DSR        'n'     // Device Status Report
#define ANSI_SCP        's'     // Save Cursor Position
#define ANSI_RCP        'u'     // Restore Cursor Position


SpecialChars::SpecialChars() { }

SpecialChars::~SpecialChars() { }

bool SpecialChars::eat(QString *)
{
    // TODO - define signals in specialchars.h
    // TODO - parse the first character and proceed from there
    return false;
}

