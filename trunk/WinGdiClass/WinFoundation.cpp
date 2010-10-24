#define WINGDICLASS_IMPL
#include "WinGdiClass.h"

BOOL IsWindowsXP()
{
    OSVERSIONINFOW ver = {0};
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    ::GetVersionExW(&ver);
    return ver.dwMajorVersion == 5;
}

BOOL CWindow::InitClass()
{
    INITCOMMONCONTROLSEX icex = {0};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = (HINSTANCE)::GetModuleHandle(0);
    wcex.lpszClassName = CWindow::ClassName();
    wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcex.lpfnWndProc = CWindow::WindowProcedure;
    wcex.style = CS_HREDRAW|CS_VREDRAW|CS_PARENTDC|CS_DBLCLKS;
    return RegisterClassEx(&wcex);
}

HINSTANCE CWindow::GetInstance() const
{
    return (HINSTANCE)::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE);
}

PCWSTR CWindow::ClassName()
{
    static const WCHAR className[] = L"LinearGraph-BaseWindow";
    return className;
}

LRESULT WINAPI CWindow::WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CWindow* pWnd = reinterpret_cast<CWindow*>(
        GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if( !pWnd )
    {
        if( msg == WM_NCCREATE )
        {
            // Normal window
            pWnd = reinterpret_cast<CWindow*>(
                ((LPCREATESTRUCT)lp)->lpCreateParams);
            pWnd->m_hWnd = hWnd;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWnd);
        }
        else if( msg == WM_INITDIALOG )
        {
            // Dialog window
            pWnd = reinterpret_cast<CWindow*>(lp);
            pWnd->m_hWnd = hWnd;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWnd);
        }
        else return (int)::DefWindowProc(hWnd, msg, wp, lp);
    }
    return pWnd->PreProcessMessage(msg, wp, lp);
}

CWindow* CWindow::Parent() const
{
    return StaticCast(::GetParent(m_hWnd));
}

CWindow* CWindow::StaticCast( HWND h )
{
    return reinterpret_cast<CWindow*>(GetWindowLongPtr(h, GWLP_USERDATA));
}

BOOL CWindow::Create(
    DWORD dwStyle, HWND pa, PCWSTR pCaption, PCWSTR pClassName, int x, int y, int cx, int cy)
{
    if( !pClassName )
    {
        pClassName = CWindow::ClassName();
    }
    m_hWnd = ::CreateWindowExW(
        0, pClassName, pCaption, dwStyle,
        x, y, cx, cy, pa, 0, 
        ::GetModuleHandleW(0), this
        );
    return m_hWnd != 0;
}

BOOL CWindow::SetTheme(PCWSTR szSubAppName, PCWSTR szSubIdList)
{
    typedef HRESULT (__stdcall *SWTPROC)(HWND, LPCWSTR, LPCWSTR);
    SWTPROC _SetWindowTheme = 0;
    HRESULT hr = S_FALSE;

    if( HMODULE hUxTheme = LoadLibraryA("UxTheme.dll") )
    {
        _SetWindowTheme = (SWTPROC)GetProcAddress(hUxTheme, "SetWindowTheme");
        if( _SetWindowTheme )
        {
            hr = _SetWindowTheme(m_hWnd, szSubAppName, szSubIdList);
        }
        FreeLibrary(hUxTheme);
    }
    return SUCCEEDED(hr);
}

BOOL CWindow::Close()
{
    return Send(WM_CLOSE);
}

BOOL CWindow::Destroy()
{
    return ::DestroyWindow(m_hWnd);
}

BOOL CWindow::IsVisible() const
{
    return ::IsWindowVisible(m_hWnd);
}

BOOL CWindow::SetForegroundWindow()
{
    return ::SetForegroundWindow(m_hWnd);
}

int CWindow::SetPosition(Position pos)
{
    HWND pa = GetParent(m_hWnd);
    if( !pa ){ pa = GetDesktopWindow(); }

    RECT rcClient, rcParent;
    GetClientRect(rcClient);
    ::GetClientRect(pa, &rcParent);

    INT x, y;
    INT left = 0, top = 0;
    INT middle = (rcParent.right - rcClient.right + 1) / 2;
    INT right = (rcParent.right - rcClient.right);
    INT center = (rcParent.bottom - rcClient.bottom + 1) / 2;
    INT bottom = (rcParent.bottom - rcClient.bottom);

    switch( pos )
    {
    case LeftTop:
        x = left, y = top;      break;
    case MiddleTop:
        x = middle, y = top;    break;
    case RightTop:
        x = right, y = top;     break;
    case LeftCenter:
        x = left, y = center;   break;
    case MiddleCenter:
        x = middle; y = center; break;
    case RightCenter:
        x = right; y = center;  break;
    case LeftBottom:
        x = left, y = bottom;   break;
    case MiddleBottom:
        x = middle, y = bottom; break;
    case RightBottom:
        x = right, y = bottom;  break;
    default:
        return 0;
    }
    return SetPosition(x, y);
}

