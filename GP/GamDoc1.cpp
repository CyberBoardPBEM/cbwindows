// GamDoc1.cpp - Command and Control for document wide operations
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
#include    "FrmMain.h"
#include    "GamDoc.h"

#include    "Board.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "Marks.h"
#include    "Trays.h"
#include    "GameBox.h"
#include    "MoveMgr.h"
#include    "DlgItray.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////
// (RECORDS)
void CGamDoc::PlacePieceOnBoard(CPoint pnt, PieceID pid, CPlayBoard *pPBrd)
{
    RecordPieceMoveToBoard(pPBrd, pid, pnt, placeTop);// Record processing

    RemovePieceFromCurrentLocation(pid, TRUE);
    CPieceObj& pObj = pPBrd->AddPiece(pnt, pid);

    // If the destination is owned, force the piece to take on the
    // same ownership. Otherwise, leave it's ownership state alone.
    if (pPBrd->IsOwned())
        GetPieceTable()->SetOwnerMask(pid, pPBrd->GetOwnerMask());

    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = &pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)
void CGamDoc::PlacePieceInTray(PieceID pid, CTraySet& pYGrp, size_t nPos)
{
    RecordPieceMoveToTray(pYGrp, pid, nPos);        // Record processing

    BOOL bTrayHintAllowed = TRUE;
    CTraySet *pCurYGrp = FindPieceInTray(pid);
    if (pCurYGrp == &pYGrp && nPos != Invalid_v<size_t>)
    {
        // May need to mess with 'nPos' if moving the piece
        // to a location higher up in the list.
        size_t nIdx = pCurYGrp->GetPieceIDIndex(pid);
        ASSERT(nIdx != Invalid_v<size_t>);
        if (nPos > nIdx)
            nPos--;
        bTrayHintAllowed = FALSE;
    }
    // OK...continue with the process.
    RemovePieceFromCurrentLocation(pid, TRUE, bTrayHintAllowed);

    // Force the piece to take on the same ownership as the
    // tray has.
    GetPieceTable()->SetOwnerMask(pid, pYGrp.IsOwned() ? pYGrp.GetOwnerMask() : 0);

    pYGrp.AddPieceID(pid, nPos);

    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pTray = &pYGrp;
        UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
    }
    SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)

