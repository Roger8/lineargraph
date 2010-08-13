#include "LinearGraphView.h"
#include "LinearGraphFrame.h"
#include "LinearGraphCodec.h"

CLinearGraphPainter::CLinearGraphPainter()
{
    m_pAlloc        = 0;
    m_pPtCount      = 0;
    m_pFirstValue   = 0;
    m_pLastValue    = 0;
    m_pMaxValue     = 0;
    m_pMinValue     = 0;

    m_sizeCanvas.cx = m_sizeCanvas.cy = 0;
}

CLinearGraphPainter::~CLinearGraphPainter()
{
    DeleteCanvas();
}

BOOL CLinearGraphPainter::CreateCanvas(int cx, int cy)
{
    if( m_sizeCanvas.cx || cx <= 0 || cy <= 0 )
    {
        return FALSE;
    }
    
    if( !(m_pAlloc = new LONG[cx * 5 + 10]) )
    {
        return FALSE;
    }

    m_pPtCount    = m_pAlloc;
    m_pFirstValue = m_pPtCount + cx + 2;
    m_pLastValue  = m_pFirstValue + cx + 2;
    m_pMinValue   = m_pLastValue + cx + 2;
    m_pMaxValue   = m_pMinValue + cx + 2;

    m_sizeCanvas.cx = cx;
    m_sizeCanvas.cy = cy;
    memset(m_pPtCount, 0, cx * sizeof(LONG));
    return TRUE;
}

BOOL CLinearGraphPainter::DeleteCanvas()
{
    delete[] m_pAlloc;
    m_pAlloc = 0;

    m_pPtCount    = 0;
    m_pMaxValue   = 0;
    m_pMinValue   = 0;
    m_pFirstValue = 0;
    m_pLastValue  = 0;

    m_rcClip.SetRect(0, 0, 0, 0);
    m_sizeCanvas.cx = m_sizeCanvas.cy = 0;
    return TRUE;
}

BOOL CLinearGraphPainter::DrawGraph(LONG* apData, LONG nData, CRectangle* rcClip)
{
    if( !m_pAlloc ){ return FALSE; }
    memset(m_pPtCount, 0, m_sizeCanvas.cx * sizeof(LONG));

    if( rcClip )
    {
        m_rcClip = *rcClip;
        if( m_rcClip.xMin < 0 ){ m_rcClip.xMin = 0; }
        if( m_rcClip.xMax > nData ){ m_rcClip.xMax = nData; }
    }
    else
    {
        // If clip window is not specified, the whole graph will be drawn
        //
        m_rcClip.SetRect(0, nData, 0, 0);
        for(int i = 0; i < nData; i++)
        {
            if( apData[i] > m_rcClip.yMax ){ m_rcClip.yMax = apData[i]; }
            if( apData[i] < m_rcClip.yMin ){ m_rcClip.yMin = apData[i]; }
        }
    }

    // Graph dimension in mathematical unit
    int lcx = m_rcClip.Width();
    int lcy = m_rcClip.Height();
    if( !lcx || !lcy )
    {
        return FALSE;
    }

    // Map the graph to the canvas
    int pti, prev_pti = -1, ptv;
    for(int x = m_rcClip.xMin; x < m_rcClip.xMax; x++, prev_pti = pti)
    {
        // NOTE CAREFULLY HERE !!!
        // To obtain the correct point index, that is pti, we need to use a 64bit
        // integer to store the intermediate result. Pure 32bit computing may result
        // in overflow
        //
        pti = (int)((__int64)(x - m_rcClip.xMin) * m_sizeCanvas.cx / lcx);
        ptv = (int)((__int64)(apData[x] - m_rcClip.yMin) * m_sizeCanvas.cy / lcy);

        m_pLastValue[pti] = ptv;
        if( pti != prev_pti )
        {
            m_pPtCount[pti] = 1;
            m_pFirstValue[pti] = m_pLastValue[pti] = 
                m_pMinValue[pti] = m_pMaxValue[pti] = ptv;
        }
        else
        {
            m_pPtCount[pti] ++;
            if( ptv > m_pMaxValue[pti] ){ m_pMaxValue[pti] = ptv; }
            if( ptv < m_pMinValue[pti] ){ m_pMinValue[pti] = ptv; }
            m_pLastValue[pti] = ptv;
        }
    }
    return TRUE;
}

BOOL CLinearGraphPainter::Print(HDC dc, int xDst, int yDst, COLORREF clr)
{
    if( !m_pAlloc ){ return FALSE; }

    POINT* ptBuffer = new POINT[ 2*m_sizeCanvas.cx + 2 ];
    if( !ptBuffer )
    {
        return FALSE;
    }

    HPEN hPen = ::CreatePen(PS_SOLID, 1, clr);
    HGDIOBJ hOldPen = ::SelectObject(dc, hPen);

    int ipt = 0;
    int y = m_sizeCanvas.cy + yDst;
    for(int i = 0; i < m_sizeCanvas.cx; ++i)
    {
        if( m_pPtCount[i] )
        {
            ptBuffer[ipt].x = ptBuffer[ipt + 1].x = xDst + i;
            ptBuffer[ipt].y = y - m_pFirstValue[i];
            ptBuffer[++ipt].y = y - m_pLastValue[i];
            ++ipt;
        }
    }
    ::Polyline(dc, ptBuffer, ipt);

    for(int i = 0; i < m_sizeCanvas.cx; ++i)
    {
        if( m_pPtCount[i] > 1 )
        {
            ptBuffer[0].x = ptBuffer[1].x = xDst + i;
            ptBuffer[0].y = y - m_pMinValue[i];
            ptBuffer[1].y = y - m_pMaxValue[i];
            ::Polyline(dc, ptBuffer, 2);
        }
    }

    delete[] ptBuffer;
    ::DeleteObject(::SelectObject(dc, hOldPen));
    return TRUE;
}


