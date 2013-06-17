
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

#include <iostream>
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
    //ShowWindow(GetConsoleWindow(), SW_HIDE);

    HANDLE in  = CreateFile(L"CONIN$",  GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    HANDLE out = CreateFile(L"CONOUT$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

    SetStdHandle(STD_INPUT_HANDLE,  in);
    SetStdHandle(STD_OUTPUT_HANDLE, out);
    SetStdHandle(STD_ERROR_HANDLE,  out);

    SetConsoleMode(out, ENABLE_WRAP_AT_EOL_OUTPUT);

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

    // A quick test
    wchar_t buffer[256];
    DWORD nb = 0;
    while (1) 
    {
        //if (ReadFile(GetStdHandle(STD_OUTPUT_HANDLE), buffer, sizeof(buffer), &nb, 0))
        if (ReadConsole(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer), &nb, 0))
        {
            std::cout << QString::fromWCharArray(buffer, nb / sizeof(wchar_t)).toUtf8().data()
                      << std::endl;
            std::cout.flush();
        }
        else
        {
            std::cout << GetLastError() << "\n";

            // Mehh so any attempt to read the screen buffer's (stdout) handle
            // causes ERROR_INVALID_HANDLE. We're going to have to poll the
            // screen buffer after all, fml. 
        }
    }

    // So, all hope is not lost, we just need to be more clever about how we
    // manage the screen buffer. If we're not worried about full-screen apps
    // like vim (which I presume writes directly to the screen buffer), we can
    // just use the entire screen buffer as an intermediate, and delete data /
    // move the cursor as necessary once we've retrieved data from the buffer.
    // Then we delete any line wraps where the line was the length of the
    // screen buffer, on the assumption that cmd.exe added an automatic line
    // break.
    //
    // There are two problems with running things like vim with this strategy:
    // - Vim, after filling the entire screen, will probably jump around and
    //   try to modify the middle of the screen buffer without redrawing
    //   everything. We'd need to cooperate with the history buffer to make
    //   this work, although generating move-cursor events might not be the end
    //   of the world
    // - Vim will fill each line on purpose. We're going to end up removing
    //   those newlines. As long as the screen buffer matches the size of the
    //   viewport in terms of rows and columns, word-wrap will fix this and
    //   it'll probably look fine, but resizing might make things look weird.
    //
    // We'd have to clear the screen buffer to some non-printable char that no
    // program would ever use. Maybe filling with nulls would be sufficient.
    // The point is, filling with spaces means we can't tell the difference
    // between "vim printed a bunch of spaces" and "vim didn't print anything"
    //
    // All in all, this is looking to be a complete mess *sigh* 
    // Better start reading up on Read/WriteConsoleOutput. 
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
