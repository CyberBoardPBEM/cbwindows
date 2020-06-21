// PalItool.cpp -- image editor tool palette
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

#include    "stdafx.h"
#include    <limits.h>
#include    "Gm.h"

#include    "PalItool.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

static char szToolPalDefPos[] = "8 32";
static char szSectSettings[] = "Settings";
static char szEntryToolPalPos[] = "ImageToolPalPos";

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CImageToolPalette, CToolBar)
    //{{AFX_MSG_MAP(CImageToolPalette)
    ON_WM_DESTROY()
    ON_WM_NCHITTEST()
    ON_WM_NCPAINT()
    ON_WM_NCCALCSIZE()
    ON_WM_NCDESTROY()
    ON_WM_NCCREATE()
    ON_WM_SYSCOMMAND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageToolPalette construction/destruction

CImageToolPalette::CImageToolPalette()
{
    m_xCurPos = INT_MIN;
    m_yCurPos = INT_MIN;
}

CImageToolPalette::~CImageToolPalette()
{
}

BOOL CImageToolPalette::Create(CWnd* pOwnerWnd)
{
    ASSERT(pOwnerWnd != NULL);

    if (m_xCurPos == INT_MIN && m_yCurPos == INT_MIN)
    {
        // First time window was created and the users hasn't set
        // the initial postion of the toolpalette. Set initial size and
        // location...
        CPoint pnt;
        GetApp()->GetProfileIntVect(szSectSettings, szEntryToolPalPos,
            szToolPalDefPos, 2, (int*)&pnt);

        m_xCurPos = pnt.x;
        m_yCurPos = pnt.y;
    }
    m_nColumns = 2;
    m_cyTopBorder = 2;
    m_cxLeftBorder = 3;
    m_cxRightBorder = 3;
    m_cyBottomBorder = 3;

    m_pOwnerWnd = pOwnerWnd;

    return CreateEx(0, "AfxControlBar", NULL, WS_POPUP | WS_SYSMENU,
        m_xCurPos, m_yCurPos, 0, 0, pOwnerWnd->GetSafeHwnd(), NULL, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// Operations

void CImageToolPalette::SaveProfileSettings()
{
    if (m_xCurPos == INT_MIN && m_yCurPos == INT_MIN)
        return;
    CRect rct;
    if (m_hWnd != NULL)
        GetWindowRect(&rct);
    else
    {
        rct.left = m_xCurPos;
        rct.top =  m_yCurPos;
    }
    GetApp()->SetProfileIntVect(szSectSettings, szEntryToolPalPos, 2,
        (int *)&rct);
}

void CImageToolPalette::SetSizes(SIZE sizeButton, SIZE sizeImage, UINT nColumns)
{
    m_nColumns = nColumns;
    CToolBar::SetSizes(sizeButton, sizeImage);
    RecalcLayout(m_nCount);
}

BOOL CImageToolPalette::SetButtons(const UINT FAR* lpIDArray,
    int nIDCount, UINT nColumns)
{
    m_nColumns = nColumns;
    RecalcLayout(nIDCount);
    return CToolBar::SetButtons(lpIDArray, nIDCount);
}

void CImageToolPalette::RecalcLayout(UINT nButtonCount)
{
    SetWindowPos(NULL, 0, 0,
        m_cxLeftBorder + (m_sizeButton.cx-1) * m_nColumns +
            m_cxRightBorder + 1 + GetNonClientPartsWidth(),
        m_cyTopBorder + m_cyBottomBorder + (m_sizeButton.cy-1) *
            ((nButtonCount + m_nColumns - 1) / m_nColumns) +
            GetNonClientPartsHeight() + 1,
        SWP_NOZORDER | SWP_NOMOVE);
    Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CImageToolPalette diagnostics

#ifdef _DEBUG
void CImageToolPalette::AssertValid() const
{
    CToolBar::AssertValid();
}

void CImageToolPalette::Dump(CDumpContext& dc) const
{
    CToolBar::Dump(dc);
}

#endif //_DEBUG

void CImageToolPalette::DoPaint(CDC* pDC)
{
    CControlBar::DoPaint(pDC);      // draws any borders

    // We need to initialize the bitmap selection process.

    DrawState ds;
    if (!PrepareDrawButton(ds))
        return;     // something went wrong

    CRect rect;
    GetClientRect(&rect);

    rect.top = m_cyTopBorder;
    rect.bottom = rect.top + m_sizeButton.cy;

    // Now draw each visible button
    for (int iButton = 0; iButton < m_nCount; )
    {
        rect.left = m_cxLeftBorder;
        for (UINT nCol = 0; nCol < m_nColumns; nCol++, iButton++)
        {
            if (iButton >= m_nCount)
                break;

            rect.right = rect.left + m_sizeButton.cx;
            if (pDC->RectVisible(&rect))
            {
                UINT nID, nStyle;
                int iImage;
                GetButtonInfo(iButton, nID, nStyle, iImage);
                DrawButton(pDC->m_hDC, rect.left, rect.top,
                    iImage, nStyle);
            }
            rect.left = rect.right - 1; // prepare for overlap
        }
        rect.top = rect.bottom-1;
        rect.bottom = rect.top + m_sizeButton.cy;
    }
    EndDrawButton(ds);
}

void CImageToolPalette::GetItemRect(int nIndex, LPRECT lpRect) const
{
    ASSERT(nIndex >= 0 && nIndex < m_nCount);
    ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));

    lpRect->left = m_cxLeftBorder +
            (nIndex - (nIndex / m_nColumns) * m_nColumns) *
            (m_sizeButton.cx-1);
    lpRect->right = lpRect->left + m_sizeButton.cx;

    lpRect->top = m_cyTopBorder + (nIndex / m_nColumns) *
            (m_sizeButton.cy-1);
    lpRect->bottom = lpRect->top + m_sizeButton.cy;
}

int CImageToolPalette::HitTest(CPoint point)  // in window relative coords
{
    if (point.x < m_cxLeftBorder ||
        point.x >= (int)((m_cxLeftBorder + m_sizeButton.cx * m_nColumns) -
            m_nColumns - 1))
        return -1;      // no X hit

    UINT nRows = (m_nCount + m_nColumns - 1) / m_nColumns;

    if (point.y < m_cyTopBorder ||
            point.y >= (int)(m_cyTopBorder + m_sizeButton.cy * nRows))
        return -1;      // no Y hit

    int iButton = ((point.y - m_cyTopBorder) / (m_sizeButton.cy-1) * m_nColumns +
            (point.x - m_cxLeftBorder) / (m_sizeButton.cx-1));
    return ( iButton < m_nCount ) ? iButton : -1;
}

/////////////////////////////////////////////////////////////////////////////
// CImageToolPalette NC message handlers

UINT CImageToolPalette::OnNcHitTest(CPoint point)
{
    CRect rct;
    GetWindowRect(&rct);
    point.x -= rct.left;
    point.y -= rct.top;
    rct.OffsetRect(-rct.left, -rct.top);

    if (point.y <= (int)m_bmapHeight && point.x < (int)m_bmapWidth)
        return HTSYSMENU;
    else if (point.y < (int)m_bmapHeight)
        return HTCAPTION;
    else
        return CToolBar::OnNcHitTest(point);
}

void CImageToolPalette::OnNcPaint()
{
    CWindowDC dc(this);

    CRect rct;
    GetWindowRect(&rct);
    rct.OffsetRect(-rct.left, -rct.top);

    // Draw the frame border

    CBrush brFrame;
    brFrame.CreateSolidBrush(::GetSysColor(COLOR_WINDOWFRAME));
    dc.FrameRect(rct, &brFrame);
    rct.bottom = m_bmapHeight + 1;
    dc.FrameRect(rct,&brFrame);

    // Fill in the caption color

    CBrush brCaption;
    brCaption.CreateSolidBrush(::GetSysColor(COLOR_ACTIVECAPTION));
    rct.left = m_bmapWidth - 1;
    rct.InflateRect(-1, -1);
    dc.FillRect(rct, &brCaption);

    // Draw the system button

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap* pPrvBMap = memDC.SelectObject(&m_bmapSysButton);
    dc.BitBlt(0, 0, m_bmapWidth, m_bmapHeight, &memDC,
        0, 0, SRCCOPY);
    memDC.SelectObject(pPrvBMap);
}

void CImageToolPalette::OnNcCalcSize(BOOL bCalcValidRects,
    NCCALCSIZE_PARAMS FAR* lpncsp)
{
    InflateRect(&lpncsp->rgrc[0], -1, -1);
    lpncsp->rgrc[0].top += m_bmapHeight;
}

void CImageToolPalette::OnNcDestroy()
{
    CToolBar::OnNcDestroy();
    m_bmapSysButton.DeleteObject();
}

BOOL CImageToolPalette::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (!CToolBar::OnNcCreate(lpCreateStruct))
        return FALSE;

    if (!m_bmapSysButton.LoadBitmap(IDB_SYSBUTTON))
        return FALSE;

    BITMAP bmap;
    m_bmapSysButton.GetObject(sizeof(BITMAP), &bmap);
    m_bmapWidth = bmap.bmWidth;
    m_bmapHeight = bmap.bmHeight;

    // Remove a bunch of useless stuff from the system menu.
    CMenu* pMenu = GetSystemMenu(FALSE);
    pMenu->RemoveMenu(7, MF_BYPOSITION);    // Second separator.
    pMenu->RemoveMenu(5, MF_BYPOSITION);    // First separator.
    pMenu->RemoveMenu(SC_RESTORE, MF_BYCOMMAND);
    pMenu->RemoveMenu(SC_MINIMIZE, MF_BYCOMMAND);
    pMenu->RemoveMenu(SC_SIZE, MF_BYCOMMAND);
    pMenu->RemoveMenu(SC_MAXIMIZE, MF_BYCOMMAND);
    pMenu->RemoveMenu(SC_TASKLIST, MF_BYCOMMAND);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////
// Non NC Message Stuff

LRESULT CImageToolPalette::WindowProc(UINT message, WPARAM wParam,
    LPARAM lParam)
{
    if (message == WM_COMMAND && lParam == 0)
    {
        return CWnd::WindowProc(message, wParam, lParam);
    }
    else
        return CToolBar::WindowProc(message, wParam, lParam);
}

void CImageToolPalette::OnDestroy()
{
    CToolBar::OnDestroy();
    // Save the current position of the palette
    CRect rct;
    GetWindowRect(&rct);
    m_xCurPos = rct.left;
    m_yCurPos = rct.top;
}

void CImageToolPalette::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == SC_MOUSEMENU || (nID & 0xFFF0) == SC_KEYMENU)
    {
        CRect rct;
        GetWindowRect(&rct);
        CRect sysRct(1, 1, m_bmapWidth - 1, m_bmapHeight);

        {
            CWindowDC dc(this);
            dc.InvertRect(&sysRct);
        }

        CMenu *pSysMenu = GetSystemMenu(FALSE);
        pSysMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,
            rct.left, rct.top + m_bmapHeight, this, &rct);
        {
            CWindowDC dc(this);
            dc.InvertRect(&sysRct);
        }
    }
    else
        CToolBar::OnSysCommand(nID, lParam);
}

BOOL CImageToolPalette::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // Hot potatoe (<- Dan Quale says so) system menu commands so they
    // are processed as system commands.
    if (wParam == SC_MOVE)
        return (BOOL) SendMessage(WM_SYSCOMMAND, SC_MOVE, NULL);
    else if (wParam == SC_CLOSE)
        return (BOOL) SendMessage(WM_SYSCOMMAND, SC_CLOSE, NULL);
    else
        return CToolBar::OnCommand(wParam, lParam);
}


