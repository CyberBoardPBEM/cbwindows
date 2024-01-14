// LBoxTray.cpp
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

BEGIN_MESSAGE_MAP(CTrayListBox, CTileBaseListBox)
    //{{AFX_MSG_MAP(CTrayListBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CTrayListBox::CTrayListBox(CGamDoc& pDoc) :
    m_pDoc(pDoc)
{
    CGrafixListBoxData::SetDocument(pDoc);
    m_eTrayViz = trayVizOneSide;
    m_bAllowTips = TRUE;
}

/////////////////////////////////////////////////////////////////////////////

const CTileManager& CTrayListBox::GetTileManager() const
{
    return CheckedDeref(m_pDoc.GetTileManager());
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
    return m_pDoc.IsShowingObjectTips() && m_bAllowTips;
}

GameElement CTrayListBox::OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) const
{
    point = ClientToItem(point);

    BOOL bOutsideClient;
    UINT nIndex = ItemFromPoint(point, bOutsideClient);
    if (nIndex >= 65535 || GetCount() <= 0)
        return Invalid_v<GameElement>;

    ASSERT(m_eTrayViz == trayVizAllSides || m_eTrayViz == trayVizOneSide);

    const CPieceTable* pPTbl = m_pDoc.GetPieceTable();
    ASSERT(pPTbl != NULL);

    PieceID nPid = MapIndexToItem(nIndex);

    TileID tidLeft = pPTbl->GetActiveTileID(nPid);
    ASSERT(tidLeft != nullTid);            // Should exist

    std::vector<TileID> tids;
    tids.push_back(tidLeft);                // Initially assume no second tile image

    if (IsShowAllSides(nPid))
    {
        std::vector<TileID> inactives = pPTbl->GetInactiveTileIDs(nPid);
        tids.insert(tids.end(), inactives.begin(), inactives.end());
    }

    std::vector<CRect> rects = GetTileRectsForItem(value_preserving_cast<int>(nIndex), tids);

    for (size_t i = size_t(0) ; i < rects.size() ; ++i)
    {
        ASSERT(!rects[i].IsRectEmpty());
        if (!rects[i].IsRectEmpty() && rects[i].PtInRect(point))
        {
            rct = ItemToClient(rects[i]);
            const CPieceTable& pieceTbl = CheckedDeref(m_pDoc.GetPieceTable());
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
    strTip = m_pDoc.GetGameElementString(nItemCode);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CTrayListBox::OnDoesItemHaveTipText(size_t nItem) const
{
    ASSERT(m_eTrayViz == trayVizAllSides || m_eTrayViz == trayVizOneSide);

    PieceID pid = MapIndexToItem(nItem);
    if (m_eTrayViz != trayVizAllSides)
    {
        uint8_t side = m_pDoc.GetPieceTable()->GetSide(pid);
        return m_pDoc.HasGameElementString(MakePieceElement(pid, side));
    }
    else
    {
        size_t sides = m_pDoc.GetPieceTable()->GetSides(pid);
        for (unsigned i = unsigned(0) ; i < sides ; ++i)
        {
            if (m_pDoc.HasGameElementString(MakePieceElement(pid, i)))
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
    ASSERT(IsMultiSelect());
    if (GetCount() < 1)
        return;
    SelItemRange(FALSE, 0, GetCount()-1);
}

size_t CTrayListBox::SelectTrayPiece(PieceID pid)
{
    size_t nIndex = MapItemToIndex(pid);
    if (nIndex != Invalid_v<size_t>)
    {
        ShowListIndex(value_preserving_cast<int>(nIndex));
        SetSel(value_preserving_cast<int>(nIndex), TRUE);
    }
    else
    {
        if (GetCount() > 0)
            SetSel(0, TRUE);
    }
    return nIndex;
}

/////////////////////////////////////////////////////////////////////////////

void CTrayListBox::ShowListIndex(int nPos)
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

CSize CTrayListBox::OnItemSize(size_t nIndex) const
{
    if (m_eTrayViz == trayVizAllSides || m_eTrayViz == trayVizOneSide)
    {
        std::vector<TileID> tids = GetPieceTileIDs(value_preserving_cast<size_t>(nIndex));
        return DoOnItemSize(nIndex, tids);
    }
    else
    {
        // Hidden pieces. Draw the supplied text.
        ASSERT(!m_strHiddenString.empty());
        // only using DC to measure text, so const_cast safe;
        CClientDC pDC(const_cast<CTrayListBox*>(this));
        pDC.SaveDC();
        CFont* prvFont = (CFont*)pDC.SelectObject(CFont::FromHandle(g_res.h8ss));
        CSize extent = pDC.GetTextExtent(m_strHiddenString);
        pDC.SelectObject(prvFont);
        pDC.RestoreDC(-1);
        return extent;
    }
}

void CTrayListBox::OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
    CRect rctItem) const
{
    // see https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-drawitemstruct
    if (nIndex == size_t(UINT(-1)))
        return;

    if (m_eTrayViz == trayVizAllSides || m_eTrayViz == trayVizOneSide)
    {
        std::vector<TileID> tids = GetPieceTileIDs(value_preserving_cast<size_t>(nIndex));
        DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tids);
    }
    else
    {
        if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
        {
            // Hidden pieces. Draw the supplied text.
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

std::vector<TileID> CTrayListBox::GetPieceTileIDs(size_t nIndex) const
{
    const CPieceTable* pPTbl = m_pDoc.GetPieceTable();
    ASSERT(pPTbl != NULL);

    PieceID pid = MapIndexToItem(nIndex);

    std::vector<TileID> retval;

    if (!m_pDoc.IsScenario() &&
        m_pDoc.HasPlayers() && pPTbl->IsPieceOwned(pid) &&
        !pPTbl->IsPieceOwnedBy(pid, m_pDoc.GetCurrentPlayerMask()))
    {
        // Piece is owned but not by the current player. Only show the
        // top image.
        retval.push_back(pPTbl->GetFrontTileID(pid));
    }
    else
    {
        retval.push_back(pPTbl->GetActiveTileID(pid));
        ASSERT(retval.front() != nullTid);

        if (IsShowAllSides(pid))
        {
            std::vector<TileID> inactives = pPTbl->GetInactiveTileIDs(pid);
            retval.insert(retval.end(), inactives.begin(), inactives.end());
        }
    }

    return retval;
}

BOOL CTrayListBox::OnDragSetup(DragInfo& pDI) const
{
    if (m_pDoc.IsPlaying())
    {
        pDI.SetDragType(DRAG_INVALID);
        return FALSE;                   // Drags not supported during play
    }

    if (IsMultiSelect())
    {
        pDI.SetDragType(DRAG_PIECELIST);
        pDI.GetSubInfo<DRAG_PIECELIST>().m_pieceIDList = &GetMappedMultiSelectList();
        pDI.GetSubInfo<DRAG_PIECELIST>().m_size = GetDragSize();
        pDI.GetSubInfo<DRAG_PIECELIST>().m_gamDoc = &m_pDoc;
        pDI.m_hcsrSuggest = g_res.hcrDragTile;
    }
    else
    {
        ASSERT(!"untested code");
        pDI.SetDragType(DRAG_PIECE);
        pDI.GetSubInfo<DRAG_PIECE>().m_pieceID = GetCurMapItem();
        pDI.GetSubInfo<DRAG_PIECE>().m_size = GetDragSize();
        pDI.GetSubInfo<DRAG_PIECE>().m_gamDoc = &m_pDoc;
        pDI.m_hcsrSuggest = g_res.hcrDragTile;
    }
    return TRUE;
}

bool CTrayListBox::IsShowAllSides(PieceID pid) const
{
    const CPieceTable& pPTbl = CheckedDeref(m_pDoc.GetPieceTable());
    const PieceDef& pPce = m_pDoc.GetPieceManager()->GetPiece(pid);

    BOOL bIsOwnedByCurrentPlayer = m_pDoc.HasPlayers() &&
        pPTbl.IsPieceOwnedBy(pid, m_pDoc.GetCurrentPlayerMask());

    // If showing all sides, only show it if the piece allows it
    // or if the current players is the owner, or if the
    // program is in scenario mode.
    if (m_eTrayViz == trayVizAllSides && (bIsOwnedByCurrentPlayer &&
        !(pPce.m_flags & PieceDef::flagShowOnlyOwnersToo) ||
        !(pPce.m_flags & PieceDef::flagShowOnlyVisibleSide)) ||
        m_pDoc.IsScenario())
    {
        return true;
    }
    return false;
}


