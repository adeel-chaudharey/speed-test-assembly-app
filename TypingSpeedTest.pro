QT       += core widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET   = TypingSpeedTest
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

# Windows subsystem (no console window)
win32: RC_ICONS =

# After build, copy the assembly executable to all potential target folders
win32 {
    # Convert paths to use strict Windows backslashes safely
    WIN_PWD = $$replace(PWD, /, \\)
    WIN_OUT = $$replace(OUT_PWD, /, \\)

    # Copy to Root, Debug, and Release folders to cover all bases
    QMAKE_POST_LINK += copy /Y \"$$WIN_PWD\\asm\\typing_analyzer.exe\" \"$$WIN_OUT\\typing_analyzer.exe\" && \
                       copy /Y \"$$WIN_PWD\\asm\\typing_analyzer.exe\" \"$$WIN_OUT\\debug\\typing_analyzer.exe\" && \
                       copy /Y \"$$WIN_PWD\\asm\\typing_analyzer.exe\" \"$$WIN_OUT\\release\\typing_analyzer.exe\"
}