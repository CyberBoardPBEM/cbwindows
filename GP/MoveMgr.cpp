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
    {
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << (m_nSeqNum == Invalid_v<size_t> ? uint16_t(0xFFFF) : value_preserving_cast<uint16_t>(m_nSeqNum));
        }
        else
        {
            CB::WriteCount(ar, m_nSeqNum);
        }
    }
    else
    {
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            uint16_t sTmp;
            ar >> sTmp; m_nSeqNum = (sTmp == uint16_t(0xFFFF) ? Invalid_v<size_t> : sTmp);
        }
        else
        {
            m_nSeqNum = CB::ReadCount(ar);
        }
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CMoveRecord::SerializeHiddenByPrivate(CGamDoc& /*doc*/,
                                    BoardSet& usedPrivateBoards)
{
    m_hiddenByPrivate = false;
    usedPrivateBoards.clear();
}

bool CMoveRecord::IsPrivateBoard(CGamDoc& pDoc, BoardID bid)
{
    CPlayBoard& pbrd = CheckedDeref(pDoc.GetPBoardManager()->
                                            GetPBoardBySerial(bid));
    return IsPrivateBoard(pDoc, pbrd);
}

bool CMoveRecord::IsPrivateBoard(CGamDoc& pDoc, const CPlayBoard& pbrd)
{
    return pbrd.IsPrivate() &&
                    pbrd.IsOwnedButNotByCurrentPlayer(pDoc);
}

/////////////////////////////////////////////////////////////////////
// CBoardPieceMove methods....

CBoardPieceMove::CBoardPieceMove(BoardID nBrdSerNum, PieceID pid, CPoint pnt,
    PlacePos ePos)
{
    m_eType = mrecPMove;
    // ------- //
    m_nBrdNum = nBrdSerNum;
    m_pid = pid;
    m_ptCtr = pnt;
    m_ePos = ePos;
}

BOOL CBoardPieceMove::ValidatePieces(const CGamDoc& pDoc) const
{
#ifdef _DEBUG
    BOOL bUsed = pDoc.GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CBoardPieceMove::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)));
    return bUsed;
#else
    return pDoc.GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CBoardPieceMove::DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(*pPBoard, ptCtr);
        CPlayBoard& pPBrdDest = CheckedDeref(pDoc.GetPBoardManager()->
            GetPBoardBySerial(m_nBrdNum));
        pDoc.IndicateBoardToBoardPieceMove(CheckedDeref(pPBoard), pPBrdDest, ptCtr, m_ptCtr,
            pObj->GetRect().Size());
    }
    else
        pDoc.SelectTrayItem(*pTray, m_pid);
}

void CBoardPieceMove::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard* pPBrdFrom;
    CTraySet* pTrayFrom;
    CPieceObj* pObj;

    CPlayBoard& pPBrdDest = CheckedDeref(pDoc.GetPBoardManager()->
        GetPBoardBySerial(m_nBrdNum));

    pDoc.EnsureBoardLocationVisible(pPBrdDest, m_ptCtr);

    if (pDoc.FindPieceCurrentLocation(m_pid, pTrayFrom, pPBrdFrom, pObj))
    {
        CRect rct = pObj->GetRect();
        CSize size = m_ptCtr - GetMidRect(rct);
        pDoc.PlaceObjectOnBoard(&pPBrdDest, pObj, size, m_ePos);
    }
    else
    {
        pDoc.PlacePieceOnBoard(m_ptCtr, m_pid, &pPBrdDest);
        VERIFY(pDoc.FindPieceOnBoard(m_pid, pObj) != NULL);

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.IndicateBoardPiece(pPBrdDest, ptCtr, rct.Size());
    }
    pDoc.SelectObjectOnBoard(pPBrdDest, pObj);
}

void CBoardPieceMove::DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
}

void CBoardPieceMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
        ar << m_nBrdNum;
        ar << (short)m_ePos;
        ar << (short)m_ptCtr.x;
        ar << (short)m_ptCtr.y;
    }
    else
    {
        short sTmp;
        ar >> m_pid;
        ar >> m_nBrdNum;
        ar >> sTmp; m_ePos = (PlacePos)sTmp;
        ar >> sTmp; m_ptCtr.x = sTmp;
        ar >> sTmp; m_ptCtr.y = sTmp;
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CBoardPieceMove::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    bool fromPrivate = false;
    bool fromHidden = false;
    CPlayBoard* pPBrdFrom;
    CTraySet* pTrayFrom;
    CPieceObj* pObj;
    if (doc.FindPieceCurrentLocation(m_pid, pTrayFrom, pPBrdFrom, pObj))
    {
        if (IsPrivateBoard(doc, CheckedDeref(pPBrdFrom)))
        {
            fromPrivate = true;
            BoardID bid = pPBrdFrom->GetBoard()->GetSerialNumber();
            if (usedPrivateBoards.find(bid) != usedPrivateBoards.end())
            {
                fromHidden = true;
            }
            else
            {
                usedPrivateBoards.insert(bid);
            }
        }
    }
    else
    {
        fromPrivate = pTrayFrom->GetTrayContentVisibility() != trayVizAllSides;
        fromHidden = fromPrivate;
    }

    bool destPrivate = false;
    bool destHidden = false;
    if (IsPrivateBoard(doc, m_nBrdNum))
    {
        destPrivate = true;
        if (usedPrivateBoards.find(m_nBrdNum) != usedPrivateBoards.end())
        {
            destHidden = true;
        }
        else
        {
            usedPrivateBoards.insert(m_nBrdNum);
        }
    }

    m_hiddenByPrivate = fromHidden && destHidden;
    if (!(fromPrivate && destPrivate))
    {
        usedPrivateBoards.clear();
    }
}

#ifdef _DEBUG
void CBoardPieceMove::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    board = %u, pos = %d, pid = %u, @(%d, %d)\r\n",
        value_preserving_cast<unsigned>(static_cast<BoardID::UNDERLYING_TYPE>(m_nBrdNum)), m_ePos, value_preserving_cast<unsigned>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)), m_ptCtr.x, m_ptCtr.y);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CTrayPieceMove methods....

CTrayPieceMove::CTrayPieceMove(size_t nTrayNum, PieceID pid, size_t nPos)
{
    m_eType = mrecTMove;
    // ------- //
    m_nTrayNum = nTrayNum;
    m_pid = pid;
    m_nPos = nPos;
}

BOOL CTrayPieceMove::ValidatePieces(const CGamDoc& pDoc) const
{
#ifdef _DEBUG
    BOOL bUsed = pDoc.GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CTrayPieceMove::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)));
    return bUsed;
#else
    return pDoc.GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CTrayPieceMove::DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(CheckedDeref(pPBoard), ptCtr);
        pDoc.IndicateBoardPiece(*pPBoard, ptCtr, rct.Size());
    }
    else
        pDoc.SelectTrayItem(CheckedDeref(pTray), m_pid);
}

void CTrayPieceMove::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CTraySet& pYGrp = pDoc.GetTrayManager()->GetTraySet(m_nTrayNum);
    pDoc.PlacePieceInTray(m_pid, pYGrp, m_nPos);
}

void CTrayPieceMove::DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CTraySet& pYGrp = pDoc.GetTrayManager()->GetTraySet(m_nTrayNum);
    pDoc.SelectTrayItem(pYGrp, m_pid);
}

void CTrayPieceMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<int16_t>(m_nTrayNum);
            ASSERT(m_nPos == Invalid_v<size_t> ||
                    m_nPos < size_t(uint16_t(int16_t(-1))));
            ar << (m_nPos == Invalid_v<size_t> ? int16_t(-1) : value_preserving_cast<int16_t>(m_nPos));
        }
        else
        {
            CB::WriteCount(ar, m_nTrayNum);
            CB::WriteCount(ar, m_nPos);
        }
    }
    else
    {
        ar >> m_pid;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            int16_t sTmp;
            ar >> sTmp; m_nTrayNum = value_preserving_cast<size_t>(sTmp);
            ar >> sTmp; m_nPos = sTmp == int16_t(-1) ? Invalid_v<size_t> : value_preserving_cast<size_t>(sTmp);
        }
        else
        {
            m_nTrayNum = CB::ReadCount(ar);
            m_nPos = CB::ReadCount(ar);
        }
    }
}

