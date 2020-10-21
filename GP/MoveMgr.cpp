// MoveMgr.cpp
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
#include    "GMisc.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "Trays.h"
#include    "DrawObj.h"
#include    "GamState.h"

#include    "MoveMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////
// Delay between submoves

const int stepDelay = 800;          // 800 milliseconds

/////////////////////////////////////////////////////////////////////
// CMoveRecord methods....

void CMoveRecord::Serialize(CArchive& ar)
{
    // The type code is stored seperately and are reconstituted
    // by the object's constructor.
    if (ar.IsStoring())
        ar << (short)m_nSeqNum;
    else
    {
        short sTmp;
        ar >> sTmp; m_nSeqNum = (int)sTmp;
    }
}

/////////////////////////////////////////////////////////////////////
// CBoardPieceMove methods....

CBoardPieceMove::CBoardPieceMove(int nBrdSerNum, PieceID pid, CPoint pnt,
    PlacePos ePos)
{
    m_eType = mrecPMove;
    // ------- //
    m_nBrdNum = nBrdSerNum;
    m_pid = pid;
    m_ptCtr = pnt;
    m_ePos = ePos;
}

BOOL CBoardPieceMove::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CBoardPieceMove::ValidatePieces - Piece %u not in piece table.\n", m_pid);
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CBoardPieceMove::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        CPlayBoard* pPBrdDest = pDoc->GetPBoardManager()->
            GetPBoardBySerial(m_nBrdNum);
        ASSERT(pPBrdDest);
        pDoc->IndicateBoardToBoardPieceMove(pPBoard, pPBrdDest, ptCtr, m_ptCtr,
            pObj->GetRect().Size());
    }
    else
        pDoc->SelectTrayItem(pTray, m_pid);
}

void CBoardPieceMove::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBrdFrom;
    CTraySet* pTrayFrom;
    CPieceObj* pObj;

    CPlayBoard* pPBrdDest = pDoc->GetPBoardManager()->
        GetPBoardBySerial(m_nBrdNum);
    ASSERT(pPBrdDest);

    pDoc->EnsureBoardLocationVisible(pPBrdDest, m_ptCtr);

    if (pDoc->FindPieceCurrentLocation(m_pid, pTrayFrom, pPBrdFrom, &pObj))
    {
        CRect rct = pObj->GetRect();
        CSize size = m_ptCtr - GetMidRect(rct);
        pDoc->PlaceObjectOnBoard(pPBrdDest, pObj, size, m_ePos);
    }
    else
    {
        pDoc->PlacePieceOnBoard(m_ptCtr, m_pid, pPBrdDest);
        VERIFY(pDoc->FindPieceOnBoard(m_pid, &pObj) != NULL);

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->IndicateBoardPiece(pPBrdDest, ptCtr, rct.Size());
    }
    pDoc->SelectObjectOnBoard(pPBrdDest, pObj);
}

void CBoardPieceMove::DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup)
{
}

void CBoardPieceMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_pid;
        ar << (short)m_nBrdNum;
        ar << (short)m_ePos;
        ar << (short)m_ptCtr.x;
        ar << (short)m_ptCtr.y;
    }
    else
    {
        WORD wTmp;
        short sTmp;
        ar >> wTmp; m_pid = (int)wTmp;
        ar >> sTmp; m_nBrdNum = (int)sTmp;
        ar >> sTmp; m_ePos = (PlacePos)sTmp;
        ar >> sTmp; m_ptCtr.x = sTmp;
        ar >> sTmp; m_ptCtr.y = sTmp;
    }
}

#ifdef _DEBUG
void CBoardPieceMove::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    board = %d, pos = %d, pid = %d, @(%d, %d)\r\n",
        m_nBrdNum, m_ePos, m_pid, m_ptCtr.x, m_ptCtr.y);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CTrayPieceMove methods....

CTrayPieceMove::CTrayPieceMove(int nTrayNum, PieceID pid, int nPos)
{
    m_eType = mrecTMove;
    // ------- //
    m_nTrayNum = nTrayNum;
    m_pid = pid;
    m_nPos = nPos;
}

BOOL CTrayPieceMove::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CTrayPieceMove::ValidatePieces - Piece %u not in piece table.\n", m_pid);
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CTrayPieceMove::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
    }
    else
        pDoc->SelectTrayItem(pTray, m_pid);
}

void CTrayPieceMove::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CTraySet* pYGrp = pDoc->GetTrayManager()->GetTraySet(m_nTrayNum);
    pDoc->PlacePieceInTray(m_pid, pYGrp, m_nPos);
}

void CTrayPieceMove::DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CTraySet* pYGrp = pDoc->GetTrayManager()->GetTraySet(m_nTrayNum);
    pDoc->SelectTrayItem(pYGrp, m_pid);
}

void CTrayPieceMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_pid;
        ar << (short)m_nTrayNum;
        ar << (short)m_nPos;
    }
    else
    {
        WORD wTmp;
        short sTmp;
        ar >> wTmp; m_pid = (int)wTmp;
        ar >> sTmp; m_nTrayNum = (int)sTmp;
        ar >> sTmp; m_nPos = (int)sTmp;
    }
}

#ifdef _DEBUG
void CTrayPieceMove::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    tray = %d, nPos = %d, pid = %d\r\n",
        m_nTrayNum, m_nPos, m_pid);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CPieceSetSide methods....

BOOL CPieceSetSide::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CPieceSetSide::ValidatePieces - Piece %u not in piece table.\n", m_pid);
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

BOOL CPieceSetSide::IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->GetPieceTable()->IsOwnedButNotByCurrentPlayer(m_pid, pDoc))
        return TRUE;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        if (pPBoard->IsOwnedButNotByCurrentPlayer(pDoc))
            return TRUE;
    }
    else
    {
        if (pTray->IsOwnedButNotByCurrentPlayer(pDoc))
            return TRUE;
    }
    return FALSE;
}

void CPieceSetSide::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
        pDoc->SelectObjectOnBoard(pPBoard, pObj);
    }
    else
        pDoc->SelectTrayItem(pTray, m_pid);
}

void CPieceSetSide::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
        pDoc->InvertPlayingPieceOnBoard(pObj, pPBoard);
    else
        pDoc->InvertPlayingPieceInTray(m_pid);
}

