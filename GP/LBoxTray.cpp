// LBoxTray.cpp
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
#include    "Gp.h"
#include    "GamDoc.h"
#include    "ResTbl.h"
#include    "PPieces.h"
#include    "Trays.h"
#include    "LBoxTray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

const int tileBorder = 3;
const int tileGap = 6;

/////////////////////////////////////////////////////////////////////////////

// KLUDGE:  macro parser can't handle the , in the template args
namespace {
    typedef CGrafixListBoxData<CTileBaseListBox, PieceID> CTrayListBoxBase;
}
wxIMPLEMENT_DYNAMIC_CLASS(CTrayListBox, CTrayListBoxBase);

/////////////////////////////////////////////////////////////////////////////

CTrayListBox::CTrayListBox() :
    m_pDoc(nullptr)
{
    m_eTrayViz = trayVizOneSide;
    m_bAllowTips = TRUE;
}

CTrayListBox::CTrayListBox(CGamDoc& pDoc) :
    CTrayListBox()
{
    Init(pDoc);
}

/////////////////////////////////////////////////////////////////////////////

const CTileManager& CTrayListBox::GetTileManager() const
{
    return m_pDoc->GetTileManager();
}

BOOL CTrayListBox::IsShowingTileImages() const
{
    return m_eTrayViz == trayVizAllSides || m_eTrayViz == trayVizOneSide;
}

