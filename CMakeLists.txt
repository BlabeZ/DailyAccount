cmake_minimum_required(VERSION 3.16)

project(DailyAccount VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

option(DA_ENABLE_QT "Build the Qt Widgets desktop application" OFF)

add_library(dailyaccount_platform_interfaces INTERFACE)
target_include_directories(dailyaccount_platform_interfaces INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/accounting)

add_library(dailyaccount_accounting_domain STATIC
    backend/category.cpp
    backend/storage.cpp
    backend/ledger.cpp)
target_include_directories(dailyaccount_accounting_domain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/backend)
target_link_libraries(dailyaccount_accounting_domain PUBLIC
    dailyaccount_platform_interfaces)

add_library(dailyaccount_accounting_module STATIC
    platform/module_registry.cpp
    modules/accounting/accounting_module.cpp)
target_include_directories(dailyaccount_accounting_module PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/accounting)
target_link_libraries(dailyaccount_accounting_module PUBLIC
    dailyaccount_platform_interfaces)

enable_testing()

add_executable(dailyaccount_backend_tests tests/backend_tests.cpp)
target_link_libraries(dailyaccount_backend_tests PRIVATE
    dailyaccount_accounting_domain)
add_test(NAME dailyaccount_backend_tests COMMAND dailyaccount_backend_tests)

add_executable(dailyaccount_registry_tests tests/registry_tests.cpp)
target_link_libraries(dailyaccount_registry_tests PRIVATE
    dailyaccount_accounting_module)
add_test(NAME dailyaccount_registry_tests COMMAND dailyaccount_registry_tests)

if(DA_ENABLE_QT)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)

    add_executable(dailyaccount_desktop
        gui/main_gui.cpp
        gui/mainwindow.cpp
        gui/dashboardpage.cpp
        gui/flowpage.cpp
        gui/flowdialog.cpp
        gui/statisticspage.cpp
        gui/categorypage.cpp
        gui/otherpage.cpp
        gui/flowpage.ui)
    set_target_properties(dailyaccount_desktop PROPERTIES
        OUTPUT_NAME DailyAccount)
    target_include_directories(dailyaccount_desktop PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/backend
        ${CMAKE_CURRENT_SOURCE_DIR}/gui)
    target_link_libraries(dailyaccount_desktop PRIVATE
        Qt6::Widgets
        dailyaccount_accounting_domain)

    if(WIN32)
        target_link_libraries(dailyaccount_desktop PRIVATE user32)
    endif()
endif()
