// LBoxTile.cpp
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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
wxIMPLEMENT_DYNAMIC_CLASS(CTileListBoxWx, CB::VListBoxHScroll);

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
    m_bDisplayIDs = AfxGetApp()->GetProfileInt("Settings"_cbstring, "DisplayIDs"_cbstring, 0);
}

CSize CTileListBox::OnItemSize(size_t nIndex) const
{
    ASSERT(m_pDoc != NULL);
    const CTileManager& pTMgr = m_pDoc->GetTileManager();

    TileID tid = MapIndexToItem(nIndex);
    CTile tile = pTMgr.GetTile(tid, fullScale);

    LONG nHt = 2 * tileBorder + tile.GetHeight();
    if (m_bDisplayIDs)          // See if we're drawing PieceIDs
        nHt = CB::max(nHt, g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading);

    int x = tileBorder;
    if (m_bDisplayIDs)
    {
        CB::string str = std::format("[{}] ", MapIndexToItem(nIndex));
        // only using DC to measure text, so const_cast safe;
        CClientDC pDC(const_cast<CTileListBox*>(this));
        pDC.SaveDC();
        CFont* prvFont = (CFont*)pDC.SelectObject(CFont::FromHandle(g_res.h8ss));
        x += pDC.GetTextExtent(str).cx;
        pDC.SelectObject(prvFont);
        pDC.RestoreDC(-1);
    }

    x += tile.GetWidth() + tileBorder;

    if (m_bDrawAllScales)
    {
        CTile tileHalf = pTMgr.GetTile(tid, halfScale);
        x += tileHalf.GetWidth() + 2 * tileBorder;
        x += 8;
    }

    return CSize(x, nHt);
}

