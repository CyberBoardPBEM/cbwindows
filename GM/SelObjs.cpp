// SelObjs.cpp
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
#include    <limits.h>
#include    "Gm.h"
#include    "GmDoc.h"
#include    "DrawObj.h"
#include    "SelObjs.h"
#include    "VwEdtbrd.h"
#include    "ClipBrd.h"

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

CPen    NEAR CSelection::c_penDot(PS_DOT, 1, RGB(0,0,0));
int     NEAR CSelection::c_nPrvROP2;
CPen*   NEAR CSelection::c_pPrvPen = NULL;
CBrush* NEAR CSelection::c_pPrvBrush = NULL;

/////////////////////////////////////////////////////////////////////

void CSelection::DrawTracker(CDC* pDC, TrackMode eMode)
{
    if (eMode == trkSelected)
        DrawHandles(pDC);
    else if (eMode == trkMoving || eMode == trkSizing)
        DrawTrackingImage(pDC, eMode);
}

void CSelection::DrawHandles(CDC* pDC)
{
    int n = GetHandleCount();
    for (int i = 0; i < n; i++)
    {
        CRect rect = GetHandleRect(i);
        pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), DSTINVERT);
    }
}

int CSelection::HitTestHandles(CPoint point)
{
    int n = GetHandleCount();
    for (int i = 0; i < n; i++)
    {
        if (GetHandleRect(i).PtInRect(point))
            return i;
    }
    return hitNothing;
}

void CSelection::InvalidateHandles()
{
    int n = GetHandleCount();
    for (int i = 0; i < n; i++)
    {
        CRect rct = GetHandleRect(i);
        m_pView->InvalidateWorkspaceRect(&rct, FALSE);
    }
}

// Returns handle rectangle in logical coords.
CRect CSelection::GetHandleRect(int nHandleID)
{
    ASSERT(m_pView != NULL);

    // Get the center of the handle in logical coords
    CPoint point = GetHandleLoc(nHandleID);

    // Convert point to client coords
    m_pView->WorkspaceToClient(point);

    // Calc CRect of handle in device coords
    CRect rect(point.x-3, point.y-3, point.x+3, point.y+3);

    m_pView->ClientToWorkspace(rect);

    return rect;
}

void CSelection::Invalidate()
{
    CRect rct = m_rect;
    rct = m_pObj->GetEnclosingRect();
    m_pView->InvalidateWorkspaceRect(&rct, FALSE);
}

//=---------------------------------------------------=//
// Static methods...

void CSelection::SetupTrackingDraw(CDC* pDC)
{
    c_nPrvROP2 = pDC->SetROP2(R2_XORPEN);
    c_pPrvPen = pDC->SelectObject(&c_penDot);
    c_pPrvBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
}

void CSelection::CleanUpTrackingDraw(CDC* pDC)
{
    pDC->SetROP2(c_nPrvROP2);
    pDC->SelectObject(c_pPrvPen);
    pDC->SelectObject(c_pPrvBrush);
}

/////////////////////////////////////////////////////////////////////
// Rectangle Selection Processing

void CSelRect::DrawTrackingImage(CDC* pDC, TrackMode eMode)
{
    SetupTrackingDraw(pDC);
    pDC->Rectangle(m_rect);
    CleanUpTrackingDraw(pDC);
}

HCURSOR CSelRect::GetHandleCursor(int nHandleID)
{
    LPCSTR id;
    switch (nHandleID)
    {
        case hitTopLeft:
        case hitBottomRight: id = IDC_SIZENWSE;  break;
        case hitTop:
        case hitBottom:      id = IDC_SIZENS;    break;
        case hitTopRight:
        case hitBottomLeft:  id = IDC_SIZENESW;  break;
        case hitRight:
        case hitLeft:        id = IDC_SIZEWE;    break;
        default: ASSERT(FALSE);
    }
    return AfxGetApp()->LoadStandardCursor(id);
}

