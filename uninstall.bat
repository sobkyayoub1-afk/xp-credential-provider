@echo off
echo Uninstalling XP Style Credential Provider...
echo.

REM Check for administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator
    echo Right-click the script and select "Run as administrator"
    pause
    exit /b 1
)

REM Unregister the credential provider
echo Unregistering credential provider...
regsvr32 /s /u %SystemRoot%\System32\XPCredentialProvider.dll
if %errorLevel% neq 0 (
    echo WARNING: Failed to unregister credential provider (may not be registered)
)

REM Remove DLL from system directory
echo Removing XPCredentialProvider.dll from System32...
del /f /q "%SystemRoot%\System32\XPCredentialProvider.dll"
if %errorLevel% neq 0 (
    echo WARNING: Failed to remove DLL from System32
)

REM Clean up registry entries
echo Cleaning up registry entries...
reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{12345678-1234-1234-1234-123456789ABC}" /f 2>nul
reg delete "HKCR\CLSID\{12345678-1234-1234-1234-123456789ABC}" /f 2>nul

echo.
echo Uninstallation completed!
echo.
echo You may need to restart Windows for changes to take effect
pause