CLinearGraphView::CLinearGraphView()
{
    m_clrBackground = RGB(255, 255, 255);
    m_clrForeground = RGB(0, 255, 0);
    m_bTrackSelection = FALSE;
    m_lMaxStretch = 32;
    m_lFontHieght = CLinearGraphView::DefaultFontHeight;
    m_nCursorX = m_nCursorY = -1;
}

CLinearGraphView::~CLinearGraphView()
{

}

CLinearGraphView* CLinearGraphView::DynamicCreate(CWindow* pa)
{
    if( CLinearGraphView* pView = new CLinearGraphView )
    {
        if( pView->Create(WS_VISIBLE|WS_CHILD|WS_VSCROLL|WS_HSCROLL, pa->m_hWnd) )
        {
            if( pView->InitGraphCache() )
            {
                return pView;
            }
            else{ pView->Destroy(); }
        }
        delete pView;
    }
    return 0;
}

BOOL CLinearGraphView::InitGraphCache()
{
    RECT rcDesktop;
    ::GetClientRect(::GetDesktopWindow(), &rcDesktop);

    HDC dc = ::GetDC(::GetDesktopWindow());
    if( m_dcGraphCache.Create(dc, rcDesktop.right, rcDesktop.bottom) )
    {
        ::DeleteObject(::SelectObject(m_dcGraphCache, CLinearGraphApp::GetThemeFont()));
        ::SetBkMode(m_dcGraphCache, TRANSPARENT);
    }
    ::ReleaseDC(::GetDesktopWindow(), dc);
    return m_dcGraphCache.GetDC() != 0;
}

CDataObjectPtr& CLinearGraphView::GetDataObject()
{
    return m_pData;
}

COLORREF CLinearGraphView::GetForegroundColor() const
{
    return m_clrForeground;
}

COLORREF CLinearGraphView::GetBackgroundColor() const
{
    return m_clrBackground;
}

BOOL CLinearGraphView::GetDateTime(SYSTEMTIME& stDate, DWORD& fbase, DWORD& fmulti) const
{
    if( !m_pData || !m_pData->hasTimestamp() )
    {
        return FALSE;
    }

    fbase  = m_pData->freqBase;
    fmulti = m_pData->freqMulti;
    ::FileTimeToSystemTime((FILETIME*)&(m_pData->timeStamp), &stDate);
    return TRUE;
}

BOOL CLinearGraphView::SetDateTime(SYSTEMTIME& stDateTime, DWORD fbase, DWORD fmulti)
{
    FILETIME ft;
    if( ::SystemTimeToFileTime(&stDateTime, &ft) )
    {
        return SetDateTime(ft, fbase, fmulti);
    }
    return FALSE;
}

BOOL CLinearGraphView::SetDateTime(FILETIME& ftDateTime, DWORD fbase, DWORD fmulti)
{
    m_pData->timeStamp = *((ULONGLONG*)&ftDateTime);
    m_pData->freqBase  = fbase;
    m_pData->freqMulti = fmulti;
    return TRUE;
}

BOOL CLinearGraphView::SetForegroundColor(COLORREF clr, BOOL bRefresh)
{
    m_clrForeground = clr;
    if( bRefresh && m_pData ){ Refresh(); }
    return TRUE;
}

BOOL CLinearGraphView::SetBackgroundColor(COLORREF clr, BOOL bRefresh)
{
    m_clrBackground = clr;
    if( bRefresh && m_pData ){ Refresh(); }
    return TRUE;
}

BOOL CLinearGraphView::SetMaxStretch(LONG lMaxStretch)
{
    if( lMaxStretch > 32 || !lMaxStretch )
    {
        return FALSE;
    }
    m_lMaxStretch = lMaxStretch;
    return TRUE;
}

int  CLinearGraphView::PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp)
{
    switch( msg )
    {
        // System messages
        //
    case WM_SIZE:
        OnSizeChanged((DWORD)wp, LOWORD(lp), HIWORD(lp));
        break;
    case WM_MOUSEMOVE:
        OnMouseMove((DWORD)wp, LOWORD(lp), HIWORD(lp));
        break;
    case WM_MOUSEWHEEL:
        OnMouseWheel(GET_KEYSTATE_WPARAM(wp), GET_WHEEL_DELTA_WPARAM(wp));
        break;
    case WM_MOUSELEAVE:
        OnMouseLeave();
        break;
    case WM_VSCROLL:
    case WM_HSCROLL:
        OnScroll(msg, LOWORD(wp));
        break;
    case WM_LBUTTONDOWN:
        OnLButtonDown((DWORD)wp, LOWORD(lp), HIWORD(lp));
        break;
    case WM_LBUTTONUP:
        OnLButtonUp((DWORD)wp, LOWORD(lp), HIWORD(lp));
        break;
    case WM_RBUTTONDOWN:
        OnRButtonDown((DWORD)wp, LOWORD(lp), HIWORD(lp));
        break;
    case WM_NCDESTROY:
        {
            DefWindowProc(WM_NCDESTROY, 0, 0);
            delete this;
        }
        break;
    default:
        return CWindow::PreProcessMessage(msg, wp, lp);
    }
    return 0;
}

void CLinearGraphView::OnPaint(HDC dc, PAINTSTRUCT& ps)
{
    ::SetBkMode(dc, TRANSPARENT);
    ::DeleteObject(::SelectObject(dc, CLinearGraphApp::GetThemeFont()));

    m_dcGraphCache.Bitblt(dc, 0, 0);
    if( m_pData )
    {
        if( m_dwOptionSet & OptHorizontalLine ){ DrawHorizontalLine(dc); }
        if( m_dwOptionSet & OptPositionLabel ){ DrawPositionLable(dc); }
    }

    RECT rcClient;
    GetClientRect(rcClient);

    if( m_dwOptionSet & OptRuler )
    {
        CRectangle rcCanvas, rcClip;
        GetCanvasRectangle(rcCanvas);
        GetClipRectangle(rcClip);
        DrawRuler(dc, rcCanvas, rcClip);
        if( !m_bTrackSelection )
        {
            DrawIndicationLine(dc);
        }
    }

    if( m_bTrackSelection )
    {
        RECT rcSelect;
        CalcSelectionRect(rcSelect);
        DrawSelectionRectangle(dc, rcSelect);
    }
}

