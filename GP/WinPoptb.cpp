// WinPoptb.cpp : implementation file
//
// Copyright (c) 1994-2020 By Dale L. Larson, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//


#include "stdafx.h"
#include "Gp.h"
#include "Gmisc.h"
#include "GdiTools.h"
#include "WinPoptb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CTinyBoardPopup, CWnd)
    //{{AFX_MSG_MAP(CTinyBoardPopup)
    ON_WM_RBUTTONDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_CHAR()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardPopup

CTinyBoardPopup::CTinyBoardPopup()
{
    m_pWnd = NULL;
    m_bRotate180 = FALSE;
}

CTinyBoardPopup::~CTinyBoardPopup()
{
}

/////////////////////////////////////////////////////////////////////////////

BOOL CTinyBoardPopup::Create(CWnd* pParent, CPoint ptCenter)
{
    CRect rct(CPoint(0,0), m_vsize);
    DWORD dwStyle = WS_BORDER | WS_POPUP | WS_VISIBLE;
    DWORD dwExStyle = WS_EX_CLIENTEDGE;

    AdjustWindowRectEx(&rct, dwStyle, FALSE, dwExStyle);
    rct.OffsetRect(ptCenter.x - rct.Width()/2, ptCenter.y - rct.Height()/2);

    // Make sure on screen...
    CRect rctWrk;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rctWrk, 0);
    if (rct.right > rctWrk.right)
        rct.OffsetRect(rctWrk.right - rct.right, 0);
    if (rct.bottom > rctWrk.bottom)
        rct.OffsetRect(0, rctWrk.bottom - rct.bottom);

    if (rct.Width() > rctWrk.Width())
    {
        rct.left = rctWrk.left;
        rct.right = rctWrk.right;
    }
    if (rct.Height() > rctWrk.Height())
    {
        rct.top = rctWrk.top;
        rct.bottom = rctWrk.bottom;
    }

    return CWnd::CreateEx(dwExStyle,
        AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)(COLOR_BTNFACE + 1)), "", dwStyle, rct, pParent, NULL, NULL);
}

int CTinyBoardPopup::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    SetCapture();

    CRect rct;
    GetWindowRect(&rct);
    CPoint pnt = rct.CenterPoint();
    SetCursorPos(pnt.x, pnt.y);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardPopup message handlers

void CTinyBoardPopup::PostNcDestroy()
{
    delete this;
}

void CTinyBoardPopup::OnPaint()
{
    CPaintDC dc(this);          // device context for painting
    SetupPalette(dc);

    CDC dcMem;
    CRect oRct;

    dcMem.CreateCompatibleDC(&dc);
    CBitmap* pPrvBMap = dcMem.SelectObject(&m_bmap);
    SetupPalette(dcMem);

    CRect rctClient;
    GetClientRect(rctClient);

    if (m_bRotate180)
    {
        dc.StretchBlt(0, 0, rctClient.right, rctClient.bottom, &dcMem,
            m_vsize.cx - 1,m_vsize.cy - 1, -m_vsize.cx, -m_vsize.cy, SRCCOPY);
    }
    else
    {
        dc.StretchBlt(0, 0, rctClient.right, rctClient.bottom, &dcMem, 0, 0,
            m_vsize.cx, m_vsize.cy, SRCCOPY);
    }

    ResetPalette(dcMem);
    ResetPalette(dc);
    dcMem.SelectObject(pPrvBMap);
}

void CTinyBoardPopup::ProcessBoardHit(UINT nFlags, CPoint point)
{
    ReleaseCapture();

    CRect rctClient;
    GetClientRect(&rctClient);
    if (rctClient.PtInRect(point))
    {
        ScalePoint(point, m_wsize, rctClient.Size());
        if (m_bRotate180)
            point = CPoint(m_wsize.cx - point.x, m_wsize.cy - point.y);
        m_pWnd->SendMessage(WM_CENTERBOARDONPOINT, (WPARAM)&point);
    }
    DestroyWindow();
}

void CTinyBoardPopup::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_ESCAPE)
    {
        ReleaseCapture();
        DestroyWindow();
    }
}

void CTinyBoardPopup::OnRButtonDown(UINT nFlags, CPoint point)
{
    ProcessBoardHit(nFlags, point);
}

void CTinyBoardPopup::OnLButtonDown(UINT nFlags, CPoint point)
{
    ProcessBoardHit(nFlags, point);
}