#ifdef _DEBUG
void CTrayPieceMove::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    tray = %zu, nPos = %zd, pid = %u\r\n",
        m_nTrayNum, m_nPos, value_preserving_cast<unsigned>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)));
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CPieceSetSide methods....

BOOL CPieceSetSide::ValidatePieces(const CGamDoc& pDoc) const
{
#ifdef _DEBUG
    BOOL bUsed = pDoc.GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CPieceSetSide::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)));
    return bUsed;
#else
    return pDoc.GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

BOOL CPieceSetSide::IsMoveHidden(const CGamDoc& pDoc,
                                int nMoveWithinGroup) const
{
    if (CMoveRecord::IsMoveHidden(pDoc, nMoveWithinGroup))
    {
        return true;
    }

    if (m_forceMoveHidden)
    {
        return true;
    }

    const CPlayBoard* pPBoard;
    const CTraySet* pTray;
    const CPieceObj* pObj;

    if (pDoc.GetPieceTable()->IsOwnedButNotByCurrentPlayer(m_pid, pDoc))
        return TRUE;

    if (pDoc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj))
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

void CPieceSetSide::DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(CheckedDeref(pPBoard), ptCtr);
        pDoc.IndicateBoardPiece(*pPBoard, ptCtr, rct.Size());
        pDoc.SelectObjectOnBoard(*pPBoard, pObj);
    }
    else
        pDoc.SelectTrayItem(CheckedDeref(pTray), m_pid);
}

void CPieceSetSide::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj))
        pDoc.InvertPlayingPieceOnBoard(*pObj, *pPBoard, m_flip, m_side);
    else
        pDoc.InvertPlayingPieceInTray(m_pid, m_flip, m_side, !m_forceMoveHidden, false);
}

void CPieceSetSide::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            if (m_flip != CPieceTable::fNext ||
                m_side >= size_t(2))
            {
                AfxThrowArchiveException(CArchiveException::badSchema);
            }
            ar << m_pid;
            ar << (BYTE)(m_side == size_t(0));
        }
        else
        {
            ar << m_pid;
            ar << value_preserving_cast<uint8_t>(m_flip);
            CB::WriteCount(ar, m_side);
        }
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            if (m_forceMoveHidden)
            {
                AfxThrowArchiveException(CArchiveException::badSchema);
            }
        }
        else
        {
            ar << static_cast<uint8_t>(m_forceMoveHidden);
        }
    }
    else
    {
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar >> m_pid;
            m_flip = CPieceTable::fNext;
            BYTE cTmp;
            ar >> cTmp; m_side = (cTmp ? size_t(0) : size_t(1));
        }
        else
        {
            ar >> m_pid;
            uint8_t temp;
            ar >> temp;
            m_flip = static_cast<CPieceTable::Flip>(temp);
            m_side = CB::ReadCount(ar);
        }
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            m_forceMoveHidden = false;
        }
        else
        {
            uint8_t temp;
            ar >> temp;
            m_forceMoveHidden = static_cast<bool>(temp);
        }
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CPieceSetSide::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;
    if (doc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj) &&
        IsPrivateBoard(doc, CheckedDeref(pPBoard)))
    {
        BoardID bid = pPBoard->GetBoard()->GetSerialNumber();
        if (usedPrivateBoards.find(bid) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(bid);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CPieceSetSide::DumpToTextFile(const CGamDoc& pDoc, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    Piece %u is flipped:%s to %s visible.\r\n",
        value_preserving_cast<unsigned>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)),
        [](CPieceTable::Flip f) {
            switch (f) {
                case CPieceTable::fPrev:
                    return "prev";
                case CPieceTable::fNext:
                    return "next";
                case CPieceTable::fSelect:
                    return "select";
                case CPieceTable::fRandom:
                    return "random";
                default:
                    ASSERT(!"invalid value");
                    return "invalid";
            }
        }(m_flip),
        [](const CGamDoc& pDoc, PieceID pid, size_t side) {
            if (pDoc.GetPieceTable()->GetSides(pid) <= size_t(2))
            {
                return side == size_t(0) ? std::string("front") : std::string("back");
            }
            else
            {
                return "side " + std::to_string(side);
            }
        }(pDoc, m_pid, m_side).c_str());
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CPieceSetFacing methods....

BOOL CPieceSetFacing::ValidatePieces(const CGamDoc& pDoc) const
{
#ifdef _DEBUG
    BOOL bUsed = pDoc.GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CPieceSetFacing::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)));
    return bUsed;
#else
    return pDoc.GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CPieceSetFacing::DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(CheckedDeref(pPBoard), ptCtr);
        pDoc.IndicateBoardPiece(*pPBoard, ptCtr, rct.Size());
        pDoc.SelectObjectOnBoard(*pPBoard, pObj);
    }
    else
        pDoc.SelectTrayItem(CheckedDeref(pTray), m_pid);
}

void CPieceSetFacing::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj))
        pDoc.ChangePlayingPieceFacingOnBoard(*pObj, pPBoard, m_nFacingDegCW);
    else
        pDoc.ChangePlayingPieceFacingInTray(m_pid, m_nFacingDegCW);
}

void CPieceSetFacing::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
        ar << m_nFacingDegCW;
    }
    else
    {
        ar >> m_pid;
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   //Ver2.90
        {
            BYTE cTmp;
            ar >> cTmp;
            m_nFacingDegCW = value_preserving_cast<uint16_t>(5 * cTmp);
        }
        else
        {
            ar >> m_nFacingDegCW;
        }
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CPieceSetFacing::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;
    if (doc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj) &&
        IsPrivateBoard(doc, CheckedDeref(pPBoard)))
    {
        BoardID bid = pPBoard->GetBoard()->GetSerialNumber();
        if (usedPrivateBoards.find(bid) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(bid);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CPieceSetFacing::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    Piece %u is rotated %d degrees.\r\n", value_preserving_cast<unsigned>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)), m_nFacingDegCW);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CPieceSetFacing methods....

BOOL CPieceSetOwnership::ValidatePieces(const CGamDoc& pDoc) const
{
#ifdef _DEBUG
    BOOL bUsed = pDoc.GetPieceTable()->IsPieceUsed(m_pid);
    if (!bUsed)
        TRACE1("CPieceSetOwnership::ValidatePieces - Piece %u not in piece table.\n", value_preserving_cast<UINT>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)));
    return bUsed;
#else
    return pDoc.GetPieceTable()->IsPieceUsed(m_pid);
#endif
}

void CPieceSetOwnership::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;

    if (pDoc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj))
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(CheckedDeref(pPBoard), ptCtr);
        pDoc.IndicateBoardPiece(*pPBoard, ptCtr, rct.Size());
        pDoc.SelectObjectOnBoard(*pPBoard, pObj);
    }
    else
        pDoc.SelectTrayItem(CheckedDeref(pTray), m_pid);
    pDoc.SetPieceOwnership(m_pid, m_dwOwnerMask);
}

