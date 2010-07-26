#include "LinearGraphFrame.h"
#include "LinearGraphDoc.h"
#include "LinearGraphView.h"
#include "LinearGraphCodec.h"

BOOL CSourceSelectDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    BindCommonControl(&m_listView, IDC_LIST_DATA);

    m_listView.SetExtendedListViewStyle(
        LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);
    m_listView.SetTheme(L"Explorer");

    WCHAR columnName[128] = {0};
    GetString(IDS_SOURCE_NAME, columnName, 128);
    m_listView.InsertColumn(0, columnName, 100, LVCFMT_LEFT);
    GetString(IDS_TIMESTAMP, columnName, 128);
    m_listView.InsertColumn(1, columnName, 80, LVCFMT_LEFT);
    GetString(IDS_FREQUENCY, columnName, 128);
    m_listView.InsertColumn(2, columnName, 100, LVCFMT_LEFT);
    GetString(IDS_DATAPTS, columnName, 128);
    m_listView.InsertColumn(3, columnName, 80, LVCFMT_LEFT);
    GetString(IDS_SOURCE_PATH, columnName, 128);
    m_listView.InsertColumn(4, columnName, 400, LVCFMT_LEFT);

    if( m_vSource.size() == m_vChoice.size() )
    {
        CString buf;
        for(size_t i = 0; i < m_vSource.size(); ++i)
        {
            m_listView.InsertItem((int)i, m_vSource[i]->name);
            m_listView.SetItemChecked((int)i, m_vChoice[i]);

            buf.Format(L"%I64d", m_vSource[i]->timeStamp);
            m_listView.SetItemText((int)i, 1, buf);

            buf.Format(L"%f", m_vSource[i]->getFrequency());
            m_listView.SetItemText((int)i, 2, buf);

            buf.Format(L"%d", m_vSource[i]->length);
            m_listView.SetItemText((int)i, 3, buf);

            m_listView.SetItemText((int)i, 4, m_vSource[i]->ownerFile);
        }
    }

    return TRUE;
}

BOOL CSourceSelectDlg::OnCommand(UINT uCode, UINT uID, HWND hCtrl)
{
    switch( uID )
    {
    case IDC_BTN_CANCEL:
        if( uCode == BN_CLICKED )
        {
            EndDialog(IDCANCEL);
        }
        break;
    case IDC_BTN_OK:
        if( uCode == BN_CLICKED )
        {
            OnBnClickedOk();
        }
        break;
    default:
        return CDialog::OnCommand(uCode, uID, hCtrl);
    }
    return TRUE;
}

void CSourceSelectDlg::OnBnClickedOk()
{
    m_vChoice.assign(m_vChoice.size(), false);

    int nSelected = 0;
    for(size_t i = 0; i < m_vSource.size(); ++i)
    {
        if( m_listView.IsItemChecked((int)i) )
        {
            m_vChoice[i] = true;
            ++nSelected;
        }
        else{ m_vChoice[i] = false; }
    }

    if( nSelected < 1 )
    {

        MessageBox(
            L"No any data source is selected !",
            MB_ICONINFORMATION|MB_OK);
    }
    else{ EndDialog(IDOK); }
}

CFormatSaveDlg::CFormatSaveDlg(BOOL bEnableCheck)
    : CDialog(CFormatSaveDlg::IDD), m_bEnableCheck(bEnableCheck)
{
    WCHAR buff[2048] = {0};

    GetCurrentDirectoryW(2046, buff);
    m_dirName = buff;
    m_fCheckOverwrite = FALSE;
    m_pCaption[0] = 0;
}

BOOL CFormatSaveDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetText(m_pCaption);

    BindCommonControl(&m_cbFmtlist, IDC_CB_FMTLIST);
    BindCommonControl(&m_editDir, IDC_EDIT_SAVEDIR);
    BindCommonControl(&m_btnReminder, IDC_CHK_REMINDONWRITE);

    m_editDir.SetLimitText(2048);
    m_editDir.SetText(m_dirName);
    m_btnReminder.SetCheck(m_fCheckOverwrite ? BST_CHECKED:BST_UNCHECKED);
    m_btnReminder.Enable(m_bEnableCheck);
    return FALSE;
}

BOOL CFormatSaveDlg::OnCommand(UINT uCode, UINT uID, HWND hCtrl)
{
    switch( uID )
    {
    case IDC_BTN_BROWSE:
        if( uCode == BN_CLICKED ){ OnBtnBrowse(); }
        break;
    case IDCANCEL:
        if( uCode == BN_CLICKED ){ EndDialog(uID); }
        break;
    case IDOK:
        if( uCode == BN_CLICKED ){ OnBtnOk(); }
        break;
    default:
        return CDialog::OnCommand(uCode, uID, hCtrl);
    }
    return TRUE;
}

void CFormatSaveDlg::OnBtnOk()
{    
    int   len;
    WCHAR dirBuff[2048] = {0};
    
    if( !(len = m_editDir.GetText(dirBuff, 2046)) )
    {
        WCHAR reasonString[128];
        GetString(IDS_FOLDER_PATH_EMPTY, reasonString, 128);
        MessageBox(reasonString, MB_ICONERROR);
        return ;
    }

    ::SetCurrentDirectoryW(dirBuff);

    if( dirBuff[len-1] != L'\\' )
    {
        dirBuff[len] = L'\\';
        dirBuff[++len] = 0;
    }

    m_dirName = dirBuff;
    m_iFormat = m_cbFmtlist.GetCurSel();

    m_fCheckOverwrite = m_btnReminder.IsChecked();
    EndDialog(IDOK);
}

void CFormatSaveDlg::OnBtnBrowse()
{
    WCHAR nameBuff[MAX_PATH+2];
    WCHAR titleString[128];
    GetString(IDS_SELECT_TARGET_FOLDER, titleString, 128);
    
    PIDLIST_ABSOLUTE pidlist;
    BROWSEINFOW bi = {0};
    bi.ulFlags = BIF_USENEWUI|BIF_UAHINT;
    bi.hwndOwner = m_hWnd;
    bi.pszDisplayName = nameBuff;
    bi.lpszTitle = titleString;
    if( pidlist = ::SHBrowseForFolderW(&bi) )
    {
        ::SHGetPathFromIDListW(pidlist, nameBuff);
        ::CoTaskMemFree(pidlist);

        m_editDir.SetText(nameBuff);
    }
}

int CFormatSaveDlg::AddFormatType(PCWSTR fmtDesc)
{
    return m_cbFmtlist.AddString(fmtDesc);
}

void CFormatSaveDlg::SetCaption(UINT capStringID)
{
    GetString(capStringID, m_pCaption, 64);
    if( m_hWnd )
    {
        SetText(m_pCaption);
    }
}

void CPrintGraphDlg::OnBtnOk()
{
    static WCHAR extensionNames[][8] = 
    {
        L".BMP", L".PNG", L".JPG", L".GIF", L".TIFF"
    };

    CFormatSaveDlg::OnBtnOk();

    if( m_iFormat < 0 || m_iFormat > 4 )
    {
        m_iFormat = 0;
    }
    m_extensionName = extensionNames[m_iFormat];
}

