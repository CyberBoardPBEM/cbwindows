// ToolObjs.cpp
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
#include    "Gm.h"
#include    "GmDoc.h"
#include    "FrmMain.h"
#include    "Tile.h"
#include    "ResTbl.h"
#include    "ToolObjs.h"
#include    "VwEdtbrd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////

const int scrollZone = 8;                   //From INI?

/////////////////////////////////////////////////////////////////
// Class variables

std::vector<CTool*> CTool::c_toolLib;          // Tool library

CPoint CTool::c_ptDown;             // Mouse down location
CPoint CTool::c_ptLast;             // Last mouse location

// The tool objects...

static CSelectTool      s_selectTool;
static CTextTool        s_textTool;
static CRectTool        s_rectTool;
static CEllipseTool     s_ellipseTool;
static CLineTool        s_lineTool;
static CPolyTool        s_polyTool;
static CTileTool        s_tileTool;
static CColorPickupTool s_colorPickupTool;
static CCellPaintTool   s_paintCellTool;
static CPaintTool       s_paintTool;
static CCellEraserTool  s_eraserCellTool;

////////////////////////////////////////////////////////////////////////
// CTool - Basic tool support (abstract class)

CTool::CTool(ToolType eType) :
    m_eToolType(eType)
{
    size_t i = static_cast<size_t>(eType);
    c_toolLib.resize(std::max(i + size_t(1), c_toolLib.size()));
    ASSERT(!c_toolLib[i]);
    c_toolLib[i] = this;
}

CTool& CTool::GetTool(ToolType eToolType)
{
    size_t i = static_cast<size_t>(eToolType);
    ASSERT(i < c_toolLib.size());
    CTool& retval = CheckedDeref(c_toolLib[i]);
    ASSERT(retval.m_eToolType == eToolType);
    return retval;
}

void CTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    pView.CaptureMouse();

    c_ptDown = point;
    c_ptLast = point;
}

void CTool::OnMouseMove(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    if (pView.HasCapture())
        c_ptLast = point;
//    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void CTool::OnLButtonUp(CBrdEditView& pView, UINT, CPoint point)
{
    if (!pView.HasCapture())
        return;
    pView.ReleaseMouse();
}

////////////////////////////////////////////////////////////////////////
// CSelectTool - Object Selection/Manipulation tool

void CSelectTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags,
    CPoint point)
{
    CSelList& pSLst = pView.GetSelectList();
    // If a a handle is clicked on, immediately start tracking the
    // resize.
    if ((m_nHandleID = pSLst.HitTestHandles(CB::Convert(point))) >= 0)
    {
        StartSizingOperation(pView, nFlags, point);
        return;
    }
    wxASSERT(!"TODO:");
#if 0
    CDrawObj* pObj = pView.ObjectHitTest(point);
    if (pObj == NULL)
    {
        if ((nFlags & MK_SHIFT) == 0)       // Shift click adds to list
            pSLst.PurgeList(TRUE);         // Clear current select list
        // No objects were under the mouse click.
        m_eSelMode = smodeNet;              // Net type selection
        CTool::OnLButtonDown(pView, nFlags, point);
        CClientDC dc(&pView);
        pView.OnPrepareScaledDC(dc);
        DrawNetRect(dc, pView);
        return;
    }
    // Object is under mouse. See if also selected. If not,
    // add to list.
    if (!pSLst.IsObjectSelected(*pObj))
    {
        if ((nFlags & MK_SHIFT) == 0)       // Shift click adds to list
            pSLst.PurgeList(TRUE);         // Clear current select list
        if ((nFlags & MK_CONTROL) != 0)     // Control click drills down
            pView.SelectAllUnderPoint(point);
        else
            pSLst.AddObject(*pObj, TRUE);
        CTool::OnLButtonDown(pView, nFlags, point);
        StartDragTimer(pView);
        if (pSLst.IsMultipleSelects())
        {
            // Setup a trigger zone. If the mouse is moved out of
            // this zone before the drag time expires, the move is
            // represented by a perimeter rectangle. Otherwise, it
            // is shown using proxy object tracker images.
            m_rectMultiBorder += point;
            m_rectMultiBorder.InflateRect(2, 2);
        }
        return;
    }
    // At this point we know s/he clicked on an object that was
    // already selected. If SHIFT is held we'll remove it from
    // the list. Otherwise, a timer is started and drag tracking
    // wont start until it expires.
    if ((nFlags & MK_SHIFT) != 0)
    {
        pSLst.RemoveObject(*pObj, TRUE);
        return;
    }
    CTool::OnLButtonDown(pView, nFlags, point);
    if (pSLst.IsMultipleSelects())
    {
        // Setup a trigger zone. If the mouse is moved out of
        // this zone before the drag time expires, the move is
        // represented by a perimeter rectangle. Otherwise, it
        // is shown using proxy object tracker images.
        m_rectMultiBorder += point;
        m_rectMultiBorder.InflateRect(2, 2);
    }
    StartDragTimer(pView);
#endif
}

