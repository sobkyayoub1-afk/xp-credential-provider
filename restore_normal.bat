@echo off
echo ========================================
echo Restore Normal Windows Shell
echo ========================================
echo.

REM Check if running as Administrator
net session >nul 2>&1
if %errorLevel% == 0 (
    echo [OK] Running as Administrator
) else (
    echo [ERROR] Must run as Administrator!
    echo Right-click this file and select "Run as administrator"
    pause
    exit /b 1
)

echo.
echo Step 1: Restoring normal Windows shell...
reg delete "HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Shell /f
if %errorLevel% == 0 (
    echo [OK] Normal shell restored
) else (
    echo [ERROR] Failed to restore shell
)

echo.
echo Step 2: Disabling auto-login...
reg delete "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v AutoAdminLogon /f
reg delete "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultUsername /f
reg delete "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultPassword /f
if %errorLevel% == 0 (
    echo [OK] Auto-login disabled
) else (
    echo [ERROR] Failed to disable auto-login
)

echo.
echo Step 3: Re-enabling UAC...
reg delete "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System" /v EnableLUA /f
if %errorLevel% == 0 (
    echo [OK] UAC re-enabled
) else (
    echo [WARNING] Failed to re-enable UAC
)

echo.
echo Step 4: Removing desktop shortcut...
if exist "%PUBLIC%\Desktop\XP Logon.lnk" (
    del "%PUBLIC%\Desktop\XP Logon.lnk"
    echo [OK] Desktop shortcut removed
)

echo.
echo ========================================
echo RESTORE COMPLETE!
echo ========================================
echo.
echo Normal Windows shell has been restored.
echo Please restart Windows to apply changes.
echo.
pause