void CTileListBox::OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
    CRect rctItem) const
{
    // see https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-drawitemstruct
    if (nIndex == size_t(UINT(-1)))
        return;

    if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
    {
        ASSERT(m_pDoc != NULL);
        const CTileManager& pTMgr = m_pDoc->GetTileManager();

        TileID tid = MapIndexToItem(nIndex);

        CTile tile = pTMgr.GetTile(tid, fullScale);

        pDC.SaveDC();

        pDC.IntersectClipRect(&rctItem);

        COLORREF crBack = GetSysColor(nState & ODS_SELECTED ?
            COLOR_HIGHLIGHT : COLOR_WINDOW);
        CBrush brBack(crBack);

        pDC.SetTextColor(GetSysColor(nState & ODS_SELECTED ?
            COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        pDC.FillRect(&rctItem, &brBack);       // Fill background color

        int x = rctItem.left + tileBorder;
        int y = rctItem.top + tileBorder;
        if (m_bDisplayIDs)
        {
            CB::string str = std::format("[{}] ", MapIndexToItem(nIndex));
            CFont* prvFont = (CFont*)pDC.SelectObject(CFont::FromHandle(g_res.h8ss));
            int prevBkMode = pDC.SetBkMode(TRANSPARENT);
            int y = rctItem.top + rctItem.Height() / 2 -
                (g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading) / 2;
            pDC.TextOut(x, y, str);
            x += pDC.GetTextExtent(str).cx;
            pDC.SetBkMode(prevBkMode);
            pDC.SelectObject(prvFont);
        }

        tile.BitBlt(pDC, x, y);

        if (m_bDrawAllScales)
        {
            x += tile.GetWidth() + 2 * tileBorder;
            y += tile.GetHeight() / 4;
            CTile tileHalf = pTMgr.GetTile(tid, halfScale);
            tileHalf.BitBlt(pDC, x, y);
            x += tileHalf.GetWidth() + 2 * tileBorder;
            y = rctItem.CenterPoint().y - 4;
            CTile tileSmall = pTMgr.GetTile(tid, smallScale);
            CBrush brSmall(tileSmall.GetSmallColor());
            CRect rctSmall(x, y, x + 8, y + 8);
            pDC.FillRect(&rctSmall, &brSmall);     // Fill background color
        }

        pDC.RestoreDC(-1);
    }
    if (nAction & ODA_FOCUS)
        pDC.DrawFocusRect(&rctItem);
}

BOOL CTileListBox::OnDragSetup(DragInfo& pDI) const
{
    if (IsMultiSelect())
    {
        pDI.SetDragType(DRAG_TILELIST);
        pDI.GetSubInfo<DRAG_TILELIST>().m_tileIDList = &GetMappedMultiSelectList();
        pDI.GetSubInfo<DRAG_TILELIST>().m_size = GetDragSize();
        pDI.GetSubInfo<DRAG_TILELIST>().m_gamDoc = m_pDoc;
        pDI.m_hcsrSuggest = g_res.hcrDragTile;
    }
    else
    {
        pDI.SetDragType(DRAG_TILE);
        pDI.GetSubInfo<DRAG_TILE>().m_tileID = GetCurMapItem();
        pDI.GetSubInfo<DRAG_TILE>().m_size = GetDragSize();
        pDI.GetSubInfo<DRAG_TILE>().m_gamDoc = m_pDoc;
        pDI.m_hcsrSuggest = g_res.hcrDragTile;
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CTileListBoxWx::CTileListBoxWx()
{
    m_pDoc = NULL;
    m_bDrawAllScales = FALSE;
    m_bDisplayIDs = AfxGetApp()->GetProfileInt("Settings"_cbstring, "DisplayIDs"_cbstring, 0);
}

wxSize CTileListBoxWx::GetItemSize(size_t nIndex) const
{
    wxASSERT(m_pDoc != NULL);
    const CTileManager& pTMgr = m_pDoc->GetTileManager();

    TileID tid = MapIndexToItem(nIndex);
    CTile tile = pTMgr.GetTile(tid, fullScale);

    LONG nHt = 2 * tileBorder + tile.GetHeight();
    if (m_bDisplayIDs)          // See if we're drawing PieceIDs
        nHt = CB::max(nHt, g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading);

    int x = tileBorder;
    if (m_bDisplayIDs)
    {
        CB::string str = std::format("[{}] ", MapIndexToItem(nIndex));
        // only using DC to measure text, so const_cast safe;
        wxWindowDC pDC(const_cast<CTileListBoxWx*>(this));
        pDC.SetFont(g_res.h8ssWx);
        x += pDC.GetTextExtent(str).x;
    }

    x += tile.GetWidth() + tileBorder;

    if (m_bDrawAllScales)
    {
        CTile tileHalf = pTMgr.GetTile(tid, halfScale);
        x += tileHalf.GetWidth() + 2 * tileBorder;
        x += 8;
    }

    return wxSize(x, nHt);
}

void CTileListBoxWx::OnDrawItem(wxDC& pDC, const wxRect& rctItem, size_t nIndex) const
{
    wxASSERT(m_pDoc != NULL);
    const CTileManager& pTMgr = m_pDoc->GetTileManager();

    TileID tid = MapIndexToItem(nIndex);

    CTile tile = pTMgr.GetTile(tid, fullScale);

    pDC.SetClippingRegion(rctItem);

    int x = rctItem.GetLeft() + tileBorder;
    int y = rctItem.GetTop() + tileBorder;
    if (m_bDisplayIDs)
    {
        CB::string str = std::format("[{}] ", MapIndexToItem(nIndex));
        pDC.SetFont(g_res.h8ssWx);
        int prevBkMode = pDC.GetBackgroundMode();
        pDC.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
        int y = rctItem.GetTop() + rctItem.GetHeight() / 2 -
            (g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading) / 2;
        pDC.DrawText(str, CalcScrolledX(x), y);
        x += pDC.GetTextExtent(str).x;
        pDC.SetBackgroundMode(prevBkMode);
    }

    tile.BitBlt(pDC, CalcScrolledX(x), y);

    if (m_bDrawAllScales)
    {
        x += tile.GetWidth() + 2 * tileBorder;
        y += tile.GetHeight() / 4;
        CTile tileHalf = pTMgr.GetTile(tid, halfScale);
        tileHalf.BitBlt(pDC, CalcScrolledX(x), y);
        x += tileHalf.GetWidth() + 2 * tileBorder;
        y = rctItem.GetTop() + rctItem.GetHeight()/2 - 4;
        CTile tileSmall = pTMgr.GetTile(tid, smallScale);
        wxPen penSmall(CB::Convert(tileSmall.GetSmallColor()));
        wxDCPenChanger setPen(pDC, penSmall);
        wxBrush brSmall(CB::Convert(tileSmall.GetSmallColor()));
        wxDCBrushChanger setBrush(pDC, brSmall);
        wxRect rctSmall(x, y, 8, 8);
        pDC.DrawRectangle(rctSmall);     // Fill background color
    }
}

BOOL CTileListBoxWx::OnDragSetup(DragInfoWx& pDI) const
{
    if (IsMultiSelect())
    {
        pDI.SetDragType(DRAG_TILELIST);
        pDI.GetSubInfo<DRAG_TILELIST>().m_tileIDList = &GetMappedMultiSelectList();
        pDI.GetSubInfo<DRAG_TILELIST>().m_size = GetDragSize();
        pDI.GetSubInfo<DRAG_TILELIST>().m_gamDoc = m_pDoc;
        pDI.m_hcsrSuggest = g_res.hcrDragTileWx;
    }
    else
    {
        pDI.SetDragType(DRAG_TILE);
        pDI.GetSubInfo<DRAG_TILE>().m_tileID = GetCurMapItem();
        pDI.GetSubInfo<DRAG_TILE>().m_size = GetDragSize();
        pDI.GetSubInfo<DRAG_TILE>().m_gamDoc = m_pDoc;
        pDI.m_hcsrSuggest = g_res.hcrDragTileWx;
    }
    return TRUE;
}


