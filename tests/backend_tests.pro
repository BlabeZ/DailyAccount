TEMPLATE = app
TARGET = backend_tests
CONFIG += console c++17
CONFIG -= app_bundle
QT -= core gui

INCLUDEPATH += ../backend

SOURCES += \
    backend_tests.cpp \
    ../backend/category.cpp \
    ../backend/storage.cpp \
    ../backend/ledger.cpp

HEADERS += \
    ../backend/record.h \
    ../backend/category.h \
    ../backend/storage.h \
    ../backend/ledger.h

QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic -Werror
