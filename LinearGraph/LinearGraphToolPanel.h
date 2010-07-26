#pragma once
#include "LinearGraph.h"
#include "LinearGraphDoc.h"
#include "LinearGraphView.h"

class CLinearGraphFrameWnd;

class CLinearGraphToolPanel : public CWindow
{
public:
    enum { IDC_TABPANEL = 1001 };

     CLinearGraphToolPanel();
    ~CLinearGraphToolPanel();

    BOOL Create(CLinearGraphFrameWnd* pa);
    int  AddTab(PCWSTR tabName);
    int  GetTabCount() const;
    BOOL RemoveViewTab(int iTab);
    int  SetActiveViewTab(int iTab);
    void RemoveAllViewTabs();
    int  GetActiveViewTab() const;

protected:
    int  PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp);
    void OnPaint(HDC dc, PAINTSTRUCT& ps);
    void OnNotify(UINT uID, NMHDR* pNmhdr);
    void OnSizeChanged(DWORD dwFlags, int cx, int cy);

protected:
    CLinearGraphFrameWnd*   m_pFrame;
    CTabPanel       m_tabPanel;
};
