#include <windows.h>
#include <wincred.h>
#include <wtsapi32.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "credui.lib")

struct UserAccount {
    wchar_t username[256];
    wchar_t fullName[256];
    bool isAdmin;
};

std::vector<UserAccount> GetUserAccounts() {
    std::vector<UserAccount> users;
    
    // Simple hardcoded users for now - will fix API later
    UserAccount admin;
    wcscpy(admin.username, L"Administrator");
    wcscpy(admin.fullName, L"Administrator");
    admin.isAdmin = true;
    users.push_back(admin);
    
    // Try to get current user
    wchar_t currentUsername[256];
    DWORD size = 256;
    if (GetUserNameW(currentUsername, &size)) {
        UserAccount user;
        wcscpy(user.username, currentUsername);
        wcscpy(user.fullName, currentUsername);
        user.isAdmin = false;
        users.push_back(user);
    }
    
    return users;
}

class XPLogonSystem {
private:
    bool isAdminLoggedIn;
    HWND hwndLogon;
    std::vector<UserAccount> userAccounts;
    
public:
    XPLogonSystem() : isAdminLoggedIn(false), hwndLogon(NULL) {
        userAccounts = GetUserAccounts();
    }
    
    bool VerifyCredentials(const wchar_t* username, const wchar_t* password, const wchar_t* domain) {
        HANDLE hToken;
        BOOL result = LogonUser(
            username,
            domain,
            password,
            LOGON32_LOGON_INTERACTIVE,
            LOGON32_PROVIDER_DEFAULT,
            &hToken
        );
        
        if (result) {
            CloseHandle(hToken);
            return true;
        }
        return false;
    }
    