void CGamDoc::PlaceObjectOnBoard(CPlayBoard *pPBrd, CDrawObj::OwnerPtr opObj,
    CSize sizeDelta, PlacePos ePos /* = placeDefault */)
{
    CDrawObj& pObj = *opObj;
    CDrawList* pDwg = pPBrd->GetPieceList();
    ASSERT(pDwg);

    // Record processing
    if (pObj.GetType() == CDrawObj::drawPieceObj)
    {
        CPieceObj& pPObj = static_cast<CPieceObj&>(pObj);
        CRect rctPce = pPObj.GetRect();
        RecordPieceMoveToBoard(pPBrd, pPObj.m_pid, GetMidRect(rctPce) +
            sizeDelta, ePos);
    }
    else if (pObj.GetType() == CDrawObj::drawMarkObj)
    {
        CMarkObj& pMObj = static_cast<CMarkObj&>(pObj);
        CRect rctMrk = pMObj.GetRect();
        RecordMarkMoveToBoard(pPBrd, pMObj.GetObjectID(), pMObj.m_mid,
            GetMidRect(rctMrk) + sizeDelta, ePos);
    }
    BOOL bOnBoard = pDwg->HasObject(&pObj);
    if (ePos == placeDefault && bOnBoard)
    {
        if (!IsQuietPlayback())
        {
            // Cause it's former location to be invalidated...
            CGamDocHint hint;
            hint.m_pPBoard = pPBrd;
            hint.m_pDrawObj = &pObj;
            UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
        }
    }
    else
        RemoveObjectFromCurrentLocation(&pObj);

    pObj.MoveObject(pObj.GetRect().TopLeft() + sizeDelta);

    if (ePos == placeTop || (!bOnBoard && ePos == placeDefault))
        pDwg->AddToFront(std::move(opObj));
    else if (ePos == placeBack)
        pDwg->AddToBack(std::move(opObj));
    else
    {
        // object must already be in this list
        ASSERT(pDwg->Find(*opObj) != pDwg->end());
        // don't let the object get deleted
        OwnerOrNullPtr<CDrawObj> temp = CB::get_underlying(std::move(opObj));
        CB::get_underlying(temp).release();
    }

    if (pPBrd->IsOwned() && pObj.GetType() == CDrawObj::drawPieceObj)
    {
        CPieceObj& pPObj = static_cast<CPieceObj&>(pObj);
        GetPieceTable()->SetOwnerMask(pPObj.m_pid, pPBrd->GetOwnerMask());
    }

    if (!IsQuietPlayback())
    {
        // Cause object to be drawn
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = &pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////
// (RECORDS) { EVENTUALLY USE ABOVE CODE!!!??? }
void CGamDoc::PlaceObjectListOnBoard(CPtrList *pLst, CPoint pntUpLeft,
    CPlayBoard *pPBrd, PlacePos ePos /* = placeDefault */)
{
    CDrawList* pDwg = pPBrd->GetPieceList();
    ASSERT(pDwg);
    CRect rct;
    FindObjectListUnionRect(pLst, rct);
    CSize size(pntUpLeft.x - rct.left, pntUpLeft.y - rct.top);

    if (pPBrd->GetPlotMoveMode())
        RecordPlotList(pPBrd);

    POSITION pos;
    pos = ePos != placeBack ? pLst->GetHeadPosition() : pLst->GetTailPosition();
    while (pos != NULL)
    {
        CDrawObj* pObj;
        pObj = (CDrawObj*)(ePos != placeBack ? pLst->GetNext(pos) :
            pLst->GetPrev(pos));
        ASSERT(pObj != NULL);

        // Record processing
        if (pObj->GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj* pPObj = (CPieceObj*)pObj;
            CRect rctPce = pPObj->GetRect();
            RecordPieceMoveToBoard(pPBrd, pPObj->m_pid, GetMidRect(rctPce) +
                size, ePos);
        }
        else if (pObj->GetType() == CDrawObj::drawMarkObj)
        {
            CMarkObj* pMObj = (CMarkObj*)pObj;
            CRect rctMrk = pMObj->GetRect();
            RecordMarkMoveToBoard(pPBrd, pMObj->GetObjectID(), pMObj->m_mid,
                GetMidRect(rctMrk) + size, ePos);
        }

        BOOL bOnBoard = pDwg->HasObject(pObj);
        if (ePos == placeDefault && bOnBoard)
        {
            if (!IsQuietPlayback())
            {
                // Cause it's former location to be invalidated...
                CGamDocHint hint;
                hint.m_pPBoard = pPBrd;
                hint.m_pDrawObj = pObj;
                UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
            }
        }
        else
            RemoveObjectFromCurrentLocation(pObj);

        pObj->MoveObject(pObj->GetRect().TopLeft() + size);

        if (ePos == placeTop || (!bOnBoard && ePos == placeDefault))
            pDwg->AddToFront(pObj);
        else if (ePos == placeBack)
            pDwg->AddToBack(pObj);

        if (pPBrd->IsOwned() && pObj->GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj* pPObj = (CPieceObj*)pObj;
            GetPieceTable()->SetOwnerMask(pPObj->m_pid, pPBrd->GetOwnerMask());
        }

        if (!IsQuietPlayback())
        {
            // Cause object to be drawn
            CGamDocHint hint;
            hint.m_pPBoard = pPBrd;
            hint.m_pDrawObj = pObj;
            UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
        }
        SetModifiedFlag();
    }
}

//////////////////////////////////////////////////////////////////////

void CGamDoc::PlaceObjectTableOnBoard(CPoint pnt, const std::vector<CDrawObj*>& pTbl,
    int xStagger, int yStagger, CPlayBoard *pPBrd)
{
    // Since we want the first entry of the list to be on top
    // in the view, we'll walk the array from bottom to top.
    if (pTbl.size() & 1)
    {
        // Odd number of pieces
        pnt += CSize(xStagger * (value_preserving_cast<int>(pTbl.size()) / 2),
            yStagger * (value_preserving_cast<int>(pTbl.size()) / 2));
    }
    else
    {
        // Even number of pieces
        pnt += CSize(xStagger * (value_preserving_cast<int>(pTbl.size()) / 2) - xStagger / 2,
            yStagger * (value_preserving_cast<int>(pTbl.size()) / 2) - yStagger / 2);
        pnt.x -= xStagger & 1 ? 1 : 0;      // Fudge for even/odd round off
        pnt.y -= yStagger & 1 ? 1 : 0;      // Fudge for even/odd round off
    }
    for (size_t i = pTbl.size(); i > 0; i--)
    {
        CDrawObj* pDObj = pTbl.at(i - 1);
        CSize sizeDelta = pnt - pDObj->GetRect().CenterPoint();
        PlaceObjectOnBoard(pPBrd, pDObj, sizeDelta, placeTop);
        pnt -= CSize(xStagger, yStagger);
    }
}

//////////////////////////////////////////////////////////////////////
// Places that objects in there current coordinates but changes
// their Z-order.

void CGamDoc::PlaceObjectTableOnBoard(const std::vector<CDrawObj*>& pTbl, CPlayBoard *pPBrd)
{
    // Since we want the first entry of the list to be on top
    // in the view, we'll walk the array from bottom to top.
    for (size_t i = pTbl.size(); i > 0; i--)
    {
        CDrawObj* pDObj = pTbl.at(i - 1);
        PlaceObjectOnBoard(pPBrd, pDObj, CSize(0,0), placeTop);
    }
}

//////////////////////////////////////////////////////////////////////

void CGamDoc::PlacePieceListOnBoard(CPoint pnt, const std::vector<PieceID>& pTbl,
    int xStagger, int yStagger, CPlayBoard *pPBrd)
{
    // Since we want the first entry of the list to be on top
    // in the view, we'll walk the array from bottom to top.
    if (pTbl.size() & 1)
    {
        // Odd number of pieces
        pnt += CSize(xStagger * (value_preserving_cast<int>(pTbl.size()) / 2),
            yStagger * (value_preserving_cast<int>(pTbl.size()) / 2));
    }
    else
    {
        // Even number of pieces
        pnt += CSize(xStagger * (value_preserving_cast<int>(pTbl.size()) / 2) - xStagger / 2,
            yStagger * (value_preserving_cast<int>(pTbl.size()) / 2) - yStagger / 2);
        pnt.x -= xStagger & 1 ? 1 : 0;      // Fudge for even/odd round off
        pnt.y -= yStagger & 1 ? 1 : 0;      // Fudge for even/odd round off
    }
    for (size_t i = pTbl.size(); i > 0; i--)
    {
        PieceID pid = pTbl.at(i - 1);
        PlacePieceOnBoard(pnt, pid, pPBrd);
        pnt -= CSize(xStagger, yStagger);
    }
}

size_t CGamDoc::PlacePieceListInTray(const std::vector<PieceID>& pTbl, CTraySet& pYGrp, size_t nPos)
{
    //TODO: This code will have to get smarter when dropping pieces
    //TODO that originate from the same tray.
    for (size_t i = 0; i < pTbl.size(); i++)
    {
        PieceID pid = pTbl.at(i);
        // We need to find out if the source of the list is in the
        // same as the target list. If it is, we shouldn't increment
        // the position.
        CTraySet *pCurYGrp = FindPieceInTray(pid);

        PlacePieceInTray(pid, pYGrp, nPos);
        if (nPos != Invalid_v<size_t>)
            nPos++;
    }
    return nPos == Invalid_v<size_t> ? nPos : nPos - 1;
}

//////////////////////////////////////////////////////////////////////
// Returns index of last piece inserted.
size_t CGamDoc::PlaceObjectListInTray(const CPtrList& pLst, CTraySet& pYGrp, size_t nPos)
{
    // Scan this list in reverse order so they show up in the
    // same visual order.
    POSITION pos;
    for (pos = pLst.GetTailPosition(); pos != NULL; )
    {
        CPieceObj* pObj = (CPieceObj*)pLst.GetPrev(pos);
        // Only pieces are placed. Other objects are left as they were.
        if (pObj->GetType() == CDrawObj::drawPieceObj)
        {
            PlacePieceInTray(pObj->m_pid, pYGrp, nPos);
            if (nPos != Invalid_v<size_t>)
                nPos++;
        }
    }
    return nPos == Invalid_v<size_t> ? nPos : nPos - 1;
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)
void CGamDoc::InvertPlayingPieceOnBoard(CPieceObj *pObj, CPlayBoard* pPBrd)
{
    if (!m_pPTbl->Is2Sided(pObj->m_pid))
        return;
    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }

    m_pPTbl->FlipPieceOver(pObj->m_pid);
    pObj->ResyncExtentRect();

    // Record processing
    RecordPieceSetSide(pObj->m_pid, GetPieceTable()->IsFrontUp(pObj->m_pid));

    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    SetModifiedFlag();
}

void CGamDoc::InvertPlayingPieceListOnBoard(CPtrList *pLst, CPlayBoard* pPBrd)
{
    POSITION pos;
    for (pos = pLst->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pObj = (CDrawObj*)pLst->GetNext(pos);
        // Only pieces are flipped. Others are left as they are.
        if (pObj->GetType() == CDrawObj::drawPieceObj)
            InvertPlayingPieceOnBoard((CPieceObj*)pObj, pPBrd);
    }
}

// RECORDS
void CGamDoc::InvertPlayingPieceInTray(PieceID pid, BOOL bOkToNotifyTray /* = TRUE */)
{
    if (!m_pPTbl->Is2Sided(pid))
        return;

    m_pPTbl->FlipPieceOver(pid);
    CTraySet* pYGrp = FindPieceInTray(pid);

    // Record processing
    RecordPieceSetSide(pid, GetPieceTable()->IsFrontUp(pid));

    if (!IsQuietPlayback() && bOkToNotifyTray)
    {
        CGamDocHint hint;
        hint.m_pTray = pYGrp;
        UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
    }
    SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)
void CGamDoc::ChangePlayingPieceFacingOnBoard(CPieceObj *pObj, CPlayBoard* pPBrd,
    int nFacingDegCW)
{
    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    m_pPTbl->SetPieceFacing(pObj->m_pid, nFacingDegCW);
    pObj->ResyncExtentRect();

    // Record processing
    RecordPieceSetFacing(pObj->m_pid, nFacingDegCW);

    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    SetModifiedFlag();
}

void CGamDoc::ChangePlayingPieceFacingListOnBoard(CPtrList *pLst,
    CPlayBoard* pPBrd, int nFacingDegCW)
{
    POSITION pos;
    for (pos = pLst->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pObj = (CDrawObj*)pLst->GetNext(pos);
        // Only pieces and markers are rotated. Others are left as they are.
        if (pObj->GetType() == CDrawObj::drawPieceObj)
            ChangePlayingPieceFacingOnBoard((CPieceObj*)pObj, pPBrd, nFacingDegCW);
        else if (pObj->GetType() == CDrawObj::drawMarkObj)
            ChangeMarkerFacingOnBoard((CMarkObj*)pObj, pPBrd, nFacingDegCW);
    }
}

// Special code for degenerate case of rotating a playing
// piece that resided in a tray. You normally can't do this. However,
// if playback is performed on a game that isn't synchronized with
// the move file's game state it can occur. Therefore, I don't do
// any record processing here.

void CGamDoc::ChangePlayingPieceFacingInTray(PieceID pid, int nFacingDegCW)
{
    m_pPTbl->SetPieceFacing(pid, nFacingDegCW);
    CTraySet* pYGrp = FindPieceInTray(pid);

    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pTray = pYGrp;
        UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
    }
    SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)

