#ifndef SPECIALCHARS_H
#define SPECIALCHARS_H

#include <QObject>

/** Utility for recognizing and removing ASCII control codes and ANSI escape
 *  sequences from an input stream. This utility only recognizes special
 *  sequences; behavior for handling the sequences is defined elsewhere.
 *
 *  To use this object, TerminalWidget does the following:
 *
 *  - Instantiate a SpecialChars object
 *  - Hook up specific behaviors to signals of the SpecialChars object
 *  - When receiving input from the shell ...
 *
 *      while (inputStr.length() > 0)
 *      {
 *          if (!m_specialChars->eat(&inputStr))
 *          {
 *              writeCharacterToConsole(inputStr[0]);
 *              inputStr = inputStr.right(inputStr.length() - 1);
 *          }
 *      }
 */
class SpecialChars : public QObject
{
    Q_OBJECT

public:
    enum EraseType
    {
        TODO,
    };

    enum Color
    {
        ALSO_TODO,
    };

    SpecialChars();
    ~SpecialChars();

    /** Tries to eat a special character sequence from the beginning of the
     *  given string.
     *
     *  If the string starts with a control sequence, this method triggers the
     *  relevant signal, modifies the given string by removing the first
     *  character, and returns true.
     *
     *  Otherwise, the method returns false and does not modify the string.
     *
     *  @param str  A pointer to the input string to eat characters from. If an
     *              escape sequence is removed from the start of the string,
     *              this pointers receives the new string.
     *
     *  @return     A boolean indicating whether a sequence was removed.
     */
    bool eat(QString *str);

signals:

    /** Play the system bell sound */
    void bell();

    /** Clear the character before the cursor and move the cursor back.
     *  No effect if the cursor is at the beginning of its line.
     */
    void backspace();

    /** Move the cursor to the beginning of its line */
    void carriageReturn();

    /** Clear the character underneath the cursor */
    void del();

    /** Clear some part of the screen. See EraseType for specific behaviors */
    void erase(EraseType type);

    /** Clear the screen */
    void formFeed();

    /** Indent the cursor */
    void horizontalTab();

    /** Move the cursor relative to its current position */
    void moveCursorBy(int rowDelta, int colDelta);

    /** Move the cursor to the given position.
     *  If either row or col is negative, don't change the corresponding
     *  property of the cursor
     */
    void moveCursorTo(int row, int col);

    /** Invoke Shell::reportCursorPosition() 
     *  TODO implement Shell::reportCursorPosition()
     */
    void reportCursorPosition();

    /** Set the cursor position to the position stored during the
     *  corresponding pushCursorPosition() operation. If there is no matching
     *  operation, do nothing.
     */
    void popCursorPosition();

    /** Store the cursor's current position on a stack */
    void pushCursorPosition();

    /** Scroll down by the given number of pages.
     *  If npages is negative, scroll up by that many pages
     */
    void scroll(int npages);

    /** Set either the foreground or the background color
      * @param c indicates which color from the theme palette to use
      * @param bright indicates whether the bright or normal color is desired
      * @param foreground indicates whether to set the foreground or background
      *                   color
      */
    void setColor(Color c, bool bright, bool foreground);

    /** Set either the foreground or the background color from a 256-color
      * scheme.
      * @param index is the color to use. 0 <= index <= 255
      * @param foreground indicates whether to set the foreground or background
      *                   color
      */
    void setColor256(int index, bool foreground);

    /** Show or hide the cursor */
    void setCursorVisible(bool visible);

    /** Set the title of the terminal's window or tab */
    void setWindowTitle(QString title);

    /** Add several newlines */
    void verticalTab();
    
private:
    /** Attempts to parse an ANSI control sequence.
     *  Removes the sequence from the input string if successful.
     *
     *  @param str  Pointer to the input string
     *  @param cmd  Receives the command code
     *  @param args Receives the command arguments
     */
    bool parseAnsi(QString *str, char *cmd, QString *args);

    /** Attempts to parse an xterm OS control sequence.
     *  Removes the sequence from the input string if successful.
     *
     *  @param str  Pointer to the input string
     *  @param cmd  Receives the command code
     *  @param args Receives the command arguments
     */
    bool parseXterm(QString *str, char *cmd, QString *args);
};

#endif // SPECIALCHARS_H