void CPieceSetSide::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_pid;
        ar << (BYTE)m_bTopUp;
    }
    else
    {
        WORD wTmp;
        ar >> wTmp; m_pid = (PieceID)wTmp;
        BYTE cTmp;
        ar >> cTmp; m_bTopUp = (BOOL)cTmp;
    }
}

#ifdef _DEBUG
void CPieceSetSide::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    Piece %d is set to %s visible.\r\n",
        m_pid, m_bTopUp ? "top" : "bottom");
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CPieceSetFacing methods....

BOOL CPieceSetFacing::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CPieceSetFacing::ValidatePieces - Piece %u not in piece table.\n", m_pid);
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CPieceSetFacing::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
        pDoc->SelectObjectOnBoard(pPBoard, pObj);
    }
    else
        pDoc->SelectTrayItem(pTray, m_pid);
}

void CPieceSetFacing::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
        pDoc->ChangePlayingPieceFacingOnBoard(pObj, pPBoard, m_nFacingDegCW);
    else
        pDoc->ChangePlayingPieceFacingInTray(m_pid, m_nFacingDegCW);
}

void CPieceSetFacing::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_pid;
        ar << (WORD)m_nFacingDegCW;
    }
    else
    {
        WORD wTmp;
        ar >> wTmp; m_pid = (PieceID)wTmp;
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   //Ver2.90
        {
            BYTE cTmp;
            ar >> cTmp;
            m_nFacingDegCW = 5 * (int)cTmp;
        }
        else
        {
            ar >> wTmp;
            m_nFacingDegCW = (int)wTmp;
        }
    }
}

#ifdef _DEBUG
void CPieceSetFacing::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    Piece %d is rotated %d degrees.\r\n", m_pid, m_nFacingDegCW);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CPieceSetFacing methods....

BOOL CPieceSetOwnership::ValidatePieces(CGamDoc* pDoc)
{
#ifdef _DEBUG
    BOOL bUsed = pDoc->GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CPieceSetOwnership::ValidatePieces - Piece %u not in piece table.\n", m_pid);
    return bUsed;
#else
    return pDoc->GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CPieceSetOwnership::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc->FindPieceCurrentLocation(m_pid, pTray, pPBoard, &pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
        pDoc->SelectObjectOnBoard(pPBoard, pObj);
    }
    else
        pDoc->SelectTrayItem(pTray, m_pid);
    pDoc->SetPieceOwnership(m_pid, m_dwOwnerMask);
}

void CPieceSetOwnership::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_pid;
        ar << m_dwOwnerMask;
    }
    else
    {
        WORD wTmp;
        ar >> wTmp; m_pid = (PieceID)wTmp;
        if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10))
        {
            ar >> wTmp;
            m_dwOwnerMask = UPGRADE_OWNER_MASK(wTmp);
        }
        else
            ar >> m_dwOwnerMask;
    }
}

#ifdef _DEBUG
void CPieceSetOwnership::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    Piece %d has ownership changed to 0x%X.\r\n",
        m_pid, m_dwOwnerMask);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CMarkerSetFacing methods....

CMarkerSetFacing::CMarkerSetFacing(ObjectID dwObjID, MarkID mid, int nFacingDegCW)
{
    m_eType = mrecMFacing;
    m_dwObjID = dwObjID;
    m_mid = mid;
    m_nFacingDegCW = nFacingDegCW;
}

void CMarkerSetFacing::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, rct.Size());
        pDoc->SelectObjectOnBoard(pPBoard, pObj);
    }
    else
        ASSERT(FALSE);          // SHOULDN'T HAPPEN
}

void CMarkerSetFacing::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
        pDoc->ChangeMarkerFacingOnBoard((CMarkObj*)pObj, pPBoard, m_nFacingDegCW);
    else
        ASSERT(FALSE);          // SHOULDN'T HAPPEN
}

void CMarkerSetFacing::Serialize(CArchive& ar)              // VER2.0 is first time used
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_dwObjID;
        ar << (WORD)m_mid;
        ar << (WORD)m_nFacingDegCW;
    }
    else
    {
        ar >> m_dwObjID;
        WORD wTmp;
        ar >> wTmp; m_mid = (PieceID)wTmp;
        ar >> wTmp; m_nFacingDegCW = (int)wTmp;
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   //Ver2.90
            m_nFacingDegCW *= 5;                                // Convert old values to degrees
    }
}

#ifdef _DEBUG
void CMarkerSetFacing::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    Marker dwObjID = %lX, mid = %d is rotated %d degrees.\r\n",
        m_dwObjID, m_mid, m_nFacingDegCW);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CBoardMarkerMove methods....

CBoardMarkerMove::CBoardMarkerMove(int nBrdSerNum, ObjectID dwObjID, MarkID mid,
    CPoint pnt, PlacePos ePos)
{
    m_eType = mrecMMove;
    // ------- //
    m_nBrdNum = nBrdSerNum;
    m_dwObjID = dwObjID;
    m_mid = mid;
    m_ptCtr = pnt;
    m_ePos = ePos;
}

void CBoardMarkerMove::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        CPlayBoard* pPBrdDest = pDoc->GetPBoardManager()->
            GetPBoardBySerial(m_nBrdNum);
        ASSERT(pPBrdDest != NULL);
        pDoc->IndicateBoardToBoardPieceMove(pPBoard, pPBrdDest, ptCtr, m_ptCtr,
            pObj->GetRect().Size());
    }
    else
        pDoc->SelectMarkerPaletteItem(m_mid);
}

void CBoardMarkerMove::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBrdDest = pDoc->GetPBoardManager()->
        GetPBoardBySerial(m_nBrdNum);
    ASSERT(pPBrdDest != NULL);

    pDoc->EnsureBoardLocationVisible(pPBrdDest, m_ptCtr);

    CDrawObj* pObj;
    CPlayBoard* pPBrdFrom = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBrdFrom != NULL)
    {
        CRect rct = pObj->GetRect();
        CSize size = m_ptCtr - GetMidRect(rct);
        pDoc->PlaceObjectOnBoard(pPBrdDest, pObj, size, m_ePos);
    }
    else
    {
        // Need to create since doesn't currently exist
        pObj = pDoc->CreateMarkerObject(pPBrdDest, m_mid, m_ptCtr, m_dwObjID);

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->IndicateBoardPiece(pPBrdDest, ptCtr, rct.Size());
    }
}

void CBoardMarkerMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_mid;
        ar << m_dwObjID;
        ar << (short)m_nBrdNum;
        ar << (short)m_ePos;
        ar << (short)m_ptCtr.x;
        ar << (short)m_ptCtr.y;
    }
    else
    {
        WORD wTmp;
        short sTmp;
        ar >> wTmp; m_mid = (int)wTmp;
        ar >> m_dwObjID;
        ar >> sTmp; m_nBrdNum = (int)sTmp;
        ar >> sTmp; m_ePos = (PlacePos)sTmp;
        ar >> sTmp; m_ptCtr.x = sTmp;
        ar >> sTmp; m_ptCtr.y = sTmp;
    }
}

#ifdef _DEBUG
void CBoardMarkerMove::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    board = %d, pos = %d, dwObjID = %lX, mid = %d, @(%d, %d)\r\n",
        m_nBrdNum, m_ePos, m_dwObjID, m_mid, m_ptCtr.x, m_ptCtr.y);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CObjectDelete methods....

CObjectDelete::CObjectDelete(ObjectID dwObjID)
{
    m_eType = mrecDelObj;
    m_dwObjID = dwObjID;
}

void CObjectDelete::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, pObj->GetRect().Size());
    }
}

void CObjectDelete::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc->FindObjectOnBoard(m_dwObjID, &pObj);

    if (pPBoard != NULL)
    {
        CPtrList list;
        list.AddHead(pObj);
        pDoc->DeleteObjectsInList(&list);
    }
}

void CObjectDelete::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
        ar << m_dwObjID;
    else
        ar >> m_dwObjID;
}

#ifdef _DEBUG
void CObjectDelete::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    dwObjID = %lX\r\n", m_dwObjID);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CObjectSetText methods....

CObjectSetText::CObjectSetText(GameElement elem, LPCTSTR pszText)
{
    m_eType = mrecSetObjText;
    m_elem = elem;
    if (pszText != NULL)
        m_strObjText = pszText;
}

BOOL CObjectSetText::IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard = NULL;
    CTraySet* pTray = NULL;
    CDrawObj* pObj = NULL;
    CPieceObj* pPObj = NULL;

    if (IsGameElementAPiece(m_elem))
    {
        PieceID pid = GetPieceIDFromElement(m_elem);
        if (pDoc->GetPieceTable()->IsOwnedButNotByCurrentPlayer(pid, pDoc))
            return TRUE;

        pDoc->FindPieceCurrentLocation(pid, pTray, pPBoard, &pPObj);
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);
    if (pPBoard != NULL)
    {
        if (pPBoard->IsOwnedButNotByCurrentPlayer(pDoc))
            return TRUE;
    }
    else if (pTray != NULL)
    {
        if (pTray->IsOwnedButNotByCurrentPlayer(pDoc))
            return TRUE;
    }
    return FALSE;
}

void CObjectSetText::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CDrawObj* pObj;
    CPieceObj* pPObj;

    pDoc->SetGameElementString(m_elem,
        m_strObjText.IsEmpty() ? NULL : (LPCTSTR)m_strObjText);

    if (IsGameElementAPiece(m_elem))
    {
        if (pDoc->FindPieceCurrentLocation(GetPieceIDFromElement(m_elem),
                pTray, pPBoard, &pPObj))
            pObj = pPObj;
        else
            pDoc->SelectTrayItem(pTray, GetPieceIDFromElement(m_elem), IDS_TIP_OBJTEXTCHG);
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, pObj->GetRect().Size());

        // Show a balloon tip so person knows what happened
        if (nMoveWithinGroup == 0)
            pDoc->IndicateTextTipOnBoard(pPBoard, ptCtr, IDS_TIP_OBJTEXTCHG);
    }
}

void CObjectSetText::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (DWORD)m_elem;
        ar << m_strObjText;
    }
    else
    {
        DWORD dwTmp;
        ar >> dwTmp; m_elem = (GameElement)dwTmp;
        ar >> m_strObjText;
    }
}

#ifdef _DEBUG
void CObjectSetText::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    elem = %lX, text = \"%s\"\r\n", m_elem, (LPCTSTR)m_strObjText);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CObjectLockdown methods....

CObjectLockdown::CObjectLockdown(GameElement elem, BOOL bLockState)
{
    m_eType = mrecLockObj;
    m_elem = elem;
    m_bLockState = bLockState;
}

BOOL CObjectLockdown::IsMoveHidden(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPieceObj* pPObj;
    CPlayBoard* pPBoard;

    if (IsGameElementAPiece(m_elem))
    {
        PieceID pid = GetPieceIDFromElement(m_elem);
        if (pDoc->GetPieceTable()->IsOwnedButNotByCurrentPlayer(pid, pDoc))
            return TRUE;
        pPBoard = pDoc->FindPieceOnBoard(GetPieceIDFromElement(m_elem), &pPObj);
        pObj = pPObj;
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);

    ASSERT(pObj != NULL);

    if (pPBoard != NULL && pPBoard->IsOwnedButNotByCurrentPlayer(pDoc))
        return TRUE;

    return FALSE;
}

void CObjectLockdown::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CDrawObj* pObj;
    CPieceObj* pPObj;
    CPlayBoard* pPBoard;

    if (IsGameElementAPiece(m_elem))
    {
        pPBoard = pDoc->FindPieceOnBoard(GetPieceIDFromElement(m_elem), &pPObj);
        pObj = pPObj;
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);


    ASSERT(pObj != NULL);

    if (pObj != NULL)
        pObj->ModifyDObjFlags(dobjFlgLockDown, m_bLockState);

    if (pPBoard != NULL)
    {

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);
        pDoc->IndicateBoardPiece(pPBoard, ptCtr, pObj->GetRect().Size());
    }
}

void CObjectLockdown::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    if (nMoveWithinGroup != 0)      // Only do this for the first record in group
        return;

    CDrawObj* pObj;
    CPieceObj* pPObj;
    CPlayBoard* pPBoard;

    if (IsGameElementAPiece(m_elem))
    {
        pPBoard = pDoc->FindPieceOnBoard(GetPieceIDFromElement(m_elem), &pPObj);
        pObj = pPObj;
    }
    else
        pPBoard = pDoc->FindObjectOnBoard(static_cast<ObjectID>(m_elem), &pObj);


    if (pPBoard != NULL)
    {

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc->EnsureBoardLocationVisible(pPBoard, ptCtr);

        // Show a balloon tip so person knows what happened
        pDoc->IndicateTextTipOnBoard(pPBoard, ptCtr,
            m_bLockState ? IDS_TIP_OBJLOCKED : IDS_TIP_OBJUNLOCKED);
    }
}

