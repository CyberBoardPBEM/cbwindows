// SelObjs.cpp
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
#include    <limits.h>
#include    "Gm.h"
#include    "GmDoc.h"
#include    "DrawObj.h"
#include    "SelObjs.h"
#include    "VwEdtbrd.h"
#include    "ClipBrd.h"
#include    "CDib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////

const int handleHalfWidth = 3;

/////////////////////////////////////////////////////////////////////
// Class level variables

const wxPen CSelection::c_penDot(*wxLIGHT_GREY, 1, wxPENSTYLE_SHORT_DASH);

/////////////////////////////////////////////////////////////////////

void CSelection::DrawTracker(wxDC& pDC, TrackMode eMode) const
{
    if (eMode == trkSelected)
        DrawHandles(pDC);
    else if (eMode == trkMoving || eMode == trkSizing)
        DrawTrackingImage(pDC, eMode);
}

void CSelection::DrawHandles(wxDC& pDC) const
{
    int n = GetHandleCount();
    for (int i = 0; i < n; i++)
    {
        wxRect rect = GetHandleRect(i);
        CB::DCLogicalFunctionChanger setLogFunc(pDC, wxXOR);
        wxDCPenChanger setPen(pDC, *wxWHITE_PEN);
        wxDCBrushChanger setBrush(pDC, *wxWHITE_BRUSH);
        pDC.DrawRectangle(rect);
    }
}

int CSelection::HitTestHandles(wxPoint point) const
{
    int n = GetHandleCount();
    for (int i = 0; i < n; i++)
    {
        if (GetHandleRect(i).Contains(point))
            return i;
    }
    return hitNothing;
}

void CSelection::InvalidateHandles()
{
    int n = GetHandleCount();
    for (int i = 0; i < n; i++)
    {
        wxRect rct = GetHandleRect(i);
        m_pView->InvalidateWorkspaceRect(rct, FALSE);
    }
}

// Returns handle rectangle in logical coords.
wxRect CSelection::GetHandleRect(int nHandleID) const
{
    // Get the center of the handle in logical coords
    wxPoint point = GetHandleLoc(nHandleID);

    // Convert point to client coords
    m_pView->WorkspaceToClient(point);

    // Calc CRect of handle in device coords
    wxRect rect(wxPoint(point.x-3, point.y-3), wxSize(6, 6));

    m_pView->ClientToWorkspace(rect);

    return rect;
}

void CSelection::Invalidate()
{
    wxRect rct = CB::Convert(m_pObj->GetEnclosingRect());
    m_pView->InvalidateWorkspaceRect(rct, FALSE);
}

//=---------------------------------------------------=//
// Static methods...

CSelection::DCSetupTrackingDraw::DCSetupTrackingDraw(wxDC& pDC) :
    setPen(pDC, c_penDot),
    setBrush(pDC, *wxTRANSPARENT_BRUSH)
{
}

/////////////////////////////////////////////////////////////////////
// Rectangle Selection Processing

void CSelRect::DrawTrackingImage(wxDC& pDC, TrackMode eMode) const
{
    DCSetupTrackingDraw setupTrackingDraw(pDC);
    pDC.DrawRectangle(m_rect);
}

wxCursor CSelRect::GetHandleCursor(int nHandleID) const
{
    wxStockCursor id;
    switch (nHandleID)
    {
        case hitTopLeft:
        case hitBottomRight: id = wxCURSOR_SIZENWSE;  break;
        case hitTop:
        case hitBottom:      id = wxCURSOR_SIZENS;    break;
        case hitTopRight:
        case hitBottomLeft:  id = wxCURSOR_SIZENESW;  break;
        case hitRight:
        case hitLeft:        id = wxCURSOR_SIZEWE;    break;
        default: ASSERT(FALSE);
            id = wxCURSOR_NONE;
    }
    return wxCursor(id);
}

// Returns handle location in logical coords.
wxPoint CSelRect::GetHandleLoc(int nHandleID) const
{
    int x, y;

    // This gets the center regardless of left/right and
    // top/bottom ordering
    wxPoint center = GetMidRect(m_rect);

    // + 1:  wxRect right/bottom != CRect right/bottom
    switch (nHandleID)
    {
        case hitTopLeft:     x = m_rect.GetLeft();      y = m_rect.GetTop();        break;
        case hitTopRight:    x = m_rect.GetRight() + 1; y = m_rect.GetTop();        break;
        case hitBottomRight: x = m_rect.GetRight() + 1; y = m_rect.GetBottom() + 1; break;
        case hitBottomLeft:  x = m_rect.GetLeft();      y = m_rect.GetBottom() + 1; break;
        case hitTop:         x = center.x;              y = m_rect.GetTop();        break;
        case hitRight:       x = m_rect.GetRight() + 1; y = center.y;               break;
        case hitBottom:      x = center.x;              y = m_rect.GetBottom() + 1; break;
        case hitLeft:        x = m_rect.GetLeft();      y = center.y;               break;
        default: ASSERT(FALSE);
            x = -1; y = -1;
    }
    return wxPoint(x, y);
}

