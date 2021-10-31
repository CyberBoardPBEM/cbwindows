// ToolPlay.cpp
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
#include    "GMisc.h"
#include    "GamDoc.h"
#include    "FrmMain.h"
#include    "Tile.h"
#include    "ResTbl.h"
#include    "ToolPlay.h"
#include    "PBoard.h"
#include    "VwPbrd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////

const int scrollZone = 16;      // From INI?

/////////////////////////////////////////////////////////////////
// Class variables

std::vector<CPlayTool*> CPlayTool::c_toolLib;          // Tool library

CPoint CPlayTool::c_ptDown;             // Mouse down location
CPoint CPlayTool::c_ptLast;             // Last mouse location

// The playing board tool objects...

static CPSelectTool     s_plySelectTool;
static CPLineTool       s_plyLineTool;
static CPTextBoxTool    s_plyTextTool;
static CPPlotTool       s_plyMovePlotTool;

////////////////////////////////////////////////////////////////////////
// CPlayTool - Basic tool support (abstract class)

CPlayTool::CPlayTool(PToolType eType) :
    m_eToolType(eType)
{
    size_t i = static_cast<size_t>(eType);
    c_toolLib.resize(std::max(i + size_t(1), c_toolLib.size()));
    ASSERT(!c_toolLib[i]);
    c_toolLib[i] = this;
}

CPlayTool& CPlayTool::GetTool(PToolType eToolType)
{
    size_t i = static_cast<size_t>(eToolType);
    ASSERT(i < c_toolLib.size());
    CPlayTool& retval = CheckedDeref(c_toolLib[i]);
    ASSERT(retval.m_eToolType == eToolType);
    return retval;
}

void CPlayTool::OnLButtonDown(CPlayBoardView* pView, UINT nFlags, CPoint point)
{
    pView->SetCapture();

    c_ptDown = point;
    c_ptLast = point;
}

void CPlayTool::OnMouseMove(CPlayBoardView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() == pView)
        c_ptLast = point;
    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void CPlayTool::OnLButtonUp(CPlayBoardView* pView, UINT, CPoint point)
{
    if (CWnd::GetCapture() != pView)
        return;
    ReleaseCapture();
}

////////////////////////////////////////////////////////////////////////
// CPSelectTool - Object Selection/Manipulation tool

void CPSelectTool::OnLButtonDown(CPlayBoardView* pView, UINT nFlags,
    CPoint point)
{
    CSelList* pSLst = pView->GetSelectList();
    CPlayBoard *pPBoard = pView->GetPlayBoard();

    // If a a handle is clicked on, immediately start tracking the
    // resize.
    if ((m_nHandleID = pSLst->HitTestHandles(point)) >= 0)
    {
        StartSizingOperation(pView, nFlags, point);
        return;
    }
    CDrawObj* pObj = pView->ObjectHitTest(point);

    BOOL bOwnedButNotOkToSelect = FALSE;
    if (pObj != NULL && pObj->GetType() == CDrawObj::drawPieceObj)
    {
        CPieceObj* pPObj = (CPieceObj*)pObj;
        DWORD dwCurrentPlayer = pView->GetDocument()->GetCurrentPlayerMask();
        bOwnedButNotOkToSelect = !pView->GetDocument()->IsScenario() &&
            !pPBoard->IsNonOwnerAccessAllowed() &&
            pPObj->IsOwned() && !pPObj->IsOwnedBy(dwCurrentPlayer);
    }

    // Only add to the list if the object is either unlocked or locks
    // are not being enforced or if the piece is owned by someone other
    // than the current player.

    if (pObj == NULL || bOwnedButNotOkToSelect ||
       (pPBoard->GetLocksEnforced() && (pObj->GetDObjFlags() & dobjFlgLockDown)))
    {
        if ((nFlags & MK_SHIFT) == 0)       // Shift click adds to list
            pSLst->PurgeList(TRUE);         // Clear current select list
        // No objects were under the mouse click.
        m_eSelMode = smodeNet;              // Net type selection
        CPlayTool::OnLButtonDown(pView, nFlags, point);
        CClientDC dc(pView);
        pView->OnPrepareScaledDC(&dc, TRUE);
        DrawNetRect(&dc, pView);
        return;
    }
    // Object is under mouse. See if also selected. If not,
    // add to list.
    if (!pSLst->IsObjectSelected(*pObj))
    {
        if ((nFlags & MK_SHIFT) == 0)       // Shift click adds to list
            pSLst->PurgeList(TRUE);         // Clear current select list
        if ((nFlags & MK_CONTROL) != 0)     // Control click drills down
            pView->SelectAllUnderPoint(point);
        else
        {
            pSLst->AddObject(*pObj, TRUE);
            if (pObj->GetType() == CDrawObj::drawPieceObj ||
                    pObj->GetType() == CDrawObj::drawMarkObj)
                pView->NotifySelectListChange();
        }
        CPlayTool::OnLButtonDown(pView, nFlags, point);
        StartDragTimer(pView);

        CRect rct = pSLst->GetEnclosingRect();
        CSize sizeOff = point - rct.TopLeft();
        pSLst->SetMouseOffset(sizeOff);
        return;
    }
    // At this point we know s/he clicked on an object that was
    // already selected. If SHIFT is held we'll remove it from
    // the list. Otherwise, a timer is started and drag tracking
    // wont start until it expires.
    if ((nFlags & MK_SHIFT) != 0)
    {
        pSLst->RemoveObject(*pObj, TRUE);
        return;
    }
    CPlayTool::OnLButtonDown(pView, nFlags, point);

    pSLst->SetSnapReferenceObject(pObj);
    CRect rct = pSLst->GetSnapReferenceRect();
    CSize sizeOff = point - rct.TopLeft();
    pSLst->SetMouseOffset(sizeOff);

    StartDragTimer(pView);
}