void CObjectLockdown::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (DWORD)m_elem;
        ar << (WORD)m_bLockState;
    }
    else
    {
        DWORD dwTmp;
        WORD wTmp;
        ar >> dwTmp; m_elem = (GameElement)dwTmp;
        ar >> wTmp; m_bLockState = (BOOL)wTmp;
    }
}

#ifdef _DEBUG
void CObjectLockdown::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    elem = %lX, state = %d\r\n", m_elem, m_bLockState);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CGameStateRcd methods....

CGameStateRcd::~CGameStateRcd()
{
    if (m_pState != NULL)
        delete m_pState;
}

void CGameStateRcd::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    if (!m_pState->RestoreState())
        AfxMessageBox(IDS_ERR_FAILEDSTATECHG, MB_OK | MB_ICONEXCLAMATION);
    if (!pDoc->IsQuietPlayback())
        pDoc->UpdateAllViews(NULL, HINT_GAMESTATEUSED);
}

void CGameStateRcd::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (!ar.IsStoring())
    {
        if (m_pState != NULL) delete m_pState;
        m_pState = new CGameState((CGamDoc*)ar.m_pDocument);
    }
    ASSERT(m_pState != NULL);
    m_pState->Serialize(ar);
}

#ifdef _DEBUG
void CGameStateRcd::DumpToTextFile(CFile& file)
{
    static char strMsg[] =
        "    To much to dump! (Trust me on this)\r\n";
    file.Write(strMsg, lstrlen(strMsg));
}
#endif

/////////////////////////////////////////////////////////////////////
// CMovePlotList

void CMovePlotList::SavePlotList(CDrawList* pDwg)
{
    m_tblPlot.RemoveAll();
    POSITION pos;
    for (pos = pDwg->GetHeadPosition(); pos != NULL; )
    {
        CLine* pObj = (CLine*)pDwg->GetNext(pos);
        ASSERT(pObj);
        if (pObj->GetType() == CDrawObj::drawLine)
        {
            m_tblPlot.Add((DWORD)MAKELONG(pObj->m_ptBeg.x, pObj->m_ptBeg.y));
            m_tblPlot.Add((DWORD)MAKELONG(pObj->m_ptEnd.x, pObj->m_ptEnd.y));
        }
    }
}

void CMovePlotList::DoMoveSetup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBrd = pDoc->GetPBoardManager()->GetPBoardBySerial(m_nBrdNum);
    ASSERT(pPBrd != NULL);
    pPBrd->SetPlotMoveMode(TRUE);
    for (int i = 0; i < m_tblPlot.GetSize(); i += 2)
    {
        CPoint ptA(m_tblPlot.GetAt(i));
        CPoint ptB(m_tblPlot.GetAt(i+1));
        pDoc->IndicateBoardPlotLine(pPBrd, ptA, ptB);
    }
}

void CMovePlotList::DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    CPlayBoard* pPBrd = pDoc->GetPBoardManager()->GetPBoardBySerial(m_nBrdNum);
    ASSERT(pPBrd != NULL);
    pPBrd->SetPlotMoveMode(FALSE);
}

void CMovePlotList::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
        ar << (short)m_nBrdNum;
    else
    {
        short sTmp;
        ar >> sTmp; m_nBrdNum = (int)sTmp;
    }
    m_tblPlot.Serialize(ar);
}

#ifdef _DEBUG
void CMovePlotList::DumpToTextFile(CFile& file)
{
    static char strMsg[] =
        "    To much to dump! (just plain lazy on this one)\r\n";
    file.Write(strMsg, lstrlen(strMsg));
}
#endif

/////////////////////////////////////////////////////////////////////

void CMessageRcd::DoMove(CGamDoc* pDoc, int nMoveWithinGroup)
{
    pDoc->MsgSetMessageText(m_strMsg);
}

void CMessageRcd::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
        ar << m_strMsg;
    else
        ar >> m_strMsg;
}

#ifdef _DEBUG
void CMessageRcd::DumpToTextFile(CFile& file)
{
    file.Write(m_strMsg, lstrlen(m_strMsg));
    file.Write("\r\n", 2);
}
#endif

/////////////////////////////////////////////////////////////////////

CEventMessageRcd::CEventMessageRcd(CString strMsg, BOOL bIsBoardEvent,
    int nID, int nVal1, int nVal2 /* = 0 */)
{
    m_eType = mrecEvtMsg;
    m_bIsBoardEvent = bIsBoardEvent;
    m_nID = nID;
    m_nVal1 = nVal1;
    m_nVal2 = nVal2;
    m_strMsg = strMsg;
}

void CEventMessageRcd::DoMoveCleanup(CGamDoc* pDoc, int nMoveWithinGroup)
{
    if (m_bIsBoardEvent)        // Board event message
        pDoc->EventShowBoardNotification(m_nID, CPoint(m_nVal1, m_nVal2), m_strMsg);
    else                        // Tray event message
        pDoc->EventShowTrayNotification(m_nID, (PieceID)m_nVal1, m_strMsg);
}

void CEventMessageRcd::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_bIsBoardEvent;
        ar << (WORD)m_nID;
        ar << (DWORD)m_nVal1;
        ar << (DWORD)m_nVal2;
        ar << m_strMsg;
    }
    else
    {
        WORD wTmp;
        DWORD dwTmp;
        ar >> wTmp; m_bIsBoardEvent = (int)wTmp;
        ar >> wTmp; m_nID = (int)wTmp;
        ar >> dwTmp; m_nVal1 = (int)dwTmp;
        ar >> dwTmp; m_nVal2 = (int)dwTmp;
        ar >> m_strMsg;
    }
}

#ifdef _DEBUG
void CEventMessageRcd::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    BoardEvt = %s, nID = %d, nVal1 = %d, nVal2 = %d\r\n",
             (LPCTSTR)(m_bIsBoardEvent ? "TRUE" : "FALSE"), m_nID, m_nVal1, m_nVal2);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////

void CCompoundMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
        ar << (WORD)m_bGroupBegin;
    else
    {
        WORD wTmp;
        ar >> wTmp; m_bGroupBegin = (BOOL)wTmp;
    }
}

#ifdef _DEBUG
void CCompoundMove::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "    %s Compound Move.\r\n", m_bGroupBegin ? "Begin" : "End");
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

CMoveList::CMoveList()
{
    m_nSeqNum = 0;
    m_nSkipCount = 0;
    m_bSkipKeepInd = FALSE;
    m_bCompoundMove = FALSE;
    m_nCompoundBaseIndex = -1;
    m_nPlaybackLock = 0;
    m_pCompoundBaseBookMark = NULL;
    m_bCompoundSingleStep = FALSE;
    m_pStateSave = NULL;
}

CMoveList::~CMoveList()
{
    Clear();
}

/////////////////////////////////////////////////////////////////////
// This is a crappy way to clone a move list but it's easier
// than propagating virtual Clone() methods throughout the
// move list object structure. It's at times like these I wish I
// could just toss it all and start over in a decent language like
// C#. The whole reason for doing this is to patch over side effects
// of switching to in-memory histories.

CMoveList* CMoveList::CloneMoveList(CGamDoc* pDoc, CMoveList* pMoveList)
{
    CMemFile memFile;
    CArchive arStore(&memFile, CArchive::store);
    arStore.m_pDocument = pDoc;
    pMoveList->Serialize(arStore, FALSE);
    arStore.Close();

    memFile.SeekToBegin();
    CArchive arLoad(&memFile, CArchive::load);
    arLoad.m_pDocument = pDoc;
    CMoveList* pNewMoveList = new CMoveList();
    pNewMoveList->Serialize(arLoad, FALSE);
    return pNewMoveList;
}

/////////////////////////////////////////////////////////////////////
// CMoveList methods....

BOOL CMoveList::ValidatePieces(CGamDoc* pDoc)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return FALSE;

    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CMoveRecord* pRcd = GetNext(pos);
        ASSERT(pRcd != NULL);
        if (!pRcd->ValidatePieces(pDoc))
            return FALSE;
    }
    return TRUE;
}

BOOL CMoveList::IsThisMovePossible(int nIndex)
{
    return nIndex < GetCount() && nIndex >= 0;
}

BOOL CMoveList::IsWithinCompoundMove(int nIndex)
{
    if (nIndex < 0)
        return FALSE;

    POSITION posPrev = FindIndex(nIndex);

    // Start looking on previous record.
    if (posPrev != NULL)
        GetPrev(posPrev);
    while (posPrev != NULL)
    {
        CMoveRecord* pRcd = GetPrev(posPrev);
        ASSERT(pRcd != NULL);
        if (pRcd->GetType() == CMoveRecord::mrecCompoundMove)
            return ((CCompoundMove*)pRcd)->IsGroupBegin();
    }
    return FALSE;
}

// Returns the starting move index
int CMoveList::SetStartingState()
{
    POSITION pos = FindIndex(1);
    ASSERT(pos != NULL);
    CGameStateRcd* pRcd = (CGameStateRcd*)GetAt(pos);
    int nStartIndex = 2;

    if (pRcd->GetType() != CMoveRecord::mrecState)
    {
        POSITION pos = FindIndex(0);
        ASSERT(pos != NULL);
        pRcd = (CGameStateRcd*)GetAt(pos);
        ASSERT(pRcd->GetType() == CMoveRecord::mrecState); // This *HAS* to be TRUE
        nStartIndex = 1;
    }
    pRcd->GetGameState()->RestoreState();
    return nStartIndex;
}

// PushAndSetState saves the current state of the playback and
// sets the playback state to what is should be at the specified
// move index.

void CMoveList::PushAndSetState(CGamDoc* pDoc, int nIndex)
{
    ASSERT(m_pStateSave == NULL); // Only one push allowed
    m_pStateSave = new CGameState(pDoc);
    pDoc->FlushAllIndicators();
    m_pStateSave->SaveState();
    m_bQuietPlaybackSave = pDoc->IsQuietPlayback();
    pDoc->SetQuietPlayback(TRUE);

    int nCurIndex = SetStartingState();
    if (nCurIndex < nIndex)
    {
        while ((nCurIndex = DoMove(pDoc, nCurIndex)) < nIndex)
            pDoc->FlushAllIndicators();
    }
    pDoc->FlushAllIndicators();
}

// Undoes what PushAndSetState did.
void CMoveList::PopAndRestoreState(CGamDoc* pDoc)
{
    ASSERT(m_pStateSave != NULL); // Better be one!
    pDoc->SetLoadingVersion(NumVersion(fileGsnVerMajor, fileGsnVerMinor));
    m_pStateSave->RestoreState();
    delete m_pStateSave;
    m_pStateSave = NULL;
    pDoc->SetQuietPlayback(m_bQuietPlaybackSave);
}

