// LBoxMark.cpp
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
#ifdef GPLAY
#include    "GamDoc.h"
#else
#include    "GmDoc.h"
#endif
#include    "ResTbl.h"
#include    "Marks.h"
#include    "LBoxMark.h"

// KLUDGE:  macro parser can't handle the , in the template args
namespace {
    typedef CGrafixListBoxDataWx<CTileBaseListBoxWx, MarkID> CMarkListBoxWxBase;
}
wxIMPLEMENT_DYNAMIC_CLASS(CMarkListBoxWx, CMarkListBoxWxBase);

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

const CTileManager& CMarkListBox::GetTileManager() const
{
    ASSERT(m_pDoc != NULL);
    return m_pDoc->GetTileManager();
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing

BOOL CMarkListBox::OnIsToolTipsEnabled() const
{
#ifdef GPLAY
    return m_pDoc->IsShowingObjectTips();
#else           // Always show them in the designer
    return TRUE;
#endif
}

GameElement CMarkListBox::OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) const
{
    point = ClientToItem(point);

    BOOL bOutsideClient;
    UINT nIndex = ItemFromPoint(point, bOutsideClient);
    if (nIndex >= 65535 || GetCount() <= 0)
        return Invalid_v<GameElement>;

    CMarkManager& pMMgr = m_pDoc->GetMarkManager();

    MarkID mid = MapIndexToItem(value_preserving_cast<size_t>(nIndex));
    MarkDef& pMark = pMMgr.GetMark(mid);
    std::vector<TileID> tids;
    tids.push_back(pMark.m_tid);

    std::vector<CRect> rects = GetTileRectsForItem(nIndex, tids);
    ASSERT(rects.size() == size_t(1));
    rct = rects[size_t(0)];

    if (rct.PtInRect(point))
    {
        rct = ItemToClient(rct);
        return GameElement(mid);
    }
    else
    {
        return Invalid_v<GameElement>;
    }
}