void CPieceSetOwnership::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
        ar << m_dwOwnerMask;
    }
    else
    {
        WORD wTmp;
        ar >> m_pid;
        if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10))
        {
            ar >> wTmp;
            m_dwOwnerMask = UPGRADE_OWNER_MASK(wTmp);
        }
        else
            ar >> m_dwOwnerMask;
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CPieceSetOwnership::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CPieceObj* pObj;
    if (doc.FindPieceCurrentLocation(m_pid, pTray, pPBoard, pObj) &&
        IsPrivateBoard(doc, CheckedDeref(pPBoard)))
    {
        BoardID bid = pPBoard->GetBoard()->GetSerialNumber();
        if (usedPrivateBoards.find(bid) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(bid);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CPieceSetOwnership::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    Piece %u has ownership changed to 0x%X.\r\n",
        value_preserving_cast<unsigned>(static_cast<PieceID::UNDERLYING_TYPE>(m_pid)), m_dwOwnerMask);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CMarkerSetFacing methods....

CMarkerSetFacing::CMarkerSetFacing(ObjectID dwObjID, MarkID mid, uint16_t nFacingDegCW)
{
    m_eType = mrecMFacing;
    m_dwObjID = dwObjID;
    m_mid = mid;
    m_nFacingDegCW = nFacingDegCW;
}

void CMarkerSetFacing::DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc.FindObjectOnBoard(m_dwObjID, pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc.IndicateBoardPiece(*pPBoard, ptCtr, rct.Size());
        pDoc.SelectObjectOnBoard(*pPBoard, pObj);
    }
    else
        ASSERT(FALSE);          // SHOULDN'T HAPPEN
}

void CMarkerSetFacing::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc.FindObjectOnBoard(m_dwObjID, pObj);

    if (pPBoard != NULL)
        pDoc.ChangeMarkerFacingOnBoard(*static_cast<CMarkObj*>(pObj), pPBoard, m_nFacingDegCW);
    else
        ASSERT(FALSE);          // SHOULDN'T HAPPEN
}

void CMarkerSetFacing::Serialize(CArchive& ar)              // VER2.0 is first time used
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_dwObjID;
        ar << m_mid;
        ar << m_nFacingDegCW;
    }
    else
    {
        ar >> m_dwObjID;
        ar >> m_mid;
        ar >> m_nFacingDegCW;
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   //Ver2.90
            m_nFacingDegCW *= uint16_t(5);                                // Convert old values to degrees
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CMarkerSetFacing::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    CDrawObj* pObj;
    CPlayBoard& pPBoard = CheckedDeref(doc.FindObjectOnBoard(m_dwObjID, pObj));
    if (IsPrivateBoard(doc, pPBoard))
    {
        BoardID bid = pPBoard.GetBoard()->GetSerialNumber();
        if (usedPrivateBoards.find(bid) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(bid);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CMarkerSetFacing::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    Marker dwObjID = %" PRIX64 ", mid = %u is rotated %d degrees.\r\n",
        value_preserving_cast<uint64_t>(reinterpret_cast<const ObjectID::UNDERLYING_TYPE&>(m_dwObjID)), value_preserving_cast<unsigned>(static_cast<MarkID::UNDERLYING_TYPE>(m_mid)), m_nFacingDegCW);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CBoardMarkerMove methods....

CBoardMarkerMove::CBoardMarkerMove(BoardID nBrdSerNum, ObjectID dwObjID, MarkID mid,
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

void CBoardMarkerMove::DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc.FindObjectOnBoard(m_dwObjID, pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(*pPBoard, ptCtr);
        CPlayBoard& pPBrdDest = CheckedDeref(pDoc.GetPBoardManager()->
            GetPBoardBySerial(m_nBrdNum));
        pDoc.IndicateBoardToBoardPieceMove(*pPBoard, pPBrdDest, ptCtr, m_ptCtr,
            pObj->GetRect().Size());
    }
    else
        pDoc.SelectMarkerPaletteItem(m_mid);
}

void CBoardMarkerMove::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard& pPBrdDest = CheckedDeref(pDoc.GetPBoardManager()->
        GetPBoardBySerial(m_nBrdNum));

    pDoc.EnsureBoardLocationVisible(pPBrdDest, m_ptCtr);

    CDrawObj* pObj;
    CPlayBoard* pPBrdFrom = pDoc.FindObjectOnBoard(m_dwObjID, pObj);

    if (pPBrdFrom != NULL)
    {
        CRect rct = pObj->GetRect();
        CSize size = m_ptCtr - GetMidRect(rct);
        pDoc.PlaceObjectOnBoard(&pPBrdDest, pObj, size, m_ePos);
    }
    else
    {
        // Need to create since doesn't currently exist
        pObj = &pDoc.CreateMarkerObject(&pPBrdDest, m_mid, m_ptCtr, m_dwObjID);

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.IndicateBoardPiece(pPBrdDest, ptCtr, rct.Size());
    }
}

void CBoardMarkerMove::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_mid;
        ar << m_dwObjID;
        ar << m_nBrdNum;
        ar << (short)m_ePos;
        ar << (short)m_ptCtr.x;
        ar << (short)m_ptCtr.y;
    }
    else
    {
        short sTmp;
        ar >> m_mid;
        ar >> m_dwObjID;
        ar >> m_nBrdNum;
        ar >> sTmp; m_ePos = (PlacePos)sTmp;
        ar >> sTmp; m_ptCtr.x = sTmp;
        ar >> sTmp; m_ptCtr.y = sTmp;
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CBoardMarkerMove::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    if (IsPrivateBoard(doc, m_nBrdNum))
    {
        if (usedPrivateBoards.find(m_nBrdNum) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(m_nBrdNum);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CBoardMarkerMove::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    board = %u, pos = %d, dwObjID = %" PRIX64 ", mid = %u, @(%d, %d)\r\n",
        value_preserving_cast<unsigned>(static_cast<BoardID::UNDERLYING_TYPE>(m_nBrdNum)), m_ePos, value_preserving_cast<uint64_t>(reinterpret_cast<const ObjectID::UNDERLYING_TYPE&>(m_dwObjID)), value_preserving_cast<unsigned>(static_cast<MarkID::UNDERLYING_TYPE>(m_mid)), m_ptCtr.x, m_ptCtr.y);
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

void CObjectDelete::DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc.FindObjectOnBoard(m_dwObjID, pObj);
    ASSERT(pPBoard);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc.IndicateBoardPiece(*pPBoard, ptCtr, pObj->GetRect().Size());
    }
}