int CMoveList::FindPreviousMove(CGamDoc* pDoc, int nIndex)
{
    CMoveRecord* pRcd;
    POSITION     posPrev;
    int          nCurIndex;

    BOOL bWithinCompoundMove = IsWithinCompoundMove(nIndex);
    if (bWithinCompoundMove && !m_bCompoundSingleStep)
    {
        // This case only happens if compound move single step was on
        // and then turned of while we were stepping WITHIN a
        // compound move group. In this case we simply locate the
        // starting record and return that record number.
        nCurIndex = nIndex;
        posPrev = FindIndex(nCurIndex);
        GetPrev(posPrev);           // Point to previous record.
        while (TRUE)
        {
            pRcd = GetPrev(posPrev);
            nCurIndex--;
            ASSERT(pRcd);       // NULL WOULD MEAN DATA IS DAMAGED!!!
            if (pRcd->GetType() == CMoveRecord::mrecCompoundMove)
                return nCurIndex;           // Found it.
        }
    }

CHECK_AGAIN:
    if (nIndex >= 0)
    {
        nCurIndex = nIndex - 1;
        posPrev = FindIndex(nIndex);
        ASSERT(posPrev);
        if (posPrev == NULL)
            return 0;
        CMoveRecord* pRcd = GetPrev(posPrev);// First GetPrev() gets current record
    }
    else
    {
        // We are past the end of the list. Last record is end
        // of previous move.
        posPrev = GetTailPosition();
        nCurIndex = GetCount() - 1;
    }
    pRcd = GetPrev(posPrev);
    ASSERT(pRcd != NULL);
    if (pRcd == NULL)
        return 0;

    // Another weird special case...If the record is an end of compound
    // move record and we are in single step mode, then step back one more
    // record.
    if (m_bCompoundSingleStep &&
        pRcd->GetType() == CMoveRecord::mrecCompoundMove &&
        !((CCompoundMove*)pRcd)->IsGroupBegin())
    {
        pRcd = GetPrev(posPrev);
        nCurIndex--;
    }

    // Use different search approach depending on whether or not the
    // previous record ended a compound move.

    ASSERT(nCurIndex >= 0);
    if (pRcd->GetType() == CMoveRecord::mrecCompoundMove &&
        !((CCompoundMove*)pRcd)->IsGroupBegin() && !m_bCompoundSingleStep)
    {
        // Previous move ended a compound move. Search for
        // starting record of this compound move grouping.
        while (TRUE)
        {
            pRcd = GetPrev(posPrev);
            nCurIndex--;
            ASSERT(pRcd);       // NULL WOULD MEAN DATA IS DAMAGED!!!
            if (pRcd->GetType() == CMoveRecord::mrecCompoundMove)
                break;          // Found it.
        }
    }
    else
    {
        // Search for starting record with this sequence number.
        int nSeqNum = pRcd->GetSeqNum();
        do
        {
            pRcd = GetPrev(posPrev);
            nCurIndex--;
        }
        while (pRcd != NULL && pRcd->GetSeqNum() == nSeqNum);
        nCurIndex++;
        if ((pRcd->GetType() == CMoveRecord::mrecCompoundMove && m_bCompoundSingleStep))
        {
            nIndex = nCurIndex;
            goto CHECK_AGAIN;       // Back another record
        }
        // If this move is hidden for this player, step back another move
        // and try again.
        PushAndSetState(pDoc, nCurIndex);   // Need to make sure game state is correct
        BOOL bMoveIsHidden = IsMoveHidden(pDoc, nCurIndex);
        PopAndRestoreState(pDoc);

        if (bMoveIsHidden)
        {
            nIndex = nCurIndex;
            goto CHECK_AGAIN;       // Back another record
        }
    }
    ASSERT(nCurIndex >= 0);
    return nCurIndex;
}

// Check for hidden operations. If the operation
// has all hidden portions, the entire move will be
// done in 'quiet' mode.

BOOL CMoveList::IsMoveHidden(CGamDoc* pDoc, int nIndex)
{
    POSITION posFirst = FindIndex(nIndex);
    ASSERT(posFirst);
    if (posFirst == NULL)
        return FALSE;

    int nGrp = INT_MIN;
    int nNextIndex = nIndex;

    POSITION pos = posFirst;
    int nElementInGroup = 0;
    while (pos != NULL)
    {
        CMoveRecord* pRcd = GetNext(pos);
        if (nGrp == INT_MIN)
            nGrp = pRcd->GetSeqNum();
        if (nGrp != pRcd->GetSeqNum())
            break;
        nNextIndex++;                       // Determine for caller
        // For purposes of this scan certain records aren't considered
        // when determining whether a move is entirely hidden.
        if (pRcd->GetType() != CMoveRecord::mrecEvtMsg)
        {
            if (!pRcd->IsMoveHidden(pDoc, nElementInGroup))
                return FALSE;
        }
        nElementInGroup++;
    }
    return TRUE;
}

// This routine plays back the moves for the group at the
// specified index. Returns the index of the next move group.