void CSelectTool::OnMouseMove(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    CSelList& pSLst = pView.GetSelectList();

    if (!pView.HasCapture())
        return;

    wxASSERT(!"TODO:");
#if 0
    if (m_eSelMode != smodeNormal)
    {
        // Autoscroll initiate possibility processing (sounds like a
        // scifi flic). Autoscroll is enabled when the mouse is captured
        // and the mouse is outside of the client area.
        CRect rct;
        CPoint pt;
        GetCursorPos(&pt);
        pView.ScreenToClient(&pt);
        pView.GetClientRect(&rct);
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
    else if (!m_rectMultiBorder.IsRectNull())
    {
        // Mode is normal and a trigger rectangle is set.
        // The trigger rectangle is only set if multiple objects are
        // selected.
        if (!m_rectMultiBorder.PtInRect(point))
        {
            // Force the point onto the snap grid.
            pView.AdjustPoint(point);

            KillDragTimer(pView);
            m_rectMultiBorder = pSLst.GetEnclosingRect();

            // Force the rectangle onto the grid.
            pView.AdjustRect(m_rectMultiBorder);
            pSLst.Offset((CPoint)(m_rectMultiBorder.TopLeft() -
                pSLst.GetEnclosingRect().TopLeft()));

            m_eSelMode = smodeMove;             // Now in move mode.
            pSLst.SetTrackingMode(trkMoving);
            CClientDC dc(&pView);
            pView.OnPrepareScaledDC(dc);
            DrawSelectionRect(dc, &m_rectMultiBorder);
            c_ptLast = point;                   // Save new 'last' position
            return;
        }
    }

    // If we get here, the mouse has been captured. Check if
    // we are doing a "net select".
    if (m_eSelMode == smodeNet)
    {
        CClientDC dc(&pView);
        pView.OnPrepareScaledDC(dc);
        DrawNetRect(dc, pView);            // Erase previous position
        CTool::OnMouseMove(pView, nFlags, point); // Update position
        DrawNetRect(dc, pView);            // Draw new position rect
        return;
    }
    // If object(s) are being moved or sized, removed last tracking
    // images, adjust the object's dimensions or postions and redraw
    // tracking image.
    if (m_eSelMode == smodeMove || m_eSelMode == smodeSizing)
    {
        pView.AdjustPoint(point);
        if (point == c_ptLast)
            return;
        if (m_eSelMode == smodeMove)
        {
            CRect rct = pSLst.GetEnclosingRect();
            CPoint pnt = pView.GetWorkspaceDim();
            if (rct.left + point.x - c_ptLast.x < 0)        // Clamp
                point.x = c_ptLast.x - rct.left;
            if (rct.top + point.y - c_ptLast.y < 0)         // Clamp
                point.y = c_ptLast.y - rct.top;
            if (rct.right + point.x - c_ptLast.x > pnt.x)   // Clamp
                point.x = pnt.x - (rct.right - c_ptLast.x);
            if (rct.bottom + point.y - c_ptLast.y > pnt.y)  // Clamp
                point.y = pnt.y - (rct.bottom - c_ptLast.y);
        }

        TrackMode eTrkMode = m_eSelMode == smodeMove ? trkMoving : trkSizing;
        CClientDC dc(&pView);
        pView.OnPrepareScaledDC(dc);

        if (m_eSelMode == smodeMove && !m_rectMultiBorder.IsRectNull())
            DrawSelectionRect(dc, m_rectMultiBorder);
        else
            pSLst.DrawTracker(dc, eTrkMode); // Erase previous tracker

        MoveSelections(pSLst, point);

        if (m_eSelMode == smodeMove && !m_rectMultiBorder.IsRectNull())
            DrawSelectionRect(dc, m_rectMultiBorder);
        else
            pSLst.DrawTracker(dc, eTrkMode); // Erase previous tracker
    }
    c_ptLast = point;               // Save new 'last' position
#endif
}

void CSelectTool::OnLButtonUp(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    if (pView.HasCapture())
    {
        if (m_eSelMode == smodeNet)
        {
            CClientDC dc(&pView);
            pView.OnPrepareScaledDC(dc);
            DrawNetRect(dc, pView);            // Erase previous position
            // If the control key is down when button was released, fields
            // that intersect the select rect will selected. Otherwise only
            // those fields that are entirely within the select rect
            // will be selected.
            CRect rect(c_ptDown.x, c_ptDown.y, c_ptLast.x, c_ptLast.y);
            rect.NormalizeRect();
            pView.SelectWithinRect(rect, (nFlags & MK_CONTROL) != 0);
            CSelList& pSLst = pView.GetSelectList();
            pSLst.InvalidateListHandles();
        }
        else if (m_eSelMode != smodeNormal)
        {
            // Instruct the selected objects to resize or reposition
            // themselves based on the new data within the selection
            // objects.
            if (m_eSelMode == smodeMove && !m_rectMultiBorder.IsRectNull())
            {
                CClientDC dc(&pView);
                pView.OnPrepareScaledDC(dc);
                DrawSelectionRect(dc, &m_rectMultiBorder);
            }
            CSelList& pSLst = pView.GetSelectList();
            pSLst.SetTrackingMode(trkSelected);
            pSLst.UpdateObjects(TRUE);
            pSLst.InvalidateListHandles();
            pView.GetDocument().SetModifiedFlag();
        }
    }
    m_eSelMode = smodeNormal;
    KillDragTimer(pView);           // Make sure timers are released
    KillScrollTimer(pView);
#endif
    m_rectMultiBorder.SetRectEmpty();// Make sure trigger rect is empty
    CTool::OnLButtonUp(pView, nFlags, point);
}

void CSelectTool::OnTimer(CBrdEditView& pView, uintptr_t nIDEvent)
{
    wxASSERT(!"TODO:");
#if 0
    if (!pView.HasCapture())
    {
        m_eSelMode = smodeNormal;
        KillDragTimer(pView);
        KillScrollTimer(pView);
        m_rectMultiBorder.SetRectEmpty();// Make sure trigger rect is empty
        return;
    }
    if (m_eSelMode == smodeNormal)
    {
        CSelList& pSLst = pView.GetSelectList();

        // Mouse is captured and no particular drag operation
        // is underway. Therefore we want a move that draws
        // object outlines.
        m_eSelMode = smodeMove;
        m_rectMultiBorder.SetRectEmpty(); // Make sure trigger rect is empty
        KillDragTimer(pView);

        // --Draw initial tracker(s) so mouse move processing can
        // pick up the work.
        CClientDC dc(&pView);
        pView.OnPrepareScaledDC(dc);
        pSLst.DrawTracker(dc, trkSelected);   // Turn off handles

        // If the snap grid is active. Force the enclosing rect onto
        // the snap grid. Also force the previously saved mouse points
        // to the snap grid.
        CRect rct = pSLst.GetEnclosingRect();
        pView.AdjustRect(rct);
        pSLst.Offset((CPoint)(rct.TopLeft() -
            pSLst.GetEnclosingRect().TopLeft()));
        pView.AdjustPoint(c_ptLast);
        pView.AdjustPoint(c_ptDown);

        // Move the tracking image to its new location and draw it.
        CPoint ptDelta = (CPoint)(c_ptLast - c_ptDown);
        pSLst.Offset(ptDelta);

        pSLst.DrawTracker(dc, trkMoving);
    }
    else
    {
        if (!ProcessAutoScroll(pView))
            KillScrollTimer(pView);
    }
#endif
}

void CSelectTool::OnLButtonDblClk(CBrdEditView& pView, UINT nFlags,
    CPoint point)
{
    // "Normal" DblClk opens properties, OLE server, etc...
    CSelList& pSLst = pView.GetSelectList();
    if (pSLst.IsSingleSelect())
        pView.GetSelectList().Open();
//  CTool::OnLButtonDblClk(pView, nFlags, point);
}

BOOL CSelectTool::OnSetCursor(const CBrdEditView& pView, UINT nHitTest) const
{
    wxASSERT(!"TODO:");
#if 0
    // Process only within the client area
    if (nHitTest != HTCLIENT)
        return FALSE;

    // Convert cursor position to document coordinates
    CPoint point;
    GetCursorPos(&point);
    pView.ScreenToClient(&point);
    pView.ClientToWorkspace(point);

    // Check for movement through handle areas. Set the
    // cursor to the appropriate shape.
    const CSelList& pSLst = pView.GetSelectList();
    if (pSLst.IsSingleSelect())
    {
        // Check if cursor is over a handle. If it is,
        // get handle cursor.
        const CSelection& pSelObj = *pSLst.front();
        int nHandle = pSelObj.HitTestHandles(point);
        if (nHandle >= 0)
        {
            SetCursor(pSelObj.GetHandleCursor(nHandle));
            return TRUE;
        }
    }
#endif
    return FALSE;       // Show default cursor
}

void CSelectTool::StartSizingOperation(CBrdEditView& pView, UINT nFlags,
    CPoint point, int nHandleID)
{
    CSelList& pSLst = pView.GetSelectList();
    if (nHandleID != -1)
        m_nHandleID = nHandleID;
    m_eSelMode = smodeSizing;
    wxASSERT(!"TODO:");
#if 0
    CTool::OnLButtonDown(pView, nFlags, point);
    CClientDC dc(&pView);
    pView.OnPrepareScaledDC(dc);
    pSLst.DrawTracker(dc, trkSizing);
#endif
}

void CSelectTool::DrawSelectionRect(CDC& pDC, const CRect& pRct) const
{
    CPen pen;
    pen.CreateStockObject(WHITE_PEN);
    CPen* pPrvPen = pDC.SelectObject(&pen);
    int nPrvROP2 = pDC.SetROP2(R2_XORPEN);

    pDC.MoveTo(pRct.TopLeft());
    pDC.LineTo(pRct.right, pRct.top);
    pDC.LineTo(pRct.BottomRight());
    pDC.LineTo(pRct.left, pRct.bottom);
    pDC.LineTo(pRct.TopLeft());

    pDC.SetROP2(nPrvROP2);
    pDC.SelectObject(pPrvPen);
}

void CSelectTool::DrawNetRect(CDC& pDC, CBrdEditView& /*pView*/) const
{
    CRect rect(c_ptDown.x, c_ptDown.y, c_ptLast.x, c_ptLast.y);
    rect.NormalizeRect();
    DrawSelectionRect(pDC, rect);
}

BOOL CSelectTool::ProcessAutoScroll(CBrdEditView& pView)
{
    wxASSERT(!"TODO:");
#if 0
    CPoint point;
    CRect  rectClient;
    CRect  rect;

    GetCursorPos(&point);
    pView.ScreenToClient(&point);
    pView.GetClientRect(&rectClient);
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
        BOOL bValidScroll = pView.OnScroll(nScrollID, 0, FALSE);
        if (bValidScroll)
        {
            CSelList& pSLst = pView.GetSelectList();
            pView.ClientToWorkspace(point);

            CClientDC dc(&pView);
            pView.OnPrepareScaledDC(dc);

            if (m_eSelMode == smodeNet)
            {
                // Erase previous position
                CRect rect(c_ptDown.x, c_ptDown.y, c_ptLast.x, c_ptLast.y);
                rect.NormalizeRect();
                DrawSelectionRect(dc, rect);
            }
            else if (m_eSelMode == smodeMove && !m_rectMultiBorder.IsRectNull())
                DrawSelectionRect(dc, m_rectMultiBorder);
            else
                pSLst.DrawTracker(dc);    // Turn off tracker

            pView.OnScroll(nScrollID, 0, TRUE);
            pView.UpdateWindow();          // Redraw image content.

            AdjustPoint(pView, point);

            MoveSelections(pSLst, point);   // Offset the tracking data
            c_ptLast = point;               // Save new 'last' position

            pView.OnPrepareScaledDC(dc);
            if (m_eSelMode == smodeNet)
            {
                GetCursorPos(&point);
                pView.ScreenToClient(&point);
                pView.ClientToWorkspace(point);
                c_ptLast = point;            // Set new 'last' position
                // Draw updated net rect
                CRect rect(c_ptDown.x, c_ptDown.y, c_ptLast.x, c_ptLast.y);
                rect.NormalizeRect();
                DrawSelectionRect(dc, &rect);
            }
            else
            {
                MoveSelections(pSLst, point);// Offset the tracking data
                c_ptLast = point;            // Save new 'last' position
                if (m_eSelMode == smodeMove && !m_rectMultiBorder.IsRectNull())
                    DrawSelectionRect(dc, &m_rectMultiBorder);
                else
                    pSLst.DrawTracker(dc);    // Turn off tracker
            }
            return TRUE;
        }
    }
#endif
    return FALSE;
}