void CPSelectTool::OnMouseMove(CPlayBoardView* pView, UINT nFlags, CPoint point)
{
    CSelList* pSLst = pView->GetSelectList();

    if (CWnd::GetCapture() != pView)
        return;

    if (m_eSelMode != smodeNormal && m_eSelMode != smodeMove)
    {
        // Autoscroll initiate possibility processing (sounds like a
        // scifi flic). Autoscroll is enabled when the mouse is captured
        // and the mouse is outside of the client area.
        CRect rct;
        CPoint pt;
        GetCursorPos(&pt);
        pView->ScreenToClient(&pt);
        pView->GetClientRect(&rct);
        if (rct.PtInRect(pt))           // In client area
        {
            rct.InflateRect(-scrollZone, -scrollZone);
            rct.NormalizeRect();        // Just in case client is too small
            if (!rct.PtInRect(pt))
            {
                // It's in the scroll zone
                if (m_nTimerID == uintptr_t(0))        // Only start if not scrolling
                    StartScrollTimer(pView);
            }
            else
                KillScrollTimer(pView);
        }
        else
            KillScrollTimer(pView);
    }

    // If we get here, the mouse has been captured. Check if
    // we are doing a "net select".
    if (m_eSelMode == smodeNet)
    {
        CClientDC dc(pView);
        pView->OnPrepareScaledDC(&dc, TRUE);
        DrawNetRect(&dc, pView);            // Erase previous position
        CPlayTool::OnMouseMove(pView, nFlags, point); // Update position
        DrawNetRect(&dc, pView);            // Draw new position rect
        return;
    }
    // If object(s) are being moved or sized, removed last tracking
    // images, adjust the object's dimensions or postions and redraw
    // tracking image.
    if (m_eSelMode == smodeMove)
    {
        pView->WorkspaceToClient(point);
        DoDragDrop(pView, point);
        return;
    }
    else if (m_eSelMode == smodeSizing)
    {
        pView->AdjustPoint(point);
        if (point == c_ptLast)
            return;

        CClientDC dc(pView);
        pView->OnPrepareScaledDC(&dc, TRUE);

        pSLst->DrawTracker(dc, trkSizing); // Erase previous tracker

        MoveSelections(pSLst, point);

        pSLst->DrawTracker(dc, trkSizing); // Erase previous tracker
    }
    c_ptLast = point;               // Save new 'last' position
}