void CLinearGraphView::OnSizeChanged(DWORD dwFlags, int cx, int cy)
{
    if( cx && cy )
    {
        if( !m_pData )
        {
            CRectangle rcCanvas(0, cx, 0, cy);
            SetGraphRectangle(rcCanvas);
            SetClipRectangle(rcCanvas);
        }
        UpdateGraphicsCache();
    }
}

void CLinearGraphView::OnMouseMove(DWORD dwFlags, int x, int y)
{
    if( m_nCursorX < 0 )
    {
        TrackMouseEvent(TME_LEAVE);
    }

    m_nCursorX = x > 0x7FFF ? 0 : x;
    m_nCursorY = y > 0x7FFF ? 0 : y;
    
    if( m_bTrackSelection || (m_dwOptionSet & OptRuler)
        || (m_pData && (m_dwOptionSet & OptPositionLabel)) )
    {
        InvalidateRect();
    }
}

void CLinearGraphView::OnMouseWheel(DWORD dwKeyFlag, int dz)
{
    CRectangle rcClip;
    GetClipRectangle(rcClip);
    
    int  nStep = dz / WHEEL_DELTA;
    BOOL bHorz = dwKeyFlag & MK_SHIFT;

    // IMPORTANT NOTE
    //  rcClip.Width() or rcClip.Height() should never be less than StepStretchRate
    //  Otherwise nDelta will get zero, which means that the view window will
    // not respond the mouse wheel message any longer. Then the screen will be "frozen"
    // forever.
    //  See the implementation of StretchClipWindow method for more information.
    //
    int  nDelta = nStep * (bHorz ? rcClip.Width() : rcClip.Height()) / StepStretchRate;

    if( dwKeyFlag & MK_CONTROL )
    {
        ResizeClipWindow(-nDelta, bHorz);
    }
    else
    {
        MoveClipWindow(nDelta, bHorz);
    }

    CRectangle rcNewClip;
    GetClipRectangle(rcNewClip);
    if( rcNewClip != rcClip )
    {
        Refresh();
    }
}

void CLinearGraphView::OnMouseLeave()
{
    m_nCursorX = m_nCursorY = -1;
    InvalidateRect();
}

void CLinearGraphView::OnScroll(UINT msg, DWORD dwAction)
{
    int nbar = (msg == WM_HSCROLL ? SB_HORZ : SB_VERT);

    SCROLLINFO si = {0};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    GetScrollInfo(nbar, si);

    switch (dwAction)
    {
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        si.nPos = si.nTrackPos; break;
    case SB_PAGEUP:
        si.nPos -= si.nPage;    break;
    case SB_PAGEDOWN:
        si.nPos += si.nPage;    break;
    case SB_LINEUP:
        si.nPos -= si.nPage/32;  break;
    case SB_LINEDOWN:
        si.nPos += si.nPage/32;  break;
    case SB_TOP:
        si.nPos = si.nMin;      break;
    case SB_BOTTOM:
        si.nPos = si.nMax;      break;
    }

    si.fMask = SIF_POS;
    SetScrollInfo(nbar, si);
    Refresh();
}

void CLinearGraphView::OnLButtonDown(DWORD dwFlags, int x, int y)
{
    m_nSelectX = x;
    m_nSelectY = y;

    SetFocus();
    SendParentalNotification(LinearGraphViewActivate);
    SetCapture();
    m_bTrackSelection = TRUE;
    InvalidateRect();
}

void CLinearGraphView::OnLButtonUp(DWORD dwFlags, int x, int y)
{
    if( m_bTrackSelection )
    {
        ReleaseCapture();
        m_bTrackSelection = FALSE;

        if( PtInClient(x, y) && (m_dwOptionSet & OptSelectionTransform) )
        {
            RECT rcSelect;
            CalcSelectionRect(rcSelect);
            OnSelectTransform(rcSelect);
        }
        InvalidateRect();
    }
}

void CLinearGraphView::OnRButtonDown(DWORD dwFlags, int x, int y)
{
    SendParentalNotification(LinearGraphViewActivate);
}

void CLinearGraphView::OnSelectTransform(RECT& rcSelect)
{
    if( rcSelect.right - rcSelect.left < 16 )
    {
        return ;
    }

    CRectangle rcCanvas;
    GetCanvasRectangle(rcCanvas);

    CRectangle rcClip;
    GetClipRectangle(rcClip);

    POINT pt[2] = {{rcSelect.left, rcSelect.top}, {rcSelect.right, rcSelect.bottom}};
    ClientPtToGraphPt(pt[0]);
    ClientPtToGraphPt(pt[1]);

    // Do not change vertical clip position
    //
    rcClip.xMin = pt[0].x;
    rcClip.xMax = pt[1].x;
    if( rcClip.Width() > rcCanvas.Width() / m_lMaxStretch )
    {
        SetClipRectangle(rcClip);
        UpdateGraphicsCache();
    }
}

BOOL CLinearGraphView::BindData(CDataObjectPtr& pData)
{
    if( !(m_pData = pData) )
    {
        return FALSE;
    }
    SetColorByName(m_pData->name);
    return UpdateGraphRectangle(TRUE) && Refresh();
}

