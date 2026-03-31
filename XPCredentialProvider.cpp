#include "XPCredentialProvider.h"
#include "XPLogonUI.h"
#include <ntsecapi.h>
#include <wincred.h>

XPCredentialProvider::XPCredentialProvider() :
    _cRef(1),
    _cpus(CPUS_INVALID),
    _pcpe(NULL),
    _upAdviseContext(0),
    _fRecreateEnumeratedCredentials(false)
{
    DllAddRef();
}

XPCredentialProvider::~XPCredentialProvider()
{
    if (_pcpe)
    {
        _pcpe->Release();
        _pcpe = NULL;
    }
    DllRelease();
}

// IUnknown methods
ULONG XPCredentialProvider::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG XPCredentialProvider::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

HRESULT XPCredentialProvider::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == NULL)
        return E_POINTER;
    
    *ppv = NULL;
    
    if (riid == IID_IUnknown || riid == IID_ICredentialProvider)
    {
        *ppv = static_cast<ICredentialProvider*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

// ICredentialProvider methods
HRESULT XPCredentialProvider::SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags)
{
    _cpus = cpus;
    
    // Only support logon scenario (unlock not supported in this version)
    if (cpus != CPUS_LOGON)
        return E_NOTIMPL;
    
    return S_OK;
}

HRESULT XPCredentialProvider::SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs)
{
    UNREFERENCED_PARAMETER(pcpcs);
    return S_OK;
}

HRESULT XPCredentialProvider::GetFieldDescriptorCount(DWORD* pdwCount)
{
    if (pdwCount == NULL)
        return E_POINTER;
    
    *pdwCount = 0; // We'll handle UI ourselves
    return S_OK;
}

HRESULT XPCredentialProvider::GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
{
    UNREFERENCED_PARAMETER(dwIndex);
    UNREFERENCED_PARAMETER(ppcpfd);
    return E_NOTIMPL;
}

HRESULT XPCredentialProvider::GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogon)
{
    if (pdwCount == NULL || pdwDefault == NULL || pbAutoLogon == NULL)
        return E_POINTER;
    
    *pdwCount = 1; // One credential
    *pdwDefault = 0; // First credential is default
    *pbAutoLogon = FALSE; // No auto-logon
    
    return S_OK;
}

HRESULT XPCredentialProvider::GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc)
{
    if (ppcpc == NULL)
        return E_POINTER;
    
    if (dwIndex != 0)
        return E_INVALIDARG;
    
    XPCredential* pCredential = new XPCredential();
    if (pCredential == NULL)
        return E_OUTOFMEMORY;
    
    HRESULT hr = pCredential->QueryInterface(IID_ICredentialProviderCredential, (void**)ppcpc);
    pCredential->Release();
    
    return hr;
}

HRESULT XPCredentialProvider::Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext)
{
    if (_pcpe != NULL)
    {
        _pcpe->Release();
    }
    
    _pcpe = pcpe;
    _pcpe->AddRef();
    _upAdviseContext = upAdviseContext;
    
    return S_OK;
}

HRESULT XPCredentialProvider::UnAdvise()
{
    if (_pcpe != NULL)
    {
        _pcpe->Release();
        _pcpe = NULL;
    }
    _upAdviseContext = 0;
    
    return S_OK;
}

// XPCredential implementation
XPCredential::XPCredential() :
    _cRef(1),
    _pcpce(NULL),
    _pwzUsername(NULL),
    _pwzPassword(NULL),
    _pwzDomain(NULL),
    _fChecked(false)
{
    DllAddRef();
}

XPCredential::~XPCredential()
{
    CoTaskMemFree(_pwzUsername);
    CoTaskMemFree(_pwzPassword);
    CoTaskMemFree(_pwzDomain);
    
    if (_pcpce)
    {
        _pcpce->Release();
        _pcpce = NULL;
    }
    
    DllRelease();
}

// IUnknown methods
ULONG XPCredential::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG XPCredential::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

HRESULT XPCredential::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == NULL)
        return E_POINTER;
    
    *ppv = NULL;
    
    if (riid == IID_IUnknown || riid == IID_ICredentialProviderCredential)
    {
        *ppv = static_cast<ICredentialProviderCredential*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

// ICredentialProviderCredential methods
HRESULT XPCredential::Advise(ICredentialProviderCredentialEvents* pcpce)
{
    if (_pcpce != NULL)
    {
        _pcpce->Release();
    }
    
    _pcpce = pcpce;
    if (_pcpce != NULL)
    {
        _pcpce->AddRef();
    }
    
    return S_OK;
}

HRESULT XPCredential::UnAdvise()
{
    if (_pcpce != NULL)
    {
        _pcpce->Release();
        _pcpce = NULL;
    }
    return S_OK;
}

HRESULT XPCredential::SetSelected(BOOL* pbAutoLogon)
{
    if (pbAutoLogon == NULL)
        return E_POINTER;
    
    *pbAutoLogon = FALSE;
    return S_OK;
}

HRESULT XPCredential::SetDeselected()
{
    return S_OK;
}

HRESULT XPCredential::GetFieldState(DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs)
{
    if (pcpfs) *pcpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
    return S_OK;
}

HRESULT XPCredential::GetStringValue(DWORD dwFieldID, PWSTR* ppwsz)
{
    if (ppwsz == NULL) return E_POINTER;
    *ppwsz = NULL;
    return S_OK;
}

HRESULT XPCredential::GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp)
{
    if (phbmp) *phbmp = NULL;
    return S_OK;
}