void CPSelectTool::OnLButtonUp(CPlayBoardView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() == pView)
    {
        if (m_eSelMode == smodeNet)
        {
            CClientDC dc(pView);
            pView->OnPrepareScaledDC(&dc, TRUE);
            DrawNetRect(&dc, pView);            // Erase previous position
            // If the control key is down when button was released, fields
            // that intersect the select rect will selected. Otherwise only
            // those fields that are entirely within the select rect
            // will be selected.
            CRect rect(c_ptDown.x, c_ptDown.y, c_ptLast.x, c_ptLast.y);
            rect.NormalizeRect();
            pView->SelectWithinRect(rect, (nFlags & MK_CONTROL) != 0);
            CSelList* pSLst = pView->GetSelectList();
            pSLst->InvalidateListHandles();
        }
        else if (m_eSelMode != smodeNormal)
        {
            CSelList* pSLst = pView->GetSelectList();
            if (m_eSelMode == smodeMove)
            {
                CPoint pnt = point;
                pSLst->SetTrackingMode(trkSelected);
                pView->WorkspaceToClient(pnt);
                DoDragDropEnd(pView, pnt);
            }
            else
            {
                pSLst->SetTrackingMode(trkSelected);
                pSLst->UpdateObjects(TRUE);
                pSLst->InvalidateListHandles();
            }
        }
    }
    m_eSelMode = smodeNormal;
    KillDragTimer(pView);           // Make sure timers are released
    KillScrollTimer(pView);
    CPlayTool::OnLButtonUp(pView, nFlags, point);
}

void CPSelectTool::OnTimer(CPlayBoardView* pView, uintptr_t nIDEvent)
{
    if (CWnd::GetCapture() != pView)
    {
        m_eSelMode = smodeNormal;
        KillDragTimer(pView);
        KillScrollTimer(pView);
        return;
    }
    if (m_eSelMode == smodeNormal)
    {
        CSelList* pSLst = pView->GetSelectList();

        // Mouse is captured and no particular drag operation
        // is underway. Therefore we want a move that draws
        // object outlines.
        m_eSelMode = smodeMove;
        KillDragTimer(pView);

        CClientDC dc(pView);
        pView->OnPrepareScaledDC(&dc, TRUE);
        pSLst->DrawTracker(dc, trkSelected);   // Turn off handles

        CPoint point;
        GetCursorPos(&point);
        pView->ScreenToClient(&point);
        pView->ClientToWorkspace(point);

        pSLst->SetTrackingMode(trkMoving);
        DoDragDropStart(pView);
        pView->WorkspaceToClient(point);
        DoDragDrop(pView, point);
    }
    else if (m_eSelMode != smodeMove)
    {
        if (!ProcessAutoScroll(pView))
            KillScrollTimer(pView);
    }
}

void CPSelectTool::OnLButtonDblClk(CPlayBoardView* pView, UINT nFlags,
    CPoint point)
{
    // Normal DblClk opens some property view of the selected
    // object. Only recognized if only one selection is active.
    CPlayTool::OnLButtonDblClk(pView, nFlags, point);
    pView->GetSelectList()->Open();
}

BOOL CPSelectTool::OnSetCursor(CPlayBoardView* pView, UINT nHitTest)
{
    // Process only within the client area
    if (nHitTest != HTCLIENT)
        return FALSE;

    // Convert cursor position to document coordinates
    CPoint point;
    GetCursorPos(&point);
    pView->ScreenToClient(&point);
    pView->ClientToWorkspace(point);

    // Check for movement through handle areas. Set the
    // cursor to the appropriate shape.
    CSelList* pSLst = pView->GetSelectList();
    if (pSLst->IsSingleSelect())
    {
        // Check if cursor is over a handle. If it is,
        // get handle cursor.
        CSelection& pSelObj = *pSLst->front();
        int nHandle = pSelObj.HitTestHandles(point);
        if (nHandle >= 0)
        {
            SetCursor(pSelObj.GetHandleCursor(nHandle));
            return TRUE;
        }
    }
    return FALSE;       // Show default cursor
}