BOOL CLinearGraphView::AddComparison(CLinearGraphView& cmpV, BOOL bRefresh)
{
    if( !m_pData->isCompatibleWith(cmpV.GetDataObject()) )
    {
        return FALSE;
    }
    m_vpComparison.push_back(cmpV.GetDataObject());
    m_vColor.push_back(cmpV.GetForegroundColor());

    CRectangle rcCmp;
    cmpV.GetDataRectangle(rcCmp);

    if( rcCmp.xMin > m_rcGraph.xMin ){ rcCmp.xMin = m_rcGraph.xMin; }
    if( rcCmp.xMax < m_rcGraph.xMax ){ rcCmp.xMax = m_rcGraph.xMax; }
    if( rcCmp.yMin > m_rcGraph.yMin ){ rcCmp.yMin = m_rcGraph.yMin; }
    if( rcCmp.yMax < m_rcGraph.yMax ){ rcCmp.yMax = m_rcGraph.yMax; }
    
    if( rcCmp != m_rcGraph )
    {
        SetGraphRectangle(rcCmp);
    }
    return bRefresh ? Refresh() : TRUE;
}

BOOL CLinearGraphView::ClearComparison()
{
    m_vpComparison.clear();
    m_vColor.clear();
    return Refresh();
}

BOOL CLinearGraphView::PrintGraph(PCWSTR dirName, PCWSTR extensionName)
{
    CRectangle rcClip;
    GetClipRectangle(rcClip);

    CRectangle rcCanvas;
    GetCanvasRectangle(rcCanvas);

    Gdiplus::Graphics g(m_hWnd);
    Gdiplus::Bitmap   bmp(rcCanvas.Width(), rcCanvas.Height(), &g);
    Gdiplus::Graphics gDrawing(&bmp);
    CWinDC dc = gDrawing.GetHDC();

    // Position label option is not available for printing
    //
    DWORD savedOpt = m_dwOptionSet;
    m_dwOptionSet &= ~(DWORD)OptPositionLabel;

    PAINTSTRUCT ps = {0};
    OnPaint(dc, ps);

    m_dwOptionSet = savedOpt;
    gDrawing.ReleaseHDC(dc);
    
    CString fileName = dirName;
    fileName.Append(m_pData->name);
    fileName.Append(extensionName);
    return CLinearGraphCodec::SaveImage(&bmp, fileName);
}

void CLinearGraphView::SendParentalNotification(Messages msg)
{
    ::SendMessage(::GetParent(m_hWnd), (UINT)msg, 0, (LPARAM)this);
}

void CLinearGraphView::ResizeClipWindow(int ds, BOOL bHorz)
{
    if( !ds || !m_pData ){ return; }

    CRectangle rcClip;
    GetClipRectangle(rcClip);

    CRectangle rcCanvas;
    GetCanvasRectangle(rcCanvas);

    if( bHorz )
    {
        if( (ds > 0 && (rcClip.xMin <= m_rcGraph.xMin || rcClip.xMax >= m_rcGraph.xMax))
            || (ds < 0 && (rcClip.Width() + 2 * ds < rcCanvas.Width() / m_lMaxStretch))
            || (ds < 0 && (rcClip.Width() + 2 * ds < StepStretchRate)) )
        {
            return;
        }
        rcClip.xMin -= ds;
        rcClip.xMax += ds;
    }
    else
    {
        if( (ds > 0 && (rcClip.yMin <= m_rcGraph.yMin || rcClip.yMax >= m_rcGraph.yMax))
            || (ds < 0 && (rcClip.Height() + 2 * ds < rcCanvas.Height() / m_lMaxStretch))
            || (ds < 0 && (rcClip.Height() + 2 * ds < StepStretchRate)) )
        {
            return;
        }
        rcClip.yMin -= ds;
        rcClip.yMax += ds;
    }
    SetClipRectangle(rcClip);
}

void CLinearGraphView::MoveClipWindow(int ds, BOOL bHorz)
{
    if( !ds || !m_pData ){ return; }

    CRectangle rcClip;
    GetClipRectangle(rcClip);

    if( bHorz )
    {
        if( ds < 0 && rcClip.xMax - ds > m_rcGraph.xMax )
        {
            ds = rcClip.xMax - m_rcGraph.xMax;
        }
        else if( ds > 0 && rcClip.xMin - ds < m_rcGraph.xMin )
        {
            ds = rcClip.xMin - m_rcGraph.xMin;
        }
        if( !ds ){ return; }

        rcClip.xMin -= ds;
        rcClip.xMax -= ds;
    }
    else
    {
        if( ds < 0 && rcClip.yMin + ds < m_rcGraph.yMin )
        {
            ds = m_rcGraph.yMin - rcClip.yMin;
        }
        else if( ds > 0 && rcClip.yMax + ds > m_rcGraph.yMax )
        {
            ds = m_rcGraph.yMax - rcClip.yMax;
        }
        if( !ds ){ return; }

        rcClip.yMin += ds;
        rcClip.yMax += ds;
    }
    SetClipRectangle(rcClip);
}

void CLinearGraphView::DrawRuler(HDC dc, CRectangle& rcCanvas, CRectangle& rcClip)
{
    CPen penRuler(RGB(0x29,0x39,0x55));
    ::SetBkColor(dc, m_clrBackground);

    HGDIOBJ hOldPen;
    hOldPen = ::SelectObject(dc, (HPEN)penRuler);

    POINT ptStart = {rcCanvas.xMin, rcCanvas.yMax};
    ClientPtToGraphPt(ptStart);

    LONG vUnit;
    LONG vSections = max(rcCanvas.Height()/10, 3);
    LONG vScope = rcClip.Height();
    for(vUnit  = 10; vUnit * vSections < vScope; vUnit += 10);
    DrawVerticalAxis(dc, ptStart, vSections, vUnit);

    if( m_pData->hasTimestamp() && (m_dwOptionSet & OptTimeLabelOnHAxis) )
    {
        LONG splitMethod;
        LONG startValue;
        SplitTimeAxis(rcClip, startValue, splitMethod);
        DrawTimeAxis(dc, ptStart, startValue, splitMethod);
    }
    else
    {
        LONG hUnit;
        LONG hSections = max(rcCanvas.Width()/10, 3);
        LONG hScope = rcClip.Width();
        for(hUnit = 10; hUnit * hSections < hScope; hUnit += 10);
        DrawHorizontalAxis(dc, ptStart, hSections, hUnit);
    }

    ::SelectObject(dc, hOldPen);
}

