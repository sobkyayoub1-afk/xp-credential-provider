#pragma once

#include <windows.h>
#include <commctrl.h>

class XPLogonUI
{
public:
    XPLogonUI();
    ~XPLogonUI();

    bool Create(HWND hParentWnd);
    void Destroy();
    void Show();
    void Hide();
    bool IsVisible() const;

    // Get user input
    bool GetUsername(wchar_t* buffer, int bufferSize);
    bool GetPassword(wchar_t* buffer, int bufferSize);
    bool GetDomain(wchar_t* buffer, int bufferSize);

    // UI state
    void SetUsername(const wchar_t* username);
    void SetPassword(const wchar_t* password);
    void SetDomain(const wchar_t* domain);
    void SetError(const wchar_t* error);

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LoginWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    bool RegisterWindowClass();
    void CreateControls();
    void UpdateLayout();
    void DrawBackground(HDC hdc);
    void DrawClassicBorder(HDC hdc, RECT rect);
    
    HWND _hWnd;
    HWND _hLoginWnd;
    HWND _hUsernameEdit;
    HWND _hPasswordEdit;
    HWND _hDomainEdit;
    HWND _hLoginButton;
    HWND _hShutdownButton;
    HWND _hRestartButton;
    HWND _hUserIcon;
    HWND _hErrorLabel;
    
    HBITMAP _hBackgroundBitmap;
    HBITMAP _hUserBitmap;
    HFONT _hTitleFont;
    HFONT _hLabelFont;
    
    int _windowWidth;
    int _windowHeight;
    bool _isVisible;
    
    static const wchar_t* CLASS_NAME;
    static const wchar_t* LOGIN_CLASS_NAME;
};
