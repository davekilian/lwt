#ifndef WINDOWSSHELL_H
#define WINDOWSSHELL_H

#include "shell.h"

/** Windows-specific driver that spawns a Windows console for the shell and
 *  children to run in. This is the Windows equivalent to spawning a
 *  pseudo-terminal on unixes. 
 */
class WindowsShell : public Shell
{
    Q_OBJECT

public:
    WindowsShell();
    ~WindowsShell();

    /** Abstract shell methods documented in shell.h */
    void open();
    void close();
    bool isOpen();
    QString write(const QString &str);
};

#endif
