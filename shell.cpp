
#include "shell.h"

#include "processshell.h"
#include "windowsshell.h"

Shell* Shell::create()
{
#ifdef Q_OS_WIN32

    return new WindowsShell(); // TODO shell binary and args

#else
    // TODO binary information belongs in a configuration file
    QString binary("C:\\MinGW\\msys\\1.0\\bin\\sh.exe");

    QStringList args;
    args.append("--login");
    args.append("-i");

    return new ProcessShell(binary, args);
#endif
}

