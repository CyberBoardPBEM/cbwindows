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

CTrayListBox::CTrayListBox()
{
    m_pDoc = NULL;
    m_eTrayViz = trayVizOneSide;
    m_bAllowTips = TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CTrayListBox::SetDocument(CGamDoc *pDoc)
{
    m_pDoc = pDoc;
}

CTileManager* CTrayListBox::GetTileManager()
{
    ASSERT(m_pDoc != NULL);
    ASSERT(m_pDoc->GetTileManager() != NULL);
    return m_pDoc->GetTileManager();
}

BOOL CTrayListBox::IsShowingTileImages()
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

BOOL CTrayListBox::OnIsToolTipsEnabled()
{
    if (m_eTrayViz != trayVizTwoSide && m_eTrayViz != trayVizOneSide)
        return FALSE;
    return m_pDoc->IsShowingObjectTips() && m_bAllowTips;
}

int  CTrayListBox::OnGetHitItemCodeAtPoint(CPoint point, CRect& rct)
{
    BOOL bOutsideClient;
    UINT nIndex = ItemFromPoint(point, bOutsideClient);
    if (nIndex >= 65535 || GetCount() <= 0)
        return -1;

    ASSERT(m_eTrayViz == trayVizTwoSide || m_eTrayViz == trayVizOneSide);

    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);

    UINT nPid = MapIndexToItem(nIndex);

    TileID tidLeft = pPTbl->GetActiveTileID((PieceID)nPid);
    ASSERT(tidLeft != nullTid);            // Should exist

    TileID tidRight = nullTid;              // Initially assume no second tile image

    if (m_eTrayViz == trayVizTwoSide)
        tidRight = pPTbl->GetInactiveTileID((PieceID)nPid);

    CRect rctLeft;
    CRect rctRight;
    GetTileRectsForItem(nIndex, tidLeft, tidRight, rctLeft, rctRight);

    if (!rctLeft.IsRectEmpty() && rctLeft.PtInRect(point))
        rct = rctLeft;
    else if (!rctRight.IsRectEmpty() && rctRight.PtInRect(point))
    {
        rct = rctRight;
        nPid |= 0x10000;             // Set flag bit indicating right side rect
    }
    else
        return -1;

    return (int)nPid;
}

void CTrayListBox::OnGetTipTextForItemCode(int nItemCode,
    CString& strTip, CString& strTitle)
{
    if (nItemCode < 0)
        return;
    PieceID pid = (PieceID)(nItemCode & 0xFFFF);
    BOOL bRightRect = (nItemCode & 0x10000) != 0;
    int nSide = m_pDoc->GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
    if (bRightRect) nSide ^= 1;         // Toggle the side
    strTip = m_pDoc->GetGameElementString(MakePieceElement(pid, nSide));
}

/////////////////////////////////////////////////////////////////////////////

