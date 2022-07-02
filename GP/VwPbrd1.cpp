// VwPbrd1.cpp : implementation of the CPlayBoardView class
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
#include    "Board.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "ToolPlay.h"
#include    "SelOPlay.h"
#include    "VwPbrd.h"
#include    "FrmMain.h"
#include    "GMisc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

const int viewZone = 24;
const int scrollZone = 16;

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::DoToolTipHitProcessing(CPoint pointClient)
{
    CGamDoc* pDoc = GetDocument();
    if (!pDoc->IsShowingObjectTips() && pDoc->IsOwnerTipsDisabled())
    {
        // Delete previous tool definition
        m_toolHitTip.DelTool(this, ID_TIP_PLAYBOARD_HIT);
        m_pCurTipObj = NULL;
        return;
    }

    CPoint pnt(pointClient);
    ClientToWorkspace(pnt);
    CDrawObj* pDObj = ObjectHitTest(pnt);

    if (pDoc->IsOwnerTipsDisabled() && pDoc->HasPlayers())
    {
        if (pDObj != NULL && pDObj->GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj* pPObj = (CPieceObj*)pDObj;
            if (pPObj->IsOwned() &&
                !pDoc->GetPieceTable()->IsPieceInvisible(pPObj->m_pid))
            {
                CString strOwner;
                pDoc->GetPieceOwnerName(pPObj->m_pid, strOwner);
                CString strOwnedBy;
                strOwnedBy.Format(IDS_TIP_OWNED_BY_UC, (LPCTSTR)strOwner);
                GetMainFrame()->GetStatusBar()->SetWindowText(strOwnedBy);
            }
            else
                GetMainFrame()->GetStatusBar()->SetWindowText("");
        }
        else
            GetMainFrame()->GetStatusBar()->SetWindowText("");
    }

    if (pDObj != m_pCurTipObj)
    {
        // Object changed so delete previous tool definition
        m_toolHitTip.DelTool(this, ID_TIP_PLAYBOARD_HIT);
        m_pCurTipObj = pDObj;
        if (pDObj != NULL)
        {
            // New object found so create a new tip
            CRect rct = pDObj->GetRect();
            WorkspaceToClient(rct);

            CString strTip;
            CString strTitle;
            if (pDoc->IsShowingObjectTips())
                pDoc->GetTipTextForObject(*pDObj, strTip, &strTitle);

            // All this stuff is used to annotate tips with owner names
            // when player accounts are active.
            if (pDoc->HasPlayers() && pDObj->GetType() == CDrawObj::drawPieceObj)
            {
                CPieceObj* pPObj = (CPieceObj*)pDObj;
                if (pPObj->IsOwned() &&
                    !pDoc->GetPieceTable()->IsPieceInvisible(pPObj->m_pid))
                {
                    CString strOwner;
                    pDoc->GetPieceOwnerName(pPObj->m_pid, strOwner);
                    CString strOwnedBy;
                    strOwnedBy.Format(IDS_TIP_OWNED_BY_UC, (LPCTSTR)strOwner);

                    if (!pDoc->IsScenario() && !pPObj->IsOwnedBy(pDoc->GetCurrentPlayerMask()))
                    {
                        strTip.Empty();             // Current player isn't allowed to see text.
                        if (!pDoc->IsOwnerTipsDisabled() && !strOwner.IsEmpty())// Replace tip with special "Owned by" tip
                            strTip = strOwnedBy;
                    }
                    else
                    {
                        if (!pDoc->IsOwnerTipsDisabled())
                        {
                            // Append actual tip to owner string.
                            if (strTip.IsEmpty())
                                strTip = strOwnedBy;
                            else
                            {
                                strOwnedBy += "\r\n";
                                strTip = strOwnedBy + strTip;
                            }
                        }
                    }
                }
            }
            if (!strTip.IsEmpty())
            {
                m_toolHitTip.AddTool(this, strTip, rct, ID_TIP_PLAYBOARD_HIT);

                if (!strTitle.IsEmpty())
                    m_toolHitTip.SendMessage(TTM_SETTITLE, 0, (LPARAM)(LPCTSTR)strTitle);

                m_toolHitTip.Activate(TRUE);
            }
            else
            {
                // Delete previous tool definition
                m_toolHitTip.DelTool(this, ID_TIP_PLAYBOARD_HIT);
                m_pCurTipObj = NULL;
                return;
            }
        }
        else
            m_toolHitTip.Activate(FALSE);
    }
}