void CLinearGraphView::DrawVerticalAxis(HDC dc, POINT ptStart, LONG nSections, LONG nUnit)
{
    POINT apt[2] = {ptStart, {ptStart.x, ptStart.y+nUnit*nSections}};
    GraphPtToClientPt(apt[0]);
    GraphPtToClientPt(apt[1]);
    ::Polyline(dc, apt, 2);

    RECT  rectText;
    WCHAR textBuff[32];

    LONG y = ptStart.y - ((ptStart.y + nUnit)%nUnit);
    for(LONG s = 0; s < nSections; ++s, y += nUnit)
    {
        apt[0].y = y;
        apt[0].x = ptStart.x;
        GraphPtToClientPt(apt[0]);

        apt[1].y = apt[0].y;
        apt[1].x = apt[0].x + 4;

        if( !(y % (nUnit*5)) )
        {
            apt[1].x += 4;
            if( !(y % (nUnit*10)) )
            {
                apt[1].x += 4;
                
                if( m_dwOptionSet & OptCoordinates )
                {
                    ::_snwprintf_s(textBuff, 32, 30, L"%d", y);

                    rectText.left = apt[1].x+4;
                    rectText.right= rectText.left + 120;
                    rectText.top = apt[1].y-16;
                    rectText.bottom = rectText.top + 32;
                    ::DrawTextW(dc, textBuff, wcslen(textBuff),
                        &rectText, DT_SINGLELINE|DT_LEFT|DT_VCENTER);
                }
            }
        }
        ::Polyline(dc, apt, 2);
    }
}

void CLinearGraphView::SplitTimeAxis(CRectangle& rcClip, LONG& startValue, LONG& splitMethod)
{
    LONGLONG t64;
    SYSTEMTIME stBegin, stEnd;

    t64 = m_pData->timeStamp + rcClip.xMin * m_pData->freqMulti / m_pData->freqBase;
    ::FileTimeToSystemTime((FILETIME*)&t64, &stBegin);
    t64 = m_pData->timeStamp + rcClip.xMax * m_pData->freqMulti / m_pData->freqBase;
    ::FileTimeToSystemTime((FILETIME*)&t64, &stBegin);

    if( stEnd.wYear != stBegin.wYear )
    {
        splitMethod = SplitByYear;
        if( stEnd.wMonth-stBegin.wMonth + (stEnd.wYear-stBegin.wYear)*12 < 13 )
        {
            // timespan less than 13 months, split by month
            ++splitMethod;
        }
    }
    else if( stEnd.wMonth != stBegin.wMonth )
    {
        splitMethod = SplitByMonth;
        if( stEnd.wDay-stBegin.wDay + (stEnd.wMonth-stBegin.wMonth)*30 < 30 )
        {
            // timespan less than month days, split by day
            ++splitMethod;
        }
    }
    else if( stEnd.wDay != stBegin.wDay )
    {
        splitMethod = SplitByDay;
        if( stEnd.wHour-stBegin.wHour + (stEnd.wDay-stBegin.wDay)*24 < 25 )
        {
            // timespan less than 25 hours, split by hour
            ++splitMethod;
        }
    }
    else if( stEnd.wHour != stBegin.wHour )
    {
        splitMethod = SplitByHour;
        if( stEnd.wMinute-stBegin.wMinute+ (stEnd.wHour-stBegin.wHour)*60 < 61 )
        {
            // timespan less than 61 minutes, split by minute
            ++splitMethod;
        }
    }
    else if( stEnd.wMinute != stBegin.wMinute )
    {
        splitMethod = SplitByMinute;
        if( stEnd.wSecond-stBegin.wSecond + (stEnd.wMinute-stBegin.wMinute)*60 < 61 )
        {
            // timespan less than 61 seconds, split by second
            ++splitMethod;
        }
    }
    else
    {
        splitMethod = SplitBySecond; // split by second
    }
}

void CLinearGraphView::DrawHorizontalAxis(HDC dc, POINT ptStart, LONG nSections, LONG nUnit)
{
    POINT apt[2] = {ptStart, {ptStart.x+nUnit*nSections, ptStart.y}};
    GraphPtToClientPt(apt[0]);
    GraphPtToClientPt(apt[1]);
    ::Polyline(dc, apt, 2);

    RECT  rectText;
    WCHAR textBuff[32];

    LONG x = ptStart.x - ((ptStart.x + nUnit)%nUnit);
    for(LONG s = 0; s < nSections; ++s, x += nUnit)
    {
        apt[0].x = x;
        apt[0].y = ptStart.y;
        GraphPtToClientPt(apt[0]);

        apt[1].x = apt[0].x;
        apt[1].y = apt[0].y - 4;

        if( !(x % (nUnit*5)) )
        {
            apt[1].y -= 4;
            if( !(x % (nUnit*10)) )
            {
                apt[1].y -= 4;

                if( m_dwOptionSet & OptCoordinates )
                {
                    ::_snwprintf_s(textBuff, 32, 30, L"%d", x);

                    rectText.left = apt[1].x;
                    rectText.right= rectText.left + 120;
                    rectText.bottom = apt[1].y - 4;
                    rectText.top = rectText.bottom - 32;
                    ::DrawTextW(dc, textBuff, wcslen(textBuff),
                        &rectText, DT_SINGLELINE|DT_LEFT|DT_BOTTOM);
                }
            }
        }
        ::Polyline(dc, apt, 2);
    }
}

void CLinearGraphView::DrawTimeAxis(HDC dc, POINT ptStart, LONG startValue, LONG splitMethod)
{
    CRectangle rcClip;
    GetClipRectangle(rcClip);

    POINT apt[2] = {ptStart};
    GraphPtToClientPt(apt[0]);
    apt[1].x = apt[0].x + rcClip.Width();
    ::Polyline(dc, apt, 2);

    RECT  rectText;
    WCHAR textBuff[32];
}

