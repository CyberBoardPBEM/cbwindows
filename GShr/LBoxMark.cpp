// LBoxMark.cpp
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
#ifdef GPLAY
#include    "GamDoc.h"
#else
#include    "GmDoc.h"
#endif
#include    "ResTbl.h"
#include    "Marks.h"
#include    "LBoxMark.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

const int tileBorder = 3;
const int tileGap = 6;

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CMarkListBox, CTileBaseListBox)
    //{{AFX_MSG_MAP(CMarkListBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CMarkListBox::CMarkListBox()
{
    m_pDoc = NULL;
    m_eTrayViz = mtrayVizNormal;
}

/////////////////////////////////////////////////////////////////////////////

CTileManager* CMarkListBox::GetTileManager()
{
    ASSERT(m_pDoc != NULL);
    ASSERT(m_pDoc->GetTileManager() != NULL);
    return m_pDoc->GetTileManager();
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing

BOOL CMarkListBox::OnIsToolTipsEnabled()
{
#ifdef GPLAY
    return m_pDoc->IsShowingObjectTips();
#else           // Always show them in the designer
    return TRUE;
#endif
}

int  CMarkListBox::OnGetHitItemCodeAtPoint(CPoint point, CRect& rct)
{
    BOOL bOutsideClient;
    int nIndex = ItemFromPoint(point, bOutsideClient);
    if (nIndex >= 65535 || GetCount() <= 0)
        return -1;

    CMarkManager* pMMgr = m_pDoc->GetMarkManager();

    MarkID mid = (MarkID)MapIndexToItem(nIndex);
    MarkDef* pMark = pMMgr->GetMark(mid);
    TileID tid = pMark->m_tid;

    GetTileRectsForItem(nIndex, tid, nullTid, rct, rct);

    return rct.PtInRect(point) ? (int)mid : -1;
}

void CMarkListBox::OnGetTipTextForItemCode(int nItemCode,
    CString& strTip, CString& strTitle)
{
    MarkID mid = (MarkID)nItemCode;
    strTip = m_pDoc->GetGameElementString(MakeMarkerElement(mid));
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMarkListBox::OnDoesItemHaveTipText(int nItem)
{
    ASSERT(m_eTrayViz == mtrayVizNormal);
    MarkID mid = (MarkID)MapIndexToItem(nItem);
    return m_pDoc->HasGameElementString(MakeMarkerElement(mid));
}

/////////////////////////////////////////////////////////////////////////////

void CMarkListBox::SelectMarker(MarkID mid)
{
    int nIndex = MapItemToIndex((int)mid);
    // NO LONGER IMPOSSIBLE SINCE HIDE ALL MARKERS. ASSERT(nIndex != -1);
    if (nIndex == -1)
    {
        if (GetCount() >= 1)
            SetCurSel(0);           // Just select the first one.
    }

    ShowListIndex(nIndex);
    SetCurSel(nIndex);
}

/////////////////////////////////////////////////////////////////////////////

void CMarkListBox::ShowListIndex(int nPos)
{
    if (nPos < GetTopIndex())
    {
        SetTopIndex(nPos);
        return;
    }
    CRect rct;
    GetItemRect(nPos, &rct);
    CRect rctClient;
    GetClientRect(&rctClient);
    if (rct.IntersectRect(&rct, &rctClient))
        return;

    SetTopIndex(nPos);
}

/////////////////////////////////////////////////////////////////////////////

int CMarkListBox::OnItemHeight(int nIndex)
{
    if (m_eTrayViz == mtrayVizNormal)
    {
        CMarkManager* pMMgr = m_pDoc->GetMarkManager();
        ASSERT(pMMgr);
        MarkDef* pMark = pMMgr->GetMark((MarkID)MapIndexToItem(nIndex));
        ASSERT(pMark->m_tid != nullTid);

        TileID tid = pMark->m_tid;

        return DoOnItemHeight(tid, nullTid);
    }
    else
    {
        // Hidden markers. Account for drawing the supplied text.
        int nHeight = g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading;
        return nHeight;
    }
}

void CMarkListBox::OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
    CRect rctItem)
{
    if (nIndex < 0)
        return;

    if (m_eTrayViz == mtrayVizNormal)
    {
        if (m_pDoc == NULL)
            return;

        CMarkManager* pMMgr = m_pDoc->GetMarkManager();
        ASSERT(pMMgr);

        MarkDef* pMark = pMMgr->GetMark((MarkID)MapIndexToItem(nIndex));
        ASSERT(pMark->m_tid != nullTid);

        TileID tid = pMark->m_tid;
        DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tid, nullTid);
    }
    else
    {
        if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
        {
            // Hidden markers. Draw the supplied text.
            pDC->SetTextAlign(TA_TOP | TA_LEFT);
            CBrush brBack(GetSysColor(nState & ODS_SELECTED ?
                COLOR_HIGHLIGHT : COLOR_WINDOW));
            pDC->FillRect(&rctItem, &brBack);       // Fill background color
            pDC->SetBkMode(TRANSPARENT);
            CFont* pPrvFont = pDC->SelectObject(CFont::FromHandle(g_res.h8ss));
            pDC->SetTextColor(GetSysColor(nState & ODS_SELECTED ?
                COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
            pDC->TextOut(rctItem.left, rctItem.top, m_strHiddenString);
            pDC->SelectObject(pPrvFont);
        }
        if (nAction & ODA_FOCUS)
            pDC->DrawFocusRect(&rctItem);
    }
}

BOOL CMarkListBox::OnDragSetup(DragInfo* pDI)
{
#ifdef GPLAY
    if (m_pDoc->IsPlaying())
        return 0;                       // Drags not supported during play
#endif
    pDI->m_dragType = DRAG_MARKER;
    pDI->GetSubInfo<DRAG_MARKER>().m_markID = GetCurMapItem();          // The MarkID
    pDI->GetSubInfo<DRAG_MARKER>().m_gamDoc = m_pDoc;
    pDI->m_hcsrSuggest = g_res.hcrDragTile;
    return TRUE;
}

