#pragma once

class WINGDI_CLASS CDialog : public CWindow
{
public:
    explicit CDialog(UINT uTemplateID = 0);
    virtual ~CDialog();

    virtual BOOL Create(UINT uTemplateID, CWindow* pa);
    virtual INT_PTR DoModal(CWindow* pa);
    virtual void EndDialog(UINT uResult);

private:
    int  DefWindowProc(UINT msg, WPARAM wp, LPARAM lp);

protected:
    virtual BOOL BindChild(HWND& hChild, UINT uCtrlID);
    virtual BOOL BindCommonControl(CCommonControl* pCtrl, UINT uCtrlID);
    virtual int  PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp);
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(UINT uCode, UINT uID, HWND hCtrl);
    virtual BOOL OnNotify(UINT uID, NMHDR* pNmhdr);

protected:
    UINT    m_uTemplate;
};