// Returns handle location in logical coords.
CPoint CSelRect::GetHandleLoc(int nHandleID)
{
    int x, y;

    // This gets the center regardless of left/right and
    // top/bottom ordering
    int xCenter = m_rect.left + m_rect.Width() / 2;
    int yCenter = m_rect.top + m_rect.Height() / 2;

    switch (nHandleID)
    {
        case hitTopLeft:     x = m_rect.left;  y = m_rect.top;    break;
        case hitTopRight:    x = m_rect.right; y = m_rect.top;    break;
        case hitBottomRight: x = m_rect.right; y = m_rect.bottom; break;
        case hitBottomLeft:  x = m_rect.left;  y = m_rect.bottom; break;
        case hitTop:         x = xCenter;      y = m_rect.top;    break;
        case hitRight:       x = m_rect.right; y = yCenter;       break;
        case hitBottom:      x = xCenter;      y = m_rect.bottom; break;
        case hitLeft:        x = m_rect.left;  y = yCenter;       break;
        default: ASSERT(FALSE);
    }
    return CPoint(x, y);
}

void CSelRect::MoveHandle(int nHandle, CPoint point)
{
    switch (nHandle)
    {
        case hitTopLeft:
            m_rect.left = point.x; m_rect.top = point.y; break;
        case hitTopRight:
            m_rect.right = point.x; m_rect.top = point.y; break;
        case hitBottomRight:
            m_rect.right = point.x; m_rect.bottom = point.y; break;
        case hitBottomLeft:
            m_rect.left = point.x; m_rect.bottom = point.y; break;
        case hitTop:
            m_rect.top = point.y; break;
        case hitRight:
            m_rect.right = point.x; break;
        case hitBottom:
            m_rect.bottom = point.y; break;
        case hitLeft:
            m_rect.left = point.x; break;
        default: ASSERT(FALSE);
    }
}

