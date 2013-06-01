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
 *          if (!m_specialChars->eat(&inputStr))
 *              writeCharacterToConsole(inputStr[0]);
 */
class SpecialChars : public QObject
{
    Q_OBJECT

public:
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
    // TODO 
};

#endif // SPECIALCHARS_H
