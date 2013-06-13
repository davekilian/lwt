
#include "specialchars.h"

#include <QDebug>
#include <QStringList>
#include <QVector>

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

#define ANSI_INS        '@'     // Insert Characters
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
#define ANSI_DEL        'P'     // Delete Characters
#define ANSI_SU         'S'     // Scroll Up
#define ANSI_SD         'T'     // Scroll Down
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

int SpecialChars::eat(const QString &str, int index)
{
    QChar c = str[index];

    switch (c.toLatin1())
    {
        case ASCII_BEL:
            emit bell();
            return index + 1;

        case ASCII_BS:
            emit moveCursorBy(0, -1);
            return index + 1;

        case ASCII_CR:
            emit carriageReturn();
            return index + 1;

        case ASCII_DEL:
            emit del(1);
            return index + 1;

        case ASCII_FF:
            emit formFeed();
            return index + 1;

        case ASCII_HT:
            emit horizontalTab();
            return index + 1;

        case ASCII_VT:
            emit verticalTab();
            return index + 1;

        case ASCII_ESC:
        {
            char cmd;
            QString args;
            int ret = index;

            if (parseAnsi(str, &cmd, &args, &ret))
            {
                // Convert the args string to a list of integers,
                // since most commands require integer arguments.
                // Convenient for the case statements below
                QVector<int> intargs;
                QStringList parts = args.split(';', QString::SkipEmptyParts);
                foreach (QString part, parts)
                {
                    bool ok = false;
                    int val = part.toInt(&ok);
                    if (ok)
                        intargs.append(val);
                    else
                        intargs.append(0xFFFFFFFF);
                }

                switch (cmd)
                {
                    case ANSI_CUU:
                        emit moveCursorBy(-intargs.value(0, 1), 0);
                        return ret;

                    case ANSI_CUD:
                        emit moveCursorBy(intargs.value(0, 1), 0);
                        return ret;

                    case ANSI_CUF:
                        emit moveCursorBy(0, intargs.value(0, 1));
                        return ret;

                    case ANSI_CUB:
                        emit moveCursorBy(0, -intargs.value(0, 1));
                        return ret;

                    case ANSI_CNL:
                        emit moveCursorBy(intargs.value(0, 1), 0);
                        emit moveCursorTo(-1, 0);
                        return ret;

                    case ANSI_CPL:
                        emit moveCursorBy(-intargs.value(0, 1), 0);
                        emit moveCursorTo(-1, 0);
                        return ret;

                    case ANSI_CHA:
                        emit moveCursorTo(-1, intargs.value(0, 0));
                        return ret;

                    case ANSI_CUP:
                    case ANSI_HVP:
                        emit moveCursorTo(intargs.value(0, 1) - 1,
                                          intargs.value(1, 1) - 1);
                        return ret;

                    case ANSI_ED:
                        emit erase((EraseType)intargs.value(0, 0));
                        return ret;

                    case ANSI_EL:
                        emit erase((EraseType)(intargs.value(0, 0) + 3));
                        return ret;

                    case ANSI_SU:
                        emit scroll(-intargs.value(0, 1));
                        return ret;

                    case ANSI_SD:
                        emit scroll(intargs.value(0, 1));
                        return ret;

                    case ANSI_SGR:
                        handleSGR(intargs);
                        return ret;

                    case ANSI_DSR:
                        emit reportCursorPosition();
                        return ret;

                    case ANSI_SCP:
                        emit pushCursorPosition();
                        return ret;

                    case ANSI_RCP:
                        emit popCursorPosition();
                        return ret;

                    case ANSI_INS:
                        emit insert(intargs.value(0, 1));
                        return ret;

                    case ANSI_DEL:
                        emit del(intargs.value(0, 1));
                        return ret;

                    case DECTCEM_HIC:
                        emit setCursorVisible(false);
                        return ret;

                    case DECTCEM_SHC:
                        emit setCursorVisible(true);
                        return ret;

                    default:
                        unknownSequence(str, index, cmd, args);
                        return ret;
                }
            }
            else if (parseXterm(str, &cmd, &args, &ret))
            {
                switch (cmd)
                {
                    case XTERM_CNW:
                    case XTERM_CIN:
                    case XTERM_CWT:
                        emit setWindowTitle(args);
                        return ret;

                    default:
                        unknownSequence(str, index, cmd, args);
                        return ret;
                }
            }
            else
            {
                unknownSequence(str, index, 0, QString());
            }

            return ret;
        }

        default:
            return index;
    }

    return index;
}