void CSelRect::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    CRectObj* pObj = (CRectObj*)m_pObj;
    if (bInvalidate)
    {
        CRect rctA = pObj->GetEnclosingRect();
        CRect rctB = pObj->GetRect();
        m_pView->WorkspaceToClient(rctB);
        rctB.InflateRect(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rctB);
        rctA |= rctB;               // Make sure we erase the handles
        m_pView->InvalidateWorkspaceRect(&rctA);
    }
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        m_rect.NormalizeRect();
        pObj->SetRect(m_rect);
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        m_rect = pObj->GetRect();
    }
    if (bInvalidate)
    {
        CRect rct = pObj->GetEnclosingRect();
        m_pView->InvalidateWorkspaceRect(&rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Ellipse Selection Processing

void CSelEllipse::DrawTrackingImage(CDC* pDC, TrackMode eMode)
{
    SetupTrackingDraw(pDC);
    pDC->Ellipse(m_rect);
    CleanUpTrackingDraw(pDC);
}

/////////////////////////////////////////////////////////////////////
// Line Selection Processing

void CSelLine::DrawTrackingImage(CDC* pDC, TrackMode eMode)
{
    SetupTrackingDraw(pDC);
    pDC->MoveTo(m_rect.left, m_rect.top);
    pDC->LineTo(m_rect.right, m_rect.bottom);
    CleanUpTrackingDraw(pDC);
}

HCURSOR CSelLine::GetHandleCursor(int nHandleID)
{
    LPCSTR id;
    switch (nHandleID)
    {

        case hitPtA:
        case hitPtB:
            id = IDC_CROSS;
            break;
        default: ASSERT(FALSE);
    }
    return AfxGetApp()->LoadStandardCursor(id);
}

// Returns handle location in logical coords.
CPoint CSelLine::GetHandleLoc(int nHandleID)
{
    int x, y;

    switch (nHandleID)
    {
        case hitPtA: x = m_rect.left;  y = m_rect.top;    break;
        case hitPtB:
            x = m_rect.right;
            y = m_rect.bottom;
            // If the points are right on top of each other,
            // nudge this hit point over a bit.
            if (m_rect.left == m_rect.right &&
                m_rect.top == m_rect.bottom)
            {
                x++;
                y++;
            }
            break;
        default: ASSERT(FALSE);
    }
    return CPoint(x, y);
}

void CSelLine::MoveHandle(int nHandle, CPoint point)
{
    switch (nHandle)
    {
        case hitPtA:
            m_rect.left = point.x; m_rect.top = point.y; break;
        case hitPtB:
            m_rect.right = point.x; m_rect.bottom = point.y; break;
        default: ASSERT(FALSE);
    }
}

CRect CSelLine::GetRect()
{
    CRect rct = m_rect;
    rct.NormalizeRect();
    return rct;
}

void CSelLine::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    CLine* pObj = (CLine*)m_pObj;
    if (bInvalidate)
    {
        CRect rctA = pObj->GetEnclosingRect();
        CRect rctB;
        pObj->GetLine(rctB.left, rctB.top, rctB.right, rctB.bottom);
        rctB.NormalizeRect();
        m_pView->WorkspaceToClient(rctB);
        rctB.InflateRect(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rctB);
        rctA |= rctB;       // Make sure we erase the handles
        m_pView->InvalidateWorkspaceRect(&rctA, FALSE);
    }
    pObj->SetLine(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        pObj->SetLine(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        pObj->GetLine(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
    }
    if (bInvalidate)
    {
        CRect rct = pObj->GetEnclosingRect();
        m_pView->InvalidateWorkspaceRect(&rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Polygon Selection Processing

CSelPoly::CSelPoly(CBrdEditView* pView, CPolyObj* pObj) :
    CSelection(pView, pObj)
{
    ASSERT(pObj->m_pPnts);
    m_pPnts = new POINT[pObj->m_nPnts];
    memcpy(m_pPnts, pObj->m_pPnts, pObj->m_nPnts * sizeof(POINT));
    m_nPnts = pObj->m_nPnts;
}

void CSelPoly::DrawTrackingImage(CDC* pDC, TrackMode eMode)
{
    ASSERT(m_pPnts);
    SetupTrackingDraw(pDC);
    pDC->Polyline(m_pPnts, m_nPnts);
    CleanUpTrackingDraw(pDC);
}

HCURSOR CSelPoly::GetHandleCursor(int nHandleID)
{
    return AfxGetApp()->LoadStandardCursor(IDC_CROSS);
}

// Returns handle location in logical coords.
CPoint CSelPoly::GetHandleLoc(int nHandleID)
{
    ASSERT(m_pPnts);
    return CPoint(m_pPnts[nHandleID]);
}

void CSelPoly::MoveHandle(int nHandle, CPoint point)
{
    ASSERT(m_pPnts);
    m_pPnts[nHandle] = point;
}

void CSelPoly::Offset(CPoint ptDelta)
{
    ASSERT(m_pPnts);
    for (int i = 0; i < m_nPnts; i++)
    {
        m_pPnts[i].x += ptDelta.x;
        m_pPnts[i].y += ptDelta.y;
    }
}

CRect CSelPoly::GetRect()
{
    CRect rct;
    rct.SetRectEmpty();
    if (m_pPnts == NULL)
        return rct;
    int xmin = INT_MAX, xmax = INT_MIN, ymin = INT_MAX, ymax = INT_MIN;
    for (int i = 0; i < m_nPnts; i++)
    {
        xmin = min(xmin, m_pPnts[i].x);
        xmax = max(xmax, m_pPnts[i].x);
        ymin = min(ymin, m_pPnts[i].y);
        ymax = max(ymax, m_pPnts[i].y);
    }
    rct.SetRect(xmin, ymin, xmax, ymax);
    return rct;
}

void CSelPoly::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    CPolyObj* pObj = (CPolyObj*)m_pObj;
    if (bInvalidate)
    {
        CRect rctA = pObj->GetEnclosingRect();
        CRect rctB = pObj->GetRect();
        m_pView->WorkspaceToClient(rctB);
        rctB.InflateRect(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rctB);
        rctA |= rctB;               // Make sure we erase the handles
        m_pView->InvalidateWorkspaceRect(&rctA);
    }
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        pObj->SetNewPolygon(m_pPnts, m_nPnts);
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        if (m_nPnts) delete m_pPnts;
        m_pPnts = new POINT[pObj->m_nPnts];
        memcpy(m_pPnts, pObj->m_pPnts, pObj->m_nPnts * sizeof(POINT));
        m_nPnts = pObj->m_nPnts;
    }
    if (bInvalidate)
    {
        CRect rct = pObj->GetEnclosingRect();
        m_pView->InvalidateWorkspaceRect(&rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Generic Object Selection Processing

void CSelGeneric::DrawTrackingImage(CDC* pDC, TrackMode eMode)
{
    SetupTrackingDraw(pDC);
    pDC->Rectangle(m_rect);
    CleanUpTrackingDraw(pDC);
}

// Returns handle location in logical coords.
CPoint CSelGeneric::GetHandleLoc(int nHandleID)
{
    int x, y;

    switch (nHandleID)
    {
        case hitTopLeft:     x = m_rect.left;  y = m_rect.top;    break;
        case hitTopRight:    x = m_rect.right; y = m_rect.top;    break;
        case hitBottomRight: x = m_rect.right; y = m_rect.bottom; break;
        case hitBottomLeft:  x = m_rect.left;  y = m_rect.bottom; break;
        default: ASSERT(FALSE);
    }
    return CPoint(x, y);
}

void CSelGeneric::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    CDrawObj* pObj = (CRectObj*)m_pObj;
    if (bInvalidate)
    {
        CRect rct = pObj->GetRect();
        m_pView->WorkspaceToClient(rct);
        rct.InflateRect(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rct);
        m_pView->InvalidateWorkspaceRect(&rct);
    }
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        m_rect.NormalizeRect();
        pObj->SetRect(m_rect);
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        m_rect = pObj->GetRect();
    }
    if (bInvalidate)
    {
        CRect rct = pObj->GetEnclosingRect();
        m_pView->InvalidateWorkspaceRect(&rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Selection List Processing

int CSelList::HitTestHandles(CPoint point)
{
    // No support for multiselect handles.
    return GetCount() == 1 ? GetHead()->HitTestHandles(point) : hitNothing;
}

void CSelList::MoveHandle(UINT m_nHandle, CPoint point)
{
    // No support for multiselect handles.
    if (GetCount() == 1)
        GetHead()->MoveHandle(m_nHandle, point);
    CalcEnclosingRect();
}

CSelection* CSelList::AddObject(CDrawObj* pObj, BOOL bInvalidate)
{
    CSelection* pSel = pObj->CreateSelectProxy(m_pView);
    ASSERT(pSel != NULL);
    AddHead(pSel);
    CalcEnclosingRect();
    if (bInvalidate)
        pSel->InvalidateHandles();
    return pSel;
}

void CSelList::RemoveObject(CDrawObj* pObj, BOOL bInvalidate)
{
    POSITION pos1, pos2;
    pos1 = GetHeadPosition();
    while (pos1 != NULL)
    {
        pos2 = pos1;
        CSelection* pSel = (CSelection*)GetNext(pos1);
        if (pSel->m_pObj == pObj)
        {
            if (bInvalidate)
                pSel->InvalidateHandles();  // So view updates
            RemoveAt(pos2);
            delete pSel;
            CalcEnclosingRect();
            return;                     // Success
        }
    }
    ASSERT(FALSE);                      //// Object wasn't in list ////
}

BOOL CSelList::IsObjectSelected(CDrawObj* pObj) const
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        if (pSel->m_pObj == pObj)
            return TRUE;
    }
    return FALSE;
}

BOOL CSelList::IsDObjFlagSetInAllSelectedObjects(DWORD dwFlag) const
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        if ((pSel->m_pObj->GetDObjFlags() & dwFlag) == 0)
            return FALSE;
    }
    return TRUE;
}

BOOL CSelList::IsDObjFlagSetInSomeSelectedObjects(DWORD dwFlag) const
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        if ((pSel->m_pObj->GetDObjFlags() & dwFlag) != 0)
            return TRUE;
    }
    return FALSE;
}

static void SetDObjFlags(CDrawObj* pObj, DWORD dwFlags)
{
    pObj->SetDObjFlags(dwFlags);
}

void CSelList::SetDObjFlagInAllSelectedObjects(DWORD dwFlag)
{
    ForAllSelections(SetDObjFlags, dwFlag);
}

static void ClearDObjFlags(CDrawObj* pObj, DWORD dwFlags)
{
    pObj->ClearDObjFlags(dwFlags);
}

void CSelList::ClearDObjFlagInAllSelectedObjects(DWORD dwFlag)
{
    ForAllSelections(ClearDObjFlags, dwFlag);
}

BOOL CSelList::IsCopyToClipboardPossible() const
{
    // AT THIS POINT..THIS ONLY SUPPORTS COPYING A BITMAP TO THE CLIPBOARD
    if (!IsSingleSelect())
        return FALSE;
    CSelection* pSel = ((CSelList*)this)->GetHead();
    ASSERT(pSel != NULL);
    CDrawObj* pDObj = pSel->m_pObj;
    ASSERT(pDObj != NULL);
    return pDObj->GetType() == CDrawObj::drawBitmap;
}

void CSelList::CopyToClipboard()
{
    ASSERT(IsCopyToClipboardPossible());

    CSelection* pSel = ((CSelList*)this)->GetHead();
    ASSERT(pSel != NULL);
    CBitmapImage* pDObj = (CBitmapImage*)pSel->m_pObj;
    ASSERT(pDObj != NULL);
    ASSERT(pDObj->GetType() == CDrawObj::drawBitmap);
    SetClipboardBitmap(m_pView, &pDObj->m_bitmap);
}

void CSelList::Open()
{
    if (!IsSingleSelect())
        return;
    CSelection* pSel = ((CSelList*)this)->GetHead();
    ASSERT(pSel != NULL);
    CText* pDObj = (CText*)pSel->m_pObj;
    ASSERT(pDObj != NULL);
    // AT THIS POINT..THIS ONLY SUPPORTS EDITTING TEXT
    if (pDObj->GetType() != CDrawObj::drawText)
        return;
    m_pView->DoEditTextDrawingObject(pDObj);
    pSel->InvalidateHandles();
    pSel->m_rect = pDObj->GetEnclosingRect();
    pSel->InvalidateHandles();
}

// Assumes RECTs are normalized!!
void CbUnionRect(RECT* pRctDst, RECT* pRctSrc1, RECT* pRctSrc2)
{
    pRctDst->left = min(pRctSrc1->left, pRctSrc2->left);
    pRctDst->top = min(pRctSrc1->top, pRctSrc2->top);
    pRctDst->right = max(pRctSrc1->right, pRctSrc2->right);
    pRctDst->bottom = max(pRctSrc1->bottom, pRctSrc2->bottom);
}

void CSelList::CalcEnclosingRect()
{
    m_rctEncl.SetRectEmpty();
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        if (m_rctEncl.IsRectNull())
            m_rctEncl = pSel->GetRect();
        else
            CbUnionRect(&m_rctEncl, &m_rctEncl, &pSel->GetRect());
    }
}

void CSelList::Offset(CPoint ptDelta)
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        pSel->Offset(ptDelta);
    }
    CalcEnclosingRect();
}

