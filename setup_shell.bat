@echo off
echo Setting up XP Logon Shell Replacement...
echo.

REM Check if running as Administrator
net session >nul 2>&1
if %errorLevel% == 0 (
    echo Running as Administrator - Good!
) else (
    echo ERROR: Must run as Administrator!
    pause
    exit /b 1
)

REM Build the shell replacer
echo Building XP Shell Replacer...
cl /EHsc XPShellReplacer.cpp user32.lib gdi32.lib advapi32.lib wtsapi32.lib credui.lib /Fe:XPShellReplacer.exe

if %errorLevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Setting XP Shell Replacer as default shell...
reg add "HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Shell /t REG_SZ /d "%CD%\XPShellReplacer.exe" /f

echo.
echo Setup complete!
echo.
echo IMPORTANT: 
echo 1. Make sure RDP Wrapper is installed for multiple RDP sessions
echo 2. Log out and back in as Administrator to activate
echo 3. The XP logon screen will appear automatically
echo.
pause