BOOL CPrintGraphDlg::OnInitDialog()
{
    CFormatSaveDlg::OnInitDialog();
    SetCaption(IDS_SAVE_IMAGE);

    WCHAR textBuff[1024];
    GetString(IDS_IMGFMT_BMP, textBuff, 1024);
    AddFormatType(textBuff);
    GetString(IDS_IMGFMT_PNG, textBuff, 1024);
    AddFormatType(textBuff);
    GetString(IDS_IMGFMT_JPEG, textBuff, 1024);
    AddFormatType(textBuff);
    GetString(IDS_IMGFMT_GIF, textBuff, 1024);
    AddFormatType(textBuff);
    GetString(IDS_IMGFMT_TIFF, textBuff, 1024);
    AddFormatType(textBuff);

    m_cbFmtlist.SetCurSel(0);
    return FALSE;
}

BOOL CSaveDataDlg::OnInitDialog()
{
    CFormatSaveDlg::OnInitDialog();
    
    WCHAR textBuff[1024];
    GetString(IDS_DATAFMT_BIN, textBuff, 1024);
    AddFormatType(textBuff);
    GetString(IDS_DATAFMT_TEXT, textBuff, 1024);
    AddFormatType(textBuff);

    if( m_fTextWithTimestamp )
    {
        GetString(IDS_DATAFMT_TEXT_WITHTIME, textBuff, 1024);
        AddFormatType(textBuff);
    }

    m_cbFmtlist.SetCurSel(0);
    return FALSE;
}

void CSaveDataDlg::OnBtnOk()
{
    CFormatSaveDlg::OnBtnOk();

    if( m_iFormat < 0 )
    {
        m_iFormat = 0;
    }
}

CDateTimeDialog::CDateTimeDialog() : CDialog(IDD)
{
    m_dwFreqBase = m_dwFreqMulti = 1;
    memset(&m_stDateTime, 0, sizeof(m_stDateTime));
}

BOOL CDateTimeDialog::OnCommand(UINT uCode, UINT uID, HWND hCtrl)
{
    switch( uID )
    {
    case IDC_BTN_CANCEL:
        if( uCode == BN_CLICKED )
        {
            EndDialog(IDCANCEL);
        }
        break;
    case IDC_BTN_OK:
        if( uCode == BN_CLICKED )
        {
            OnBtnOK();
        }
        break;
    default:
        return CDialog::OnCommand(uCode, uID, hCtrl);
    }
    return TRUE;
}

BOOL CDateTimeDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    BindCommonControl(&m_editFreqMulti, IDC_EDIT_FREQ_MULTI);
    BindCommonControl(&m_editFreqBase, IDC_EDIT_FREQ_BASE);
    BindCommonControl(&m_editTimeMs, IDC_EDIT_TIME_MS);

    m_editFreqMulti.SetLimitText(7);
    m_editFreqBase.SetLimitText(7);
    m_editTimeMs.SetLimitText(3);

    WCHAR buff[16];
    _snwprintf_s(buff, 16, 4, L"%d", m_stDateTime.wMilliseconds);
    m_editTimeMs.SetText(buff);

    _snwprintf_s(buff, 16, 14, L"%d", m_dwFreqMulti);
    m_editFreqMulti.SetText(buff);

    _snwprintf_s(buff, 16, 14, L"%d", m_dwFreqBase);
    m_editFreqBase.SetText(buff);

    if( m_stDateTime.wYear )
    {
        DateTime_SetSystemtime(GetDlgItem(m_hWnd, IDC_DTPICKER_DATE), GDT_VALID, &m_stDateTime);
        DateTime_SetSystemtime(GetDlgItem(m_hWnd, IDC_DTPICKER_TIME), GDT_VALID, &m_stDateTime);
    }
    return FALSE;
}

void CDateTimeDialog::OnBtnOK()
{
    WCHAR buff[8];
    m_editFreqMulti.GetText(buff, 8);
    m_dwFreqMulti = (WORD)::_wtoi(buff);

    m_editFreqBase.GetText(buff, 8);
    m_dwFreqBase = (WORD)::_wtoi(buff);

    if( !m_dwFreqMulti || !m_dwFreqBase)
    {
        WCHAR reasonString[128];
        GetString(IDS_FREQUENCY_EMPTY, reasonString, 128);
        MessageBox(reasonString, MB_ICONERROR);
        return ;
    }

    SYSTEMTIME sysTime;
    DateTime_GetSystemtime(GetDlgItem(m_hWnd, IDC_DTPICKER_DATE), &m_stDateTime);
    DateTime_GetSystemtime(GetDlgItem(m_hWnd, IDC_DTPICKER_TIME), &sysTime);
    m_stDateTime.wHour = sysTime.wHour;
    m_stDateTime.wMinute = sysTime.wMinute;
    m_stDateTime.wSecond = sysTime.wSecond;

    m_editTimeMs.GetText(buff, 8);
    m_stDateTime.wMilliseconds = (WORD)::_wtoi(buff);
    EndDialog(IDOK);
}

CNameDialog::CNameDialog() : CDialog(CNameDialog::IDD)
{
    m_lFontHeight = CLinearGraphView::DefaultFontHeight;
}

BOOL CNameDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    BindCommonControl(&m_editName, IDC_EDIT_NAME);
    BindCommonControl(&m_cbFonts, IDC_CB_FONTS);

    m_cbFonts.AddString(L"12");
    m_cbFonts.AddString(L"20");
    m_cbFonts.AddString(L"30");
    m_cbFonts.AddString(L"40");
    m_cbFonts.AddString(L"50");
    m_cbFonts.AddString(L"60");
    m_cbFonts.AddString(L"70");
    m_cbFonts.AddString(L"80");
    m_cbFonts.AddString(L"90");

    m_cbFonts.SetCurSel(
        m_lFontHeight<CLinearGraphView::DefaultFontHeight ? (-m_lFontHeight/10-1) : 0);

    m_editName.SetText(m_strName);
    m_editName.SetLimitText(1022);
    return FALSE;
}

BOOL CNameDialog::OnCommand(UINT uCode, UINT uID, HWND hCtrl)
{
    switch( uID )
    {
    case IDOK:
        {
            WCHAR buf[1024];
            m_editName.GetText(buf, 1024);
            m_strName = buf;

            int nSel = m_cbFonts.GetCurSel();
            if( nSel <= 0 )
            {
                m_lFontHeight = CLinearGraphView::DefaultFontHeight;
            }
            else
            {
                m_lFontHeight = -10 * (nSel+1);
            }
            EndDialog(IDOK);
        }
        break;
    case IDCANCEL:
        EndDialog(IDCANCEL);
        break;
    default:
        return CDialog::OnCommand(uCode, uID, hCtrl);
    }
    return TRUE;
}

BOOL CLinearTransformDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    BindCommonControl(&m_editK, IDC_EDIT_PARAM1);
    BindCommonControl(&m_editC, IDC_EDIT_PARAM2);

    m_editC.SetLimitText(10);
    m_editK.SetLimitText(10);

    m_editC.SetText(L"0");
    m_editK.SetText(L"1");
    return FALSE;
}

BOOL CLinearTransformDialog::OnCommand(UINT uCode, UINT uID, HWND hCtrl)
{
    switch( uID )
    {
    case IDOK:
        if( uCode == BN_CLICKED )
        {
            WCHAR buff[32];
            if( !m_editC.GetText(buff, 30) )
            {
                m_C = 0;
            }
            else{ m_C = _wtof(buff); }

            if( !m_editK.GetText(buff, 30) )
            {
                WCHAR reasonString[128];
                GetString(IDS_COEFFICIENT_EMPTY, reasonString, 128);
                MessageBox(reasonString, MB_ICONERROR);
                return TRUE;
            }
            else{ m_K = _wtof(buff); }
            
            EndDialog(IDOK);
        }
        break;
    case IDCANCEL:
        if( uCode == BN_CLICKED )
        {
            EndDialog(IDCANCEL);
        }
        break;
    default:
        return CDialog::OnCommand(uCode, uID, hCtrl);
    }
    return TRUE;
}


