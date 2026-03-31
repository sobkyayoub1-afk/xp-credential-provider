#include "XPLogonUI.h"
#include <commctrl.h>
#include <shellapi.h>

const wchar_t* XPLogonUI::CLASS_NAME = L"XPLogonUI";
const wchar_t* XPLogonUI::LOGIN_CLASS_NAME = L"XPLoginWindow";

XPLogonUI::XPLogonUI() :
    _hWnd(NULL),
    _hLoginWnd(NULL),
    _hUsernameEdit(NULL),
    _hPasswordEdit(NULL),
    _hDomainEdit(NULL),
    _hLoginButton(NULL),
    _hShutdownButton(NULL),
    _hRestartButton(NULL),
    _hUserIcon(NULL),
    _hErrorLabel(NULL),
    _hBackgroundBitmap(NULL),
    _hUserBitmap(NULL),
    _hTitleFont(NULL),
    _hLabelFont(NULL),
    _windowWidth(400),
    _windowHeight(300),
    _isVisible(false)
{
    // Create fonts
    _hTitleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Tahoma");
    
    _hLabelFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Tahoma");
}

XPLogonUI::~XPLogonUI()
{
    Destroy();
    
    if (_hTitleFont) DeleteObject(_hTitleFont);
    if (_hLabelFont) DeleteObject(_hLabelFont);
    if (_hBackgroundBitmap) DeleteObject(_hBackgroundBitmap);
    if (_hUserBitmap) DeleteObject(_hUserBitmap);
}

bool XPLogonUI::Create(HWND hParentWnd)
{
    if (!RegisterWindowClass())
        return false;
    
    // Create full-screen overlay window
    _hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED,
                           CLASS_NAME,
                           L"XP Logon",
                           WS_POPUP | WS_VISIBLE,
                           0, 0,
                           GetSystemMetrics(SM_CXSCREEN),
                           GetSystemMetrics(SM_CYSCREEN),
                           hParentWnd,
                           NULL,
                           GetModuleHandle(NULL),
                           this);
    
    if (!_hWnd)
        return false;
    
    // Set layered window attributes for transparency
    SetLayeredWindowAttributes(_hWnd, RGB(0, 0, 0), 240, LWA_ALPHA);
    
    // Create login window (centered)
    int x = (GetSystemMetrics(SM_CXSCREEN) - _windowWidth) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - _windowHeight) / 2;
    
    _hLoginWnd = CreateWindowEx(0,
                                LOGIN_CLASS_NAME,
                                L"Welcome",
                                WS_POPUP | WS_BORDER | WS_CAPTION,
                                x, y,
                                _windowWidth, _windowHeight,
                                _hWnd,
                                NULL,
                                GetModuleHandle(NULL),
                                this);
    
    if (!_hLoginWnd)
    {
        DestroyWindow(_hWnd);
        return false;
    }
    
    CreateControls();
    UpdateLayout();
    
    return true;
}

void XPLogonUI::Destroy()
{
    if (_hLoginWnd)
    {
        DestroyWindow(_hLoginWnd);
        _hLoginWnd = NULL;
    }
    
    if (_hWnd)
    {
        DestroyWindow(_hWnd);
        _hWnd = NULL;
    }
}

void XPLogonUI::Show()
{
    if (_hWnd && _hLoginWnd)
    {
        ShowWindow(_hWnd, SW_SHOW);
        ShowWindow(_hLoginWnd, SW_SHOW);
        SetForegroundWindow(_hLoginWnd);
        _isVisible = true;
    }
}

void XPLogonUI::Hide()
{
    if (_hWnd && _hLoginWnd)
    {
        ShowWindow(_hLoginWnd, SW_HIDE);
        ShowWindow(_hWnd, SW_HIDE);
        _isVisible = false;
    }
}

bool XPLogonUI::IsVisible() const
{
    return _isVisible;
}

bool XPLogonUI::RegisterWindowClass()
{
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hbrBackground = CreateSolidBrush(RGB(58, 110, 165)); // XP blue
    wc.lpszClassName = CLASS_NAME;
    
    if (!RegisterClassEx(&wc))
        return false;
    
    // Register login window class
    wc.lpfnWndProc = LoginWndProc;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = LOGIN_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    
    return RegisterClassEx(&wc) != 0;
}

