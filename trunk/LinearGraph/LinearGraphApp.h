#pragma once
#include "LinearGraphTaskWnd.h"

class CLinearGraphApp : public CApplication
{
public:
     CLinearGraphApp();
    ~CLinearGraphApp();

    BOOL OnInitApplication();
    BOOL OnExitApplication();

	HICON GetLogo();
    DWORD GetString(UINT stringID, PWSTR stringBuff, DWORD buffLength);

    static CLinearGraphApp* GetApp()
    {
        return static_cast<CLinearGraphApp*>(CApplication::GetCurrentApp());
    }

    static HFONT  GetThemeFont();
    static HBRUSH GetThemeBrush();
    static HBRUSH GetThemeToolWindowBrush();

    void BeginHeavyTask();
    void EnterHeavyTaskStep(UINT stepStrinID);
    void EnterHeavyOpenTask(PCWSTR fileName);
    void EndHeavyTask();

private:
    BOOL InitAsyncTaskMonitor();
    DWORD   m_dwMonitorID;
    HANDLE  m_hMonitor;
    CLinearGraphTaskMonitorWnd m_taskWnd;

    static DWORD WINAPI TaskMonitorThread(PVOID);
    static CSolidBrush  m_themeBrush;
    static CSolidBrush  m_themeToolWndBrush;
    static CFont        m_themeFont;

protected:
	HICON		m_hLogoIcon;
    ULONG_PTR   m_upGdiplus;
};