void CSelectTool::MoveSelections(CSelList& pSLst, CPoint point)
{
    if (m_eSelMode == smodeMove)
    {
        CPoint ptDelta = (CPoint)(point - c_ptLast);
        pSLst.Offset(CB::Convert(ptDelta));
        if (!m_rectMultiBorder.IsRectNull())
            m_rectMultiBorder += ptDelta;
    }
    else
        pSLst.MoveHandle(m_nHandleID, CB::Convert(point));
}

BOOL CSelectTool::AdjustPoint(const CBrdEditView& pView, CPoint& point) const
{
    wxASSERT(!"TODO:");
#if 0
    pView.AdjustPoint(point);
    if (point == c_ptLast)
        return FALSE;
    if (m_eSelMode == smodeMove)
    {
        CRect rct = pView.GetSelectList().GetEnclosingRect();
        CPoint pnt = pView.GetWorkspaceDim();
        if (rct.left + point.x - c_ptLast.x < 0)        // Clamp
            point.x = c_ptLast.x - rct.left;
        if (rct.top + point.y - c_ptLast.y < 0)         // Clamp
            point.y = c_ptLast.y - rct.top;
        if (rct.right + point.x - c_ptLast.x > pnt.x)   // Clamp
            point.x = pnt.x - (rct.right - c_ptLast.x);
        if (rct.bottom + point.y - c_ptLast.y > pnt.y)  // Clamp
            point.y = pnt.y - (rct.bottom - c_ptLast.y);
    }
#endif
    return TRUE;
}