void CSelRect::MoveHandle(int nHandle, wxPoint point)
{
    // SetLeft() and SetTop() also change right and bottom
    switch (nHandle)
    {
        case hitTopLeft:
            m_rect = wxRect(point, wxSize(m_rect.GetRight() + 1 - point.x, m_rect.GetBottom() + 1 - point.y)); break;
        case hitTopRight:
            m_rect = wxRect(wxPoint(m_rect.GetLeft(), point.y), wxSize(point.x - m_rect.GetLeft(), m_rect.GetBottom() + 1 - point.y)); break;
        case hitBottomRight:
            m_rect = wxRect(m_rect.GetLeftTop(), wxSize(point.x - m_rect.GetLeft(), point.y - m_rect.GetTop())); break;
        case hitBottomLeft:
            m_rect = wxRect(wxPoint(point.x, m_rect.GetTop()), wxSize(m_rect.GetRight() + 1 - point.x, point.y - m_rect.GetTop())); break;
        case hitTop:
            m_rect = wxRect(wxPoint(m_rect.GetLeft(), point.y), wxSize(m_rect.GetWidth(), m_rect.GetBottom() + 1 - point.y)); break;
        case hitRight:
            m_rect = wxRect(m_rect.GetLeftTop(), wxSize(point.x - m_rect.GetLeft(), m_rect.GetHeight())); break;
        case hitBottom:
            m_rect = wxRect(m_rect.GetLeftTop(), wxSize(m_rect.GetWidth(), point.y - m_rect.GetTop())); break;
        case hitLeft:
            m_rect = wxRect(wxPoint(point.x, m_rect.GetTop()), wxSize(m_rect.GetRight() + 1 - point.x, m_rect.GetHeight())); break;
        default: ASSERT(FALSE);
    }
}

