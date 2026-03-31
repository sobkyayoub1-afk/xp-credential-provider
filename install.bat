@echo off
echo ========================================
echo XP Logon UI Installation
echo ========================================
echo.

REM Check for administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator
    echo Right-click script and select "Run as administrator"
    pause
    exit /b 1
)

echo.
echo Step 1: Installing Credential Provider...
copy XPCredentialProvider.dll %SystemRoot%\System32\
if %errorLevel% neq 0 (
    echo WARNING: Failed to copy credential provider DLL
) else (
    echo [OK] Credential provider DLL copied
)

regsvr32 /s %SystemRoot%\System32\XPCredentialProvider.dll
if %errorLevel% neq 0 (
    echo WARNING: Failed to register credential provider
) else (
    echo [OK] Credential provider registered
)

echo.
echo Step 2: Setting up Administrator account...
net user Administrator /active:yes
if %errorLevel% == 0 (
    echo [OK] Administrator account enabled
) else (
    echo [WARNING] Administrator account may already be enabled
)

net user Administrator Password123
if %errorLevel% == 0 (
    echo [OK] Administrator password set to: Password123
) else (
    echo [WARNING] Failed to set Administrator password (may already exist)
)

echo.
echo Step 3: Setting up auto-login for Administrator...
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v AutoAdminLogon /t REG_SZ /d "1" /f
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultUsername /t REG_SZ /d "Administrator" /f
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultPassword /t REG_SZ /d "Password123" /f
if %errorLevel% == 0 (
    echo [OK] Auto-login configured for Administrator
) else (
    echo [WARNING] Failed to configure auto-login
)

echo.
echo Step 4: Setting up XP Shell Replacer...
if exist XPShellReplacer.exe (
    reg add "HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Shell /t REG_SZ /d "%~dp0XPShellReplacer.exe" /f
    if %errorLevel% == 0 (
        echo [OK] XP Shell Replacer configured as default shell
    ) else (
        echo [WARNING] Failed to set XP Shell Replacer as shell
    )
) else (
    echo [WARNING] XPShellReplacer.exe not found - shell replacement skipped
)

echo.
echo Step 5: Disabling UAC for seamless experience...
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System" /v EnableLUA /t REG_DWORD /d 0 /f
if %errorLevel% == 0 (
    echo [OK] UAC disabled
) else (
    echo [WARNING] Failed to disable UAC
)

echo.
echo ========================================
echo INSTALLATION COMPLETE!
echo ========================================
echo.
echo What has been configured:
echo - Administrator account: enabled (password: Password123)
echo - Auto-login: enabled for Administrator
echo - Credential Provider: registered
echo - XP Shell Replacer: set as default shell
echo - UAC: disabled
echo.
echo NEXT STEPS:
echo 1. Install RDP Wrapper (see RDP_WRAPPER_INSTALL.md)
echo 2. Restart Windows
echo 3. XP Logon UI will appear automatically
echo.
echo To restore normal Windows:
echo - Run restore_normal.bat as administrator
echo.
pause
