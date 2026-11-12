QT       += core gui widgets

CONFIG   += c++17

TARGET = MyNotepad
TEMPLATE = app

SOURCES += main.cpp \
    mainwindow.cpp \
    codeeditor.cpp \
    aboutdialog.cpp \
    searchdialog.cpp \
    replacedialog.cpp

HEADERS += mainwindow.h \
    codeeditor.h \
    aboutdialog.h \
    searchdialog.h \
    replacedialog.h

FORMS += mainwindow.ui \
    aboutdialog.ui \
    searchdialog.ui \
    replacedialog.ui