void XPLogonUI::CreateControls()
{
    // User icon
    _hUserIcon = CreateWindow(L"STATIC", L"",
                              WS_CHILD | WS_VISIBLE | SS_BITMAP,
                              20, 20, 48, 48,
                              _hLoginWnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Username label and edit
    CreateWindow(L"STATIC", L"User name:",
                 WS_CHILD | WS_VISIBLE,
                 80, 20, 100, 20,
                 _hLoginWnd, NULL, GetModuleHandle(NULL), NULL);
    
    _hUsernameEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                    80, 40, 200, 20,
                                    _hLoginWnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Password label and edit
    CreateWindow(L"STATIC", L"Password:",
                 WS_CHILD | WS_VISIBLE,
                 80, 70, 100, 20,
                 _hLoginWnd, NULL, GetModuleHandle(NULL), NULL);
    
    _hPasswordEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
                                    80, 90, 200, 20,
                                    _hLoginWnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Domain label and edit (optional)
    CreateWindow(L"STATIC", L"Log on to:",
                 WS_CHILD | WS_VISIBLE,
                 80, 120, 100, 20,
                 _hLoginWnd, NULL, GetModuleHandle(NULL), NULL);
    
    _hDomainEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"THIS COMPUTER",
                                  WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                  80, 140, 200, 20,
                                  _hLoginWnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Login button
    _hLoginButton = CreateWindow(L"BUTTON", L"OK",
                                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                290, 40, 80, 25,
                                _hLoginWnd, (HMENU)1, GetModuleHandle(NULL), NULL);
    
    // Cancel button
    CreateWindow(L"BUTTON", L"Cancel",
                 WS_CHILD | WS_VISIBLE,
                 290, 70, 80, 25,
                 _hLoginWnd, (HMENU)2, GetModuleHandle(NULL), NULL);
    
    // Error label (initially hidden)
    _hErrorLabel = CreateWindow(L"STATIC", L"",
                               WS_CHILD | SS_LEFT,
                               20, 180, 350, 20,
                               _hLoginWnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Set fonts
    SendMessage(_hUsernameEdit, WM_SETFONT, (WPARAM)_hLabelFont, TRUE);
    SendMessage(_hPasswordEdit, WM_SETFONT, (WPARAM)_hLabelFont, TRUE);
    SendMessage(_hDomainEdit, WM_SETFONT, (WPARAM)_hLabelFont, TRUE);
    SendMessage(_hLoginButton, WM_SETFONT, (WPARAM)_hLabelFont, TRUE);
}

void XPLogonUI::UpdateLayout()
{
    // Layout is handled in CreateControls
}

bool XPLogonUI::GetUsername(wchar_t* buffer, int bufferSize)
{
    if (!_hUsernameEdit)
        return false;
    
    return GetWindowText(_hUsernameEdit, buffer, bufferSize) > 0;
}

bool XPLogonUI::GetPassword(wchar_t* buffer, int bufferSize)
{
    if (!_hPasswordEdit)
        return false;
    
    return GetWindowText(_hPasswordEdit, buffer, bufferSize) > 0;
}

bool XPLogonUI::GetDomain(wchar_t* buffer, int bufferSize)
{
    if (!_hDomainEdit)
        return false;
    
    return GetWindowText(_hDomainEdit, buffer, bufferSize) > 0;
}

void XPLogonUI::SetUsername(const wchar_t* username)
{
    if (_hUsernameEdit)
        SetWindowText(_hUsernameEdit, username);
}

void XPLogonUI::SetPassword(const wchar_t* password)
{
    if (_hPasswordEdit)
        SetWindowText(_hPasswordEdit, password);
}

void XPLogonUI::SetDomain(const wchar_t* domain)
{
    if (_hDomainEdit)
        SetWindowText(_hDomainEdit, domain);
}

void XPLogonUI::SetError(const wchar_t* error)
{
    if (_hErrorLabel)
    {
        SetWindowText(_hErrorLabel, error);
        ShowWindow(_hErrorLabel, error ? SW_SHOW : SW_HIDE);
    }
}

LRESULT CALLBACK XPLogonUI::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    XPLogonUI* pThis = NULL;
    
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (XPLogonUI*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else
    {
        pThis = (XPLogonUI*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }
    
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            // Draw semi-transparent background
            RECT rect;
            GetClientRect(hWnd, &rect);
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 128));
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);
            
            EndPaint(hWnd, &ps);
        }
        break;
        
    case WM_ERASEBKGND:
        return 1;
        
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    
    return 0;
}

LRESULT CALLBACK XPLogonUI::LoginWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    XPLogonUI* pThis = NULL;
    
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (XPLogonUI*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else
    {
        pThis = (XPLogonUI*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }
    
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case 1: // OK button
                if (pThis)
                {
                    // Validate input
                    wchar_t username[256];
                    wchar_t password[256];
                    
                    if (pThis->GetUsername(username, 256) && wcslen(username) > 0 &&
                        pThis->GetPassword(password, 256) && wcslen(password) > 0)
                    {
                        // Send message to indicate login attempt
                        PostMessage(GetParent(hWnd), WM_USER + 100, 0, 0);
                    }
                    else
                    {
                        pThis->SetError(L"Please enter both username and password");
                    }
                }
                break;
                
            case 2: // Cancel button
                PostQuitMessage(0);
                break;
            }
        }
        break;
        
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            // Draw XP-style title bar
            RECT rect;
            GetClientRect(hWnd, &rect);
            rect.bottom = 30;
            
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 128));
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);
            
            // Draw title text
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, pThis ? pThis->_hTitleFont : GetStockObject(DEFAULT_GUI_FONT));
            DrawText(hdc, L"Log On to Windows", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
            EndPaint(hWnd, &ps);
        }
        break;
        
    case WM_DESTROY:
        if (pThis)
        {
            pThis->_hLoginWnd = NULL;
        }
        break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    
    return 0;
}