void CSelRect::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    CRectObj& pObj = static_cast<CRectObj&>(*m_pObj);
    if (bInvalidate)
    {
        wxRect rctA = CB::Convert(pObj.GetEnclosingRect());
        wxRect rctB = CB::Convert(pObj.GetRect());
        m_pView->WorkspaceToClient(rctB);
        rctB.Inflate(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rctB);
        rctA += rctB;               // Make sure we erase the handles
        m_pView->InvalidateWorkspaceRect(rctA);
    }
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        CB::Normalize(m_rect);
        pObj.SetRect(CB::Convert(m_rect));
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        m_rect = CB::Convert(pObj.GetRect());
    }
    if (bInvalidate)
    {
        wxRect rct = CB::Convert(pObj.GetEnclosingRect());
        m_pView->InvalidateWorkspaceRect(rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Ellipse Selection Processing

void CSelEllipse::DrawTrackingImage(wxDC& pDC, TrackMode eMode) const
{
    DCSetupTrackingDraw setupTrackingDraw(pDC);
    CB::DrawEllipse(pDC, m_rect);
}

/////////////////////////////////////////////////////////////////////
// Line Selection Processing

void CSelLine::DrawTrackingImage(wxDC& pDC, TrackMode eMode) const
{
    DCSetupTrackingDraw setupTrackingDraw(pDC);
    pDC.DrawLine(m_rect.GetLeftTop(), m_rect.GetRightBottom() += wxPoint(1, 1));
}

wxCursor CSelLine::GetHandleCursor(int nHandleID) const
{
    wxStockCursor id;
    switch (nHandleID)
    {

        case hitPtA:
        case hitPtB:
            id = wxCURSOR_CROSS;
            break;
        default: ASSERT(FALSE);
            id = wxCURSOR_NONE;
    }
    return wxCursor(id);
}

// Returns handle location in logical coords.
wxPoint CSelLine::GetHandleLoc(int nHandleID) const
{
    int x, y;

    switch (nHandleID)
    {
        case hitPtA: x = m_rect.GetLeft();  y = m_rect.GetTop();    break;
        case hitPtB:
            x = m_rect.GetRight() + 1;
            y = m_rect.GetBottom() + 1;
            // If the points are right on top of each other,
            // nudge this hit point over a bit.
            if (m_rect.GetLeft() == m_rect.GetRight() + 1 &&
                m_rect.GetTop() == m_rect.GetBottom() + 1)
            {
                x++;
                y++;
            }
            break;
        default: ASSERT(FALSE);
            x = -1; y = -1;
    }
    return wxPoint(x, y);
}

void CSelLine::MoveHandle(int nHandle, wxPoint point)
{
    switch (nHandle)
    {
        case hitPtA:
            m_rect = wxRect(wxPoint(point.x, point.y), wxSize(m_rect.GetRight() + 1 - point.x, m_rect.GetBottom() + 1 - point.y)); break;
        case hitPtB:
            m_rect = wxRect(m_rect.GetLeftTop(), wxSize(point.x - m_rect.GetLeft(), point.y - m_rect.GetTop())); break;
        default: ASSERT(FALSE);
    }
}

wxRect CSelLine::GetRect() const
{
    wxRect rct = m_rect;
    CB::Normalize(rct);
    return rct;
}

void CSelLine::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    CLine& pObj = static_cast<CLine&>(*m_pObj);
    if (bInvalidate)
    {
        wxRect rctA = CB::Convert(pObj.GetEnclosingRect());
        wxRect rctB = pObj.GetLine();
        m_pView->WorkspaceToClient(rctB);
        rctB.Inflate(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rctB);
        rctA += rctB;       // Make sure we erase the handles
        m_pView->InvalidateWorkspaceRect(rctA, FALSE);
    }
    pObj.SetLine(m_rect.GetLeft(), m_rect.GetTop(), m_rect.GetRight() + 1, m_rect.GetBottom() + 1);
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        pObj.SetLine(m_rect.GetLeft(), m_rect.GetTop(), m_rect.GetRight() + 1, m_rect.GetBottom() + 1);
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        m_rect = pObj.GetLine();
    }
    if (bInvalidate)
    {
        wxRect rct = CB::Convert(pObj.GetEnclosingRect());
        m_pView->InvalidateWorkspaceRect(rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Polygon Selection Processing

CSelPoly::CSelPoly(CBrdEditView& pView, CPolyObj& pObj) :
    CSelection(pView, pObj)
{
    ASSERT(!pObj.m_Pnts.empty());
    m_Pnts = CB::Convert(pObj.m_Pnts);
}

void CSelPoly::DrawTrackingImage(wxDC& pDC, TrackMode eMode) const
{
    ASSERT(!m_Pnts.empty());
    DCSetupTrackingDraw setupTrackingDraw(pDC);
    pDC.DrawLines(value_preserving_cast<int>(m_Pnts.size()), m_Pnts.data());
}

wxCursor CSelPoly::GetHandleCursor(int nHandleID) const
{
    return wxCursor(wxCURSOR_CROSS);
}

// Returns handle location in logical coords.
wxPoint CSelPoly::GetHandleLoc(int nHandleID) const
{
    ASSERT(m_Pnts.size() > value_preserving_cast<size_t>(nHandleID));
    return m_Pnts[value_preserving_cast<size_t>(nHandleID)];
}

void CSelPoly::MoveHandle(int nHandle, wxPoint point)
{
    ASSERT(m_Pnts.size() > value_preserving_cast<size_t>(nHandle));
    m_Pnts[value_preserving_cast<size_t>(nHandle)] = point;
}

void CSelPoly::Offset(wxPoint ptDelta)
{
    ASSERT(!m_Pnts.empty());
    for (size_t i = size_t(0) ; i < m_Pnts.size() ; ++i)
    {
        m_Pnts[i].x += ptDelta.x;
        m_Pnts[i].y += ptDelta.y;
    }
}

wxRect CSelPoly::GetRect() const
{
    wxRect rct;
    if (m_Pnts.empty())
        return rct;
    int xmin = INT_MAX, xmax = INT_MIN, ymin = INT_MAX, ymax = INT_MIN;
    for (size_t i = size_t(0) ; i < m_Pnts.size() ; ++i)
    {
        xmin = CB::min(xmin, m_Pnts[i].x);
        xmax = CB::max(xmax, m_Pnts[i].x);
        ymin = CB::min(ymin, m_Pnts[i].y);
        ymax = CB::max(ymax, m_Pnts[i].y);
    }
    rct = wxRect(wxPoint(xmin, ymin), wxSize(xmax - xmin, ymax - ymin));
    return rct;
}

void CSelPoly::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    CPolyObj& pObj = static_cast<CPolyObj&>(*m_pObj);
    if (bInvalidate)
    {
        wxRect rctA = CB::Convert(pObj.GetEnclosingRect());
        wxRect rctB = CB::Convert(pObj.GetRect());
        m_pView->WorkspaceToClient(rctB);
        rctB.Inflate(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rctB);
        rctA += rctB;               // Make sure we erase the handles
        m_pView->InvalidateWorkspaceRect(rctA);
    }
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        pObj.SetNewPolygon(CB::Convert(m_Pnts));
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        m_Pnts = CB::Convert(pObj.m_Pnts);
    }
    if (bInvalidate)
    {
        wxRect rct = CB::Convert(pObj.GetEnclosingRect());
        m_pView->InvalidateWorkspaceRect(rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Generic Object Selection Processing

void CSelGeneric::DrawTrackingImage(wxDC& pDC, TrackMode eMode) const
{
    DCSetupTrackingDraw setupTrackingDraw(pDC);
    pDC.DrawRectangle(m_rect);
}

// Returns handle location in logical coords.
wxPoint CSelGeneric::GetHandleLoc(int nHandleID) const
{
    int x, y;

    switch (nHandleID)
    {
        case hitTopLeft:     x = m_rect.GetLeft();       y = m_rect.GetTop();        break;
        case hitTopRight:    x = m_rect.GetRight() + 1;  y = m_rect.GetTop();        break;
        case hitBottomRight: x = m_rect.GetRight() + 1 ; y = m_rect.GetBottom() + 1; break;
        case hitBottomLeft:  x = m_rect.GetLeft();       y = m_rect.GetBottom() + 1; break;
        default: ASSERT(FALSE);
            x = -1; y = -1;
    }
    return wxPoint(x, y);
}

void CSelGeneric::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    CDrawObj& pObj = static_cast<CRectObj&>(*m_pObj);
    if (bInvalidate)
    {
        wxRect rct = CB::Convert(pObj.GetRect());
        m_pView->WorkspaceToClient(rct);
        rct.Inflate(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rct);
        m_pView->InvalidateWorkspaceRect(rct);
    }
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        CB::Normalize(m_rect);
        pObj.SetRect(CB::Convert(m_rect));
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        m_rect = CB::Convert(pObj.GetRect());
    }
    if (bInvalidate)
    {
        wxRect rct = CB::Convert(pObj.GetEnclosingRect());
        m_pView->InvalidateWorkspaceRect(rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Selection List Processing

int CSelList::HitTestHandles(wxPoint point) const
{
    // No support for multiselect handles.
    return IsSingleSelect() ? front()->HitTestHandles(point) : hitNothing;
}

void CSelList::MoveHandle(int m_nHandle, wxPoint point)
{
    // No support for multiselect handles.
    if (IsSingleSelect())
        front()->MoveHandle(m_nHandle, point);
    CalcEnclosingRect();
}

CSelection& CSelList::AddObject(CDrawObj& pObj, BOOL bInvalidate)
{
    {
        OwnerPtr<CSelection> pSel = pObj.CreateSelectProxy(*m_pView);
        push_front(std::move(pSel));
    }
    CSelection& pSel = *front();
    CalcEnclosingRect();
    if (bInvalidate)
        pSel.InvalidateHandles();
    return pSel;
}

void CSelList::RemoveObject(const CDrawObj& pObj, BOOL bInvalidate)
{
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        if (pSel.m_pObj == &pObj)
        {
            if (bInvalidate)
                pSel.InvalidateHandles();  // So view updates
            erase(pos);
            CalcEnclosingRect();
            return;                     // Success
        }
    }
    ASSERT(FALSE);                      //// Object wasn't in list ////
}

BOOL CSelList::IsObjectSelected(const CDrawObj& pObj) const
{
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj == &pObj)
            return TRUE;
    }
    return FALSE;
}

BOOL CSelList::IsDObjFlagSetInAllSelectedObjects(DWORD dwFlag) const
{
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if ((pSel.m_pObj->GetDObjFlags() & dwFlag) == 0)
            return FALSE;
    }
    return TRUE;
}

BOOL CSelList::IsDObjFlagSetInSomeSelectedObjects(DWORD dwFlag) const
{
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if ((pSel.m_pObj->GetDObjFlags() & dwFlag) != 0)
            return TRUE;
    }
    return FALSE;
}