void CLinearGraphView::DrawIndicationLine(HDC dc)
{
    // Do not draw indication line if the cursor has left the client area
    //
    CPen penLine(RGB(0x29,0x39,0x55), PS_DOT);
    HGDIOBJ hOldPen = ::SelectObject(dc, (HPEN)penLine);

    POINT ptLine[2];
    ptLine[0].x = ptLine[1].x = m_nCursorX;
    ptLine[0].y = 0;
    ptLine[1].y = Height();
    ::Polyline(dc, ptLine, 2);

    ptLine[0].y = ptLine[1].y = m_nCursorY;
    ptLine[0].x = 0;
    ptLine[1].x = Width();
    ::Polyline(dc, ptLine, 2);
    ::SelectObject(dc, hOldPen);
}

void CLinearGraphView::DrawSelectionRectangle(HDC dc, RECT& rcSelect)
{
    CSolidBrush brush(RGB(0x33,0x99,0xFF));
    ::FrameRect(dc, &rcSelect, brush);

    if( m_dwOptionSet & OptTranslucentSelection )
    {
        DrawBackground(dc, rcSelect, 0x803E96C8, 0);
    }
}

void CLinearGraphView::DrawBackground(HDC dc, RECT& rcArea, DWORD dwARGB, int r)
{
    Gdiplus::Color  clr(dwARGB);
    Gdiplus::SolidBrush brush(clr);
    Gdiplus::Graphics g(dc);

    if( r != 0 )
    {
        HRGN hRgn = ::CreateRoundRectRgn(
            rcArea.left, rcArea.top, rcArea.right, rcArea.bottom, r, r);
        Gdiplus::Region rgn(hRgn);
        g.FillRegion(&brush, &rgn);
        ::DeleteObject(hRgn);
    }
    else
    {
        g.FillRectangle(&brush, rcArea.left, rcArea.top,
            rcArea.right-rcArea.left, rcArea.bottom-rcArea.top);
    }
}

void CLinearGraphView::DrawHorizontalLine(HDC dc)
{
    CPen pen(RGB(0,255,0));
    HGDIOBJ hOldPen = ::SelectObject(dc, (HPEN)pen);

    CRectangle rcCanvas;
    GetCanvasRectangle(rcCanvas);
    
    POINT apt[2] = {{m_rcGraph.xMin, 0}, {m_rcGraph.xMax, 0}};
    GraphPtToClientPt(apt[0]);
    GraphPtToClientPt(apt[1]);

    apt[0].x = rcCanvas.xMin;
    apt[1].x = rcCanvas.xMax;
    ::Polyline(dc, apt, 2);
    ::SelectObject(dc, hOldPen);
}

void CLinearGraphView::DrawPositionLable(HDC dc)
{
    int labelSize;
    WCHAR labelBuff[128] = {0};
    if( (labelSize = GetPositionLabel(labelBuff, 128)) > 0 )
    {
        RECT rcText = {0}, rcBackground = {0};
        ::DrawText(dc, labelBuff, labelSize, &rcText, DT_CENTER|DT_CALCRECT);
        CalcPositionLabelRect(rcText, rcBackground);

        DrawBackground(dc, rcBackground, 0xC0293955);
        ::SetTextColor(dc, RGB(255,255,255));
        ::DrawTextW(dc, labelBuff, labelSize, &rcText, DT_CENTER);
        ::SetTextColor(dc, RGB(0,0,0));
    }
}

void CLinearGraphView::DrawLegend(HDC dc, RECT& rcLegend, COLORREF clr, PCWSTR szName)
{
    CFont legendFont(m_lFontHieght);
    HGDIOBJ oldFont = ::SelectObject(dc, legendFont);

    RECT rcText = rcLegend;
    ::DrawTextW(dc, szName, (int)::wcslen(szName), &rcText, DT_CALCRECT);
    rcText.left += rcLegend.right - 40 - rcText.right;
    rcText.right = rcLegend.right - 40;

    ::DrawTextW(dc, szName, (int)::wcslen(szName), &rcText,
        DT_RIGHT|DT_SINGLELINE|DT_END_ELLIPSIS|DT_VCENTER);
    ::SelectObject(dc, oldFont);

    POINT pt[2];
    pt[0].x = rcText.left - 60;
    pt[0].y = (rcText.bottom+rcText.top)/2;
    pt[1].x = rcText.left - 20;
    pt[1].y = pt[0].y;

    HPEN hPen    = ::CreatePen(PS_SOLID, 1, 0);
    HPEN hOldPen = (HPEN)::SelectObject(dc, hPen);
    ::Polyline(dc, pt, 2);
    ::DeleteObject(::SelectObject(dc, hOldPen));
}

BOOL CLinearGraphView::UpdateGraphRectangle(BOOL bUpdateClip)
{
    if( !m_pData )
    {
        return FALSE;
    }

    m_rcData.SetRect(0, m_pData->length, m_pData->data[0], m_pData->data[0]);
    for(int i = 0; i < m_pData->length; i++)
    {
        if( m_pData->data[i] > m_rcData.yMax ){ m_rcData.yMax = m_pData->data[i]; }
        if( m_pData->data[i] < m_rcData.yMin ){ m_rcData.yMin = m_pData->data[i]; }
    }

    CRectangle rcGraph(m_rcData);
    rcGraph.yMin = (m_rcData.yMin+m_rcData.yMax)/2 - 2*m_rcData.Height();
    rcGraph.yMax = (m_rcData.yMin+m_rcData.yMax)/2 + 2*m_rcData.Height();
    return SetGraphRectangle(rcGraph, bUpdateClip);
}

