// LBoxTile.cpp
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
#include    "ResTbl.h"
#include    "GmDoc.h"
#include    "LBoxTile.h"

/////////////////////////////////////////////////////////////////////////////

const int tileBorder = 3;

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTileListBox, CGrafixListBox)
    //{{AFX_MSG_MAP(CTileListBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CTileListBox::CTileListBox()
{
    m_pDoc = NULL;
    m_bDrawAllScales = FALSE;
    m_bDisplayIDs = AfxGetApp()->GetProfileInt("Settings", "DisplayIDs", 0);
}

int CTileListBox::OnItemHeight(int nIndex)
{
    ASSERT(m_pDoc != NULL);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    CTile tile;
    pTMgr->GetTile((TileID)MapIndexToItem(nIndex), &tile, fullScale);

    int nHt = 2 * tileBorder + tile.GetHeight();
    if (m_bDisplayIDs)          // See if we're drawing PieceIDs
        nHt = max(nHt, g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading);
    return nHt;
}

void CTileListBox::OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
    CRect rctItem)
{
    if (nIndex < 0)
        return;

    if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
    {
        ASSERT(m_pDoc != NULL);
        CTileManager* pTMgr = m_pDoc->GetTileManager();
        ASSERT(pTMgr != NULL);

        TileID tid = (TileID)MapIndexToItem(nIndex);
        CTile tile;
        CTile tileHalf;
        CTile tileSmall;

        pTMgr->GetTile(tid, &tile, fullScale);
        if (m_bDrawAllScales)
        {
            pTMgr->GetTile(tid, &tileHalf, halfScale);
            pTMgr->GetTile(tid, &tileSmall, smallScale);
        }

        SetupPalette(pDC);
        pDC->SaveDC();

        pDC->IntersectClipRect(&rctItem);

        COLORREF crBack = GetSysColor(nState & ODS_SELECTED ?
            COLOR_HIGHLIGHT : COLOR_WINDOW);
        CBrush brBack(crBack);

        pDC->SetTextColor(GetSysColor(nState & ODS_SELECTED ?
            COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        pDC->FillRect(&rctItem, &brBack);       // Fill background color

        int x = rctItem.left + tileBorder;
        int y = rctItem.top + tileBorder;
        if (m_bDisplayIDs)
        {
            CString str;
            str.Format("[%d] ", MapIndexToItem(nIndex));
            CFont* prvFont = (CFont*)pDC->SelectObject(CFont::FromHandle(g_res.h8ss));
            int y = rctItem.top + rctItem.Height() / 2 -
                (g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading) / 2;
            pDC->TextOut(x, y, str);
            x += pDC->GetTextExtent(str).cx;
            pDC->SelectObject(prvFont);
        }

        tile.BitBlt(pDC, x, y);

        if (m_bDrawAllScales)
        {
            x += tile.GetWidth() + 2 * tileBorder;
            y += tile.GetHeight() / 4;
            tileHalf.BitBlt(pDC, x, y);
            x += tileHalf.GetWidth() + 2 * tileBorder;
            y = rctItem.CenterPoint().y - 4;
            CBrush brSmall(tileSmall.GetSmallColor());
            CRect rctSmall(x, y, x + 8, y + 8);
            pDC->FillRect(&rctSmall, &brSmall);     // Fill background color
        }

        pDC->RestoreDC(-1);
        ResetPalette(pDC);
    }
    if (nAction & ODA_FOCUS)
        pDC->DrawFocusRect(&rctItem);
}

BOOL CTileListBox::OnDragSetup(DragInfo* pDI)
{
    if (IsMultiSelect())
    {
        pDI->m_dragType = DRAG_TILELIST;
        pDI->GetSubInfo<DRAG_TILELIST>().m_tileIDList = GetMappedMultiSelectList();// TileID array
        pDI->GetSubInfo<DRAG_TILELIST>().m_gamDoc = m_pDoc;
        pDI->m_hcsrSuggest = g_res.hcrDragTile;
    }
    else
    {
        pDI->m_dragType = DRAG_TILE;
        pDI->GetSubInfo<DRAG_TILE>().m_tileID = GetCurMapItem();      // The TileID
        pDI->GetSubInfo<DRAG_TILE>().m_gamDoc = m_pDoc;
        pDI->m_hcsrSuggest = g_res.hcrDragTile;
    }
    return TRUE;
}