int CWindow::Width() const
{
    RECT rcClient;
    GetClientRect(rcClient);
    return rcClient.right;
}

int CWindow::Height() const
{
    RECT rcClient;
    GetClientRect(rcClient);
    return rcClient.bottom;
}

int CWindow::WindowWidth() const
{
    RECT rcWnd;
    GetWindowRect(rcWnd);
    return rcWnd.right - rcWnd.left;
}

int CWindow::WindowHeight() const
{
    RECT rcWnd;
    GetWindowRect(rcWnd);
    return rcWnd.bottom - rcWnd.top;
}

int CWindow::SetPosition(int x, int y)
{
    return ::SetWindowPos(m_hWnd, 0, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
}

int CWindow::SetSize(int cx, int cy)
{
    return ::SetWindowPos(m_hWnd, 0, 0, 0, cx, cy, SWP_NOZORDER|SWP_NOMOVE);
}

int CWindow::SetPlacement(int x, int y, int cx, int cy)
{
    return ::SetWindowPos(m_hWnd, 0, x, y, cx, cy, SWP_NOZORDER);
}

int CWindow::SetPlacement(Position pos, int cx, int cy)
{
    SetSize(cx, cy);
    return SetPosition(pos);
}

int CWindow::MessageBox(PCWSTR szText, int mbType, PCWSTR szCaption)
{
    WCHAR szMyCap[32];
    if( !szCaption )
    {
        GetText(szMyCap, 32);
        szCaption = szMyCap;
    }
    return ::MessageBox(m_hWnd, szText, szCaption, mbType);
}

int CWindow::GetClientRect( RECT& rc ) const
{
    return ::GetClientRect(m_hWnd, &rc);
}

int CWindow::GetWindowRect( RECT& rc ) const
{
    return ::GetWindowRect(m_hWnd, &rc);
}

int CWindow::GetChildRect(CWindow* child, RECT& rcChild) const
{
    child->GetWindowRect(rcChild);

    POINT pt = {rcChild.left, rcChild.top};
    ScreenToClient(m_hWnd, &pt);
    rcChild.left = pt.x;
    rcChild.top = pt.y;
    rcChild.right = pt.x + child->Width();
    rcChild.bottom = pt.y + child->Height();
    return 1;
}

int CWindow::DefWindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
    return (int)::DefWindowProc(m_hWnd, msg, wp, lp);
}

int CWindow::PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch ( msg )
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            ::BeginPaint(m_hWnd, &ps);

            CBufferedDC bufferedDC(ps.hdc, Width(), Height());
            OnPaint(bufferedDC, ps);
            bufferedDC.Bitblt(ps.hdc, 0, 0);

            ::EndPaint(m_hWnd, &ps);
        }
        break;
    case WM_PRINTCLIENT:
        OnPrintClient((HDC)wp, lp);
        break;
    default:
        return DefWindowProc(msg, wp, lp);
    }
    return 0;
}

int CWindow::SetMenu(HMENU hMenu)
{
    return ::SetMenu(m_hWnd, hMenu);
}

int CWindow::SetText( PCWSTR szText )
{
    return ::SetWindowText(m_hWnd, szText);
}

int CWindow::SetWindowID( DWORD dwID )
{
    return ::SetWindowLong(m_hWnd, GWL_ID, dwID);
}

int CWindow::SetExtendedStyle( DWORD dwExStyle )
{
    return ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle);
}

int CWindow::SetCaptionIcon( HICON icon )
{
    return Send(WM_SETICON, ICON_SMALL, (LPARAM)icon);
}

int CWindow::SetTaskbarIcon( HICON icon )
{
    return Send(WM_SETICON, ICON_BIG, (LPARAM)icon);
}

int CWindow::Send(UINT msg, WPARAM wp, LPARAM lp) const
{
    return (int)::SendMessageW(m_hWnd, msg, wp, lp);
}

int CWindow::ShowWindow(int nCmdShow) const
{
    return ::ShowWindow(m_hWnd, nCmdShow);
}

int CWindow::InvalidateRect(RECT *lpRect, BOOL bErase) const
{
    return ::InvalidateRect(m_hWnd, lpRect, bErase);
}

int CWindow::TrackMouseEvent(DWORD tmef) const
{
    TRACKMOUSEEVENT tme = {0};
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = tmef;
    return ::TrackMouseEvent(&tme);
}

int CWindow::PtInClient(POINT pt) const
{
    RECT rcClient;
    GetClientRect(rcClient);
    return ::PtInRect(&rcClient, pt);
}

