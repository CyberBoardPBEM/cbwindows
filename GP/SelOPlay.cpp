// SelOPlay.cpp
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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

void CSelection::DrawTracker(CDC& pDC, TrackMode eMode) const
{
    if (eMode == trkSelected)
        DrawHandles(pDC);
    else if (eMode == trkMoving || eMode == trkSizing)
        DrawTrackingImage(pDC, eMode);
}

void CSelection::DrawHandles(CDC& pDC) const
{
    int n = GetHandleCount();
    for (int i = 0; i < n; i++)
    {
        CRect rect = GetHandleRect(i);
        pDC.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), DSTINVERT);
    }
}

int CSelection::HitTestHandles(CPoint point) const
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
CRect CSelection::GetHandleRect(int nHandleID) const
{
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
        m_pView->SendMessage(WM_COMMAND, MAKEWPARAM(uint16_t(ID_EDIT_ELEMENT_TEXT), uint16_t(0)));
    }
}

//=---------------------------------------------------=//
// Static methods...

void CSelection::SetupTrackingDraw(CDC& pDC)
{
    c_nPrvROP2 = pDC.SetROP2(R2_XORPEN);
    c_pPrvPen = pDC.SelectObject(&c_penDot);
    c_pPrvBrush = (CBrush*)pDC.SelectStockObject(NULL_BRUSH);
}

void CSelection::CleanUpTrackingDraw(CDC& pDC)
{
    pDC.SetROP2(c_nPrvROP2);
    pDC.SelectObject(c_pPrvPen);
    pDC.SelectObject(c_pPrvBrush);
}

/////////////////////////////////////////////////////////////////////
// Line Selection Processing

void CSelLine::AddHandles(CHandleList& listHandles)
{
    listHandles.AddHandle(GetHandleLoc(hitPtA));
    listHandles.AddHandle(GetHandleLoc(hitPtB));
}

void CSelLine::DrawTrackingImage(CDC& pDC, TrackMode eMode) const
{
    SetupTrackingDraw(pDC);
    pDC.MoveTo(m_rect.left, m_rect.top);
    pDC.LineTo(m_rect.right, m_rect.bottom);
    CleanUpTrackingDraw(pDC);
}

HCURSOR CSelLine::GetHandleCursor(int nHandleID) const
{
    const CB::string::value_type* id;
    switch (nHandleID)
    {

        case hitPtA:
        case hitPtB:
            id = IDC_CROSS;
            break;
        default: ASSERT(FALSE);
            id = nullptr;
    }
    return AfxGetApp()->LoadStandardCursor(id);
}

