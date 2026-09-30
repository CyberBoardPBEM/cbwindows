// LBoxTile.cpp
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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
wxIMPLEMENT_DYNAMIC_CLASS(CTileListBox, CB::VListBoxHScroll);

/////////////////////////////////////////////////////////////////////////////

const int tileBorder = 3;

/////////////////////////////////////////////////////////////////////////////

CTileListBox::CTileListBox()
{
    m_pDoc = NULL;
    m_bDrawAllScales = FALSE;
    m_bDisplayIDs = AfxGetApp()->GetProfileInt("Settings"_cbstring, "DisplayIDs"_cbstring, 0);
}

wxSize CTileListBox::GetItemSize(size_t nIndex) const
{
    wxASSERT(m_pDoc != NULL);
    const CTileManager& pTMgr = m_pDoc->GetTileManager();

    TileID tid = MapIndexToItem(nIndex);
    CTile tile = pTMgr.GetTile(tid, fullScale);

    int nHt = 2 * tileBorder + value_preserving_cast<int>(tile.GetHeight());
    if (m_bDisplayIDs)          // See if we're drawing PieceIDs
        nHt = CB::max(nHt, Get_tm8ssHeight() + Get_tm8ssExternalLeading());

    int x = tileBorder;
    if (m_bDisplayIDs)
    {
        CB::string str = std::format("[{}] ", MapIndexToItem(nIndex));
        // only using DC to measure text, so const_cast safe;
        wxWindowDC pDC(const_cast<CTileListBox*>(this));
        pDC.SetFont(g_res.h8ssWx);
        x += pDC.GetTextExtent(str).x;
    }

    x += value_preserving_cast<int>(tile.GetWidth()) + tileBorder;

    if (m_bDrawAllScales)
    {
        CTile tileHalf = pTMgr.GetTile(tid, halfScale);
        x += value_preserving_cast<int>(tileHalf.GetWidth()) + 2 * tileBorder;
        x += 8;
    }

    return wxSize(x, nHt);
}

void CTileListBox::OnDrawItem(wxDC& pDC, const wxRect& rctItem, size_t nIndex) const
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
            (Get_tm8ssHeight() + Get_tm8ssExternalLeading()) / 2;
        pDC.DrawText(str, CalcScrolledX(x), y);
        x += pDC.GetTextExtent(str).x;
        pDC.SetBackgroundMode(prevBkMode);
    }

    tile.BitBlt(pDC, CalcScrolledX(x), y);

    if (m_bDrawAllScales)
    {
        x += value_preserving_cast<int>(tile.GetWidth()) + 2 * tileBorder;
        y += value_preserving_cast<int>(tile.GetHeight()) / 4;
        CTile tileHalf = pTMgr.GetTile(tid, halfScale);
        tileHalf.BitBlt(pDC, CalcScrolledX(x), y);
        x += value_preserving_cast<int>(tileHalf.GetWidth()) + 2 * tileBorder;
        y = rctItem.GetTop() + rctItem.GetHeight()/2 - 4;
        CTile tileSmall = pTMgr.GetTile(tid, smallScale);
        wxPen penSmall(tileSmall.GetSmallColor());
        wxDCPenChanger setPen(pDC, penSmall);
        wxBrush brSmall(tileSmall.GetSmallColor());
        wxDCBrushChanger setBrush(pDC, brSmall);
        wxRect rctSmall(x, y, 8, 8);
        pDC.DrawRectangle(rctSmall);     // Fill background color
    }
}

BOOL CTileListBox::OnDragSetup(DragInfoWx& pDI) const
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

wxCoord CTileListBox::Get_tm8ssHeight() const
{
    /* KLUDGE:  wxInfoDC should not modify wxWindow,
        so const_cast<> should be safe */
    wxInfoDC dc(const_cast<CTileListBox*>(this));
    dc.SetFont(g_res.h8ssWx);
    return dc.GetCharHeight();
}

wxCoord CTileListBox::Get_tm8ssExternalLeading() const
{
    /* KLUDGE:  wxInfoDC should not modify wxWindow,
        so const_cast<> should be safe */
    wxInfoDC dc(const_cast<CTileListBox*>(this));
    dc.SetFont(g_res.h8ssWx);
    return dc.GetFontMetrics().externalLeading;
}