// Called by view OnDraw(). This entry makes it possible
// to turn off handles during a drag operation.

void CSelList::OnDraw(CDC *pDC)
{
    if (m_eTrkMode == trkSelected)
        DrawTracker(pDC);
}

void CSelList::DrawTracker(CDC *pDC, TrackMode eTrkMode)
{
    if (eTrkMode != trkCurrent)
        m_eTrkMode = eTrkMode;

    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        pSel->DrawTracker(pDC, m_eTrkMode);
    }
}

void CSelList::InvalidateListHandles(BOOL bUpdate)
{
    BOOL bFoundOne = FALSE;

    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        pSel->InvalidateHandles();
        bFoundOne = TRUE;
    }
    if (bFoundOne && bUpdate)
        m_pView->UpdateWindow();
}

void CSelList::InvalidateList(BOOL bUpdate)
{
    BOOL bFoundOne = FALSE;

    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        pSel->Invalidate();
        bFoundOne = TRUE;
    }
    if (bFoundOne && bUpdate)
        m_pView->UpdateWindow();
}

void CSelList::PurgeList(BOOL bInvalidate)
{
    if (bInvalidate)
        InvalidateListHandles();
    while (!IsEmpty())
        delete (CSelection*)RemoveHead();
}

// Called after a move or size operation to change underlying
// objects to new postions/sizes.

void CSelList::UpdateObjects(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        pSel->UpdateObject(bInvalidate, bUpdateObjectExtent);
    }
}

void CSelList::ForAllSelections(void (*pFunc)(CDrawObj* pObj, DWORD dwUser),
    DWORD dwUserVal)
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        pFunc(pSel->m_pObj, dwUserVal);
    }
}