void CObjectDelete::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = pDoc.FindObjectOnBoard(m_dwObjID, pObj);
    ASSERT(pPBoard);

    if (pPBoard != NULL)
    {
        std::vector<CB::not_null<CDrawObj*>> list;
        list.push_back(pObj);
        pDoc.DeleteObjectsInTable(list);
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

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CObjectDelete::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    CDrawObj* pObj;
    CPlayBoard* pPBoard = doc.FindObjectOnBoard(m_dwObjID, pObj);
    ASSERT(pPBoard);
    if (!pPBoard)
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
        return;
    }
    if (IsPrivateBoard(doc, *pPBoard))
    {
        BoardID bid = pPBoard->GetBoard()->GetSerialNumber();
        if (usedPrivateBoards.find(bid) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(bid);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CObjectDelete::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    dwObjID = %" PRIX64 "\r\n", value_preserving_cast<uint64_t>(reinterpret_cast<const ObjectID::UNDERLYING_TYPE&>(m_dwObjID)));
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

BOOL CObjectSetText::IsMoveHidden(const CGamDoc& pDoc,
                                int nMoveWithinGroup) const
{
    if (CMoveRecord::IsMoveHidden(pDoc, nMoveWithinGroup))
    {
        return true;
    }

    const CPlayBoard* pPBoard = NULL;
    const CTraySet* pTray = NULL;
    const CDrawObj* pObj = NULL;
    const CPieceObj* pPObj = NULL;

    if (IsGameElementAPiece(m_elem))
    {
        PieceID pid = GetPieceIDFromElement(m_elem);
        if (pDoc.GetPieceTable()->IsOwnedButNotByCurrentPlayer(pid, pDoc))
            return TRUE;

        pDoc.FindPieceCurrentLocation(pid, pTray, pPBoard, pPObj);
    }
    else
        pPBoard = pDoc.FindObjectOnBoard(static_cast<ObjectID>(m_elem), pObj);
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

void CObjectSetText::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard* pPBoard;
    CTraySet* pTray;
    CDrawObj* pObj = nullptr;
    CPieceObj* pPObj;

    pDoc.SetGameElementString(m_elem,
        m_strObjText.IsEmpty() ? NULL : (LPCTSTR)m_strObjText);

    if (IsGameElementAPiece(m_elem))
    {
        if (pDoc.FindPieceCurrentLocation(GetPieceIDFromElement(m_elem),
                pTray, pPBoard, pPObj))
            pObj = pPObj;
        else
            pDoc.SelectTrayItem(*pTray, GetPieceIDFromElement(m_elem), IDS_TIP_OBJTEXTCHG);
    }
    else
        pPBoard = pDoc.FindObjectOnBoard(static_cast<ObjectID>(m_elem), pObj);

    if (pPBoard != NULL)
    {
        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc.IndicateBoardPiece(*pPBoard, ptCtr, pObj->GetRect().Size());

        // Show a balloon tip so person knows what happened
        if (nMoveWithinGroup == 0)
            pDoc.IndicateTextTipOnBoard(*pPBoard, ptCtr, IDS_TIP_OBJTEXTCHG);
    }
}

void CObjectSetText::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_elem;
        ar << m_strObjText;
    }
    else
    {
        ar >> m_elem;
        ar >> m_strObjText;
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CObjectSetText::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    CPlayBoard* pPBoard;
    if (IsGameElementAPiece(m_elem))
    {
        CTraySet* pTray;
        CPieceObj* pPObj;
        doc.FindPieceCurrentLocation(GetPieceIDFromElement(m_elem),
            pTray, pPBoard, pPObj);
    }
    else
    {
        CDrawObj* pObj;
        pPBoard = doc.FindObjectOnBoard(static_cast<ObjectID>(m_elem), pObj);
    }

    if (pPBoard && IsPrivateBoard(doc, *pPBoard))
    {
        BoardID bid = pPBoard->GetBoard()->GetSerialNumber();
        if (usedPrivateBoards.find(bid) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(bid);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CObjectSetText::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    elem = %" PRIX64 ", text = \"%s\"\r\n", value_preserving_cast<uint64_t>(reinterpret_cast<const GameElement::UNDERLYING_TYPE&>(m_elem)), (LPCTSTR)m_strObjText);
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

BOOL CObjectLockdown::IsMoveHidden(const CGamDoc& pDoc,
                                int nMoveWithinGroup) const
{
    if (CMoveRecord::IsMoveHidden(pDoc, nMoveWithinGroup))
    {
        return true;
    }

    const CDrawObj* pObj;
    const CPieceObj* pPObj;
    const CPlayBoard* pPBoard;

    if (IsGameElementAPiece(m_elem))
    {
        PieceID pid = GetPieceIDFromElement(m_elem);
        if (pDoc.GetPieceTable()->IsOwnedButNotByCurrentPlayer(pid, pDoc))
            return TRUE;
        pPBoard = pDoc.FindPieceOnBoard(GetPieceIDFromElement(m_elem), pPObj);
        pObj = pPObj;
    }
    else
        pPBoard = pDoc.FindObjectOnBoard(static_cast<ObjectID>(m_elem), pObj);

    ASSERT(pObj != NULL);

    if (pPBoard != NULL && pPBoard->IsOwnedButNotByCurrentPlayer(pDoc))
        return TRUE;

    return FALSE;
}

void CObjectLockdown::DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CDrawObj* pObj;
    CPieceObj* pPObj;
    CPlayBoard* pPBoard;

    if (IsGameElementAPiece(m_elem))
    {
        pPBoard = pDoc.FindPieceOnBoard(GetPieceIDFromElement(m_elem), pPObj);
        pObj = pPObj;
    }
    else
        pPBoard = pDoc.FindObjectOnBoard(static_cast<ObjectID>(m_elem), pObj);


    ASSERT(pObj != NULL);

    if (pObj != NULL)
        pObj->ModifyDObjFlags(dobjFlgLockDown, m_bLockState);

    if (pPBoard != NULL)
    {

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(*pPBoard, ptCtr);
        pDoc.IndicateBoardPiece(*pPBoard, ptCtr, pObj->GetRect().Size());
    }
}

void CObjectLockdown::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    if (nMoveWithinGroup != 0)      // Only do this for the first record in group
        return;

    CDrawObj* pObj;
    CPieceObj* pPObj;
    CPlayBoard* pPBoard;

    if (IsGameElementAPiece(m_elem))
    {
        pPBoard = pDoc.FindPieceOnBoard(GetPieceIDFromElement(m_elem), pPObj);
        pObj = pPObj;
    }
    else
        pPBoard = pDoc.FindObjectOnBoard(static_cast<ObjectID>(m_elem), pObj);


    if (pPBoard != NULL)
    {

        CRect rct = pObj->GetRect();
        CPoint ptCtr = GetMidRect(rct);
        pDoc.EnsureBoardLocationVisible(*pPBoard, ptCtr);

        // Show a balloon tip so person knows what happened
        pDoc.IndicateTextTipOnBoard(*pPBoard, ptCtr,
            m_bLockState ? IDS_TIP_OBJLOCKED : IDS_TIP_OBJUNLOCKED);
    }
}

void CObjectLockdown::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_elem;
        ar << (WORD)m_bLockState;
    }
    else
    {
        WORD wTmp;
        ar >> m_elem;
        ar >> wTmp; m_bLockState = (BOOL)wTmp;
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CObjectLockdown::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    CPlayBoard* pPBoard;
    if (IsGameElementAPiece(m_elem))
    {
        CTraySet* pTray;
        CPieceObj* pPObj;
        doc.FindPieceCurrentLocation(GetPieceIDFromElement(m_elem),
            pTray, pPBoard, pPObj);
    }
    else
    {
        CDrawObj* pObj;
        pPBoard = doc.FindObjectOnBoard(static_cast<ObjectID>(m_elem), pObj);
    }

    if (pPBoard && IsPrivateBoard(doc, *pPBoard))
    {
        BoardID bid = pPBoard->GetBoard()->GetSerialNumber();
        if (usedPrivateBoards.find(bid) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(bid);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CObjectLockdown::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    elem = %" PRIX64 ", state = %d\r\n", value_preserving_cast<uint64_t>(reinterpret_cast<const GameElement::UNDERLYING_TYPE&>(m_elem)), m_bLockState);
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
// CGameStateRcd methods....

void CGameStateRcd::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    if (!m_pState->RestoreState(pDoc))
        AfxMessageBox(IDS_ERR_FAILEDSTATECHG, MB_OK | MB_ICONEXCLAMATION);
    if (!pDoc.IsQuietPlayback())
        pDoc.UpdateAllViews(NULL, HINT_GAMESTATEUSED);
}

void CGameStateRcd::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (!ar.IsStoring())
    {
        m_pState = MakeOwner<CGameState>();
    }
    ASSERT(m_pState != NULL);
    m_pState->Serialize(ar);
}

#ifdef _DEBUG
void CGameStateRcd::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    static char strMsg[] =
        "    To much to dump! (Trust me on this)\r\n";
    file.Write(strMsg, lstrlen(strMsg));
}
#endif

/////////////////////////////////////////////////////////////////////
// CMovePlotList

void CMovePlotList::SavePlotList(const CDrawList& pDwg)
{
    m_tblPlot.RemoveAll();
    for (CDrawList::const_iterator pos = pDwg.begin(); pos != pDwg.end(); ++pos)
    {
        const CDrawObj& pObj = **pos;
        if (pObj.GetType() == CDrawObj::drawLine)
        {
            const CLine& pLObj = static_cast<const CLine&>(pObj);
            m_tblPlot.Add((DWORD)MAKELONG(static_cast<int16_t>(pLObj.m_ptBeg.x), static_cast<int16_t>(pLObj.m_ptBeg.y)));
            m_tblPlot.Add((DWORD)MAKELONG(static_cast<int16_t>(pLObj.m_ptEnd.x), static_cast<int16_t>(pLObj.m_ptEnd.y)));
        }
    }
}