void CTrayListBox::SetTrayContentVisibility(TrayViz eTrayViz, CB::string pszHiddenString)
{
    m_eTrayViz = eTrayViz;
    m_strHiddenString = std::move(pszHiddenString);
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing

BOOL CTrayListBox::OnIsToolTipsEnabled() const
{
    if (m_eTrayViz != trayVizAllSides && m_eTrayViz != trayVizOneSide)
        return FALSE;
    return m_pDoc->IsShowingObjectTips() && m_bAllowTips;
}

GameElement CTrayListBox::OnGetHitItemCodeAtPoint(wxPoint point, wxRect& rct) const
{
    point = ClientToItem(point);

    int nIndex = VirtualHitTest(point.y);
    if (nIndex == wxNOT_FOUND)
    {
        return Invalid_v<GameElement>;
    }

    wxASSERT(m_eTrayViz == trayVizAllSides || m_eTrayViz == trayVizOneSide);

    const CPieceTable& pPTbl = m_pDoc->GetPieceTable();

    PieceID nPid = MapIndexToItem(value_preserving_cast<size_t>(nIndex));

    TileID tidLeft = pPTbl.GetActiveTileID(nPid);
    wxASSERT(tidLeft != nullTid);            // Should exist

    std::vector<TileID> tids;
    tids.push_back(tidLeft);                // Initially assume no second tile image

    if (IsShowAllSides(nPid))
    {
        std::vector<TileID> inactives = pPTbl.GetInactiveTileIDs(nPid);
        tids.insert(tids.end(), inactives.begin(), inactives.end());
    }

    std::vector<wxRect> rects = GetTileRectsForItem(value_preserving_cast<size_t>(nIndex), tids);

    for (size_t i = size_t(0) ; i < rects.size() ; ++i)
    {
        wxASSERT(!rects[i].IsEmpty());
        if (!rects[i].IsEmpty() && rects[i].Contains(point))
        {
            rct = ItemToClient(rects[i]);
            const CPieceTable& pieceTbl = m_pDoc->GetPieceTable();
            uint8_t side = pieceTbl.GetSide(nPid, i);
            return GameElement(nPid, side);
        }
    }

    return Invalid_v<GameElement>;
}

void CTrayListBox::OnGetTipTextForItemCode(GameElement nItemCode,
    CB::string& strTip) const
{
    if (nItemCode == Invalid_v<GameElement>)
        return;
    strTip = m_pDoc->GetGameElementString(nItemCode);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CTrayListBox::OnDoesItemHaveTipText(size_t nItem) const
{
    wxASSERT(m_eTrayViz == trayVizAllSides || m_eTrayViz == trayVizOneSide);

    PieceID pid = MapIndexToItem(nItem);
    if (m_eTrayViz != trayVizAllSides)
    {
        uint8_t side = m_pDoc->GetPieceTable().GetSide(pid);
        return m_pDoc->HasGameElementString(MakePieceElement(pid, side));
    }
    else
    {
        size_t sides = m_pDoc->GetPieceTable().GetSides(pid);
        for (unsigned i = unsigned(0) ; i < sides ; ++i)
        {
            if (m_pDoc->HasGameElementString(MakePieceElement(pid, i)))
            {
                return true;
            }
        }
        return false;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTrayListBox::DeselectAll()
{
    wxASSERT(IsMultiSelect());
    if (GetItemCount() < 1)
        return;
    BASE::DeselectAll();
}

size_t CTrayListBox::SelectTrayPiece(PieceID pid)
{
    size_t nIndex = MapItemToIndex(pid);
    if (nIndex != Invalid_v<size_t>)
    {
        ShowListIndex(nIndex);
        Select(nIndex, TRUE);
    }
    else
    {
        if (GetItemCount() > 0)
            Select(0, TRUE);
    }
    return nIndex;
}

/////////////////////////////////////////////////////////////////////////////

void CTrayListBox::ShowListIndex(size_t nPos)
{
    if (nPos < this->GetVisibleRowsBegin())
    {
        ScrollToRow(nPos);
        return;
    }
    wxRect rct = GetItemRect(nPos);
    wxRect rctClient = GetClientRect();
    if (rct.Intersects(rctClient))
        return;

    ScrollToRow(nPos);
}

/////////////////////////////////////////////////////////////////////////////

wxSize CTrayListBox::GetItemSize(size_t nIndex) const
{
    if (m_eTrayViz == trayVizAllSides || m_eTrayViz == trayVizOneSide)
    {
        std::vector<TileID> tids = GetPieceTileIDs(value_preserving_cast<size_t>(nIndex));
        return DoOnItemSize(nIndex, tids);
    }
    else
    {
        // Hidden pieces. Draw the supplied text.
        wxASSERT(!m_strHiddenString.empty());
        /* KLUDGE:  wxInfoDC should not modify wxWindow,
            so const_cast<> should be safe */
        wxInfoDC pDC(const_cast<CTrayListBox*>(this));
        pDC.SetFont(g_res.h8ssWx);
        wxSize extent = pDC.GetTextExtent(m_strHiddenString);
        return extent;
    }
}

void CTrayListBox::OnDrawItem(wxDC& pDC, const wxRect& rctItem, size_t nIndex) const
{
    if (m_eTrayViz == trayVizAllSides || m_eTrayViz == trayVizOneSide)
    {
        std::vector<TileID> tids = GetPieceTileIDs(value_preserving_cast<size_t>(nIndex));
        DoOnDrawItem(pDC, nIndex, rctItem, tids);
    }
    else
    {
        // Hidden pieces. Draw the supplied text.
        pDC.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
        pDC.SetFont(g_res.h8ssWx);
        pDC.SetTextForeground(wxSystemSettings::GetColour(IsSelected(nIndex) ?
            wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT));
        pDC.DrawLabel(m_strHiddenString, rctItem, wxALIGN_TOP | wxALIGN_LEFT);
#if 0
        if (nAction & ODA_FOCUS)
            pDC.DrawFocusRect(&rctItem);
#else
        // wx automatically draws focus rect
#endif
    }
}

std::vector<TileID> CTrayListBox::GetPieceTileIDs(size_t nIndex) const
{
    const CPieceTable& pPTbl = m_pDoc->GetPieceTable();

    PieceID pid = MapIndexToItem(nIndex);

    std::vector<TileID> retval;

    if (!m_pDoc->IsScenario() &&
        m_pDoc->HasPlayers() && pPTbl.IsPieceOwned(pid) &&
        !pPTbl.IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask()))
    {
        // Piece is owned but not by the current player. Only show the
        // top image.
        retval.push_back(pPTbl.GetFrontTileID(pid));
    }
    else
    {
        retval.push_back(pPTbl.GetActiveTileID(pid));
        wxASSERT(retval.front() != nullTid);

        if (IsShowAllSides(pid))
        {
            std::vector<TileID> inactives = pPTbl.GetInactiveTileIDs(pid);
            retval.insert(retval.end(), inactives.begin(), inactives.end());
        }
    }

    return retval;
}

BOOL CTrayListBox::OnDragSetup(DragInfoWx& pDI) const
{
    if (m_pDoc->IsPlaying())
    {
        pDI.SetDragType(DRAG_INVALID);
        return FALSE;                   // Drags not supported during play
    }

    if (IsMultiSelect())
    {
        pDI.SetDragType(DRAG_PIECELIST);
        pDI.GetSubInfo<DRAG_PIECELIST>().m_pieceIDList = &GetMappedMultiSelectList();
        pDI.GetSubInfo<DRAG_PIECELIST>().m_size = GetDragSize();
        pDI.GetSubInfo<DRAG_PIECELIST>().m_gamDoc = m_pDoc;
        pDI.m_hcsrSuggest = g_res.hcrDragTileWx;
    }
    else
    {
        wxASSERT(!"untested code");
        pDI.SetDragType(DRAG_PIECE);
        pDI.GetSubInfo<DRAG_PIECE>().m_pieceID = GetCurMapItem();
        pDI.GetSubInfo<DRAG_PIECE>().m_size = GetDragSize();
        pDI.GetSubInfo<DRAG_PIECE>().m_gamDoc = m_pDoc;
        pDI.m_hcsrSuggest = g_res.hcrDragTileWx;
    }
    return TRUE;
}

bool CTrayListBox::IsShowAllSides(PieceID pid) const
{
    const CPieceTable& pPTbl = m_pDoc->GetPieceTable();
    const PieceDef& pPce = m_pDoc->GetPieceManager().GetPiece(pid);

    BOOL bIsOwnedByCurrentPlayer = m_pDoc->HasPlayers() &&
        pPTbl.IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask());

    // If showing all sides, only show it if the piece allows it
    // or if the current players is the owner, or if the
    // program is in scenario mode.
    if (m_eTrayViz == trayVizAllSides && (bIsOwnedByCurrentPlayer &&
        !(pPce.m_flags & PieceDef::flagShowOnlyOwnersToo) ||
        !(pPce.m_flags & PieceDef::flagShowOnlyVisibleSide)) ||
        m_pDoc->IsScenario())
    {
        return true;
    }
    return false;
}