void CSelectTool::StartDragTimer(CBrdEditView& pView)
{
    wxASSERT(!"TODO:");
#if 0
    m_nTimerID = pView.SetTimer(timerIDSelectDelay,
        timerSelDelay, NULL);
#endif
}

void CSelectTool::KillDragTimer(CBrdEditView& pView)
{
    wxASSERT(!"TODO:");
#if 0
    if (m_nTimerID != uintptr_t(0))
    {
        pView.KillTimer(m_nTimerID);
        m_nTimerID = uintptr_t(0);
    }
#endif
}

void CSelectTool::StartScrollTimer(CBrdEditView& pView)
{
    wxASSERT(!"TODO:");
#if 0
    m_nTimerID = pView.SetTimer(timerIDAutoScroll, timerAutoScroll, NULL);
#endif
}

void CSelectTool::KillScrollTimer(CBrdEditView& pView)
{
    wxASSERT(!"TODO:");
#if 0
    if (m_nTimerID != uintptr_t(0))
    {
        pView.KillTimer(m_nTimerID);
        m_nTimerID = uintptr_t(0);
    }
#endif
}

////////////////////////////////////////////////////////////////////////
// CShapeTool - tool used to create rectangles.

void CShapeTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    CSelList& pSLst = pView.GetSelectList();
    pSLst.PurgeList(TRUE);         // Clear current select list
    int nDragHandle;
    pView.AdjustPoint(point);
    m_pObj = CreateDrawObj(pView, point, nDragHandle);
    pSLst.AddObject(*m_pObj, TRUE);
    s_selectTool.StartSizingOperation(pView, nFlags, point, nDragHandle);