void CPSelectTool::StartSizingOperation(CPlayBoardView* pView, UINT nFlags,
    CPoint point, int nHandleID)
{
    CSelList* pSLst = pView->GetSelectList();
    if (nHandleID != -1)
        m_nHandleID = nHandleID;
    m_eSelMode = smodeSizing;
    CPlayTool::OnLButtonDown(pView, nFlags, point);
    CClientDC dc(pView);
    pView->OnPrepareScaledDC(&dc, TRUE);
    pSLst->DrawTracker(dc, trkSizing);
}

void CPSelectTool::DrawSelectionRect(CDC* pDC, CRect* pRct)
{
    CPen pen;
    pen.CreateStockObject(WHITE_PEN);
    CPen* pPrvPen = pDC->SelectObject(&pen);
    int nPrvROP2 = pDC->SetROP2(R2_XORPEN);

    pDC->MoveTo(pRct->TopLeft());
    pDC->LineTo(pRct->right, pRct->top);
    pDC->LineTo(pRct->BottomRight());
    pDC->LineTo(pRct->left, pRct->bottom);
    pDC->LineTo(pRct->TopLeft());

    pDC->SetROP2(nPrvROP2);
    pDC->SelectObject(pPrvPen);
}

void CPSelectTool::DrawNetRect(CDC* pDC, CPlayBoardView* pView)
{
    CRect rect(c_ptDown.x, c_ptDown.y, c_ptLast.x, c_ptLast.y);
    rect.NormalizeRect();
    DrawSelectionRect(pDC, &rect);
}

BOOL CPSelectTool::ProcessAutoScroll(CPlayBoardView* pView)
{
    CPoint point;
    CRect  rectClient;
    CRect  rect;

    GetCursorPos(&point);
    pView->ScreenToClient(&point);
    pView->GetClientRect(&rectClient);
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
        BOOL bValidScroll = pView->OnScroll(nScrollID, 0, FALSE);
        if (bValidScroll)
        {
            CSelList* pSLst = pView->GetSelectList();
            pView->ClientToWorkspace(point);

            CClientDC dc(pView);
            pView->OnPrepareScaledDC(&dc, TRUE);

            if (m_eSelMode == smodeNet)
            {
                // Erase previous position
                CRect rect(c_ptDown.x, c_ptDown.y, c_ptLast.x, c_ptLast.y);
                rect.NormalizeRect();
                DrawSelectionRect(&dc, &rect);
            }
            else
                pSLst->DrawTracker(dc);    // Turn off tracker

            pView->OnScroll(nScrollID, 0, TRUE);
            pView->UpdateWindow();          // Redraw image content.

            AdjustPoint(pView, point);

            MoveSelections(pSLst, point);   // Offset the tracking data
            c_ptLast = point;               // Save new 'last' position

            pView->OnPrepareScaledDC(&dc, TRUE);
            if (m_eSelMode == smodeNet)
            {
                GetCursorPos(&point);
                pView->ScreenToClient(&point);
                pView->ClientToWorkspace(point);
                c_ptLast = point;            // Set new 'last' position
                // Draw updated net rect
                CRect rect(c_ptDown.x, c_ptDown.y, c_ptLast.x, c_ptLast.y);
                rect.NormalizeRect();
                DrawSelectionRect(&dc, &rect);
            }
            else
            {
                MoveSelections(pSLst, point);// Offset the tracking data
                c_ptLast = point;            // Save new 'last' position
                pSLst->DrawTracker(dc);    // Turn off tracker
            }
            return TRUE;
        }
    }
    return FALSE;
}

void CPSelectTool::MoveSelections(CSelList *pSLst, CPoint point)
{
    if (m_eSelMode == smodeMove)
    {
        CPoint ptDelta = (CPoint)(point - c_ptLast);
        pSLst->Offset(ptDelta);
    }
    else
        pSLst->MoveHandle(m_nHandleID, point);
}

