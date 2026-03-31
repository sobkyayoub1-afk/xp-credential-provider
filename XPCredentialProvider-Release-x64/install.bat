@echo off
echo Installing XP Style Credential Provider...
echo.

REM Check for administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator
    echo Right-click the script and select "Run as administrator"
    pause
    exit /b 1
)

REM Copy DLL to system directory
echo Copying XPCredentialProvider.dll to System32...
copy XPCredentialProvider.dll %SystemRoot%\System32\
if %errorLevel% neq 0 (
    echo ERROR: Failed to copy DLL to System32
    pause
    exit /b 1
)

REM Register the credential provider
echo Registering credential provider...
regsvr32 /s %SystemRoot%\System32\XPCredentialProvider.dll
if %errorLevel% neq 0 (
    echo ERROR: Failed to register credential provider
    pause
    exit /b 1
)

echo.
echo Installation completed successfully!
echo.
echo The XP-style logon UI will appear when you:
echo 1. Lock your computer (Win+L)
echo 2. Restart Windows
echo 3. Switch users
echo.
echo To uninstall, run uninstall.bat as administrator
pause
