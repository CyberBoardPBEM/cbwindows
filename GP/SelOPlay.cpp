// SelOPlay.cpp
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
#include    "Gp.h"
#include    "GamDoc.h"
#include    "GMisc.h"
#include    "DrawObj.h"
#include    "PPieces.h"
#include    "SelOPlay.h"
#include    "PBoard.h"
#include    "VwPbrd.h"

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

void CHandleList::AddHandle(POINT pntNew)
{
    POSITION pos = GetHeadPosition();

    // We assume a point value no larger than 16 bits and
    // are never negative so we can make efficient comparisons.
    DWORD dwPntNew = ((DWORD)pntNew.x << 16) + (WORD)pntNew.y;

    while (pos != NULL)
    {
        POSITION posPrev = pos;
        POINT pntCur = GetNext(pos);
        if (pntNew.x == pntCur.x && pntNew.y == pntCur.y)
            return;                     // Already in the list

        DWORD dwPntCur = ((DWORD)pntCur.x << 16) + (WORD)pntCur.y;
        if (dwPntNew < dwPntCur)
        {
            InsertBefore(posPrev, pntNew);
            return;
        }
    }
    AddTail(pntNew);         // Add to end
}

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

void CSelection::Open()
{
    if (m_pObj->GetType() == CDrawObj::drawMarkObj ||
        m_pObj->GetType() == CDrawObj::drawPieceObj)
    {
        m_pView->SendMessage(WM_COMMAND, MAKEWPARAM(ID_EDIT_ELEMENT_TEXT, 0));
    }
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
// Line Selection Processing

void CSelLine::AddHandles(CHandleList& listHandles)
{
    listHandles.AddHandle(GetHandleLoc(hitPtA));
    listHandles.AddHandle(GetHandleLoc(hitPtB));
}

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
        case hitPtB: x = m_rect.right; y = m_rect.bottom; break;
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
        pObj->GetLine(rctB);
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
        pObj->GetLine(m_rect);
    }
    if (bInvalidate)
    {
        CRect rct = pObj->GetEnclosingRect();
        m_pView->InvalidateWorkspaceRect(&rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Generic Object Selection Processing

void CSelGeneric::AddHandles(CHandleList& listHandles)
{
    listHandles.AddHandle(GetHandleLoc(hitTopLeft));
    listHandles.AddHandle(GetHandleLoc(hitTopRight));
    listHandles.AddHandle(GetHandleLoc(hitBottomRight));
    listHandles.AddHandle(GetHandleLoc(hitBottomLeft));
}

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

void CSelList::Open()
{
    if (!IsSingleSelect())
        return;
    CSelection* pSel = ((CSelList*)this)->GetHead();
    ASSERT(pSel != NULL);
    pSel->Open();
}

int CSelList::HitTestHandles(CPoint point)
{
    // No support for multiselect handles.
    return GetCount() == 1 ? GetHead()->HitTestHandles(point) : hitNothing;
}

void CSelList::RegenerateHandleList()
{
    m_listHandles.RemoveAll();
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        pSel->AddHandles(m_listHandles);
    }
}

void CSelList::MoveHandle(UINT m_nHandle, CPoint point)
{
    // No support for multiselect handles.
    if (GetCount() == 1)
        GetHead()->MoveHandle(m_nHandle, point);
    RegenerateHandleList();
    CalcEnclosingRect();
}

CSelection* CSelList::AddObject(CDrawObj* pObj, BOOL bInvalidate)
{
    // Check if the object is already selected. If it is
    // remove it and then add the new definition.
    if (FindObject(pObj) != NULL)
        RemoveObject(pObj, TRUE);
    CSelection* pSel = pObj->CreateSelectProxy(m_pView);
    ASSERT(pSel != NULL);
    AddHead(pSel);
    CalcEnclosingRect();
    pSel->AddHandles(m_listHandles);
    if (bInvalidate)
        pSel->InvalidateHandles();
    return pSel;
}

void CSelList::RemoveObject(CDrawObj* pObj, BOOL bInvalidate)
{
    POSITION pos1, pos2;
    pos1 = GetHeadPosition();
    if (pObj == m_pobjSnapReference)
        m_pobjSnapReference = NULL;
    while (pos1 != NULL)
    {
        pos2 = pos1;
        CSelection* pSel = (CSelection*)GetNext(pos1);
        if (pSel->m_pObj == pObj)
        {
            if (bInvalidate)
                pSel->InvalidateHandles();  // So view updates
            RemoveAt(pos2);
            RegenerateHandleList();
            if (pObj->GetType() == CDrawObj::drawPieceObj ||
                    pObj->GetType() == CDrawObj::drawMarkObj)
                m_pView->NotifySelectListChange();
            delete pSel;
            CalcEnclosingRect();
            return;                     // Success
        }
    }
    ASSERT(FALSE);                      //// Object wasn't in list ////
}

BOOL CSelList::IsObjectSelected(const CDrawObj* pObj) const
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

void CSelList::SetSnapReferenceObject(CDrawObj* pObj)
{
    m_pobjSnapReference = pObj;
}

CRect CSelList::GetSnapReferenceRect()
{
    CRect rct;
    rct.SetRectEmpty();

    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        if (pSel->m_pObj == m_pobjSnapReference)
        {
            rct = pSel->GetRect();
            break;
        }
    }
    if (rct.IsRectEmpty())
        rct = GetEnclosingRect();
    return rct;
}

