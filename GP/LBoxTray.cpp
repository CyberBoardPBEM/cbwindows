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
    return m_eTrayViz == trayVizTwoSide || m_eTrayViz == trayVizOneSide;
}

void CTrayListBox::SetTrayContentVisibility(TrayViz eTrayViz, LPCTSTR pszHiddenString)
{
    m_eTrayViz = eTrayViz;
    m_strHiddenString = pszHiddenString;
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing

BOOL CTrayListBox::OnIsToolTipsEnabled() const
{
    if (m_eTrayViz != trayVizTwoSide && m_eTrayViz != trayVizOneSide)
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

    ASSERT(m_eTrayViz == trayVizTwoSide || m_eTrayViz == trayVizOneSide);

    const CPieceTable* pPTbl = m_pDoc.GetPieceTable();
    ASSERT(pPTbl != NULL);

    PieceID nPid = MapIndexToItem(nIndex);
    unsigned side = 0u;

    TileID tidLeft = pPTbl->GetActiveTileID(nPid);
    ASSERT(tidLeft != nullTid);            // Should exist

    TileID tidRight = nullTid;              // Initially assume no second tile image

    if (IsShowAllSides(nPid))
        tidRight = pPTbl->GetInactiveTileID(nPid);

    CRect rctLeft;
    CRect rctRight;
    GetTileRectsForItem(value_preserving_cast<int>(nIndex), tidLeft, tidRight, rctLeft, rctRight);

    if (!rctLeft.IsRectEmpty() && rctLeft.PtInRect(point))
        rct = rctLeft;
    else if (!rctRight.IsRectEmpty() && rctRight.PtInRect(point))
    {
        rct = ItemToClient(rctRight);
        side = 1u;
    }
    else
        return Invalid_v<GameElement>;

    return GameElement(nPid, side);
}

void CTrayListBox::OnGetTipTextForItemCode(GameElement nItemCode,
    CString& strTip, CString& strTitle) const
{
    if (nItemCode == Invalid_v<GameElement>)
        return;
    PieceID pid = static_cast<PieceID>(nItemCode);
    bool bRightRect = nItemCode.GetSide() != 0;
    int nSide = m_pDoc.GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
    if (bRightRect) nSide ^= 1;         // Toggle the side
    strTip = m_pDoc.GetGameElementString(MakePieceElement(pid, value_preserving_cast<unsigned>(nSide)));
}

/////////////////////////////////////////////////////////////////////////////

BOOL CTrayListBox::OnDoesItemHaveTipText(size_t nItem) const
{
    ASSERT(m_eTrayViz == trayVizTwoSide || m_eTrayViz == trayVizOneSide);

    PieceID pid = MapIndexToItem(nItem);
    int nSide = m_pDoc.GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
    if (m_pDoc.HasGameElementString(MakePieceElement(pid, value_preserving_cast<unsigned>(nSide))))
        return TRUE;
    if (m_eTrayViz == trayVizTwoSide)
    {
        // Check for tip on optional second side only of both sides are
        // visible in the tray.
        if (m_pDoc.GetPieceTable()->Is2Sided(pid) &&
            m_pDoc.HasGameElementString(MakePieceElement(pid, value_preserving_cast<unsigned>(nSide ^ 1))))
            return TRUE;
    }
    return FALSE;
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
    if (m_eTrayViz == trayVizTwoSide || m_eTrayViz == trayVizOneSide)
    {
        TileID tid1, tid2;
        GetPieceTileIDs(value_preserving_cast<size_t>(nIndex), tid1, tid2);
        return DoOnItemSize(nIndex, tid1, tid2);
    }
    else
    {
        // Hidden pieces. Draw the supplied text.
        ASSERT(!m_strHiddenString.IsEmpty());
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

    if (m_eTrayViz == trayVizTwoSide || m_eTrayViz == trayVizOneSide)
    {
        TileID tid1, tid2;
        GetPieceTileIDs(value_preserving_cast<size_t>(nIndex), tid1, tid2);
        DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tid1, tid2);
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

void CTrayListBox::GetPieceTileIDs(size_t nIndex, TileID& tid1, TileID& tid2) const
{
    const CPieceTable* pPTbl = m_pDoc.GetPieceTable();
    ASSERT(pPTbl != NULL);

    PieceID pid = MapIndexToItem(nIndex);

    tid2 = nullTid;              // Initially assume no second tile image

    if (!m_pDoc.IsScenario() &&
        m_pDoc.HasPlayers() && pPTbl->IsPieceOwned(pid) &&
        !pPTbl->IsPieceOwnedBy(pid, m_pDoc.GetCurrentPlayerMask()))
    {
        // Piece is owned but not by the current player. Only show the
        // top image.
        tid1 = pPTbl->GetFrontTileID(pid);
    }
    else
    {
        tid1 = pPTbl->GetActiveTileID(pid);
        ASSERT(tid1 != nullTid);

        if (IsShowAllSides(pid))
            tid2 = pPTbl->GetInactiveTileID(pid);
    }
}

BOOL CTrayListBox::OnDragSetup(DragInfo& pDI) const
{
    if (m_pDoc.IsPlaying())
    {
        pDI.m_dragType = DRAG_INVALID;
        return FALSE;                   // Drags not supported during play
    }

    if (IsMultiSelect())
    {
        pDI.m_dragType = DRAG_PIECELIST;
        pDI.GetSubInfo<DRAG_PIECELIST>().m_pieceIDList = &GetMappedMultiSelectList();
        pDI.GetSubInfo<DRAG_PIECELIST>().m_gamDoc = &m_pDoc;
        pDI.m_hcsrSuggest = g_res.hcrDragTile;
    }
    else
    {
        pDI.m_dragType = DRAG_PIECE;
        pDI.GetSubInfo<DRAG_PIECE>().m_pieceID = GetCurMapItem();
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