    void CreateRDPConnection(const wchar_t* username, const wchar_t* password, const wchar_t* domain) {
        wchar_t rdpCmd[512];
        swprintf(rdpCmd, 512, L"mstsc.exe /v:localhost /f /admin /u:\"%s\" /p:\"%s\"", username, password);
        
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        
        CreateProcess(
            NULL, rdpCmd, NULL, NULL, FALSE,
            CREATE_NO_WINDOW, NULL, NULL, &si, &pi
        );
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    
    bool WaitForRDPSession() {
        WTS_SESSION_INFO* pSessionInfo = NULL;
        DWORD dwCount = 0;
        
        while (true) {
            if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount)) {
                for (DWORD i = 0; i < dwCount; i++) {
                    if (pSessionInfo[i].State == WTSActive && pSessionInfo[i].SessionId != WTS_CURRENT_SESSION) {
                        WTSFreeMemory(pSessionInfo);
                        return true;
                    }
                }
                WTSFreeMemory(pSessionInfo);
            }
            Sleep(1000); // Check every second
        }
    }
    
    void ConnectToConsole(DWORD sessionId) {
        WTSConnectSession((ULONG)WTS_CURRENT_SERVER_HANDLE, sessionId, NULL, FALSE);
    }
    
    bool InputBox(HWND hwndParent, const wchar_t* title, const wchar_t* prompt, wchar_t* buffer, int bufferSize) {
        // Use Windows API for proper input dialog
        CREDUI_INFO_W cuiInfo = { sizeof(CREDUI_INFO_W) };
        cuiInfo.pszCaptionText = title;
        cuiInfo.pszMessageText = prompt;
        
        DWORD dwAuthError = 0;
        BOOL result = CredUIPromptForWindowsCredentialsW(
            hwndParent,
            &cuiInfo,
            0,
            NULL,
            CREDUIWIN_GENERIC,
            NULL,
            &dwAuthError,
            buffer,
            bufferSize,
            NULL,
            FALSE,
            NULL
        );
        
        return result == ERROR_SUCCESS;
    }
    
    void ShowXPLogonUI() {
        // Create full-screen XP-style window
        WNDCLASS wc = {0};
        wc.lpfnWndProc = LogonWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hbrBackground = CreateSolidBrush(RGB(58, 110, 165)); // XP blue gradient
        wc.lpszClassName = L"XPLogonClass";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        
        RegisterClass(&wc);
        
        // Get screen dimensions
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        
        hwndLogon = CreateWindowEx(
            WS_EX_TOPMOST,
            L"XPLogonClass",
            L"Windows XP Logon",
            WS_POPUP | WS_VISIBLE,
            0, 0, screenWidth, screenHeight,
            NULL, NULL, GetModuleHandle(NULL), this
        );
        
        // Create user account buttons dynamically based on actual users
        int buttonY = screenHeight / 2 - 50;
        for (size_t i = 0; i < userAccounts.size(); i++) {
            CreateWindowEx(0, L"BUTTON", userAccounts[i].fullName, 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT,
                (screenWidth/2) - 100, buttonY, 200, 40, 
                hwndLogon, (HMENU)(i + 1), GetModuleHandle(NULL), NULL);
            buttonY += 50;
        }
        
        ShowWindow(hwndLogon, SW_SHOW);
        UpdateWindow(hwndLogon);
        
        // Message loop
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            // Handle user button clicks
            if (msg.message == WM_COMMAND) {
                DWORD userId = LOWORD(msg.wParam) - 1;
                if (userId < userAccounts.size()) {
                    wchar_t password[256];
                    if (InputBox(hwndLogon, L"Enter Password", 
                        userAccounts[userId].fullName, password, 256)) {
                        if (VerifyCredentials(userAccounts[userId].username, password, L".")) {
                            CreateRDPConnection(userAccounts[userId].username, password, L".");
                            WaitForRDPSession();
                            ConnectToConsole(userId + 1); // Session IDs start from 1
                            break;
                        } else {
                            MessageBox(hwndLogon, L"Invalid password", L"Logon Message", MB_OK | MB_ICONERROR);
                        }
                    }
                }
            }
        }
    }
    
    static LRESULT CALLBACK LogonWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        XPLogonSystem* pThis = (XPLogonSystem*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        
        switch (msg) {
            case WM_CREATE:
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
                break;
                
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Get window dimensions
                RECT rect;
                GetClientRect(hwnd, &rect);
                
                // Create XP-style gradient background
                HBRUSH hBrush = CreateSolidBrush(RGB(58, 110, 165));
                FillRect(hdc, &rect, hBrush);
                
                // Add gradient effect
                RECT gradientRect = rect;
                gradientRect.bottom = rect.top + 200;
                HBRUSH hGradientBrush = CreateSolidBrush(RGB(0, 45, 114));
                FillRect(hdc, &gradientRect, hGradientBrush);
                
                // Set text properties
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkMode(hdc, TRANSPARENT);
                HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft Sans Serif");
                SelectObject(hdc, hFont);
                
                // Draw XP welcome text
                RECT textRect = rect;
                textRect.top = rect.bottom / 2 - 150;
                DrawText(hdc, L"To begin, click your user name", -1, &textRect, DT_CENTER | DT_SINGLELINE);
                
                // Draw Windows XP title with proper XP styling
                RECT titleRect = rect;
                titleRect.top = rect.top + 50;
                HFONT hTitleFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft Sans Serif");
                SelectObject(hdc, hTitleFont);
                
                // Add shadow effect for title
                SetTextColor(hdc, RGB(0, 0, 0));
                RECT shadowRect = titleRect;
                shadowRect.left += 2;
                shadowRect.top += 2;
                DrawText(hdc, L"Windows XP", -1, &shadowRect, DT_CENTER | DT_SINGLELINE);
                
                SetTextColor(hdc, RGB(255, 255, 255));
                DrawText(hdc, L"Windows XP", -1, &titleRect, DT_CENTER | DT_SINGLELINE);
                
                // Draw XP-style welcome message
                RECT welcomeRect = rect;
                welcomeRect.top = rect.top + 120;
                HFONT hWelcomeFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft Sans Serif");
                SelectObject(hdc, hWelcomeFont);
                DrawText(hdc, L"Welcome", -1, &welcomeRect, DT_CENTER | DT_SINGLELINE);
                
                // Clean up
                DeleteObject(hBrush);
                DeleteObject(hGradientBrush);
                DeleteObject(hFont);
                DeleteObject(hTitleFont);
                DeleteObject(hWelcomeFont);
                
                EndPaint(hwnd, &ps);
                return 0;
            }
            
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Check if we're running as Administrator
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        MessageBox(NULL, L"Failed to get process token", L"Error", MB_ICONERROR);
        return 1;
    }
    
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup = NULL;
    
    if (AllocateAndInitializeSid(&NtAuthority, SECURITY_NT_AUTHORITY,
                              DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, &adminGroup)) {
        BOOL isAdmin = FALSE;
        CheckTokenMembership(hToken, adminGroup, &isAdmin);
        FreeSid(adminGroup);
        CloseHandle(hToken);
        
        if (!isAdmin) {
            MessageBox(NULL, L"This program must be run as Administrator", L"Error", MB_ICONERROR);
            return 1;
        }
    } else {
        if (adminGroup) FreeSid(adminGroup);
        CloseHandle(hToken);
        MessageBox(NULL, L"Failed to initialize security", L"Error", MB_ICONERROR);
        return 1;
    }
    
    XPLogonSystem logonSystem;
    logonSystem.ShowXPLogonUI();
    
    return 0;
}
