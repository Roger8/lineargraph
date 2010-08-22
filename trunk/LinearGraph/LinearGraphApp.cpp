#include "LinearGraphFrame.h"
#include "LinearGraphApp.h"
#include <new>

void linear_graph_new_handler()
{
    int nRet = ::MessageBox(0,
        L"We are sorry that the program is run out of memory.\n"
        L"Press Ok to retry or Cancel to close the program.",
        L"LinearGraph", MB_ICONWARNING | MB_OKCANCEL
        );
    if( nRet != IDOK )
    {
        LG_CLOSE_LOG();
        ::ExitProcess(0);
    }
}

CFont CLinearGraphApp::m_themeFont;

CSolidBrush CLinearGraphApp::m_themeBrush(RGB(0x29, 0x39, 0x55));

CSolidBrush CLinearGraphApp::m_themeToolWndBrush(RGB(0xBC, 0xC7, 0xD8));

CLinearGraphApp::CLinearGraphApp()
{
    m_hLogoIcon = 0;
}

CLinearGraphApp::~CLinearGraphApp()
{
}

BOOL CLinearGraphApp::OnInitApplication()
{
    if( !CApplication::OnInitApplication() )
    {
        return FALSE;
    }

    WCHAR reasonString[128];
    WCHAR appCaption[64];
    GetString(IDS_APP_CAPTION, appCaption, 64);

    OSVERSIONINFOW osv = {0};
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    ::GetVersionExW(&osv);
    if( osv.dwMajorVersion < 5 )
    {
        GetString(IDS_ERR_OSVER_LOW, reasonString, 128);
        ::MessageBoxW(0, reasonString, appCaption, MB_ICONERROR);
        return FALSE;
    }

    if( !LG_OPEN_LOG() )
    {
        GetString(IDS_ERR_LOG_FAILED, reasonString, 128);
        ::MessageBoxW(0, reasonString, appCaption, MB_ICONWARNING);
    }
    LG_TRACE_FUNCTION();

    std::set_new_handler(linear_graph_new_handler);

    LoadAcceleratorTable(IDR_ACCELERATOR);

    Gdiplus::GdiplusStartupInput gsi;
    Gdiplus::GdiplusStartup(&m_upGdiplus, &gsi, 0);

    CLinearGraphFrameWnd* pFrameWnd;
    pFrameWnd = new CLinearGraphFrameWnd;
	pFrameWnd->Create();
	pFrameWnd->SetTaskbarIcon( GetLogo() );
	pFrameWnd->SetCaptionIcon( GetLogo() );
    m_pMainWnd = static_cast<CWindow*>(pFrameWnd);

    InitAsyncTaskMonitor();
    ::DragAcceptFiles(pFrameWnd->m_hWnd, TRUE);

    INT     argc = 0;
    WCHAR** argv = 0;
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    
    BeginHeavyTask();
    for(int i = 1; i < argc; i++)
    {
        LG_TRACE("Open document from command line: %ws", argv[i]);
        
        EnterHeavyFileTask(argv[i]);
        if( !pFrameWnd->OpenDocument(argv[i]) )
        {
            GetString(IDS_ERR_OPEN_FAILED, reasonString, 128);

            CString strMsg;
            strMsg.Format(L"%s:\n%s", reasonString, argv[i]);
            pFrameWnd->MessageBox(strMsg, MB_ICONWARNING);
        }
    }
    EndHeavyTask();

    ::SetForegroundWindow(m_pMainWnd->m_hWnd);
    return TRUE;
}

BOOL CLinearGraphApp::OnExitApplication()
{
    LG_TRACE_FUNCTION();

    CApplication::OnExitApplication();

    Gdiplus::GdiplusShutdown(m_upGdiplus);

    CLinearGraphFrameWnd* pFrameWnd;
    pFrameWnd = dynamic_cast<CLinearGraphFrameWnd*>(m_pMainWnd);
    delete pFrameWnd;

    m_taskWnd.Close();
    ::WaitForSingleObject(m_hMonitor, INFINITE);
    ::CloseHandle(m_hMonitor);

    LG_CLOSE_LOG();
    return TRUE;
}

