#include <windows.h>
#include <wincred.h>
#include <wtsapi32.h>
#include <stdio.h>
#include <tchar.h>

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "credui.lib")

bool InputBox(HWND hwndParent, const wchar_t* title, const wchar_t* prompt, wchar_t* buffer, int bufferSize) {
    // Simple input dialog implementation
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    // Create dialog template
    struct InputData {
        wchar_t buffer[256];
        bool result;
    } inputData;
    
    wcscpy_s(inputData.buffer, 256, L"");
    inputData.result = false;
    
    // Create a simple input dialog
    HWND hwndDlg = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"DIALOG",
        title,
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 150,
        hwndParent, NULL, hInstance, NULL
    );
    
    // Create controls
    CreateWindowEx(0, L"STATIC", prompt, WS_CHILD | WS_VISIBLE,
        10, 10, 280, 20, hwndDlg, NULL, hInstance, NULL);
    
    HWND hwndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER,
        10, 40, 280, 25, hwndDlg, NULL, hInstance, NULL);
    
    CreateWindowEx(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        110, 80, 80, 25, hwndDlg, (HMENU)1, hInstance, NULL);
    
    CreateWindowEx(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        200, 80, 80, 25, hwndDlg, (HMENU)2, hInstance, NULL);
    
    ShowWindow(hwndDlg, SW_SHOW);
    UpdateWindow(hwndDlg);
    
    // Message loop for dialog
    MSG msg;
    bool dialogResult = false;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        if (msg.message == WM_COMMAND) {
            if (LOWORD(msg.wParam) == 1) { // OK
                GetWindowText(hwndEdit, inputData.buffer, 256);
                dialogResult = true;
                break;
            } else if (LOWORD(msg.wParam) == 2) { // Cancel
                dialogResult = false;
                break;
            }
        }
    }
    
    DestroyWindow(hwndDlg);
    
    if (dialogResult) {
        wcscpy_s(buffer, bufferSize, inputData.buffer);
        return true;
    }
    
    return false;
}

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
        
        // Create user account buttons (XP style)
        CreateWindowEx(0, L"BUTTON", L"Administrator", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT,
            (screenWidth/2) - 100, (screenHeight/2) - 50, 200, 40, hwndLogon, (HMENU)1, GetModuleHandle(NULL), NULL);
        
        CreateWindowEx(0, L"BUTTON", L"User", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT,
            (screenWidth/2) - 100, (screenHeight/2) + 10, 200, 40, hwndLogon, (HMENU)2, GetModuleHandle(NULL), NULL);
        
        ShowWindow(hwndLogon, SW_SHOW);
        UpdateWindow(hwndLogon);
        
        // Message loop
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            // Handle user button clicks
            if (msg.message == WM_COMMAND) {
                if (LOWORD(msg.wParam) == 1) {
                    // Administrator clicked - show password dialog
                    wchar_t password[256];
                    if (InputBox(hwndLogon, L"Enter Administrator Password", L"Password:", password, 256)) {
                        if (VerifyCredentials(L"Administrator", password, L".")) {
                            CreateRDPConnection(L"Administrator", password, L".");
                            WaitForRDPSession();
                            ConnectToConsole(1); // Connect to session 1
                            break;
                        } else {
                            MessageBox(hwndLogon, L"Invalid password", L"Logon Message", MB_OK | MB_ICONERROR);
                        }
                    }
                } else if (LOWORD(msg.wParam) == 2) {
                    // User clicked - show password dialog
                    wchar_t username[256];
                    wchar_t password[256];
                    if (InputBox(hwndLogon, L"Enter Username", L"Username:", username, 256)) {
                        if (InputBox(hwndLogon, L"Enter Password", L"Password:", password, 256)) {
                            if (VerifyCredentials(username, password, L".")) {
                                CreateRDPConnection(username, password, L".");
                                WaitForRDPSession();
                                ConnectToConsole(2); // Connect to session 2
                                break;
                            } else {
                                MessageBox(hwndLogon, L"Invalid credentials", L"Logon Message", MB_OK | MB_ICONERROR);
                            }
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
                HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 128));
                FillRect(hdc, &rect, hBrush);
                
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
                
                // Draw Windows XP title
                RECT titleRect = rect;
                titleRect.top = rect.top + 50;
                HFONT hTitleFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft Sans Serif");
                SelectObject(hdc, hTitleFont);
                DrawText(hdc, L"Windows XP", -1, &titleRect, DT_CENTER | DT_SINGLELINE);
                
                // Clean up
                DeleteObject(hBrush);
                DeleteObject(hFont);
                DeleteObject(hTitleFont);
                
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
