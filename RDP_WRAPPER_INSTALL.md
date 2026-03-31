# RDP Wrapper Installation Required

## What This Approach Does

Instead of trying to replace the complex Windows LogonUI, this approach:

1. **Replaces the shell** - Sets your XP logon program as the default shell for Administrator account
2. **Uses RDP wrapper** - Patches Windows to allow multiple simultaneous RDP connections
3. **Authenticates via RDP** - Uses Windows' built-in authentication through local RDP
4. **Session switching** - Automatically switches between user sessions

## Step 1: Install RDP Wrapper

1. Download RDP Wrapper from: https://github.com/stascorp/rdpwrap/releases
2. Download `RDPWrap-v1.6.2.zip` (or latest)
3. Extract and run `install.bat` as Administrator
4. Verify installation with `RDPWInst.exe -c`

## Step 2: Build and Install XP Shell Replacer

1. Copy all files to your Windows VM
2. Run `setup_shell.bat` as Administrator
3. Log out and log back in as Administrator

## Step 3: Test the XP Logon

When you log in as Administrator:
- XP-style logon screen will appear
- Enter credentials for any user
- System will automatically create RDP session and switch to that user

## How It Works

```
Administrator logs in → XP Shell Replacer starts → User enters credentials → RDP session created → Session switched → Administrator logged out
```

## Benefits

✅ **Bypasses LogonUI completely** - No complex credential provider needed
✅ **Uses native Windows authentication** - Full compatibility
✅ **Real user switching** - Not just an overlay
✅ **XP-style UI** - Classic blue logon screen

## Troubleshooting

- If RDP wrapper fails: Make sure Windows is up to date
- If shell replacement fails: Check registry permissions
- If sessions don't switch: Verify RDP wrapper is working