HICON CLinearGraphApp::GetLogo()
{
	if( !m_hLogoIcon )
	{
		m_hLogoIcon = ::LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_LOGO));
	}
	return m_hLogoIcon;
}

DWORD CLinearGraphApp::GetString(UINT stringID, PWSTR stringBuff, DWORD buffLength)
{
    return (DWORD)::LoadStringW(GetInstance(), stringID, stringBuff, buffLength);
}

struct TASKMONITORPARAM
{
    CLinearGraphApp*    pApplication;
    HANDLE              hSyncEvent;
};

BOOL CLinearGraphApp::InitAsyncTaskMonitor()
{
    LG_TRACE_FUNCTION();

    TASKMONITORPARAM tmp;
    tmp.pApplication = this;
    tmp.hSyncEvent = ::CreateEvent(0, TRUE, FALSE, 0);
    m_hMonitor = ::CreateThread(0, 0, TaskMonitorThread, &tmp, 0, 0);

    ::WaitForSingleObject(tmp.hSyncEvent, INFINITE);
    ::CloseHandle(tmp.hSyncEvent);
    return m_hMonitor != 0;
}

DWORD WINAPI CLinearGraphApp::TaskMonitorThread(PVOID pvParam)
{
    CLinearGraphApp* pThis = ((TASKMONITORPARAM*)pvParam)->pApplication;
    pThis->m_dwMonitorID = ::GetCurrentThreadId();
    pThis->m_taskWnd.Create();

    ::SetEvent(((TASKMONITORPARAM*)pvParam)->hSyncEvent);

    MSG msg;
    int nRet;
    while( nRet = ::GetMessage(&msg, 0, 0, 0) )
    {
        if( nRet == -1 ){ break; }

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
    return 0;
}

void CLinearGraphApp::BeginHeavyTask()
{
    ::SetCursor(::LoadCursor(0, IDC_WAIT));
    if( m_pMainWnd )
    {
        m_pMainWnd->ShowWindow(SW_HIDE);
        //m_pMainWnd->Enable(FALSE);
        //::AttachThreadInput(::GetCurrentThreadId(), m_dwMonitorID, TRUE);
    }

    WCHAR buff[128];
    GetString(IDS_APPLICATION_BUSY, buff, 128);
    m_taskWnd.SetStatusText(buff);
    m_taskWnd.DelayShow();
}

void CLinearGraphApp::EnterHeavyTaskStep(UINT stepStrinID)
{
    WCHAR buff[256];
    ::LoadStringW(::GetModuleHandle(0), stepStrinID, buff, 256);
    m_taskWnd.SetStatusText(buff);
}

void CLinearGraphApp::EnterHeavyFileTask(PCWSTR fileName)
{
    WCHAR buff[2048];
    ::LoadStringW(::GetModuleHandle(0), IDS_OPEN_DOCUMENT, buff, 256);
    
    wcsncat(buff, fileName, 2048);
    m_taskWnd.SetStatusText(buff);
}

void CLinearGraphApp::EndHeavyTask()
{
    ::SetCursor(::LoadCursor(0, IDC_ARROW));
    m_taskWnd.Hide();
    if( m_pMainWnd )
    {
        //::AttachThreadInput(::GetCurrentThreadId(), m_dwMonitorID, FALSE);
        //m_pMainWnd->Enable(TRUE);
        m_pMainWnd->ShowWindow();
        m_pMainWnd->SetForegroundWindow();
    }
}

HFONT CLinearGraphApp::GetThemeFont()
{
    return (HFONT)m_themeFont;
}

HBRUSH CLinearGraphApp::GetThemeBrush()
{
    return (HBRUSH)m_themeBrush;
}

HBRUSH CLinearGraphApp::GetThemeToolWindowBrush()
{
    return (HBRUSH)m_themeToolWndBrush;
}

CString CLinearGraphApp::GetLogDirectory()
{
    CString temp = CApplication::GetAppDirectory();
    temp.Append(L"log\\\0", 4);
    return temp;
}