BOOL CPSelectTool::AdjustPoint(CPlayBoardView* pView, CPoint& point)
{
    pView->AdjustPoint(point);
    if (point == c_ptLast)
        return FALSE;
    if (m_eSelMode == smodeMove)
    {
        CRect rct = pView->GetSelectList()->GetEnclosingRect();
        CPoint pnt = pView->GetWorkspaceDim();
        if (rct.left + point.x - c_ptLast.x < 0)        // Clamp
            point.x = c_ptLast.x - rct.left;
        if (rct.top + point.y - c_ptLast.y < 0)         // Clamp
            point.y = c_ptLast.y - rct.top;
        if (rct.right + point.x - c_ptLast.x > pnt.x)   // Clamp
            point.x = pnt.x - (rct.right - c_ptLast.x);
        if (rct.bottom + point.y - c_ptLast.y > pnt.y)  // Clamp
            point.y = pnt.y - (rct.bottom - c_ptLast.y);
    }
    return TRUE;
}

void CPSelectTool::StartDragTimer(CPlayBoardView* pView)
{
    m_nTimerID = pView->SetTimer(timerIDSelectDelay,
        timerSelDelay, NULL);
}

void CPSelectTool::KillDragTimer(CPlayBoardView* pView)
{
    if (m_nTimerID != uintptr_t(0))
    {
        pView->KillTimer(m_nTimerID);
        m_nTimerID = uintptr_t(0);
    }
}

void CPSelectTool::StartScrollTimer(CPlayBoardView* pView)
{
    m_nTimerID = pView->SetTimer(timerIDAutoScroll, timerAutoScroll, NULL);
}

void CPSelectTool::KillScrollTimer(CPlayBoardView* pView)
{
    if (m_nTimerID != uintptr_t(0))
    {
        pView->KillTimer(m_nTimerID);
        m_nTimerID = uintptr_t(0);
    }
}

////////////////////////////////////////////////////////////////////////
// Note: The CSelList should have had the mouse offset value set at
//      this time.
void CPSelectTool::DoDragDropStart(CPlayBoardView* pView)
{
    m_di.m_dragType = DRAG_SELECTLIST;
    m_di.GetSubInfo<DRAG_SELECTLIST>().m_selectList = pView->GetSelectList();
    m_di.GetSubInfo<DRAG_SELECTLIST>().m_gamDoc = pView->GetDocument();
    m_di.m_hcsrSuggest = g_res.hcrDragTile;

    m_hLastWnd = NULL;
}

void CPSelectTool::DoDragDrop(CPlayBoardView* pView, CPoint pntClient)
{
    CPoint pnt = pntClient;
    pView->ClientToScreen(&pnt);
    CWnd* pWnd = GetWindowFromPoint(pnt);

    HWND hWnd = pWnd ? pWnd->m_hWnd : NULL; // Get actual window handle
    if (hWnd != m_hLastWnd)
    {
        if (m_hLastWnd != NULL)
        {
            // Signal previous window we are leaving them
            CWnd* pLstWnd = CWnd::FromHandle(m_hLastWnd);
            pLstWnd->SendMessage(WM_DRAGDROP, phaseDragExit,
                (LPARAM)(LPVOID)&m_di);
        }
        // Signal new window we have entered it.
        if (pWnd != NULL)
        {
            pWnd->SendMessage(WM_DRAGDROP, phaseDragEnter,
                (LPARAM)(LPVOID)&m_di);
        }
    }
    HCURSOR hCursor = NULL;
    if (hWnd != NULL)
    {
        m_di.m_point = pntClient;
        pView->ClientToScreen(&m_di.m_point);   // Move point into new coord system
        pWnd->ScreenToClient(&m_di.m_point);
        hCursor = (HCURSOR)pWnd->SendMessage(WM_DRAGDROP, phaseDragOver,
                (LPARAM)(LPVOID)&m_di);
    }
    m_hLastWnd = hWnd;

    if (hCursor)
        SetCursor(hCursor);
    else
        SetCursor(g_res.hcrNoDrop);
}

void CPSelectTool::DoDragDropEnd(CPlayBoardView* pView, CPoint pntClient)
{
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    CPoint pnt = pntClient;
    pView->ClientToScreen(&pnt);
    CWnd* pWnd = GetWindowFromPoint(pnt);
    if (pWnd == NULL)
        return;
    m_di.m_point = pntClient;
    pView->ClientToScreen(&m_di.m_point);
    pWnd->ScreenToClient(&m_di.m_point);

    pWnd->SendMessage(WM_DRAGDROP, phaseDragDrop, (LPARAM)(LPVOID)&m_di);
}

