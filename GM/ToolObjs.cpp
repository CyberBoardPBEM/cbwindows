// ToolObjs.cpp
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
// Class variables

std::vector<CTool*> CTool::c_toolLib;          // Tool library

wxPoint CTool::c_ptDown;             // Mouse down location
wxPoint CTool::c_ptLast;             // Last mouse location

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

void CTool::OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point)
{
    pView.CaptureMouse();

    c_ptDown = point;
    c_ptLast = point;
}

void CTool::OnMouseMove(CBrdEditView& pView, int nMods, int /*nButs*/, wxPoint point)
{
    if (pView.HasCapture())
        c_ptLast = point;
    pView.SetCursor(wxCursor(wxCURSOR_ARROW));
}

void CTool::OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point)
{
    if (!pView.HasCapture())
        return;
    pView.ReleaseMouse();
}

void CTool::OnMouseCaptureLost(CBrdEditView& pView)
{
}

////////////////////////////////////////////////////////////////////////
// CSelectTool - Object Selection/Manipulation tool

void CSelectTool::OnLButtonDown(CBrdEditView& pView, int nMods,
    wxPoint point)
{
    CSelList& pSLst = pView.GetSelectList();
    // If a a handle is clicked on, immediately start tracking the
    // resize.
    if ((m_nHandleID = pSLst.HitTestHandles(point)) >= 0)
    {
        StartSizingOperation(pView, nMods, point);
        return;
    }
    CDrawObj* pObj = pView.ObjectHitTest(point);
    if (pObj == NULL)
    {
        if ((nMods & wxMOD_SHIFT) == 0)       // Shift click adds to list
            pSLst.PurgeList(TRUE);         // Clear current select list
        // No objects were under the mouse click.
        m_eSelMode = smodeNet;              // Net type selection
        CTool::OnLButtonDown(pView, nMods, point);
        return;
    }
    // Object is under mouse. See if also selected. If not,
    // add to list.
    if (!pSLst.IsObjectSelected(*pObj))
    {
        if ((nMods & wxMOD_SHIFT) == 0)       // Shift click adds to list
            pSLst.PurgeList(TRUE);         // Clear current select list
        if ((nMods & wxMOD_CONTROL) != 0)     // Control click drills down
            pView.SelectAllUnderPoint(point);
        else
            pSLst.AddObject(*pObj, TRUE);
        CTool::OnLButtonDown(pView, nMods, point);
        StartDragTimer(pView);
        if (pSLst.IsMultipleSelects())
        {
            // Setup a trigger zone. If the mouse is moved out of
            // this zone before the drag time expires, the move is
            // represented by a perimeter rectangle. Otherwise, it
            // is shown using proxy object tracker images.
            m_rectMultiBorder.Offset(point);
            m_rectMultiBorder.Inflate(2, 2);
        }
        return;
    }
    // At this point we know s/he clicked on an object that was
    // already selected. If SHIFT is held we'll remove it from
    // the list. Otherwise, a timer is started and drag tracking
    // wont start until it expires.
    if ((nMods & wxMOD_SHIFT) != 0)
    {
        pSLst.RemoveObject(*pObj, TRUE);
        return;
    }
    CTool::OnLButtonDown(pView, nMods, point);
    if (pSLst.IsMultipleSelects())
    {
        // Setup a trigger zone. If the mouse is moved out of
        // this zone before the drag time expires, the move is
        // represented by a perimeter rectangle. Otherwise, it
        // is shown using proxy object tracker images.
        m_rectMultiBorder.Offset(point);
        m_rectMultiBorder.Inflate(2, 2);
    }
    StartDragTimer(pView);
}