HRESULT XPCredential::GetCheckboxValue(DWORD dwFieldID, BOOL* pfChecked, PWSTR* ppwszLabel)
{
    UNREFERENCED_PARAMETER(dwFieldID);
    if (pfChecked) *pfChecked = _fChecked;
    if (ppwszLabel) *ppwszLabel = NULL;
    return S_OK;
}

HRESULT XPCredential::GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo)
{
    if (pdwAdjacentTo) *pdwAdjacentTo = 0;
    return S_OK;
}

HRESULT XPCredential::SetStringValue(DWORD dwFieldID, PCWSTR pwzs)
{
    return S_OK;
}

HRESULT XPCredential::SetCheckboxValue(DWORD dwFieldID, BOOL fChecked)
{
    _fChecked = fChecked;
    return S_OK;
}

HRESULT XPCredential::SetBitmapValue(DWORD dwFieldID, HBITMAP hbmp)
{
    return S_OK;
}

HRESULT XPCredential::CommandLinkClicked(DWORD dwFieldID)
{
    return S_OK;
}

HRESULT XPCredential::GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, 
                                       CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
                                       PWSTR* ppwszOptionalStatusText, 
                                       CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    if (pcpgsr == NULL || pcpcs == NULL || ppwszOptionalStatusText == NULL || pcpsiOptionalStatusIcon == NULL)
        return E_POINTER;
    
    *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
    *ppwszOptionalStatusText = NULL;
    *pcpsiOptionalStatusIcon = CPSI_NONE;
    
    // Create XP-style UI and get credentials
    XPLogonUI logonUI;
    if (!logonUI.Create(NULL))
    {
        *ppwszOptionalStatusText = (PWSTR)CoTaskMemAlloc(256 * sizeof(wchar_t));
        StringCchCopy(*ppwszOptionalStatusText, 256, L"Failed to create logon UI");
        *pcpsiOptionalStatusIcon = CPSI_ERROR;
        return S_OK;
    }
    
    logonUI.Show();
    
    // Wait for user input (simplified - in real implementation would use message loop)
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        // Check if user clicked login
        if (msg.message == WM_USER + 100) // Custom message for login
        {
            break;
        }
    }
    
    // Get credentials
    wchar_t username[256];
    wchar_t password[256];
    wchar_t domain[256];
    
    if (logonUI.GetUsername(username, 256) && logonUI.GetPassword(password, 256))
    {
        logonUI.GetDomain(domain, 256);
        
        // Allocate memory for credentials
        CoTaskMemFree(_pwzUsername);
        CoTaskMemFree(_pwzPassword);
        CoTaskMemFree(_pwzDomain);
        
        _pwzUsername = (PWSTR)CoTaskMemAlloc((wcslen(username) + 1) * sizeof(wchar_t));
        _pwzPassword = (PWSTR)CoTaskMemAlloc((wcslen(password) + 1) * sizeof(wchar_t));
        _pwzDomain = (PWSTR)CoTaskMemAlloc((wcslen(domain) + 1) * sizeof(wchar_t));
        
        StringCchCopy(_pwzUsername, wcslen(username) + 1, username);
        StringCchCopy(_pwzPassword, wcslen(password) + 1, password);
        StringCchCopy(_pwzDomain, wcslen(domain) + 1, domain);
        
        // Set up serialization
        pcpcs->clsidCredentialProvider = CLSID_XPCredentialProvider;
        pcpcs->rgbSerialization = NULL;
        pcpcs->cbSerialization = 0;
        pcpcs->ulAuthenticationPackage = 0; // Negotiate
        
        // Create credential structure
        KERB_INTERACTIVE_LOGON kil;
        ZeroMemory(&kil, sizeof(kil));
        
        kil.Identity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
        kil.Identity.User.Length = (USHORT)wcslen(_pwzUsername) * sizeof(wchar_t);
        kil.Identity.User.Buffer = (PWSTR)_pwzUsername;
        kil.Identity.Domain.Length = (USHORT)wcslen(_pwzDomain) * sizeof(wchar_t);
        kil.Identity.Domain.Buffer = (PWSTR)_pwzDomain;
        kil.Identity.Password.Length = (USHORT)wcslen(_pwzPassword) * sizeof(wchar_t);
        kil.Identity.Password.Buffer = (PWSTR)_pwzPassword;
        
        kil.MessageType = KerbInteractiveLogon;
        
        pcpcs->rgbSerialization = (BYTE*)CoTaskMemAlloc(sizeof(kil));
        CopyMemory(pcpcs->rgbSerialization, &kil, sizeof(kil));
        pcpcs->cbSerialization = sizeof(kil);
        
        *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
    }
    else
    {
        *ppwszOptionalStatusText = (PWSTR)CoTaskMemAlloc(256 * sizeof(wchar_t));
        StringCchCopy(*ppwszOptionalStatusText, 256, L"Invalid credentials");
        *pcpsiOptionalStatusIcon = CPSI_ERROR;
    }
    
    logonUI.Destroy();
    return S_OK;
}

HRESULT XPCredential::ReportResult(NTSTATUS ntsStatus, NTSTATUS ntsSubstatus, 
                                  PWSTR* ppwszOptionalStatusText, 
                                  CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    UNREFERENCED_PARAMETER(ntsStatus);
    UNREFERENCED_PARAMETER(ntsSubstatus);
    UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
    UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);
    return S_OK;
}
