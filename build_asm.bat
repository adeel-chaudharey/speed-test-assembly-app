@echo off
REM ============================================================
REM  build_asm.bat  –  Assemble typing_analyzer.asm with NASM
REM  Run this from the project root before building the Qt app.
REM ============================================================

setlocal

echo [1/3] Assembling typing_analyzer.asm ...
nasm -f win64 asm\typing_analyzer.asm -o asm\typing_analyzer.obj
if errorlevel 1 (
    echo FAILED: NASM assembly error.
    exit /b 1
)

echo [2/3] Linking object file ...
REM Link with standard MinGW gcc (enables safe argc/argv initialization)
gcc -o asm\typing_analyzer.exe asm\typing_analyzer.obj -lkernel32 -Wl,--subsystem,console
if errorlevel 1 (
    echo FAILED: Linking error.
    exit /b 1
)

echo [3/3] Done!  asm\typing_analyzer.exe created successfully.
echo.
endlocal
