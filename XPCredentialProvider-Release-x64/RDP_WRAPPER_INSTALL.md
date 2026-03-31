# Complete XP Logon Setup Guide

## Quick Setup (Recommended)

### Option 1: One-Click Setup
1. Copy all files to your Windows VM
2. Right-click `setup_complete.bat` → "Run as administrator"
3. Follow the prompts
4. Restart Windows

### Option 2: Manual Setup
1. **Enable Administrator Account:**
   ```batch
   net user Administrator /active:yes
   net user Administrator Password123
   ```

2. **Set Auto-Login:**
   ```batch
   reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v AutoAdminLogon /t REG_SZ /d "1" /f
   reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultUsername /t REG_SZ /d "Administrator" /f
   reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultPassword /t REG_SZ /d "Password123" /f
   ```

3. **Set XP Shell Replacer:**
   ```batch
   reg add "HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Shell /t REG_SZ /d "C:\Path\To\XPShellReplacer.exe" /f
   ```

## RDP Wrapper Installation

### Why RDP Wrapper is Needed
Windows doesn't allow multiple simultaneous RDP connections by default. RDP Wrapper patches this limitation.

### Installation Steps
1. Download RDP Wrapper: https://github.com/stascorp/rdpwrap/releases
2. Download `RDPWrap-v1.6.2.zip` (or latest)
3. Extract to a folder
4. Run `install.bat` as Administrator
5. Verify with: `RDPWInst.exe -c`

## What Happens After Setup

1. **Windows starts** → Auto-logs into Administrator
2. **Administrator session** → XP Shell Replacer launches
3. **XP Logon UI appears** → Full-screen classic XP interface
4. **User enters credentials** → RDP session created and switched
5. **User logged in** → Administrator session automatically logs out

## Features

✅ **Classic XP Blue Background** - Authenticistic Windows XP appearance
✅ **Full-Screen Overlay** - Covers entire desktop
✅ **Native Authentication** - Uses Windows' built-in RDP authentication
✅ **Session Switching** - Proper user session management
✅ **Auto-Configuration** - One-click setup

## Troubleshooting

### XP Logon Doesn't Appear
- Check if XPShellReplacer.exe is in the correct path
- Verify registry entry: `reg query "HKCU\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v Shell`
- Ensure Administrator account is enabled

### RDP Issues
- Verify RDP Wrapper is installed: `RDPWInst.exe -c`
- Check Windows Firewall settings
- Ensure Remote Desktop is enabled

### Restore Normal Shell
Run `restore_normal.bat` as Administrator to revert all changes.

## Security Notes

- Administrator password is set to: `Password123`
- UAC is disabled for seamless experience
- Auto-login is enabled for Administrator account
- All changes can be reverted with `restore_normal.bat`