void CMovePlotList::DoMoveSetup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard& pPBrd = CheckedDeref(pDoc.GetPBoardManager()->GetPBoardBySerial(m_nBrdNum));
    pPBrd.SetPlotMoveMode(TRUE);
    for (int i = 0; i < m_tblPlot.GetSize(); i += 2)
    {
        CPoint ptA(m_tblPlot.GetAt(i));
        CPoint ptB(m_tblPlot.GetAt(i+1));
        pDoc.IndicateBoardPlotLine(pPBrd, ptA, ptB);
    }
}

void CMovePlotList::DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    CPlayBoard& pPBrd = CheckedDeref(pDoc.GetPBoardManager()->GetPBoardBySerial(m_nBrdNum));
    pPBrd.SetPlotMoveMode(FALSE);
}

void CMovePlotList::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_nBrdNum;
        ar << m_tblPlot;
    }
    else
    {
        ar >> m_nBrdNum;
        ar >> m_tblPlot;
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CMovePlotList::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    if (IsPrivateBoard(doc, m_nBrdNum))
    {
        if (usedPrivateBoards.find(m_nBrdNum) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(m_nBrdNum);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CMovePlotList::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    static char strMsg[] =
        "    To much to dump! (just plain lazy on this one)\r\n";
    file.Write(strMsg, lstrlen(strMsg));
}
#endif

/////////////////////////////////////////////////////////////////////

void CMessageRcd::DoMove(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    pDoc.MsgSetMessageText(m_strMsg);
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
void CMessageRcd::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    file.Write(m_strMsg, lstrlen(m_strMsg));
    file.Write("\r\n", 2);
}
#endif

/////////////////////////////////////////////////////////////////////

CEventMessageRcd::CEventMessageRcd(CString strMsg,
    BoardID nBoard, int x, int y)
{
    m_eType = mrecEvtMsg;
    m_bIsBoardEvent = TRUE;
    m_nBoard = nBoard;
    m_x = x;
    m_y = y;
    m_strMsg = strMsg;
}

CEventMessageRcd::CEventMessageRcd(CString strMsg,
    size_t nTray, PieceID pieceID)
{
    m_eType = mrecEvtMsg;
    m_bIsBoardEvent = FALSE;
    m_nTray = nTray;
    m_pieceID = pieceID;
    m_strMsg = strMsg;
}

void CEventMessageRcd::DoMoveCleanup(CGamDoc& pDoc, int nMoveWithinGroup) const
{
    if (m_bIsBoardEvent)        // Board event message
        pDoc.EventShowBoardNotification(m_nBoard, CPoint(m_x, m_y), m_strMsg);
    else                        // Tray event message
        pDoc.EventShowTrayNotification(m_nTray, m_pieceID, m_strMsg);
}

void CEventMessageRcd::Serialize(CArchive& ar)
{
    CMoveRecord::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_bIsBoardEvent;
        if (m_bIsBoardEvent)
        {
            ar << m_nBoard;
            ar << value_preserving_cast<DWORD>(m_x);
            ar << value_preserving_cast<DWORD>(m_y);
        }
        else
        {
            if (CB::GetVersion(ar) <= NumVersion(3, 90))
            {
                ar << value_preserving_cast<WORD>(m_nTray);
                ar << value_preserving_cast<DWORD>(static_cast<PieceID::UNDERLYING_TYPE>(m_pieceID));
                ar << (DWORD)0;
            }
            else
            {
                CB::WriteCount(ar, m_nTray);
                ar << m_pieceID;
            }
        }
        ar << m_strMsg;
    }
    else
    {
        WORD wTmp;
        DWORD dwTmp;
        ar >> wTmp; m_bIsBoardEvent = (int)wTmp;
        if (m_bIsBoardEvent)
        {
            ar >> m_nBoard;
            ar >> dwTmp; m_x = value_preserving_cast<int>(dwTmp);
            ar >> dwTmp; m_y = value_preserving_cast<int>(dwTmp);
        }
        else
        {
            if (CB::GetVersion(ar) <= NumVersion(3, 90))
            {
                ar >> wTmp; m_nTray = value_preserving_cast<size_t>(wTmp);
                ar >> dwTmp; m_pieceID = static_cast<PieceID>(dwTmp);
                ar >> dwTmp;
            }
            else
            {
                m_nTray = CB::ReadCount(ar);
                ar >> m_pieceID;
            }
        }
        ar >> m_strMsg;
    }
}

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CEventMessageRcd::SerializeHiddenByPrivate(CGamDoc& doc,
                                    BoardSet& usedPrivateBoards)
{
    if (m_bIsBoardEvent && IsPrivateBoard(doc, m_nBoard))
    {
        if (usedPrivateBoards.find(m_nBoard) != usedPrivateBoards.end())
        {
            m_hiddenByPrivate = true;
        }
        else
        {
            m_hiddenByPrivate = false;
            usedPrivateBoards.insert(m_nBoard);
        }
    }
    else
    {
        CMoveRecord::SerializeHiddenByPrivate(doc, usedPrivateBoards);
    }
}

#ifdef _DEBUG
void CEventMessageRcd::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    if (m_bIsBoardEvent)
    {
        sprintf(szBfr, "    BoardEvt = %s, nBoard = %u, (x, y) = (%d, %d)\r\n",
                 (LPCTSTR)(m_bIsBoardEvent ? "TRUE" : "FALSE"), value_preserving_cast<unsigned>(static_cast<BoardID::UNDERLYING_TYPE>(m_nBoard)), m_x, m_y);
    }
    else
    {
        sprintf(szBfr, "    BoardEvt = %s, nTray = %zu, pieceID = %u\r\n",
                 (LPCTSTR)(m_bIsBoardEvent ? "TRUE" : "FALSE"), m_nTray, value_preserving_cast<unsigned>(static_cast<PieceID::UNDERLYING_TYPE>(m_pieceID)));
    }
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

/* provide visual feedback for only the first move in a
    block of moves involving the same hidden board(s) */
void CCompoundMove::SerializeHiddenByPrivate(CGamDoc& /*doc*/,
                                    BoardSet& /*usedPrivateBoards*/)
{
    m_hiddenByPrivate = false;
    /* DoMove should never be called, so this shouldn't halt a
        run of hiddent moves, so don't clear usedPrivateBoards */
}

#ifdef _DEBUG
void CCompoundMove::DumpToTextFile(const CGamDoc& /*pDoc*/, CFile& file) const
{
    char szBfr[256];
    sprintf(szBfr, "    %s Compound Move.\r\n", m_bGroupBegin ? "Begin" : "End");
    file.Write(szBfr, lstrlen(szBfr));
}
#endif

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

CMoveList::CMoveList()
{
    m_nSeqNum = size_t(0);
    m_nSkipCount = 0;
    m_bSkipKeepInd = FALSE;
    m_bCompoundMove = FALSE;
    m_nCompoundBaseIndex = Invalid_v<size_t>;
    m_nPlaybackLock = 0;
    m_bCompoundSingleStep = FALSE;
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

OwnerPtr<CMoveList> CMoveList::CloneMoveList(CGamDoc& pDoc, const CMoveList& pMoveList)
{
    CMemFile memFile;
    CArchive arStore(&memFile, CArchive::store);
    arStore.m_pDocument = &pDoc;
    // store shouldn't modify src, so const_cast safe
    const_cast<CMoveList&>(pMoveList).Serialize(arStore, FALSE);
    arStore.Close();

    memFile.SeekToBegin();
    CArchive arLoad(&memFile, CArchive::load);
    arLoad.m_pDocument = &pDoc;
    OwnerPtr<CMoveList> pNewMoveList = MakeOwner<CMoveList>();
    pNewMoveList->Serialize(arLoad, FALSE);
    return pNewMoveList;
}

/////////////////////////////////////////////////////////////////////
// CMoveList methods....

BOOL CMoveList::ValidatePieces(const CGamDoc& pDoc) const
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return FALSE;

    const_iterator pos;
    for (pos = begin(); pos != end(); )
    {
        const CMoveRecord& pRcd = GetNext(pos);
        if (!pRcd.ValidatePieces(pDoc))
            return FALSE;
    }
    return TRUE;
}

bool CMoveList::IsThisMovePossible(size_t nIndex) const
{
    return nIndex < size() && nIndex != Invalid_v<size_t>;
}

bool CMoveList::IsWithinCompoundMove(size_t nIndex) const
{
    if (nIndex == Invalid_v<size_t>)
        return false;

    const_iterator posPrev = FindIndex(nIndex);

    // Start looking on previous record.
    if (posPrev != end())
        GetPrev(posPrev);
    while (posPrev != end())
    {
        const CMoveRecord& pRcd = GetPrev(posPrev);
        if (pRcd.GetType() == CMoveRecord::mrecCompoundMove)
            return static_cast<const CCompoundMove&>(pRcd).IsGroupBegin();
    }
    return false;
}

// Returns the starting move index
size_t CMoveList::SetStartingState(CGamDoc& doc)
{
    iterator pos = ++begin();
    ASSERT(pos != end());
    CB::not_null<CMoveRecord*> temp = pos->get();
    size_t nStartIndex = size_t(2);

    if (temp->GetType() != CMoveRecord::mrecState)
    {
        iterator pos = begin();
        ASSERT(pos != end());
        temp = pos->get();
        ASSERT(temp->GetType() == CMoveRecord::mrecState); // This *HAS* to be TRUE
        nStartIndex = size_t(1);
    }
    CGameStateRcd& pRcd = static_cast<CGameStateRcd&>(*temp);
    pRcd.GetGameState().RestoreState(doc);
    return nStartIndex;
}

// PushAndSetState saves the current state of the playback and
// sets the playback state to what is should be at the specified
// move index.

void CMoveList::PushAndSetState(CGamDoc& pDoc, size_t nIndex)
{
    ASSERT(m_pStateSave == NULL); // Only one push allowed
    m_pStateSave = new CGameState();
    pDoc.FlushAllIndicators();
    m_pStateSave->SaveState(pDoc);
    m_bQuietPlaybackSave = pDoc.IsQuietPlayback();
    pDoc.SetQuietPlayback(TRUE);

    size_t nCurIndex = SetStartingState(pDoc);
    if (nCurIndex < nIndex)
    {
        while ((nCurIndex = DoMove(pDoc, nCurIndex)) < nIndex)
            pDoc.FlushAllIndicators();
    }
    pDoc.FlushAllIndicators();
}

// Undoes what PushAndSetState did.
void CMoveList::PopAndRestoreState(CGamDoc& pDoc)
{
    ASSERT(m_pStateSave != NULL); // Better be one!
    pDoc.SetLoadingVersion(NumVersion(fileGsnVerMajor, fileGsnVerMinor));
    m_pStateSave->RestoreState(pDoc);
    m_pStateSave = NULL;
    pDoc.SetQuietPlayback(m_bQuietPlaybackSave);
}

size_t CMoveList::FindPreviousMove(CGamDoc& pDoc, size_t nIndex)
{
    const_iterator posPrev;
    size_t       nCurIndex;

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
            const CMoveRecord& pRcd = GetPrev(posPrev);
            nCurIndex--;
            if (pRcd.GetType() == CMoveRecord::mrecCompoundMove)
                return nCurIndex;           // Found it.
        }
    }

