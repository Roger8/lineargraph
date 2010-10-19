#pragma once

struct WINGDI_CLASS CRectangle
{
    CRectangle()
    :xMin(0), xMax(0), yMin(0), yMax(0){}

    CRectangle(LONG x, LONG xm, LONG y, LONG ym)
    :xMin(x), xMax(xm), yMin(y), yMax(ym){}

    bool operator == (const CRectangle& r) const
    {
        return xMax == r.xMax && yMax == r.yMax
            && xMin == r.xMin && yMin == r.yMin;
    }

    bool operator != (const CRectangle& r) const
    {
        return !(*this == r);
    }

    void SetRect(LONG x, LONG xm, LONG y, LONG ym)
    {
        xMin = x;
        yMin = y;
        xMax = xm;
        yMax = ym;
    }

    LONG Width()  const { return xMax - xMin; }
    LONG Height() const { return yMax - yMin; }

    LONG xMin, xMax;
    LONG yMin, yMax;
};

class WINGDI_CLASS CWindow
{
public:
    enum Position
    {
        LeftTop = 0, MiddleTop, RightTop,
        LeftCenter, MiddleCenter, RightCenter,
        LeftBottom, MiddleBottom, RightBottom
    };

    CWindow() : m_hWnd(0){}

    virtual ~CWindow(){}

    BOOL Create(
        DWORD dwStyle, HWND pa = 0, PCWSTR pCaption = 0, PCWSTR pClassName = 0,
        int x = 0, int y = 0, int cx = 0, int cy = 0);
    BOOL Destroy();
    BOOL Close();
    BOOL Enable(BOOL bEnable = TRUE);
    BOOL SetTheme(PCWSTR szSubAppName, PCWSTR szSubIdList = 0);
    BOOL IsVisible() const;
    BOOL SetForegroundWindow();

    int GetClientRect(RECT& rc) const;
    int GetWindowRect(RECT& rc) const;
    int GetText(PWSTR szBuf, int cchMax);
    int GetChildRect(CWindow* child, RECT& rcChild) const;
    HICON GetTaskbarIcon() const;
    HICON GetCaptionIcon() const;
    int SetMenu(HMENU hMenu);
    int SetText(PCWSTR szText);
    int SetWindowID(DWORD dwID);
    int SetExtendedStyle(DWORD dwExStyle);
    int SetCaptionIcon(HICON icon);
    int SetTaskbarIcon(HICON icon);
    int SetPosition(int x, int y);
    int SetPosition(Position pos);
    int SetSize(int cx, int cy);
    int SetPlacement(int x, int y, int cx, int cy);
    int SetPlacement(Position pos, int cx, int cy);
    int MessageBox(PCWSTR szText, int mbType = 0x40, PCWSTR szCaption = 0);
    int Width()  const;
    int Height() const;
    int WindowWidth() const;
    int WindowHeight() const;
    int PtInClient(POINT pt) const;
    int PtInClient(int x, int y) const;
    int PtInWindow(POINT pt) const;
    int PtInWindow(int x, int y) const;
    int ShowWindow(int nShowCmd = SW_SHOW) const;
    int Send(UINT msg, WPARAM wp = 0, LPARAM lp = 0) const;
    int TrackMouseEvent(DWORD tmef) const;
    int InvalidateRect(RECT *lpRect = 0, BOOL bErase = FALSE) const;
    int GetScrollInfo(int nBar, SCROLLINFO& si) const;
    int SetScrollInfo(int nBar, SCROLLINFO& si, BOOL bRedraw = TRUE);
    DWORD GetWindowID() const;
    DWORD GetString(UINT stringID, PTSTR bufString, DWORD stringLength) const;
    void SetFocus() const   { ::SetFocus(m_hWnd); }
    void SetCapture() const { ::SetCapture(m_hWnd); }
    
