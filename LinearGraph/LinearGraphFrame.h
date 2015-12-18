#pragma once
#include "LinearGraph.h"
#include "LinearGraphApp.h"
#include "LinearGraphDoc.h"
#include "LinearGraphView.h"
#include "LinearGraphToolPanel.h"
#include "LinearGraphStatusPanel.h"

class CLinearGraphDoc;

class CSourceSelectDlg : public CDialog
{
public:
    enum{ IDD = IDD_DLG_SELDATA };
    CSourceSelectDlg(): CDialog(IDD){}

protected:
    BOOL OnInitDialog();
    BOOL OnCommand(UINT uCode, UINT uID, HWND hCtrl);
    void OnBnClickedOk();
	void OnSelectAll();//全选
	void OnAntiSelectAll();//反选

protected:
    CListView       m_listView;

public:
    CDataObjectPtrVect      m_vSource;
    CMaskVect               m_vChoice;
};

class CFormatSaveDlg : public CDialog
{
public:
    enum{ IDD = IDD_DLG_FORMATSAVE };

    CFormatSaveDlg(BOOL bEnableCheck = TRUE);
    int AddFormatType(PCWSTR fmtDesc);
    void SetCaption(UINT capStringID);

protected:
    BOOL OnCommand(UINT uCode, UINT uID, HWND hCtrl);
    void OnBtnBrowse();
    virtual BOOL OnInitDialog();
    virtual void OnBtnOk();

protected:
    CComboBox   m_cbFmtlist;
    CEdit       m_editDir;
    CButton     m_btnReminder;
    BOOL        m_bEnableCheck;
    WCHAR       m_pCaption[64];

public:
    CString     m_dirName;
    int         m_iFormat;
    BOOL        m_fCheckOverwrite;
};

class CPrintGraphDlg : public CFormatSaveDlg
{
public:
    enum ImageFormat
    {
        ImageBMP, ImagePNG, ImageJPEG, ImageGIF, ImageTIFF
    };

    CPrintGraphDlg() : CFormatSaveDlg(FALSE)
    {

    }

protected:
    BOOL OnInitDialog();
    void OnBtnOk();

public:
    CString     m_extensionName;
};

class CSaveDataDlg : public CFormatSaveDlg
{
public:
    enum DataFormat
    {
        DataBIN = 0, DataTXT, DataTXTWithTimestamp
    };

    explicit CSaveDataDlg(BOOL fTextWithTimestamp = FALSE)
        : m_fTextWithTimestamp(fTextWithTimestamp)
    {
        m_fCheckOverwrite = TRUE;
    }

    BOOL OnInitDialog();
    void OnBtnOk();

protected:
    BOOL    m_fTextWithTimestamp;
};

class CDateTimeDialog : public CDialog
{
public:
    enum{ IDD = IDD_DLG_DATETIME };

    CDateTimeDialog();

protected:
    BOOL OnInitDialog();
    BOOL OnCommand(UINT uCode, UINT uID, HWND hCtrl);

private:
    void OnBtnOK();

protected:
    CEdit       m_editFreqMulti;
    CEdit       m_editFreqBase;
    CEdit       m_editTimeMs;

public:
    SYSTEMTIME  m_stDateTime;
    DWORD       m_dwFreqBase;
    DWORD       m_dwFreqMulti;
};

class CNameDialog : public CDialog
{
public:
    enum{ IDD = IDD_DLG_GRAPHNAME };

    CNameDialog();

protected:
    BOOL OnInitDialog();
    BOOL OnCommand(UINT uCode, UINT uID, HWND hCtrl);

public:
    CString m_strName;
    LONG    m_lFontHeight;

private:
    CEdit       m_editName;
    CComboBox   m_cbFonts;
};

class CLinearTransformDialog : public CDialog
{
public:
    enum{ IDD = IDD_DLG_LINEAR };

    CLinearTransformDialog(): CDialog(IDD){}

protected:
    BOOL OnInitDialog();
    BOOL OnCommand(UINT uCode, UINT uID, HWND hCtrl);

protected:
    CEdit   m_editK, m_editC;

public:
    DOUBLE m_K, m_C;
};

class CLinearGraphFrameWnd : public CWindow
{
public:
    enum LayoutStyle
    {
        TabbedDoc = 0, SingleColumn, DoubleColumn, TrebleColumn 
    };

