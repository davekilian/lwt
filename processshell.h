#ifndef PROCESSSHELL_H
#define PROCESSSHELL_H

#include "shell.h"

#include <QObject>
#include <QProcess>

/** A basic, process-based shell driver.
 *
 *  This driver simply spins up a process and communicates with it via its std
 *  streams (stdin/stdout/stderr). However, this implementation does not spawn
 *  any sort of pseudo-terminal, which causes the shell (and any processes the
 *  shell spawns) to get isatty() = false. 
 *
 *  This driver is hence a fallback for when we don't have a platform-specific
 *  driver. It's guaranteed to work on just about any OS, but it forces the
 *  user to run all programs in non-interactive mode.
 */
class ProcessShell : public Shell
{
    Q_OBJECT

public:
    ProcessShell(const QString &command, const QStringList &args = QStringList());
    virtual ~ProcessShell();

    /** The shell binary to run when the shell is opened */
    const QString &command() const;

    /** The list of arguments to pass to the shell binary */
    const QStringList &args() const;

    /** Abstract shell methods documented in shell.h */
    void open();
    void close();
    bool isOpen();
    QString write(const QString &str);

private slots:
    void onstdout();
    void onclose();

private:
    QString m_command;
    QStringList m_args;
    QProcess m_process;
};

#endif // PROCESSSHELL_H
