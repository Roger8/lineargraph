#include "LinearGraphStatusPanel.h"
#include "LinearGraphFrame.h"

CLinearGraphStatusPanel::CLinearGraphStatusPanel() : m_pFrame(0)
{

}

CLinearGraphStatusPanel::~CLinearGraphStatusPanel()
{

}

BOOL CLinearGraphStatusPanel::Create(CLinearGraphFrameWnd* pa)
{
    return CWindow::Create(WS_CHILD|WS_VISIBLE, pa->m_hWnd);
}

void CLinearGraphStatusPanel::OnPaint(HDC dc, PAINTSTRUCT& ps)
{
    RECT rcClient;
    GetClientRect(rcClient);

    ::FillRect(dc, &rcClient, CLinearGraphApp::GetThemeBrush());
    ::DeleteObject(::SelectObject(dc, CLinearGraphApp::GetThemeFont()));
    ::SetTextColor(dc, RGB(255,255,255));
    ::SetBkMode(dc, TRANSPARENT);

    int  cchLength = 0;
    RECT rcText = rcClient;

    if( cchLength = m_strPointCount.GetLength() )
    {
        ::DrawTextW(dc, m_strPointCount, cchLength, &rcText,
            DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
        rcText.right -= 120;
    }

    if( cchLength = m_strDateTime.GetLength() )
    {
        ::DrawTextW(dc, m_strDateTime, cchLength, &rcText,
            DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
        rcText.right -= 200;
    }

    if( cchLength = m_strFrequency.GetLength() )
    {
        ::DrawTextW(dc, m_strFrequency, cchLength, &rcText,
            DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
        rcText.right -= 120;
    }

    if( cchLength = m_strFileName.GetLength() )
    {
        ::DrawTextW(dc, m_strFileName, cchLength, &rcText,
            DT_SINGLELINE|DT_VCENTER|DT_PATH_ELLIPSIS);
    }
}

void CLinearGraphStatusPanel::SetInformation(const CDataObjectPtr& pData)
{
    m_strFileName = pData->ownerFile;

    if( pData->hasTimestamp() )
    {
        SYSTEMTIME stDate = {0};
        ::FileTimeToSystemTime((FILETIME*)&(pData->timeStamp), &stDate);

        m_strDateTime.Format(L"%04d/%02d/%02d %02d:%02d:%02d.%03d",
            (DWORD)stDate.wYear, (DWORD)stDate.wMonth, (DWORD)stDate.wDay,
            (DWORD)stDate.wHour, (DWORD)stDate.wMinute, (DWORD)stDate.wSecond,
            (DWORD)stDate.wMilliseconds
            );

        m_strFrequency.Format(L"%.4fHz", pData->getFrequency());
    }
    
    m_strPointCount.Format(L"%dpts", pData->length);
    InvalidateRect();
}

void CLinearGraphStatusPanel::ClearInformation()
{
    m_strFileName.Empty();
    m_strDateTime.Empty();
    m_strFrequency.Empty();
    m_strPointCount.Empty();

    InvalidateRect();
}
