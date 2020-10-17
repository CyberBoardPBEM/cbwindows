// PBoard.cpp
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

CPlayBoard::CPlayBoard()
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
    m_pPceList = new CDrawList;
    m_pIndList = new CDrawList;
    // ------- //
    m_bPlotMode = FALSE;
    m_ptPrevPlot = CPoint(-1, -1);
    // ------- //
    m_bGridSnap = FALSE;
    m_xGridSnap = 4000;
    m_yGridSnap = 4000;
    m_xGridSnapOff = 0;
    m_yGridSnapOff = 0;
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
    m_fontID = CGameBox::GetFontManager()->AddFont(
        TenthPointsToScreenPixels(100), taBold, FF_SWISS, "Arial");

    m_bLockedDrawnBeneath = TRUE;
    m_bPVisible = TRUE;
    m_bRotate180 = FALSE;
    m_bIVisible = TRUE;
    m_bIndOnTop = FALSE;
    m_bEnforceLocks = TRUE;

    m_bNonOwnerAccess = FALSE;                  // Owned stuff can't be messed with
    m_dwOwnerMask = 0;                          // No player owns it

    m_pBoard = NULL;                            // Loaded from Game Box
    m_pDoc = NULL;                              // Set by document code.
    m_nSerialNum = -1;                          // Initially set from game box.

    m_pGeoBoard = NULL;                         // Normal board
}

