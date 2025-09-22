// LBoxTileBase2.cpp - base class used to handle a variety of tile oriented
//      listbox functions.
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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

#include    "LBoxTileBase2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

const int tileBorder = 3;
const int tileGap = 6;

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CTileBaseListBox2, CGrafixListBox2)
    EVT_WINDOW_CREATE(OnCreate)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////

CTileBaseListBox2::CTileBaseListBox2()
{
    m_bDisplayIDs = AfxGetApp()->GetProfileInt("Settings"_cbstring, "DisplayIDs"_cbstring, 0);

    m_bTipMarkItems = TRUE;
    m_sizeTipMark = wxSize(0,0);
}

/////////////////////////////////////////////////////////////////////////////

wxSize CTileBaseListBox2::DoOnItemSize(size_t nItem, const std::vector<TileID>& tids) const
{
    wxASSERT(!tids.empty() && tids[size_t(0)] != nullTid);        // At least one tile needs to exist

    int nHt = 0;
    for (size_t i = size_t(0) ; i < tids.size() ; ++i)
    {
        CTile tile = GetTileManager().GetTile(tids[i], fullScale);
        nHt = std::max(nHt, tile.GetHeight());
    }

    // Listbox lines can only be 255 pixels high.
    nHt = std::min(2 * tileBorder + nHt, 255);

    if (m_bDisplayIDs || m_bTipMarkItems)   // See if we're drawing debug ID's
        nHt = std::max(nHt, value_preserving_cast<int>(g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading));

    BOOL bItemHasTipText = OnDoesItemHaveTipText(nItem);

    // only using DC for measurement, so const_cast safe
    wxWindowDC pDC(const_cast<CTileBaseListBox2*>(this));
    wxRect rctItem(0, 0, 32000, 32000);

    int x = rctItem.GetLeft() + tileBorder;

    DrawTipMarker(pDC, rctItem, bItemHasTipText, x);
    DrawItemDebugIDCode(pDC, nItem, rctItem, false, x);
    for (size_t i = size_t(0); i < tids.size(); ++i)
    {
        DrawTileImage(pDC, rctItem, false, x, tids[i]);
    }

    return wxSize(x, nHt);
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBox2::DoOnDrawItem(wxDC& pDC, size_t nItem,
    wxRect rctItem, const std::vector<TileID>& tids) const
{
    wxASSERT(!tids.empty() && tids[size_t(0)] != nullTid);

    BOOL bItemHasTipText = OnDoesItemHaveTipText(nItem);

    pDC.SetClippingRegion(rctItem);

    pDC.SetTextForeground(wxSystemSettings::GetColour(IsSelected(nItem) ?
        wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT));

    wxCoord x = rctItem.GetLeft() + tileBorder;

    DrawTipMarker(pDC, rctItem, bItemHasTipText, x);
    DrawItemDebugIDCode(pDC, nItem, rctItem, TRUE, x);
    for (size_t i = size_t(0); i < tids.size(); ++i)
    {
        DrawTileImage(pDC, rctItem, true, x, tids[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBox2::DrawTileImage(wxDC& pDC, wxRect rctItem, BOOL bDrawIt, wxCoord& x, TileID tid) const
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

void CTileBaseListBox2::DrawItemDebugIDCode(wxDC& pDC, size_t nItem, wxRect rctItem, BOOL bDrawIt, wxCoord& x) const
{
    if (m_bDisplayIDs)
    {
        CB::string str = OnGetItemDebugString(nItem);

        pDC.SetFont(g_res.h8ssWx);
        wxCoord y = rctItem.GetTop() + rctItem.GetHeight() / 2 -
            (g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading) / 2;
        if (bDrawIt)
        {
            pDC.DrawText(str, CalcScrolledX(x), y);
        }
        x += pDC.GetTextExtent(str).x;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBox2::SetupTipMarkerIfRequired()
{
    if (m_bTipMarkItems)
    {
        wxASSERT(GetHandle());
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

void CTileBaseListBox2::DrawTipMarker(wxDC& pDC, wxRect rctItem, BOOL bVisible, int& x) const
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

CB::string CTileBaseListBox2::OnGetItemDebugString(size_t nItem) const
{
    return std::format(L"[{}] ", OnGetItemDebugIDCode(nItem));
}

/////////////////////////////////////////////////////////////////////////////

std::vector<wxRect> CTileBaseListBox2::GetTileRectsForItem(size_t nItem,
    const std::vector<TileID>& tids) const
{
    wxASSERT(!tids.empty() && tids[size_t(0)] != nullTid);

    wxRect rctItem = GetItemRect(nItem);

    wxCoord x = CalcScrolledX(rctItem.GetLeft()) + tileBorder;          // Set starting x position

    // Need to account for possible markers and debug strings
    // rendered to left of tile images
    // only using DC for measurement, so const_cast safe
    wxWindowDC pDC(const_cast<CTileBaseListBox2*>(this));
    DrawTipMarker(pDC, rctItem, FALSE, x);
    DrawItemDebugIDCode(pDC, nItem, rctItem, FALSE, x);

    std::vector<wxRect> retval(tids.size());
    for (size_t i = size_t(0) ; i < tids.size() ; ++i)
    {
        retval[i].SetTop(rctItem.GetTop());                  // Set the top & bottom values
        retval[i].SetBottom(rctItem.GetBottom());
        retval[i].SetLeft(x);
        DrawTileImage(pDC, rctItem, FALSE, x, tids[i]);
        retval[i].SetRight(x);
    }
    return retval;
}

void CTileBaseListBox2::OnCreate(wxWindowCreateEvent& event)
{
    event.Skip();
    SetupTipMarkerIfRequired();
}


