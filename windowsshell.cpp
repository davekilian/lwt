
#include "windowsshell.h"


#ifndef Q_OS_WIN32
// Stub imlpementations for non-Win32 platforms
WindowsShell::WindowsShell() { }
WindowsShell::~WindowsShell() { }
void WindowsShell::open() { }
void WindowsShell::close() { }
bool WindowsShell::isOpen() { return false; }
QString WindowsShell::write(const QString &str) { return str; }
#else


#include <QDebug>

#include <windows.h>

WindowsShell::WindowsShell() { }

WindowsShell::~WindowsShell() { }

void WindowsShell::open()
{
    // Set up the console that we'll use to communicate with the process
    // Setting up a console in a middle causes isatty() to return true for the
    // child processes, which means the coreutils will run in interactive mode
    //
    // XXX this method of allocating the console shows the console momentarily
    // before hiding it. There may be a better way to do this, but this works
    // and other things I tried did not. Let's get the driver working before
    // ironing out the quirks.
    AllocConsole();
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    HANDLE in  = CreateFile(L"CONIN$",  GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    HANDLE out = CreateFile(L"CONOUT$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

    SetStdHandle(STD_INPUT_HANDLE,  in);
    SetStdHandle(STD_OUTPUT_HANDLE, out);
    SetStdHandle(STD_ERROR_HANDLE,  out);

    // n.b. - a call to isatty() here will return false, but any child
    // process started by the shell will get isatty() = true (verified
    // experimentally -- comment out the ShowWindow() call above and interact
    // with the shell via the console). Not expected behavior, but not a
    // deal-breaker.

    // Start the shell process
    QString cmd("C:\\MinGW\\msys\\1.0\\bin\\sh.exe --login -i");
    wchar_t *w_cmd = new wchar_t[cmd.size()];
    cmd.toWCharArray(w_cmd);

    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));

    if (!CreateProcess(0, w_cmd, 0, 0, false, 0,
                       0, 0, &si, &pi))
    {
        qDebug() << "WindowsShell::open() can't CreateProcess():"
                 << GetLastError();

        delete[] w_cmd;
        return;
    }

    delete[] w_cmd;
}

void WindowsShell::close()
{
}

bool WindowsShell::isOpen()
{
    return false;
}

QString WindowsShell::write(const QString &str)
{
    return str;
}


#endif // Q_OS_WIN32
