@echo off
echo ========================================
echo XP Logon UI Complete Setup
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
echo Step 1: Enabling Administrator account...
net user Administrator /active:yes
if %errorLevel% == 0 (
    echo [OK] Administrator account enabled
) else (
    echo [ERROR] Failed to enable Administrator account
)

echo.
echo Step 2: Setting Administrator password...
net user Administrator Password123
if %errorLevel% == 0 (
    echo [OK] Administrator password set to: Password123
) else (
    echo [ERROR] Failed to set Administrator password
)

echo.
echo Step 3: Enabling auto-login for Administrator...
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v AutoAdminLogon /t REG_SZ /d "1" /f
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultUsername /t REG_SZ /d "Administrator" /f
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultPassword /t REG_SZ /d "Password123" /f
if %errorLevel% == 0 (
    echo [OK] Auto-login enabled for Administrator
) else (
    echo [ERROR] Failed to enable auto-login
)

echo.
echo Step 4: Setting XP Shell Replacer as default shell...
reg add "HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Shell /t REG_SZ /d "%~dp0XPShellReplacer.exe" /f
if %errorLevel% == 0 (
    echo [OK] XP Shell Replacer set as default shell
) else (
    echo [ERROR] Failed to set shell
)

echo.
echo Step 5: Creating desktop shortcut...
powershell -Command "$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('%PUBLIC%\Desktop\XP Logon.lnk'); $Shortcut.TargetPath = '%~dp0XPShellReplacer.exe'; $Shortcut.Save()"

echo.
echo Step 6: Disabling UAC (for seamless experience)...
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System" /v EnableLUA /t REG_DWORD /d 0 /f
if %errorLevel% == 0 (
    echo [OK] UAC disabled
) else (
    echo [WARNING] Failed to disable UAC (may need manual intervention)
)

echo.
echo ========================================
echo SETUP COMPLETE!
echo ========================================
echo.
echo What has been configured:
echo - Administrator account enabled with password: Password123
echo - Auto-login enabled for Administrator account
echo - XP Shell Replacer set as default shell
echo - UAC disabled for seamless experience
echo - Desktop shortcut created
echo.
echo NEXT STEPS:
echo 1. Install RDP Wrapper (see RDP_WRAPPER_INSTALL.md)
echo 2. Restart Windows
echo 3. XP Logon UI will appear automatically
echo.
echo To restore normal shell:
echo - Run: reg delete "HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Shell /f
echo.
pause