void CSelectTool::OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point)
{
    CSelList& pSLst = pView.GetSelectList();

    if (!pView.HasCapture())
        return;

    if (m_eSelMode == smodeNormal &&
        m_rectMultiBorder != wxRect())
    {
        // Mode is normal and a trigger rectangle is set.
        // The trigger rectangle is only set if multiple objects are
        // selected.
        if (!m_rectMultiBorder.Contains(point))
        {
            // Force the point onto the snap grid.
            pView.AdjustPoint(point);

            KillDragTimer(pView);
            m_rectMultiBorder = pSLst.GetEnclosingRect();

            // Force the rectangle onto the grid.
            pView.AdjustRect(m_rectMultiBorder);
            pSLst.Offset(m_rectMultiBorder.GetTopLeft() -
                pSLst.GetEnclosingRect().GetTopLeft());

            m_eSelMode = smodeMove;             // Now in move mode.
            pSLst.SetTrackingMode(trkMoving);
            wxOverlayDC dc(pView.GetOverlay(), &pView);
            pView.OnPrepareScaledDC(dc);
            dc.Clear();
            DrawSelectionRect(dc, m_rectMultiBorder);
            c_ptLast = point;                   // Save new 'last' position
            return;
        }
    }

    // If we get here, the mouse has been captured. Check if
    // we are doing a "net select".
    if (m_eSelMode == smodeNet)
    {
        wxOverlayDC dc(pView.GetOverlay(), &pView);
        pView.OnPrepareScaledDC(dc);
        dc.Clear();
        CTool::OnMouseMove(pView, nMods, nButs, point); // Update position
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
            wxRect rct = pSLst.GetEnclosingRect();
            wxPoint pnt = pView.GetWorkspaceDim();
            if (rct.GetLeft() + point.x - c_ptLast.x < 0)        // Clamp
                point.x = c_ptLast.x - rct.GetLeft();
            if (rct.GetTop() + point.y - c_ptLast.y < 0)         // Clamp
                point.y = c_ptLast.y - rct.GetTop();
            if (rct.GetRight() + point.x - c_ptLast.x > pnt.x)   // Clamp
                point.x = pnt.x - (rct.GetRight() - c_ptLast.x);
            if (rct.GetBottom() + point.y - c_ptLast.y > pnt.y)  // Clamp
                point.y = pnt.y - (rct.GetBottom() - c_ptLast.y);
        }

        TrackMode eTrkMode = m_eSelMode == smodeMove ? trkMoving : trkSizing;
        wxOverlayDC dc(pView.GetOverlay(), &pView);
        pView.OnPrepareScaledDC(dc);
        dc.Clear();

        MoveSelections(pSLst, point);

        if (m_eSelMode == smodeMove && m_rectMultiBorder != wxRect())
            DrawSelectionRect(dc, m_rectMultiBorder);
        else
            pSLst.DrawTracker(dc, eTrkMode); // Erase previous tracker
    }
    c_ptLast = point;               // Save new 'last' position
}

void CSelectTool::OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point)
{
    if (pView.HasCapture())
    {
        if (m_eSelMode == smodeNet)
        {
            pView.GetOverlay().Reset();
            // If the control key is down when button was released, fields
            // that intersect the select rect will selected. Otherwise only
            // those fields that are entirely within the select rect
            // will be selected.
            wxRect rect(wxPoint(std::min(c_ptDown.x, c_ptLast.x), std::min(c_ptDown.y, c_ptLast.y)),
                        wxSize(std::abs(c_ptLast.x - c_ptDown.x), std::abs(c_ptLast.y - c_ptDown.y)));
            pView.SelectWithinRect(rect, (nMods & wxMOD_CONTROL) != 0);
            CSelList& pSLst = pView.GetSelectList();
            pSLst.InvalidateListHandles();
        }
        else if (m_eSelMode != smodeNormal)
        {
            CSelList& pSLst = pView.GetSelectList();
            pSLst.SetTrackingMode(trkSelected);
            pSLst.UpdateObjects(TRUE);
            pSLst.InvalidateListHandles();
            pView.GetDocument().SetModifiedFlag();
        }
    }
    m_eSelMode = smodeNormal;
    KillDragTimer(pView);           // Make sure timers are released
    m_rectMultiBorder = wxRect();// Make sure trigger rect is empty
    CTool::OnLButtonUp(pView, nMods, point);
}

void CSelectTool::OnMouseCaptureLost(CBrdEditView& pView)
{
    pView.GetOverlay().Reset();
    m_eSelMode = smodeNormal;
    KillDragTimer(pView);           // Make sure timers are released
    m_rectMultiBorder = wxRect();// Make sure trigger rect is empty
    CSelList& pSLst = pView.GetSelectList();
    pSLst.SetTrackingMode(trkSelected);
    // undo move the tracking image
    wxPoint ptDelta = c_ptDown - c_ptLast;
    pSLst.Offset(ptDelta);
    CTool::OnMouseCaptureLost(pView);
}