// Returns handle location in logical coords.
CPoint CSelLine::GetHandleLoc(int nHandleID) const
{
    int x, y;

    switch (nHandleID)
    {
        case hitPtA: x = m_rect.left;  y = m_rect.top;    break;
        case hitPtB: x = m_rect.right; y = m_rect.bottom; break;
        default: ASSERT(FALSE);
            x = -1; y = -1;
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

CRect CSelLine::GetRect() const
{
    CRect rct = m_rect;
    rct.NormalizeRect();
    return rct;
}

void CSelLine::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    CLine& pObj = static_cast<CLine&>(*m_pObj);
    if (bInvalidate)
    {
        CRect rctA = pObj.GetEnclosingRect();
        CRect rctB;
        pObj.GetLine(rctB);
        rctB.NormalizeRect();
        m_pView->WorkspaceToClient(rctB);
        rctB.InflateRect(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rctB);
        rctA |= rctB;       // Make sure we erase the handles
        m_pView->InvalidateWorkspaceRect(&rctA, FALSE);
    }
    pObj.SetLine(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        pObj.SetLine(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        pObj.GetLine(m_rect);
    }
    if (bInvalidate)
    {
        CRect rct = pObj.GetEnclosingRect();
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

void CSelGeneric::DrawTrackingImage(CDC& pDC, TrackMode eMode) const
{
    SetupTrackingDraw(pDC);
    pDC.Rectangle(m_rect);
    CleanUpTrackingDraw(pDC);
}

// Returns handle location in logical coords.
CPoint CSelGeneric::GetHandleLoc(int nHandleID) const
{
    int x, y;

    switch (nHandleID)
    {
        case hitTopLeft:     x = m_rect.left;  y = m_rect.top;    break;
        case hitTopRight:    x = m_rect.right; y = m_rect.top;    break;
        case hitBottomRight: x = m_rect.right; y = m_rect.bottom; break;
        case hitBottomLeft:  x = m_rect.left;  y = m_rect.bottom; break;
        default: ASSERT(FALSE);
            x = -1; y = -1;
    }
    return CPoint(x, y);
}

void CSelGeneric::UpdateObject(BOOL bInvalidate,
    BOOL bUpdateObjectExtent /* = TRUE */)
{
    if (bInvalidate)
    {
        CRect rct = m_pObj->GetRect();
        m_pView->WorkspaceToClient(rct);
        rct.InflateRect(handleHalfWidth, handleHalfWidth);
        m_pView->ClientToWorkspace(rct);
        m_pView->InvalidateWorkspaceRect(&rct);
    }
    if (bUpdateObjectExtent)
    {
        // Normal case is when object needs to be updated.
        m_rect.NormalizeRect();
        m_pObj->SetRect(m_rect);
    }
    else
    {
        // Degenerate case when an operation on an object changed
        // its size and the select rect must reflect this.
        m_rect = m_pObj->GetRect();
    }
    if (bInvalidate)
    {
        CRect rct = m_pObj->GetEnclosingRect();
        m_pView->InvalidateWorkspaceRect(&rct);
    }
}

/////////////////////////////////////////////////////////////////////
// Selection List Processing

void CSelList::Open()
{
    if (!IsSingleSelect())
        return;
    CSelection& pSel = *front();
    pSel.Open();
}

int CSelList::HitTestHandles(CPoint point) const
{
    // No support for multiselect handles.
    return IsSingleSelect() ? front()->HitTestHandles(point) : hitNothing;
}

void CSelList::RegenerateHandleList()
{
    m_listHandles.RemoveAll();
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pSel.AddHandles(m_listHandles);
    }
}

void CSelList::MoveHandle(int m_nHandle, CPoint point)
{
    // No support for multiselect handles.
    if (IsSingleSelect())
        front()->MoveHandle(m_nHandle, point);
    RegenerateHandleList();
    CalcEnclosingRect();
}

CSelection& CSelList::AddObject(CDrawObj& pObj, BOOL bInvalidate)
{
    // Check if the object is already selected. If it is
    // remove it and then add the new definition.
    if (FindObject(pObj) != NULL)
        RemoveObject(pObj, TRUE);
    {
        OwnerPtr<CSelection> pSel = pObj.CreateSelectProxy(*m_pView);
        push_front(std::move(pSel));
    }
    CSelection& pSel = *front();
    CalcEnclosingRect();
    pSel.AddHandles(m_listHandles);
    if (bInvalidate)
        pSel.InvalidateHandles();
    return pSel;
}

void CSelList::RemoveObject(const CDrawObj& pObj, BOOL bInvalidate)
{
    if (&pObj == m_pobjSnapReference)
        m_pobjSnapReference = NULL;
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        if (pSel.m_pObj == &pObj)
        {
            if (bInvalidate)
                pSel.InvalidateHandles();  // So view updates
            RegenerateHandleList();
            if (pObj.GetType() == CDrawObj::drawPieceObj ||
                    pObj.GetType() == CDrawObj::drawMarkObj)
                m_pView->NotifySelectListChange();
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

void CSelList::SetSnapReferenceObject(CDrawObj* pObj)
{
    m_pobjSnapReference = pObj;
}

CRect CSelList::GetSnapReferenceRect() const
{
    CRect rct;
    rct.SetRectEmpty();

    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (m_pobjSnapReference &&
            pSel.m_pObj == m_pobjSnapReference)
        {
            rct = pSel.GetRect();
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
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        m_rctEncl |= pSel.GetRect();
    }
}

void CSelList::Offset(CPoint ptDelta)
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

void CSelList::OnDraw(CDC& pDC)
{
    if (m_eTrkMode == trkSelected)
        DrawTracker(pDC);
}

void CSelList::DrawTracker(CDC& pDC, TrackMode eTrkMode)
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
            pDC.PatBlt(pnt.x-3, pnt.y-3, 6, 6, DSTINVERT);
        }
    }
    else
    {
        // Not drawing handles so let the selection objects handle it.
        for (iterator pos = begin() ; pos != end() ; ++pos)
        {
            CSelection& pSel = **pos;
            pSel.DrawTracker(pDC, m_eTrkMode);
        }
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
        m_pView->UpdateWindow();
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
        m_pView->UpdateWindow();
}

void CSelList::PurgeList(BOOL bInvalidate)
{
    if (bInvalidate)
        InvalidateListHandles();
    clear();

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
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pSel.UpdateObject(bInvalidate, bUpdateObjectExtent);
    }
    RegenerateHandleList();
}

void CSelList::ForAllSelections(std::function<void (CDrawObj& pObj)> pFunc)
{
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pFunc(*pSel.m_pObj);
    }
}

CRect CSelList::GetPiecesEnclosingRect(BOOL bIncludeMarkers /* = TRUE */) const
{
    CRect rct;
    rct.SetRectEmpty();
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj->GetType() == CDrawObj::drawPieceObj ||
                bIncludeMarkers && pSel.m_pObj->GetType() == CDrawObj::drawMarkObj)
            rct |= pSel.GetRect();
    }
    return rct;
}