void CSelList::CalcEnclosingRect()
{
    m_rctEncl.SetRectEmpty();
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        m_rctEncl |= pSel->GetRect();
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

    if (m_eTrkMode == trkSelected)
    {
        // Drawing selection handles. Draw them using the
        // handle list so when an even number of identical
        // points don't XOR themselves out of existance.
        POSITION pos = m_listHandles.GetHeadPosition();
        while (pos != NULL)
        {
            POINT pnt = m_listHandles.GetNext(pos);
            pDC->PatBlt(pnt.x-3, pnt.y-3, 6, 6, DSTINVERT);
        }
    }
    else
    {
        // Not drawing handles so let the selection objects handle it.
        POSITION pos = GetHeadPosition();
        while (pos != NULL)
        {
            CSelection* pSel = (CSelection*)GetNext(pos);
            pSel->DrawTracker(pDC, m_eTrkMode);
        }
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
    {
        CSelection* pSel = (CSelection*)RemoveHead();
        delete pSel;
    }

    RegenerateHandleList();

    m_eTrkMode = trkSelected;                   // Reset tracking mode

    m_pobjSnapReference = NULL;

    m_pView->NotifySelectListChange();
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
    RegenerateHandleList();
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

CRect CSelList::GetPiecesEnclosingRect(BOOL bIncludeMarkers /* = TRUE */)
{
    CRect rct;
    rct.SetRectEmpty();
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj->GetType() == CDrawObj::drawPieceObj ||
                bIncludeMarkers && pSel->m_pObj->GetType() == CDrawObj::drawMarkObj)
            rct |= pSel->GetRect();
    }
    return rct;
}

void CSelList::LoadListWithObjectPtrs(CPtrList* pList,
    BOOL bPiecesOnly /* = FALSE*/, BOOL bVisualOrder /* = FALSE */)
{
    pList->RemoveAll();
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (bPiecesOnly)
        {
            if (pSel->m_pObj->GetType() == CDrawObj::drawPieceObj)
                pList->AddTail(pSel->m_pObj);
        }
        else
            pList->AddTail(pSel->m_pObj);
    }
    // This is a backdoor cheat....Since we know the original view and
    // hence the board associated, we can call the draw list
    // method the arrange the pieces in the proper visual order.
    ASSERT(m_pView != NULL);
    CDrawList* pDwg = m_pView->GetPlayBoard()->GetPieceList();
    ASSERT(pDwg != NULL);
    if (bVisualOrder)
        pDwg->ArrangeObjectListInVisualOrder(pList);
    else
        pDwg->ArrangeObjectListInDrawOrder(pList);
}

BOOL CSelList::HasPieces() const
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj->GetType() == CDrawObj::drawPieceObj)
            return TRUE;
    }
    return FALSE;
}

BOOL CSelList::HasNonOwnedPieces() const
{
    CPieceTable* pPTbl = m_pView->GetDocument()->GetPieceTable();
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj* pPObj = (CPieceObj*)pSel->m_pObj;
            if (pPTbl->GetOwnerMask(pPObj->m_pid) == 0)
                return TRUE;
        }
    }
    return FALSE;
}

BOOL CSelList::HasOwnedPieces() const
{
    CPieceTable* pPTbl = m_pView->GetDocument()->GetPieceTable();
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj* pPObj = (CPieceObj*)pSel->m_pObj;
            if (pPTbl->GetOwnerMask(pPObj->m_pid) != 0)
                return TRUE;
        }
    }
    return FALSE;
}

BOOL CSelList::HasOwnedPiecesNotMatching(DWORD dwOwnerMask) const
{
    CPieceTable* pPTbl = m_pView->GetDocument()->GetPieceTable();
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj* pPObj = (CPieceObj*)pSel->m_pObj;
            if (pPTbl->IsPieceOwned(pPObj->m_pid) &&
               !pPTbl->IsPieceOwnedBy(pPObj->m_pid, dwOwnerMask))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL CSelList::HasMarkers() const
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj->GetType() == CDrawObj::drawMarkObj)
            return TRUE;
    }
    return FALSE;
}