void CMarkListBox::OnGetTipTextForItemCode(GameElement nItemCode,
    CB::string& strTip) const
{
    MarkID mid = static_cast<MarkID>(nItemCode);
    strTip = m_pDoc->GetGameElementString(MakeMarkerElement(mid));
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMarkListBox::OnDoesItemHaveTipText(size_t nItem) const
{
    ASSERT(m_eTrayViz == mtrayVizNormal);
    MarkID mid = MapIndexToItem(nItem);
    return m_pDoc->HasGameElementString(MakeMarkerElement(mid));
}

/////////////////////////////////////////////////////////////////////////////

void CMarkListBox::SelectMarker(MarkID mid)
{
    size_t nIndex = MapItemToIndex(mid);
    // NO LONGER IMPOSSIBLE SINCE HIDE ALL MARKERS. ASSERT(nIndex != -1);
    if (nIndex == Invalid_v<size_t>)
    {
        if (GetCount() >= 1)
            SetCurSel(0);           // Just select the first one.
    }
    else
    {
        ShowListIndex(value_preserving_cast<int>(nIndex));
        SetCurSel(value_preserving_cast<int>(nIndex));
    }
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

CSize CMarkListBox::OnItemSize(size_t nIndex) const
{
    if (m_eTrayViz == mtrayVizNormal)
    {
        CMarkManager& pMMgr = m_pDoc->GetMarkManager();
        MarkDef& pMark = pMMgr.GetMark(MapIndexToItem(nIndex));
        ASSERT(pMark.m_tid != nullTid);

        std::vector<TileID> tids;
        tids.push_back(pMark.m_tid);

        return DoOnItemSize(nIndex, tids);
    }
    else
    {
        // Hidden markers. Account for drawing the supplied text.
        ASSERT(!m_strHiddenString.empty());
        // only using DC to measure text, so const_cast safe;
        CClientDC pDC(const_cast<CMarkListBox*>(this));
        pDC.SaveDC();
        CFont* prvFont = (CFont*)pDC.SelectObject(CFont::FromHandle(g_res.h8ss));
        CSize extent = pDC.GetTextExtent(m_strHiddenString);
        pDC.SelectObject(prvFont);
        pDC.RestoreDC(-1);
        return extent;
    }
}

void CMarkListBox::OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
    CRect rctItem) const
{
    // see https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-drawitemstruct
    if (nIndex == size_t(UINT(-1)))
        return;

    if (m_eTrayViz == mtrayVizNormal)
    {
        if (m_pDoc == NULL)
            return;

        CMarkManager& pMMgr = m_pDoc->GetMarkManager();

        MarkDef& pMark = pMMgr.GetMark(MapIndexToItem(nIndex));
        ASSERT(pMark.m_tid != nullTid);

        std::vector<TileID> tids;
        tids.push_back(pMark.m_tid);
        DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tids);
    }
    else
    {
        if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
        {
            // Hidden markers. Draw the supplied text.
            pDC.SetTextAlign(TA_TOP | TA_LEFT);
            CBrush brBack(GetSysColor(nState & ODS_SELECTED ?
                COLOR_HIGHLIGHT : COLOR_WINDOW));
            pDC.FillRect(&rctItem, &brBack);       // Fill background color
            pDC.SetBkMode(TRANSPARENT);
            CFont* pPrvFont = pDC.SelectObject(CFont::FromHandle(g_res.h8ss));
            pDC.SetTextColor(GetSysColor(nState & ODS_SELECTED ?
                COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
            pDC.TextOut(rctItem.left, rctItem.top, m_strHiddenString);
            pDC.SelectObject(pPrvFont);
        }
        if (nAction & ODA_FOCUS)
            pDC.DrawFocusRect(&rctItem);
    }
}

BOOL CMarkListBox::OnDragSetup(DragInfo& pDI) const
{
#ifdef GPLAY
    if (m_pDoc->IsPlaying())
    {
        pDI.SetDragType(DRAG_INVALID);
        return 0;                       // Drags not supported during play
    }
#endif
    pDI.SetDragType(DRAG_MARKER);
    pDI.GetSubInfo<DRAG_MARKER>().m_markID = GetCurMapItem();
    pDI.GetSubInfo<DRAG_MARKER>().m_size = GetDragSize();
    pDI.GetSubInfo<DRAG_MARKER>().m_gamDoc = m_pDoc;
    pDI.m_hcsrSuggest = g_res.hcrDragTile;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CMarkListBoxWx::CMarkListBoxWx()
{
    m_pDoc = NULL;
    m_eTrayViz = mtrayVizNormal;
}

/////////////////////////////////////////////////////////////////////////////

const CTileManager& CMarkListBoxWx::GetTileManager() const
{
    ASSERT(m_pDoc != NULL);
    return m_pDoc->GetTileManager();
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing

BOOL CMarkListBoxWx::OnIsToolTipsEnabled() const
{
#ifdef GPLAY
    return m_pDoc->IsShowingObjectTips();
#else           // Always show them in the designer
    return TRUE;
#endif
}

GameElement CMarkListBoxWx::OnGetHitItemCodeAtPoint(wxPoint point, wxRect& rct) const
{
    point = ClientToItem(point);

    int nIndex = VirtualHitTest(point.y);
    if (nIndex == wxNOT_FOUND)
    {
        return Invalid_v<GameElement>;
    }

    CMarkManager& pMMgr = m_pDoc->GetMarkManager();

    MarkID mid = MapIndexToItem(value_preserving_cast<size_t>(nIndex));
    MarkDef& pMark = pMMgr.GetMark(mid);
    std::vector<TileID> tids;
    tids.push_back(pMark.m_tid);

    std::vector<wxRect> rects = GetTileRectsForItem(value_preserving_cast<size_t>(nIndex), tids);
    wxASSERT(rects.size() == size_t(1));
    rct = rects[size_t(0)];

    if (rct.Contains(point))
    {
        rct = ItemToClient(rct);
        return GameElement(mid);
    }
    else
    {
        return Invalid_v<GameElement>;
    }
}

void CMarkListBoxWx::OnGetTipTextForItemCode(GameElement nItemCode,
    CB::string& strTip) const
{
    MarkID mid = static_cast<MarkID>(nItemCode);
    strTip = m_pDoc->GetGameElementString(MakeMarkerElement(mid));
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMarkListBoxWx::OnDoesItemHaveTipText(size_t nItem) const
{
    wxASSERT(m_eTrayViz == mtrayVizNormal);
    MarkID mid = MapIndexToItem(nItem);
    return m_pDoc->HasGameElementString(MakeMarkerElement(mid));
}

/////////////////////////////////////////////////////////////////////////////

#if 0
void CMarkListBox::SelectMarker(MarkID mid)
{
    size_t nIndex = MapItemToIndex(mid);
    // NO LONGER IMPOSSIBLE SINCE HIDE ALL MARKERS. ASSERT(nIndex != -1);
    if (nIndex == Invalid_v<size_t>)
    {
        if (GetCount() >= 1)
            SetCurSel(0);           // Just select the first one.
    }
    else
    {
        ShowListIndex(value_preserving_cast<int>(nIndex));
        SetCurSel(value_preserving_cast<int>(nIndex));
    }
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
#endif

/////////////////////////////////////////////////////////////////////////////

wxSize CMarkListBoxWx::GetItemSize(size_t nIndex) const
{
    if (m_eTrayViz == mtrayVizNormal)
    {
        CMarkManager& pMMgr = m_pDoc->GetMarkManager();
        MarkDef& pMark = pMMgr.GetMark(MapIndexToItem(nIndex));
        ASSERT(pMark.m_tid != nullTid);

        std::vector<TileID> tids;
        tids.push_back(pMark.m_tid);

        return DoOnItemSize(nIndex, tids);
    }
    else
    {
        // Hidden markers. Account for drawing the supplied text.
        wxASSERT(!m_strHiddenString.empty());
        // only using DC to measure text, so const_cast safe;
        wxWindowDC pDC(const_cast<CMarkListBoxWx*>(this));
        pDC.SetFont(g_res.h8ssWx);
        wxSize extent = pDC.GetTextExtent(m_strHiddenString);
        return extent;
    }
}

void CMarkListBoxWx::OnDrawItem(wxDC& pDC, const wxRect& rctItem, size_t nIndex) const
{
    if (m_eTrayViz == mtrayVizNormal)
    {
        if (m_pDoc == NULL)
            return;

        CMarkManager& pMMgr = m_pDoc->GetMarkManager();

        MarkDef& pMark = pMMgr.GetMark(MapIndexToItem(nIndex));
        ASSERT(pMark.m_tid != nullTid);

        std::vector<TileID> tids;
        tids.push_back(pMark.m_tid);
        DoOnDrawItem(pDC, nIndex, rctItem, tids);
    }
    else
    {
        // Hidden markers. Draw the supplied text.
        bool selected = IsSelected(nIndex);
        pDC.SetFont(g_res.h8ssWx);
        pDC.SetTextForeground(wxSystemSettings::GetColour(selected ?
            wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT));
        pDC.DrawText(m_strHiddenString, CalcScrolledPosition(rctItem.GetTopLeft()));
    }
}

#if 0
BOOL CMarkListBox::OnDragSetup(DragInfo& pDI) const
{
#ifdef GPLAY
    if (m_pDoc->IsPlaying())
    {
        pDI.SetDragType(DRAG_INVALID);
        return 0;                       // Drags not supported during play
    }
#endif
    pDI.SetDragType(DRAG_MARKER);
    pDI.GetSubInfo<DRAG_MARKER>().m_markID = GetCurMapItem();
    pDI.GetSubInfo<DRAG_MARKER>().m_size = GetDragSize();
    pDI.GetSubInfo<DRAG_MARKER>().m_gamDoc = m_pDoc;
    pDI.m_hcsrSuggest = g_res.hcrDragTile;
    return TRUE;
}
#endif