#endif
}

void CShapeTool::OnLButtonUp(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    if (!pView.HasCapture())
        return;
    s_selectTool.OnLButtonUp(pView, nFlags, point);
    pView.GetSelectList().PurgeList(TRUE); // Clear current select list
    wxASSERT(!"TODO:");
#if 0
    if (!IsEmptyObject())
        pView.AddDrawObject(std::move(m_pObj));
    else
    {
        m_pObj = nullptr;
    }
    pView.ResetToDefaultTool();
#endif
}

void CShapeTool::OnMouseMove(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    if (pView.HasCapture())
        s_selectTool.OnMouseMove(pView, nFlags, point);
}

void CShapeTool::OnTimer(CBrdEditView& pView, uintptr_t nIDEvent)
{
    s_selectTool.OnTimer(pView, nIDEvent);
}

BOOL CShapeTool::OnSetCursor(const CBrdEditView& pView, UINT nHitTest) const
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CRectTool - tool used to create rectangles.

CDrawObj::OwnerPtr CRectTool::CreateDrawObj(const CBrdEditView& pView, CPoint point,
    int& nHandle) const
{
    CRectObj::OwnerPtr pObj = new CRectObj;
    pObj->SetRect(CRect(point, CSize(0,0)));
    pObj->SetForeColor(CB::Convert(pView.GetForeColor()));
    pObj->SetBackColor(CB::Convert(pView.GetBackColor()));
    pObj->SetLineWidth(pView.GetLineWidth());
    nHandle = hitBottomRight;
    return pObj;
}