void CSelectTool::OnTimer(CBrdEditView& pView)
{
    if (!pView.HasCapture())
    {
        m_eSelMode = smodeNormal;
        KillDragTimer(pView);
        m_rectMultiBorder = wxRect();// Make sure trigger rect is empty
        return;
    }
    if (m_eSelMode == smodeNormal)
    {
        CSelList& pSLst = pView.GetSelectList();

        // Mouse is captured and no particular drag operation
        // is underway. Therefore we want a move that draws
        // object outlines.
        m_eSelMode = smodeMove;
        m_rectMultiBorder = wxRect(); // Make sure trigger rect is empty
        KillDragTimer(pView);

        wxOverlayDC dc(pView.GetOverlay(), &pView);
        pView.OnPrepareScaledDC(dc);
        dc.Clear();

        // If the snap grid is active. Force the enclosing rect onto
        // the snap grid. Also force the previously saved mouse points
        // to the snap grid.
        wxRect rct = pSLst.GetEnclosingRect();
        pView.AdjustRect(rct);
        pSLst.Offset(rct.GetTopLeft() -
            pSLst.GetEnclosingRect().GetTopLeft());
        pView.AdjustPoint(c_ptLast);
        pView.AdjustPoint(c_ptDown);

        // Move the tracking image to its new location and draw it.
        wxPoint ptDelta = c_ptLast - c_ptDown;
        pSLst.Offset(ptDelta);

        pSLst.DrawTracker(dc, trkMoving);
    }
}

void CSelectTool::OnLButtonDblClk(CBrdEditView& pView, int nMods,
    wxPoint point)
{
    // "Normal" DblClk opens properties, OLE server, etc...
    CSelList& pSLst = pView.GetSelectList();
    if (pSLst.IsSingleSelect())
        pView.GetSelectList().Open();
//  CTool::OnLButtonDblClk(pView, nFlags, point);
}

wxCursor CSelectTool::OnSetCursor(const CBrdEditView& pView, wxPoint point) const
{
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
            return pSelObj.GetHandleCursor(nHandle);
        }
    }
    return wxNullCursor;       // Show default cursor
}

void CSelectTool::StartSizingOperation(CBrdEditView& pView, int nMods,
    wxPoint point, int nHandleID)
{
    CSelList& pSLst = pView.GetSelectList();
    if (nHandleID != -1)
        m_nHandleID = nHandleID;
    m_eSelMode = smodeSizing;
    CTool::OnLButtonDown(pView, nMods, point);
    wxOverlayDC dc(pView.GetOverlay(), &pView);
    pView.OnPrepareScaledDC(dc);
    dc.Clear();
    pSLst.DrawTracker(dc, trkSizing);
}

void CSelectTool::DrawSelectionRect(wxDC& pDC, const wxRect& pRct) const
{
    wxDCPenChanger setPen(pDC, *wxLIGHT_GREY);
    wxDCBrushChanger setBrush(pDC, *wxTRANSPARENT_BRUSH);

    pDC.DrawRectangle(pRct);
}

void CSelectTool::DrawNetRect(wxDC& pDC, CBrdEditView& /*pView*/) const
{
    wxRect rect(wxPoint(std::min(c_ptDown.x, c_ptLast.x), std::min(c_ptDown.y, c_ptLast.y)),
                wxSize(std::abs(c_ptLast.x - c_ptDown.x), std::abs(c_ptLast.y - c_ptDown.y)));
    DrawSelectionRect(pDC, rect);
}

void CSelectTool::MoveSelections(CSelList& pSLst, wxPoint point)
{
    if (m_eSelMode == smodeMove)
    {
        wxPoint ptDelta = point - c_ptLast;
        pSLst.Offset(ptDelta);
        if (m_rectMultiBorder != wxRect())
            m_rectMultiBorder.Offset(ptDelta);
    }
    else
        pSLst.MoveHandle(m_nHandleID, point);
}

void CSelectTool::StartDragTimer(CBrdEditView& pView)
{
    wxASSERT(!pView.GetTimer().IsRunning());
    pView.GetTimer().Start(timerSelDelay);
}

void CSelectTool::KillDragTimer(CBrdEditView& pView)
{
    pView.GetTimer().Stop();
}

////////////////////////////////////////////////////////////////////////
// CShapeTool - tool used to create rectangles.