CHECK_AGAIN:
    if (nIndex != Invalid_v<size_t>)
    {
        nCurIndex = nIndex - size_t(1);
        posPrev = FindIndex(nIndex);
        ASSERT(posPrev != end());
        if (posPrev == end())
            return size_t(0);
        const CMoveRecord& pRcd = GetPrev(posPrev);// First GetPrev() gets current record
    }
    else
    {
        // We are past the end of the list. Last record is end
        // of previous move.
        posPrev = --end();
        nCurIndex = size() - size_t(1);
    }
    CB::not_null<const CMoveRecord*> pRcd = &GetPrev(posPrev);

    // Another weird special case...If the record is an end of compound
    // move record and we are in single step mode, then step back one more
    // record.
    if (m_bCompoundSingleStep &&
        pRcd->GetType() == CMoveRecord::mrecCompoundMove &&
        !static_cast<const CCompoundMove&>(*pRcd).IsGroupBegin())
    {
        pRcd = &GetPrev(posPrev);
        nCurIndex--;
    }

    // Use different search approach depending on whether or not the
    // previous record ended a compound move.

    ASSERT(nCurIndex >= 0);
    if (pRcd->GetType() == CMoveRecord::mrecCompoundMove &&
        !static_cast<const CCompoundMove&>(*pRcd).IsGroupBegin() && !m_bCompoundSingleStep)
    {
        // Previous move ended a compound move. Search for
        // starting record of this compound move grouping.
        while (TRUE)
        {
            pRcd = &GetPrev(posPrev);
            nCurIndex--;
            if (pRcd->GetType() == CMoveRecord::mrecCompoundMove)
                break;          // Found it.
        }
    }
    else
    {
        // Search for starting record with this sequence number.
        size_t nSeqNum = pRcd->GetSeqNum();
        do
        {
            pRcd = &GetPrev(posPrev);
            nCurIndex--;
        }
        while (pRcd->GetSeqNum() == nSeqNum);
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
    ASSERT(nCurIndex != Invalid_v<size_t>);
    return nCurIndex;
}

// Check for hidden operations. If the operation
// has all hidden portions, the entire move will be
// done in 'quiet' mode.

bool CMoveList::IsMoveHidden(const CGamDoc& pDoc, size_t nIndex) const
{
    const_iterator posFirst = FindIndex(nIndex);
    ASSERT(posFirst != end());
    if (posFirst == end())
        return false;

    size_t nGrp = Invalid_v<size_t>;
    size_t nNextIndex = nIndex;

    const_iterator pos = posFirst;
    int nElementInGroup = 0;
    while (pos != end())
    {
        const CMoveRecord& pRcd = GetNext(pos);
        if (nGrp == Invalid_v<size_t>)
            nGrp = pRcd.GetSeqNum();
        if (nGrp != pRcd.GetSeqNum())
            break;
        nNextIndex++;                       // Determine for caller
        // For purposes of this scan certain records aren't considered
        // when determining whether a move is entirely hidden.
        if (pRcd.GetType() != CMoveRecord::mrecEvtMsg)
        {
            if (!pRcd.IsMoveHidden(pDoc, nElementInGroup))
                return FALSE;
        }
        nElementInGroup++;
    }
    return TRUE;
}

// This routine plays back the moves for the group at the
// specified index. Returns the index of the next move group.

