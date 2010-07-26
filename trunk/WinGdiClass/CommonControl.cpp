#define WINGDICLASS_IMPL
#include "WinGdiClass.h"

CCommonControl::CCommonControl() : m_fnControlProc(0)
{
}

CCommonControl::~CCommonControl()
{

}

BOOL CCommonControl::Create(CWindow* pa, PCWSTR szClass, DWORD dwStyle, DWORD dwID)
{
    HWND hCtrl = ::CreateWindowExW(0,
        szClass, 0, dwStyle,
        0, 0, 0, 0, pa->m_hWnd, (HMENU)dwID,
        ::GetModuleHandle(0), 0);

    return Bind(hCtrl);
}

int CCommonControl::PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    return (int)m_fnControlProc(m_hWnd, msg, wp, lp);
}

BOOL CCommonControl::Bind(HWND hCommCtrl)
{
    if( m_hWnd || !hCommCtrl )
    {
        return FALSE;
    }

    m_hWnd = hCommCtrl;

    ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)static_cast<CWindow*>(this));
    m_fnControlProc = (WNDPROC)::GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);
    ::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)CWindow::WindowProcedure);
    return TRUE;
}
//
//  Static Control
//

BOOL CStatic::Create(CWindow* pa, DWORD dwStyle, DWORD dwID)
{
    return CCommonControl::Create(pa, WC_STATIC, dwStyle, dwID);
}

//
//  Edit Control
//
BOOL CEdit::Create( CWindow* pa, DWORD dwStyle, DWORD dwID )
{
    return CCommonControl::Create(pa, WC_EDITW, dwStyle, dwID);
}

BOOL CEdit::SetLimitText(size_t cchMax)
{
    return Send(EM_SETLIMITTEXT, cchMax);
}

//
//  Button Control
//
BOOL CButton::Create( CWindow* pa, DWORD dwStyle, DWORD dwID )
{
    return CCommonControl::Create(pa, WC_BUTTON, dwStyle, dwID);
}

int CButton::GetState() const
{
    return Send(BM_GETSTATE);
}

BOOL CButton::IsChecked() const
{
    return GetState() & BST_CHECKED;
}

int CButton::SetHilight(BOOL bHighlight)
{
    return Send(BM_SETSTATE, bHighlight);
}

BOOL CButton::SetCheck(int nCheck)
{
    return Send(BM_SETCHECK, nCheck);
}
//
//  Progress Bar
//
BOOL CProgressControl::Create(CWindow* pa, DWORD dwStyle, DWORD dwID)
{
    return CCommonControl::Create(pa, PROGRESS_CLASSW, dwStyle, dwID);
}

int CProgressControl::SetRange(int nMin, int nMax)
{
    return Send(PBM_SETRANGE32, nMin, nMax);
}

int CProgressControl::SetCurPos(int nPos)
{
    return Send(PBM_SETPOS, nPos, 0);
}

int CProgressControl::GetCurPos() const
{
    return Send(PBM_GETPOS, 0, 0);
}

int CProgressControl::SetStep(int nStep)
{
    return Send(PBM_SETSTEP, nStep);
}

int CProgressControl::StepIt()
{
    return Send(PBM_STEPIT);
}

int CProgressControl::GetRange(int& nMin, int& nMax) const
{
    PBRANGE rg = {0, 100};
    Send(PBM_GETRANGE, TRUE, (LPARAM)&rg);

    nMin = rg.iLow;
    nMax = rg.iHigh;
    return nMin;
}

BOOL CProgressControl::SetMarquee( BOOL bEnable, DWORD dwAniPeriod )
{
    return Send(PBM_SETMARQUEE, bEnable, (LPARAM)dwAniPeriod);
}

//
//
//
BOOL CTrackbar::Create(CWindow* pa, DWORD dwStyle, DWORD dwID)
{
    return CCommonControl::Create(pa, TRACKBAR_CLASSW, dwStyle, dwID);
}

int CTrackbar::SetRange(int nMin, int nMax)
{
    return Send(TBM_SETRANGE, 1, MAKELPARAM(nMin, nMax));
}

int CTrackbar::SetCurPos(int pos)
{
    return Send(TBM_SETPOS, 1, (LPARAM)pos);
}

