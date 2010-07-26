#pragma once

class CLinearGraphTaskMonitorWnd : public CWindow
{
public:
    enum { ID_SAFEEXIT = 1001 };
    
    CLinearGraphTaskMonitorWnd();

    BOOL Create();
    void DelayShow();
    void SetStatusText(PCWSTR statusText);
    void Hide();
    
protected:
    int  PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp);
    void OnPaint(HDC dc, PAINTSTRUCT& ps);
    void OnCommand(UINT uCode, UINT uID, HWND hCtrl);
    void OnShowWindow(BOOL bShow);
    void OnSafeExit();

private:
    CProgressControl    m_progress;
    CButton             m_safeExitButton;
    CString             m_statusText;
    DWORD               m_busyTime;
};