BOOL CSelList::Has2SidedPieces() const
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            if (m_pView->GetDocument()->GetPieceTable()->Is2Sided(
                    ((CPieceObj*)(pSel->m_pObj))->m_pid))
                return TRUE;
        }
    }
    return FALSE;
}

void CSelList::CountDObjFlags(DWORD dwFlagBits, int& nSet, int& nCleared)
{
    nSet = 0;
    nCleared = 0;
    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        if (pSel->m_pObj->GetDObjFlags() & dwFlagBits)
            nSet++;
        else
            nCleared++;
    }
}

void CSelList::DeselectIfDObjFlagsSet(DWORD dwFlagBits)
{
    std::vector<CDrawObj*> tblDeselObjs;
    // First find them...
    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        if (pSel->m_pObj->GetDObjFlags() & dwFlagBits)
            tblDeselObjs.push_back(pSel->m_pObj);
    }
    // Then deselect them...
    for (size_t i = 0; i < tblDeselObjs.size(); i++)
        RemoveObject(tblDeselObjs.at(i), TRUE);
}

CSelection* CSelList::FindObject(CDrawObj* pObj) const
{
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        POSITION posLast = pos;
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj == pObj)
            return pSel;
    }
    return NULL;
}

void CSelList::LoadTableWithPieceIDs(std::vector<PieceID>& pTbl, BOOL bVisualOrder /* = TRUE */)
{
    pTbl.clear();
    pTbl.reserve(value_preserving_cast<size_t>(GetSize()));
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj->GetType() == CDrawObj::drawPieceObj)
            pTbl.push_back(static_cast<CPieceObj*>(pSel->m_pObj)->m_pid);
    }
    if (pTbl.empty())
        return;
    // This is a bit of a cheat....Since we know the originating view and
    // thus the board associated with it, we can call the draw list
    // method the arrange the pieces in the proper visual order.
    ASSERT(m_pView != NULL);
    CDrawList* pDwg = m_pView->GetPlayBoard()->GetPieceList();
    ASSERT(pDwg != NULL);
    if (bVisualOrder)
        pDwg->ArrangePieceTableInVisualOrder(pTbl);
    else
        pDwg->ArrangePieceTableInDrawOrder(pTbl);
}

void CSelList::LoadTableWithOwnerStatePieceIDs(std::vector<PieceID>& pTbl, LoadFilter eWantOwned,
    BOOL bVisualOrder /* = TRUE */)
{
    CPieceTable* pPTbl = m_pView->GetDocument()->GetPieceTable();

    pTbl.clear();
    pTbl.reserve(value_preserving_cast<size_t>(GetSize()));
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        if (pSel->m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj* pPObj = (CPieceObj*)pSel->m_pObj;
            if (eWantOwned == LF_OWNED    && pPTbl->GetOwnerMask(pPObj->m_pid) != 0 ||
                eWantOwned == LF_NOTOWNED && pPTbl->GetOwnerMask(pPObj->m_pid) == 0 ||
                eWantOwned == LF_BOTH)
            {
                pTbl.push_back(pPObj->m_pid);
            }
        }
    }
    if (pTbl.empty())
        return;
    // This is a bit of a cheat....Since we know the originating view and
    // thus the board associated with it, we can call the draw list
    // method the arrange the pieces in the proper visual order.
    CDrawList* pDwg = m_pView->GetPlayBoard()->GetPieceList();
    ASSERT(pDwg != NULL);
    if (bVisualOrder)
        pDwg->ArrangePieceTableInVisualOrder(pTbl);
    else
        pDwg->ArrangePieceTableInDrawOrder(pTbl);
}

// Loads the table with the CDrawObj pointers of playing pieces and markers.
void CSelList::LoadTableWithObjectPtrs(std::vector<CB::not_null<CDrawObj*>>& pTbl, BOOL bVisualOrder /* = TRUE */)
{
    pTbl.clear();
    POSITION pos = GetHeadPosition();
    while (pos != NULL)
    {
        CSelection* pSel = (CSelection*)GetNext(pos);
        ASSERT(pSel != NULL);
        CDrawObj::CDrawObjType type = pSel->m_pObj->GetType();
        if (type == CDrawObj::drawPieceObj || type == CDrawObj::drawMarkObj)
            pTbl.push_back(pSel->m_pObj);
    }
    if (pTbl.empty())
        return;
    // This is a bit of a cheat....Since we know the originating view and
    // thus the board associated with it, we can call the draw list
    // method the arrange the pieces in the proper visual order.
    ASSERT(m_pView != NULL);
    CDrawList* pDwg = m_pView->GetPlayBoard()->GetPieceList();
    ASSERT(pDwg != NULL);
    if (bVisualOrder)
        pDwg->ArrangeObjectPtrTableInVisualOrder(pTbl);
    else
        pDwg->ArrangeObjectPtrTableInDrawOrder(pTbl);
}