CLinearGraphFrameWnd::CLinearGraphFrameWnd()
    : m_pLastActiveView(0)
{
    m_dwViewOpts = ~CLinearGraphView::OptHorizontalLine;
    m_layoutStyle = TabbedDoc;
}

CLinearGraphFrameWnd::~CLinearGraphFrameWnd()
{
}

BOOL CLinearGraphFrameWnd::Create()
{
	CWindow::Create(WS_OVERLAPPEDWINDOW);
    
    WCHAR appCaptioinString[64];
    GetString(IDS_APP_CAPTION, appCaptioinString, 128);
    SetText(appCaptioinString);
    SetPlacement(CWindow::MiddleCenter, 1000, 640);
    Send(WM_SYSCOMMAND, SC_MAXIMIZE);
    return TRUE;
}

CLinearGraphView* CLinearGraphFrameWnd::GetActiveView()
{
    if( !m_toolPanel.GetTabCount() )
    {
        return 0;
    }
    return m_vpView[m_toolPanel.GetActiveViewTab()];
}

BOOL CLinearGraphFrameWnd::GetCustomizedColor(COLORREF& clr) const
{
    COLORREF custClr[16] = {0xFFFFFFFF};

    CHOOSECOLOR chclr = {0};
    chclr.lStructSize = sizeof(CHOOSECOLOR);
    chclr.Flags = CC_RGBINIT;
    chclr.hwndOwner = m_hWnd;
    chclr.rgbResult = clr;
    chclr.lpCustColors = custClr;

    if( ChooseColor(&chclr) )
    {
        clr = chclr.rgbResult;
        return TRUE;
    }
    return FALSE;
}

BOOL CLinearGraphFrameWnd::OpenDocument(PWSTR szDocName)
{
    return Send(LinearGraphOpenDocument,
        (WPARAM)wcslen(szDocName), (LPARAM)szDocName);
}

CLinearGraphView* CLinearGraphFrameWnd::FindAttachedView(CDataObjectPtr& pdo)
{
    for(size_t i = 0; i < m_vpView.size(); ++i)
    {
        if( m_vpView[i]->GetDataObject() == pdo )
        {
            return m_vpView[i];
        }
    }
    return 0;
}

BOOL CLinearGraphFrameWnd::UpdateStatusPanel()
{
    m_statusPanel.ClearInformation();

    CString strText;
    if( m_pLastActiveView )
    {
        m_statusPanel.SetInformation(
            m_pLastActiveView->GetDataObject());
        strText = m_pLastActiveView->GetDataObject()->name;
        strText.Append(L" - ");
    }

    WCHAR appCaptioinString[64];
    GetString(IDS_APP_CAPTION, appCaptioinString, 128);
    strText.Append(appCaptioinString);
    SetText(strText);
    return TRUE;
}

void CLinearGraphFrameWnd::UpdateWindowLayout()
{
    RECT rcClient;
    GetClientRect(rcClient);

    const int toolPanelHeight   = 28;
    const int statusPanelHeight = 24;

    int cx = rcClient.right;
    int cy = rcClient.bottom - toolPanelHeight - statusPanelHeight;

    m_toolPanel.SetPlacement(LeftTop, cx, toolPanelHeight);
    m_statusPanel.SetPlacement(LeftBottom, cx, statusPanelHeight);

    if( m_layoutStyle == TabbedDoc )
    {
        for(size_t i = 0; i < m_vpView.size(); i++ )
        {
            if( m_vpView[i] != m_pLastActiveView )
            {
                m_vpView[i]->ShowWindow(SW_HIDE);
            }
            m_vpView[i]->SetPlacement(0, toolPanelHeight, cx, cy);
        }
    }
    else
    {
        int viewWidth  = cx / m_layoutStyle;
        int viewHeight =
            m_vpView.size() ? cy/((m_vpView.size()+m_layoutStyle-1)/m_layoutStyle) : cy;

        for(size_t i = 0; i < m_vpView.size(); i++ )
        {
            m_vpView[i]->SetPlacement(
                viewWidth*(i%m_layoutStyle), toolPanelHeight + viewHeight*(i/m_layoutStyle),
                viewWidth, viewHeight
                );
            m_vpView[i]->ShowWindow(SW_SHOW);
        }
    }
}

size_t CLinearGraphFrameWnd::GetDataSourceCount() const
{
    return m_vpData.size();
}

size_t CLinearGraphFrameWnd::GetGraphViewCount() const
{
    return m_vpView.size();
}

int  CLinearGraphFrameWnd::PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch( msg )
	{
    case LinearGraphOpenDocument:
        {
            if( (size_t)wp != wcslen((PCWSTR)lp) )
            {
                return FALSE;
            }
            return OnOpenDocument((PCWSTR)lp);
        }
        break;
    case LinearGraphViewActivate:
        OnActivateView((CLinearGraphView*)lp);
        break;
    case LinearGraphTabSelectChange:
        {
            if( wp == CLinearGraphToolPanel::IDC_TABPANEL )
            {
                OnActiveViewChanged();
            }
        }
        break;

    case WM_SIZING:
        OnSizing((DWORD)wp, (PRECT)lp);
        break;
    case WM_SIZE:
        OnSizeChanged((DWORD)wp, LOWORD(lp), HIWORD(lp));
        break;
    case WM_COMMAND:
        OnCommand(HIWORD(wp), LOWORD(wp), (HWND)lp);
        break;
    case WM_DROPFILES:
        OnDropFiles((HDROP)wp);
        break;
    case WM_CONTEXTMENU:
        if( !OnContextMenu(LOWORD(lp), HIWORD(lp)) )
        {
            return CWindow::PreProcessMessage(msg, wp, lp);
        }
        break;
    case WM_INITMENUPOPUP:
        OnInitMenuPopup((HMENU)wp, LOWORD(lp), HIWORD(lp));
        break;
    case WM_CREATE:
        OnCreate((LPCREATESTRUCT)lp);
        break;
    case WM_DESTROY:
        OnFileCloseAll();
        ::PostQuitMessage(0);
        break;
    default:
        return CWindow::PreProcessMessage(msg, wp, lp);
    }
	return 0;
}

void CLinearGraphFrameWnd::OnCreate(LPCREATESTRUCT lpcs)
{
    SetMenu(::LoadMenuW(GetInstance(), MAKEINTRESOURCEW(IDR_MENU_MAIN)));

    m_toolPanel.Create(this);
    m_statusPanel.Create(this);
}

void CLinearGraphFrameWnd::OnSizeChanged(DWORD dwFlags, int cx, int cy)
{
    UpdateWindowLayout();
}

void CLinearGraphFrameWnd::OnSizing(DWORD dwFlag, PRECT pRcWnd)
{
    if( pRcWnd->right - pRcWnd->left < 540 )
    {
        if( dwFlag == WMSZ_RIGHT || dwFlag == WMSZ_BOTTOMRIGHT
            || dwFlag == WMSZ_TOPRIGHT )
        {
            pRcWnd->right = pRcWnd->left + 540;
        }
        else{ pRcWnd->left = pRcWnd->right - 540; }
    }
    if( pRcWnd->bottom - pRcWnd->top < 360 )
    {
        if( dwFlag == WMSZ_BOTTOM || dwFlag == WMSZ_BOTTOMRIGHT
            || dwFlag == WMSZ_BOTTOMLEFT )
        {
            pRcWnd->bottom = pRcWnd->top + 360;
        }
        else{ pRcWnd->top = pRcWnd->bottom - 360; }
    }
}