int CTrackbar::GetCurPos() const
{
    return Send(TBM_GETPOS);
}
//
//
//
BOOL CToolbar::Create(CWindow* pa, DWORD dwStyle, DWORD dwID)
{
    CCommonControl::Create(pa, TOOLBARCLASSNAMEW, dwStyle, dwID);
    Send(TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    return m_hWnd != 0;
}

int CToolbar::AddButton(DWORD dwID, PCWSTR szText, DWORD dwBitmap, DWORD dwStyle)
{
    int iString = 0;
    if( szText )
    {
        iString = Send(TB_ADDSTRING, 0, (LPARAM)szText);
    }

    TBBUTTON tbtn = {0};
    tbtn.fsState = TBSTATE_ENABLED;
    tbtn.idCommand = dwID;
    tbtn.iString = iString;
    tbtn.iBitmap = dwBitmap;
    tbtn.fsStyle = (BYTE)dwStyle;
    return Send(TB_ADDBUTTONS, 1, (LPARAM)&tbtn);
}

int CToolbar::SetButtonSize(int cx, int cy)
{
    return Send(TB_SETBUTTONSIZE, 0, MAKELPARAM(cx, cy));
}

int CToolbar::AddBitmap(DWORD dwBitmapID, int nBitmaps)
{
    TBADDBITMAP tbab = {0};
    tbab.hInst = GetModuleHandle(0);
    tbab.nID = dwBitmapID;
    return Send(TB_ADDBITMAP, nBitmaps, (LPARAM)&tbab);
}

BOOL CComboBox::Create(CWindow* pa, DWORD dwStyle, DWORD dwID)
{
    return CCommonControl::Create(pa, WC_COMBOBOXW, dwStyle, dwID);
}

int CComboBox::AddString(PCWSTR szText)
{
    return Send(CB_ADDSTRING, 0, (LPARAM)szText);
}

int CComboBox::DeleteString(int i)
{
    return Send(CB_DELETESTRING, i);
}

int CComboBox::GetCurSel() const
{
    return Send(CB_GETCURSEL);
}

int CComboBox::GetItemCount() const
{
    return Send(CB_GETCOUNT);
}

int CComboBox::SetCurSel(int iSel)
{
    return Send(CB_SETCURSEL, iSel);
}

BOOL CTabPanel::Create( CWindow* pa, DWORD dwStyle, DWORD dwID )
{
    return CCommonControl::Create(pa, WC_TABCONTROLW, dwStyle, dwID);
}

int CTabPanel::InsertTabItem(PWSTR szCaption, int iItem, int iImage)
{
    TCITEM  item = {0};
    item.mask = TCIF_TEXT | TCIF_IMAGE;
    item.pszText = szCaption;
    item.cchTextMax = (int)::wcslen(szCaption);
    item.iImage = iImage;

    return TabCtrl_InsertItem(m_hWnd, iItem, &item);
}

BOOL CTabPanel::DeleteItem(int iItem)
{
    return TabCtrl_DeleteItem(m_hWnd, iItem);
}

BOOL CTabPanel::DeleteAllItems()
{
    return TabCtrl_DeleteAllItems(m_hWnd);
}

int CTabPanel::SetCurSel(int iSel)
{
    return TabCtrl_SetCurSel(m_hWnd, iSel);
}

int CTabPanel::GetCurSel() const
{
    return TabCtrl_GetCurSel(m_hWnd);
}

int CTabPanel::GetItemCount() const
{
    return TabCtrl_GetItemCount(m_hWnd);
}

int CTabPanel::SetItemSize( int cx, int cy )
{
    return TabCtrl_SetItemSize(m_hWnd, cx, cy);
}

DWORD CTabPanel::GetExtendedTabStyle() const
{
    return TabCtrl_GetExtendedStyle(m_hWnd);
}

DWORD CTabPanel::SetExtendedTabStyle( DWORD dwExTabStyle )
{
    return TabCtrl_SetExtendedStyle(m_hWnd, dwExTabStyle);
}

BOOL CListView::Create(CWindow* pa, DWORD dwStyle, DWORD dwID)
{
    return CCommonControl::Create(pa, WC_LISTVIEWW, dwStyle, dwID);
}

DWORD CListView::GetExtendedListViewStyle() const
{
    return ListView_GetExtendedListViewStyle(m_hWnd);
}

DWORD CListView::SetExtendedListViewStyle(DWORD dwExtLvStyle)
{
    return ListView_SetExtendedListViewStyle(m_hWnd, dwExtLvStyle);
}

int CListView::InsertColumn(int iCol, PCWSTR szText, int nWidth, int fmt)
{
    WCHAR buf[128] = {0};
    wcsncpy_s(buf, 128, szText, 126);

    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_FMT|LVCF_TEXT|LVCF_WIDTH;
    lvc.fmt = fmt;
    lvc.pszText = buf;
    lvc.cchTextMax = (int)::wcslen(buf);
    lvc.cx = nWidth;
    return ListView_InsertColumn(m_hWnd, iCol, &lvc);
}

int CListView::InsertItem(int iItem, PCWSTR szText)
{
    WCHAR buf[128] = {0};
    ::wcsncpy_s(buf, 128, szText, 126);

    LVITEMW lvi = {0};
    lvi.mask = LVIF_TEXT;
    lvi.iItem = iItem;
    lvi.pszText = buf;
    lvi.cchTextMax = (int)::wcslen(buf);
    return ListView_InsertItem(m_hWnd, &lvi);
}

BOOL CListView::SetItemText(int iItem, int iSubItem, PCWSTR szText)
{
    WCHAR buf[128] = {0};
    ::wcsncpy_s(buf, 128, szText, 126);

    ListView_SetItemText(m_hWnd, iItem, iSubItem, buf);
    return TRUE;
}

BOOL CListView::SetItemState(int iItem, UINT uState, UINT uMask)
{
    ListView_SetItemState(m_hWnd, iItem, uState, uMask);
    return TRUE;
}

UINT CListView::GetItemState(int iItem, UINT uMask)
{
    return ListView_GetItemState(m_hWnd, iItem, uMask);
}

BOOL CListView::SetItemChecked(int iItem, BOOL bCheck)
{
    ListView_SetCheckState(m_hWnd, iItem, bCheck);
    return TRUE;
}

BOOL CListView::IsItemChecked(int iItem) const
{
    return ListView_GetCheckState(m_hWnd, iItem);
}