// Loads the table with the CDrawObj pointers of playing pieces and markers.
void CSelList::LoadTableWithObjectPtrs(std::vector<CB::not_null<CDrawObj*>>& pTbl,
    CSelList::ObjTypes objTypes, BOOL bVisualOrder)
{
    pTbl.clear();
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        bool match;
        switch (objTypes) {
            case otPieces:
                match = pSel.m_pObj->GetType() == CDrawObj::drawPieceObj;
                break;
            case otPiecesMarks: {
                CDrawObj::CDrawObjType type = pSel.m_pObj->GetType();
                match = type == CDrawObj::drawPieceObj || type == CDrawObj::drawMarkObj;
                break;
            }
            case otAll:
                match = true;
                break;
            default:
                AfxThrowInvalidArgException();
        }
        if (match)
            pTbl.push_back(pSel.m_pObj.get());
    }
    if (pTbl.empty())
        return;
    // This is a bit of a cheat....Since we know the originating view and
    // thus the board associated with it, we can call the draw list
    // method the arrange the pieces in the proper visual order.
    CDrawList* pDwg = m_pView->GetPlayBoard()->GetPieceList();
    ASSERT(pDwg != NULL);
    if (bVisualOrder)
        pDwg->ArrangeObjectPtrTableInVisualOrder(pTbl);
    else
        pDwg->ArrangeObjectPtrTableInDrawOrder(pTbl);
}

BOOL CSelList::HasPieces() const
{
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj->GetType() == CDrawObj::drawPieceObj)
            return TRUE;
    }
    return FALSE;
}

BOOL CSelList::HasNonOwnedPieces() const
{
    const CPieceTable& pPTbl = m_pView->GetDocument()->GetPieceTable();
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            const CPieceObj& pPObj = static_cast<const CPieceObj&>(*pSel.m_pObj);
            if (pPTbl.GetOwnerMask(pPObj.m_pid) == OWNER_MASK_SPECTATOR)
                return TRUE;
        }
    }
    return FALSE;
}

BOOL CSelList::HasOwnedPieces() const
{
    const CPieceTable& pPTbl = m_pView->GetDocument()->GetPieceTable();
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            const CPieceObj& pPObj = static_cast<const CPieceObj&>(*pSel.m_pObj);
            if (pPTbl.GetOwnerMask(pPObj.m_pid) != OWNER_MASK_SPECTATOR)
                return TRUE;
        }
    }
    return FALSE;
}

BOOL CSelList::HasOwnedPiecesNotMatching(PlayerMask dwOwnerMask) const
{
    const CPieceTable& pPTbl = m_pView->GetDocument()->GetPieceTable();
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            const CPieceObj& pPObj = static_cast<const CPieceObj&>(*pSel.m_pObj);
            if (pPTbl.IsPieceOwned(pPObj.m_pid) &&
               !pPTbl.IsPieceOwnedBy(pPObj.m_pid, dwOwnerMask))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL CSelList::HasMarkers() const
{
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj->GetType() == CDrawObj::drawMarkObj)
            return TRUE;
    }
    return FALSE;
}

BOOL CSelList::HasFlippablePieces() const
{
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            if (m_pView->GetDocument()->GetPieceTable().GetSides(
                    static_cast<const CPieceObj&>(*pSel.m_pObj).m_pid) >= size_t(2))
                return TRUE;
        }
    }
    return FALSE;
}

void CSelList::CountDObjFlags(DWORD dwFlagBits, int& nSet, int& nCleared) const
{
    nSet = 0;
    nCleared = 0;
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj->GetDObjFlags() & dwFlagBits)
            nSet++;
        else
            nCleared++;
    }
}

void CSelList::DeselectIfDObjFlagsSet(DWORD dwFlagBits)
{
    std::vector<CDrawObj*> tblDeselObjs;
    // First find them...
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        if (pSel.m_pObj->GetDObjFlags() & dwFlagBits)
            tblDeselObjs.push_back(pSel.m_pObj.get());
    }
    // Then deselect them...
    for (size_t i = size_t(0); i < tblDeselObjs.size(); i++)
        RemoveObject(*tblDeselObjs.at(i), TRUE);
}

const CSelection* CSelList::FindObject(const CDrawObj& pObj) const
{
    for (const_iterator pos = begin() ; pos != end() ; ++pos)
    {
        const CSelection& pSel = **pos;
        if (pSel.m_pObj == &pObj)
            return &pSel;
    }
    return NULL;
}

void CSelList::LoadTableWithPieceIDs(std::vector<PieceID>& pTbl, BOOL bVisualOrder /* = TRUE */)
{
    pTbl.clear();
    pTbl.reserve(size());
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        if (pSel.m_pObj->GetType() == CDrawObj::drawPieceObj)
            pTbl.push_back(static_cast<CPieceObj&>(*pSel.m_pObj).m_pid);
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

void CSelList::LoadTableWithOwnerStatePieceIDs(std::vector<PieceID>& pTbl, LoadFilter eWantOwned,
    BOOL bVisualOrder /* = TRUE */)
{
    CPieceTable& pPTbl = m_pView->GetDocument()->GetPieceTable();

    pTbl.clear();
    pTbl.reserve(size());
    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CSelection& pSel = **pos;
        if (pSel.m_pObj->GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj& pPObj = static_cast<CPieceObj&>(*pSel.m_pObj);
            if (eWantOwned == LF_OWNED    && pPTbl.GetOwnerMask(pPObj.m_pid) != OWNER_MASK_SPECTATOR ||
                eWantOwned == LF_NOTOWNED && pPTbl.GetOwnerMask(pPObj.m_pid) == OWNER_MASK_SPECTATOR ||
                eWantOwned == LF_BOTH)
            {
                pTbl.push_back(pPObj.m_pid);
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