void CGamDoc::ChangeMarkerFacingOnBoard(CMarkObj *pObj, CPlayBoard* pPBrd,
    int nFacingDegCW)
{
    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    pObj->SetFacing(nFacingDegCW);
    pObj->ResyncExtentRect();

    // Record processing
    RecordMarkerSetFacing(pObj->GetObjectID(), pObj->m_mid, nFacingDegCW);

    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)

void CGamDoc::DeleteObjectsInList(CPtrList *pLst)
{
    POSITION pos;
    for (pos = pLst->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pObj = (CDrawObj*)pLst->GetNext(pos);
        // Only nonpieces are deleted. Pieces are left as they were.
        if (pObj->GetType() != CDrawObj::drawPieceObj)
        {
            if (pObj->GetType() == CDrawObj::drawMarkObj)
                RecordObjectDelete(pObj->GetObjectID());

            CPlayBoard* pPBrd = FindObjectOnBoard(pObj);
            ASSERT(pPBrd != NULL);
            pPBrd->RemoveObject(pObj);

            // Erase any associated text
            SetGameElementString(MakeObjectIDElement(pObj->GetObjectID()), NULL);

            if (!IsQuietPlayback())
            {
                CGamDocHint hint;
                hint.m_pPBoard = pPBrd;
                hint.m_pDrawObj = pObj;
                UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
            }
            SetModifiedFlag();

            delete pObj;
        }
    }
    UpdateAllViews(NULL, HINT_UPDATESELECTLIST);
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)

