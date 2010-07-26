#pragma once
#include "LinearGraph.h"
#include "LinearGraphDoc.h"

class CWindow;
class CDataObjectPtr;
class CLinearGraphFrameWnd;
class CLinearGraphView;

class CLinearGraphPainter
{
public:
    CLinearGraphPainter();
    ~CLinearGraphPainter();

    // Create a virtual canvas
    //
    BOOL CreateCanvas(int cx, int cy);

    //  Draw a graph on the canvas
    //  This method will clear the canvas before any drawing, thus the previous drawn
    // graph will be dropped.
    //
    BOOL DrawGraph(LONG* apData, LONG nData, CRectangle* rcClip = 0);

    // Print the graph on a physical device or memory DC
    // xDst and yDst specify the position to print the graph
    // clr specifies the graph color
    //
    BOOL Print(HDC dc, int xDst, int yDst, COLORREF clr = 0);

    // Delete the virtual canvas
    //
    BOOL DeleteCanvas();

protected:
    // Buffer allocated for the canvas
    //
    LONG*   m_pAlloc;

    //
    // In case the graph is larger than canvas, which will be the most
    // often circumstances we will face, the graph will be stretched to
    // fit the canvas. Thus two or more points will fall to the same
    // position on the canvas. To draw the graph without distortion we
    // need to keep several important records.
    // The canvas, virtual of course, is described by these records
    //

    // Extreme values at each position
    //
    LONG*       m_pMaxValue;
    LONG*       m_pMinValue;

    // First and last value that fall to the same position
    //
    LONG*       m_pFirstValue;
    LONG*       m_pLastValue;

    // Count the points that fall to the same position
    //
    LONG*       m_pPtCount;

    // Size of canvas. Position of canvas is not necessary until the 
    // time we need to print the graph on a physical device such as
    // screen or memory DC.
    //
    SIZE        m_sizeCanvas;

    // Rectangle that used to clip the graph. Parts of graph
    // that lays out of this rectangle will not be drawn.
    // 
    CRectangle  m_rcClip;
};

typedef CLinearGraphView* CLinearGraphViewPtr;

typedef std::vector<COLORREF>   CColorVect;

class CLinearGraphView : public CWindow
{
protected:
    // Do not construct or destruct CLinearGraphView object explicitly
    // Use DynamicCreate method instead
    //
     CLinearGraphView();
    ~CLinearGraphView();

public:
    // Create a CLinearGraphView object
    // Deleting the object explicitly is neither allowed nor needed.
    // The object will be deleted automatically by the time a WM_NCDESTROY
    // message is sent to the window.
    //
    static CLinearGraphViewPtr DynamicCreate(CWindow* pa);

    enum OptionFlags
    {
        OptNone = 0,
        OptAll = 0xFFFFFFFF,

        OptRuler = 0x0001,
        OptHorizontalLine = 0x0002,
        OptPositionLabel = 0x0004,
        OptTranslucentSelection = 0x0008,
        OptLegend = 0x0010,
        OptCoordinates = 0x0020,
        OptSelectionTransform = 0x1000,
    };

    enum Constants
    {
        DefaultFontHeight = -12,
        StepStretchRate = 16
    };

    COLORREF GetForegroundColor() const;
    COLORREF GetBackgroundColor() const;
    BOOL GetNameFontSize(CString& name, LONG& lHeight) const;
    BOOL SetOptions(DWORD dwOpts, BOOL bRefresh = TRUE);
    BOOL BindData(CDataObjectPtr& pdo);
    BOOL UpdateGraphRectangle(BOOL bUpdateClip = FALSE);
    BOOL AddComparison(CLinearGraphView& pvComp, BOOL bRefresh);
    BOOL ClearComparison();
    BOOL PrintGraph(PCWSTR dirName, PCWSTR extensionName);
    BOOL Refresh();
    BOOL SetClipRectangle(CRectangle& rcClip);
    BOOL SetColorByName(const CString& name);
    BOOL SetForegroundColor(COLORREF clr, BOOL bRefresh);
    BOOL SetBackgroundColor(COLORREF clr, BOOL bRefresh);
    BOOL SetNameFontSize(CString& name, LONG lHeight);
    BOOL SetDateTime(SYSTEMTIME& stDateTime, DWORD fbase, DWORD fmulti);
    BOOL SetDateTime(FILETIME& ftDateTime, DWORD fbase, DWORD fmulti);
    BOOL GetDateTime(SYSTEMTIME& stDateTime, DWORD& fbase, DWORD& fmulti) const;
    BOOL SetMaxStretch(LONG lMaxStretch = 32);
    BOOL GetClipRectangle(CRectangle& rcClip) const;
    BOOL GetCanvasRectangle(CRectangle& rcCanvas) const;
    BOOL GetDataRectangle(CRectangle& rcData) const;
    BOOL ClientPtToGraphPt(POINT& pt) const;
    BOOL GraphPtToFileTime(LONG x, LONGLONG& t) const;
    BOOL GraphPtToClientPt(POINT& pt) const;