int CMoveList::DoMove(CGamDoc* pDoc, int nIndex, BOOL bAutoStepHiddenMove /* = TRUE*/)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return -1;

    m_nPlaybackLock++;                  // Stop recursion

    int nNextIndex = nIndex;

    m_nSkipCount = 0;                   // Make sure we don't have any pent up skips

    do
    {
        // Check at the start of the loop. This causes the next move
        // to play normally if the current move is skipped.
        if (m_nSkipCount > 0)
        {
            m_nSkipCount--;
            if (!m_bSkipKeepInd)
                pDoc->FlushAllIndicators();
        }
        pDoc->FlushAllSelections();

        nNextIndex = nIndex;

        BOOL bCompoundMove = FALSE;
        BOOL bDoNextMove = FALSE;           // Set if hidden move was executed

        do
        {
            bDoNextMove = FALSE;

            if (nIndex >= GetCount() || nIndex < 0)
                break;

            POSITION posFirst = FindIndex(nIndex);
            ASSERT(posFirst);
            if (posFirst == NULL)
                break;

            // First check for compound move record...
            CMoveRecord* pRcd = GetAt(posFirst);
            if (pRcd->GetType() == CMoveRecord::mrecCompoundMove)
            {
                GetNext(posFirst);              // Step past record
                nNextIndex++;                   // calc'ed for caller
                if (!((CCompoundMove*)pRcd)->IsGroupBegin() && !m_bCompoundSingleStep)
                    break;
                if (!m_bCompoundSingleStep)
                    bCompoundMove = TRUE;
            }

            POSITION pos = posFirst;
            int  nGrp = INT_MIN;

            if (bCompoundMove)
                pDoc->FlushAllSelections();

            // Check for hidden operations. If the operation
            // has any hidden portions, the entire move will be
            // done in 'quiet' mode.

            BOOL bQuietModeSave = pDoc->IsQuietPlayback();

            if (!pDoc->IsQuietPlayback() && IsMoveHidden(pDoc, nIndex))
            {
                pDoc->SetQuietPlayback(TRUE);
                bDoNextMove = bAutoStepHiddenMove;
            }

            // Call setup routines

            pos = posFirst;
            int nElementInGroup = 0;
            while (pos != NULL)
            {
                CMoveRecord* pRcd = GetNext(pos);
                if (nGrp == INT_MIN)
                    nGrp = pRcd->GetSeqNum();
                if (nGrp != pRcd->GetSeqNum())
                    break;
                nNextIndex++;                   // Determine for caller
                BOOL bHidden = pRcd->IsMoveHidden(pDoc, nElementInGroup);
                BOOL bTmpQuietSave = pDoc->IsQuietPlayback();
                if (bHidden)
                    pDoc->SetQuietPlayback(TRUE);
                pRcd->DoMoveSetup(pDoc, nElementInGroup);
                if (bHidden)
                    pDoc->SetQuietPlayback(bTmpQuietSave);
                nElementInGroup++;
            }

            // Wait a moment.

            if (!pDoc->IsQuietPlayback())
                GetApp()->Delay(stepDelay, (BOOL*)&m_nSkipCount);

            // Do actual moves

            pos = posFirst;
            nElementInGroup = 0;
            while (pos != NULL)
            {
                CMoveRecord* pRcd = GetNext(pos);
                if (nGrp != pRcd->GetSeqNum())
                    break;
                BOOL bHidden = pRcd->IsMoveHidden(pDoc, nElementInGroup);
                BOOL bTmpQuietSave = pDoc->IsQuietPlayback();
                if (bHidden)
                    pDoc->SetQuietPlayback(TRUE);
                pRcd->DoMove(pDoc, nElementInGroup);
                if (bHidden)
                    pDoc->SetQuietPlayback(bTmpQuietSave);
                nElementInGroup++;
            }

            // Do move clean up

            pos = posFirst;
            nElementInGroup = 0;
            while (pos != NULL)
            {
                CMoveRecord* pRcd = GetNext(pos);
                if (nGrp != pRcd->GetSeqNum())
                    break;
                BOOL bHidden = pRcd->IsMoveHidden(pDoc, nElementInGroup);
                BOOL bTmpQuietSave = pDoc->IsQuietPlayback();
                if (bHidden)
                    pDoc->SetQuietPlayback(TRUE);
                pRcd->DoMoveCleanup(pDoc, nElementInGroup);
                if (bHidden)
                    pDoc->SetQuietPlayback(bTmpQuietSave);
                nElementInGroup++;
            }

            // Restore quite mode playback if mode was initially different.
            pDoc->SetQuietPlayback(bQuietModeSave);

            nIndex = nNextIndex >= GetCount() ? -1 : nNextIndex;

            // Short delay between moves in compound move.
            if (bCompoundMove && !pDoc->IsQuietPlayback())
                GetApp()->Delay((2 * stepDelay) / 3, (BOOL*)&m_nSkipCount);

        } while (bCompoundMove || bDoNextMove);


        nNextIndex = nNextIndex >= GetCount() ? -1 : nNextIndex;

        if (nNextIndex >= 0 && m_bCompoundSingleStep)
        {
            // If the next record to be executed is a compound move
            // end record AND we are single stepping the compound
            // records, we need to step to the next record.
            POSITION pos = FindIndex(nNextIndex);
            CMoveRecord* pRcd = GetAt(pos);
            if (pRcd->GetType() == CMoveRecord::mrecCompoundMove &&
                !((CCompoundMove*)pRcd)->IsGroupBegin())
            {
                nNextIndex++;
                nNextIndex = nNextIndex >= GetCount() ? -1 : nNextIndex;
            }
        }
    } while (m_nSkipCount > 0);

    m_nPlaybackLock--;

    return nNextIndex;
}

POSITION CMoveList::AppendMoveRecord(CMoveRecord* pRec)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return NULL;

    pRec->SetSeqNum(m_nSeqNum);
    return AddTail(pRec);
}

POSITION CMoveList::PrependMoveRecord(CMoveRecord* pRec,
    BOOL bSetSeqNum /* = TRUE */)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return NULL;

    if (bSetSeqNum)
        pRec->SetSeqNum(m_nSeqNum);
    return AddHead(pRec);
}

void CMoveList::PurgeAfter(int nIndex)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (nIndex <= m_nCompoundBaseIndex && m_pCompoundBaseBookMark != NULL)
    {
        if (m_pCompoundBaseBookMark)
        {
            delete m_pCompoundBaseBookMark;
            m_pCompoundBaseBookMark = NULL;
        }
        m_bCompoundMove = FALSE;
        m_nCompoundBaseIndex = -1;
    }

    POSITION pos = FindIndex(nIndex);
    if (pos == NULL)
        return;             // Doesn't exist
    while (pos != NULL)
    {
        POSITION posPrev = pos;
        CMoveRecord* pRcd = GetNext(pos);
        delete pRcd;
        RemoveAt(posPrev);
    }
}

void CMoveList::Clear()
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (m_pCompoundBaseBookMark)
        delete m_pCompoundBaseBookMark;
    m_pCompoundBaseBookMark = NULL;
    m_bCompoundMove = FALSE;
    m_nCompoundBaseIndex = -1;

    m_nSeqNum = 0;
    while (!IsEmpty())
        delete (CMoveRecord*)RemoveHead();
}

void CMoveList::BeginRecordingCompoundMove(CGamDoc* pDoc)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (m_bCompoundMove)
    {
        if (m_nCompoundBaseIndex == GetCount() - 1) // Check if any moves recorded
            return;                             // Nope. Keep current marker record
        EndRecordingCompoundMove();             // Mark end of current block
    }
    ASSERT(!m_bCompoundMove);

    m_pCompoundBaseBookMark = new CGameState(pDoc);
    if (!m_pCompoundBaseBookMark->SaveState())
    {
        // Memory low warning?....
        delete m_pCompoundBaseBookMark;
        m_pCompoundBaseBookMark = NULL;
        return;
    }

    AssignNewMoveGroup();
    m_nCompoundBaseIndex = GetCount();
    AppendMoveRecord(new CCompoundMove(TRUE));
    m_bCompoundMove = TRUE;
}

void CMoveList::CancelRecordingCompoundMove(CGamDoc* pDoc)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (!m_bCompoundMove)
        return;                                 // Nothing to get rid of

    pDoc->FlushAllIndicators();

    ASSERT(m_pCompoundBaseBookMark != NULL);
    pDoc->SetLoadingVersion(NumVersion(fileGsnVerMajor, fileGsnVerMinor));
    if (!m_pCompoundBaseBookMark->RestoreState())
    {
        // Memory error message should be here
        delete m_pCompoundBaseBookMark;
        m_pCompoundBaseBookMark = NULL;
        return;
    }
    if (m_pCompoundBaseBookMark)
    {
        delete m_pCompoundBaseBookMark;
        m_pCompoundBaseBookMark = NULL;
    }
    m_bCompoundMove = FALSE;
    PurgeAfter(m_nCompoundBaseIndex);
    m_nCompoundBaseIndex = -1;
    pDoc->UpdateAllViews(NULL, HINT_GAMESTATEUSED);
}