void CGamDoc::SetObjectText(GameElement elem, LPCTSTR pszObjText)
{
    SetGameElementString(elem, pszObjText);

    // Record processing
    RecordObjectSetText(elem, pszObjText);

    SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)

void CGamDoc::SetObjectLockdownList(CPtrList *pLst, BOOL bLockState)
{
    POSITION pos;
    for (pos = pLst->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pObj = (CDrawObj*)pLst->GetNext(pos);
        SetObjectLockdown(pObj, bLockState);
    }
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)

void CGamDoc::SetObjectLockdown(CDrawObj* pDObj, BOOL bLockState)
{
    pDObj->ModifyDObjFlags(dobjFlgLockDown, bLockState);

    // Record processing
    RecordObjectLockdown(GetGameElementCodeForObject(pDObj), bLockState);

    SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////
// (RECORDS)

void CGamDoc::SetPieceOwnership(PieceID pid, DWORD dwOwnerMask)
{
    GetPieceTable()->SetOwnerMask(pid, dwOwnerMask);

    // Record processing
    RecordPieceSetOwnership(pid, dwOwnerMask);

    SetModifiedFlag();
}

void CGamDoc::SetPieceOwnershipTable(const std::vector<PieceID>& pTblPieces, DWORD dwOwnerMask)
{
    for (size_t i = 0; i < pTblPieces.size(); i++)
        SetPieceOwnership(pTblPieces.at(i), dwOwnerMask);
}

//////////////////////////////////////////////////////////////////////
// pnt is center of mark image

CDrawObj& CGamDoc::CreateMarkerObject(CPlayBoard* pPBrd, MarkID mid, CPoint pnt,
    ObjectID dwObjID)
{
    CDrawList* pDwg = pPBrd->GetPieceList();
    ASSERT(pDwg);
    if (dwObjID == ObjectID())
        dwObjID = CreateObjectID(CDrawObj::drawMarkObj);

    CMarkManager* pMMgr = GetMarkManager();
    ASSERT(pMMgr != NULL);

    MarkDef& pMark = pMMgr->GetMark(mid);
    ASSERT(pMark.m_tid != nullTid);

    // Marker is centered on point.
    CTile tile;
    GetTileManager()->GetTile(pMark.m_tid,  &tile, fullScale);
    CRect rct(pnt, tile.GetSize());
    rct -= CPoint(tile.GetWidth() / 2, tile.GetHeight() / 2);
    pPBrd->LimitRectToBoard(rct);

    // Create the marker object
    {
        OwnerPtr<CMarkObj> pObj(MakeOwner<CMarkObj>(this));
        pObj->SetObjectID(dwObjID);
        pObj->SetMark(rct, mid);

        RecordMarkMoveToBoard(pPBrd, dwObjID, mid, GetMidRect(rct), placeTop);

        // Finally add it to the board's object list.
        pDwg->AddToFront(std::move(pObj));
    }
    CDrawObj& pObj = pDwg->Front();
    SetModifiedFlag();

    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = &pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    SetModifiedFlag();

    return pObj;
}

//////////////////////////////////////////////////////////////////////

CDrawObj& CGamDoc::CreateLineObject(CPlayBoard* pPBrd, CPoint ptBeg,
    CPoint ptEnd, UINT nLineWd, COLORREF crLine, ObjectID dwObjID)
{
    CDrawList* pDwg = pPBrd->GetPieceList();
    ASSERT(pDwg);
    if (dwObjID == ObjectID())
        dwObjID = CreateObjectID(CDrawObj::drawLineObj);

    {
        OwnerPtr<CLineObj> pObj = MakeOwner<CLineObj>();
        pObj->SetObjectID(dwObjID);
        pObj->SetLine(ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y);
        pObj->SetForeColor(pPBrd->GetLineColor());
        pObj->SetLineWidth(pPBrd->GetLineWidth());

        pDwg->AddToFront(std::move(pObj));
    }
    CDrawObj& pObj = pDwg->Front();
    SetModifiedFlag();

    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = &pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    SetModifiedFlag();

    return pObj;
}

void CGamDoc::ModifyLineObject(CPlayBoard* pPBrd, CPoint ptBeg,
    CPoint ptEnd, UINT nLineWd, COLORREF crLine, CLine* pObj)
{
    ASSERT(pObj->GetObjectID() != ObjectID());
    CDrawList* pDwg = pPBrd->GetPieceList();
    ASSERT(pDwg);

    if (!IsQuietPlayback())
    {
        // Cause invalidation of current positions....
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }

    // Set new values
    pObj->SetLine(ptBeg.x, ptBeg.y, ptEnd.x, ptEnd.y);
    pObj->SetForeColor(pPBrd->GetLineColor());
    pObj->SetLineWidth(pPBrd->GetLineWidth());

    if (!IsQuietPlayback())
    {
        // Refresh drawing of object.
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pDrawObj = pObj;
        UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
    }
    SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////

void CGamDoc::ReorgObjsInDrawList(CPlayBoard *pPBrd, CPtrList* pList,
    BOOL bToFront)
{
    CDrawList* pDwg = pPBrd->GetPieceList();
    ASSERT(pDwg);

    pDwg->ArrangeObjectListInDrawOrder(pList);
    pDwg->RemoveObjectsInList(pList);
    if (bToFront)
    {
        POSITION pos = pList->GetHeadPosition();
        while (pos != NULL)
            pDwg->AddToFront((CDrawObj*)pList->GetNext(pos));
    }
    else
    {
        POSITION pos = pList->GetTailPosition();
        while (pos != NULL)
            pDwg->AddToBack((CDrawObj*)pList->GetPrev(pos));
    }
    SetModifiedFlag();

    if (!IsQuietPlayback())
    {
        CGamDocHint hint;
        hint.m_pPBoard = pPBrd;
        hint.m_pPtrList = pList;
        UpdateAllViews(NULL, HINT_UPDATEOBJLIST, &hint);
    }
    SetModifiedFlag();
}

////////////////////////////////////////////////////////////////////

CPlayBoard* CGamDoc::FindObjectOnBoard(ObjectID dwObjID, CDrawObj** ppObj)
{
    ASSERT(m_pPBMgr != NULL);
    return m_pPBMgr->FindObjectOnBoard(dwObjID, ppObj);
}

CPlayBoard* CGamDoc::FindObjectOnBoard(CDrawObj* pObj)
{
    ASSERT(m_pPBMgr != NULL);
    return m_pPBMgr->FindObjectOnBoard(pObj);
}

CPlayBoard* CGamDoc::FindPieceOnBoard(PieceID pid, CPieceObj** ppObj)
{
    ASSERT(m_pPBMgr != NULL);
    return m_pPBMgr->FindPieceOnBoard(pid, ppObj);
}

CTraySet* CGamDoc::FindPieceInTray(PieceID pid)
{
    ASSERT(m_pYMgr != NULL);
    return m_pYMgr->FindPieceIDInTraySet(pid);
}

////////////////////////////////////////////////////////////////////
// Returns true if blanket update for tray views are required.
// Can only return TRUE if parameter bTrayHintAllowed == FALSE;

BOOL CGamDoc::RemovePieceFromCurrentLocation(PieceID pid, BOOL bDeleteIfBoard,
    BOOL bTrayHintAllowed)
{
    CPieceObj* pObj;
    CPlayBoard* pPBoard = FindPieceOnBoard(pid, &pObj);
    if (pPBoard != NULL)
    {
        pPBoard->RemoveObject(pObj);
        if (!IsQuietPlayback())
        {
            // Cause it's former location to be invalidated...
            CGamDocHint hint;
            hint.m_pPBoard = pPBoard;
            hint.m_pDrawObj = pObj;
            UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
        }
        if (bDeleteIfBoard)
            delete pObj;
        return FALSE;
    }
    CTraySet *pYGrp = FindPieceInTray(pid);
    ASSERT(pYGrp != NULL);      // It has to be somewhere!
    if (pYGrp != NULL)
    {
        pYGrp->RemovePieceID(pid);
        if (bTrayHintAllowed && !IsQuietPlayback())
        {
            CGamDocHint hint;
            hint.m_pTray = pYGrp;
            UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
            return FALSE;
        }
        return TRUE;            // Tell caller they need to do tray updates
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////
// Locates the current location for a playing piece. returns
// TRUE if found on a board. FALSE if in tray.

BOOL CGamDoc::FindPieceCurrentLocation(PieceID pid, CTraySet*& pTraySet,
    CPlayBoard*& pPBoard, CPieceObj** ppObj /* = NULL */)
{
    pTraySet = NULL;
    pPBoard = FindPieceOnBoard(pid, ppObj);
    if (pPBoard != NULL)
        return TRUE;

    pTraySet = FindPieceInTray(pid);
    ASSERT(pTraySet != NULL);   // It HAS to be somewhere!
    return FALSE;
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RemoveObjectFromCurrentLocation(CDrawObj* pObj)
{
    CPlayBoard* pPBoard = FindObjectOnBoard(pObj);
    if (pPBoard != NULL)
    {
        pPBoard->RemoveObject(pObj);
        if (!IsQuietPlayback())
        {
            // Cause it's former location to be invalidated...
            CGamDocHint hint;
            hint.m_pPBoard = pPBoard;
            hint.m_pDrawObj = pObj;
            UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
        }
    }
}

////////////////////////////////////////////////////////////////////

void CGamDoc::FindObjectListUnionRect(CPtrList* pLst, CRect& rct)
{
    rct.SetRectEmpty();

    POSITION pos;
    for (pos = pLst->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pObj = (CDrawObj*)pLst->GetNext(pos);
        if (rct.IsRectEmpty())
            rct = pObj->GetRect();
        else
            rct |= pObj->GetRect();
    }
}

////////////////////////////////////////////////////////////////////

void CGamDoc::ExpungeUnusedPiecesFromBoards()
{
    for (size_t i = 0; i < m_pPBMgr->GetNumPBoards(); i++)
    {
        CPlayBoard& pPBrd = m_pPBMgr->GetPBoard(i);
        CDrawList* pDwg = pPBrd.GetPieceList();

        CPtrList listPtr;
        pDwg->GetPieceObjectPtrList(&listPtr);

        POSITION pos;
        for (pos = listPtr.GetHeadPosition(); pos != NULL; )
        {
            CPieceObj* pObj = (CPieceObj*)listPtr.GetNext(pos);
            ASSERT(pObj != NULL);
            if (!m_pPTbl->IsPieceUsed(pObj->m_pid))
            {
                pPBrd.RemoveObject(pObj);

                if (!IsQuietPlayback())
                {
                    // Cause object display area to be invalidated
                    CGamDocHint hint;
                    hint.m_pPBoard = &pPBrd;
                    hint.m_pDrawObj = pObj;
                    UpdateAllViews(NULL, HINT_UPDATEOBJECT, &hint);
                }

                // Destroy to object
                delete pObj;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////

ObjectID CGamDoc::CreateObjectID(CDrawObj::CDrawObjType nObjType)
{
    // Make sure we don't get redundant object ID's if we are getting them
    // all at the same moment in time.
    return ObjectID(GetTimeBasedRandomNumber(FALSE), m_wDocRand++, nObjType);
}