    CWindow*        Parent() const;
    HINSTANCE       GetInstance() const;
    virtual int     DefWindowProc(UINT msg, WPARAM wp, LPARAM lp);
    static  PCWSTR  ClassName();
    static  BOOL    InitClass();

protected:
    virtual int  PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp);
    virtual void OnPaint(HDC dc, PAINTSTRUCT& ps);
    virtual void OnPrintClient(HDC dc, LPARAM prFormat);

    static CWindow*  StaticCast(HWND h);
    static LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

public:
    HWND m_hWnd;
};

inline DWORD MakeARGB(DWORD dwAlpha, COLORREF rgb)
{
    return ((rgb & 0x000000FF) << 16) | (rgb & 0x0000FF00)
             | ((rgb & 0x00FF0000) >> 16) | (dwAlpha & 0xFF)<<24;
}

class WINGDI_CLASS CSolidBrush
{
public:
    CSolidBrush();
    explicit CSolidBrush(COLORREF clr);
    ~CSolidBrush();

    void Delete();
    void SetColor(COLORREF clr);

    operator HBRUSH() const { return m_hBrush; }

private:
    HBRUSH  m_hBrush;
};

class WINGDI_CLASS CWinDC
{
public:
    CWinDC(HDC dc = 0) : m_hDC(dc){}

    HDC GetDC() const    { return m_hDC; }
    operator HDC() const { return m_hDC; }

    HGDIOBJ SelectObject(HGDIOBJ hObj)
    {
        return ::SelectObject(m_hDC, hObj);
    }

    // A GDI pen or brush object can not be deleted by using DeleteObjct when 
    // it's selected in a DC
    // This function replace the current selected pen or brush object with
    // corresponding system stock object, which need not to be deleted, and
    // then deletes them
    //
    void ReleaseBrush();
    void ReleasePen();

protected:
    HDC     m_hDC;
};

class WINGDI_CLASS CDirectDC : public CWinDC
{
public:
    explicit CDirectDC(CWindow* pWnd);
    explicit CDirectDC(HWND hWnd);
    ~CDirectDC();

    void Release();

private:
    HWND    m_hWnd;
};

class WINGDI_CLASS CBufferedDC : public CWinDC
{
public:
    CBufferedDC();
    CBufferedDC(HDC dc, int cx, int cy);
    ~CBufferedDC();

    BOOL Create(HDC dc, int cx, int cy);
    void Destroy();

    BOOL ResizeBitmap(int cx, int cy);
    BOOL Bitblt(HDC dcTarget, int xDst, int yDst);
    BOOL Bitblt(int xSrc, int ySrc, HDC dcTarget);
    BOOL FillRect(COLORREF clr, RECT* pRect = 0);
    BOOL FrameRect(COLORREF clr, RECT* pRect = 0);
    BOOL Copy(HDC dcSrc, int x, int y);
    BOOL AlphaCopy(HDC dcSrc, int x, int y, int al);

    int Width()  const      { return m_cx; }
    int Height() const      { return m_cy; }

private:
    HBITMAP     m_hMemBitmap;
    int         m_cx, m_cy;
};

class WINGDI_CLASS CPen
{
public:
    CPen();
    explicit CPen(COLORREF clr, int nPenStyle = PS_SOLID, int nWidth = 1);
    ~CPen();

    operator HPEN() const { return m_hPen; }

    BOOL Create(COLORREF clr = 0, int nPenStyle = PS_SOLID, int nWidth = 1);
    void Destroy();

protected:
    HPEN    m_hPen;
};

class WINGDI_CLASS CFont
{
public:
    explicit CFont(LONG fh = 0);
    CFont(const LOGFONTW& lf);
    ~CFont();
    void Delete();

    operator HFONT() const
    {
        return m_hFont;
    }

    operator HGDIOBJ() const
    {
        return (HGDIOBJ)m_hFont;
    }

    static BOOL  GetSysLogFont(LOGFONTW& lf);
    static HFONT CreateSysFont(LONG fh = 0);

protected:
    HFONT m_hFont;
};