bool SpecialChars::parseAnsi(const QString &str, char *cmd, QString *args, int *index)
{
    char next = str[*index + 1].toLatin1();
    if (next == '[')
    {
        // Multi-character sequence
        int i = 2;
        next = str[*index + i].toLatin1();
        while (next < '@' || next > '~')
        {
            args->append(next);
            ++i;
            next = str[*index + i].toLatin1();
        }

        *cmd = next;
        *index += i + 1;

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
        *index += 2;

        return true;
    }
    else
    {
        // Don't know how to parse this
        return false;
    }
}

bool SpecialChars::parseXterm(const QString &str, char *cmd, QString *args, int *index)
{
    // Make sure this is an Operating System Control sequence
    char next = str[*index + 1].toLatin1();
    if (next != ']')
    {
        return false;
    }

    // Parse the command ID
    int i = 2;
    QString cmdstr;
    while ((next = str[*index + i].toLatin1()) != ';')
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
    while ((next = str[*index + i].toLatin1()) != ASCII_STX && next != ASCII_BEL)
    {
        args->append(next);
        ++i;
    }

    *index += i + 1;
    return true;
}

void SpecialChars::handleSGR(QVector<int> args)
{
    // Check if colors need to be reset
    foreach (int arg, args)
    {
        if (arg == 0)
        {
            emit resetColors();
            break;
        }
    }

    // Check if the color needs to be bright
    bool bright = false;
    foreach (int arg, args)
    {
        if (arg == 1)
        {
            bright = true;
            break;
        }
    }

    // Check for a color to switch to
    bool haveColor = false;
    Color color = DEFAULT;
    bool foreground = true;

    foreach (int arg, args)
    {
        if (arg >= 30 && arg <= 39 && arg != 38)
        {
            haveColor = true;
            foreground = true;
            color = (Color)(arg - 30);

            break;
        }

        if (arg >= 40 && arg <= 49 && arg != 48)
        {
            haveColor = true;
            foreground = false;
            color = (Color)(arg - 40);

            break;
        }
    }

    // If there's a color to switch to, switch
    if (haveColor)
    {
        emit setColor(color, bright, foreground);
    }

    // Otherwise check for any xterm-256 commands
    else
    {
        for (int i = 0; i < args.size(); ++i)
        {
            int cmd = args[i];

            if (cmd == 38 || cmd == 48)
            {
                bool foreground = (cmd == 38);

                if (i + 2 < args.size() && args[i + 1] == 5)
                {
                    emit setColor256(args[i + 2], foreground);
                }
            }
        }
    }
}

// http://unix.stackexchange.com/questions/16530/what-does-raw-unraw-keyboard-mode-mean
QString SpecialChars::translate(QKeyEvent *ev)
{
    QString ret = ev->text();

    // Add ANSI escape sequences for special keys
    QList<char> commands;
    
    if ((ev->key() & Qt::Key_Up) == Qt::Key_Up)
        commands.append(ANSI_CUU);

    if ((ev->key() & Qt::Key_Down) == Qt::Key_Down)
        commands.append(ANSI_CUD);

    if ((ev->key() & Qt::Key_Left) == Qt::Key_Left)
        commands.append(ANSI_CUB);

    if ((ev->key() & Qt::Key_Right) == Qt::Key_Right)
        commands.append(ANSI_CUF);

    foreach (char command, commands)
    {
        ret.append(ASCII_ESC);
        ret.append('[');
        ret.append(command);
    }

    return ret;
}

void SpecialChars::unknownSequence(const QString &str, int index, 
                                   char cmd, const QString &args)
{
    qDebug() << "Unknown control sequence starting at position"
             << index;

    // This is a separate call to qDebug() because (at least on my machine)
    // the entire qDebug statement will no-op if the str has non-printable
    // characters in it >.>
    qDebug() << "in:\n"
             << str;

    if (cmd != 0)
    {
        qDebug() << "Parsed command: " << (int)cmd << " (" << cmd << ")\n"
                 << "With args: " << args;
    }

    qDebug() << "Output may be garbled\n";
}