     CLinearGraphFrameWnd();
    ~CLinearGraphFrameWnd();

    BOOL Create();
    BOOL OpenDocument(PWSTR szDocName);
    void CloseAll();
    BOOL UpdateStatusPanel();
    BOOL CreateWithCheck(HANDLE& hFile, PCWSTR fileName, BOOL bCheck = TRUE);
    size_t GetDataSourceCount() const;
    size_t GetGraphViewCount() const;
    void   SendCommand(UINT uCmd);
    CLinearGraphViewPtr GetActiveView();
    CLinearGraphViewPtr FindAttachedView(CDataObjectPtr& dataObj);

protected:
    int  PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp);
    void OnPaint(HDC dc, PAINTSTRUCT& ps);
    void OnCreate(LPCREATESTRUCT lpcs);
    void OnSizeChanged(DWORD dwFlags, int cx, int cy);
    void OnSizing(DWORD dwFlag, PRECT pRcWnd);
    void OnDropFiles(HDROP hDrop);
    void OnCommand(UINT uCode, UINT uID, HWND hCtrl);
    BOOL OnContextMenu(int x, int y);
    void OnInitMenuPopup(HMENU hMenu, UINT nPos, BOOL bIsWindowMenu);

private:
    enum
    {
        FileMenu = 0, EditMenu, LayoutMenu, ViewMenu,
        AnalyzeMenu, ToolMenu, HelpMenu
    };

    void BeginHeavyTask()
    {
        CLinearGraphApp::GetApp()->BeginHeavyTask();
    }

    void EnterHeavyTaskStep(UINT stepStrinID)
    {
        CLinearGraphApp::GetApp()->EnterHeavyTaskStep(stepStrinID);
    }

    void EnterHeavyFileTask(PCWSTR fileName)
    {
        CLinearGraphApp::GetApp()->EnterHeavyFileTask(fileName);
    }

    void EndHeavyTask()
    {
        CLinearGraphApp::GetApp()->EndHeavyTask();
    }

    BOOL SaveDataToFile(const CDataObjectPtr pdo, CRectangle* pClip,
        BOOL bCheckOverwrite, DWORD dwFormat, CString& dirName);

    BOOL GetCustomizedColor(COLORREF& clr) const;
    int  ShowErrorBox(UINT reasonStringID, UINT titleStringID = -1);
    void UpdateWindowLayout();
    void UpdateViewOptions();
    void OnActivateView(CLinearGraphView* pView);
    void OnActiveViewChanged();
    BOOL OnOpenDocument(PCWSTR szFileName);
    void OnFileOpen();
    void OnFileClose();
    void OnFileCloseAll();
    void OnFileExportData();
    void OnFileSave();
    void OnFileSaveImage();
    void OnFileSaveAllImages();
    void OnFileExit();
    void OnEditRename();
    void OnEditDateTime();
    void OnEditReload();
    void OnLayoutTab();
    void OnLayoutSingleCol();
    void OnLayoutDiCol();
    void OnLayoutTreCol();
    void OnViewForegndClr();
    void OnViewBkgndClr();
    void OnViewRuler();
    void OnViewHorzLine();
    void OnViewCoord();
    void OnViewLegend();
    void OnViewPosLabel();
    void OnViewHAxisIndex();
    void OnViewHAxisStdTime();
    void OnViewHAxisChineseTime();
    void OnViewRestore();
    void OnViewZoomIn();
    void OnViewZoomOut();
    void OnViewClearCmp();
    void OnAnalyzeCompare();
    void OnAnalyzeSyncSelect();
    void OnAnalyzeLinear();
    void OnAnalyzePower();
    void OnAnalyzeDiff();
    void OnToolTranslucentSel();
    void OnToolSelectTransform();
    void OnToolOpenLogDir();
    void OnHelpAbout();
    void OnHelpHelp();

private:
    CLinearGraphToolPanel       m_toolPanel;
    CLinearGraphStatusPanel     m_statusPanel;

    DWORD                       m_layoutStyle;
    CLinearGraphViewPtr         m_pLastActiveView;
    CLinearGraphViewPtrVect     m_vpView;
    CDataObjectPtrVect          m_vpData;
    
    DWORD       m_dwViewOpts;
};