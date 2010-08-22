#include "LinearGraphToolPanel.h"
#include "LinearGraphFrame.h"

CLinearGraphToolPanel::CLinearGraphToolPanel() : m_pFrame(0)
{
    
}

CLinearGraphToolPanel::~CLinearGraphToolPanel()
{

}

BOOL CLinearGraphToolPanel::Create(CLinearGraphFrameWnd* pa)
{
    m_pFrame = pa;
    CWindow::Create(WS_CHILD|WS_VISIBLE, pa->m_hWnd);

    m_tabPanel.Create(this, WS_CHILD, CLinearGraphToolPanel::IDC_TABPANEL);
    m_tabPanel.Send(WM_SETFONT, (WPARAM)CLinearGraphApp::GetThemeFont());
    m_tabPanel.SetItemSize(120, 24);
    return TRUE;
}

int  CLinearGraphToolPanel::PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch( msg )
    {
    case WM_SIZE:
        OnSizeChanged((DWORD)wp, LOWORD(lp), HIWORD(lp));
        break;
    case WM_NOTIFY:
        OnNotify((UINT)wp, (NMHDR*)lp);
        break;
    default:
        return CWindow::PreProcessMessage(msg, wp, lp);
    }
    return 0;
}

void CLinearGraphToolPanel::OnPaint(HDC dc, PAINTSTRUCT& ps)
{
    ::FillRect(dc, &ps.rcPaint, CLinearGraphApp::GetThemeToolWindowBrush());
}

void CLinearGraphToolPanel::OnSizeChanged(DWORD dwFlags, int cx, int cy)
{
    m_tabPanel.SetPlacement(0, 2, cx, cy-2);
}

void CLinearGraphToolPanel::OnNotify(UINT uID, NMHDR* pNmhdr)
{
    switch( uID )
    {
    case IDC_TABPANEL:
        if( pNmhdr->code == TCN_SELCHANGE )
        {
            m_pFrame->Send(MSG_TabSelectChange, IDC_TABPANEL, (LPARAM)this);
            InvalidateRect();
        }
        break;
    default:
        CWindow::PreProcessMessage(WM_NOTIFY, uID, (LPARAM)pNmhdr);
    }
}

int CLinearGraphToolPanel::AddTab(PCWSTR tabName)
{
    WCHAR buff[64] = {0};
    wcsncpy_s(buff, 64, tabName, 62);

    int iTab = m_tabPanel.InsertTabItem(buff, m_tabPanel.GetItemCount());
    if( 0 == iTab )
    {
        m_tabPanel.ShowWindow(SW_SHOW);
    }
    InvalidateRect();
    return iTab;
}

BOOL CLinearGraphToolPanel::RemoveViewTab( int iTab )
{
    bool bAdjustSel = (m_tabPanel.GetCurSel() == iTab);

    if( m_tabPanel.DeleteItem(iTab) )
    {
        if( !m_tabPanel.GetItemCount() )
        {
            m_tabPanel.ShowWindow(SW_HIDE);
        }
        else if( bAdjustSel )
        {
            m_tabPanel.SetCurSel(0);
        }
        return InvalidateRect();
    }
    return FALSE;
}

void CLinearGraphToolPanel::RemoveAllViewTabs()
{
    m_tabPanel.ShowWindow(SW_HIDE);
    m_tabPanel.DeleteAllItems();
}

int CLinearGraphToolPanel::SetActiveViewTab( int iTab )
{
    return m_tabPanel.SetCurSel(iTab);
}

int CLinearGraphToolPanel::GetActiveViewTab() const
{
    return m_tabPanel.GetCurSel();
}

int CLinearGraphToolPanel::GetTabCount() const
{
    return m_tabPanel.GetItemCount();
}