    BOOL ZoomIn();
    BOOL ZoomOut();

    //  If there is no any comparison data, this method returns the same rectangle
    // as GetDataRectangle() method. Otherwise this method returns the largest
    // boundary of all the data.
    //
    BOOL GetGraphRectangle(CRectangle& rcGraph) const;

    CDataObjectPtr& GetDataObject();

protected:
    virtual int  PreProcessMessage(UINT msg, WPARAM wp, LPARAM lp);
    virtual BOOL UpdateGraphicsCache();

    void OnPaint(HDC dc, PAINTSTRUCT& ps);
    void OnSizeChanged(DWORD dwFlags, int cx, int cy);
    void OnMouseMove(DWORD dwFlags, int x, int y);
    void OnMouseWheel(DWORD dwKeyFlag, int dz);
    void OnMouseLeave();
    void OnLButtonDown(DWORD dwFlags, int x, int y);
    void OnLButtonUp(DWORD dwFlags, int x, int y);
    void OnRButtonDown(DWORD dwFlags, int x, int y);
    void OnSelectTransform(RECT& rcSelect);
    void OnScroll(UINT msg, DWORD dwAction);

private:
    LONG GetGraphYPos(size_t iGraph) const;
    int  GetPositionLabel(PWSTR labelBuff, int cchSize) const;
    BOOL SetGraphRectangle(CRectangle& rcGraph, BOOL bResetClip = FALSE);
    void SendParentalNotification(Messages msg);
    BOOL InitGraphCache();
    void CalcSelectionRect(RECT& rcSelect) const;
    void CalcPositionLabelRect(RECT& rcText, RECT& rcBackground) const;
    void CalcLegendLabelRect(CRectangle& rcCanvas, size_t i, RECT& rcLegend) const;
    void DrawRuler(HDC dc, CRectangle& rcCanvas, CRectangle& rcClip);
    void DrawIndicationLine(HDC dc);
    void DrawSelectionRectangle(HDC dc, RECT& rcSelect);
    void DrawBackground(HDC dc, RECT& rcArea, DWORD dwARGB, int r = 3);
    void DrawHorizontalLine(HDC dc);
    void DrawPositionLable(HDC dc);
    void DrawLegend(HDC dc, RECT& rcLegend, COLORREF clr, PCWSTR szName);
    void DrawVerticalAxis(HDC dc, POINT ptStart, LONG nSections, LONG nUnit);
    void DrawHorizontalAxis(HDC dc, POINT ptStart, LONG nSections, LONG nUnit);
    void ResizeClipWindow(int ds, BOOL bHorz);
    void MoveClipWindow(int ds, BOOL bHorz);

protected:
    DWORD       m_dwOptionSet;
    CBufferedDC m_dcGraphCache;
    COLORREF    m_clrBackground;
    COLORREF    m_clrForeground;
    
    //  An in-client cursor position ranges from 0 to 32767 while other
    // values indicate that the cursor has left the client area
    //
    int m_nCursorX, m_nCursorY;
    int m_nSelectX, m_nSelectY;
    
    BOOL        m_bTrackSelection;
    LONG        m_lMaxStretch;
    LONG        m_lFontHieght;

    CDataObjectPtr      m_pData;
    CRectangle          m_rcData;
    CRectangle          m_rcGraph;
    CDataObjectPtrVect  m_vpComparison;
    CColorVect          m_vColor;
};

typedef std::vector<CLinearGraphViewPtr>  CLinearGraphViewPtrVect;