void CLinearGraphFrameWnd::OnPaint(HDC dc, PAINTSTRUCT& ps)
{
    if( !m_vpView.size() )
    {
        ::FillRect(dc, &ps.rcPaint, CLinearGraphApp::GetThemeBrush());
        ::DeleteObject(::SelectObject(dc, CLinearGraphApp::GetThemeFont()));
        ::SetTextColor(dc, RGB(255,255,255));
        ::SetBkMode(dc, TRANSPARENT);

        RECT rcVersion;
        GetClientRect(rcVersion);
        rcVersion.right -= 20;
        rcVersion.bottom -= 30;
        rcVersion.top = rcVersion.bottom - 20;

#define LINEAR_GRAPH_BUILD_DATETIME "Build Date: "__DATE__##" "##__TIME__##" "

        ::DrawTextA(dc, LINEAR_GRAPH_BUILD_DATETIME,
            (int)::strlen(LINEAR_GRAPH_BUILD_DATETIME),
            &rcVersion, DT_RIGHT|DT_WORD_ELLIPSIS);
        ::SetBkMode(dc, OPAQUE);
    }
}

void CLinearGraphFrameWnd::OnCommand(UINT uCode, UINT uID, HWND hCtrl)
{
    switch( uID )
    {
    case ID_FILE_OPENDATA:      return OnFileOpen();
    case ID_FILE_CLOSEDATA:     return OnFileClose();
    case ID_FILE_CLOSEALL:      return OnFileCloseAll();
    case ID_FILE_SAVEDATA:      return OnFileSave();
    case ID_FILE_SAVEIMAGE:     return OnFileSaveImage();
    case ID_FILE_SAVEALL_IMAGE: return OnFileSaveAllImages();
    case ID_FILE_EXPORT_DATA:   return OnFileExportData();
    case ID_FILE_EXITAPP:       return OnFileExit();

    case ID_EDIT_RENAME:        return OnEditRename();
    case ID_EDIT_DATETIME:      return OnEditDateTime();
    case ID_EDIT_RELOAD:        return OnEditReload();

    case ID_LAYOUT_TAB:         return OnLayoutTab();
    case ID_LAYOUT_SINGLECOL:   return OnLayoutSingleCol();
    case ID_LAYOUT_DICOL:       return OnLayoutDiCol();
    case ID_LAYOUT_TRECOL:      return OnLayoutTreCol();

    case ID_VIEW_FOREGNDCLR:    return OnViewForegndClr();
    case ID_VIEW_BKGNDCLR:      return OnViewBkgndClr();
    case ID_VIEW_RULER:         return OnViewRuler();
    case ID_VIEW_HORZLINE:      return OnViewHorzLine();
    case ID_VIEW_ZOOMIN:        return OnViewZoomIn();
    case ID_VIEW_ZOOMOUT:       return OnViewZoomOut();
    case ID_VIEW_RESTORE:       return OnViewRestore();
    case ID_VIEW_POSLABEL:      return OnViewPosLabel();
    case ID_VIEW_COORD:         return OnViewCoord();
    case ID_VIEW_LEGEND:        return OnViewLegend();
    case ID_VIEW_CLEARCMP:      return OnViewClearCmp();

    case ID_ANALYZE_COMPARE:    return OnAnalyzeCompare();
    case ID_ANALYZE_SYNC:       return OnAnalyzeSyncSelect();
    case ID_ANALYZE_LINEAR:     return OnAnalyzeLinear();
    case ID_ANALYZE_POWER:      return OnAnalyzePower();
    case ID_ANALYZE_DIFF:       return OnAnalyzeDiff();

    case ID_OPT_TRANSLUCENTSEL: return OnOptTranslucentSel();
    case ID_OPT_SELECTTRANSFORM:return OnOptSelectTransform();
    
    case ID_HELP_ABOUT:         return OnHelpAbout();
    case ID_HELP_HELP:          return OnHelpHelp();
    }
}

BOOL CLinearGraphFrameWnd::OnContextMenu(int x, int y)
{
    if( !m_pLastActiveView || !m_pLastActiveView->PtInWindow(x, y) )
    {
        return FALSE;
    }
    
    if( HMENU hMenu = ::GetSubMenu(::GetMenu(m_hWnd), 3) )
    {
        OnInitMenuPopup(hMenu, 3, FALSE);
        return ::TrackPopupMenu(hMenu, 0, x, y, 0, m_hWnd, 0);
    }
    return FALSE;
}

void CLinearGraphFrameWnd::OnInitMenuPopup(HMENU hMenu, UINT nPos, BOOL bIsWindowMenu)
{
    MENUITEMINFOW mii = {0};
    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_STATE;

    switch( nPos )
    {
    case FileMenu:
        {
            mii.fState = m_vpView.size() ? 0 : MFS_GRAYED;
            ::SetMenuItemInfo(hMenu, ID_FILE_CLOSEDATA, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_FILE_CLOSEALL, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_FILE_SAVEIMAGE, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_FILE_SAVEDATA, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_FILE_SAVEALL_IMAGE, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_FILE_EXPORT_DATA, FALSE, &mii);
        }
        break;
    case EditMenu:
        {
            mii.fState = m_vpView.size() ? 0 : MFS_GRAYED;
            ::SetMenuItemInfo(hMenu, ID_EDIT_RENAME, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_EDIT_DATETIME, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_EDIT_RELOAD, FALSE, &mii);
        }
        break;
    case LayoutMenu:
        {
            if( m_layoutStyle >= 4 )
            {
                m_layoutStyle = TabbedDoc;
            }

            UINT stateMap[4] = {MFS_UNCHECKED};
            stateMap[m_layoutStyle] = MFS_CHECKED;

            mii.fState = stateMap[TabbedDoc];
            ::SetMenuItemInfo(hMenu, ID_LAYOUT_TAB, FALSE, &mii);

            mii.fState = stateMap[SingleColumn];
            ::SetMenuItemInfo(hMenu, ID_LAYOUT_SINGLECOL, FALSE, &mii);

            mii.fState = stateMap[DoubleColumn] | (m_vpView.size()<2 ? MFS_GRAYED : 0);
            ::SetMenuItemInfo(hMenu, ID_LAYOUT_DICOL, FALSE, &mii);

            mii.fState = stateMap[TrebleColumn] | (m_vpView.size()<3 ? MFS_GRAYED : 0);
            ::SetMenuItemInfo(hMenu, ID_LAYOUT_TRECOL, FALSE, &mii);
        }
        break;
    case ViewMenu:
        {
            mii.fState = m_vpView.size() ? 0 : MFS_GRAYED;
            ::SetMenuItemInfo(hMenu, ID_VIEW_FOREGNDCLR, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_VIEW_BKGNDCLR, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_VIEW_RESTORE, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_VIEW_CLEARCMP, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_VIEW_ZOOMIN, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_VIEW_ZOOMOUT, FALSE, &mii);

            mii.fState = (m_dwViewOpts & CLinearGraphView::OptRuler) ? MFS_CHECKED : 0;
            ::SetMenuItemInfo(hMenu, ID_VIEW_RULER, FALSE, &mii);

            mii.fState = (m_dwViewOpts & CLinearGraphView::OptHorizontalLine) ? MFS_CHECKED : 0;
            ::SetMenuItemInfo(hMenu, ID_VIEW_HORZLINE, FALSE, &mii);

            mii.fState = (m_dwViewOpts & CLinearGraphView::OptCoordinates) ? MFS_CHECKED : 0;
            ::SetMenuItemInfo(hMenu, ID_VIEW_COORD, FALSE, &mii);

            mii.fState = (m_dwViewOpts & CLinearGraphView::OptLegend) ? MFS_CHECKED : 0;
            ::SetMenuItemInfo(hMenu, ID_VIEW_LEGEND, FALSE, &mii);

            mii.fState = (m_dwViewOpts & CLinearGraphView::OptPositionLabel) ? MFS_CHECKED : 0;
            ::SetMenuItemInfo(hMenu, ID_VIEW_POSLABEL, FALSE, &mii);
        }
        break;
    case AnalyzeMenu:
        {
            mii.fState = m_vpView.size() > 1 ? 0 : MFS_GRAYED;
            ::SetMenuItemInfo(hMenu, ID_ANALYZE_COMPARE, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_ANALYZE_SYNC, FALSE, &mii);

            mii.fState = m_vpView.size() ? 0 : MFS_GRAYED;
            ::SetMenuItemInfo(hMenu, ID_ANALYZE_LINEAR, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_ANALYZE_POWER, FALSE, &mii);
            ::SetMenuItemInfo(hMenu, ID_ANALYZE_DIFF, FALSE, &mii);
        }
        break;
    case OptionMenu:
        {
            mii.fState = (m_dwViewOpts & CLinearGraphView::OptSelectionTransform) ? MFS_CHECKED : 0;
            ::SetMenuItemInfo(hMenu, ID_OPT_SELECTTRANSFORM, FALSE, &mii);
            mii.fState = (m_dwViewOpts & CLinearGraphView::OptTranslucentSelection) ? MFS_CHECKED : 0;
            ::SetMenuItemInfo(hMenu, ID_OPT_TRANSLUCENTSEL, FALSE, &mii);
        }
        break;
    case HelpMenu:
        break;
    }
}

