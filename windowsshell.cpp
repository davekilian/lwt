
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

#include <io.h> // isatty
#include <iostream>
void WindowsShell::open()
{
    // First experiment -- let's see if we can get isatty to return true
    std::cout << "isatty(): " << (int)_isatty(_fileno(stdout)) << "\n";
    AllocConsole();
    std::cout << "isatty(): " << (int)_isatty(_fileno(stdout)) << "\n";

    HANDLE in  = CreateFile(L"CONIN$",  GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    HANDLE out = CreateFile(L"CONOUT$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    HANDLE err = CreateFile(L"CONERR$", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

    std::cout << "CONIN$:  " << (int)in  << "\n";
    std::cout << "CONOUT$: " << (int)out << "\n";
    std::cout << "CONERR$: " << (int)err << "\n";

    SetStdHandle(STD_INPUT_HANDLE, in);
    SetStdHandle(STD_OUTPUT_HANDLE, out);
    SetStdHandle(STD_ERROR_HANDLE, out);

    std::cout << "stdin handle:  " << (int)GetStdHandle(STD_INPUT_HANDLE) << "\n";
    std::cout << "stdout handle: " << (int)GetStdHandle(STD_OUTPUT_HANDLE) << "\n";
    std::cout << "stderr handle: " << (int)GetStdHandle(STD_ERROR_HANDLE) << "\n";

    std::cout << "isatty(): " << (int)_isatty(_fileno(stdout)) << "\n";
    std::cout << "fileno(stdout): " << (int)_fileno(stdout) << "\n";

    // TODO coooool back to square one. Even if we alloc and console and set
    // the std streams to the console's input/output handles, we still aren't a
    // tty ...
    //
    // Well, fuck, I have no idea what to do. 
    //
    // Well, let's try out winpty and see if we have similar issues

    return;


    /*
    // Spawn the console window in a hidden desktop
    // Most of this was adapted from git@github.com:rprichard/winpty.git
    HWINSTA orig = GetProcessWindowStation(),
            station = CreateWindowStation(0, 0, WINSTA_ALL_ACCESS, 0);

    if (!orig)
    {
        qDebug() << "WindowsShell::open() can't GetProcessWindowStation()!";
        return;
    }

    if (!station)
    {
        qDebug() << "WindowsShell::open() can't CreateWindowStation()!";
        return;
    }

    if (!SetProcessWindowStation(station))
    {
        CloseWindowStation(station);
        qDebug() << "WindowsShell::open() can't SetProcessWindowStation()!";
        return;
    }

    HDESK desktop = CreateDesktop(L"Default", 0, 0, 0, GENERIC_ALL, 0);
    if (!desktop)
    {
        SetProcessWindowStation(orig);
        CloseWindowStation(station);

        qDebug() << "WindowsShell::open() can't CreateDesktop()!";
        return;
    }

    // Create the console
    if (!AllocConsole())
    {
        SetProcessWindowStation(orig);
        CloseDesktop(desktop);
        CloseWindowStation(station);

        qDebug() << "WindowsShell::open() can't AllocConsole()!";
        return;
    }

    // Spawn the shell process
    // TODO

    // TODO this background desktop business doesn't work
    // Some ideas:
    //
    // - ShowWindow(GetConsoleWindow(), SW_HIDE) -- but causes a brief flicker
    // - Use the StartupInfo struct to start a process with the window hidden.
    //   But then we'd have to find a way to attach to that process's console
    
    // Clean up our handles to the hidden desktop
    SetProcessWindowStation(orig);
    CloseDesktop(desktop);
    CloseWindowStation(station);
    */

    // TODO Okay, new strategy
    // - First figure out how to spawn the shell process. We should use the
    //   startupinfo struct to make sure the console window is hidden. See:
    //   http://stackoverflow.com/questions/4743559/how-to-execute-child-console-programs-without-showing-the-console-window-from-th
    // - Then, knowing that process's ID (I assume we'll get that back from
    //   CreateProcess), we can just call AttachConsole(consoleId). Then we can
    //   hopefully manipulate the console as necessary? If we didn't create the
    //   console, I foresee possible permission issues however. We'll see ...
    //
    // Another strategy is to ShowWindow(GetConsoleWindow(), SW_HIDE) after we
    // alloc the console. This shows the console window, which is unfortunate,
    // but it also means we own the console and thus should have all
    // permissions on it.

    /*
		STARTUPINFO startupInfo;
		PROCESS_INFORMATION processInfo;
		::ZeroMemory(&startupInfo,sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		startupInfo.dwFlags = STARTF_USESHOWWINDOW;
		startupInfo.wShowWindow = SW_HIDE;
//		startupInfo.wShowWindow = SW_SHOW;
		if (!CreateProcess( NULL, "cmd.exe", NULL, NULL, FALSE, 
			CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInfo))
		{
			reportFailure("CreateProcess",GetLastError());
		}
		if (!AttachConsole(processInfo.dwProcessId))
		{
			reportFailure("AttachConsole",GetLastError());
		}
     * 
     */

    // TODO take as args somehow
    //      this really belongs in a configuration file
    QString cmd("C:\\MinGW\\msys\\1.0\\bin\\sh.exe --login -i");
    wchar_t *w_cmd = new wchar_t[cmd.size()];
    cmd.toWCharArray(w_cmd);

    STARTUPINFO si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));

    if (!CreateProcess(0, w_cmd, 0, 0, false,
                       CREATE_NEW_CONSOLE, 0, 0, &si, &pi))
    {
        qDebug() << "WindowsShell::open() can't CreateProcess():"
                 << GetLastError();

        delete[] w_cmd;
        return;
    }

    delete[] w_cmd;


    /////////////////////////////////////////////////////////////////////
    // Herein I tried to figure out how to attach to a console without
    // displaying a window. Needless to say, I had a few issues :)
    /////////////////////////////////////////////////////////////////////

    // while (FreeConsole()) ;
    // This throws the app into an infinite loop. In MinGW doing something?
    // Mehh let's grep the mingw source for console stuff

    if (!AttachConsole(pi.dwProcessId))
    {
        qDebug() << "WindowsShell::open() can't AttachConsole(): "
                 << GetLastError();

        // AttachConsole is returning 0x5 -- access denied
        // That should only happen if we have an open console
        // Interestingly, AllocConsole is totally fine
        // http://www.progtown.com/topic843578-winapi-attachconsole-attachparentprocessattachconsole-dwprocessid.html
        // In fact, we could totally AllocConsole / FreeConsole / AttachConsole
        // now. But that's no better than AllocConsole + hiding the window.

        // Okay, well do we have a console?
        /*
        wchar_t title[256];
        if (256 != GetConsoleTitle(title, 256))
        {
            std::cout << GetLastError() << "\n";
            // 0x6 -- ERROR_INVALID_HANDLE
            throw 0;
        }
        */

        // Well, we don't have a console window. But maybe someone closed the
        // window and we actually have a console?
        /*
        CONSOLE_CURSOR_INFO cci;
        memset(&cci, 0, sizeof(cci));
        if (!GetConsoleCursorInfo
        */

        TerminateProcess(pi.hProcess, 127);
        //return;
        throw 0;
    }

    // TODO so we're definitely not a console. We can even CreateFile on CONIN$
    // and friends. Why FreeConsole seems to think there's a console is beyond
    // me. 
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