void CPlayBoardView::ClearToolTip()
{
    // Object changed so delete previous tool definition
    m_toolHitTip.DelTool(this, ID_TIP_PLAYBOARD_HIT);
    m_pCurTipObj = NULL;
    m_toolHitTip.Activate(FALSE);
}

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::SetNotificationTip(CPoint pointClient, UINT nResID)
{
    CString str;
    str.LoadString(nResID);
    SetNotificationTip(pointClient, str);
}

void CPlayBoardView::SetNotificationTip(CPoint pointClient, LPCTSTR pszTip)
{
    ClearNotificationTip();

    TOOLINFO ti;
    m_toolMsgTip.FillInToolInfo(ti, this, ID_TIP_PLAYBOARD_MSG);
    ti.uFlags |= TTF_TRACK;
    ti.lpszText = (LPTSTR)pszTip;

    m_toolMsgTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);

    CPoint pointScreen(pointClient);
    ClientToScreen(&pointScreen);

    m_toolMsgTip.Activate(TRUE);
    m_toolMsgTip.SendMessage(TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
    m_toolMsgTip.SendMessage(TTM_TRACKPOSITION, 0,
        (LPARAM)MAKELONG(static_cast<int16_t>(pointScreen.x), static_cast<int16_t>(pointScreen.y)));

    SetTimer(ID_TIP_MSG_TIMER, MAX_TIP_MSG_TIME, NotificationTipTimeoutHandler);
}

