#include <windows.h>
#include <credentialprovider.h>
#include "XPCredentialProvider.h"

// Global variables
HINSTANCE g_hInstance = NULL;
LONG g_cDllRef = 0;

// DLL reference counting functions
void DllAddRef()
{
    InterlockedIncrement(&g_cDllRef);
}

void DllRelease()
{
    InterlockedDecrement(&g_cDllRef);
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// Standard COM exports
STDAPI DllCanUnloadNow()
{
    return g_cDllRef == 0 ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;
    
    if (ppv == NULL)
        return E_POINTER;
    
    *ppv = NULL;
    
    if (rclsid == CLSID_XPCredentialProvider)
    {
        XPCredentialProvider* pProvider = new XPCredentialProvider();
        if (pProvider)
        {
            hr = pProvider->QueryInterface(riid, ppv);
            pProvider->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }
    
    return hr;
}

STDAPI DllRegisterServer()
{
    HRESULT hr;
    HKEY hKey = NULL;
    HKEY hSubKey = NULL;
    wchar_t szModulePath[MAX_PATH];
    
    // Get the path of this DLL
    if (!GetModuleFileName(g_hInstance, szModulePath, ARRAYSIZE(szModulePath)))
        return HRESULT_FROM_WIN32(GetLastError());
    
    // Create the CLSID key
    wchar_t szClsid[64];
    StringFromGUID2(CLSID_XPCredentialProvider, szClsid, ARRAYSIZE(szClsid));
    
    hr = HRESULT_FROM_WIN32(RegCreateKeyEx(HKEY_CLASSES_ROOT, L"CLSID", 0, NULL, 
                                          REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL));
    if (FAILED(hr)) goto cleanup;
    
    hr = HRESULT_FROM_WIN32(RegCreateKeyEx(hKey, szClsid, 0, NULL, 
                                          REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL));
    if (FAILED(hr)) goto cleanup;
    
    RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (LPBYTE)L"XP Credential Provider", 
                  sizeof(L"XP Credential Provider"));
    
    // Create InprocServer32 key
    RegCloseKey(hSubKey);
    hr = HRESULT_FROM_WIN32(RegCreateKeyEx(hKey, szClsid, 0, NULL, 
                                          REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL));
    if (FAILED(hr)) goto cleanup;
    
    HKEY hInprocKey;
    hr = HRESULT_FROM_WIN32(RegCreateKeyEx(hSubKey, L"InprocServer32", 0, NULL, 
                                          REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hInprocKey, NULL));
    if (FAILED(hr)) goto cleanup;
    
    RegSetValueEx(hInprocKey, NULL, 0, REG_SZ, (LPBYTE)szModulePath, 
                  (DWORD)((wcslen(szModulePath) + 1) * sizeof(wchar_t)));
    RegSetValueEx(hInprocKey, L"ThreadingModel", 0, REG_SZ, (LPBYTE)L"Apartment", 
                  sizeof(L"Apartment"));
    
    RegCloseKey(hInprocKey);
    
    // Register as credential provider
    RegCloseKey(hSubKey);
    RegCloseKey(hKey);
    
    hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers", 
        0, KEY_WRITE, &hKey));
    if (FAILED(hr)) goto cleanup;
    
    hr = HRESULT_FROM_WIN32(RegCreateKeyEx(hKey, szClsid, 0, NULL, 
                                          REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, NULL));
    if (FAILED(hr)) goto cleanup;
    
    RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (LPBYTE)L"XP Credential Provider", 
                  sizeof(L"XP Credential Provider"));
    
cleanup:
    if (hSubKey) RegCloseKey(hSubKey);
    if (hKey) RegCloseKey(hKey);
    
    return hr;
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;
    HKEY hKey = NULL;
    wchar_t szClsid[64];
    
    StringFromGUID2(CLSID_XPCredentialProvider, szClsid, ARRAYSIZE(szClsid));
    
    // Delete from credential providers
    hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers", 
        0, KEY_WRITE, &hKey));
    if (SUCCEEDED(hr))
    {
        RegDeleteKey(hKey, szClsid);
        RegCloseKey(hKey);
    }
    
    // Delete CLSID key
    hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_CLASSES_ROOT, L"CLSID", 0, KEY_WRITE, &hKey));
    if (SUCCEEDED(hr))
    {
        RegDeleteTree(hKey, szClsid);
        RegCloseKey(hKey);
    }
    
    return S_OK;
}