BOOL CRectTool::IsEmptyObject() const
{
    wxASSERT(m_pObj && dynamic_cast<const CRectObj*>(&*m_pObj));
    return static_cast<const CRectObj&>(*m_pObj).GetRect().IsRectEmpty();
}

////////////////////////////////////////////////////////////////////////
// CEllipseTool - tool used to create ellipses.

CDrawObj::OwnerPtr CEllipseTool::CreateDrawObj(const CBrdEditView& pView, CPoint point,
    int& nHandle) const
{
    CEllipse::OwnerPtr pObj = new CEllipse;
    pObj->SetRect(CRect(point, CSize(0,0)));
    pObj->SetForeColor(CB::Convert(pView.GetForeColor()));
    pObj->SetBackColor(CB::Convert(pView.GetBackColor()));
    pObj->SetLineWidth(pView.GetLineWidth());
    nHandle = hitBottomRight;
    return pObj;
}

BOOL CEllipseTool::IsEmptyObject() const
{
    wxASSERT(m_pObj && dynamic_cast<const CEllipse*>(&*m_pObj));
    return static_cast<const CEllipse&>(*m_pObj).GetRect().IsRectEmpty();
}

////////////////////////////////////////////////////////////////////////
// CLineTool - tool used to create lines

CDrawObj::OwnerPtr CLineTool::CreateDrawObj(const CBrdEditView& pView, CPoint point,
    int& nHandle) const
{
    OwnerPtr<CLine> pObj = new CLine;
    pObj->SetLine(point.x, point.y, point.x, point.y);
    pObj->SetForeColor(CB::Convert(pView.GetForeColor()));
    pObj->SetLineWidth(pView.GetLineWidth());
    nHandle = hitPtB;
    return pObj;
}

BOOL CLineTool::IsEmptyObject() const
{
    wxASSERT(m_pObj && dynamic_cast<const CLine*>(&*m_pObj));
    CRect rct = static_cast<const CLine&>(*m_pObj).GetRect();
    return rct.Width() < 3 && rct.Height() < 3;
}

////////////////////////////////////////////////////////////////////////
// CPolyTool - tool used to create polygons and polylines

void CPolyTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    CRect rctClient;
    pView.GetClientRect(&rctClient);
    CPoint pntClient(point);
    pView.WorkspaceToClient(pntClient);
    if (!rctClient.PtInRect(pntClient))
        return;

    CPoint rawPoint(point);         // Make copy of unadjusted point

    pView.AdjustPoint(point);

    // Check if this is a new object.
    if (m_pObj == NULL)
    {
        CSelList& pSLst = pView.GetSelectList();
        pSLst.PurgeList(TRUE);         // Clear current select list
        // First we create a dummy object which will be modified when the
        // Polygon is completed. The dummy object will be draw with
        // black lines and a single pixel line.
        m_pObj = new CPolyObj;
        m_pObj->AddPoint(point);
        pView.AddDrawObject(m_pObj);
        CTool::OnLButtonDown(pView, nFlags, point); // Capture and save point
    }
    else
    {
        CClientDC dc(&pView);
        pView.OnPrepareScaledDC(dc);
//      CRect rct;
//      pView->GetClientRect(&rct);
//      dc.DPtoLP(&rct);
//      if (rct.PtInRect(point))            // Don't draw if off view.
//      {
            DrawRubberLine(dc);            // Turn off last rubber line

            // Check if back at the original point
            CPoint pnt(m_pObj->m_Pnts[size_t(0)]);
            if (pnt == point)
            {
                FinalizePolygon(pView);
                pView->ReleaseMouse();
                return;
            }
            m_pObj->AddPoint(point);
            c_ptDown = point;
            c_ptLast = rawPoint;
            DrawRubberLine(dc);            // Turn on last rubber line

            // Directly draw the object.
            m_pObj->Draw(dc, pView.GetCurrentScale());
//      }
    }