BOOL CLinearGraphView::UpdateGraphicsCache()
{
    if( !m_pData )
    {
        return FALSE;
    }

    m_dcGraphCache.FillRect(m_clrBackground);
    
    CRectangle rcCanvas;
    GetCanvasRectangle(rcCanvas);

    CLinearGraphPainter painter;
    if( !painter.CreateCanvas(rcCanvas.Width(), rcCanvas.Height()) )
    {
        return FALSE;
    }

    CRectangle rcClip;
    GetClipRectangle(rcClip);

    painter.DrawGraph(m_pData->data, m_pData->length, &rcClip);
    painter.Print(m_dcGraphCache, rcCanvas.xMin, rcCanvas.yMin, m_clrForeground);

    RECT rcLegend;
    if( m_dwOptionSet & OptLegend )
    {
        CalcLegendLabelRect(rcCanvas, 0, rcLegend);
        DrawLegend(m_dcGraphCache, rcLegend, m_clrForeground, m_pData->name);
    }

    // Draw comparison graphs
    //
    for(size_t i = 0; i < m_vpComparison.size(); ++i)
    {
        painter.DrawGraph(m_vpComparison[i]->data, m_vpComparison[i]->length, &rcClip);
        painter.Print(m_dcGraphCache, rcCanvas.xMin, rcCanvas.yMin, m_vColor[i]);

        if( m_dwOptionSet & OptLegend )
        {
            CalcLegendLabelRect(rcCanvas, (int)i+1, rcLegend);
            DrawLegend(m_dcGraphCache, rcLegend, m_vColor[i], m_vpComparison[i]->name);
        }
    }
    return TRUE;
}

BOOL CLinearGraphView::Refresh()
{
    return UpdateGraphicsCache() && InvalidateRect();
}

