#pragma once

class CWindow;
class WINGDI_CLASS CCommonControl : public CWindow
{
public:
    CCommonControl();
    virtual ~CCommonControl();
    
    BOOL Create(CWindow* pa, PCWSTR szClass, DWORD dwStyle, DWORD dwID);
    virtual BOOL Bind(HWND hCommCtrl);

protected:
    virtual int  PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp);

protected:
    WNDPROC     m_fnControlProc;
};

class WINGDI_CLASS CStatic: public CCommonControl
{
public:
    BOOL Create(CWindow* pa, DWORD dwStyle, DWORD dwID);
};

class WINGDI_CLASS CEdit : public CCommonControl
{
public:
    BOOL Create(CWindow* pa, DWORD dwStyle, DWORD dwID);
    BOOL SetLimitText(size_t cchMax);
};

class WINGDI_CLASS CButton : public CCommonControl
{
public:
    BOOL Create(CWindow* pa, DWORD dwStyle, DWORD dwID);

    int  GetState() const;
    int  SetHilight(BOOL bHighlight = TRUE);
    BOOL IsChecked() const;
    BOOL SetCheck(int nCheck = BST_CHECKED);
};

class WINGDI_CLASS CProgressControl : public CCommonControl
{
public:
    BOOL Create(CWindow* pa, DWORD dwStyle, DWORD dwID);

    int SetRange(int nMin, int nMax);
    int SetStep(int nStep);
    int StepIt();
    int SetCurPos(int nPos);
    int GetCurPos() const;
    int GetRange(int& nMin, int& nMax) const;
    BOOL SetMarquee(BOOL bEnable = TRUE, DWORD dwAniPeriod = 0);
};

class WINGDI_CLASS CTrackbar : public CCommonControl
{
public:
    BOOL Create(CWindow* pa, DWORD dwStyle, DWORD dwID);

    int SetRange(int nMin, int nMax);
    int SetCurPos(int nPos);
    int GetCurPos() const;
};

class WINGDI_CLASS CToolbar : public CCommonControl
{
public:
    BOOL Create(CWindow* pa, DWORD dwStyle, DWORD dwID);

    int AddButton(DWORD dwID, PCWSTR szText, DWORD dwBitmap, DWORD dwStyle);
    int AddBitmap(DWORD dwBitmapID, int nBitmaps);
    int SetButtonSize(int cx, int cy);
};

class WINGDI_CLASS CComboBox : public CCommonControl
{
public:
    BOOL Create(CWindow* pa, DWORD dwStyle, DWORD dwID);
    
    int AddString(PCWSTR szText);
    int DeleteString(int i);
    int GetItemCount() const;
    int GetCurSel() const;
    int SetCurSel(int iSel);

};

class WINGDI_CLASS CTabPanel : public CCommonControl
{
public:
    BOOL Create(CWindow* pa, DWORD dwStyle, DWORD dwID);

    int InsertTabItem(PWSTR szCaption, int iItem, int iImage = -1);
    int GetCurSel() const;
    int GetItemCount() const;
    int SetCurSel(int iSel);
    int SetItemSize(int cx, int cy);

    DWORD GetExtendedTabStyle() const;
    DWORD SetExtendedTabStyle(DWORD dwExTabStyle);

    BOOL DeleteItem(int iItem);
    BOOL DeleteAllItems();
};

class WINGDI_CLASS CListView : public CCommonControl
{
public:
    BOOL Create(CWindow* pa, DWORD dwStyle, DWORD dwID);
    int  InsertColumn(int iCol, PCWSTR szText, int nWidth, int fmt);
    int  InsertItem(int iItem, PCWSTR szText);
    BOOL SetItemText(int iItem, int iSubItem, PCWSTR szText);
    BOOL SetItemState(int iItem, UINT uState, UINT uMask);
    UINT GetItemState(int iItem, UINT uMask);

    BOOL SetItemChecked(int iItem, BOOL bCheck = TRUE);
    BOOL IsItemChecked(int iItem) const;

    DWORD GetExtendedListViewStyle() const;
    DWORD SetExtendedListViewStyle(DWORD dwExtLvStyle);
};