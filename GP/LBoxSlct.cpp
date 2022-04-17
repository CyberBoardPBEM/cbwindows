// LBoxSlct.cpp
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
#include    "LBoxSlct.h"
#include    "DrawObj.h"
#include    "PPieces.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

const int tileBorder = 3;
const int tileGap = 6;

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSelectListBox, CTileBaseListBox2)
    //{{AFX_MSG_MAP(CSelectListBox)
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragItem)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

const CTileManager& CSelectListBox::GetTileManager() const
{
    ASSERT(m_pDoc != NULL);
    return CheckedDeref(m_pDoc->GetTileManager());
}

/////////////////////////////////////////////////////////////////////////////

BOOL CSelectListBox::OnDragSetup(DragInfo& pDI) const
{
    ASSERT(!"untested code");
    if (GetCount() <= 1)
    {
        pDI.m_dragType = DRAG_INVALID;
        return FALSE;
    }

    if (!IsMultiSelect())
    {
        ASSERT(!"unreachable code");
        /* if this ever happens, rewrite like LBoxGrfx w/
            list/single distinction
        m_multiSelList.clear();
        m_multiSelList.push_back(&GetCurMapItem());
        */
    }
    pDI.m_dragType = DRAG_SELECTVIEW;
    pDI.GetSubInfo<DRAG_SELECTVIEW>().m_ptrArray = &GetMappedMultiSelectList();
    pDI.m_hcsrSuggest = g_res.hcrDragTile;
    pDI.GetSubInfo<DRAG_SELECTVIEW>().m_gamDoc = m_pDoc;
    return TRUE;
}

LRESULT CSelectListBox::OnDragItem(WPARAM wParam, LPARAM lParam)
{
    const DragInfo& pdi = CheckedDeref(reinterpret_cast<const DragInfo*>(lParam));

    DoInsertLineProcessing((UINT)wParam, pdi);

    if (pdi.m_dragType != DRAG_SELECTVIEW)
        return 0;               // Only our drops allowed

    if (pdi.GetSubInfo<DRAG_SELECTVIEW>().m_gamDoc != m_pDoc)
        return 0;               // Only pieces from our document.

    DoAutoScrollProcessing(pdi);

    if (wParam == phaseDragOver)
        return (LRESULT)(LPVOID)pdi.m_hcsrSuggest;
    else if (wParam == phaseDragDrop)
    {
        int nSel = SpecialItemFromPoint(pdi.m_point);

        if (nSel < GetCount())
        {
            // If the selection is out of view, force it into view.
            MakeItemVisible(nSel);
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing

BOOL CSelectListBox::OnIsToolTipsEnabled() const
{
    return m_pDoc->IsShowingObjectTips();
}

GameElement CSelectListBox::OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) const
{
    point = ClientToItem(point);

    BOOL bOutsideClient;
    UINT nIndex = ItemFromPoint(point, bOutsideClient);
    if (nIndex >= 65535 || GetCount() <= 0)
        return Invalid_v<GameElement>;

    TileID tidLeft = GetTileID(TRUE, nIndex);
    ASSERT(tidLeft != nullTid);
    TileID tidRight = GetTileID(FALSE, nIndex);

    CRect rctLeft;
    CRect rctRight;
    GetTileRectsForItem(nIndex, tidLeft, tidRight, rctLeft, rctRight);

    GameElement elem = Invalid_v<GameElement>;

    if (!rctLeft.IsRectEmpty() && rctLeft.PtInRect(point))
    {
        const CDrawObj& pObj = MapIndexToItem(nIndex);
        elem = m_pDoc->GetVerifiedGameElementCodeForObject(pObj);
        rct = ItemToClient(rctLeft);
    }
    else if (!rctRight.IsRectEmpty() && rctRight.PtInRect(point))
    {
        const CDrawObj& pObj = MapIndexToItem(nIndex);
        if (pObj.GetType() != CDrawObj::drawPieceObj)
        {
            CbThrowBadCastException();
        }
        const CPieceObj& pieceObj = static_cast<const CPieceObj&>(pObj);
        ASSERT(pieceObj.m_pDoc == m_pDoc);
        const CPieceTable& pieceTbl = CheckedDeref(pieceObj.m_pDoc->GetPieceTable());
        size_t nSide = pieceTbl.GetSide(pieceObj.m_pid, size_t(1));
        elem = m_pDoc->GetVerifiedGameElementCodeForObject(pObj, nSide);
        rct = ItemToClient(rctRight);
    }

    return elem;
}

void CSelectListBox::OnGetTipTextForItemCode(GameElement nItemCode,
    CString& strTip, CString& strTitle) const
{
    if (nItemCode == Invalid_v<GameElement>)
        return;
    strTip = m_pDoc->GetGameElementString(nItemCode);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CSelectListBox::OnDoesItemHaveTipText(size_t nItem) const
{
    const CDrawObj& pObj = MapIndexToItem(nItem);
    GameElement elem1 = m_pDoc->GetVerifiedGameElementCodeForObject(pObj, size_t(0));
    GameElement elem2 = m_pDoc->GetVerifiedGameElementCodeForObject(pObj, size_t(1));
    return elem1 != Invalid_v<GameElement> || elem2 != Invalid_v<GameElement>;
}

/////////////////////////////////////////////////////////////////////////////

std::string CSelectListBox::OnGetItemDebugString(size_t nIndex) const
{
    const CDrawObj& pDObj = MapIndexToItem(nIndex);
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        PieceID pid = static_cast<const CPieceObj&>(pDObj).m_pid;
        return CB::Sprintf("[pid:%u] ", value_preserving_cast<UINT>(static_cast<PieceID::UNDERLYING_TYPE>(pid)));
    }
    else if (pDObj.GetType() == CDrawObj::drawMarkObj)
    {
        MarkID mid = static_cast<const CMarkObj&>(pDObj).m_mid;
        return CB::Sprintf("[mid:%u] ", value_preserving_cast<UINT>(static_cast<MarkID::UNDERLYING_TYPE>(mid)));
    }
    return std::string();
}

/////////////////////////////////////////////////////////////////////////////

CSize CSelectListBox::OnItemSize(size_t nIndex) const
{
    ASSERT(m_pDoc != NULL);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    TileID tid1 = GetTileID(TRUE, nIndex);
    ASSERT(tid1 != nullTid);
    TileID tid2 = GetTileID(FALSE, nIndex);

    return DoOnItemSize(nIndex, tid1, tid2);
}

void CSelectListBox::OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
    CRect rctItem) const
{
    // see https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-drawitemstruct
    if (nIndex == size_t(UINT(-1)))
        return;                 // Nothing to draw.

    TileID tid1 = GetTileID(TRUE, nIndex);
    ASSERT(tid1 != nullTid);
    TileID tid2 = GetTileID(FALSE, nIndex);
    DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tid1, tid2);
}

