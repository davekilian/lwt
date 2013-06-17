
TARGET    = lwt
TEMPLATE  = app

QT       += core gui widgets

HEADERS  += cursor.h \
            history.h \
            mainwindow.h \
            processshell.h \
            renderdata.h \
            shell.h \
            specialchars.h \
            terminalwidget.h \
            theme.h \
            windowsshell.h 

SOURCES  += cursor.cpp \
            main.cpp \
            history.cpp \
            renderdata.cpp \
            mainwindow.cpp \
            processshell.cpp \
            shell.cpp \
            specialchars.cpp \
            terminalwidget.cpp \
            theme.cpp \
            windowsshell.cpp

FORMS    += mainwindow.ui

