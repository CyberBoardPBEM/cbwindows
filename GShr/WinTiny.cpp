// WinTiny.cpp - Window with tiny caption bar and system button.
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
#ifdef      GPLAY
    #include    "Gp.h"
#else
    #include    "Gm.h"
    #include    "GmDoc.h"
#endif
#include "wintiny.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTinyWnd, CWnd)
    //{{AFX_MSG_MAP(CTinyWnd)
    ON_WM_NCPAINT()
    ON_WM_NCCALCSIZE()
    ON_WM_NCCREATE()
    ON_WM_NCHITTEST()
    ON_WM_NCDESTROY()
    ON_WM_SYSCOMMAND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTinyWnd

CTinyWnd::CTinyWnd()
{
}

CTinyWnd::~CTinyWnd()
{
}

BOOL CTinyWnd::Create(LPCSTR lpszClassName, DWORD dwStyle,
    int x, int y, int nWidth, int nHeight, CWnd* pParentWnd,
    UINT nID, LPSTR lpParam /* = NULL*/)
{
    return CreateEx(0, lpszClassName, "", dwStyle, x, y, nWidth, nHeight,
        pParentWnd->GetSafeHwnd(), (HMENU)nID, lpParam);
}

/////////////////////////////////////////////////////////////////////////////
// CTinyWnd message handlers

BOOL CTinyWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // Hot potatoe (<- Dan Quale says so) system menu commands so they
    // are processed as system commands.
    if (wParam == SC_MOVE)
        return (BOOL) SendMessage(WM_SYSCOMMAND, SC_MOVE, NULL);
    else if (wParam == SC_CLOSE)
        return (BOOL) SendMessage(WM_SYSCOMMAND, SC_CLOSE, NULL);
    else
        return CWnd::OnCommand(wParam, lParam);
}

void CTinyWnd::OnNcPaint()
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

void CTinyWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
    InflateRect(&lpncsp->rgrc[0], -1, -1);
    lpncsp->rgrc[0].top += m_bmapHeight;
}

BOOL CTinyWnd::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (!CWnd::OnNcCreate(lpCreateStruct))
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

void CTinyWnd::OnNcDestroy()
{
    CWnd::OnNcDestroy();
    m_bmapSysButton.DeleteObject();
}

UINT CTinyWnd::OnNcHitTest(CPoint point)
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
        return CWnd::OnNcHitTest(point);
}

void CTinyWnd::OnSysCommand(UINT nID, LPARAM lParam)
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
        CWnd::OnSysCommand(nID, lParam);
}
