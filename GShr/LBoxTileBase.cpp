// LBoxTileBase.cpp - base class used to handle a variety of tile oriented
//      listbox functions
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
#include    "Resource.h"
#include    "ResTbl.h"

#include    "LBoxTileBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

const int tileBorder = 3;
const int tileGap = 6;

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTileBaseListBox, CGrafixListBox)
    //{{AFX_MSG_MAP(CTileBaseListBox)
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CTileBaseListBox::CTileBaseListBox()
{
    m_bDisplayIDs = AfxGetApp()->GetProfileInt("Settings"_cbstring, "DisplayIDs"_cbstring, 0);

    m_bTipMarkItems = TRUE;
    m_sizeTipMark = CSize(0,0);
}

int CTileBaseListBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int retval = CGrafixListBox::OnCreate(lpCreateStruct);
    if (retval == 0)
    {
        SetupTipMarkerIfRequired();
    }
    return retval;
}

/////////////////////////////////////////////////////////////////////////////

CSize CTileBaseListBox::DoOnItemSize(size_t nItem, const std::vector<TileID>& tids) const
{
    ASSERT(!tids.empty() &&
            tids[size_t(0)] != nullTid);        // At least one tile needs to exist

    // only using DC for measurement, so const_cast safe
    CClientDC pDC(const_cast<CTileBaseListBox*>(this));
    pDC.SaveDC();
    CRect rect(0, 0, 32000, 32000);

    int htTiles = 0;
    int wdTiles = 0;
    for (size_t i = size_t(0) ; i < tids.size() ; ++i)
    {
        CTile tile = GetTileManager().GetTile(tids[i], fullScale);
        htTiles = std::max(htTiles, tile.GetHeight());
        DrawTileImage(pDC, rect, FALSE, wdTiles, tids[i]);
    }

    // Listbox lines can only be 255 pixels high.
    LONG nHt = std::min(2 * tileBorder + htTiles, 255);

    int nWd = wdTiles;

    if (m_bDisplayIDs || m_bTipMarkItems)   // See if we're drawing debug ID's
    {
        nHt = std::max(nHt, g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading);
        BOOL bItemHasTipText = OnDoesItemHaveTipText(nItem);
        DrawTipMarker(pDC, rect, bItemHasTipText, nWd);
        DrawItemDebugIDCode(pDC, nItem, rect, false, nWd);
    }

    pDC.RestoreDC(-1);

    return CSize(nWd , nHt);
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBox::DoOnDrawItem(CDC& pDC, size_t nItem, UINT nAction, UINT nState, CRect rctItem,
    const std::vector<TileID>& tids) const
{
    if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
    {
        ASSERT(!tids.empty() &&
            tids[size_t(0)] != nullTid);

        BOOL bItemHasTipText = OnDoesItemHaveTipText(nItem);

        SetupPalette(pDC);

        pDC.SaveDC();
        pDC.IntersectClipRect(&rctItem);
        pDC.SetBkMode(TRANSPARENT);

        CBrush brBack(GetSysColor(nState & ODS_SELECTED ?
            COLOR_HIGHLIGHT : COLOR_WINDOW));
        pDC.FillRect(&rctItem, &brBack);       // Fill background color

        pDC.SetTextColor(GetSysColor(nState & ODS_SELECTED ?
            COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        int x = rctItem.left + tileBorder;

        DrawTipMarker(pDC, rctItem, bItemHasTipText, x);
        DrawItemDebugIDCode(pDC, nItem, rctItem, TRUE, x);
        for (size_t i = size_t(0) ; i < tids.size() ; ++i)
        {
            DrawTileImage(pDC, rctItem, TRUE, x, tids[i]);
        }

        pDC.RestoreDC(-1);
        ResetPalette(pDC);
    }
    if (nAction & ODA_FOCUS)
        pDC.DrawFocusRect(&rctItem);
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBox::DrawTileImage(CDC& pDC, CRect rctItem, BOOL bDrawIt, int& x, TileID tid) const
{
    if (tid == nullTid)
        return;                             // Nothing to do

    CTile tile = GetTileManager().GetTile(tid, fullScale);

    if (bDrawIt)
    {
        if (tile.GetHeight() >= 255)
            tile.BitBlt(pDC, x, rctItem.top + tileBorder);// Too large. Don't draw vertically centered
        else
            tile.BitBlt(pDC, x, (rctItem.Height() - tile.GetHeight()) / 2 + rctItem.top);
    }
    x += tile.GetWidth() + tileGap;
}

/////////////////////////////////////////////////////////////////////////////
// Optionally draw debug code string for item. If bDrawIt is false,
// x is advanced the size of the string anyway but nothing is rendered

void CTileBaseListBox::DrawItemDebugIDCode(CDC& pDC, size_t nItem, CRect rctItem, BOOL bDrawIt, int& x) const
{
    if (m_bDisplayIDs)
    {
        CB::string str = OnGetItemDebugString(nItem);

        CFont* prvFont = (CFont*)pDC.SelectObject(CFont::FromHandle(g_res.h8ss));
        int y = rctItem.top + rctItem.Height() / 2 -
            (g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading) / 2;
        if (bDrawIt)
            pDC.TextOut(x, y, str);
        x += pDC.GetTextExtent(str).cx;
        pDC.SelectObject(prvFont);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBox::SetupTipMarkerIfRequired()
{
    if (m_bTipMarkItems)
    {
        ASSERT(m_hWnd != NULL);
        if (m_sizeTipMark.cx == 0)
        {
            // Hasn't been initialized yet.
            m_strTipMark = CB::string::LoadString(IDS_TIP_LBOXITEM_MARKER);

            CDC* pDC = GetDC();

            CFont* prvFont = (CFont*)pDC->SelectObject(CFont::FromHandle(g_res.h8ss));
            m_sizeTipMark.cx = pDC->GetTextExtent(m_strTipMark).cx;
            m_sizeTipMark.cy = g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading;
            pDC->SelectObject(prvFont);

            ReleaseDC(pDC);
        }
    }
}

void CTileBaseListBox::DrawTipMarker(CDC& pDC, CRect rctItem, BOOL bVisible, int& x) const
{
    if (m_bTipMarkItems)
    {
        CFont* prvFont = (CFont*)pDC.SelectObject(CFont::FromHandle(g_res.h8ss));
        if (bVisible)   // Draw only if visible. Else just move 'x'
        {
            int y = rctItem.top + (rctItem.Height() - m_sizeTipMark.cy) / 2;
            pDC.TextOut(x, y, m_strTipMark);
        }
        x += m_sizeTipMark.cx;
        pDC.SelectObject(prvFont);
    }
}

CB::string CTileBaseListBox::OnGetItemDebugString(size_t nItem) const
{
    return std::format(L"[{}] ", OnGetItemDebugIDCode(nItem));
}

/////////////////////////////////////////////////////////////////////////////

std::vector<CRect> CTileBaseListBox::GetTileRectsForItem(size_t nItem, const std::vector<TileID>& tids) const
{
    ASSERT(!tids.empty() &&
        tids[size_t(0)] != nullTid);

    CRect rctItem;
    GetItemRect(value_preserving_cast<int>(nItem), &rctItem);

    int x = rctItem.left + tileBorder;          // Set starting x position

    // Need to account for possible markers and debug strings
    // rendered to left of tile images
    /* safe to use const_cast here because the DC isn't
        actually drawn on; it's just used for measuring text,
        so this window isn't being changed */
    CDC& pDC = CheckedDeref(const_cast<CTileBaseListBox*>(this)->GetDC());
    DrawTipMarker(pDC, rctItem, FALSE, x);
    DrawItemDebugIDCode(pDC, nItem, rctItem, FALSE, x);
    const_cast<CTileBaseListBox*>(this)->ReleaseDC(&pDC);

    std::vector<CRect> retval(tids.size());
    for (size_t i = size_t(0); i < tids.size(); ++i)
    {
        retval[i].top = rctItem.top;            // Set the top & bottom values
        retval[i].bottom = rctItem.bottom;
        retval[i].left = x;
        DrawTileImage(pDC, rctItem, FALSE, x, tids[i]);
        retval[i].right = x;
    }

    return retval;
}

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CTileBaseListBoxWx, CGrafixListBoxWx)
    EVT_WINDOW_CREATE(OnCreate)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////

CTileBaseListBoxWx::CTileBaseListBoxWx()
{
    ASSERT(!GetHandle());
    m_bDisplayIDs = AfxGetApp()->GetProfileInt("Settings"_cbstring, "DisplayIDs"_cbstring, 0);

    m_bTipMarkItems = TRUE;
    m_sizeTipMark = wxSize(0, 0);
}

void CTileBaseListBoxWx::OnCreate(wxWindowCreateEvent& event)
{
    SetupTipMarkerIfRequired();
}

/////////////////////////////////////////////////////////////////////////////

wxSize CTileBaseListBoxWx::DoOnItemSize(size_t nItem, const std::vector<TileID>& tids) const
{
    ASSERT(!tids.empty() &&
            tids[size_t(0)] != nullTid);        // At least one tile needs to exist

    // only using DC for measurement, so const_cast safe
    wxWindowDC pDC(const_cast<CTileBaseListBoxWx*>(this));
    wxRect rect(0, 0, 32000, 32000);

    int htTiles = 0;
    int wdTiles = 0;
    for (size_t i = size_t(0) ; i < tids.size() ; ++i)
    {
        CTile tile = GetTileManager().GetTile(tids[i], fullScale);
        htTiles = CB::max(htTiles, tile.GetHeight());
        DrawTileImage(pDC, rect, FALSE, wdTiles, tids[i]);
    }

    LONG nHt = 2 * tileBorder + htTiles;

    int nWd = wdTiles;

    if (m_bDisplayIDs || m_bTipMarkItems)   // See if we're drawing debug ID's
    {
        nHt = CB::max(nHt, g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading);
        BOOL bItemHasTipText = OnDoesItemHaveTipText(nItem);
        DrawTipMarker(pDC, rect, bItemHasTipText, nWd);
        DrawItemDebugIDCode(pDC, nItem, rect, false, nWd);
    }

    return wxSize(nWd , nHt);
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBoxWx::DoOnDrawItem(wxDC& pDC, size_t nItem, wxRect rctItem,
    const std::vector<TileID>& tids) const
{
    ASSERT(!tids.empty() &&
        tids[size_t(0)] != nullTid);

    BOOL bItemHasTipText = OnDoesItemHaveTipText(nItem);

    pDC.SetClippingRegion(rctItem);

    pDC.SetTextForeground(wxSystemSettings::GetColour(IsSelected(nItem) ?
        wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT));

    wxCoord x = rctItem.GetLeft() + tileBorder;

    DrawTipMarker(pDC, rctItem, bItemHasTipText, x);
    DrawItemDebugIDCode(pDC, nItem, rctItem, TRUE, x);
    for (size_t i = size_t(0) ; i < tids.size() ; ++i)
    {
        DrawTileImage(pDC, rctItem, TRUE, x, tids[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBoxWx::DrawTileImage(wxDC& pDC, wxRect rctItem, BOOL bDrawIt, wxCoord& x, TileID tid) const
{
    if (tid == nullTid)
        return;                             // Nothing to do

    CTile tile = GetTileManager().GetTile(tid, fullScale);

    if (bDrawIt)
    {
        tile.BitBlt(pDC, CalcScrolledX(x), (rctItem.GetHeight() - tile.GetHeight()) / 2 + rctItem.GetTop());
    }
    x += tile.GetWidth() + tileGap;
}

/////////////////////////////////////////////////////////////////////////////
// Optionally draw debug code string for item. If bDrawIt is false,
// x is advanced the size of the string anyway but nothing is rendered

void CTileBaseListBoxWx::DrawItemDebugIDCode(wxDC& pDC, size_t nItem, wxRect rctItem, BOOL bDrawIt, wxCoord& x) const
{
    if (m_bDisplayIDs)
    {
        CB::string str = OnGetItemDebugString(nItem);

        pDC.SetFont(g_res.h8ssWx);
        wxCoord y = rctItem.GetTop() + rctItem.GetHeight() / 2 -
            (g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading) / 2;
        if (bDrawIt)
            pDC.DrawText(str, CalcScrolledX(x), y);
        x += pDC.GetTextExtent(str).x;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBoxWx::SetupTipMarkerIfRequired()
{
    if (m_bTipMarkItems)
    {
        ASSERT(GetHandle());
        if (m_sizeTipMark.x == 0)
        {
            // Hasn't been initialized yet.
            m_strTipMark = CB::string::LoadString(IDS_TIP_LBOXITEM_MARKER);

            wxWindowDC pDC(this);

            pDC.SetFont(g_res.h8ssWx);
            m_sizeTipMark.x = pDC.GetTextExtent(m_strTipMark).x;
            m_sizeTipMark.y = g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading;
        }
    }
}

void CTileBaseListBoxWx::DrawTipMarker(wxDC& pDC, wxRect rctItem, BOOL bVisible, wxCoord& x) const
{
    if (m_bTipMarkItems)
    {
        pDC.SetFont(g_res.h8ssWx);
        if (bVisible)   // Draw only if visible. Else just move 'x'
        {
            wxCoord y = rctItem.GetTop() + (rctItem.GetHeight() - m_sizeTipMark.y) / 2;
            pDC.DrawText(m_strTipMark, CalcScrolledX(x), y);
        }
        x += m_sizeTipMark.x;
    }
}

CB::string CTileBaseListBoxWx::OnGetItemDebugString(size_t nItem) const
{
    return std::format(L"[{}] ", OnGetItemDebugIDCode(nItem));
}

/////////////////////////////////////////////////////////////////////////////

#if 0
std::vector<CRect> CTileBaseListBoxWx::GetTileRectsForItem(size_t nItem, const std::vector<TileID>& tids) const
{
    ASSERT(!tids.empty() &&
        tids[size_t(0)] != nullTid);

    CRect rctItem;
    GetItemRect(value_preserving_cast<int>(nItem), &rctItem);

    int x = rctItem.left + tileBorder;          // Set starting x position

    // Need to account for possible markers and debug strings
    // rendered to left of tile images
    /* safe to use const_cast here because the DC isn't
        actually drawn on; it's just used for measuring text,
        so this window isn't being changed */
    CDC& pDC = CheckedDeref(const_cast<CTileBaseListBoxWx*>(this)->GetDC());
    DrawTipMarker(pDC, rctItem, FALSE, x);
    DrawItemDebugIDCode(pDC, nItem, rctItem, FALSE, x);
    const_cast<CTileBaseListBoxWx*>(this)->ReleaseDC(&pDC);

    std::vector<CRect> retval(tids.size());
    for (size_t i = size_t(0); i < tids.size(); ++i)
    {
        retval[i].top = rctItem.top;            // Set the top & bottom values
        retval[i].bottom = rctItem.bottom;
        retval[i].left = x;
        DrawTileImage(pDC, rctItem, FALSE, x, tids[i]);
        retval[i].right = x;
    }

    return retval;
}
#endif