void CSelList::SetDObjFlagInAllSelectedObjects(DWORD dwFlag)
{
    ForAllSelections([dwFlag](CDrawObj& pObj) { pObj.SetDObjFlags(dwFlag); });
}

void CSelList::ClearDObjFlagInAllSelectedObjects(DWORD dwFlag)
{
    ForAllSelections([dwFlag](CDrawObj& pObj) { pObj.ClearDObjFlags(dwFlag); });
}

BOOL CSelList::IsCopyToClipboardPossible() const
{
    // AT THIS POINT..THIS ONLY SUPPORTS COPYING A BITMAP TO THE CLIPBOARD
    if (!IsSingleSelect())
        return FALSE;
    const CSelection& pSel = *front();
    const CDrawObj& pDObj = *pSel.m_pObj;
    return pDObj.GetType() == CDrawObj::drawBitmap;
}

void CSelList::CopyToClipboard()
{
    ASSERT(IsCopyToClipboardPossible());

    CSelection& pSel = *front();
    wxASSERT(pSel.m_pObj->GetType() == CDrawObj::drawBitmap);
    CBitmapImage& pDObj = static_cast<CBitmapImage&>(*pSel.m_pObj);
    SetClipboardBitmap(CB::Convert(pDObj.m_bitmap));
}