CPlayBoard::~CPlayBoard()
{
    Clear();
    if (m_fontID != 0)
        CGamDoc::GetFontManager()->DeleteFont(m_fontID);
    if (m_pGeoBoard != NULL)
    {
        // Delete the auto created geomorphic board.
        CBoardManager* pBMgr = m_pDoc->GetBoardManager();
        if (pBMgr != NULL)
        {
            int nBrd = pBMgr->FindBoardBySerial(m_pGeoBoard->GetSerialNumber());
            pBMgr->DeleteBoard(nBrd);
        }
        delete m_pGeoBoard;
    }
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::Clear()
{
    if (m_pPceList) delete m_pPceList;
    m_pPceList = NULL;
    if (m_pIndList) delete m_pIndList;
    m_pIndList = NULL;
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::PropagateOwnerMaskToAllPieces(CGamDoc* pDoc)
{
    // Make sure that if this board is owned, all pieces
    // have their ownership masks set appropriately!
    if (IsOwned())
        m_pPceList->SetOwnerMasks(GetOwnerMask());
}

//////////////////////////////////////////////////////////////////////

BOOL CPlayBoard::IsOwnedButNotByCurrentPlayer(CGamDoc* pDoc)
{
    return IsOwned() && !IsOwnedBy(pDoc->GetCurrentPlayerMask());
}

//////////////////////////////////////////////////////////////////////
// Draw stuff on top of a board. The actual board image is draw by
// code outside of this object to easily support image caching.

void CPlayBoard::Draw(CDC* pDC, CRect* pDrawRct, TileScale eScale)
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

CPieceObj* CPlayBoard::AddPiece(CPoint pnt, PieceID pid)
{
    ASSERT(m_pDoc);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl);
    TileID tid = pPTbl->GetActiveTileID(pid);
    ASSERT(tid != nullTid);

    if (IsOwned())
        pPTbl->SetOwnerMask(pid, GetOwnerMask());// Force piece to be owned by this player

    CTile tile;
    pTMgr->GetTile(tid,  &tile, fullScale);
    CRect rct(pnt, tile.GetSize());
    rct -= CPoint(tile.GetWidth() / 2, tile.GetHeight() / 2);
    LimitRectToBoard(rct);

    CPieceObj* pObj = new CPieceObj(m_pDoc);
    pObj->SetPiece(rct, pid);
    m_pPceList->AddToFront(pObj);
    return pObj;
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::AddIndicatorObject(CDrawObj* pObj)
{
    ASSERT(m_pIndList != NULL);
    m_pIndList->AddToFront(pObj);
}

void CPlayBoard::FlushAllIndicators()
{
    ASSERT(m_pIndList != NULL);
    m_pIndList->Flush();
}

//////////////////////////////////////////////////////////////////////

CPieceObj* CPlayBoard::FindPieceID(PieceID pid)
{
    ASSERT(m_pPceList != NULL);
    return m_pPceList->FindPieceID(pid);
}

CDrawObj* CPlayBoard::FindObjectID(ObjectID oid)
{
    ASSERT(m_pPceList != NULL);
    return m_pPceList->FindObjectID(oid);
}

BOOL CPlayBoard::IsObjectOnBoard(CDrawObj *pObj)
{
    ASSERT(m_pPceList != NULL);
    return m_pPceList->Find(pObj) != NULL;
}

//////////////////////////////////////////////////////////////////////
// Caller must delete the CPieceObj!

void CPlayBoard::RemoveObject(CDrawObj* pObj)
{
    m_pPceList->RemoveObject(pObj);
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::LimitRectToBoard(CRect& rct)
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

void CPlayBoard::SetBoard(CGeomorphicBoard* pGeoBoard, BOOL bInheritSettings /* = FALSE */)
{
    ASSERT(m_pGeoBoard == NULL);
    m_pGeoBoard = new CGeomorphicBoard(pGeoBoard);
    CBoard* pBrd = CreateGeoBoard();
    SetBoard(pBrd, bInheritSettings);
}

CBoard* CPlayBoard::CreateGeoBoard()
{
    ASSERT(m_pGeoBoard != NULL);
    CBoard* pBrd = m_pGeoBoard->CreateBoard(m_pDoc);
    m_pDoc->GetBoardManager()->Add(pBrd);
    return pBrd;
}

//////////////////////////////////////////////////////////////////////

void CPlayBoard::SetBoard(CBoard* pBoard, BOOL bInheritSettings /* = FALSE */)
{
    ASSERT(pBoard != NULL);
    m_pBoard = pBoard;
    m_nSerialNum = pBoard->GetSerialNumber();

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

CPlayBoard* CPlayBoard::Clone(CGamDoc *pDoc)
{
    CPlayBoard* pBrd = new CPlayBoard;
    pBrd->Clear();          // Delete empty drawlist and unused font id.
    pBrd->m_pPceList = m_pPceList->Clone(pDoc);
    pBrd->m_pIndList = m_pIndList->Clone(pDoc);
    pBrd->m_bPlotMode = m_bPlotMode;
    pBrd->m_ptPrevPlot = m_ptPrevPlot;
    pBrd->m_nSerialNum = m_nSerialNum;
    pBrd->m_bEnforceLocks = m_bEnforceLocks;
    return pBrd;
}

void CPlayBoard::Restore(CGamDoc *pDoc, CPlayBoard* pBrd)
{
    m_pPceList->Restore(pDoc, pBrd->m_pPceList);
    m_pIndList->Restore(pDoc, pBrd->m_pIndList);
    m_bPlotMode = pBrd->m_bPlotMode;
    m_ptPrevPlot = pBrd->m_ptPrevPlot;
    m_nSerialNum = pBrd->m_nSerialNum;
    m_bEnforceLocks = pBrd->m_bEnforceLocks;
}

BOOL CPlayBoard::Compare(CPlayBoard* pBrd)
{
    if (m_nSerialNum != pBrd->m_nSerialNum)
        return FALSE;
    if (m_pPceList == NULL || pBrd->m_pPceList == NULL)
        return FALSE;
    if (!m_pPceList->Compare(pBrd->m_pPceList))
        return FALSE;
    if (m_pIndList == NULL || pBrd->m_pIndList == NULL)
        return FALSE;
    return m_pIndList->Compare(pBrd->m_pIndList);
}

void CPlayBoard::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (BYTE)(m_pGeoBoard != NULL ? 1 : 0);
        if (m_pGeoBoard != NULL)
            m_pGeoBoard->Serialize(ar);

        ar << (WORD)m_nSerialNum;

        ar << (WORD)m_bGridSnap;
        ar << (DWORD)m_xGridSnap;
        ar << (DWORD)m_yGridSnap;
        ar << (DWORD)m_xGridSnapOff;
        ar << (DWORD)m_yGridSnapOff;

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

        CFontTbl* pFontMgr = CGamDoc::GetFontManager();
        pFontMgr->Archive(ar, m_fontID);

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
    }
    else
    {
        Clear();
        m_pDoc = (CGamDoc*)ar.m_pDocument;
        BYTE cTmp;
        WORD wTmp;
        DWORD dwTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 01))  // Ver2.01
        {
            ar >> cTmp;
            if (cTmp != 0)
            {
                m_pGeoBoard = new CGeomorphicBoard;
                m_pGeoBoard->Serialize(ar);
                CreateGeoBoard();
            }
        }
        else
            m_pGeoBoard = NULL;

        ar >> wTmp; m_nSerialNum = (int)wTmp;

        CBoardManager* pBMgr = m_pDoc->GetBoardManager();
        ASSERT(pBMgr);
        int nBrdNum = pBMgr->FindBoardBySerial(m_nSerialNum);
        if (nBrdNum < 0)
        {
            AfxMessageBox(IDS_ERR_BOARDMISSING, MB_OK | MB_ICONSTOP);
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        CBoard* pBrd = pBMgr->GetBoard(nBrdNum);
        ASSERT(pBrd != NULL);
        SetBoard(pBrd);

        ar >> wTmp; m_bGridSnap = (BOOL)wTmp;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(0, 58))
        {
            ar >> dwTmp; m_xGridSnap = (int)dwTmp;
            ar >> dwTmp; m_yGridSnap = (int)dwTmp;
            ar >> dwTmp; m_xGridSnapOff = (int)dwTmp;
            ar >> dwTmp; m_yGridSnapOff = (int)dwTmp;
        }
        else
        {
            ar >> wTmp; m_xGridSnap = 1000 * (int)wTmp;
            ar >> wTmp; m_yGridSnap = 1000 * (int)wTmp;
            ar >> wTmp; m_xGridSnapOff = 1000 * (int)wTmp;
            ar >> wTmp; m_yGridSnapOff = 1000 * (int)wTmp;
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

        CFontTbl* pFontMgr = CGamDoc::GetFontManager();
        if (m_fontID != 0)
        {
            pFontMgr->DeleteFont(m_fontID);
            m_fontID = 0;
        }
        pFontMgr->Archive(ar, m_fontID);

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
        m_pPceList = new CDrawList;
        m_pPceList->Serialize(ar);  // Board's piece and annotation list

        ASSERT(m_pIndList == NULL);
        m_pIndList = new CDrawList;
        m_pIndList->Serialize(ar);  // Board's indicator list
    }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CPBoardManager::CPBoardManager()
{
    m_pBMgr = NULL;
    m_pDoc = NULL;
    m_nNextGeoSerialNum = GEO_BOARD_SERNUM_BASE;
    //m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

BOOL CPBoardManager::GetPBoardList(CWordArray& tblBrds)
{
    if (GetNumPBoards() == 0)
        return FALSE;

    for (int i = 0; i < GetSize(); i++)
        tblBrds.Add((WORD)GetPBoard(i)->GetSerialNumber());

    return TRUE;
}

// Find all existing play boards are not in the caller's list.
void CPBoardManager::FindPBoardsNotInList(CWordArray& tblBrdSerNum, CPtrArray& tblNotInList)
{
    for (int i = 0; i < GetSize(); i++)
    {
        if (NumInWordArray(tblBrdSerNum, GetPBoard(i)->GetSerialNumber()) == -1)
        {
            CPlayBoard *pPBrd = GetPBoard(i);
            tblNotInList.Add(pPBrd);
        }
    }
}

void CPBoardManager::SetPBoardList(CWordArray& tblBrds)
{
    ASSERT(m_pDoc != NULL);
    ASSERT(m_pBMgr != NULL);
    ASSERT(tblBrds.GetSize() >= 0);
    // First all existing play boards are checked to see if they
    // are in the new list. If they are not, they are removed. Any
    // playing pieces that are on the board(s) will need to be
    // reimported into the scenario.
    for (int i = 0; i < GetSize(); i++)
    {
        if (NumInWordArray(tblBrds, GetPBoard(i)->GetSerialNumber()) == -1)
        {
            CPlayBoard *pPBrd = GetPBoard(i);
            RemoveAt(i);

            CDrawList* pDwg = pPBrd->GetPieceList();
            CWordArray tblPid;
            pDwg->GetPieceIDTable(&tblPid);
            m_pDoc->GetPieceTable()->SetPieceListAsUnused(&tblPid);

            delete pPBrd;
            i--;                // Recycle 'i' to look at next element
        }
    }
    // Ok... now add newly selected boards to end of list
    for (int i = 0; i < tblBrds.GetSize(); i++)
    {
        int nSerial = (int)tblBrds[i];
        if (FindPBoardBySerial(nSerial) == -1)
            AddBoard(nSerial, TRUE);
    }
}

void CPBoardManager::AddBoard(int nSerialNum, BOOL bInheritSettings)
{
    int nBrd = m_pBMgr->FindBoardBySerial(nSerialNum);
    ASSERT(nBrd != -1);
    CBoard* pBoard = m_pBMgr->GetBoard(nBrd);
    AddBoard(pBoard, bInheritSettings);
}

void CPBoardManager::AddBoard(CBoard* pBoard, BOOL bInheritSettings)
{
    CPlayBoard* pPBrd = new CPlayBoard;
    pPBrd->SetDocument(m_pDoc);
    pPBrd->SetBoard(pBoard, bInheritSettings);
    Add(pPBrd);
}

void CPBoardManager::AddBoard(CGeomorphicBoard* pGeoBoard, BOOL bInheritSettings)
{
    CPlayBoard* pPBrd = new CPlayBoard;
    pPBrd->SetDocument(m_pDoc);
    pPBrd->SetBoard(pGeoBoard, bInheritSettings);
    Add(pPBrd);
}

void CPBoardManager::DeletePBoard(int nBrd)
{
    CPlayBoard *pPBrd = GetPBoard(nBrd);
    RemoveAt(nBrd);

    CDrawList* pDwg = pPBrd->GetPieceList();
    CWordArray tblPid;
    pDwg->GetPieceIDTable(&tblPid);
    m_pDoc->GetPieceTable()->SetPieceListAsUnused(&tblPid);

    delete pPBrd;
}

CPlayBoard* CPBoardManager::GetPBoardBySerial(int nSerialNum)
{
    int nBrdNum = FindPBoardBySerial(nSerialNum);
    if (nBrdNum == -1)
        return NULL;
    return GetPBoard(nBrdNum);
}

// Returns -1 if board doesn't exist.
int CPBoardManager::FindPBoardBySerial(int nSerialNum)
{
    for (int i = 0; i < GetNumPBoards(); i++)
    {
        if (GetPBoard(i)->GetSerialNumber() == nSerialNum)
            return i;
    }
    return -1;
}

void CPBoardManager::ClearAllOwnership()
{
    for (int i = 0; i < GetNumPBoards(); i++)
        GetPBoard(i)->SetOwnerMask(0);
}

void CPBoardManager::PropagateOwnerMaskToAllPieces(CGamDoc* pDoc)
{
    for (int i = 0; i < GetNumPBoards(); i++)
        GetPBoard(i)->PropagateOwnerMaskToAllPieces(pDoc);
}

int CPBoardManager::FindPBoardByPtr(CPlayBoard *pPBrd)
{
    for (int i = 0; i < GetNumPBoards(); i++)
    {
        if (GetPBoard(i) == pPBrd)
            return i;
    }
    return -1;
}

CDrawObj* CPBoardManager::RemoveObjectID(ObjectID oid)
{
    CDrawObj* pObj;
    CPlayBoard* pPBrd = FindObjectOnBoard(oid, &pObj);
    if (pPBrd != NULL)
        pPBrd->RemoveObject(pObj);
    return pObj;
}

CPlayBoard* CPBoardManager::FindObjectOnBoard(ObjectID oid, CDrawObj** ppObj)
{
    if (ppObj != NULL) *ppObj = NULL;
    for (int i = 0; i < GetNumPBoards(); i++)
    {
        CPlayBoard *pPBrd = GetPBoard(i);
        CDrawObj* pObj = pPBrd->FindObjectID(oid);
        if (pObj != NULL)
        {
            if (ppObj != NULL) *ppObj = pObj;
            return pPBrd;
        }
    }
    return NULL;
}

CPlayBoard* CPBoardManager::FindPieceOnBoard(PieceID pid, CPieceObj** ppObj)
{
    if (ppObj != NULL) *ppObj = NULL;
    for (int i = 0; i < GetNumPBoards(); i++)
    {
        CPlayBoard *pPBrd = GetPBoard(i);
        CPieceObj* pObj = pPBrd->FindPieceID(pid);
        if (pObj != NULL)
        {
            if (ppObj != NULL) *ppObj = pObj;
            return pPBrd;
        }
    }
    return NULL;
}

CPlayBoard* CPBoardManager::FindObjectOnBoard(CDrawObj* pObj)
{
    ASSERT(pObj != NULL);
    for (int i = 0; i < GetNumPBoards(); i++)
    {
        CPlayBoard *pPBrd = GetPBoard(i);
        if (pPBrd->IsObjectOnBoard(pObj))
            return pPBrd;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////

void CPBoardManager::DestroyAllElements()
{
    for (int i = 0; i < GetSize(); i++)
        delete GetPBoard(i);
    RemoveAll();
}

//////////////////////////////////////////////////////////////////////

CPBoardManager* CPBoardManager::Clone(CGamDoc *pDoc)
{
    CPBoardManager* pMgr = new CPBoardManager;
    for (int i = 0; i < GetNumPBoards(); i++)
        pMgr->Add(GetPBoard(i)->Clone(pDoc));
    return pMgr;
}

void CPBoardManager::Restore(CGamDoc *pDoc, CPBoardManager* pMgr)
{
    int nBrdLimit = min(GetNumPBoards(), pMgr->GetNumPBoards());
    for (int i = 0; i < nBrdLimit; i++)
        GetPBoard(i)->Restore(pDoc, pMgr->GetPBoard(i));
}

BOOL CPBoardManager::Compare(CPBoardManager* pMgr)
{
    if (GetNumPBoards() != pMgr->GetNumPBoards())
        return FALSE;
    for (int i = 0; i < GetNumPBoards(); i++)
    {
        if (!GetPBoard(i)->Compare(pMgr->GetPBoard(i)))
            return FALSE;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////////////////

void CPBoardManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // USED FOR m_nNextGeoSerialNum ar << m_wReserved1;     // Ver2.01
        ar << (WORD)m_nNextGeoSerialNum;                        // Ver2.01
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;

        ar << (WORD)GetNumPBoards();
        for (int i = 0; i < GetNumPBoards(); i++)
            GetPBoard(i)->Serialize(ar);
    }
    else
    {
        WORD wTmp;
        DestroyAllElements();
        m_pDoc = (CGamDoc*)ar.m_pDocument;

        // ar >> m_wReserved1;                                  // Ver2.01
        ar >> wTmp; m_nNextGeoSerialNum = (int)wTmp;            // Ver2.01 (was m_wReserved1)
        if (m_nNextGeoSerialNum == 0)
            m_nNextGeoSerialNum = GEO_BOARD_SERNUM_BASE;

        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;

        ar >> wTmp;
        for (int i = 0; i < (int)wTmp; i++)
        {
            CPlayBoard *pPBoard = new CPlayBoard;
            pPBoard->SetDocument(m_pDoc);
            pPBoard->Serialize(ar);
            Add(pPBoard);
        }
    }
}