int CWindow::PtInClient( int x, int y ) const
{
    POINT pt = {x,y};
    return PtInClient(pt);
}

int CWindow::PtInWindow( POINT pt ) const
{
    RECT rcWindow;
    GetWindowRect(rcWindow);
    return ::PtInRect(&rcWindow, pt);
}

int CWindow::PtInWindow( int x, int y ) const
{
    POINT pt = {x,y};
    return PtInWindow(pt);
}

void CWindow::OnPaint(HDC dc, PAINTSTRUCT& ps)
{
}

void CWindow::OnPrintClient(HDC dc, LPARAM prFormat)
{
}

int CWindow::GetScrollInfo(int nBar, SCROLLINFO& si) const
{
    return ::GetScrollInfo(m_hWnd, nBar, &si);
}

int CWindow::SetScrollInfo(int nBar, SCROLLINFO& si, BOOL bRedraw)
{
    return ::SetScrollInfo(m_hWnd, nBar, &si, bRedraw);
}

BOOL CWindow::Enable(BOOL bEnable)
{
    return ::EnableWindow(m_hWnd, bEnable);
}

DWORD CWindow::GetWindowID() const
{
    return (DWORD)::GetWindowLongPtr(m_hWnd, GWL_ID);
}

int CWindow::GetText(PWSTR szBuf, int cchMax)
{
    return ::GetWindowTextW(m_hWnd, szBuf, cchMax);
}

HICON CWindow::GetTaskbarIcon() const
{
    return (HICON)Send(WM_GETICON, ICON_BIG);
}

HICON CWindow::GetCaptionIcon() const
{
    return (HICON)Send(WM_GETICON, ICON_SMALL);
}

DWORD CWindow::GetString(UINT stringID, PTSTR bufString, DWORD stringLength) const
{
    return (DWORD)::LoadStringW(GetInstance(), stringID, bufString, stringLength);
}

//
//
//
CSolidBrush::CSolidBrush() : m_hBrush(0)
{

}

CSolidBrush::~CSolidBrush()
{
    Delete();
}

CSolidBrush::CSolidBrush( COLORREF clr )
{
    m_hBrush = ::CreateSolidBrush(clr);
}

void CSolidBrush::Delete()
{
    if( m_hBrush )
    { 
        ::DeleteObject(m_hBrush);
        m_hBrush = 0;
    }
}

void CSolidBrush::SetColor( COLORREF clr )
{
    Delete();
    m_hBrush = ::CreateSolidBrush(clr);
}
//
//
//

void CWinDC::ReleaseBrush()
{
    ::DeleteObject(::SelectObject(m_hDC, ::GetStockObject(BLACK_BRUSH)));
}

void CWinDC::ReleasePen()
{
    ::DeleteObject(::SelectObject(m_hDC, ::GetStockObject(BLACK_PEN)));
}

//
//
//
CDirectDC::CDirectDC(CWindow* pWnd)
    : CWinDC(::GetDC(pWnd->m_hWnd)), m_hWnd(pWnd->m_hWnd)
{

}

CDirectDC::CDirectDC(HWND hWnd)
    : CWinDC(::GetDC(hWnd)), m_hWnd(hWnd)
{
    
}

CDirectDC::~CDirectDC()
{
    Release();
}

void CDirectDC::Release()
{
    if( m_hDC )
    {
        ::ReleaseDC(m_hWnd, m_hDC);
        m_hDC  = 0;
        m_hWnd = 0;
    }
}
//
//
//
CBufferedDC::CBufferedDC() : m_hMemBitmap(0), m_cx(0), m_cy(0)
{

}

CBufferedDC::CBufferedDC(HDC dc, int cx, int cy) : m_hMemBitmap(0)
{
    Create(dc, cx, cy);
}

CBufferedDC::~CBufferedDC()
{
    Destroy();
}

BOOL CBufferedDC::Create( HDC dc, int cx, int cy )
{
    if( m_hDC ){ return FALSE; }

    m_hDC = ::CreateCompatibleDC(dc);
    if( !m_hDC )
    {
        return FALSE;
    }

    m_hMemBitmap = ::CreateCompatibleBitmap(dc, cx, cy);
    if( !m_hMemBitmap )
    {
        ::DeleteObject(m_hDC);
        m_hDC = 0;
        return FALSE;
    }

    m_cx = cx, m_cy = cy;
    ::DeleteObject(::SelectObject(m_hDC, m_hMemBitmap));
    return TRUE;
}

void CBufferedDC::Destroy()
{
    if( m_hDC )
    {
        ::DeleteObject(m_hDC);
        m_hDC = 0;
    }
    if( m_hMemBitmap )
    {
        ::DeleteObject(m_hMemBitmap);
        m_hMemBitmap = 0;
    }
    m_cx = m_cy = 0;
}