BOOL CTrayListBox::OnDoesItemHaveTipText(int nItem)
{
    ASSERT(m_eTrayViz == trayVizTwoSide || m_eTrayViz == trayVizOneSide);

    PieceID pid = (PieceID)MapIndexToItem(nItem);
    int nSide = m_pDoc->GetPieceTable()->IsFrontUp(pid) ? 0 : 1;
    if (m_pDoc->HasGameElementString(MakePieceElement(pid, nSide)))
        return TRUE;
    if (m_eTrayViz == trayVizTwoSide)
    {
        // Check for tip on optional second side only of both sides are
        // visible in the tray.
        if (m_pDoc->GetPieceTable()->Is2Sided(pid) &&
            m_pDoc->HasGameElementString(MakePieceElement(pid, nSide ^ 1)))
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

int CTrayListBox::SelectTrayPiece(PieceID pid)
{
    int nIndex = MapItemToIndex((int)pid);
    if (nIndex != -1)
    {
        ShowListIndex(nIndex);
        SetSel(nIndex, TRUE);
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

int CTrayListBox::OnItemHeight(int nIndex)
{
    if (m_eTrayViz == trayVizTwoSide || m_eTrayViz == trayVizOneSide)
    {
        TileID tid1, tid2;
        GetPieceTileIDs(nIndex, tid1, tid2);
        return DoOnItemHeight(tid1, tid2);
    }
    else
    {
        // Hidden pieces. Draw the supplied text.
        int nHeight = g_res.tm8ss.tmHeight + g_res.tm8ss.tmExternalLeading;
        return nHeight;
    }
}

void CTrayListBox::OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
    CRect rctItem)
{
    if (nIndex == -1)
        return;

    if (m_eTrayViz == trayVizTwoSide || m_eTrayViz == trayVizOneSide)
    {
        TileID tid1, tid2;
        GetPieceTileIDs(nIndex, tid1, tid2);
        DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tid1, tid2);
    }
    else
    {
        if (nAction & (ODA_DRAWENTIRE | ODA_SELECT))
        {
            // Hidden pieces. Draw the supplied text.
            pDC->SetTextAlign(TA_TOP | TA_LEFT);
            CBrush brBack(GetSysColor(nState & ODS_SELECTED ?
                COLOR_HIGHLIGHT : COLOR_WINDOW));
            pDC->FillRect(&rctItem, &brBack);       // Fill background color
            pDC->SetBkMode(TRANSPARENT);
            CFont* pPrvFont = pDC->SelectObject(CFont::FromHandle(g_res.h8ss));
            pDC->SetTextColor(GetSysColor(nState & ODS_SELECTED ?
                COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
            pDC->TextOut(rctItem.left, rctItem.top, m_strHiddenString);
            pDC->SelectObject(pPrvFont);
        }
        if (nAction & ODA_FOCUS)
            pDC->DrawFocusRect(&rctItem);
    }
}

void CTrayListBox::GetPieceTileIDs(int nIndex, TileID& tid1, TileID& tid2)
{
    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);

    PieceID pid = (PieceID)MapIndexToItem(nIndex);

    tid2 = nullTid;              // Initially assume no second tile image

    if (!m_pDoc->IsScenario() &&
        m_pDoc->HasPlayers() && pPTbl->IsPieceOwned(pid) &&
        !pPTbl->IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask()))
    {
        // Piece is owned but not by the current player. Only show the
        // top image.
        tid1 = pPTbl->GetFrontTileID(pid);
    }
    else
    {
        tid1 = pPTbl->GetActiveTileID(pid);
        ASSERT(tid1 != nullTid);

        PieceDef* pPce = m_pDoc->GetPieceManager()->GetPiece(pid);

        BOOL bIsOwnedByCurrentPlayer = m_pDoc->HasPlayers() &&
            pPTbl->IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask());

        // If showing two sides, only show it if the piece allows it
        // or if the current players is the owner, or if the
        // program is in scenario mode.
        if (m_eTrayViz == trayVizTwoSide && (bIsOwnedByCurrentPlayer &&
                !(pPce->m_flags & PieceDef::flagShowOnlyOwnersToo)   ||
                !(pPce->m_flags & PieceDef::flagShowOnlyVisibleSide))||
                 m_pDoc->IsScenario())
            tid2 = pPTbl->GetInactiveTileID(pid);
    }
}

BOOL CTrayListBox::OnDragSetup(DragInfo* pDI)
{
    if (m_pDoc->IsPlaying())
        return FALSE;                   // Drags not supported during play

    if (IsMultiSelect())
    {
        pDI->m_dragType = DRAG_PIECELIST;
        pDI->GetSubInfo<DRAG_PIECELIST>().m_pieceIDList = GetMappedMultiSelectList();// PieceID array
        pDI->GetSubInfo<DRAG_PIECELIST>().m_gamDoc = m_pDoc;
        pDI->m_hcsrSuggest = g_res.hcrDragTile;
    }
    else
    {
        pDI->m_dragType = DRAG_PIECE;
        pDI->GetSubInfo<DRAG_PIECE>().m_pieceID = GetCurMapItem();      // The PieceID
        pDI->GetSubInfo<DRAG_PIECE>().m_gamDoc = m_pDoc;
        pDI->m_hcsrSuggest = g_res.hcrDragTile;
    }
    return TRUE;
}