void CSelList::Open()
{
    if (!IsSingleSelect())
        return;
    CSelection& pSel = *front();
    // AT THIS POINT..THIS ONLY SUPPORTS EDITTING TEXT
    if (pSel.m_pObj->GetType() != CDrawObj::drawText)
        return;
    CText& pDObj = static_cast<CText&>(*pSel.m_pObj);
    m_pView->DoEditTextDrawingObject(pDObj);
    pSel.InvalidateHandles();
    pSel.m_rect = CB::Convert(pDObj.GetEnclosingRect());
    pSel.InvalidateHandles();
}

// Assumes RECTs are normalized!!
void CbUnionRect(wxRect& pRctDst, const wxRect& pRctSrc1, const wxRect& pRctSrc2)
{
    typedef decltype(std::declval<wxRect>().GetLeft()) Coord;
    Coord left = CB::min(pRctSrc1.GetLeft(), pRctSrc2.GetLeft());
    Coord top = CB::min(pRctSrc1.GetTop(), pRctSrc2.GetTop());
    Coord right = CB::max(pRctSrc1.GetRight(), pRctSrc2.GetRight());
    Coord bottom = CB::max(pRctSrc1.GetBottom(), pRctSrc2.GetBottom());
    pRctDst = wxRect(wxPoint(left, top), wxPoint(right, bottom));
}

void CSelList::CalcEnclosingRect()
{
    m_rctEncl = wxRect();
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        if (m_rctEncl == wxRect())
            m_rctEncl = pSel.GetRect();
        else
            CbUnionRect(m_rctEncl, m_rctEncl, pSel.GetRect());
    }
}

void CSelList::Offset(wxPoint ptDelta)
{
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pSel.Offset(ptDelta);
    }
    CalcEnclosingRect();
}

// Called by view OnDraw(). This entry makes it possible
// to turn off handles during a drag operation.

void CSelList::OnDraw(wxDC& pDC)
{
    if (m_eTrkMode == trkSelected)
        DrawTracker(pDC);
}

void CSelList::DrawTracker(wxDC& pDC, TrackMode eTrkMode)
{
    if (eTrkMode != trkCurrent)
        m_eTrkMode = eTrkMode;

    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pSel.DrawTracker(pDC, m_eTrkMode);
    }
}

void CSelList::InvalidateListHandles(BOOL bUpdate)
{
    BOOL bFoundOne = FALSE;

    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pSel.InvalidateHandles();
        bFoundOne = TRUE;
    }
    if (bFoundOne && bUpdate)
        m_pView->Update();
}

void CSelList::InvalidateList(BOOL bUpdate)
{
    BOOL bFoundOne = FALSE;

    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pSel.Invalidate();
        bFoundOne = TRUE;
    }
    if (bFoundOne && bUpdate)
        m_pView->Update();
}

void CSelList::PurgeList(BOOL bInvalidate)
{
    if (bInvalidate)
        InvalidateListHandles();
    clear();
}

// Called after a move or size operation to change underlying
// objects to new postions/sizes.

void CSelList::UpdateObjects(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pSel.UpdateObject(bInvalidate, bUpdateObjectExtent);
    }
}

void CSelList::ForAllSelections(std::function<void (CDrawObj& pObj)> pFunc)
{
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pFunc(*pSel.m_pObj);
    }
}


