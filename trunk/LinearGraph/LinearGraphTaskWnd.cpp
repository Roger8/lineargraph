#include "LinearGraph.h"
#include "LinearGraphApp.h"
#include "LinearGraphTaskWnd.h"

CLinearGraphTaskMonitorWnd::CLinearGraphTaskMonitorWnd()
{

}

BOOL CLinearGraphTaskMonitorWnd::Create()
{
    CWindow::Create(WS_BORDER|WS_CAPTION);

    WCHAR captioinString[64];
    GetString(IDS_TASKMONITOR_CAPTION, captioinString, 128);
    SetText(captioinString);
    SetPlacement(MiddleCenter, 500, 200);

    m_progress.Create(this, WS_CHILD|WS_VISIBLE|PBS_MARQUEE, 0);
    m_progress.SetPlacement(20, 80, 460, 26);

    GetString(IDS_SAFE_EXIT, captioinString, 64);
    m_safeExitButton.Create(this, WS_CHILD|WS_VISIBLE, ID_SAFEEXIT);
    m_safeExitButton.SetPlacement(380, 140, 100, 24);
    m_safeExitButton.Send(WM_SETFONT, (WPARAM)CLinearGraphApp::GetThemeFont());
    m_safeExitButton.SetText(captioinString);
    return TRUE;
}

int CLinearGraphTaskMonitorWnd::PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch( msg )
    {
    case WM_TIMER:
        {
            if( 0 == m_busyTime )
            {
                ShowWindow();
            }
            m_busyTime += 100;

        }
        break;
    case WM_CTLCOLORBTN:
        return (int)CLinearGraphApp::GetThemeToolWindowBrush();

    case WM_SHOWWINDOW:
        OnShowWindow(wp);
        break;
    case WM_COMMAND:
        OnCommand(HIWORD(wp), LOWORD(wp), (HWND)lp);
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;
    default:
        return CWindow::PreProcessMessage(msg, wp, lp);
    }
    return 0;
}

void CLinearGraphTaskMonitorWnd::OnPaint(HDC dc, PAINTSTRUCT& ps)
{
    RECT rect;
    GetClientRect(rect);
    ::FillRect(dc, &rect, CLinearGraphApp::GetThemeBrush());

    RECT rcBottom = {0, rect.bottom-40, rect.right, rect.bottom};
    ::FillRect(dc, &rcBottom, CLinearGraphApp::GetThemeToolWindowBrush());

    rect.left += 15;
    rect.top += 15;
    rect.right -= 15;
    ::DeleteObject(::SelectObject(dc, CLinearGraphApp::GetThemeFont()));
    ::SetBkMode(dc, TRANSPARENT);
    rect.bottom = 40;
    ::DrawTextW(dc, m_statusText, m_statusText.GetLength(), &rect, DT_SINGLELINE|DT_END_ELLIPSIS);
    ::SetBkMode(dc, OPAQUE);
}

void CLinearGraphTaskMonitorWnd::SetStatusText(PCWSTR statusText)
{
    m_statusText = statusText;
    InvalidateRect();
}

void CLinearGraphTaskMonitorWnd::DelayShow()
{
    m_busyTime = 0;
    ::SetTimer(m_hWnd, 1, 100, 0);
}

void CLinearGraphTaskMonitorWnd::Hide()
{
    LG_TRACE("Last operation costs %dms", m_busyTime);

    ::KillTimer(m_hWnd, 1);
    ShowWindow(SW_HIDE);
}

void CLinearGraphTaskMonitorWnd::OnCommand( UINT uCode, UINT uID, HWND hCtrl )
{
    switch( uID )
    {
    case ID_SAFEEXIT:
        OnSafeExit();
        break;
    }
}

void CLinearGraphTaskMonitorWnd::OnSafeExit()
{
    LG_TRACE_FUNCTION();

    WCHAR buff[128];
    GetString(IDS_EXIT_CONFIRM, buff, 128);

    if( IDYES == MessageBox(buff, MB_ICONQUESTION|MB_YESNO) )
    {
        LG_CLOSE_LOG();
        ::ExitProcess(0);
    }
}

void CLinearGraphTaskMonitorWnd::OnShowWindow( BOOL bShow )
{
    m_progress.SetMarquee(bShow);
    
    if( bShow )
    {
        WCHAR buff[128];
        GetString(IDS_APPLICATIOIN_BUSY, buff, 128);
        m_statusText = buff;
        SetForegroundWindow();
    }
}