////////////////////////////////////////////////////////////////////////
// CPShapeTool - tool used to create rectangles.

void CPShapeTool::OnLButtonDown(CPlayBoardView* pView, UINT nFlags, CPoint point)
{
    CSelList* pSLst = pView->GetSelectList();
    pSLst->PurgeList(TRUE);         // Clear current select list
    int nDragHandle;
    pView->AdjustPoint(point);
    m_pObj = CreateDrawObj(pView, point, nDragHandle);
    pSLst->AddObject(*m_pObj, TRUE);
    s_plySelectTool.StartSizingOperation(pView, nFlags, point, nDragHandle);
}

void CPShapeTool::OnLButtonUp(CPlayBoardView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() != pView)
        return;
    s_plySelectTool.OnLButtonUp(pView, nFlags, point);
    pView->GetSelectList()->PurgeList(TRUE); // Clear current select list
    if (!IsEmptyObject())
        pView->AddDrawObject(m_pObj);
    else
    {
        delete m_pObj;
        m_pObj = NULL;
    }
}

void CPShapeTool::OnMouseMove(CPlayBoardView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() == pView)
        s_plySelectTool.OnMouseMove(pView, nFlags, point);
}

void CPShapeTool::OnTimer(CPlayBoardView* pView, uintptr_t nIDEvent)
{
    s_plySelectTool.OnTimer(pView, nIDEvent);
}

BOOL CPShapeTool::OnSetCursor(CPlayBoardView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CPLineTool - tool used to create lines

CDrawObj* CPLineTool::CreateDrawObj(CPlayBoardView* pView, CPoint point,
    int& nHandle)
{
    CLine* pObj = new CLine;
    pObj->SetLine(point.x, point.y, point.x, point.y);
    pObj->SetForeColor(pView->GetLineColor());
    pObj->SetLineWidth(pView->GetLineWidth());
    nHandle = hitPtB;
    return pObj;
}

BOOL CPLineTool::IsEmptyObject()
{
    CRect rct = ((CLine*)m_pObj)->GetRect();
    return rct.Width() < 3 && rct.Height() < 3;
}

////////////////////////////////////////////////////////////////////////
// CPTextBoxTool - Text box drawing object tool

void CPTextBoxTool::OnLButtonDown(CPlayBoardView* pView, UINT nFlags,
    CPoint point)
{
//  pView->DoCreateTextDrawingObject(point);
}

BOOL CPTextBoxTool::OnSetCursor(CPlayBoardView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CPPlotTool - move plot tool

void CPPlotTool::OnLButtonDown(CPlayBoardView* pView, UINT nFlags,
    CPoint point)
{
    CGamDoc* pDoc = pView->GetDocument();
    CPlayBoard* pPBrd = pView->GetPlayBoard();
    ASSERT(pPBrd->GetPlotMoveMode());

    if (pPBrd->m_bSnapMovePlot)
        pView->AdjustPoint(point);      // Keep on the grid (if enabled)

    CPoint pntPrev = pPBrd->GetPrevPlotPoint();

    if (pntPrev == CPoint(-1, -1))
    {
        // Draw a line for each piece to the opening location
        std::vector<CB::not_null<CDrawObj*>> listObjs;
        pView->GetSelectList()->LoadTableWithObjectPtrs(listObjs, CSelList::otAll, FALSE);
        for (auto pos = listObjs.begin() ; pos != listObjs.end() ; ++pos)
        {
            CDrawObj& pObj = **pos;
            ASSERT(pObj.GetType() == CDrawObj::drawPieceObj ||
                pObj.GetType() == CDrawObj::drawMarkObj);
            CRect rct = pObj.GetRect();
            CPoint pnt = GetMidRect(rct);
            pDoc->IndicateBoardPlotLine(pPBrd, pnt, point);
        }
    }
    else
        pDoc->IndicateBoardPlotLine(pPBrd, pntPrev, point);
    pPBrd->SetPrevPlotPoint(point);
}

BOOL CPPlotTool::OnSetCursor(CPlayBoardView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}

