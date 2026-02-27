@echo off
REM Build script for Windows ARM64 (MSVC)
REM This script sets up the MSVC environment and builds the tests.
REM
REM Usage:
REM   build_msvc.bat          - Build arm64_ctx_test.exe (default)
REM   build_msvc.bat all      - Build all tests
REM   build_msvc.bat clean    - Clean build artifacts

setlocal EnableDelayedExpansion

REM Try to find vcvarsall.bat
set VCVARS=
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"
)

if "%VCVARS%"=="" (
    echo Error: Could not find vcvarsall.bat
    echo Please run this script from "ARM64 Native Tools Command Prompt for VS"
    echo or install Visual Studio with C++ ARM64 workload.
    exit /b 1
)

echo Setting up MSVC environment...
call "%VCVARS%" arm64 >nul 2>&1

REM Get script directory
set "SRCDIR=%~dp0"
REM Remove trailing backslash
if "%SRCDIR:~-1%"=="\" set "SRCDIR=%SRCDIR:~0,-1%"
set "BUILDDIR=%SRCDIR%\..\build\arm64-win"
set "ROOTDIR=%SRCDIR%\.."

if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"

if "%1"=="clean" goto clean
if "%1"=="all" goto all

:default
echo Building arm64_ctx_test.exe...
call :build_ctx_test
goto end

:all
echo Building all tests...
call :build_ctx_test
if errorlevel 1 goto end
call :build_fiber_test
if errorlevel 1 goto end
call :build_cont_test
if errorlevel 1 goto end
call :build_cont_clone_test
if errorlevel 1 goto end
call :build_eff_test
goto end

:clean
echo Cleaning build artifacts...
if exist "%BUILDDIR%\*.obj" del "%BUILDDIR%\*.obj"
if exist "%BUILDDIR%\*.exe" del "%BUILDDIR%\*.exe"
if exist "%BUILDDIR%\*.pdb" del "%BUILDDIR%\*.pdb"
if exist "%BUILDDIR%\*.ilk" del "%BUILDDIR%\*.ilk"
goto end

:build_ctx_test
echo [AS] arm64_ctx.asm
armasm64 /nologo -o "%BUILDDIR%\arm64_ctx.obj" "%SRCDIR%\arm64_ctx.asm"
if errorlevel 1 exit /b 1

echo [CC] arm64_ctx_test.c
cl /nologo /W4 /Od /Zi /I "%SRCDIR%" /c /Fo"%BUILDDIR%\arm64_ctx_test.obj" "%SRCDIR%\arm64_ctx_test.c"
if errorlevel 1 exit /b 1

echo [LINK] arm64_ctx_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\arm64_ctx_test.exe" "%BUILDDIR%\arm64_ctx_test.obj" "%BUILDDIR%\arm64_ctx.obj"
if errorlevel 1 exit /b 1

echo arm64_ctx_test.exe built successfully.
exit /b 0

:build_fiber_test
echo [AS] fiber_trampoline.asm
armasm64 /nologo -o "%BUILDDIR%\fiber_trampoline.obj" "%SRCDIR%\fiber_trampoline.asm"
if errorlevel 1 exit /b 1

echo [CC] fiber.c
cl /nologo /W4 /Od /Zi /I "%SRCDIR%" /c /Fo"%BUILDDIR%\fiber.obj" "%ROOTDIR%\fiber.c"
if errorlevel 1 exit /b 1

echo [CC] scheduler.c
cl /nologo /W4 /Od /Zi /I "%SRCDIR%" /c /Fo"%BUILDDIR%\scheduler.obj" "%ROOTDIR%\scheduler.c"
if errorlevel 1 exit /b 1

echo [CC] fiber_test.c
cl /nologo /W4 /Od /Zi /I "%SRCDIR%" /c /Fo"%BUILDDIR%\fiber_test.obj" "%ROOTDIR%\fiber_test.c"
if errorlevel 1 exit /b 1

echo [LINK] fiber_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\fiber_test.exe" "%BUILDDIR%\fiber_test.obj" "%BUILDDIR%\fiber.obj" "%BUILDDIR%\fiber_trampoline.obj" "%BUILDDIR%\arm64_ctx.obj" "%BUILDDIR%\scheduler.obj"
if errorlevel 1 exit /b 1

echo fiber_test.exe built successfully.
exit /b 0

:build_cont_test
echo [AS] cont_trampoline.asm
armasm64 /nologo -o "%BUILDDIR%\cont_trampoline.obj" "%SRCDIR%\cont_trampoline.asm"
if errorlevel 1 exit /b 1

echo [CC] cont.c
cl /nologo /W4 /Od /Zi /I "%SRCDIR%" /c /Fo"%BUILDDIR%\cont.obj" "%ROOTDIR%\cont.c"
if errorlevel 1 exit /b 1

echo [CC] cont_test.c
cl /nologo /W4 /Od /Zi /I "%SRCDIR%" /c /Fo"%BUILDDIR%\cont_test.obj" "%ROOTDIR%\cont_test.c"
if errorlevel 1 exit /b 1

echo [LINK] cont_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\cont_test.exe" "%BUILDDIR%\cont_test.obj" "%BUILDDIR%\cont.obj" "%BUILDDIR%\arm64_ctx.obj" "%BUILDDIR%\cont_trampoline.obj"
if errorlevel 1 exit /b 1

echo cont_test.exe built successfully.
exit /b 0

:build_cont_clone_test
echo [CC] cont_clone_test.c
cl /nologo /W4 /Od /Zi /I "%SRCDIR%" /c /Fo"%BUILDDIR%\cont_clone_test.obj" "%ROOTDIR%\cont_clone_test.c"
if errorlevel 1 exit /b 1

echo [LINK] cont_clone_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\cont_clone_test.exe" "%BUILDDIR%\cont_clone_test.obj" "%BUILDDIR%\cont.obj" "%BUILDDIR%\arm64_ctx.obj" "%BUILDDIR%\cont_trampoline.obj"
if errorlevel 1 exit /b 1

echo cont_clone_test.exe built successfully.
exit /b 0

:build_eff_test
echo [CC] eff.c
cl /nologo /W4 /Od /Zi /I "%SRCDIR%" /c /Fo"%BUILDDIR%\eff.obj" "%ROOTDIR%\eff.c"
if errorlevel 1 exit /b 1

echo [CC] eff_test.c
cl /nologo /W4 /Od /Zi /I "%SRCDIR%" /c /Fo"%BUILDDIR%\eff_test.obj" "%ROOTDIR%\eff_test.c"
if errorlevel 1 exit /b 1

echo [LINK] eff_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\eff_test.exe" "%BUILDDIR%\eff_test.obj" "%BUILDDIR%\eff.obj" "%BUILDDIR%\cont.obj" "%BUILDDIR%\arm64_ctx.obj" "%BUILDDIR%\cont_trampoline.obj"
if errorlevel 1 exit /b 1

echo eff_test.exe built successfully.
exit /b 0

:end
echo Done.
endlocal