int CLinearGraphView::GetPositionLabel(PWSTR labelBuff, int cchSize) const
{
    // Do not draw label if the cursor has left the client area
    //
    if( m_nCursorX < 0 || cchSize < 64 )
    {
        return 0;
    }

    LONGLONG t;
    POINT pt = {m_nCursorX, m_nCursorY};
    ClientPtToGraphPt(pt);

    int cchText = ::_snwprintf(labelBuff, 36, L"[%d, %d]", pt.x, pt.y);
    if( GraphPtToFileTime(pt.x, t) )
    {
        SYSTEMTIME sysTime = {0};
        ::FileTimeToSystemTime((FILETIME*)&t, &sysTime);

        cchText +=::_snwprintf(labelBuff+cchText, cchSize-cchText,
            L"\n%04d/%02d/%02d %02d:%02d:%02d.%03d",
            sysTime.wYear, sysTime.wMonth, sysTime.wDay,
            sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
    }
    return cchText;
}

void CLinearGraphView::CalcSelectionRect(RECT& rcSelect) const
{
    rcSelect.left = min(m_nCursorX, m_nSelectX);
    rcSelect.top  = min(m_nCursorY, m_nSelectY);
    rcSelect.right  = max(m_nCursorX, m_nSelectX);
    rcSelect.bottom = max(m_nCursorY, m_nSelectY);
}

void CLinearGraphView::CalcPositionLabelRect(RECT& rcText, RECT& rcBackground) const
{
    LONG labelWidth = rcText.right - rcText.left;
    LONG labelHeight = rcText.bottom - rcText.top;

    RECT rcClient;
    GetClientRect(rcClient);

    if( m_nCursorX + labelWidth + 15 < rcClient.right )
    {
        rcBackground.left  = m_nCursorX + 15;
        rcBackground.right = rcBackground.left + labelWidth;
    }
    else
    {
        rcBackground.right = m_nCursorX - 15;
        rcBackground.left  = rcBackground.right - labelWidth;
    }

    if( m_nCursorY - labelHeight - 15 > rcClient.top )
    {
        rcBackground.bottom = m_nCursorY - 15;
        rcBackground.top    = rcBackground.bottom - labelHeight;
    }
    else
    {
        rcBackground.top    = m_nCursorY + 15;
        rcBackground.bottom = rcBackground.top + labelHeight;
    }

    rcText.left   += rcBackground.left;
    rcText.right  += rcBackground.left;
    rcText.top    += rcBackground.top;
    rcText.bottom += rcBackground.top;
    rcBackground.left -= 6;
    rcBackground.right += 6;
    rcBackground.top -= 6;
    rcBackground.bottom += 6;
}

BOOL CLinearGraphView::GetGraphRectangle(CRectangle& rcGraph) const
{
    if( m_pData )
    {
        rcGraph = m_rcGraph;
        return TRUE;
    }
    rcGraph.SetRect(0,0,0,0);
    return FALSE;
}

BOOL CLinearGraphView::GetClipRectangle(CRectangle& rcClip) const
{
    SCROLLINFO si = {0};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS | SIF_PAGE;

    GetScrollInfo(SB_HORZ, si);
    rcClip.xMin = si.nPos;
    rcClip.xMax = si.nPos + (int)si.nPage - 1;

    GetScrollInfo(SB_VERT, si);
    rcClip.yMax = -si.nPos;
    rcClip.yMin = -si.nPos - (int)si.nPage + 1;
    return TRUE;
}

BOOL CLinearGraphView::GetCanvasRectangle(CRectangle& rcCanvas) const
{
    RECT rcClient;
    GetClientRect(rcClient);
    
    //if( rcClient.right - rcClient.left > 80 )
    //{
    //    rcClient.left += 40;
    //    rcClient.right -= 40;
    //}
    
    //if( rcClient.bottom - rcClient.top > 80 )
    //{
    //    rcClient.top += 40;
    //    rcClient.bottom -= 40;
    //}
    rcCanvas.SetRect(rcClient.left, rcClient.right, rcClient.top-1, rcClient.bottom-1);
    return TRUE;
}

void CLinearGraphView::CalcLegendLabelRect(CRectangle& rcCanvas, size_t i, RECT& rcLegend) const
{
    // font height is negative
    //
    rcLegend.left = rcCanvas.xMin;
    rcLegend.right = rcCanvas.xMax;
    rcLegend.top = 20 - (int)i * (m_lFontHieght-4);
    rcLegend.bottom = rcLegend.top - (m_lFontHieght-4);
}

BOOL CLinearGraphView::GetDataRectangle(CRectangle& rcData) const
{
    if( m_pData )
    {
        rcData = m_rcData;
        return TRUE;
    }
    rcData.SetRect(0, 0, 0, 0);
    return FALSE;
}

BOOL CLinearGraphView::SetGraphRectangle(CRectangle& rcGraph, BOOL bResetClip)
{
    m_rcGraph = rcGraph;

    SCROLLINFO si = {0};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = bResetClip ? (SIF_RANGE|SIF_POS|SIF_PAGE) : SIF_RANGE;

    si.nMin  = m_rcGraph.xMin;
    si.nMax  = m_rcGraph.xMax;
    si.nPage = m_rcGraph.Width() + 1;
    si.nPos  = m_rcGraph.xMin;
    SetScrollInfo(SB_HORZ, si);

    si.nMin  = -m_rcGraph.yMax;
    si.nMax  = -m_rcGraph.yMin;
    si.nPage = m_rcGraph.Height() + 1;
    si.nPos  = -m_rcGraph.yMax;
    SetScrollInfo(SB_VERT, si);
    return TRUE;
}

BOOL CLinearGraphView::SetClipRectangle(CRectangle& rcClip)
{
    SCROLLINFO si = {0};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS | SIF_PAGE;

    si.nPos = rcClip.xMin;
    si.nPage = rcClip.Width() + 1;
    SetScrollInfo(SB_HORZ, si);

    si.nPos = -rcClip.yMax;
    si.nPage = rcClip.Height() + 1;
    SetScrollInfo(SB_VERT, si);
    return TRUE;
}

BOOL CLinearGraphView::ClientPtToGraphPt(POINT& pt) const
{
    if( !m_pData ){ return FALSE; }

    CRectangle rcCanvas;
    GetCanvasRectangle(rcCanvas);

    CRectangle rcClip;
    GetClipRectangle(rcClip);

    int clipWidth = rcClip.Width();
    int clipHeight = rcClip.Height();
    int canvasWidth = rcCanvas.Width();
    int canvasHeight = rcCanvas.Height();

    pt.x = (int)(rcClip.xMin + (__int64)(pt.x - rcCanvas.xMin) * clipWidth / canvasWidth);
    pt.y = (int)(rcClip.yMin - (__int64)(pt.y - rcCanvas.yMax) * clipHeight / canvasHeight);
    return TRUE;
}

BOOL CLinearGraphView::GraphPtToFileTime(LONG x, LONGLONG& t) const
{
    if( !m_pData->hasTimestamp() )
    {
        return FALSE;
    }
    t = m_pData->timeStamp + (LONGLONG)x * 10000000 * m_pData->freqBase / m_pData->freqMulti;
    return TRUE;
}

BOOL CLinearGraphView::GraphPtToClientPt(POINT& pt) const
{
    if( !m_pData ){ return FALSE; }

    CRectangle rcCanvas;
    GetCanvasRectangle(rcCanvas);

    CRectangle rcClip;
    GetClipRectangle(rcClip);

    LONG clipWidth = rcClip.Width();
    LONG clipHeight = rcClip.Height();
    LONG viewWidth = rcCanvas.Width();
    LONG viewHeight = rcCanvas.Height();

    pt.x = (LONG)(rcCanvas.xMin + (__int64)(pt.x - rcClip.xMin) * viewWidth / clipWidth);
    pt.y = (LONG)(rcCanvas.yMax - (__int64)(pt.y - rcClip.yMin) * viewHeight / clipHeight);
    return TRUE;
}

BOOL CLinearGraphView::SetOptions(DWORD dwOpts, BOOL bRefresh)
{
    m_dwOptionSet = dwOpts;
    if( IsVisible() && bRefresh )
    {
        Refresh();
    }
    return TRUE;
}

BOOL CLinearGraphView::GetNameFontSize(CString& name, LONG& lHeight) const
{
    if( !m_pData )
    {
        return FALSE;
    }

    name = m_pData->name;
    lHeight = m_lFontHieght;
    return TRUE;
}

BOOL CLinearGraphView::SetNameFontSize(CString& name, LONG lHeight)
{
    if( !m_pData )
    {
        return FALSE;
    }

    m_pData->name = name;
    m_lFontHieght = lHeight;
    return Refresh();
}

BOOL CLinearGraphView::SetColorByName(const CString& name)
{
    if( !name.GetLength() )
    {
        return FALSE;
    }

    WCHAR chd = 0;
    int ipos = name.Find(L".");
    if( ipos == -1 )
    {
        chd = name[name.GetLength()-1];
    }
    else
    {
        ipos = name.Find(L'.', ipos+1);
        if( ipos != -1 )
        {
            chd = name[ipos-1];
        }
    }

    switch( chd )
    {
    case L'Z':
    case L'z':
        SetForegroundColor(RGB(0,0,0), FALSE);
        break;
    case L'N':
    case L'n':
    case L'S':
    case L's':
        SetForegroundColor(RGB(0,0,255), FALSE);
        break;
    case L'E':
    case L'e':
    case L'W':
    case L'w':
        SetForegroundColor(RGB(255,0,0), FALSE);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

BOOL CLinearGraphView::ZoomIn()
{
    CRectangle rcClip;
    GetClipRectangle(rcClip);

    ResizeClipWindow(-rcClip.Width()/StepStretchRate, TRUE);
    ResizeClipWindow(-rcClip.Height()/StepStretchRate, FALSE);
    return Refresh();
}

BOOL CLinearGraphView::ZoomOut()
{
    CRectangle rcClip;
    GetClipRectangle(rcClip);

    ResizeClipWindow(rcClip.Width()/StepStretchRate, TRUE);
    ResizeClipWindow(rcClip.Height()/StepStretchRate, FALSE);
    return Refresh();
}