size_t CMoveList::DoMove(CGamDoc& pDoc, size_t nIndex, BOOL bAutoStepHiddenMove /* = TRUE*/)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return Invalid_v<size_t>;

    m_nPlaybackLock++;                  // Stop recursion

    size_t nNextIndex = nIndex;

    m_nSkipCount = 0;                   // Make sure we don't have any pent up skips

    do
    {
        // Check at the start of the loop. This causes the next move
        // to play normally if the current move is skipped.
        if (m_nSkipCount > 0)
        {
            m_nSkipCount--;
            if (!m_bSkipKeepInd)
                pDoc.FlushAllIndicators();
        }
        pDoc.FlushAllSelections();

        nNextIndex = nIndex;

        BOOL bCompoundMove = FALSE;
        BOOL bDoNextMove = FALSE;           // Set if hidden move was executed

        do
        {
            bDoNextMove = FALSE;

            if (nIndex >= size() || nIndex == Invalid_v<size_t>)
                break;

            const_iterator posFirst = FindIndex(nIndex);
            ASSERT(posFirst != end());
            if (posFirst == end())
                break;

            // First check for compound move record...
            const CMoveRecord& pRcd = GetAt(posFirst);
            if (pRcd.GetType() == CMoveRecord::mrecCompoundMove)
            {
                GetNext(posFirst);              // Step past record
                nNextIndex++;                   // calc'ed for caller
                if (!static_cast<const CCompoundMove&>(pRcd).IsGroupBegin() && !m_bCompoundSingleStep)
                    break;
                if (!m_bCompoundSingleStep)
                    bCompoundMove = TRUE;
            }

            const_iterator pos = posFirst;
            size_t nGrp = Invalid_v<size_t>;

            if (bCompoundMove)
                pDoc.FlushAllSelections();

            // Check for hidden operations. If the operation
            // has all hidden portions, the entire move will be
            // done in 'quiet' mode.

            BOOL bQuietModeSave = pDoc.IsQuietPlayback();

            if (!pDoc.IsQuietPlayback() && IsMoveHidden(pDoc, nIndex))
            {
                pDoc.SetQuietPlayback(TRUE);
                bDoNextMove = bAutoStepHiddenMove;
            }

            // Call setup routines

            pos = posFirst;
            int nElementInGroup = 0;
            while (pos != end())
            {
                const CMoveRecord& pRcd = GetNext(pos);
                if (nGrp == Invalid_v<size_t>)
                    nGrp = pRcd.GetSeqNum();
                if (nGrp != pRcd.GetSeqNum())
                    break;
                nNextIndex++;                   // Determine for caller
                BOOL bHidden = pRcd.IsMoveHidden(pDoc, nElementInGroup);
                BOOL bTmpQuietSave = pDoc.IsQuietPlayback();
                if (bHidden)
                    pDoc.SetQuietPlayback(TRUE);
                pRcd.DoMoveSetup(pDoc, nElementInGroup);
                if (bHidden)
                    pDoc.SetQuietPlayback(bTmpQuietSave);
                nElementInGroup++;
            }

            // Wait a moment.

            if (!pDoc.IsQuietPlayback())
                GetApp()->Delay(stepDelay, (BOOL*)&m_nSkipCount);

            // Do actual moves

            pos = posFirst;
            nElementInGroup = 0;
            while (pos != end())
            {
                const CMoveRecord& pRcd = GetNext(pos);
                if (nGrp != pRcd.GetSeqNum())
                    break;
                BOOL bHidden = pRcd.IsMoveHidden(pDoc, nElementInGroup);
                BOOL bTmpQuietSave = pDoc.IsQuietPlayback();
                if (bHidden)
                    pDoc.SetQuietPlayback(TRUE);
                pRcd.DoMove(pDoc, nElementInGroup);
                if (bHidden)
                    pDoc.SetQuietPlayback(bTmpQuietSave);
                nElementInGroup++;
            }

            // Do move clean up

            pos = posFirst;
            nElementInGroup = 0;
            while (pos != end())
            {
                const CMoveRecord& pRcd = GetNext(pos);
                if (nGrp != pRcd.GetSeqNum())
                    break;
                BOOL bHidden = pRcd.IsMoveHidden(pDoc, nElementInGroup);
                BOOL bTmpQuietSave = pDoc.IsQuietPlayback();
                if (bHidden)
                    pDoc.SetQuietPlayback(TRUE);
                pRcd.DoMoveCleanup(pDoc, nElementInGroup);
                if (bHidden)
                    pDoc.SetQuietPlayback(bTmpQuietSave);
                nElementInGroup++;
            }

            // Restore quite mode playback if mode was initially different.
            pDoc.SetQuietPlayback(bQuietModeSave);

            nIndex = nNextIndex >= size() ? Invalid_v<size_t> : nNextIndex;

            // Short delay between moves in compound move.
            if (bCompoundMove && !pDoc.IsQuietPlayback())
                GetApp()->Delay((2 * stepDelay) / 3, (BOOL*)&m_nSkipCount);

            #pragma message("warning:  TODO:  check for private board next step match")

        } while (bCompoundMove || bDoNextMove);


        nNextIndex = nNextIndex >= size() ? Invalid_v<size_t> : nNextIndex;

        if (nNextIndex != Invalid_v<size_t> && m_bCompoundSingleStep)
        {
            // If the next record to be executed is a compound move
            // end record AND we are single stepping the compound
            // records, we need to step to the next record.
            const_iterator pos = FindIndex(nNextIndex);
            const CMoveRecord& pRcd = GetAt(pos);
            if (pRcd.GetType() == CMoveRecord::mrecCompoundMove &&
                !static_cast<const CCompoundMove&>(pRcd).IsGroupBegin())
            {
                nNextIndex++;
                nNextIndex = nNextIndex >= size() ? Invalid_v<size_t> : nNextIndex;
            }
        }
    } while (m_nSkipCount > 0);

    m_nPlaybackLock--;

    return nNextIndex;
}

CMoveList::iterator CMoveList::AppendMoveRecord(OwnerPtr<CMoveRecord> pRec)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return end();

    pRec->SetSeqNum(m_nSeqNum);
    push_back(std::move(pRec));
    return --end();
}

CMoveList::iterator CMoveList::PrependMoveRecord(OwnerPtr<CMoveRecord> pRec,
    BOOL bSetSeqNum /* = TRUE */)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return end();

    if (bSetSeqNum)
        pRec->SetSeqNum(m_nSeqNum);
    push_front(std::move(pRec));
    return begin();
}

void CMoveList::PurgeAfter(size_t nIndex)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (nIndex <= m_nCompoundBaseIndex && m_pCompoundBaseBookMark != NULL)
    {
        if (m_pCompoundBaseBookMark)
        {
            m_pCompoundBaseBookMark = NULL;
        }
        m_bCompoundMove = FALSE;
        m_nCompoundBaseIndex = Invalid_v<size_t>;
    }

    const_iterator pos = FindIndex(nIndex);
    if (pos == end())
        return;             // Doesn't exist
    while (pos != end())
    {
        erase(pos++);
    }
}

void CMoveList::Clear()
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    m_pCompoundBaseBookMark = NULL;
    m_bCompoundMove = FALSE;
    m_nCompoundBaseIndex = Invalid_v<size_t>;

    m_nSeqNum = size_t(0);
    clear();
}

void CMoveList::BeginRecordingCompoundMove(CGamDoc& pDoc)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (m_bCompoundMove)
    {
        if (m_nCompoundBaseIndex == size() - size_t(1)) // Check if any moves recorded
            return;                             // Nope. Keep current marker record
        EndRecordingCompoundMove();             // Mark end of current block
    }
    ASSERT(!m_bCompoundMove);

    m_pCompoundBaseBookMark = new CGameState();
    if (!m_pCompoundBaseBookMark->SaveState(pDoc))
    {
        // Memory low warning?....
        m_pCompoundBaseBookMark = NULL;
        return;
    }

    AssignNewMoveGroup();
    m_nCompoundBaseIndex = size();
    AppendMoveRecord(MakeOwner<CCompoundMove>(TRUE));
    m_bCompoundMove = TRUE;
}

