
#include "shell.h"

#include "processshell.h"

Shell* Shell::create()
{
    // TODO binary information belongs in a configuration file
    QString binary("C:\\MinGW\\msys\\1.0\\bin\\sh.exe");

    QStringList args;
    args.append("--login");
    args.append("-i");

    return new ProcessShell(binary, args);
}