void CPlayBoardView::ClearNotificationTip()
{
    KillTimer(ID_TIP_MSG_TIMER);            // Kill it in case it's still running

    CToolInfo ti;
    m_toolMsgTip.GetToolInfo(ti, this, ID_TIP_PLAYBOARD_MSG);
    m_toolMsgTip.SendMessage(TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
    m_toolMsgTip.DelTool(this, ID_TIP_PLAYBOARD_MSG);
    m_toolMsgTip.Activate(FALSE);
}

void CALLBACK CPlayBoardView::NotificationTipTimeoutHandler(HWND hwnd,
    UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    CPlayBoardView* pView = (CPlayBoardView*)CWnd::FromHandle(hwnd);
    ASSERT(pView != NULL);
    pView->ClearNotificationTip();
}

//////////////////////////////////////////////////////////////////////

CDrawObj* CPlayBoardView::ObjectHitTest(CPoint point)
{
    CDrawList& pDwg = CheckedDeref(m_pPBoard->GetPieceList());
    return pDwg.HitTest(point);
}

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::SelectWithinRect(CRect rctNet, BOOL bInclIntersects)
{
    CGamDoc* pDoc = GetDocument();
    CDrawList& pDwg = CheckedDeref(m_pPBoard->GetPieceList());

    BOOL bPieceSelected = FALSE;
    for (CDrawList::iterator pos = pDwg.begin(); pos != pDwg.end(); ++pos)
    {
        CDrawObj& pObj = **pos;
        if (!m_selList.IsObjectSelected(pObj))
        {
            if ((!bInclIntersects &&
                ((pObj.GetEnclosingRect() | rctNet) == rctNet)) ||
                (bInclIntersects &&
                (!(pObj.GetEnclosingRect() & rctNet).IsRectEmpty())))
            {
                BOOL bOwnedByCurrentPlayer = TRUE;
                if (pObj.GetType() == CDrawObj::drawPieceObj)
                {
                    CPieceObj& pPObj = static_cast<CPieceObj&>(pObj);
                    DWORD dwCurrentPlayer = pDoc->GetCurrentPlayerMask();
                    bOwnedByCurrentPlayer =
                        !pPObj.IsOwned() || pPObj.IsOwnedBy(dwCurrentPlayer) ||
                        m_pPBoard->IsNonOwnerAccessAllowed();
                }

                if (m_pPBoard->GetLocksEnforced() &&
                    (pObj.GetDObjFlags() & dobjFlgLockDown) != 0)
                {
                    // Don't select this object because locks are enforced
                    // and the object is locked.
                    continue;
                }
                // Select if:
                // - In scenario mode. OR (i.e., Scenario designer accesses everything)
                // - Not owned at all OR
                // - Owned by the current player OR
                // - Owned but non-owner access is allowed.
                // (the last three conditions were checked above.)
                if (pDoc->IsScenario() || bOwnedByCurrentPlayer)
                {
                    m_selList.AddObject(pObj, TRUE);
                    bPieceSelected |= pObj.GetType() == CDrawObj::drawPieceObj ||
                        pObj.GetType() == CDrawObj::drawMarkObj;
                }
            }
        }
    }
    if (bPieceSelected)
        NotifySelectListChange();
}

void CPlayBoardView::SelectAllUnderPoint(CPoint point)
{
    CDrawList* pDwg = m_pPBoard->GetPieceList();
    ASSERT(pDwg);

    BOOL bPieceSelected = FALSE;

    std::vector<CB::not_null<CDrawObj*>> selLst;
    pDwg->DrillDownHitTest(point, selLst);

    for (size_t i = size_t(0) ; i < selLst.size() ; ++i)
    {
        CDrawObj& pObj = *selLst[i];
        if (!m_selList.IsObjectSelected(pObj))
        {
            BOOL bOwnedByCurrentPlayer = TRUE;
            if (pObj.GetType() == CDrawObj::drawPieceObj)
            {
                CPieceObj& pPObj = static_cast<CPieceObj&>(pObj);
                DWORD dwCurrentPlayer = GetDocument()->GetCurrentPlayerMask();
                bOwnedByCurrentPlayer =
                    !pPObj.IsOwned() || pPObj.IsOwnedBy(dwCurrentPlayer);
            }

            // Only add to the list if the object is either unlocked or locks
            // are not being enforced or if the piece is owned by someone other
            // than the current player.
            if (bOwnedByCurrentPlayer &&
                (!m_pPBoard->GetLocksEnforced() ||
                 !(pObj.GetDObjFlags() & dobjFlgLockDown)))
            {
                m_selList.AddObject(pObj, TRUE);
                bPieceSelected |= pObj.GetType() == CDrawObj::drawPieceObj ||
                    pObj.GetType() == CDrawObj::drawMarkObj;
            }
        }
    }
    if (bPieceSelected)
        NotifySelectListChange();
}

void CPlayBoardView::SelectAllObjectsInList(const std::vector<CB::not_null<CDrawObj*>>& pLst)
{
    BOOL bPieceSelected = FALSE;
    for (size_t i = size_t(0) ; i < pLst.size() ; ++i)
    {
        CDrawObj& pObj = *pLst[i];
        if (!m_selList.IsObjectSelected(pObj))
        {
            m_selList.AddObject(pObj, TRUE);
            bPieceSelected |= pObj.GetType() == CDrawObj::drawPieceObj ||
                pObj.GetType() == CDrawObj::drawMarkObj;
        }
    }
    if (bPieceSelected)
        NotifySelectListChange();
}

void CPlayBoardView::SelectAllObjectsInTable(const std::vector<CB::not_null<CDrawObj*>>& pTbl)
{
    BOOL bPieceSelected = FALSE;

    for (size_t i = 0; i < pTbl.size(); i++)
    {
        CDrawObj& pObj = *pTbl.at(i);
        if (!m_selList.IsObjectSelected(pObj))
        {
            m_selList.AddObject(pObj, TRUE);
            bPieceSelected |= pObj.GetType() == CDrawObj::drawPieceObj ||
                pObj.GetType() == CDrawObj::drawMarkObj;
        }
    }
    if (bPieceSelected)
        NotifySelectListChange();
}

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::SelectAllMarkers()
{
    CDrawList& pDwg = CheckedDeref(m_pPBoard->GetPieceList());

    m_selList.PurgeList();

    for (CDrawList::iterator pos = pDwg.begin(); pos != pDwg.end(); ++pos)
    {
        CDrawObj& pObj = **pos;
        if (m_pPBoard->GetLocksEnforced() && (pObj.GetDObjFlags() & dobjFlgLockDown))
            continue;           // Ignore this object since it's locked
        if (!m_selList.IsObjectSelected(pObj))
        {
            if (pObj.GetType() == CDrawObj::drawMarkObj)
                m_selList.AddObject(pObj, TRUE);
        }
    }
    NotifySelectListChange();
}

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::SelectMarkersInGroup(size_t nGroup)
{
    CDrawList& pDwg = CheckedDeref(m_pPBoard->GetPieceList());
    CMarkManager& pMgr = CheckedDeref(GetDocument()->GetMarkManager());

    m_selList.PurgeList();

    for (CDrawList::iterator pos = pDwg.begin(); pos != pDwg.end(); ++pos)
    {
        CDrawObj& pObj = **pos;
        if (m_pPBoard->GetLocksEnforced() && (pObj.GetDObjFlags() & dobjFlgLockDown))
            continue;           // Ignore this object since it's locked
        if (!m_selList.IsObjectSelected(pObj))
        {
            if (pObj.GetType() == CDrawObj::drawMarkObj)
            {
                if (pMgr.IsMarkerInGroup(nGroup, static_cast<CMarkObj&>(pObj).m_mid))
                    m_selList.AddObject(pObj, TRUE);
            }
        }
    }
    NotifySelectListChange();
}


//////////////////////////////////////////////////////////////////////

void CPlayBoardView::AddDrawObject(CDrawObj::OwnerPtr pObj)
{
    CDrawList* pDwg = m_pPBoard->GetPieceList();
    ASSERT(pDwg);
    if (pDwg != NULL)
    {
        pDwg->AddToFront(std::move(pObj));
        GetDocument()->SetModifiedFlag();
    }
}

void CPlayBoardView::MoveObjsInSelectList(BOOL bToFront, BOOL bInvalidate)
{
    CDrawList* pDwg = m_pPBoard->GetPieceList();
    ASSERT(pDwg);

    // Loop through the drawing list looking for objects that are
    // selected. Remove and add them to a local list. Then take
    // the objects and add them to the front or back. The reason for
    // the temp list is to maintain ordering of selected objects.
    std::vector<CB::not_null<CDrawObj*>> m_tmpLst;

    for (CSelList::iterator pos = m_selList.begin() ; pos != m_selList.end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pDwg->RemoveObject(*pSel.m_pObj);
        m_tmpLst.push_back(pSel.m_pObj.get());
    }
    if (bToFront)
    {
        for (auto pos = m_tmpLst.begin() ; pos != m_tmpLst.end() ; ++pos)
        {
            pDwg->AddToFront(pos->get());
        }
    }
    else
    {
        for (auto pos = m_tmpLst.rbegin() ; pos != m_tmpLst.rend() ; ++pos)
        {
            pDwg->AddToBack(pos->get());
        }
    }
    if (bInvalidate)
        m_selList.InvalidateList();
    GetDocument()->SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////

CPoint CPlayBoardView::GetWorkspaceDim() const
{
    // First get MM_TEXT size of board for this scaling mode.
    CPoint pnt = (CPoint)m_pPBoard->GetBoard()->GetSize(m_nZoom);

    // Translate to current scaling mode.
    pnt -= (CSize)GetDeviceScrollPosition();
    ClientToWorkspace(pnt);
    return pnt;
}

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::WorkspaceToClient(CPoint& point) const
{
    CPoint dpnt = GetDeviceScrollPosition();
    CSize wsize, vsize;
    m_pPBoard->GetBoard()->GetBoardArray().
        GetBoardScaling(m_nZoom, wsize, vsize);
    if (m_pPBoard->IsBoardRotated180())
        point = CPoint(wsize.cx - point.x, wsize.cy - point.y);
    ScalePoint(point, vsize, wsize);
    point -= (CSize)dpnt;
}

void CPlayBoardView::WorkspaceToClient(CRect& rect) const
{
    CPoint dpnt = GetDeviceScrollPosition();
    CSize wsize, vsize;
    m_pPBoard->GetBoard()->GetBoardArray().
        GetBoardScaling(m_nZoom, wsize, vsize);
    if (m_pPBoard->IsBoardRotated180())
    {
        rect = CRect(wsize.cx - rect.left, wsize.cy - rect.top,
             wsize.cx - rect.right, wsize.cy - rect.bottom);
        rect.NormalizeRect();
    }
    ScaleRect(rect, vsize, wsize);
    rect -= dpnt;
}

void CPlayBoardView::InvalidateWorkspaceRect(const CRect& pRect, BOOL bErase)
{
    CRect rct(pRect);
    WorkspaceToClient(rct);
    rct.InflateRect(1, 1);
    InvalidateRect(&rct, bErase);
}

void CPlayBoardView::ClientToWorkspace(CPoint& point) const
{
    CPoint dpnt = GetDeviceScrollPosition();
    point += (CSize)dpnt;
    CSize wsize, vsize;
    m_pPBoard->GetBoard()->GetBoardArray().
        GetBoardScaling(m_nZoom, wsize, vsize);
    ScalePoint(point, wsize, vsize);
    if (m_pPBoard->IsBoardRotated180())
        point = CPoint(wsize.cx - point.x, wsize.cy - point.y);
}

void CPlayBoardView::ClientToWorkspace(CRect& rect) const
{
    CPoint dpnt = GetDeviceScrollPosition();
    rect += dpnt;
    CSize wsize, vsize;
    m_pPBoard->GetBoard()->GetBoardArray().
        GetBoardScaling(m_nZoom, wsize, vsize);
    ScaleRect(rect, wsize, vsize);
    if (m_pPBoard->IsBoardRotated180())
    {
        rect = CRect(wsize.cx - rect.left, wsize.cy - rect.top,
            wsize.cx - rect.right, wsize.cy - rect.bottom);
        rect.NormalizeRect();
    }
}

//////////////////////////////////////////////////////////////////////

BOOL CPlayBoardView::IsGridizeActive() const
{
    BOOL bGridSnap = m_pPBoard->m_bGridSnap;
    int bControl = GetAsyncKeyState(VK_CONTROL) < 0;
    return !(bControl && bGridSnap || !bControl && !bGridSnap);
}

void CPlayBoardView::GridizeX(long& xPos) const
{
    if (IsGridizeActive())
    {
        xPos = GridizeClosest1000(xPos,
            m_pPBoard->m_xGridSnap, m_pPBoard->m_xGridSnapOff);
    }
}

void CPlayBoardView::GridizeY(long& yPos) const
{
    if (IsGridizeActive())
    {
        yPos = GridizeClosest1000(yPos,
            m_pPBoard->m_yGridSnap, m_pPBoard->m_yGridSnapOff);
    }
}

void CPlayBoardView::LimitPoint(POINT& pPnt) const
{
    const CBoard* pBoard = m_pPBoard->GetBoard();
    if (pPnt.x < 0) pPnt.x = 0;
    if (pPnt.x > pBoard->GetWidth(fullScale))
        pPnt.x = pBoard->GetWidth(fullScale);
    if (pPnt.y < 0) pPnt.y = 0;
    if (pPnt.y > pBoard->GetHeight(fullScale))
        pPnt.y = pBoard->GetHeight(fullScale);
}

void CPlayBoardView::LimitRect(RECT& pRct) const
{
    CRect rct(pRct);
    const CBoard* pBoard = m_pPBoard->GetBoard();

    if (rct.left < 0)
        rct.OffsetRect(-rct.left, 0);
    if (rct.top < 0)
        rct.OffsetRect(0, -rct.top);
    if (rct.right > pBoard->GetWidth(fullScale))
        rct.OffsetRect(pBoard->GetWidth(fullScale) - rct.right, 0);
    if (rct.bottom > pBoard->GetHeight(fullScale))
        rct.OffsetRect(0, pBoard->GetHeight(fullScale) - rct.bottom);
    pRct = rct;
}

BOOL CPlayBoardView::IsRectFullyOnBoard(const RECT& pRct, BOOL* pbXOK, BOOL* pbYOK) const
{
    CRect rct(pRct);
    const CBoard* pBoard = m_pPBoard->GetBoard();
    BOOL bXOK = TRUE;
    BOOL bYOK = TRUE;
    BOOL bOK = TRUE;

    if (rct.left < 0 || rct.right > pBoard->GetWidth(fullScale))
    {
        bXOK = FALSE;
        bOK = FALSE;
    }
    if (rct.top < 0 || rct.bottom > pBoard->GetHeight(fullScale))
    {
        bYOK = FALSE;
        bOK = FALSE;
    }
    if (pbXOK != NULL) *pbXOK = bXOK;
    if (pbYOK != NULL) *pbYOK = bYOK;
    return bOK;
}

void CPlayBoardView::AdjustPoint(CPoint& pnt) const
{
    GridizeX(pnt.x);
    GridizeY(pnt.y);
    LimitPoint(pnt);
}

void CPlayBoardView::AdjustRect(CRect& rct) const
{
    CPoint pnt;
    if (m_pPBoard->m_bGridRectCenters)
        pnt = GetMidRect(rct);
    else
        pnt = rct.TopLeft();

    GridizeX(pnt.x);
    GridizeY(pnt.y);
    LimitPoint(pnt);

    if (m_pPBoard->m_bGridRectCenters)
    {
        if (pnt != GetMidRect(rct))
            rct.OffsetRect(pnt - GetMidRect(rct));
    }
    else
    {
        if (pnt != rct.TopLeft())
            rct.OffsetRect(pnt - rct.TopLeft());
    }
    LimitRect(rct);
}

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::ScrollWorkspacePointIntoView(CPoint point)
{
    CRect rct;
    GetClientRect(&rct);
    if ((rct.Height() < 2 * viewZone) || (rct.Width() < 2 * viewZone))
    {
        // Attempt to center point.
        CenterViewOnWorkspacePoint(point);
        return;
    }
    rct.InflateRect(-viewZone, -viewZone);
    ClientToWorkspace(rct);
    if (rct.PtInRect(point))
        return;                     // Everthing is ok
    CenterViewOnWorkspacePoint(point);
}

void CPlayBoardView::CenterViewOnWorkspacePoint(CPoint point)
{
    WorkspaceToClient(point);
    CRect rct;
    GetClientRect(&rct);
    CPoint pt = GetMidRect(rct);
    CSize size = point - pt;
    CPoint newUpLeft = GetDeviceScrollPosition() + size;
    // If the axis being scrolled is entirely visible then set
    // that scroll position to zero.

    CSize sizeTotal = GetTotalSize();   // Logical is in device units for us
    if (rct.Width() >= sizeTotal.cx)
        newUpLeft.x = 0;
    if (rct.Height() >= sizeTotal.cy)
        newUpLeft.y = 0;

    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

//////////////////////////////////////////////////////////////////////////
// point is in client coords

BOOL CPlayBoardView::CheckAutoScroll(CPoint point)
{
    CRect  rctClient;
    CRect  rct;

    GetClientRect(&rctClient);
    rct = rctClient;
    rct.InflateRect(-scrollZone, -scrollZone);
    rct.NormalizeRect();

    return rctClient.PtInRect(point) && !rct.PtInRect(point);
}

// Returns TRUE if scroll happened
BOOL CPlayBoardView::ProcessAutoScroll(CPoint point)
{
    CRect  rectClient;
    CRect  rect;

    GetClientRect(&rectClient);
    rect = rectClient;
    rect.InflateRect(-scrollZone, -scrollZone);
    rect.NormalizeRect();

    UINT nScrollID = MAKEWORD(-1, -1);
    if (rectClient.PtInRect(point) && !rect.PtInRect(point))
    {
        // Mouse is in the scroll zone....
        // Determine which way to scroll along both X & Y axis
        if (point.x < rect.left)
            nScrollID = MAKEWORD(SB_LINEUP, HIBYTE(nScrollID));
        else if (point.x >= rect.right)
            nScrollID = MAKEWORD(SB_LINEDOWN, HIBYTE(nScrollID));
        if (point.y < rect.top)
            nScrollID = MAKEWORD(LOBYTE(nScrollID), SB_LINEUP);
        else if (point.y >= rect.bottom)
            nScrollID = MAKEWORD(LOBYTE(nScrollID), SB_LINEDOWN);
        ASSERT(nScrollID != MAKEWORD(-1, -1));
        // First check if scroll can happen.
        BOOL bValidScroll = OnScroll(nScrollID, 0, FALSE);
        if (bValidScroll)
        {
            OnScroll(nScrollID, 0, TRUE);
            UpdateWindow();         // Redraw image content.
            return TRUE;
        }
    }
    return FALSE;
}


