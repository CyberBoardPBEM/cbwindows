// LBoxTileBase2.cpp - base class used to handle a variety of tile oriented
//      listbox functions.
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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

BEGIN_MESSAGE_MAP(CTileBaseListBox2, CGrafixListBox2)
    //{{AFX_MSG_MAP(CTileBaseListBox2)
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CTileBaseListBox2::CTileBaseListBox2()
{
    m_bDisplayIDs = AfxGetApp()->GetProfileInt("Settings"_cbstring, "DisplayIDs"_cbstring, 0);

    m_bTipMarkItems = TRUE;
    m_sizeTipMark = CSize(0,0);
}

/////////////////////////////////////////////////////////////////////////////

CSize CTileBaseListBox2::DoOnItemSize(size_t nItem, const std::vector<TileID>& tids) const
{
    ASSERT(!tids.empty() && tids[size_t(0)] != nullTid);        // At least one tile needs to exist

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
    CClientDC pDC(const_cast<CTileBaseListBox2*>(this));
    CRect rctItem(0, 0, 32000, 32000);

    pDC.SaveDC();

    int x = rctItem.left + tileBorder;

    DrawTipMarker(pDC, rctItem, bItemHasTipText, x);
    DrawItemDebugIDCode(pDC, nItem, rctItem, false, x);
    for (size_t i = size_t(0); i < tids.size(); ++i)
    {
        DrawTileImage(pDC, rctItem, false, x, tids[i]);
    }

    pDC.RestoreDC(-1);

    return CSize(x, nHt);
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBox2::DoOnDrawItem(CDC& pDC, size_t nItem, UINT nAction, UINT nState,
    CRect rctItem, const std::vector<TileID>& tids) const
{
    if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
    {
        ASSERT(!tids.empty() && tids[size_t(0)] != nullTid);

        BOOL bItemHasTipText = OnDoesItemHaveTipText(nItem);

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
        for (size_t i = size_t(0); i < tids.size(); ++i)
        {
            DrawTileImage(pDC, rctItem, true, x, tids[i]);
        }

        pDC.RestoreDC(-1);
    }
    if (nAction & ODA_FOCUS)
        pDC.DrawFocusRect(&rctItem);
}

/////////////////////////////////////////////////////////////////////////////

void CTileBaseListBox2::DrawTileImage(CDC& pDC, CRect rctItem, BOOL bDrawIt, int& x, TileID tid) const
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

void CTileBaseListBox2::DrawItemDebugIDCode(CDC& pDC, size_t nItem, CRect rctItem, BOOL bDrawIt, int& x) const
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

void CTileBaseListBox2::SetupTipMarkerIfRequired()
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

void CTileBaseListBox2::DrawTipMarker(CDC& pDC, CRect rctItem, BOOL bVisible, int& x) const
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

CB::string CTileBaseListBox2::OnGetItemDebugString(size_t nItem) const
{
    return std::format(L"[{}] ", OnGetItemDebugIDCode(nItem));
}

/////////////////////////////////////////////////////////////////////////////

std::vector<CRect> CTileBaseListBox2::GetTileRectsForItem(size_t nItem,
    const std::vector<TileID>& tids) const
{
    ASSERT(!tids.empty() && tids[size_t(0)] != nullTid);

    CRect rctItem;
    GetItemRect(value_preserving_cast<int>(nItem), &rctItem);

    int x = rctItem.left + tileBorder;          // Set starting x position

    // Need to account for possible markers and debug strings
    // rendered to left of tile images
    // only using DC for measurement, so const_cast safe
    CDC& pDC = CheckedDeref(const_cast<CTileBaseListBox2*>(this)->GetDC());
    DrawTipMarker(pDC, rctItem, FALSE, x);
    DrawItemDebugIDCode(pDC, nItem, rctItem, FALSE, x);
    const_cast<CTileBaseListBox2*>(this)->ReleaseDC(&pDC);

    std::vector<CRect> retval(tids.size());
    for (size_t i = size_t(0) ; i < tids.size() ; ++i)
    {
        retval[i].SetRectEmpty();
        retval[i].top = rctItem.top;                  // Set the top & bottom values
        retval[i].bottom = rctItem.bottom;
        retval[i].left = x;
        DrawTileImage(pDC, rctItem, FALSE, x, tids[i]);
        retval[i].right = x;
    }
    return retval;
}

int CTileBaseListBox2::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int retval = CGrafixListBox2::OnCreate(lpCreateStruct);
    if (retval == 0)
    {
        SetupTipMarkerIfRequired();
    }
    return retval;
}