BOOL CBufferedDC::ResizeBitmap(int cx, int cy)
{
    if( m_cx != cx || m_cy != cy )
    {
        if( HBITMAP hBmp = ::CreateCompatibleBitmap(m_hDC, cx, cy) )
        {
            m_cx = cx, m_cy = cy;
            m_hMemBitmap = hBmp;
            ::DeleteObject(::SelectObject(m_hDC, m_hMemBitmap));
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CBufferedDC::Bitblt(HDC dcTarget, int xDst, int yDst)
{
    return ::BitBlt(dcTarget, xDst, yDst, m_cx, m_cy, m_hDC, 0, 0, SRCCOPY);
}

BOOL CBufferedDC::Bitblt(int xSrc, int ySrc, HDC dcTarget)
{
    return ::BitBlt(dcTarget, 0, 0, m_cx, m_cy, m_hDC, xSrc, ySrc, SRCCOPY);
}

BOOL CBufferedDC::FillRect(COLORREF clr, RECT* pRect)
{
    RECT rc = {0, 0, m_cx, m_cy};
    if( !pRect )
    {
        pRect = &rc;
    }
    return ::FillRect(m_hDC, pRect, CSolidBrush(clr));
}

BOOL CBufferedDC::FrameRect(COLORREF clr, RECT* pRect)
{
    RECT rc = {0, 0, m_cx, m_cy};
    if( !pRect )
    {
        pRect = &rc;
    }
    return ::FrameRect(m_hDC, pRect, CSolidBrush(clr));
}

BOOL CBufferedDC::FrameRegion(COLORREF clr, HRGN rgn, int w, int h)
{
    return ::FrameRgn(m_hDC, rgn, CSolidBrush(clr), w, h);
}

BOOL CBufferedDC::Copy(HDC dcSrc, int x, int y)
{
    return ::BitBlt(m_hDC, 0, 0, m_cx, m_cy, dcSrc, x, y, SRCCOPY);
}

BOOL CBufferedDC::AlphaCopy(HDC dcSrc, int x, int y, int al)
{
    BLENDFUNCTION bf = {0};
    bf.AlphaFormat = AC_SRC_ALPHA;
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 0x00FF & al;

    return ::GdiAlphaBlend(m_hDC, 0, 0, m_cx, m_cy, dcSrc, x, y, m_cx, m_cy, bf);
}

CPen::CPen() : m_hPen(0)
{

}

CPen::CPen(COLORREF clr, int nPenStyle , int nWidth) : m_hPen(0)
{
    Create(clr, nPenStyle, nWidth);
}

CPen::~CPen()
{
    Destroy();
}

BOOL CPen::Create(COLORREF clr, int nPenStyle , int nWidth)
{
    if( m_hPen ){ return FALSE; }
    m_hPen = ::CreatePen(nPenStyle, nWidth, clr);
    return m_hPen != 0;
}

void CPen::Destroy()
{
    ::DeleteObject(m_hPen);
    m_hPen = 0;
}

CFont::CFont(): m_hFont(0)
{

}

CFont::CFont(LONG fh)
{
    m_hFont = CreateSysFont(fh);
}

CFont::CFont( const LOGFONTW& lf )
{
    m_hFont = CreateFontIndirectW(&lf);
}

CFont::~CFont()
{
    Delete();
}

void CFont::Delete()
{
    if( m_hFont )
    {
        ::DeleteObject(m_hFont);
    }
    m_hFont = 0;
}

BOOL CFont::GetSysLogFont(LOGFONTW& lf)
{
    if( IsWindowsXP() )
    {
        memset(&lf, 0, sizeof(LOGFONTW));
        lf.lfHeight = -12;
        lf.lfCharSet = GB2312_CHARSET;
        lf.lfQuality = CLEARTYPE_QUALITY;
        lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
        wcscpy(lf.lfFaceName, L"Segoe UI\0");
    }
    else
    {
        NONCLIENTMETRICSW ncm = {0};
        ncm.cbSize = sizeof(NONCLIENTMETRICSW);
        if( !::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0) )
        {
            return FALSE;
        }
        lf = ncm.lfMessageFont;
    }
    return TRUE;
}

HFONT CFont::CreateSysFont(LONG fh)
{
    LOGFONTW lf;
    GetSysLogFont(lf);

    if( fh ){ lf.lfHeight = fh; }
    return ::CreateFontIndirectW(&lf);
}

BOOL CFont::Create( const LOGFONTW& lf )
{
    if( m_hFont )
    {
        return FALSE;
    }
    m_hFont = ::CreateFontIndirectW(&lf);
    return m_hFont != 0;
}