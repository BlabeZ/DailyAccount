@echo off
setlocal EnableExtensions

for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%~dp0out"
set "TEST_BUILD_DIR=%BUILD_DIR%\tests"
set "APP_BUILD_DIR=%BUILD_DIR%\app"
set "DIST_DIR=%~dp0dist"

if not defined QT_DIR set "QT_DIR=D:\tools\Qt\6.9.3\mingw_64"
if not defined MINGW_DIR set "MINGW_DIR=D:\tools\mingw64\bin"
set "QMAKE=%QT_DIR%\bin\qmake.exe"
set "WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe"
set "MAKE=%MINGW_DIR%\mingw32-make.exe"
set "PATH=%QT_DIR%\bin;%MINGW_DIR%;%PATH%"

echo ====================================
echo   DailyAccount - Windows Build
echo ====================================
echo Project: "%ROOT_DIR%"
echo Qt:      "%QT_DIR%"
echo MinGW:   "%MINGW_DIR%"
echo.

if not exist "%QMAKE%" goto :missing_qmake
if not exist "%WINDEPLOYQT%" goto :missing_windeployqt
if not exist "%MAKE%" goto :missing_make

echo [1/5] Cleaning build output...
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "%DIST_DIR%" rmdir /s /q "%DIST_DIR%"
mkdir "%TEST_BUILD_DIR%" || goto :mkdir_failed
mkdir "%APP_BUILD_DIR%" || goto :mkdir_failed
mkdir "%DIST_DIR%" || goto :mkdir_failed

echo [2/5] Building backend tests...
pushd "%TEST_BUILD_DIR%" || goto :pushd_failed
"%QMAKE%" "%ROOT_DIR%\tests\backend_tests.pro" CONFIG+=release
if errorlevel 1 (popd & goto :test_qmake_failed)
"%MAKE%"
if errorlevel 1 (popd & goto :test_build_failed)
popd

set "TEST_EXE=%TEST_BUILD_DIR%\release\backend_tests.exe"
if not exist "%TEST_EXE%" set "TEST_EXE=%TEST_BUILD_DIR%\backend_tests.exe"
if not exist "%TEST_EXE%" goto :missing_test_binary

echo [3/5] Running backend tests...
"%TEST_EXE%"
if errorlevel 1 goto :tests_failed

echo [4/5] Building application...
pushd "%APP_BUILD_DIR%" || goto :pushd_failed
"%QMAKE%" "%ROOT_DIR%\jizhang.pro" CONFIG+=release
if errorlevel 1 (popd & goto :app_qmake_failed)
"%MAKE%"
if errorlevel 1 (popd & goto :app_build_failed)
popd

set "APP_EXE=%APP_BUILD_DIR%\release\jizhang.exe"
if not exist "%APP_EXE%" set "APP_EXE=%APP_BUILD_DIR%\jizhang.exe"
if not exist "%APP_EXE%" goto :missing_app_binary
copy /y "%APP_EXE%" "%DIST_DIR%\DailyAccount.exe" >nul
if errorlevel 1 goto :copy_failed

echo [5/5] Deploying Qt and compiler runtime...
"%WINDEPLOYQT%" --release --compiler-runtime --no-translations "%DIST_DIR%\DailyAccount.exe"
if errorlevel 1 goto :deploy_failed

echo.
echo ====================================
echo   Build and tests succeeded
echo   Package: "%DIST_DIR%"
echo ====================================
exit /b 0

:missing_qmake
echo [ERROR] qmake.exe not found: "%QMAKE%"
goto :failed
:missing_windeployqt
echo [ERROR] windeployqt.exe not found: "%WINDEPLOYQT%"
goto :failed
:missing_make
echo [ERROR] mingw32-make.exe not found: "%MAKE%"
goto :failed
:mkdir_failed
echo [ERROR] Could not create a build directory.
goto :failed
:pushd_failed
echo [ERROR] Could not enter a build directory.
goto :failed
:test_qmake_failed
echo [ERROR] Test qmake step failed.
goto :failed
:test_build_failed
echo [ERROR] Test compilation failed.
goto :failed
:missing_test_binary
echo [ERROR] backend_tests.exe was not produced.
goto :failed
:tests_failed
echo [ERROR] Backend tests failed.
goto :failed
:app_qmake_failed
echo [ERROR] Application qmake step failed.
goto :failed
:app_build_failed
echo [ERROR] Application compilation failed.
goto :failed
:missing_app_binary
echo [ERROR] jizhang.exe was not produced.
goto :failed
:copy_failed
echo [ERROR] Could not copy the application binary.
goto :failed
:deploy_failed
echo [ERROR] windeployqt failed.
goto :failed

:failed
exit /b 1