void CMoveList::EndRecordingCompoundMove()
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    ASSERT(m_bCompoundMove);
    if (!m_bCompoundMove)
        return;                                 // Nothing to mark the end of

    AssignNewMoveGroup();
    if (m_nCompoundBaseIndex < GetCount() - 1)  // Check if any moves recorded
    {
        AppendMoveRecord(new CCompoundMove(FALSE));
        m_bCompoundMove = FALSE;
        m_nCompoundBaseIndex = -1;

        if (m_pCompoundBaseBookMark)
        {
            delete m_pCompoundBaseBookMark;
            m_pCompoundBaseBookMark = NULL;
        }
    }
    else
    {
        // Just make believe the compound move never occurred.
        PurgeAfter(m_nCompoundBaseIndex);
    }
}

void CMoveList::Serialize(CArchive& ar, BOOL bSaveUndo)
{
    ASSERT(m_nPlaybackLock == 0);

    if (ar.IsStoring())
    {
        ASSERT(m_pStateSave == NULL); // Should never save with this active!

        ar << (short)m_nSeqNum;
        ar << (WORD)m_bCompoundMove;
        ar << (DWORD)m_nCompoundBaseIndex;
        ar << (BYTE)(m_pCompoundBaseBookMark != NULL ? 1 : 0);
        if (m_pCompoundBaseBookMark)
            m_pCompoundBaseBookMark->Serialize(ar);

        ar << (WORD)GetCount();
        POSITION pos;
        for (pos = GetHeadPosition(); pos != NULL; )
        {
            CMoveRecord* pRcd = GetNext(pos);
            ASSERT(pRcd != NULL);
            ar << (short)pRcd->GetType();
            pRcd->Serialize(ar);
        }
    }
    else
    {
        Clear();
        WORD wCount;
        short sTmp;

        ar >> sTmp; m_nSeqNum = (int)sTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(0, 60))
        {
            BYTE  cTmp;
            WORD  wTmp;
            DWORD dwTmp;
            ar >> wTmp; m_bCompoundMove = (BOOL)wTmp;
            ar >> dwTmp; m_nCompoundBaseIndex = (int)dwTmp;
            ar >> cTmp;             // Check for a bookmark
            if (cTmp)
            {
                ASSERT(m_pCompoundBaseBookMark == NULL);
                if (m_pCompoundBaseBookMark)
                    delete m_pCompoundBaseBookMark;
                m_pCompoundBaseBookMark = new CGameState((CGamDoc*)ar.m_pDocument);
                m_pCompoundBaseBookMark->Serialize(ar);
            }
        }

        ar >> wCount;
        for (WORD i = 0; i < wCount; i++)
        {
            CMoveRecord* pRcd;
            short sType;
            ar >> sType;
            switch ((CMoveRecord::RcdType)sType)
            {
                case CMoveRecord::mrecState:
                    pRcd = new CGameStateRcd;
                    break;
                case CMoveRecord::mrecPMove:
                    pRcd = new CBoardPieceMove;
                    break;
                case CMoveRecord::mrecTMove:
                    pRcd = new CTrayPieceMove;
                    break;
                case CMoveRecord::mrecPSide:
                    pRcd = new CPieceSetSide;
                    break;
                case CMoveRecord::mrecPFacing:
                    pRcd = new CPieceSetFacing;
                    break;
                case CMoveRecord::mrecMMove:
                    pRcd = new CBoardMarkerMove;
                    break;
                case CMoveRecord::mrecMPlot:
                    pRcd = new CMovePlotList;
                    break;
                case CMoveRecord::mrecMsg:
                    pRcd = new CMessageRcd;
                    break;
                case CMoveRecord::mrecDelObj:
                    pRcd = new CObjectDelete;
                    break;
                case CMoveRecord::mrecCompoundMove:
                    pRcd = new CCompoundMove;
                    break;
                case CMoveRecord::mrecMFacing:
                    pRcd = new CMarkerSetFacing;
                    break;
                case CMoveRecord::mrecSetObjText:
                    pRcd = new CObjectSetText;
                    break;
                case CMoveRecord::mrecLockObj:
                    pRcd = new CObjectLockdown;
                    break;
                case CMoveRecord::mrecEvtMsg:
                    pRcd = new CEventMessageRcd;
                    break;
                case CMoveRecord::mrecPOwner:
                    pRcd = new CPieceSetOwnership;
                    break;
                default:
                    ASSERT(FALSE);
                    AfxThrowArchiveException(CArchiveException::badClass);
            }
            pRcd->Serialize(ar);
            AddTail(pRcd);
            BYTE cUndoFlag;
            if (CGamDoc::GetLoadingVersion() < NumVersion(2, 0))
                ar >> cUndoFlag;        // Eat UNDO flag info (never used)
        }
    }
}

#ifdef _DEBUG

static char *tblTypes[CMoveRecord::mrecMax] =
{
    "UnKnown", "GameStateSnapshot", "PieceToBoardMove", "PieceToTrayMove",
    "PieceSetSide", "MarkerToBoardMove", "MovePlotTrack", "PlayerMessage",
    "DeleteObject", "PieceFacing", "CompoundMove", "MarkerFacing",
    "SetObjectText", "LockObject", "EventMessage", "SetPieceOwnership"
};

void CMoveList::DumpToTextFile(CFile& file)
{
    char szBfr[256];
    wsprintf(szBfr, "Current Move Group: %d\r\n", m_nSeqNum);
    file.Write(szBfr, lstrlen(szBfr));
    wsprintf(szBfr, "Number of move records: %d\r\n", GetCount());
    file.Write(szBfr, lstrlen(szBfr));

    POSITION pos;
    int nIndex = 0;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CMoveRecord* pRcd = GetNext(pos);
        ASSERT(pRcd != NULL);
        CMoveRecord::RcdType eType = pRcd->GetType();
        ASSERT(eType >= 0 && eType < CMoveRecord::mrecMax);
        wsprintf(szBfr, "[Index=%04d; Seq=%04d: %s]\r\n", nIndex, pRcd->GetSeqNum(),
            (LPCSTR)tblTypes[eType]);
        file.Write(szBfr, lstrlen(szBfr));
        pRcd->DumpToTextFile(file);
        nIndex++;
    }
}
#endif

