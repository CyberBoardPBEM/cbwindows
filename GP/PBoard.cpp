// PBoard.cpp
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
#include    "Gp.h"
#include    "GamDoc.h"
#include    "GameBox.h"
#include    "GMisc.h"
#include    "Board.h"
#include    "GeoBoard.h"
#include    "PBoard.h"
#include    "PPieces.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

CPlayBoard::CPlayBoard(CGamDoc& doc) :
    m_pDoc(&doc)
{
    //  m_wReserved1 = 0;
    m_bGridRectCenters = FALSE;         // Replaced m_wReserved1
    //  m_wReserved2 = 0;
    m_bSnapMovePlot = FALSE;            // Replaced m_wReserved2
    // m_wReserved3 = 0;
    m_bOpenBoardOnLoad = FALSE;
    m_bShowSelListAndTinyMap = TRUE;
    // m_wReserved4 = 0;                // Replaced by m_bOpenBoardOnLoad
    // ------- //
    m_pPceList = MakeOwner<CDrawList>();
    m_pIndList = MakeOwner<CDrawList>();
    // ------- //
    m_bPlotMode = FALSE;
    m_ptPrevPlot = CPoint(-1, -1);
    // ------- //
    m_bGridSnap = FALSE;
    m_xGridSnap = 4000u;
    m_yGridSnap = 4000u;
    m_xGridSnapOff = 0u;
    m_yGridSnapOff = 0u;
    // ------- //
    m_bCellBorders = TRUE;
    m_bSmallCellBorders = FALSE;
    // ------- //
    m_xStackStagger = 3;
    m_yStackStagger = 3;
    // ------- //
    m_crPlotLineColor = RGB(0, 255, 0);
    m_nPlotLineWidth = 3;
    m_crLineColor = RGB(255, 0, 255);
    m_nLineWidth = 1;
    m_crTextColor = RGB(0, 0, 0);
    m_crTextBoxColor = RGB(255, 255, 255);
    m_fontID = CGameBox::GetFontManager().AddFont(
        TenthPointsToScreenPixels(100), taBold, uint8_t(FF_SWISS), "Arial");

    m_bLockedDrawnBeneath = TRUE;
    m_bPVisible = TRUE;
    m_bRotate180 = FALSE;
    m_bIVisible = TRUE;
    m_bIndOnTop = FALSE;
    m_bEnforceLocks = TRUE;

    m_bNonOwnerAccess = FALSE;                  // Owned stuff can't be messed with
    m_dwOwnerMask = 0;                          // No player owns it

    m_bPrivate = false;

    m_pBoard = NULL;                            // Loaded from Game Box
    m_nSerialNum = nullBid;                 // Initially set from game box.
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::Clear()
{
    m_pPceList = nullptr;
    m_pIndList = nullptr;
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::PropagateOwnerMaskToAllPieces()
{
    // Make sure that if this board is owned, all pieces
    // have their ownership masks set appropriately!
    if (IsOwned())
        m_pPceList->SetOwnerMasks(GetOwnerMask());
}

//////////////////////////////////////////////////////////////////////

BOOL CPlayBoard::IsOwnedButNotByCurrentPlayer(const CGamDoc& pDoc) const
{
    return IsOwned() && !IsOwnedBy(pDoc.GetCurrentPlayerMask());
}

//////////////////////////////////////////////////////////////////////
// Draw stuff on top of a board. The actual board image is draw by
// code outside of this object to easily support image caching.

void CPlayBoard::Draw(CDC& pDC, const CRect& pDrawRct, TileScale eScale)
{
    ASSERT(m_pBoard);
    ASSERT(m_pPceList);

    if (m_bIVisible && !m_bIndOnTop)
        m_pIndList->Draw(pDC, pDrawRct, eScale);

    m_pPceList->Draw(pDC, pDrawRct, eScale, TRUE, FALSE, !m_bPVisible, m_bLockedDrawnBeneath);

    if (m_bIVisible && m_bIndOnTop)
        m_pIndList->Draw(pDC, pDrawRct, eScale);
}

//////////////////////////////////////////////////////////////////////
// Piece is centered on point.

CPieceObj& CPlayBoard::AddPiece(CPoint pnt, PieceID pid)
{
    CTileManager& pTMgr = m_pDoc->GetTileManager();
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl);
    TileID tid = pPTbl->GetActiveTileID(pid);
    ASSERT(tid != nullTid);

    if (IsOwned())
        pPTbl->SetOwnerMask(pid, GetOwnerMask());// Force piece to be owned by this player

    CTile tile = pTMgr.GetTile(tid, fullScale);
    CRect rct(pnt, tile.GetSize());
    rct -= CPoint(tile.GetWidth() / 2, tile.GetHeight() / 2);
    LimitRectToBoard(rct);

    {
        OwnerPtr<CPieceObj> pObj = MakeOwner<CPieceObj>(&*m_pDoc);
        pObj->SetPiece(rct, pid);
        m_pPceList->AddToFront(std::move(pObj));
    }
    return static_cast<CPieceObj&>(m_pPceList->Front());
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::AddIndicatorObject(CDrawObj::OwnerPtr pObj)
{
    ASSERT(m_pIndList != NULL);
    m_pIndList->AddToFront(std::move(pObj));
}

void CPlayBoard::FlushAllIndicators()
{
    ASSERT(m_pIndList != NULL);
    m_pIndList->clear();
}

//////////////////////////////////////////////////////////////////////

const CPieceObj* CPlayBoard::FindPieceID(PieceID pid) const
{
    ASSERT(m_pPceList != NULL);
    return m_pPceList->FindPieceID(pid);
}

const CDrawObj* CPlayBoard::FindObjectID(ObjectID oid) const
{
    ASSERT(m_pPceList != NULL);
    return m_pPceList->FindObjectID(oid);
}

BOOL CPlayBoard::IsObjectOnBoard(const CDrawObj& pObj) const
{
    ASSERT(m_pPceList != NULL);
    return m_pPceList->Find(pObj) != m_pPceList->end();
}

//////////////////////////////////////////////////////////////////////
// Caller must delete the CPieceObj!

void CPlayBoard::RemoveObject(const CDrawObj& pObj)
{
    m_pPceList->RemoveObject(pObj);
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::LimitRectToBoard(CRect& rct) const
{
    if (rct.right > m_pBoard->GetWidth(fullScale))
        rct.OffsetRect(m_pBoard->GetWidth(fullScale) - rct.right, 0);
    if (rct.bottom > m_pBoard->GetHeight(fullScale))
        rct.OffsetRect(0, m_pBoard->GetHeight(fullScale) - rct.bottom);
    if (rct.left < 0)
        rct.OffsetRect(-rct.left, 0);
    if (rct.top < 0)
        rct.OffsetRect(0, -rct.top);
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::SetBoard(OwnerPtr<CGeomorphicBoard> pGeoBoard, BOOL bInheritSettings /* = FALSE */)
{
    ASSERT(!m_pGeoBoard);
    CB::propagate_const<std::unique_ptr<CGeomorphicBoard>> temp1 = CB::get_underlying(std::move(pGeoBoard));
    std::unique_ptr<CGeomorphicBoard> temp2 = CB::get_underlying(std::move(temp1));

    m_pGeoBoard = temp2.release();
    CBoard& pBrd = CreateGeoBoard();
    SetBoard(pBrd, bInheritSettings);
}

CBoard& CPlayBoard::CreateGeoBoard()
{
    ASSERT(m_pGeoBoard);
    OwnerPtr<CBoard> pBrd = m_pGeoBoard->CreateBoard();
    CBoard& retval = *pBrd;
    m_pDoc->GetBoardManager().Add(std::move(pBrd));
    return retval;
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::SetBoard(CBoard& pBoard, BOOL bInheritSettings /* = FALSE */)
{
    m_pBoard = &pBoard;
    m_nSerialNum = pBoard.GetSerialNumber();

    if (bInheritSettings)
    {
        m_bGridSnap = m_pBoard->m_bGridSnap;
        m_xGridSnap = m_pBoard->m_xGridSnap;
        m_yGridSnap = m_pBoard->m_yGridSnap;
        m_xGridSnapOff = m_pBoard->m_xGridSnapOff;
        m_yGridSnapOff = m_pBoard->m_yGridSnapOff;
        m_bCellBorders = m_pBoard->GetCellBorder();
    }
}

//////////////////////////////////////////////////////////////////////

OwnerPtr<CPlayBoard> CPlayBoard::Clone(CGamDoc& pDoc) const
{
    OwnerPtr<CPlayBoard> pBrd = MakeOwner<CPlayBoard>(pDoc);
    pBrd->m_pPceList = MakeOwner<CDrawList>(m_pPceList->Clone(&pDoc));
    pBrd->m_pIndList = MakeOwner<CDrawList>(m_pIndList->Clone(&pDoc));
    pBrd->m_bPlotMode = m_bPlotMode;
    pBrd->m_ptPrevPlot = m_ptPrevPlot;
    pBrd->m_nSerialNum = m_nSerialNum;
    pBrd->m_bEnforceLocks = m_bEnforceLocks;
    return pBrd;
}

void CPlayBoard::Restore(CGamDoc& pDoc, const CPlayBoard& pBrd)
{
    m_pPceList->Restore(&pDoc, *pBrd.m_pPceList);
    m_pIndList->Restore(&pDoc, *pBrd.m_pIndList);
    m_bPlotMode = pBrd.m_bPlotMode;
    m_ptPrevPlot = pBrd.m_ptPrevPlot;
    m_nSerialNum = pBrd.m_nSerialNum;
    m_bEnforceLocks = pBrd.m_bEnforceLocks;
}

bool CPlayBoard::Compare(const CPlayBoard& pBrd) const
{
    if (m_nSerialNum != pBrd.m_nSerialNum)
        return FALSE;
    if (m_pPceList == NULL || pBrd.m_pPceList == NULL)
        return FALSE;
    if (!m_pPceList->Compare(*pBrd.m_pPceList))
        return FALSE;
    if (m_pIndList == NULL || pBrd.m_pIndList == NULL)
        return FALSE;
    return m_pIndList->Compare(*pBrd.m_pIndList);
}

bool CPlayBoard::NeedsGeoRotate() const
{
    return m_pGeoBoard && m_pGeoBoard->NeedsGeoRotate();
}

void CPlayBoard::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (BYTE)(m_pGeoBoard ? 1 : 0);
        if (m_pGeoBoard)
            m_pGeoBoard->Serialize(ar);

        ar << m_nSerialNum;

        ar << static_cast<uint16_t>(m_bGridSnap);
        ar << m_xGridSnap;
        ar << m_yGridSnap;
        ar << m_xGridSnapOff;
        ar << m_yGridSnapOff;

        ar << (WORD)m_xStackStagger;
        ar << (WORD)m_yStackStagger;
        ar << (WORD)m_bPVisible;
        ar << (WORD)m_bLockedDrawnBeneath;
        ar << (WORD)m_bRotate180;
        ar << (WORD)m_bShowSelListAndTinyMap;
        ar << (WORD)m_bIVisible;
        ar << (WORD)m_bCellBorders;
        ar << (WORD)m_bSmallCellBorders;
        ar << (WORD)m_bEnforceLocks;

        ar << (DWORD)m_crPlotLineColor;
        ar << (WORD)m_nPlotLineWidth;
        ar << (DWORD)m_crLineColor;
        ar << (WORD)m_nLineWidth;
        ar << (DWORD)m_crTextColor;
        ar << (DWORD)m_crTextBoxColor;

        CFontTbl& pFontMgr = CGamDoc::GetFontManager();
        pFontMgr.Archive(ar, m_fontID);

        ar << (WORD)m_bGridRectCenters;
        ar << (WORD)m_bSnapMovePlot;
        ar << (WORD)m_bIndOnTop;
        ar << (WORD)m_bOpenBoardOnLoad;

        ar << (WORD)m_bPlotMode;

        ar << (short)m_ptPrevPlot.x;
        ar << (short)m_ptPrevPlot.y;

        ar << m_dwOwnerMask;
        ar << (WORD)m_bNonOwnerAccess;

        ASSERT(m_pPceList != NULL);
        m_pPceList->Serialize(ar);  // Board's piece and annotation list
        ASSERT(m_pIndList != NULL);
        m_pIndList->Serialize(ar);  // Board's indicator list

        if (!CB::GetFeatures(ar).Check(ftrPrivatePlayerBoard))
        {
            if (m_bPrivate)
            {
                AfxThrowArchiveException(CArchiveException::badSchema);
            }
        }
        else
        {
            ar << m_bPrivate;
        }
    }
    else
    {
        Clear();
        ASSERT(m_pDoc == (CGamDoc*)ar.m_pDocument);
        BYTE cTmp;
        uint16_t wTmp;
        uint32_t dwTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 01))  // Ver2.01
        {
            ar >> cTmp;
            if (cTmp != 0)
            {
                m_pGeoBoard = new CGeomorphicBoard(*m_pDoc);
                m_pGeoBoard->Serialize(ar);
                CreateGeoBoard();
            }
        }
        else
            m_pGeoBoard = nullptr;

        ar >> m_nSerialNum;

        CBoardManager& pBMgr = m_pDoc->GetBoardManager();
        size_t nBrdNum = pBMgr.FindBoardBySerial(m_nSerialNum);
        if (nBrdNum == Invalid_v<size_t>)
        {
            AfxMessageBox(IDS_ERR_BOARDMISSING, MB_OK | MB_ICONSTOP);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        CBoard& pBrd = pBMgr.GetBoard(nBrdNum);
        SetBoard(pBrd);

        ar >> wTmp; m_bGridSnap = (BOOL)wTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(0, 58))
        {
            ar >> m_xGridSnap;
            ar >> m_yGridSnap;
            ar >> m_xGridSnapOff;
            ar >> m_yGridSnapOff;
        }
        else
        {
            ar >> wTmp; m_xGridSnap = 1000 * static_cast<uint32_t>(wTmp);
            ar >> wTmp; m_yGridSnap = 1000 * static_cast<uint32_t>(wTmp);
            ar >> wTmp; m_xGridSnapOff = 1000 * static_cast<uint32_t>(wTmp);
            ar >> wTmp; m_yGridSnapOff = 1000 * static_cast<uint32_t>(wTmp);
        }

        ar >> wTmp; m_xStackStagger = (int)(short)wTmp;
        ar >> wTmp; m_yStackStagger = (int)(short)wTmp;
        ar >> wTmp; m_bPVisible = (BOOL)wTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))
        {
            ar >> wTmp; m_bLockedDrawnBeneath = (BOOL)wTmp; // Ver2.90
            ar >> wTmp; m_bRotate180 = (BOOL)wTmp;          // Ver2.90
        }
        else
        {
            // Default states when upgerading a file.
            m_bLockedDrawnBeneath = FALSE;                 // Ver2.90
            m_bRotate180 = FALSE;                          // Ver2.90
        }

        ar >> wTmp; m_bShowSelListAndTinyMap = (BOOL)wTmp;
        ar >> wTmp; m_bIVisible = (BOOL)wTmp;
        ar >> wTmp; m_bCellBorders = (BOOL)wTmp;
        ar >> wTmp; m_bSmallCellBorders = (BOOL)wTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
        {
            ar >> wTmp; m_bEnforceLocks = (BOOL)wTmp;
        }

        ar >> dwTmp; m_crPlotLineColor = (COLORREF)dwTmp;
        ar >> wTmp; m_nPlotLineWidth = (int)wTmp;
        ar >> dwTmp; m_crLineColor = (COLORREF)dwTmp;
        ar >> wTmp; m_nLineWidth = (int)wTmp;
        ar >> dwTmp; m_crTextColor = (COLORREF)dwTmp;
        ar >> dwTmp; m_crTextBoxColor = (COLORREF)dwTmp;

        CFontTbl& pFontMgr = CGamDoc::GetFontManager();
        m_fontID = 0;
        pFontMgr.Archive(ar, m_fontID);

        ar >> wTmp; m_bGridRectCenters = (BOOL)wTmp;
        ar >> wTmp; m_bSnapMovePlot = (BOOL)wTmp;
        ar >> wTmp; m_bIndOnTop = (BOOL)wTmp;
        ar >> wTmp; m_bOpenBoardOnLoad = (BOOL)wTmp;

        ar >> wTmp; m_bPlotMode = (BOOL)wTmp;
        short sTmp;
        ar >> sTmp; m_ptPrevPlot.x = sTmp;
        ar >> sTmp; m_ptPrevPlot.y = sTmp;


        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 0))
        {
            m_dwOwnerMask = 0;
            m_bNonOwnerAccess = FALSE;
        }
        else if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10))
        {
            ar >> wTmp;
            m_dwOwnerMask = UPGRADE_OWNER_MASK(wTmp);
            ar >> wTmp;
            m_bNonOwnerAccess = (BOOL)wTmp;
        }
        else
        {
            ar >> m_dwOwnerMask;
            ar >> wTmp;
            m_bNonOwnerAccess = (BOOL)wTmp;
        }

        ASSERT(m_pPceList == NULL);
        m_pPceList = MakeOwner<CDrawList>();
        m_pPceList->Serialize(ar);  // Board's piece and annotation list

        ASSERT(m_pIndList == NULL);
        m_pIndList = MakeOwner<CDrawList>();
        m_pIndList->Serialize(ar);  // Board's indicator list

        if (!CB::GetFeatures(ar).Check(ftrPrivatePlayerBoard))
        {
            m_bPrivate = false;
        }
        else
        {
            ar >> m_bPrivate;
        }
    }
}