void CMoveList::CancelRecordingCompoundMove(CGamDoc& pDoc)
{
    ASSERT(m_nPlaybackLock == 0);
    if (m_nPlaybackLock != 0)
        return;

    if (!m_bCompoundMove)
        return;                                 // Nothing to get rid of

    pDoc.FlushAllIndicators();

    ASSERT(m_pCompoundBaseBookMark != NULL);
    pDoc.SetLoadingVersion(NumVersion(fileGsnVerMajor, fileGsnVerMinor));
    if (!m_pCompoundBaseBookMark->RestoreState(pDoc))
    {
        // Memory error message should be here
        m_pCompoundBaseBookMark = NULL;
        return;
    }
    m_pCompoundBaseBookMark = NULL;
    m_bCompoundMove = FALSE;
    PurgeAfter(m_nCompoundBaseIndex);
    m_nCompoundBaseIndex = Invalid_v<size_t>;
    pDoc.UpdateAllViews(NULL, HINT_GAMESTATEUSED);
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
    if (m_nCompoundBaseIndex < size() - size_t(1))  // Check if any moves recorded
    {
        AppendMoveRecord(MakeOwner<CCompoundMove>(FALSE));
        m_bCompoundMove = FALSE;
        m_nCompoundBaseIndex = Invalid_v<size_t>;

        m_pCompoundBaseBookMark = NULL;
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

        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<uint16_t>(m_nSeqNum);
        }
        else
        {
            CB::WriteCount(ar, m_nSeqNum);
        }
        ar << (WORD)m_bCompoundMove;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << (m_nCompoundBaseIndex != Invalid_v<size_t> ? value_preserving_cast<uint32_t>(m_nCompoundBaseIndex) : uint32_t(INT32_C(-1)));
        }
        else
        {
            CB::WriteCount(ar, m_nCompoundBaseIndex);
        }
        ar << (BYTE)(m_pCompoundBaseBookMark != NULL ? 1 : 0);
        if (m_pCompoundBaseBookMark)
            m_pCompoundBaseBookMark->Serialize(ar);

        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<WORD>(size());
        }
        else
        {
            CB::WriteCount(ar, size());
        }
        iterator pos;
        for (pos = begin(); pos != end(); )
        {
            const CMoveRecord& pRcd = GetNext(pos);
            ar << (short)pRcd.GetType();
            // store should not modify pRcd, so const_cast safe
            const_cast<CMoveRecord&>(pRcd).Serialize(ar);
        }
    }
    else
    {
        Clear();
        size_t wCount;
        uint16_t sTmp;

        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar >> sTmp; m_nSeqNum = sTmp;
        }
        else
        {
            m_nSeqNum = CB::ReadCount(ar);
        }

        if (CGamDoc::GetLoadingVersion() >= NumVersion(0, 60))
        {
            BYTE  cTmp;
            WORD  wTmp;
            uint32_t dwTmp;
            ar >> wTmp; m_bCompoundMove = (BOOL)wTmp;
            if (CB::GetVersion(ar) <= NumVersion(3, 90))
            {
                ar >> dwTmp; m_nCompoundBaseIndex = (dwTmp != uint32_t(INT32_C(-1)) ? value_preserving_cast<size_t>(dwTmp) : Invalid_v<size_t>);
            }
            else
            {
                m_nCompoundBaseIndex = CB::ReadCount(ar);
            }
            ar >> cTmp;             // Check for a bookmark
            if (cTmp)
            {
                ASSERT(m_pCompoundBaseBookMark == NULL);
                m_pCompoundBaseBookMark = MakeOwner<CGameState>();
                m_pCompoundBaseBookMark->Serialize(ar);
            }
        }

        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            WORD temp;
            ar >> temp;
            wCount = temp;
        }
        else
        {
            wCount = CB::ReadCount(ar);
        }
        for (size_t i = size_t(0) ; i < wCount ; ++i)
        {
            OwnerOrNullPtr<CMoveRecord> pRcd;
            short sType;
            ar >> sType;
            switch ((CMoveRecord::RcdType)sType)
            {
                case CMoveRecord::mrecState:
                    pRcd = MakeOwner<CGameStateRcd>();
                    break;
                case CMoveRecord::mrecPMove:
                    pRcd = MakeOwner<CBoardPieceMove>();
                    break;
                case CMoveRecord::mrecTMove:
                    pRcd = MakeOwner<CTrayPieceMove>();
                    break;
                case CMoveRecord::mrecPSide:
                    pRcd = MakeOwner<CPieceSetSide>();
                    break;
                case CMoveRecord::mrecPFacing:
                    pRcd = MakeOwner<CPieceSetFacing>();
                    break;
                case CMoveRecord::mrecMMove:
                    pRcd = MakeOwner<CBoardMarkerMove>();
                    break;
                case CMoveRecord::mrecMPlot:
                    pRcd = MakeOwner<CMovePlotList>();
                    break;
                case CMoveRecord::mrecMsg:
                    pRcd = MakeOwner<CMessageRcd>();
                    break;
                case CMoveRecord::mrecDelObj:
                    pRcd = MakeOwner<CObjectDelete>();
                    break;
                case CMoveRecord::mrecCompoundMove:
                    pRcd = MakeOwner<CCompoundMove>();
                    break;
                case CMoveRecord::mrecMFacing:
                    pRcd = MakeOwner<CMarkerSetFacing>();
                    break;
                case CMoveRecord::mrecSetObjText:
                    pRcd = MakeOwner<CObjectSetText>();
                    break;
                case CMoveRecord::mrecLockObj:
                    pRcd = MakeOwner<CObjectLockdown>();
                    break;
                case CMoveRecord::mrecEvtMsg:
                    pRcd = MakeOwner<CEventMessageRcd>();
                    break;
                case CMoveRecord::mrecPOwner:
                    pRcd = MakeOwner<CPieceSetOwnership>();
                    break;
                default:
                    ASSERT(FALSE);
                    AfxThrowArchiveException(CArchiveException::badClass);
            }
            pRcd->Serialize(ar);
            push_back(std::move(pRcd));
            BYTE cUndoFlag;
            if (CGamDoc::GetLoadingVersion() < NumVersion(2, 0))
                ar >> cUndoFlag;        // Eat UNDO flag info (never used)
        }

        /* provide visual feedback for only the first move in a
            block of moves involving the same hidden board(s) */
        /* need to examine moves in original context,
            so push current state */
        CGamDoc& doc = CheckedDeref((CGamDoc*)ar.m_pDocument);
        class PushGameState
        {
        public:
            PushGameState(CGamDoc& d) :
                doc(d),
                recordState(doc.GetGameState())
            {
                doc.SetGameState(CGamDoc::stateNotRecording);
                gs.SaveState(doc);
                ASSERT(!doc.IsQuietPlayback());
                doc.SetQuietPlayback(true);
            }
            ~PushGameState()
            {
                doc.SetQuietPlayback(false);
                gs.RestoreState(doc);
                doc.SetGameState(recordState);
            }
        private:
            CGamDoc& doc;
            const CGamDoc::GameState recordState;
            CGameState gs;
        } pushGameState(doc);

        // every move should start with its base game state
        if (empty() || front()->GetType() != CMoveRecord::mrecState)
        {
            AfxThrowArchiveException(CArchiveException::badSchema);
        }

        CMoveRecord::BoardSet usedPrivateBoards;
        for (iterator it = begin() ; it != end() ; ++it)
        {
            (*it)->SerializeHiddenByPrivate(doc,
                                            usedPrivateBoards);
            /* actually apply moves to make sure future
                move records find objects in their proper
                locations */
            switch ((*it)->GetType())
            {
                // doesn't support DoMove
                case CMoveRecord::mrecCompoundMove:
                    break;
                /* DoMove doesn't respect quiet playback, and
                    won't affect object locations */
                case CMoveRecord::mrecMsg:
                    break;
                default:
                    (*it)->DoMove(doc, 0);
            }
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

void CMoveList::DumpToTextFile(const CGamDoc& pDoc, CFile& file)
{
    char szBfr[256];
    sprintf(szBfr, "Current Move Group: %zu\r\n", m_nSeqNum);
    file.Write(szBfr, lstrlen(szBfr));
    sprintf(szBfr, "Number of move records: %zu\r\n", size());
    file.Write(szBfr, lstrlen(szBfr));

    iterator pos;
    int nIndex = 0;
    for (pos = begin(); pos != end(); )
    {
        const CMoveRecord& pRcd = GetNext(pos);
        CMoveRecord::RcdType eType = pRcd.GetType();
        ASSERT(eType >= 0 && eType < CMoveRecord::mrecMax);
        sprintf(szBfr, "[Index=%04d; Seq=%04zd: %s]\r\n", nIndex, pRcd.GetSeqNum(),
            (LPCSTR)tblTypes[eType]);
        file.Write(szBfr, lstrlen(szBfr));
        pRcd.DumpToTextFile(pDoc, file);
        nIndex++;
    }
}
#endif

CMoveList::const_iterator CMoveList::FindIndex(size_t nIndex) const
{
    if (nIndex >= size())
    {
        ASSERT(!"out of bounds");
        return end();
    }
    const_iterator retval = begin();
    for (size_t i = size_t(0) ; i < nIndex ; ++i)
    {
        ++retval;
    }
    return retval;
}
