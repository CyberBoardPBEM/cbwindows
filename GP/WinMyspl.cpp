// WinMyspl.cpp
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
#include <afximpl.h>
#include "WinMyspl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

static void MyDeferClientPos(AFX_SIZEPARENTPARAMS* lpLayout, CWnd* pWnd,
    int x, int y, int cx, int cy, BOOL bScrollBar);
static void MyLayoutRowCol(CSplitterWndEx::CRowColInfo* pInfoArray,
    int nMax, int nSize, int nSizeSplitter);

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMySplitWnd, CCbSplitterWnd)

BEGIN_MESSAGE_MAP(CMySplitWnd, CCbSplitterWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void CMySplitWnd::RecalcLayout()
{
    ASSERT_VALID(this);
    ASSERT(m_nRows > 0 && m_nCols > 0); // must have at least one pane

    CRect rectClient;
    GetClientRect(rectClient);
    rectClient.InflateRect(-m_cxBorder, -m_cyBorder);

    CRect rectInside;
    GetInsideRect(rectInside);

    // layout columns (restrict to possible sizes)
    MyLayoutRowCol(m_pColInfo, m_nCols, rectInside.Width(), m_cxSplitterGap);
    MyLayoutRowCol(m_pRowInfo, m_nRows, rectInside.Height(), m_cySplitterGap);

    // adjust the panes (and optionally scroll bars)

    // give the hint for the maximum number of HWNDs
    AFX_SIZEPARENTPARAMS layout;
    layout.hDWP = ::BeginDeferWindowPos((m_nCols + 1) * (m_nRows + 1) + 1);

    // size of scrollbars
    int cx = rectClient.right - rectInside.right;
    int cy = rectClient.bottom - rectInside.bottom;

    // reposition size box
    if (m_bHasHScroll && m_bHasVScroll)
    {
        CWnd* pScrollBar = GetDlgItem(AFX_IDW_SIZE_BOX);
        ASSERT(pScrollBar != NULL);

        // fix style if necessary
        BOOL bSizingParent = (GetSizingParent() != NULL);
        // modifyStyle returns TRUE if style changes
        if (pScrollBar->ModifyStyle(SBS_SIZEGRIP|SBS_SIZEBOX,
                bSizingParent ? SBS_SIZEGRIP : SBS_SIZEBOX))
            pScrollBar->Invalidate();
        pScrollBar->EnableWindow(bSizingParent);

        // reposition the size box
        MyDeferClientPos(&layout, pScrollBar,
            rectInside.right, rectInside.bottom, cx, cy, TRUE);
    }

    // reposition scroll bars
    if (m_bHasHScroll)
    {
        int cxSplitterBox = m_cxSplitter;
        int x = rectClient.left;
        int y = rectInside.bottom;

        for (int col = 0; col < m_nCols; col++)
        {
            CWnd* pScrollBar = GetDlgItem(AFX_IDW_HSCROLL_FIRST + col);
            ASSERT(pScrollBar != NULL);
            int cx = m_pColInfo[col].nCurSize;
            if (col == 0 && m_nCols < m_nMaxCols)
                x += cxSplitterBox, cx -= cxSplitterBox;
            MyDeferClientPos(&layout, pScrollBar, x, y, cx, cy, TRUE);
            x += cx + m_cxSplitterGap;
        }
    }

    if (m_bHasVScroll)
    {
        int cySplitterBox = m_cySplitter;
        int x = rectInside.right;
        int y = rectClient.top;
        for (int row = 0; row < m_nRows; row++)
        {
            CWnd* pScrollBar = GetDlgItem(AFX_IDW_VSCROLL_FIRST + row);
            ASSERT(pScrollBar != NULL);
            int cy = m_pRowInfo[row].nCurSize;
            if (row == 0 && m_nRows < m_nMaxRows)
                y += cySplitterBox, cy -= cySplitterBox;
            MyDeferClientPos(&layout, pScrollBar, x, y, cx, cy, TRUE);
            y += cy + m_cySplitterGap;
        }
    }

    //BLOCK: Reposition all the panes
    {
        int x = rectClient.left;
        for (int col = 0; col < m_nCols; col++)
        {
            int cx = m_pColInfo[col].nCurSize;
            int y = rectClient.top;
            for (int row = 0; row < m_nRows; row++)
            {
                int cy = m_pRowInfo[row].nCurSize;
                CWnd* pWnd = GetPane(row, col);
                MyDeferClientPos(&layout, pWnd, x, y, cx, cy, FALSE);
                y += cy + m_cySplitterGap;
            }
            x += cx + m_cxSplitterGap;
        }
    }

    // move and resize all the windows at once!
    if (layout.hDWP == NULL || !::EndDeferWindowPos(layout.hDWP))
    {
        TRACE0("Warning: DeferWindowPos failed - low system resources.\n");
    }
}

/////////////////////////////////////////////////////////////////////////////

void CMySplitWnd::TrackRowSize(int y, int row)
{
    ASSERT_VALID(this);
    ASSERT(m_nRows > 1);

    CPoint pt(0, y);
    ClientToScreen(&pt);
    GetPane(row + 1, 0)->ScreenToClient(&pt);
    m_pRowInfo[row + 1].nIdealSize = m_pRowInfo[row + 1].nCurSize - pt.y;      // new size
}

void CMySplitWnd::TrackColumnSize(int x, int col)
{
    ASSERT_VALID(this);
    ASSERT(m_nCols > 1);

    CPoint pt(x, 0);
    ClientToScreen(&pt);
    GetPane(0, col + 1)->ScreenToClient(&pt);
    m_pColInfo[col + 1].nIdealSize = m_pColInfo[col + 1].nCurSize - pt.x; // new size
}

/////////////////////////////////////////////////////////////////////////////
// Generic routine:
//  for X direction: pInfo = m_pColInfo, nMax = m_nMaxCols, nSize = cx
//  for Y direction: pInfo = m_pRowInfo, nMax = m_nMaxRows, nSize = cy
//
// This routine gives size preference from right to left which
// is opposite of the standard MFC approach.

static void MyLayoutRowCol(CSplitterWndEx::CRowColInfo* pInfoArray,
    int nMax, int nSize, int nSizeSplitter)
{
    ASSERT(pInfoArray != NULL);
    ASSERT(nMax > 0);
    ASSERT(nSizeSplitter > 0);

    CSplitterWndEx::CRowColInfo* pInfo;
    int i;

    if (nSize < 0)
        nSize = 0;  // if really too small, layout as zero size

    // start with ideal sizes
    for (i = 0, pInfo = &pInfoArray[nMax - 1]; i < nMax - 1; i++, pInfo--)
    {
        if (pInfo->nIdealSize < pInfo->nMinSize)
            pInfo->nIdealSize = 0;      // too small to see
        pInfo->nCurSize = pInfo->nIdealSize;
    }
    pInfo->nCurSize = INT_MAX;  // last row/column takes the rest

    for (i = 0, pInfo = &pInfoArray[nMax - 1]; i < nMax; i++, pInfo--)
    {
        ASSERT(nSize >= 0);
        if (nSize == 0)
        {
            // no more room (set pane to be invisible)
            pInfo->nCurSize = 0;
            continue;       // don't worry about splitters
        }
        else if (nSize < pInfo->nMinSize && i != 0)
        {
            // additional panes below the recommended minimum size
            //   aren't shown and the size goes to the previous pane
            pInfo->nCurSize = 0;

            // previous pane already has room for splitter + border
            //   add remaining size and remove the extra border
            ASSERT(afxData.cxBorder2 == afxData.cyBorder2);
            (pInfo+1)->nCurSize += nSize + afxData.cxBorder2;
            nSize = 0;
        }
        else
        {
            // otherwise we can add the second pane
            ASSERT(nSize > 0);
            if (pInfo->nCurSize == 0)
            {
                // too small to see
                if (i != 0)
                    pInfo->nCurSize = 0;
            }
            else if (nSize < pInfo->nCurSize)
            {
                // this row/col won't fit completely - make as small as possible
                pInfo->nCurSize = nSize;
                nSize = 0;
            }
            else
            {
                // can fit everything
                nSize -= pInfo->nCurSize;
            }
        }

        // see if we should add a splitter
        ASSERT(nSize >= 0);
        if (i != nMax - 1)
        {
            // should have a splitter
            if (nSize > nSizeSplitter)
            {
                nSize -= nSizeSplitter; // leave room for splitter + border
                ASSERT(nSize > 0);
            }
            else
            {
                // not enough room - add left over less splitter size
                ASSERT(afxData.cxBorder2 == afxData.cyBorder2);
                pInfo->nCurSize += nSize;
                if (pInfo->nCurSize > (nSizeSplitter - afxData.cxBorder2))
                    pInfo->nCurSize -= (nSizeSplitter - afxData.cyBorder2);
                nSize = 0;
            }
        }
    }
    ASSERT(nSize == 0); // all space should be allocated
}

/////////////////////////////////////////////////////////////////////////////
// repositions client area of specified window
// assumes everything has WS_BORDER or is inset like it does
// (includes scroll bars)

static void MyDeferClientPos(AFX_SIZEPARENTPARAMS* lpLayout,
    CWnd* pWnd, int x, int y, int cx, int cy, BOOL bScrollBar)
{
    ASSERT(pWnd != NULL);
    ASSERT(pWnd->m_hWnd != NULL);

    if (bScrollBar)
    {
        // if there is enough room, draw scroll bar without border
        // if there is not enough room, set the WS_BORDER bit so that
        //   we will at least get a proper border drawn
        BOOL bNeedBorder = (cx <= AFX_CX_BORDER || cy <= AFX_CY_BORDER);
        pWnd->ModifyStyle(bNeedBorder ? 0 : WS_BORDER,
            bNeedBorder ? WS_BORDER : 0);
    }
    CRect rect(x, y, x+cx, y+cy);

    // adjust for 3d border (splitter windows have implied border)
    if ((pWnd->GetExStyle() & WS_EX_CLIENTEDGE) ||
          pWnd->IsKindOf(RUNTIME_CLASS(CSplitterWndEx)))
        rect.InflateRect(afxData.cxBorder2, afxData.cyBorder2);

    // first check if the new rectangle is the same as the current
    CRect rectOld;
    pWnd->GetWindowRect(rectOld);
    pWnd->GetParent()->ScreenToClient(&rectOld);
    if (rect != rectOld)
        AfxRepositionWindow(lpLayout, pWnd->m_hWnd, rect);
}

