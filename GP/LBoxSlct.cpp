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

BEGIN_MESSAGE_MAP(CSelectListBox, CGrafixListBox2)
    //{{AFX_MSG_MAP(CSelectListBox)
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragItem)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CTileManager* CSelectListBox::GetTileManager()
{
    ASSERT(m_pDoc != NULL);
    ASSERT(m_pDoc->GetTileManager() != NULL);
    return m_pDoc->GetTileManager();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CSelectListBox::OnDragSetup(DragInfo* pDI)
{
    if (GetCount() <= 1)
        return FALSE;

    if (!IsMultiSelect())
    {
        m_multiSelList.RemoveAll();
        m_multiSelList.Add(GetCurMapItem());
    }
    pDI->m_dragType = DRAG_SELECTVIEW;
    pDI->m_dwVal = (DWORD)GetMappedMultiSelectList();
        pDI->m_hcsrSuggest = g_res.hcrDragTile;
    pDI->m_pObj = (void*)m_pDoc;
    return TRUE;
}

LRESULT CSelectListBox::OnDragItem(WPARAM wParam, LPARAM lParam)
{
    DragInfo* pdi = (DragInfo*)lParam;

    DoInsertLineProcessing((UINT)wParam, pdi);

    if (pdi->m_dragType != DRAG_SELECTVIEW)
        return 0;               // Only our drops allowed

    if (pdi->m_pObj != (void*)m_pDoc)
        return 0;               // Only pieces from our document.

    DoAutoScrollProcessing(pdi);

    if (wParam == phaseDragOver)
        return (LRESULT)(LPVOID)pdi->m_hcsrSuggest;
    else if (wParam == phaseDragDrop)
    {
        int nSel = SpecialItemFromPoint(pdi->m_point);

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

BOOL CSelectListBox::OnIsToolTipsEnabled()
{
    return m_pDoc->IsShowingObjectTips();
}

int  CSelectListBox::OnGetHitItemCodeAtPoint(CPoint point, CRect& rct)
{
    BOOL bOutsideClient;
    UINT nIndex = ItemFromPoint(point, bOutsideClient);
    if (nIndex >= 65535 || GetCount() <= 0)
        return -1;

    TileID tidLeft = GetTileID(TRUE, nIndex);
    ASSERT(tidLeft != nullTid);
    TileID tidRight = GetTileID(FALSE, nIndex);

    CRect rctLeft;
    CRect rctRight;
    GetTileRectsForItem(nIndex, tidLeft, tidRight, rctLeft, rctRight);

    GameElement elem = (GameElement)-1;

    if (!rctLeft.IsRectEmpty() && rctLeft.PtInRect(point))
    {
        CDrawObj* pObj = (CDrawObj*)MapIndexToItem(nIndex);
        elem = m_pDoc->GetVerifiedGameElementCodeForObject(pObj);
        rct = rctLeft;
    }
    else if (!rctRight.IsRectEmpty() && rctRight.PtInRect(point))
    {
        CDrawObj* pObj = (CDrawObj*)MapIndexToItem(nIndex);
        elem = m_pDoc->GetVerifiedGameElementCodeForObject(pObj, TRUE);
        rct = rctRight;
    }
    else
        return -1;

    return (int)elem;
}

void CSelectListBox::OnGetTipTextForItemCode(int nItemCode,
    CString& strTip, CString& strTitle)
{
    if (nItemCode == -1)
        return;
    GameElement elem = (GameElement)nItemCode;
    strTip = m_pDoc->GetGameElementString(elem);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CSelectListBox::OnDoesItemHaveTipText(int nItem)
{
    CDrawObj* pObj = (CDrawObj*)MapIndexToItem(nItem);
    GameElement elem1 = m_pDoc->GetVerifiedGameElementCodeForObject(pObj, FALSE);
    GameElement elem2 = m_pDoc->GetVerifiedGameElementCodeForObject(pObj, TRUE);
    return elem1 != (GameElement)-1 || elem2 != (GameElement)-1;
}

/////////////////////////////////////////////////////////////////////////////

void CSelectListBox::OnGetItemDebugString(int nIndex, CString& str)
{
    CDrawObj* pDObj = (CDrawObj*)MapIndexToItem(nIndex);
    if (pDObj->GetType() == CDrawObj::drawPieceObj)
    {
        PieceID pid = ((CPieceObj*)pDObj)->m_pid;
        str.Format("[pid:%d] ", (UINT)pid);
    }
    else if (pDObj->GetType() == CDrawObj::drawMarkObj)
    {
        MarkID mid = ((CMarkObj*)pDObj)->m_mid;
        str.Format("[mid:%d] ", (UINT)mid);
    }
}

/////////////////////////////////////////////////////////////////////////////

int CSelectListBox::OnItemHeight(int nIndex)
{
    ASSERT(m_pDoc != NULL);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    TileID tid1 = GetTileID(TRUE, nIndex);
    ASSERT(tid1 != nullTid);
    TileID tid2 = GetTileID(FALSE, nIndex);

    return DoOnItemHeight(tid1, tid2);
}

void CSelectListBox::OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
    CRect rctItem)
{
    if (nIndex < 0)
        return;                 // Nothing to draw.

    TileID tid1 = GetTileID(TRUE, nIndex);
    ASSERT(tid1 != nullTid);
    TileID tid2 = GetTileID(FALSE, nIndex);
    DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tid1, tid2);
}

TileID CSelectListBox::GetTileID(BOOL bActiveIfApplies, int nIndex)
{
    CDrawObj* pDObj = (CDrawObj*)MapIndexToItem(nIndex);

    if (pDObj->GetType() == CDrawObj::drawPieceObj)
    {
        CPieceTable* pPTbl = m_pDoc->GetPieceTable();
        ASSERT(pPTbl != NULL);

        PieceID pid = ((CPieceObj*)pDObj)->m_pid;

        if (!m_pDoc->IsScenario() && pPTbl->IsPieceOwned(pid) &&
            !pPTbl->IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask()))
        {
            if (bActiveIfApplies)
                return pPTbl->GetFrontTileID(pid);
            else
                return nullTid;
        }

        PieceDef* pPce = m_pDoc->GetPieceManager()->GetPiece(pid);

        if ((pPce->m_flags & PieceDef::flagShowOnlyVisibleSide) && !bActiveIfApplies &&
            (!pPTbl->IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask()) ||
             pPce->m_flags & PieceDef::flagShowOnlyOwnersToo))
        {
            return nullTid;
        }

        return bActiveIfApplies ? pPTbl->GetActiveTileID(pid) :
            pPTbl->GetInactiveTileID(pid);

    }
    else if (pDObj->GetType() == CDrawObj::drawMarkObj)
    {
        if (!bActiveIfApplies) return nullTid;      // Inactive side doesn't apply

        MarkID mid = ((CMarkObj*)pDObj)->m_mid;
        CMarkManager* pMMgr = m_pDoc->GetMarkManager();
        ASSERT(pMMgr != NULL);
        return pMMgr->GetMark(mid)->m_tid;
    }
    else
    {
        ASSERT(FALSE);                              // Shouldn't happen
        return nullTid;
    }
}
