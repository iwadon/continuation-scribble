@echo off
REM Build script for Windows x64 (MSVC)
REM
REM This script must be run from "x64 Native Tools Command Prompt for VS"
REM or use the vcvars64.bat to set up the environment first.
REM
REM Usage:
REM   build.bat          - Build x86_64_ctx_test.exe (default)
REM   build.bat all      - Build all tests
REM   build.bat clean    - Clean build artifacts

setlocal

set BUILDDIR=..\build\x86_64-win

if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"

if "%1"=="clean" goto clean
if "%1"=="all" goto all

:default
echo Building x86_64_ctx_test.exe...
call :build_ctx_test
goto end

:all
echo Building all tests...
call :build_ctx_test
call :build_fiber_test
call :build_cont_test
call :build_cont_clone_test
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
echo [AS] x86_64_ctx.asm
ml64 /nologo /c /Zi /Fo"%BUILDDIR%\x86_64_ctx.obj" x86_64_ctx.asm
if errorlevel 1 exit /b 1

echo [CC] x86_64_ctx_test.c
cl /nologo /W4 /Od /Zi /I . /c /Fo"%BUILDDIR%\x86_64_ctx_test.obj" x86_64_ctx_test.c
if errorlevel 1 exit /b 1

echo [LINK] x86_64_ctx_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\x86_64_ctx_test.exe" "%BUILDDIR%\x86_64_ctx_test.obj" "%BUILDDIR%\x86_64_ctx.obj"
if errorlevel 1 exit /b 1

echo x86_64_ctx_test.exe built successfully.
exit /b 0

:build_fiber_test
echo [AS] fiber_trampoline.asm
ml64 /nologo /c /Zi /Fo"%BUILDDIR%\fiber_trampoline.obj" fiber_trampoline.asm
if errorlevel 1 exit /b 1

echo [CC] fiber.c
cl /nologo /W4 /Od /Zi /I . /c /Fo"%BUILDDIR%\fiber.obj" ..\fiber.c
if errorlevel 1 exit /b 1

echo [CC] scheduler.c
cl /nologo /W4 /Od /Zi /I . /c /Fo"%BUILDDIR%\scheduler.obj" ..\scheduler.c
if errorlevel 1 exit /b 1

echo [CC] fiber_test.c
cl /nologo /W4 /Od /Zi /I . /c /Fo"%BUILDDIR%\fiber_test.obj" ..\fiber_test.c
if errorlevel 1 exit /b 1

echo [LINK] fiber_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\fiber_test.exe" "%BUILDDIR%\fiber_test.obj" "%BUILDDIR%\fiber.obj" "%BUILDDIR%\fiber_trampoline.obj" "%BUILDDIR%\x86_64_ctx.obj" "%BUILDDIR%\scheduler.obj"
if errorlevel 1 exit /b 1

echo fiber_test.exe built successfully.
exit /b 0

:build_cont_test
echo [AS] cont_trampoline.asm
ml64 /nologo /c /Zi /Fo"%BUILDDIR%\cont_trampoline.obj" cont_trampoline.asm
if errorlevel 1 exit /b 1

echo [CC] cont.c
cl /nologo /W4 /Od /Zi /I . /c /Fo"%BUILDDIR%\cont.obj" ..\cont.c
if errorlevel 1 exit /b 1

echo [CC] cont_test.c
cl /nologo /W4 /Od /Zi /I . /c /Fo"%BUILDDIR%\cont_test.obj" ..\cont_test.c
if errorlevel 1 exit /b 1

echo [LINK] cont_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\cont_test.exe" "%BUILDDIR%\cont_test.obj" "%BUILDDIR%\cont.obj" "%BUILDDIR%\x86_64_ctx.obj" "%BUILDDIR%\cont_trampoline.obj"
if errorlevel 1 exit /b 1

echo cont_test.exe built successfully.
exit /b 0

:build_cont_clone_test
echo [CC] cont_clone_test.c
cl /nologo /W4 /Od /Zi /I . /c /Fo"%BUILDDIR%\cont_clone_test.obj" ..\cont_clone_test.c
if errorlevel 1 exit /b 1

echo [LINK] cont_clone_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\cont_clone_test.exe" "%BUILDDIR%\cont_clone_test.obj" "%BUILDDIR%\cont.obj" "%BUILDDIR%\x86_64_ctx.obj" "%BUILDDIR%\cont_trampoline.obj"
if errorlevel 1 exit /b 1

echo cont_clone_test.exe built successfully.
exit /b 0

:build_eff_test
echo [CC] eff.c
cl /nologo /W4 /Od /Zi /I . /c /Fo"%BUILDDIR%\eff.obj" ..\eff.c
if errorlevel 1 exit /b 1

echo [CC] eff_test.c
cl /nologo /W4 /Od /Zi /I . /c /Fo"%BUILDDIR%\eff_test.obj" ..\eff_test.c
if errorlevel 1 exit /b 1

echo [LINK] eff_test.exe
link /nologo /DEBUG /OUT:"%BUILDDIR%\eff_test.exe" "%BUILDDIR%\eff_test.obj" "%BUILDDIR%\eff.obj" "%BUILDDIR%\cont.obj" "%BUILDDIR%\x86_64_ctx.obj" "%BUILDDIR%\cont_trampoline.obj"
if errorlevel 1 exit /b 1

echo eff_test.exe built successfully.
exit /b 0

:end
echo Done.
endlocal
