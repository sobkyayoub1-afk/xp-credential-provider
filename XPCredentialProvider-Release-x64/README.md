# XP Style Windows Logon UI

A Windows Credential Provider that overlays the default Windows 10/11 logon screen with a classic Windows XP-style logon interface.

## Features

- **XP-style UI**: Classic blue background with traditional logon interface
- **Full-screen overlay**: Covers the default Windows logon screen
- **Standard authentication**: Uses Windows authentication APIs for secure logon
- **Windows 10/11 compatible**: Works on modern Windows versions
- **Domain support**: Supports both local and domain authentication

## Architecture

The project consists of several key components:

### Credential Provider
- `XPCredentialProvider.cpp/h`: Main credential provider implementation
- Integrates with Windows authentication system
- Handles credential enumeration and serialization

### XP-style UI
- `XPLogonUI.cpp/h`: Classic XP-style logon interface
- Full-screen overlay with semi-transparent background
- Traditional username/password/domain input fields

### COM Registration
- `dllmain.cpp`: DLL entry point and COM registration
- Registers the credential provider with Windows

## Building

### Prerequisites
- Visual Studio 2019 or later with C++ desktop development workload
- Windows 10 SDK
- Administrator privileges for registration

### Build Steps
1. Open `XPCredentialProvider.vcxproj` in Visual Studio
2. Build the solution in Release or Debug mode for x64
3. Register the DLL: `regsvr32 XPCredentialProvider.dll`

## Installation

### Automatic Installation
```batch
copy XPCredentialProvider.dll %SystemRoot%\System32\
regsvr32 %SystemRoot%\System32\XPCredentialProvider.dll
```

### Manual Registration
The credential provider must be registered in:
```
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers
```

## Usage

1. Install the credential provider
2. Restart Windows or lock the computer
3. The XP-style logon screen will appear instead of the default Windows logon UI
4. Enter your credentials and click OK to log in

## Security Notes

- This credential provider uses standard Windows authentication APIs
- Passwords are handled securely and not stored
- The provider runs in the secure Windows logon environment
- Requires administrator privileges for installation

## Troubleshooting

### Credential Provider Not Showing
- Verify the DLL is registered correctly
- Check Windows Event Viewer for authentication errors
- Ensure running as administrator during registration

### Build Errors
- Verify Windows SDK is installed
- Check that all required libraries are linked
- Ensure building for x64 architecture

### UI Issues
- Verify graphics drivers are up to date
- Check display scaling settings
- Test in different screen resolutions

## Development

### Adding Features
- Modify `XPLogonUI.cpp` for UI changes
- Update `XPCredentialProvider.cpp` for authentication logic
- Add new fields in the resource files

### Debugging
- Use Visual Studio debugger with administrator privileges
- Check Windows Event Viewer for credential provider errors
- Use `logman` to trace authentication events

## License

This project is provided as-is for educational purposes. Use at your own risk.

## Compatibility

- Windows 10 (1903 and later)
- Windows 11 (all versions)
- Requires x64 architecture
- Administrator privileges required for installation
