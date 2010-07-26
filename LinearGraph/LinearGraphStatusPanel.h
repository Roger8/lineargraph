#pragma once
#include "LinearGraph.h"
#include "LinearGraphDoc.h"

class CLinearGraphFrameWnd;

class CLinearGraphStatusPanel : public CWindow
{
public:
     CLinearGraphStatusPanel();
    ~CLinearGraphStatusPanel();

    BOOL Create(CLinearGraphFrameWnd* pa);
    void SetInformation(const CDataObjectPtr& pData);
    void ClearInformation();

protected:
    void OnPaint(HDC dc, PAINTSTRUCT& ps);

public:
    CLinearGraphFrameWnd*  m_pFrame;
    CString     m_strFileName;
    CString     m_strDateTime;
    CString     m_strPointCount;
    CString     m_strFrequency;
};