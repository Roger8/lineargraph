#define WINGDICLASS_IMPL
#include "WinGdiClass.h"

CDialog::CDialog(UINT uID) : m_uTemplate(uID)
{

}

CDialog::~CDialog()
{

}

BOOL CDialog::Create(UINT uTemplateID, CWindow* pa)
{
    m_hWnd = CreateDialogParam(
        ::GetModuleHandle(0), MAKEINTRESOURCE(uTemplateID),
        pa ? pa->m_hWnd : 0,
        (DLGPROC)CWindow::WindowProcedure, (LPARAM)this);
    return m_hWnd != 0;
}

BOOL CDialog::BindChild(HWND& hCtrl, UINT uCtrlID)
{
    hCtrl = ::GetDlgItem(m_hWnd, uCtrlID);
    return hCtrl != 0;
}

BOOL CDialog::BindCommonControl(CCommonControl* pCtrl, UINT uCtrlID)
{
    if( pCtrl )
    {
        return pCtrl->Bind(::GetDlgItem(m_hWnd, uCtrlID));
    }
    return FALSE;
}

INT_PTR CDialog::DoModal(CWindow* pa)
{
    INT_PTR nResult;
    nResult = ::DialogBoxParamW(
        ::GetModuleHandle(0), MAKEINTRESOURCE(m_uTemplate),
        pa ? pa->m_hWnd : 0,
        (DLGPROC)CWindow::WindowProcedure, (LPARAM)this);
    m_hWnd = 0;
    return nResult;
}

int CDialog::PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch( msg )
    {
    case WM_INITDIALOG:
    	return OnInitDialog();
    case WM_COMMAND:
        return OnCommand(HIWORD(wp), LOWORD(wp), (HWND)lp);
    case WM_NOTIFY:
        return OnNotify((UINT)wp, (NMHDR*)lp);
    default:
        return CDialog::DefWindowProc(msg, wp, lp);
    }
    return 0;
}

int CDialog::DefWindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
    //  For a dialog, the default window procedure does
    // nothing but returns FALSE. The system dialog procedure
    // will do all left.
    return 0;
}

void CDialog::EndDialog(UINT uResult)
{
    ::EndDialog(m_hWnd, uResult);
}

BOOL CDialog::OnInitDialog()
{
    return TRUE;
}

BOOL CDialog::OnCommand(UINT uCode, UINT uID, HWND hCtrl)
{
    return (BOOL)CDialog::DefWindowProc(WM_COMMAND,
        MAKEWPARAM(uID,uCode), (LPARAM)hCtrl);
}

BOOL CDialog::OnNotify( UINT uID, NMHDR* pNmhdr )
{
    return (BOOL)CDialog::DefWindowProc(WM_NOTIFY,
        (WPARAM)uID, (LPARAM)pNmhdr);
}