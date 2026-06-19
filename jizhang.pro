# ============================================================================
# Qt Project File - Accounting Tool (记账工具 GUI版)
# Encoding: UTF-8 (all source files)
# Build: qmake jizhang.pro && mingw32-make
# ============================================================================

QT       += core gui widgets
TEMPLATE  = app
CONFIG   += c++17
TARGET   = jizhang
RC_ICONS  =

# Header include paths (backend business logic + GUI layer)
INCLUDEPATH += backend gui

# MinGW static link workaround for -O2 optimization crash
QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

# MinGW auto-detects UTF-8 source encoding.
# No explicit -finput-charset needed.

SOURCES += \
    gui/main_gui.cpp \
    gui/mainwindow.cpp \
    gui/dashboardpage.cpp \
    gui/flowpage.cpp \
    gui/flowdialog.cpp \
    gui/statisticspage.cpp \
    gui/categorypage.cpp \
    gui/otherpage.cpp \
    backend/category.cpp \
    backend/storage.cpp \
    backend/ledger.cpp

HEADERS += \
    gui/mainwindow.h \
    gui/dashboardpage.h \
    gui/flowpage.h \
    gui/flowdialog.h \
    gui/statisticspage.h \
    gui/categorypage.h \
    gui/otherpage.h \
    backend/record.h \
    backend/category.h \
    backend/storage.h \
    backend/ledger.h

# Windows specific
win32 {
    LIBS += -luser32
}