void CLinearGraphFrameWnd::OnDropFiles(HDROP hDrop)
{
    LGTRACE_FUNCTION();

    int     cFiles;
    int     cchSize;
    PWCHAR  fileName;

    cFiles = ::DragQueryFile(hDrop, 0xFFFFFFFF, 0, 0);
    LGTRACE("%d documents by drop", cFiles);

    for(int i = 0; i < cFiles; i++)
    {
        cchSize = ::DragQueryFile(hDrop, i, 0, 0);
        if( fileName = new WCHAR[cchSize + 2] )
        {
            ::DragQueryFile(hDrop, i, fileName, cchSize+2);
            if( !OpenDocument(fileName) )
            {
                WCHAR errString[128];
                GetString(IDS_ERR_OPEN_FAILED, errString, 128);

                CString strMsg;
                strMsg.Format(L"%s\n%s", errString, fileName);
                MessageBox(strMsg, MB_ICONWARNING);
            }
            delete[] fileName;
        }
    }

    ::DragFinish(hDrop);
}

BOOL CLinearGraphFrameWnd::OnOpenDocument(PCWSTR szFileName)
{
    LGTRACE_FUNCTION();

    BeginHeavyTask();
    EnterHeavyOpenTask(szFileName);

    CLinearGraphDoc doc;
    if( !doc.Open(szFileName) )
    {
        LGTRACE("FAILURE %ws", szFileName);
        LGFLUSH();
        CLinearGraphApp::GetApp()->EndHeavyTask();
        return FALSE;
    }

    for(int i = 0; i < doc.GetSampleCount(); i++)
    {
        if( m_pLastActiveView && (TabbedDoc == m_layoutStyle) )
        {
            m_pLastActiveView->ShowWindow(SW_HIDE);
        }

        CDataObjectPtr pData = doc.GetSample(i);
        if( !pData )
        {
            continue ;
        }

        CLinearGraphViewPtr pView = CLinearGraphView::DynamicCreate(this);
        if( !pView  )
        {
            ShowErrorBox(IDS_ERR_GMEM_OVER, IDS_ERR_OUT_OF_MEMORY);
            break;
        }

        pView->SetOptions(m_dwViewOpts, FALSE);
        pView->BindData(pData);

        m_vpData.push_back(pData);
        m_vpView.push_back(pView);

        m_pLastActiveView = pView;
        m_toolPanel.SetActiveViewTab( m_toolPanel.AddTab(pData->name) );
    }
    EndHeavyTask();

    UpdateWindowLayout();
    UpdateStatusPanel();
    LGTRACE("SUCCESS %ws", szFileName);
    return TRUE;
}

void CLinearGraphFrameWnd::OnActivateView(CLinearGraphView* pView)
{
    LGTRACE_FUNCTION();

    if( m_pLastActiveView == pView )
    {
        return ;
    }
    
    for(size_t i = 0; i < m_vpView.size(); ++i)
    {
        if( pView == m_vpView[i] )
        {
            m_toolPanel.SetActiveViewTab(i);
        }
    }
    OnActiveViewChanged();
}

void CLinearGraphFrameWnd::OnActiveViewChanged()
{
    LGTRACE_FUNCTION();

    if( m_pLastActiveView && (TabbedDoc == m_layoutStyle))
    {
        m_pLastActiveView->ShowWindow(SW_HIDE);
    }
    m_pLastActiveView = GetActiveView();
    if( m_pLastActiveView )
    {
        m_pLastActiveView->ShowWindow(SW_SHOW);
    }
    UpdateStatusPanel();
}

void CLinearGraphFrameWnd::OnFileOpen()
{
    LGTRACE_FUNCTION();

    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = L"Seismic Data File\0*.BINX;*.TXT;*.SEED;*.MSEED\0All files\0*.*\0\0";
    ofn.lpstrTitle = L"Open Seismic Data";
    ofn.Flags = OFN_ALLOWMULTISELECT|OFN_EXPLORER|OFN_FILEMUSTEXIST;

    WCHAR nameBuff[8192] = {0};
    ofn.lpstrFile = nameBuff;
    ofn.nMaxFile = 8190;
    if( !GetOpenFileNameW(&ofn) ){ return; }

    CString dirName;
    dirName.Append(nameBuff, ofn.nFileOffset);
    if( nameBuff[ofn.nFileOffset-1] != L'\\' )
    {
        dirName.AppendChar(L'\\');
    }

    WCHAR fileName[8192];
    for(size_t off = ofn.nFileOffset; off < 8190; off += wcslen(nameBuff+off) + 1)
    {
        if( !nameBuff[off] )
        {
            break;
        }

        wcscpy_s(fileName, 8192, dirName);
        wcscat_s(fileName, 8192, nameBuff+off);

        if( !OpenDocument(fileName) )
        {
            CString infoString;
            infoString.Format(L"Failed to open document:\n%s", fileName);
            MessageBox(infoString, MB_ICONERROR|MB_OK);
        }
    }
}

void CLinearGraphFrameWnd::OnFileSaveImage()
{
    LGTRACE_FUNCTION();

    CLinearGraphViewPtr pView = GetActiveView();
    if( !pView )
    {
        return ;
    }

    CPrintGraphDlg dlg;
    if( IDOK != dlg.DoModal(this) )
    {
        return;
    }

    if( !pView->PrintGraph(dlg.m_dirName, dlg.m_extensionName) )
    {
        CString info(L"Failed to save the graph:\n");
        info.AppendFormat(L"Name: %s\n", (PCWSTR)pView->GetDataObject()->name);
        info.AppendFormat(L"Format: %s", (PCWSTR)dlg.m_extensionName);
        MessageBox(info, MB_ICONERROR|MB_OK);
    }
    else{ ::MessageBeep(MB_ICONINFORMATION); }

}