void CShapeTool::OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point)
{
    CSelList& pSLst = pView.GetSelectList();
    pSLst.PurgeList(TRUE);         // Clear current select list
    int nDragHandle;
    pView.AdjustPoint(point);
    m_pObj = CreateDrawObj(pView, point, nDragHandle);
    pSLst.AddObject(*m_pObj, TRUE);
    s_selectTool.StartSizingOperation(pView, nMods, point, nDragHandle);
}

void CShapeTool::OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point)
{
    if (!pView.HasCapture())
        return;
    s_selectTool.OnLButtonUp(pView, nMods, point);
    pView.GetSelectList().PurgeList(TRUE); // Clear current select list
    if (!IsEmptyObject())
        pView.AddDrawObject(std::move(m_pObj));
    else
    {
        m_pObj = nullptr;
    }
    pView.ResetToDefaultTool();
}

void CShapeTool::OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point)
{
    if (pView.HasCapture())
        s_selectTool.OnMouseMove(pView, nMods, nButs, point);
}

void CShapeTool::OnMouseCaptureLost(CBrdEditView& pView)
{
    s_selectTool.OnMouseCaptureLost(pView);
    pView.GetSelectList().PurgeList(TRUE); // Clear current select list
    m_pObj = nullptr;
    pView.ResetToDefaultTool();
}

void CShapeTool::OnTimer(CBrdEditView& pView)
{
    s_selectTool.OnTimer(pView);
}

wxCursor CShapeTool::OnSetCursor(const CBrdEditView& pView, wxPoint point) const
{
    wxASSERT(pView.GetClientRect().Contains(pView.WorkspaceToClient(std::as_const(point))));
    return g_res.hcrCrossHairWx;
}

////////////////////////////////////////////////////////////////////////
// CRectTool - tool used to create rectangles.

CDrawObj::OwnerPtr CRectTool::CreateDrawObj(const CBrdEditView& pView, wxPoint point,
    int& nHandle) const
{
    CRectObj::OwnerPtr pObj = new CRectObj;
    pObj->SetRect(CRect(CB::Convert(point), CSize(0,0)));
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

CDrawObj::OwnerPtr CEllipseTool::CreateDrawObj(const CBrdEditView& pView, wxPoint point,
    int& nHandle) const
{
    CEllipse::OwnerPtr pObj = new CEllipse;
    pObj->SetRect(CRect(CB::Convert(point), CSize(0,0)));
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

CDrawObj::OwnerPtr CLineTool::CreateDrawObj(const CBrdEditView& pView, wxPoint point,
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

void CPolyTool::OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point)
{
    wxRect rctClient = pView.GetClientRect();
    wxPoint pntClient(point);
    pView.WorkspaceToClient(pntClient);
    if (!rctClient.Contains(pntClient))
        return;

    wxPoint rawPoint(point);         // Make copy of unadjusted point

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
        m_pObj->AddPoint(CB::Convert(point));
        pView.AddDrawObject(m_pObj);
        CTool::OnLButtonDown(pView, nMods, point); // Capture and save point
    }
    else
    {
        wxOverlayDC dc(pView.GetOverlay(), &pView);
        pView.OnPrepareScaledDC(dc);
        dc.Clear();
//      CRect rct;
//      pView->GetClientRect(&rct);
//      dc.DPtoLP(&rct);
//      if (rct.PtInRect(point))            // Don't draw if off view.
//      {
            DrawRubberLine(dc);            // Turn off last rubber line

            // Check if back at the original point
            wxPoint pnt(CB::Convert(m_pObj->m_Pnts[size_t(0)]));
            if (pnt == point)
            {
                FinalizePolygon(pView);
                pView.ReleaseMouse();
                return;
            }
            m_pObj->AddPoint(CB::Convert(point));
            c_ptDown = point;
            c_ptLast = rawPoint;
            DrawRubberLine(dc);            // Turn on last rubber line

            // Directly draw the object.
            m_pObj->Draw(dc, pView.GetCurrentScale());
//      }
    }
}

void CPolyTool::OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point)
{
    wxRect rctClient = pView.GetClientRect();
    wxPoint pntClient(point);
    pView.WorkspaceToClient(pntClient);
    if (!rctClient.Contains(pntClient))
        return;
    if (m_pObj)
    {
        FinalizePolygon(pView);
    }
    pView.ReleaseMouse();
}