TileID CSelectListBox::GetTileID(BOOL bActiveIfApplies, size_t nIndex) const
{
    const CDrawObj& pDObj = MapIndexToItem(nIndex);

    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        CPieceTable* pPTbl = m_pDoc->GetPieceTable();
        ASSERT(pPTbl != NULL);

        PieceID pid = static_cast<const CPieceObj&>(pDObj).m_pid;

        if (!m_pDoc->IsScenario() && pPTbl->IsPieceOwned(pid) &&
            !pPTbl->IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask()))
        {
            if (bActiveIfApplies)
                return pPTbl->GetFrontTileID(pid);
            else
                return nullTid;
        }

        const PieceDef& pPce = m_pDoc->GetPieceManager()->GetPiece(pid);

        if ((pPce.m_flags & PieceDef::flagShowOnlyVisibleSide) && !bActiveIfApplies &&
            (!pPTbl->IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask()) ||
             pPce.m_flags & PieceDef::flagShowOnlyOwnersToo))
        {
            return nullTid;
        }

        return bActiveIfApplies ? pPTbl->GetActiveTileID(pid) :
            pPTbl->GetInactiveTileID(pid);

    }
    else if (pDObj.GetType() == CDrawObj::drawMarkObj)
    {
        if (!bActiveIfApplies) return nullTid;      // Inactive side doesn't apply

        MarkID mid = static_cast<const CMarkObj&>(pDObj).m_mid;
        CMarkManager* pMMgr = m_pDoc->GetMarkManager();
        ASSERT(pMMgr != NULL);
        return pMMgr->GetMark(mid).m_tid;
    }
    else
    {
        ASSERT(FALSE);                              // Shouldn't happen
        return nullTid;
    }
}