void CLinearGraphFrameWnd::OnFileSaveAllImages()
{
    LGTRACE_FUNCTION();

    CLinearGraphViewPtr pView = GetActiveView();
    if( !pView )
    {
        return ;
    }

    CPrintGraphDlg dlg;
    if( IDOK != dlg.DoModal(this) )
    {
        return;
    }

    for(size_t i = 0; i < m_vpView.size(); ++i)
    {
        pView->PrintGraph(dlg.m_dirName, dlg.m_extensionName);
    }

    ::MessageBeep(MB_ICONINFORMATION);
}

void CLinearGraphFrameWnd::OnFileClose()
{
    LGTRACE_FUNCTION();

    if( !m_pLastActiveView )
    {
        return ;
    }

    for(size_t i = 0; i < m_vpView.size(); ++i)
    {
        if( m_vpView[i] == m_pLastActiveView )
        {
            m_vpView.erase( m_vpView.begin()+i );
            m_vpData.erase( m_vpData.begin()+i );
            m_toolPanel.RemoveViewTab((int)i);
            m_pLastActiveView->Close();
            break;
        }
    }

    m_pLastActiveView = GetActiveView();
    if( m_pLastActiveView )
    {
        m_toolPanel.SetActiveViewTab(0);
        m_pLastActiveView->ShowWindow();
    }

    UpdateWindowLayout();
    UpdateStatusPanel();
}

void CLinearGraphFrameWnd::OnFileCloseAll()
{
    LGTRACE_FUNCTION();

    m_toolPanel.RemoveAllViewTabs();
    m_pLastActiveView = 0;

    for(size_t i = 0; i < m_vpView.size(); i++)
    {
        m_vpView[i]->Close();
    }

    m_vpView.clear();
    m_vpData.clear();
    UpdateStatusPanel();
}

void CLinearGraphFrameWnd::OnFileExportData()
{
    LGTRACE_FUNCTION();

    if( !m_pLastActiveView || !m_pLastActiveView->GetDataObject() )
    {
        return ;
    }

    CSaveDataDlg dlg(m_pLastActiveView->GetDataObject()->hasTimestamp());
    dlg.SetCaption(IDS_EXPORT_DATA);
    if( dlg.DoModal(this) != IDOK )
    {
        return ;
    }

    CRectangle rcClip;
    m_pLastActiveView->GetClipRectangle(rcClip);
    
    if( SaveDataToFile(m_pLastActiveView->GetDataObject(), &rcClip,
        dlg.m_fCheckOverwrite, dlg.m_iFormat, dlg.m_dirName) )
    {
        ::MessageBeep(MB_ICONINFORMATION);
    }
}

void CLinearGraphFrameWnd::OnFileSave()
{
    LGTRACE_FUNCTION();

    if( !m_pLastActiveView || !m_pLastActiveView->GetDataObject() )
    {
        return ;
    }

    CSaveDataDlg dlg(m_pLastActiveView->GetDataObject()->hasTimestamp());
    dlg.SetCaption(IDS_SAVE_DATA);
    if( dlg.DoModal(this) != IDOK )
    {
        return ;
    }

    if( SaveDataToFile(m_pLastActiveView->GetDataObject(), 0,
        dlg.m_fCheckOverwrite, dlg.m_iFormat, dlg.m_dirName) )
    {
        ::MessageBeep(MB_ICONINFORMATION);
    }
}

