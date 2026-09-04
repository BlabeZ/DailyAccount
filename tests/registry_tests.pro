TEMPLATE = app
TARGET = registry_tests
CONFIG += console c++17
CONFIG -= app_bundle
QT -= core gui

INCLUDEPATH += \
    ../platform \
    ../modules/accounting

SOURCES += \
    registry_tests.cpp \
    ../platform/module_registry.cpp \
    ../modules/accounting/accounting_module.cpp

HEADERS += \
    ../platform/module_descriptor.h \
    ../platform/module_registry.h \
    ../modules/accounting/accounting_module.h

QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic -Werror