void CPolyTool::OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point)
{
    if (!pView.HasCapture())
        return;
    if (m_pObj == NULL)
        pView.ReleaseMouse();
    wxOverlayDC dc(pView.GetOverlay(), &pView);
    pView.OnPrepareScaledDC(dc);
    dc.Clear();

    if (nButs & wxMOUSE_BTN_LEFT)
    {
        wxPoint rawPoint(point);         // Make copy of unadjusted point

        pView.AdjustPoint(point);
        // Check if back at the original point
        wxPoint pnt(CB::Convert(m_pObj->m_Pnts[size_t(0)]));
        if (pnt == point)
        {
            FinalizePolygon(pView);
            pView.ReleaseMouse();
            return;
        }
        m_pObj->AddPoint(CB::Convert(point));
        c_ptDown = point;
        c_ptLast = rawPoint;
        // Directly draw the object.
        m_pObj->Draw(dc, pView.GetCurrentScale());
    }
    else
    {
        // draw needed for wxOverlay
        m_pObj->Draw(dc, pView.GetCurrentScale());
        c_ptLast = point;
    }
    DrawRubberLine(dc);
}

void CPolyTool::OnMouseCaptureLost(CBrdEditView& pView)
{
    FinalizePolygon(pView, TRUE);
}

wxCursor CPolyTool::OnSetCursor(const CBrdEditView& /*pView*/, wxPoint /*point*/) const
{
    return g_res.hcrCrossHairWx;
}

void CPolyTool::DrawRubberLine(wxDC& pDC)
{
    wxPen pen(*wxLIGHT_GREY, 1, wxPENSTYLE_LONG_DASH);
    wxDCPenChanger setPen(pDC, pen);
    wxDCBrushChanger setBrush(pDC, *wxTRANSPARENT_BRUSH);

    pDC.DrawLine(c_ptDown, c_ptLast);
}

void CPolyTool::RemoveRubberBand(CBrdEditView& pView)
{
    if (m_pObj != NULL)
    {
        pView.ReleaseMouse();
    }
}

void CPolyTool::FinalizePolygon(CBrdEditView& pView,
    BOOL bForceDestroy /* = FALSE */)
{
    pView.ResetToDefaultTool();
    if (m_pObj == NULL)
        return;         // Nothing to do.

    if (m_pObj->m_Pnts.size() >= size_t(2) && !bForceDestroy)
    {
        // Update the "dummy" object to make it the real thing.
        m_pObj->SetForeColor(CB::Convert(pView.GetForeColor()));
        m_pObj->SetBackColor(CB::Convert(pView.GetBackColor()));
        m_pObj->SetLineWidth(pView.GetLineWidth());
        wxRect rct = CB::Convert(m_pObj->GetEnclosingRect());
        pView.InvalidateWorkspaceRect(rct);
    }
    else
    {
        // Not enough to work with...
        wxRect rct = CB::Convert(m_pObj->GetEnclosingRect());
        pView.InvalidateWorkspaceRect(rct);
        pView.DeleteDrawObject(m_pObj);
    }
    m_pObj = NULL;
}

////////////////////////////////////////////////////////////////////////
// CTextTool - Text drawing object tool

void CTextTool::OnLButtonDown(CBrdEditView& pView, int nMods,
    wxPoint point)
{
    pView.DoCreateTextDrawingObject(point);
    pView.ResetToDefaultTool();
}

wxCursor CTextTool::OnSetCursor(const CBrdEditView& /*pView*/, wxPoint /*point*/) const
{
    return g_res.hcrCrossHairWx;
}

////////////////////////////////////////////////////////////////////////
// CColorPickupTool - Used to extract colors from view.

void CColorPickupTool::OnLButtonDown(CBrdEditView& pView, int nMods,
    wxPoint point)
{
    pView.WorkspaceToClient(point);
    /* TODO:  wxClientDC and wxDC::GetPixel are non-portable
        (https://groups.google.com/g/wx-dev/c/qBMM0PxEJ48/m/rPysSjWVCgAJ) */
    wxClientDC dc(&pView);
    wxColour crPxl;
    CB_VERIFY(dc.GetPixel(point.x, point.y, &crPxl));
    if ((nMods & (wxMOD_CONTROL | wxMOD_SHIFT)) == 0)
        pView.SetForeColor(crPxl);
    else if (nMods & wxMOD_SHIFT)
        pView.SetBackColor(crPxl);
    pView.CaptureMouse();
}