BOOL CLinearGraphFrameWnd::SaveDataToFile(const CDataObjectPtr pdo, CRectangle* pClip,
    BOOL bCheckOverwrite, DWORD dwFormat, CString& dirName)
{
    LGTRACE_FUNCTION();
    
    if( !pdo->hasTimestamp() && dwFormat == CSaveDataDlg::DataTXTWithTimestamp )
    {
        return FALSE;
    }

    CString fileName(dirName);
    fileName.Append(pdo->name);
    if( pClip )
    {
        if( pClip->xMin < 0 ){ pClip->xMin = 0; }
        if( pClip->xMax > pdo->length ){ pClip->xMax = pdo->length; }

        fileName.AppendChar(L'.');
        fileName.AppendFormat(L"[%d-%d]", pClip->xMin, pClip->xMax);
    }

    switch( dwFormat )
    {
    case CSaveDataDlg::DataBIN:
        fileName.Append(L".binx");
        break;
    case CSaveDataDlg::DataTXT:
    case CSaveDataDlg::DataTXTWithTimestamp:
        fileName.Append(L".txt");
        break;
    default:
        return FALSE;
    }

    HANDLE hFile;
    if( !CreateWithCheck(hFile, fileName, bCheckOverwrite))
    {
        ShowErrorBox(IDS_ERR_SAVE_FAILED, IDS_ACCESS_DENIED);
        return FALSE;
    }
    
    if( hFile == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    // These variables are copied for fast access
    //
    DWORD freqBase = pdo->freqBase;
    DWORD freqMulti = pdo->freqMulti;

    LONGLONG timeStamp = pdo->timeStamp;
    if( pClip )
    {
        timeStamp += pClip->xMin * (LONGLONG)10000000 * freqBase / freqMulti;
    }

    LONG* pData = pdo->data + (pClip ? pClip->xMin : 0);
    LONG  cData = pClip ? pClip->Width() : pdo->length;

    BeginHeavyTask();

    char  buff[128];
    DWORD dwBytes = 0;
    switch( dwFormat )
    {
    case CSaveDataDlg::DataTXT:
        {
            for(LONG i = 0; i < cData; ++i)
            {
                ::WriteFile(hFile, buff, sprintf(buff, "%d\n", pData[i]), &dwBytes, 0);
            }
        }
        break;
    case CSaveDataDlg::DataBIN:
        {
            CBinSampleFile::HeaderStruct hs = {0};
            strncpy(hs.dataType, "I32\0", 4);
            hs.freqBase = freqBase;
            hs.freqMulti = freqMulti;
            hs.timeStamp = *(FILETIME*)&timeStamp;
            ::WriteFile(hFile, &hs, sizeof(hs), &dwBytes, 0);
            ::WriteFile(hFile, pData, cData*sizeof(LONG), &dwBytes, 0);
        }
        break;
    case CSaveDataDlg::DataTXTWithTimestamp:
        {
            SYSTEMTIME sysTime;
            if( freqMulti / freqBase > 1 )
            {
                for(LONG i = 0; i < cData; ++i)
                {
                    ::FileTimeToSystemTime((FILETIME*)&timeStamp, &sysTime);
                    dwBytes = (DWORD)sprintf(
                        buff, "%04d%02d%02d%02d%02d%02d%03d %d\n",
                        (DWORD)sysTime.wYear, (DWORD)sysTime.wMonth, (DWORD)sysTime.wDay,
                        (DWORD)sysTime.wHour, (DWORD)sysTime.wMinute, (DWORD)sysTime.wSecond,
                        (DWORD)sysTime.wMilliseconds, pData[i]);
                    ::WriteFile(hFile, buff, dwBytes, &dwBytes, 0);
                    timeStamp += 10000000 * freqBase / freqMulti;
                }
            }
            else
            {
                for(LONG i = 0; i < cData; ++i)
                {
                    ::FileTimeToSystemTime((FILETIME*)&timeStamp, &sysTime);
                    dwBytes = (DWORD)sprintf(
                        buff, "%04d%02d%02d%02d%02d%02d %d\n",
                        (DWORD)sysTime.wYear, (DWORD)sysTime.wMonth, (DWORD)sysTime.wDay,
                        (DWORD)sysTime.wHour, (DWORD)sysTime.wMinute, (DWORD)sysTime.wSecond,
                        pData[i]);
                    ::WriteFile(hFile, buff, dwBytes, &dwBytes, 0);
                    timeStamp += 10000000 * freqBase / freqMulti;
                }
            }
        }
        break;
    default:
        return FALSE;
    }
    ::CloseHandle(hFile);

    EndHeavyTask();
    return TRUE;
}

void CLinearGraphFrameWnd::OnEditRename()
{
    LGTRACE_FUNCTION();

    if( CLinearGraphViewPtr pView = GetActiveView() )
    {
        CNameDialog dlg;
        if( pView->GetNameFontSize(dlg.m_strName, dlg.m_lFontHeight)
            && IDOK == dlg.DoModal(this))
        {
            pView->SetNameFontSize(dlg.m_strName, dlg.m_lFontHeight);
        }
    }
}

void CLinearGraphFrameWnd::OnEditDateTime()
{
    LGTRACE_FUNCTION();

    if( CLinearGraphViewPtr pView = GetActiveView() )
    {
        CDateTimeDialog dlg;
        pView->GetDateTime(dlg.m_stDateTime, dlg.m_dwFreqBase, dlg.m_dwFreqMulti);

        if( dlg.DoModal(this) == IDOK )
        {
            pView->SetDateTime(dlg.m_stDateTime, dlg.m_dwFreqBase, dlg.m_dwFreqMulti);
            UpdateStatusPanel();
        }
    }
}

void CLinearGraphFrameWnd::OnEditReload()
{
    LGTRACE_FUNCTION();

    CDataObjectPtr pdo = m_pLastActiveView->GetDataObject();
    if( !m_pLastActiveView || !pdo )
    {
        return ;
    }

    BeginHeavyTask();
    EnterHeavyOpenTask(pdo->ownerFile);
   
    CLinearGraphDoc doc;
    if( doc.Open(pdo->ownerFile) )
    {
        if( pdo->indexInDoc < (size_t)doc.GetSampleCount() )
        {
            CLinearGraphApp::GetApp()->EnterHeavyTaskStep(IDS_PREPARE_VIEW);
            m_pLastActiveView->BindData(doc.GetSample(pdo->indexInDoc));
        }
        else{ ShowErrorBox(IDS_ERR_DOC_CHANGED, IDS_APP_CAPTION); }
    }
    else{ ShowErrorBox(IDS_ERR_OPEN_FAILED, IDS_APP_CAPTION); }
    
    EndHeavyTask();
}

void CLinearGraphFrameWnd::OnLayoutTab()
{
    LGTRACE_FUNCTION();

    m_layoutStyle = TabbedDoc;
    UpdateWindowLayout();
}

void CLinearGraphFrameWnd::OnLayoutSingleCol()
{
    LGTRACE_FUNCTION();

    m_layoutStyle = SingleColumn;
    UpdateWindowLayout();
}

void CLinearGraphFrameWnd::OnLayoutDiCol()
{
    LGTRACE_FUNCTION();

    m_layoutStyle = DoubleColumn;
    UpdateWindowLayout();
}

void CLinearGraphFrameWnd::OnLayoutTreCol()
{
    LGTRACE_FUNCTION();

    m_layoutStyle = TrebleColumn;
    UpdateWindowLayout();
}

void CLinearGraphFrameWnd::OnViewForegndClr()
{
    LGTRACE_FUNCTION();

    if( CLinearGraphViewPtr pView = GetActiveView() )
    {
        COLORREF clr = pView->GetForegroundColor();
        if( GetCustomizedColor(clr) )
        {
            pView->SetForegroundColor(clr, TRUE);
        }
    }
}

void CLinearGraphFrameWnd::OnViewBkgndClr()
{
    LGTRACE_FUNCTION();

    if( CLinearGraphViewPtr pView = GetActiveView() )
    {
        COLORREF clr = pView->GetBackgroundColor();
        if( GetCustomizedColor(clr) )
        {
            pView->SetBackgroundColor(clr, TRUE);
        }
    }
}

void CLinearGraphFrameWnd::OnViewRuler()
{
    LGTRACE_FUNCTION();

    m_dwViewOpts ^= CLinearGraphView::OptRuler;
    UpdateViewOptions();
}

void CLinearGraphFrameWnd::OnViewHorzLine()
{
    LGTRACE_FUNCTION();

    m_dwViewOpts ^= CLinearGraphView::OptHorizontalLine;
    UpdateViewOptions();
}

void CLinearGraphFrameWnd::OnViewCoord()
{
    LGTRACE_FUNCTION();

    m_dwViewOpts ^= CLinearGraphView::OptCoordinates;
    UpdateViewOptions();
}

void CLinearGraphFrameWnd::OnViewLegend()
{
    LGTRACE_FUNCTION();

    m_dwViewOpts ^= CLinearGraphView::OptLegend;
    UpdateViewOptions();
}

void CLinearGraphFrameWnd::OnViewPosLabel()
{
    LGTRACE_FUNCTION();

    m_dwViewOpts ^= CLinearGraphView::OptPositionLabel;
    UpdateViewOptions();
}

void CLinearGraphFrameWnd::OnOptTranslucentSel()
{
    LGTRACE_FUNCTION();

    m_dwViewOpts ^= CLinearGraphView::OptTranslucentSelection;
    UpdateViewOptions();
}

void CLinearGraphFrameWnd::OnOptSelectTransform()
{
    LGTRACE_FUNCTION();

    m_dwViewOpts ^= CLinearGraphView::OptSelectionTransform;
    UpdateViewOptions();
}

void CLinearGraphFrameWnd::OnViewRestore()
{
    LGTRACE_FUNCTION();

    if( CLinearGraphViewPtr pView = GetActiveView() )
    {
        CRectangle rcGraph;
        pView->UpdateGraphRectangle(TRUE);
        pView->Refresh();
    }
}

void CLinearGraphFrameWnd::OnViewClearCmp()
{
    LGTRACE_FUNCTION();

    if( CLinearGraphViewPtr pView = GetActiveView() )
    {
        pView->ClearComparison();
    }
}

void CLinearGraphFrameWnd::OnViewZoomIn()
{
    LGTRACE_FUNCTION();

    if( CLinearGraphViewPtr pView = GetActiveView() )
    {
        pView->ZoomIn();
    }
}

void CLinearGraphFrameWnd::OnViewZoomOut()
{
    LGTRACE_FUNCTION();

    if( CLinearGraphViewPtr pView = GetActiveView() )
    {
        pView->ZoomOut();
    }
}

void CLinearGraphFrameWnd::OnAnalyzeCompare()
{
    LGTRACE_FUNCTION();

    if( !m_pLastActiveView )
    {
        return ;
    }

    const CDataObjectPtr& pdo = m_pLastActiveView->GetDataObject();

    CSourceSelectDlg dlg;
    for(size_t i = 0; i < m_vpData.size(); ++i)
    {
        if( m_vpData[i] != pdo && m_vpData[i]->isCompatibleWith(pdo) )
        {
            dlg.m_vSource.push_back(m_vpData[i]);
        }
    }
    dlg.m_vChoice.assign(dlg.m_vSource.size(), false);

    if( IDOK != dlg.DoModal(this) )
    {
        return ;
    }

    CLinearGraphViewPtr pComp;
    for (size_t i = 0; i < dlg.m_vSource.size(); ++i)
    {
        if( dlg.m_vChoice[i] && (pComp = FindAttachedView(dlg.m_vSource[i])) )
        {
            m_pLastActiveView->AddComparison(*pComp, FALSE);
        }
    }
    m_pLastActiveView->Refresh();
}

void CLinearGraphFrameWnd::OnAnalyzeSyncSelect()
{
    LGTRACE_FUNCTION();

    if( !m_pLastActiveView )
    {
        return ;
    }

    const CDataObjectPtr& pdo = m_pLastActiveView->GetDataObject();

    CSourceSelectDlg dlg;
    for(size_t i = 0; i < m_vpData.size(); ++i)
    {
        if( m_vpData[i] != pdo && m_vpData[i]->isCompatibleWith(pdo) )
        {
            dlg.m_vSource.push_back(m_vpData[i]);
        }
    }
    dlg.m_vChoice.assign(dlg.m_vSource.size(), false);

    if( IDOK != dlg.DoModal(this) )
    {
        return ;
    }

    CRectangle rcClip;
    m_pLastActiveView->GetClipRectangle(rcClip);

    CLinearGraphViewPtr pDst;
    for (size_t i = 0; i < dlg.m_vSource.size(); ++i)
    {
        if( dlg.m_vChoice[i] && (pDst = FindAttachedView(dlg.m_vSource[i])) )
        {
            pDst->SetClipRectangle(rcClip);
            pDst->Refresh();
        }
    }
}

void CLinearGraphFrameWnd::OnAnalyzeLinear()
{
    LGTRACE_FUNCTION();

    if( !m_pLastActiveView || !m_pLastActiveView->GetDataObject() )
    {
        return ;
    }

    LONG* pData = m_pLastActiveView->GetDataObject()->data;
    LONG  len = m_pLastActiveView->GetDataObject()->length;
    
    CLinearTransformDialog dlg;
    if( IDOK != dlg.DoModal(this) )
    {
        return ;
    }

    DOUBLE k = dlg.m_K;
    DOUBLE c = dlg.m_C;

    CRectangle dataRect;
    m_pLastActiveView->GetDataRectangle(dataRect);

    double upl = k * dataRect.yMax + c;
    double lol = k * dataRect.yMin + c;

    if( fabs(upl) > (double)0x100000000 || fabs(lol) > (double)0x100000000
        || fabs(upl - lol) < (double)1 )
    {
        ShowErrorBox(IDS_NUMERIC_OVERFLOW, IDS_INVALID_ARGS);
        return ;
    }

    BeginHeavyTask();
    for(LONG i = 0; i < len; ++i)
    {
        pData[i] = (LONG)((DOUBLE)pData[i] * k + c + 0.5);
    }

    EnterHeavyTaskStep(IDS_PREPARE_VIEW);
    m_pLastActiveView->UpdateGraphRectangle(TRUE);
    m_pLastActiveView->Refresh();
    EndHeavyTask();
}

void CLinearGraphFrameWnd::OnAnalyzePower()
{
    LGTRACE_FUNCTION();

    if( !m_pLastActiveView || !m_pLastActiveView->GetDataObject() )
    {
        return ;
    }

    LONG* pData = m_pLastActiveView->GetDataObject()->data;
    LONG  len = m_pLastActiveView->GetDataObject()->length;

    CRectangle dataRect;
    m_pLastActiveView->GetDataRectangle(dataRect);

    if( dataRect.yMax >= 46340 || dataRect.yMin <= -46340 )
    {
        ShowErrorBox(IDS_NUMERIC_OVERFLOW, IDS_ACTION_STOPPED);
        return ;
    }

    BeginHeavyTask();
    for(LONG i = 0; i < len; ++i)
    {
        pData[i] *= pData[i];
    }

    EnterHeavyTaskStep(IDS_PREPARE_VIEW);
    m_pLastActiveView->UpdateGraphRectangle(TRUE);
    m_pLastActiveView->Refresh();
    EndHeavyTask();
}

void CLinearGraphFrameWnd::OnAnalyzeDiff()
{
    LGTRACE_FUNCTION();

    if( !m_pLastActiveView || !m_pLastActiveView->GetDataObject() )
    {
        return ;
    }

    LONG* pData = m_pLastActiveView->GetDataObject()->data;
    LONG  len = m_pLastActiveView->GetDataObject()->length;

    BeginHeavyTask();
    for(LONG i = 1; i < len; ++i)
    {
        pData[i-1] = pData[i] - pData[i-1];
    }
    pData[len-1] = 0;

    EnterHeavyTaskStep(IDS_PREPARE_VIEW);
    m_pLastActiveView->UpdateGraphRectangle(TRUE);
    m_pLastActiveView->Refresh();
    EndHeavyTask();
}

void CLinearGraphFrameWnd::OnHelpAbout()
{
    LGTRACE_FUNCTION();

    ::ShellExecuteW(m_hWnd, L"open", L"http://code.google.com/p/lineargraph/", 0, 0, SW_SHOW);
}

void CLinearGraphFrameWnd::OnHelpHelp()
{
    LGTRACE_FUNCTION();

    ::ShellExecuteW(m_hWnd, L"open", L".\\doc\\index.htm", 0, 0, SW_SHOW);
}

void CLinearGraphFrameWnd::OnFileExit()
{
    LGTRACE_FUNCTION();

    Close();
}

void CLinearGraphFrameWnd::CloseAll()
{
    SendCommand(ID_FILE_CLOSEALL);
}

void CLinearGraphFrameWnd::UpdateViewOptions()
{
    for(size_t i = 0; i < m_vpView.size(); ++i)
    {
        m_vpView[i]->SetOptions(m_dwViewOpts);
    }
}

void CLinearGraphFrameWnd::SendCommand(UINT uCmd)
{
    Send(WM_COMMAND, MAKEWPARAM(uCmd, 0));
}

int  CLinearGraphFrameWnd::ShowErrorBox(UINT reasonStringID, UINT titleStringID)
{
    WCHAR reasonString[256] = {0};
    GetString(reasonStringID, reasonString, 256);

    if( titleStringID != (UINT)-1 )
    {
        WCHAR titleString[128] = {0};
        GetString(titleStringID, titleString, 128);
        return MessageBox(reasonString, MB_ICONERROR, titleString);
    }
    return MessageBox(reasonString, MB_ICONERROR);
}

BOOL CLinearGraphFrameWnd::CreateWithCheck(HANDLE& hFile, PCWSTR fileName, BOOL bCheck)
{
    hFile = ::CreateFileW(fileName, GENERIC_READ|GENERIC_WRITE, 0,
        0, OPEN_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, 0);
    if( hFile == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    if( ::GetLastError() == ERROR_ALREADY_EXISTS )
    {
        WCHAR buff[128];
        CString text(buff, GetString(IDS_FILE_EXIST, buff, 128));
        text.AppendChar(L'\n');
        text.Append(fileName);

        GetString(IDS_APP_CAPTION, buff, 128);
        if( !bCheck || IDYES == MessageBox(text, MB_YESNO|MB_ICONQUESTION, buff) )
        {
            ::SetEndOfFile(hFile);
        }
        else
        {
            ::CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
        }
    }
    return TRUE;
}
