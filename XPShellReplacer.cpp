#include <windows.h>
#include <wincred.h>
#include <wtsapi32.h>
#include <stdio.h>
#include <tchar.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "credui.lib")

class XPLogonSystem {
private:
    bool isAdminLoggedIn;
    HWND hwndLogon;
    
public:
    XPLogonSystem() : isAdminLoggedIn(false), hwndLogon(NULL) {}
    
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
        swprintf_s(rdpCmd, L"mstsc.exe /v:localhost /f /admin /u:\"%s\" /p:\"%s\"", username, password);
        
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
                    if (pSessionInfo[i].State == WTSActive) {
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
        // Correct WTSConnectSession signature - cast server handle to ULONG
        WTSConnectSession((ULONG)WTS_CURRENT_SERVER_HANDLE, sessionId, NULL, FALSE);
    }
    
    void ShowXPLogonUI() {
        // Create full-screen XP-style window
        WNDCLASS wc = {0};
        wc.lpfnWndProc = LogonWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 128)); // XP blue
        wc.lpszClassName = L"XPLogonClass";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        
        RegisterClass(&wc);
        
        hwndLogon = CreateWindowEx(
            WS_EX_TOPMOST,
            L"XPLogonClass",
            L"Windows XP Logon",
            WS_POPUP | WS_VISIBLE,
            0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
            NULL, NULL, GetModuleHandle(NULL), this
        );
        
        // Create input fields
        CreateWindowEx(0, L"EDIT", L"Username", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            400, 200, 200, 25, hwndLogon, (HMENU)1, GetModuleHandle(NULL), NULL);
        
        CreateWindowEx(0, L"EDIT", L"Password", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD,
            400, 240, 200, 25, hwndLogon, (HMENU)2, GetModuleHandle(NULL), NULL);
        
        CreateWindowEx(0, L"BUTTON", L"Log On", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            400, 280, 100, 30, hwndLogon, (HMENU)3, GetModuleHandle(NULL), NULL);
        
        ShowWindow(hwndLogon, SW_SHOW);
        UpdateWindow(hwndLogon);
        
        // Message loop
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            // Handle logon button click
            if (msg.message == WM_COMMAND && LOWORD(msg.wParam) == 3) {
                wchar_t username[256];
                wchar_t password[256];
                
                GetWindowText(GetDlgItem(hwndLogon, 1), username, 256);
                GetWindowText(GetDlgItem(hwndLogon, 2), password, 256);
                
                if (VerifyCredentials(username, password, L".")) {
                    CreateRDPConnection(username, password, L".");
                    WaitForRDPSession();
                    // TODO: Connect to actual session and logout admin
                    break;
                } else {
                    MessageBox(hwndLogon, L"Invalid credentials", L"Error", MB_OK | MB_ICONERROR);
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
                
                // Draw XP-style background
                RECT rect;
                GetClientRect(hwnd, &rect);
                FillRect(hdc, &rect, CreateSolidBrush(RGB(0, 0, 128)));
                
                // Draw welcome text
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkMode(hdc, TRANSPARENT);
                DrawText(hdc, L"Welcome to Windows XP", -1, &rect, DT_CENTER | DT_TOP | DT_SINGLELINE);
                
                // Draw input field labels
                TextOut(hdc, 320, 180, L"Username:", 9);
                TextOut(hdc, 320, 220, L"Password:", 9);
                
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
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup;
    
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                              DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    if (!isAdmin) {
        MessageBox(NULL, L"This program must be run as Administrator", L"Error", MB_ICONERROR);
        return 1;
    }
    
    XPLogonSystem logonSystem;
    logonSystem.ShowXPLogonUI();
    
    return 0;
}
