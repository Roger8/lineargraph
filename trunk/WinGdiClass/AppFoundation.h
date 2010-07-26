#pragma once
#include <atlstr.h>

class CWindow;

class WINGDI_CLASS CApplication
{
public:
    CApplication();
    ~CApplication();

    CWindow*  GetMainWindow() const;
    HINSTANCE GetInstance() const;

    // Shows the main window and go into message loop
    // This function will not return until the message loop is ended
    //
    BOOL DoMessageLoop();

    BOOL LoadAcceleratorTable(UINT uAccelID);
    
    // Returns the directory in which the application is deployed
    // The returned directory string ends with a backslash
    //
    static CString GetAppDirectory();

    // Returns pointer to current application instance
    //
    static CApplication* GetCurrentApp();

    virtual BOOL OnInitApplication();
    virtual BOOL OnExitApplication();

private:
    friend int WINAPI DllMain(void*, DWORD, void*);
    static PVOID    m_pvBase;
    static DWORD    m_tlsAppPtr;

public:
    CWindow*    m_pMainWnd;
    HACCEL      m_hAccelTable;
};