CB::string CPlayBoard::GetCellNumberStr(CPoint pnt, TileScale eScale) const
{
    if (!m_pGeoBoard)
    {
        const CBoardArray& pba = GetBoard()->GetBoardArray();
        return pba.GetCellNumberStr(pnt, eScale);
    }
    else
    {
        return m_pGeoBoard->GetCellNumberStr(pnt, eScale);
    }
}

void CPlayBoard::DeleteGeoBoard::operator()(CGeomorphicBoard* p) const
{
    if (p)
    {
        // Delete the auto created geomorphic board.
        p->DeleteFromBoardManager();
        delete p;
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CPBoardManager::CPBoardManager(CGamDoc& doc) :
    m_pDoc(&doc)
{
    m_nNextGeoSerialNum = BoardID(GEO_BOARD_SERNUM_BASE);
    //m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

BoardID CPBoardManager::IssueGeoSerialNumber()
{
    size_t maxBoards = std::min(max_size(), value_preserving_cast<size_t>(std::numeric_limits<BoardID::UNDERLYING_TYPE>::max()));
    if (static_cast<BoardID::UNDERLYING_TYPE>(m_nNextGeoSerialNum) >= maxBoards)
    {
        AfxThrowMemoryException();
    }
    BoardID retval = m_nNextGeoSerialNum;
    m_nNextGeoSerialNum = static_cast<BoardID>(static_cast<BoardID::UNDERLYING_TYPE>(m_nNextGeoSerialNum) + BoardID::UNDERLYING_TYPE(1));
    return retval;
}

bool CPBoardManager::GetPBoardList(std::vector<BoardID>& tblBrds) const
{
    ASSERT(tblBrds.empty());
    if (IsEmpty())
        return false;

    tblBrds.reserve(tblBrds.size() + GetNumPBoards());
    for (size_t i = size_t(0); i < GetNumPBoards(); i++)
        tblBrds.push_back(GetPBoard(i).GetSerialNumber());

    return true;
}

// Find all existing play boards are not in the caller's list.
void CPBoardManager::FindPBoardsNotInList(const std::vector<BoardID>& tblBrdSerNum, std::vector<CB::not_null<CPlayBoard*>>& tblNotInList)
{
    ASSERT(tblNotInList.empty());
    for (size_t i = size_t(0); i < GetNumPBoards(); i++)
    {
        if (std::find(tblBrdSerNum.begin(), tblBrdSerNum.end(), GetPBoard(i).GetSerialNumber()) == tblBrdSerNum.end())
        {
            CPlayBoard& pPBrd = GetPBoard(i);
            tblNotInList.push_back(&pPBrd);
        }
    }
}

void CPBoardManager::SetPBoardList(const std::vector<BoardID>& tblBrds)
{
    ASSERT(tblBrds.size() >= 0);
    // First all existing play boards are checked to see if they
    // are in the new list. If they are not, they are removed. Any
    // playing pieces that are on the board(s) will need to be
    // reimported into the scenario.
    for (size_t i = 0; i < GetNumPBoards(); i++)
    {
        if (std::find(tblBrds.begin(), tblBrds.end(), GetPBoard(i).GetSerialNumber()) == tblBrds.end())
        {
            CPlayBoard &pPBrd = GetPBoard(i);

            CDrawList* pDwg = pPBrd.GetPieceList();
            std::vector<PieceID> tblPid;
            pDwg->GetPieceIDTable(tblPid);
            m_pDoc->GetPieceTable()->SetPieceListAsUnused(tblPid);

            erase(begin() + value_preserving_cast<ptrdiff_t>(i));
            i--;                // Recycle 'i' to look at next element
        }
    }
    // Ok... now add newly selected boards to end of list
    for (size_t i = 0; i < tblBrds.size(); i++)
    {
        BoardID nSerial = tblBrds[i];
        if (FindPBoardBySerial(nSerial) == Invalid_v<size_t>)
            AddBoard(nSerial, TRUE);
    }
}

void CPBoardManager::AddBoard(BoardID nSerialNum, BOOL bInheritSettings)
{
    CBoardManager& m_pBMgr = m_pDoc->GetBoardManager();
    size_t nBrd = m_pBMgr.FindBoardBySerial(nSerialNum);
    ASSERT(nBrd != Invalid_v<size_t>);
    CBoard& pBoard = m_pBMgr.GetBoard(nBrd);
    AddBoard(pBoard, bInheritSettings);
}

void CPBoardManager::AddBoard(CBoard& pBoard, BOOL bInheritSettings)
{
    push_back(new CPlayBoard(*m_pDoc));
    back()->SetBoard(pBoard, bInheritSettings);
}

void CPBoardManager::AddBoard(OwnerPtr<CGeomorphicBoard> pGeoBoard, BOOL bInheritSettings)
{
    push_back(new CPlayBoard(*m_pDoc));
    back()->SetBoard(std::move(pGeoBoard), bInheritSettings);
}

void CPBoardManager::DeletePBoard(size_t nBrd)
{
    CPlayBoard& pPBrd = GetPBoard(nBrd);

    CDrawList* pDwg = pPBrd.GetPieceList();
    std::vector<PieceID> tblPid;
    pDwg->GetPieceIDTable(tblPid);
    m_pDoc->GetPieceTable()->SetPieceListAsUnused(tblPid);

    erase(begin() + value_preserving_cast<ptrdiff_t>(nBrd));
}

const CPlayBoard* CPBoardManager::GetPBoardBySerial(BoardID nSerialNum) const
{
    size_t nBrdNum = FindPBoardBySerial(nSerialNum);
    if (nBrdNum == Invalid_v<size_t>)
        return NULL;
    return &GetPBoard(nBrdNum);
}

// Returns Invalid_v<size_t> if board doesn't exist.
size_t CPBoardManager::FindPBoardBySerial(BoardID nSerialNum) const
{
    for (size_t i = 0; i < GetNumPBoards(); i++)
    {
        if (GetPBoard(i).GetSerialNumber() == nSerialNum)
            return i;
    }
    return Invalid_v<size_t>;
}

void CPBoardManager::ClearAllOwnership()
{
    for (size_t i = 0; i < GetNumPBoards(); i++)
        GetPBoard(i).SetOwnerMask(0);
}

void CPBoardManager::PropagateOwnerMaskToAllPieces()
{
    for (size_t i = 0; i < GetNumPBoards(); i++)
        GetPBoard(i).PropagateOwnerMaskToAllPieces();
}

size_t CPBoardManager::FindPBoardByRef(const CPlayBoard& pPBrd) const
{
    for (size_t i = 0; i < GetNumPBoards(); i++)
    {
        if (&GetPBoard(i) == &pPBrd)
            return i;
    }
    return Invalid_v<size_t>;
}

OwnerPtr<CDrawObj> CPBoardManager::RemoveObjectID(ObjectID oid)
{
    CDrawObj* pObj;
    CPlayBoard* pPBrd = FindObjectOnBoard(oid, pObj);
    if (pPBrd != NULL)
        pPBrd->RemoveObject(*pObj);
    return pObj;
}

const CPlayBoard* CPBoardManager::FindObjectOnBoard(ObjectID oid, const CDrawObj*& ppObj) const
{
    ppObj = NULL;
    for (size_t i = 0; i < GetNumPBoards(); i++)
    {
        const CPlayBoard& pPBrd = GetPBoard(i);
        const CDrawObj* pObj = pPBrd.FindObjectID(oid);
        if (pObj != NULL)
        {
            ppObj = pObj;
            return &pPBrd;
        }
    }
    return NULL;
}

const CPlayBoard* CPBoardManager::FindPieceOnBoard(PieceID pid, const CPieceObj*& ppObj) const
{
    ppObj = NULL;
    for (size_t i = 0; i < GetNumPBoards(); i++)
    {
        const CPlayBoard& pPBrd = GetPBoard(i);
        const CPieceObj* pObj = pPBrd.FindPieceID(pid);
        if (pObj != NULL)
        {
            ppObj = pObj;
            return &pPBrd;
        }
    }
    return NULL;
}

const CPlayBoard* CPBoardManager::FindObjectOnBoard(const CDrawObj& pObj) const
{
    for (size_t i = 0; i < GetNumPBoards(); i++)
    {
        const CPlayBoard& pPBrd = GetPBoard(i);
        if (pPBrd.IsObjectOnBoard(pObj))
            return &pPBrd;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////

void CPBoardManager::DestroyAllElements()
{
    clear();
}

//////////////////////////////////////////////////////////////////////

OwnerPtr<CPBoardManager> CPBoardManager::Clone(CGamDoc& pDoc) const
{
    ASSERT(&pDoc == m_pDoc);
    OwnerPtr<CPBoardManager> pMgr = MakeOwner<CPBoardManager>(pDoc);
    pMgr->reserve(GetNumPBoards());
    for (size_t i = size_t(0); i < GetNumPBoards(); i++)
        pMgr->push_back(GetPBoard(i).Clone(pDoc));
    return pMgr;
}

void CPBoardManager::Restore(CGamDoc& pDoc, const CPBoardManager& pMgr)
{
    size_t nBrdLimit = CB::min(GetNumPBoards(), pMgr.GetNumPBoards());
    for (size_t i = 0; i < nBrdLimit; i++)
        GetPBoard(i).Restore(pDoc, pMgr.GetPBoard(i));
}

bool CPBoardManager::Compare(const CPBoardManager& pMgr) const
{
    if (GetNumPBoards() != pMgr.GetNumPBoards())
        return false;
    for (size_t i = 0; i < GetNumPBoards(); i++)
    {
        if (!GetPBoard(i).Compare(pMgr.GetPBoard(i)))
            return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////

void CPBoardManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // USED FOR m_nNextGeoSerialNum ar << m_wReserved1;     // Ver2.01
        ar << m_nNextGeoSerialNum;                        // Ver2.01
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;

        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(GetNumPBoards());
        }
        else
        {
            CB::WriteCount(ar, GetNumPBoards());
        }

        /* if any CPBoard needs Feature "private-player-board"
            or Feature "geo-rotate-unit", all must use that new
            feature */
        for (const OwnerPtr<CPlayBoard>& board : *this)
        {
            if (board->IsPrivate())
            {
                if (GetCBFeatures().Check(ftrPrivatePlayerBoard))
                {
                    CB::AddFeature(ar, ftrPrivatePlayerBoard);
                }
                // else will be checked by CPBoard::Serialize
            }
            if (board->NeedsGeoRotate())
            {
                if (GetCBFeatures().Check(ftrGeoRotateUnit))
                {
                    CB::AddFeature(ar, ftrGeoRotateUnit);
                }
                // else will be checked by CGeoBoardElement::Serialize
            }

            const Features& fs = CB::GetFeatures(ar);
            if (fs.Check(ftrPrivatePlayerBoard) &&
                fs.Check(ftrGeoRotateUnit))
            {
                break;
            }
        }

        for (size_t i = size_t(0); i < GetNumPBoards(); i++)
            GetPBoard(i).Serialize(ar);
    }
    else
    {
        size_t wTmp;
        DestroyAllElements();
        ASSERT(m_pDoc == ar.m_pDocument);

        // ar >> m_wReserved1;                                  // Ver2.01
        ar >> m_nNextGeoSerialNum;            // Ver2.01 (was m_wReserved1)
        if (m_nNextGeoSerialNum == BoardID(0))
            m_nNextGeoSerialNum = BoardID(GEO_BOARD_SERNUM_BASE);

        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;

        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            WORD tmp;
            ar >> tmp;
            wTmp = tmp;
        }
        else
        {
            wTmp = CB::ReadCount(ar);
        }
        for (size_t i = size_t(0); i < wTmp; i++)
        {
            push_back(new CPlayBoard(*m_pDoc));
            GetPBoard(i).Serialize(ar);
        }
    }
}

