
TARGET    = lwt
TEMPLATE  = app

QT       += core gui widgets

HEADERS  += cursor.h \
            mainwindow.h \
            shell.h \
            specialchars.h \
            terminalwidget.h \

SOURCES  += cursor.cpp \
            main.cpp \
            mainwindow.cpp \
            shell.cpp \
            specialchars.cpp \
            terminalwidget.cpp

FORMS    += mainwindow.ui

