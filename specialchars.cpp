
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

#define DECTCEM_HIC     'l'     // Hide Cursor
#define DECTCEM_SHC     'h'     // Show Cursor


// OS Control Sequences defined by xterm
// http://www.xfree86.org/current/ctlseqs.html

#define XTERM_CNW       0       // Change Icon Name and Window Title
#define XTERM_CIN       1       // Change Icon Name
#define XTERM_CWT       2       // Change Window Title
#define XTERM_SXP       3       // Set X Property
#define XTERM_CCN       4       // Change Color Number


SpecialChars::SpecialChars() { }

SpecialChars::~SpecialChars() { }

bool SpecialChars::eat(QString *str)
{
    QChar c = (*str)[0];

    switch (c.toLatin1())
    {
        case ASCII_BEL:
            emit bell();
            *str = str->right(str->length() - 1);
            return true;

        case ASCII_BS:
            emit backspace();
            *str = str->right(str->length() - 1);
            return true;

        case ASCII_CR:
            emit carriageReturn();
            *str = str->right(str->length() - 1);
            return true;

        case ASCII_DEL:
            emit del();
            *str = str->right(str->length() - 1);
            return true;

        case ASCII_FF:
            emit formFeed();
            *str = str->right(str->length() - 1);
            return true;

        case ASCII_HT:
            emit horizontalTab();
            *str = str->right(str->length() - 1);
            return true;

        case ASCII_VT:
            emit verticalTab();
            *str = str->right(str->length() - 1);
            return true;

        case ASCII_ESC:
        {
            char cmd;
            QString args;

            if (parseAnsi(str, &cmd, &args))
            {
                /* TODO raise signal
                void erase(EraseType type);
                void moveCursorBy(int rowDelta, int colDelta);
                void moveCursorTo(int row, int col);
                void reportCursorPosition();
                void popCursorPosition();
                void pushCursorPosition();
                void scroll(int npages);
                void setColor(Color c, bool bright, bool foreground);
                void setColor256(int index, bool foreground);
                void setCursorVisible(bool visible);
                 */
            }
            else if (parseXterm(str, &cmd, &args))
            {
                switch (cmd)
                {
                    case XTERM_CNW:
                    case XTERM_CIN:
                    case XTERM_CWT:
                        emit setWindowTitle(args);
                        return true;

                    case XTERM_CCN:
                        // TODO xterm change color number
                        return true;

                    default:
                        return false;
                }
            }

            return true;
        }

        default:
            return false;
    }

    return true;
}

bool SpecialChars::parseAnsi(QString *str, char *cmd, QString *args)
{
    char next = (*str)[1].toLatin1();
    if (next == '[')
    {
        // Multi-character sequence
        int i = 2;
        next = (*str)[i].toLatin1();
        while (next < '@' || next > '~')
        {
            args->append(next);
            ++i;
            next = (*str)[i].toLatin1();
        }

        *cmd = next;
        *str = str->right(str->length() - i - 1);

        return true;
    }
    else if (next == ']')
    {
        // xterm control sequence -- don't parse here
        return false;
    }
    else if (next >= '@' && next <= '_')
    {
        // Single-character sequence
        *cmd = next;
        *str = str->right(str->length() - 2);

        return true;
    }
    else
    {
        // Don't know how to parse this
        return false;
    }
}

bool SpecialChars::parseXterm(QString *str, char *cmd, QString *args)
{
    // Make sure this is an Operating System Control sequence
    char next = (*str)[1].toLatin1();
    if (next != ']')
    {
        return false;
    }

    // Parse the command ID
    int i = 2;
    QString cmdstr;
    while ((next = (*str)[i].toLatin1()) != ';')
    {
        cmdstr.append(next);
        ++i;
    }

    bool ok = false;
    *cmd = cmdstr.toInt(&ok);
    if (!ok)
    {
        return false;
    }

    // Parse the arguments
    ++i; // Skip over the semicolon
    while ((next = (*str)[i].toLatin1()) != ASCII_STX && next != ASCII_BEL)
    {
        args->append(next);
        ++i;
    }

    // Truncate the input string
    *str = str->right(str->length() - i - 1);

    return true;
}

