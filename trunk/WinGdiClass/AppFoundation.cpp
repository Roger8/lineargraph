#define WINGDICLASS_IMPL
#include "WinGdiClass.h"

BOOL WINAPI DllMain(void* pvBase, DWORD dwReason, void*)
{
    if( dwReason == DLL_PROCESS_ATTACH )
    {
        CApplication::m_pvBase = pvBase;
        CApplication::m_tlsAppPtr = ::TlsAlloc();
        CApplication::m_tlsMsgPtr = ::TlsAlloc();

        if( CApplication::m_tlsAppPtr == TLS_OUT_OF_INDEXES
            || CApplication::m_tlsMsgPtr == TLS_OUT_OF_INDEXES )
        {
            return FALSE;
        }
    }
    return TRUE;
}

//
//

PVOID CApplication::m_pvBase = 0;
DWORD CApplication::m_tlsAppPtr = TLS_OUT_OF_INDEXES;
DWORD CApplication::m_tlsMsgPtr = TLS_OUT_OF_INDEXES;

CApplication* CApplication::GetCurrentApp()
{
    return reinterpret_cast<CApplication*>(::TlsGetValue(m_tlsAppPtr));
}

MSG* CApplication::GetCurrentMsg()
{
    return reinterpret_cast<MSG*>(::TlsGetValue(m_tlsMsgPtr));
}

CString CApplication::GetAppDirectory()
{
    DWORD  dwNameLength = 512;
    WCHAR* moduleName = new WCHAR[dwNameLength];

    while( dwNameLength == ::GetModuleFileNameW(0, moduleName, dwNameLength) )
    {
        delete[] moduleName;
        dwNameLength *= 2;
        moduleName = new WCHAR[dwNameLength];
    }

    CString strDir(moduleName);
    delete[] moduleName;

    strDir.Delete(strDir.ReverseFind(L'\\') + 1, strDir.GetLength());
    return strDir;
}

CApplication::CApplication() : m_pMainWnd(0)
{
    ::TlsSetValue(m_tlsAppPtr, this);

    m_hAccelTable = 0;
}

CApplication::~CApplication()
{
    ::TlsFree(m_tlsAppPtr);
}

CWindow* CApplication::GetMainWindow() const
{
    return m_pMainWnd;
}

HINSTANCE CApplication::GetInstance() const
{
    return (HINSTANCE)::GetModuleHandle(0);
}

BOOL CApplication::DoMessageLoop()
{
    if( !m_pMainWnd || !m_pMainWnd->m_hWnd )
    {
        return FALSE;
    }
    
	m_pMainWnd->ShowWindow();

    MSG msg = {0};
    ::TlsSetValue(m_tlsMsgPtr, &msg);   // Save current message

    int nRet;
    while( nRet = ::GetMessage(&msg, 0, 0, 0) )
    {
        if( nRet == -1 ){ break; }

        if( !m_hAccelTable || !::TranslateAccelerator(m_pMainWnd->m_hWnd, m_hAccelTable, &msg) )
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
    return TRUE;
}

BOOL CApplication::OnInitApplication()
{
    ::OleInitialize(0);
    CWindow::InitClass();
    return TRUE;
}

BOOL CApplication::OnExitApplication()
{
    return TRUE;
}

BOOL CApplication::LoadAcceleratorTable( UINT uAccelID )
{
    m_hAccelTable = ::LoadAccelerators(GetInstance(), MAKEINTRESOURCE(uAccelID));
    return m_hAccelTable != 0;
}