#endif
}

void CPolyTool::OnLButtonDblClk(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    CRect rctClient;
    pView.GetClientRect(&rctClient);
    CPoint pntClient(point);
    pView.WorkspaceToClient(pntClient);
    if (!rctClient.PtInRect(pntClient))
        return;
    if (m_pObj)
    {
        CClientDC dc(&pView);
        pView.OnPrepareScaledDC(dc);
        DrawRubberLine(dc);                // Turn off rubber band
        FinalizePolygon(pView);
    }
#endif
    pView.ReleaseMouse();
}

void CPolyTool::OnMouseMove(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    if (!pView.HasCapture())
        return;
    if (m_pObj == NULL)
        pView.ReleaseMouse();
    wxASSERT(!"TODO:");
#if 0
    CClientDC dc(&pView);
    pView.OnPrepareScaledDC(dc);
    DrawRubberLine(dc);

    if (nFlags & MK_LBUTTON)
    {
        CPoint rawPoint(point);         // Make copy of unadjusted point

        pView.AdjustPoint(point);
        // Check if back at the original point
        CPoint pnt(m_pObj->m_Pnts[size_t(0)]);
        if (pnt == point)
        {
            FinalizePolygon(pView);
            pView->ReleaseMouse();
            return;
        }
        m_pObj->AddPoint(point);
        c_ptDown = point;
        c_ptLast = rawPoint;
        // Directly draw the object.
        m_pObj->Draw(dc, pView.GetCurrentScale());
    }
    else
        c_ptLast = point;
    DrawRubberLine(dc);
#endif
}

BOOL CPolyTool::OnSetCursor(const CBrdEditView& pView, UINT nHitTest) const
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}

void CPolyTool::DrawRubberLine(CDC& pDC)
{
    CPen pen(PS_DOT, 1, RGB(0,0,0));
    int nPrvRop = pDC.SetROP2(R2_XORPEN);
    CPen* pPrvPen = pDC.SelectObject(&pen);
    CBrush* pPrvBrush = (CBrush*)pDC.SelectStockObject(NULL_BRUSH);

    pDC.MoveTo(c_ptDown);      // Erase previous
    pDC.LineTo(c_ptLast);

    pDC.SetROP2(nPrvRop);
    pDC.SelectObject(pPrvPen);
    pDC.SelectObject(pPrvBrush);
}

void CPolyTool::RemoveRubberBand(CBrdEditView& pView)
{
    if (m_pObj != NULL)
    {
        wxASSERT(!"TODO:");
#if 0
        CClientDC dc(&pView);
        pView.OnPrepareScaledDC(dc);
        DrawRubberLine(dc);
#endif
        pView.ReleaseMouse();
    }
}

void CPolyTool::FinalizePolygon(CBrdEditView& pView,
    BOOL bForceDestroy /* = FALSE */)
{
    wxASSERT(!"TODO:");
#if 0
    pView.ResetToDefaultTool();
    if (m_pObj == NULL)
        return;         // Nothing to do.

    if (m_pObj->m_Pnts.size() >= size_t(2) && !bForceDestroy)
    {
        // Update the "dummy" object to make it the real thing.
        m_pObj->SetForeColor(pView.GetForeColor());
        m_pObj->SetBackColor(pView.GetBackColor());
        m_pObj->SetLineWidth(pView.GetLineWidth());
        CRect rct = m_pObj->GetEnclosingRect();
        pView.InvalidateWorkspaceRect(&rct);
    }
    else
    {
        // Not enough to work with...
        CRect rct = m_pObj->GetEnclosingRect();
        pView.InvalidateWorkspaceRect(&rct);
        pView.DeleteDrawObject(m_pObj);
    }
#endif
    m_pObj = NULL;
}

////////////////////////////////////////////////////////////////////////
// CTextTool - Text drawing object tool

void CTextTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags,
    CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    pView.DoCreateTextDrawingObject(point);
    pView.ResetToDefaultTool();
#endif
}

BOOL CTextTool::OnSetCursor(const CBrdEditView& pView, UINT nHitTest) const
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CColorPickupTool - Used to extract colors from view.

void CColorPickupTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags,
    CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    pView.WorkspaceToClient(point);
    CWindowDC dc(&pView);
    COLORREF crPxl = dc.GetPixel(point);
    if ((nFlags & (MK_CONTROL | MK_SHIFT)) == 0)
        pView.SetForeColor(crPxl);
    else if (nFlags & MK_SHIFT)
        pView.SetBackColor(crPxl);
    pView.SetCapture();
#endif
}

void CColorPickupTool::OnLButtonUp(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    if (!pView.HasCapture())
        return;
    wxASSERT(!"TODO:");
#if 0
    pView.WorkspaceToClient(point);

    CWindowDC dc(&pView);
    COLORREF crPxl = dc.GetPixel(point);

    if ((nFlags & (MK_CONTROL | MK_SHIFT)) == 0)
        pView.SetForeColor(crPxl);
    else if (nFlags & MK_SHIFT)
        pView.SetBackColor(crPxl);

    pView.RestoreLastTool();               // For convenience
#endif
    pView.ReleaseMouse();
}

BOOL CColorPickupTool::OnSetCursor(const CBrdEditView& pView, UINT nHitTest) const
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrDropper);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CCellPaintTool - Paint bucket tool for cell grid layer

void CCellPaintTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags,
    CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    pView.SetCellColor(pView->GetForeColor(), point, TRUE);
#endif
}

void CCellPaintTool::OnMouseMove(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    if (nFlags & MK_LBUTTON)
        pView.SetCellColor(pView->GetForeColor(), point, TRUE);
#endif
}

BOOL CCellPaintTool::OnSetCursor(const CBrdEditView& pView, UINT nHitTest) const
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrFill);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CPaintTool - Paint bucket tool for base layer

void CPaintTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags,
    CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    if (pView.GetBoard().GetMaxDrawLayer() == LAYER_BASE)
        pView.SetBoardBackColor(pView.GetForeColor(), TRUE);
#endif
}

BOOL CPaintTool::OnSetCursor(const CBrdEditView& pView, UINT nHitTest) const
{
    if (pView.GetBoard().GetMaxDrawLayer() != LAYER_BASE)
        return FALSE;
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrFill);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CTileTool - Tile painting tool

void CTileTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags,
    CPoint point)
{
    TileID tid = pView.GetDocument().GetTilePalWnd()->GetCurrentTileID();
    wxASSERT(!"TODO:");
#if 0
    if (pView.GetBoard().GetMaxDrawLayer() == LAYER_GRID)
        pView.SetCellTile(tid, point, TRUE);
    else
    {
        CDrawList& pDwg = CheckedDeref(pView.GetDrawList(TRUE));
        pView.SetDrawingTile(pDwg, tid, point, TRUE);
    }
#endif
}

void CTileTool::OnMouseMove(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    if ((nFlags & MK_LBUTTON) != 0 &&
        pView.GetBoard().GetMaxDrawLayer() == LAYER_GRID)
    {
        TileID tid = pView.GetDocument().GetTilePalWnd()->GetCurrentTileID();
        pView.SetCellTile(tid, point, TRUE);
    }
#endif
}

BOOL CTileTool::OnSetCursor(const CBrdEditView& pView, UINT nHitTest) const
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrDragTile);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CCellEraserTool - eraser tool for cell grid layer

void CCellEraserTool::OnLButtonDown(CBrdEditView& pView, UINT nFlags,
    CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
        pView.SetCellColor(noColor, point, TRUE);
//      pView->SetCellTile(nullTid, point, TRUE);
#endif
}

void CCellEraserTool::OnMouseMove(CBrdEditView& pView, UINT nFlags, CPoint point)
{
    wxASSERT(!"TODO:");
#if 0
    if (nFlags & MK_LBUTTON)
    {
        pView.SetCellColor(noColor, point, TRUE);
//      pView->SetCellTile(nullTid, point, TRUE);
    }
#endif
}

BOOL CCellEraserTool::OnSetCursor(const CBrdEditView& pView, UINT nHitTest) const
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrEraser);
    return TRUE;
}