void CColorPickupTool::OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point)
{
    if (!pView.HasCapture())
        return;
    pView.WorkspaceToClient(point);

    /* TODO:  wxClientDC and wxDC::GetPixel are non-portable
        (https://groups.google.com/g/wx-dev/c/qBMM0PxEJ48/m/rPysSjWVCgAJ) */
    wxClientDC dc(&pView);
    wxColour crPxl;
    CB_VERIFY(dc.GetPixel(point.x, point.y, &crPxl));

    if ((nMods & (wxMOD_CONTROL | wxMOD_SHIFT)) == 0)
        pView.SetForeColor(crPxl);
    else if (nMods & wxMOD_SHIFT)
        pView.SetBackColor(crPxl);

    pView.RestoreLastTool();               // For convenience
    pView.ReleaseMouse();
}

void CColorPickupTool::OnMouseCaptureLost(CBrdEditView& pView)
{
    pView.RestoreLastTool();               // For convenience
}

wxCursor CColorPickupTool::OnSetCursor(const CBrdEditView& /*pView*/, wxPoint /*point*/) const
{
    return g_res.hcrDropperWx;
}

////////////////////////////////////////////////////////////////////////
// CCellPaintTool - Paint bucket tool for cell grid layer

void CCellPaintTool::OnLButtonDown(CBrdEditView& pView, int nMods,
    wxPoint point)
{
    pView.SetCellColor(pView.GetForeColor(), point, TRUE);
}

void CCellPaintTool::OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point)
{
    if (nButs & wxMOUSE_BTN_LEFT)
        pView.SetCellColor(pView.GetForeColor(), point, TRUE);
}

wxCursor CCellPaintTool::OnSetCursor(const CBrdEditView& /*pView*/, wxPoint /*point*/) const
{
    return g_res.hcrFillWx;
}

////////////////////////////////////////////////////////////////////////
// CPaintTool - Paint bucket tool for base layer

void CPaintTool::OnLButtonDown(CBrdEditView& pView, int nMods,
    wxPoint point)
{
    if (pView.GetBoard().GetMaxDrawLayer() == LAYER_BASE)
        pView.SetBoardBackColor(pView.GetForeColor(), TRUE);
}

wxCursor CPaintTool::OnSetCursor(const CBrdEditView& pView, wxPoint /*point*/) const
{
    if (pView.GetBoard().GetMaxDrawLayer() != LAYER_BASE)
        return wxNullCursor;
    return g_res.hcrFillWx;
}

////////////////////////////////////////////////////////////////////////
// CTileTool - Tile painting tool

void CTileTool::OnLButtonDown(CBrdEditView& pView, int nMods,
    wxPoint point)
{
    TileID tid = pView.GetDocument().GetTilePalWnd().GetCurrentTileID();
    if (pView.GetBoard().GetMaxDrawLayer() == LAYER_GRID)
        pView.SetCellTile(tid, point, TRUE);
    else
    {
        CDrawList& pDwg = CheckedDeref(pView.GetDrawList(TRUE));
        pView.SetDrawingTile(pDwg, tid, point, TRUE);
    }
}

void CTileTool::OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point)
{
    if ((nButs & wxMOUSE_BTN_LEFT) != 0 &&
        pView.GetBoard().GetMaxDrawLayer() == LAYER_GRID)
    {
        TileID tid = pView.GetDocument().GetTilePalWnd().GetCurrentTileID();
        pView.SetCellTile(tid, point, TRUE);
    }
}

wxCursor CTileTool::OnSetCursor(const CBrdEditView& /*pView*/, wxPoint /*point*/) const
{
    return g_res.hcrDragTileWx;
}

////////////////////////////////////////////////////////////////////////
// CCellEraserTool - eraser tool for cell grid layer

void CCellEraserTool::OnLButtonDown(CBrdEditView& pView, int nMods,
    wxPoint point)
{
        pView.SetCellColor(noColor, point, TRUE);
//      pView->SetCellTile(nullTid, point, TRUE);
}

void CCellEraserTool::OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point)
{
    if (nButs & wxMOUSE_BTN_LEFT)
    {
        pView.SetCellColor(noColor, point, TRUE);
//      pView->SetCellTile(nullTid, point, TRUE);
    }
}

wxCursor CCellEraserTool::OnSetCursor(const CBrdEditView& /*pView*/, wxPoint /*point*/) const
{
    return g_res.hcrEraserWx;
}



