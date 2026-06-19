@echo off
echo ====================================
echo   Accounting Tool - Build Script
echo ====================================
echo.

set QT_DIR=D:\tools\Qt\6.9.3\mingw_64
set MINGW_DIR=D:\tools\mingw64\bin
set PATH=%QT_DIR%\bin;%MINGW_DIR%;%PATH%

:: Check qmake
where qmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] qmake not found. Check Qt installation.
    echo QT_DIR=%QT_DIR%
    pause
    exit /b 1
)

echo [1/3] Cleaning old build files...
if exist Makefile del /q Makefile
if exist Makefile.Release del /q Makefile.Release
if exist Makefile.Debug del /q Makefile.Debug
if exist release rmdir /s /q release
if exist debug rmdir /s /q debug
if exist .qmake.stash del /q .qmake.stash

echo [2/3] Running qmake...
qmake jizhang.pro
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] qmake failed!
    pause
    exit /b 1
)

echo [3/3] Building...
mingw32-make
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ====================================
    echo   Build SUCCESS!
    echo   Binary: release\jizhang.exe
    echo ====================================
    echo.
    copy /y release\jizhang.exe "..\记账.exe" >nul 2>nul
    echo   Copied to: ..\记账.exe
) else (
    echo.
    echo [ERROR] Build failed. Check errors above.
)

pause
