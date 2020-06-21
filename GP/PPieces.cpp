// PPieces.cpp
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

#include    <stdafx.h>
#include    "WinExt.h"
#include    "Gp.h"
#include    "GamDoc.h"
#include    "GMisc.h"
#include    "Trays.h"
#include    "DrawObj.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "CDib.h"
#include    "MapFace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////

CPieceTable::CPieceTable()
{
    m_pPieceTbl = NULL;
    m_nTblSize = 0;
    // ------ //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
    // ------ //
    m_pPMgr = NULL;
}

CPieceTable::~CPieceTable()
{
    Clear();
}

///////////////////////////////////////////////////////////////////////
// Loads a word array with piece ID's that aren't already marked as
// is in use and are part of a particular piece set.

void CPieceTable::LoadUnusedPieceList(CWordArray *pPTbl, int nPieceSet,
    BOOL bClear)
{
    ASSERT(m_pPMgr != NULL);
    CPieceSet *pPSet = m_pPMgr->GetPieceSet(nPieceSet);
    ASSERT(pPSet != NULL);
    LoadUnusedPieceList(pPTbl, pPSet, bClear);
}

void CPieceTable::LoadUnusedPieceList(CWordArray *pPTbl, CPieceSet* pPceSet,
    BOOL bClear)
{
    if (bClear) pPTbl->RemoveAll();
    CWordArray* pPidTbl = pPceSet->GetPieceIDTable();
    for (int i = 0; i < pPidTbl->GetSize(); i++)
    {
        PieceID pid = (PieceID)pPidTbl->GetAt(i);
        if (!GetPiece(pid)->IsUsed())
            pPTbl->Add(pid);
    }
}

///////////////////////////////////////////////////////////////////////
// Marks all piece ID's corresponding to the input array's entries
// as unused.

void CPieceTable::SetPieceListAsUnused(CWordArray *pPTbl)
{
    for (int i = 0; i < pPTbl->GetSize(); i++)
    {
        PieceID pid = (PieceID)pPTbl->GetAt(i);
        GetPiece(pid)->SetUnused();
    }
}

void CPieceTable::SetPieceListAsFrontUp(CWordArray *pPTbl)
{
    for (int i = 0; i < pPTbl->GetSize(); i++)
    {
        PieceID pid = (PieceID)pPTbl->GetAt(i);
        GetPiece(pid)->SetSide(0);          // Front is up
    }
}

///////////////////////////////////////////////////////////////////////
// Remove all pieces that are now undefined.  Called from document
// code during deserialize.

void CPieceTable::PurgeUndefinedPieceIDs()
{
    ASSERT(m_pPMgr != NULL);
    ASSERT(m_pDoc != NULL);
    CTrayManager* pYMgr = m_pDoc->GetTrayManager();
    ASSERT(pYMgr != NULL);
    CPBoardManager* pPBMgr = m_pDoc->GetPBoardManager();
    ASSERT(pPBMgr != NULL);

    int nPiecesDeleted = 0;

    for (UINT i = 0; i < m_nTblSize; i++)
    {
        Piece* pPce;
        PieceDef* pDef;
        GetPieceDefinitionPair((PieceID)i, pPce, pDef);
        if (pDef->IsEmpty())
        {
            if (pPce->IsUsed())
            {
                TRACE1("ERROR: Piece %d was defined but had no Tile Images. "
                       "Piece removed from game!\n", i);
                pYMgr->RemovePieceIDFromTraySets((PieceID)i);
                CDrawObj* pObj = pPBMgr->RemoveObjectID((DWORD)i);
                if (pObj != NULL) delete pObj;
                pPce->SetUnused();      // Render it gone!
                nPiecesDeleted++;
            }
        }
    }

    if (nPiecesDeleted > 0)
    {
        // Post a message that will be displayed after the game has
        // been fully loaded.
        CString strMsg;
        strMsg.LoadString(IDS_WARN_PIECESDELETED);
        CString* pStr = new CString;
        pStr->Format(strMsg, nPiecesDeleted);
        GetApp()->GetMainWnd()->PostMessage(WM_MESSAGEBOX,
            (WPARAM)WMB_PTR_CSTRING, (LPARAM)pStr);
    }
}

///////////////////////////////////////////////////////////////////////
// Creates an empty playing piece table that is the same size as the
// the GameBoxes' Piece table.

void CPieceTable::CreatePlayingPieceTable()
{
    ASSERT(m_pPMgr != NULL);
    Clear();
    UINT nPieces = m_pPMgr->GetPieceTableSize();
    ASSERT(nPieces < maxPieces);
    if (nPieces == 0)
        return;
    m_pPieceTbl = (Piece*)GlobalAllocPtr(GHND, nPieces * sizeof(Piece));
    if (m_pPieceTbl == NULL)
        AfxThrowMemoryException();
    m_nTblSize = nPieces;
    for (UINT i = 0; i < m_nTblSize; i++)
        GetPiece((PieceID)i)->SetUnused();
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::SetPieceFacing(PieceID pid, int nFacingDegCW)
{
    GetPiece(pid)->SetFacing(nFacingDegCW);
}

int  CPieceTable::GetPieceFacing(PieceID pid)
{
    return GetPiece(pid)->GetFacing();
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::FlipPieceOver(PieceID pid)
{
    ASSERT(Is2Sided(pid));
    GetPiece(pid)->InvertSide();
}

void CPieceTable::SetPieceUnused(PieceID pid)
{
    GetPiece(pid)->SetUnused();
}

BOOL CPieceTable::IsPieceUsed(PieceID pid)
{
    return GetPiece(pid)->IsUsed();
}

BOOL CPieceTable::IsFrontUp(PieceID pid)
{
    return GetPiece(pid)->IsFrontUp();
}

BOOL CPieceTable::Is2Sided(PieceID pid)
{
    Piece* pPce;
    PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);
    return pDef->Is2Sided();
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::ClearAllOwnership()
{
    for (UINT i = 0; i < m_nTblSize; i++)
        m_pPieceTbl[i].SetOwnerMask(0);
}

BOOL CPieceTable::IsPieceOwned(PieceID pid)
{
    return GetPiece(pid)->IsOwned();
}

BOOL CPieceTable::IsPieceOwnedBy(PieceID pid, DWORD dwOwnerMask)
{
    return GetPiece(pid)->IsOwnedBy(dwOwnerMask);
}

BOOL CPieceTable::IsOwnedButNotByCurrentPlayer(PieceID pid, CGamDoc* pDoc)
{
    return IsPieceOwned(pid) && !IsPieceOwnedBy(pid, pDoc->GetCurrentPlayerMask());
}

DWORD CPieceTable::GetOwnerMask(PieceID pid)
{
    return GetPiece(pid)->GetOwnerMask();
}

void CPieceTable::SetOwnerMask(PieceID pid, DWORD dwMask)
{
    GetPiece(pid)->SetOwnerMask(dwMask);
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::SetPiece(PieceID pid, int nSide, int nFacing)
{
    Piece* pPce = GetPiece(pid);
    pPce->SetSide(nSide);
    pPce->SetFacing(nFacing);
}

///////////////////////////////////////////////////////////////////////

CSize CPieceTable::GetPieceSize(PieceID pid, BOOL bWithFacing)
{
    ASSERT(m_pDoc != NULL);
    CTile tile;
    m_pDoc->GetTileManager()->GetTile(GetActiveTileID(pid, bWithFacing), &tile);
    return tile.GetSize();
}

CSize CPieceTable::GetStackedSize(CWordArray* pTbl, int xDelta, int yDelta,
    BOOL bWithFacing)
{
    CRect rctFull;
    rctFull.SetRectEmpty();

    CPoint pntCtr(0, 0);

    for (int i = 0; i < pTbl->GetSize(); i++)
    {
        CSize sz = GetPieceSize((PieceID)pTbl->GetAt(i), bWithFacing);
        // First create rect centered on zero
        CRect rct(CPoint(-sz.cx/2, -sz.cy/2), sz);
        // Offset by staggered point
        rct += pntCtr;
        // Combine with master rect.
        if (i == 0)
            rctFull = rct;
        else
            rctFull |= rct;
        // Move stagger point along
        pntCtr += CSize(xDelta, yDelta);
    }
    return rctFull.Size();
}

///////////////////////////////////////////////////////////////////////

TileID CPieceTable::GetFrontTileID(PieceID pid, BOOL bWithFacing)
{
    Piece* pPce;
    PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);

    TileID tidBase = pDef->m_tidFront;

    if (!bWithFacing || pPce->GetFacing() == 0)
        return tidBase;

    // Handle rotated pieces...
    return GetFacedTileID(pid, tidBase, pPce->GetFacing(), 0);
}

TileID CPieceTable::GetBackTileID(PieceID pid, BOOL bWithFacing)
{
    Piece* pPce;
    PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);

    TileID tidBase = pDef->m_tidFront;

    if (!bWithFacing || pPce->GetFacing() == 0)
        return tidBase;

    // Handle rotated pieces...
    return GetFacedTileID(pid, tidBase, pPce->GetFacing(), 0);
}

///////////////////////////////////////////////////////////////////////

BOOL CPieceTable::IsPieceInvisible(PieceID pid)
{
    TileID tid = GetActiveTileID(pid, FALSE);
    CTile tile;
    m_pDoc->GetTileManager()->GetTile(tid, &tile, smallScale);
    return tile.GetTransparent() == tile.GetSmallColor();
}

///////////////////////////////////////////////////////////////////////

TileID CPieceTable::GetActiveTileID(PieceID pid, BOOL bWithFacing)
{
    Piece* pPce;
    PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);
    TileID tidBase = pPce->IsFrontUp() ? pDef->GetFrontTID() : pDef->GetBackTID();

    if (!bWithFacing || pPce->GetFacing() == 0)
        return tidBase;

    // Handle rotated pieces...
    return GetFacedTileID(pid, tidBase,  pPce->GetFacing(), pPce->GetSide());
}

TileID CPieceTable::GetInactiveTileID(PieceID pid, BOOL bWithFacing)
{
    Piece* pPce;
    PieceDef* pDef;
    GetPieceDefinitionPair(pid, pPce, pDef);
    TileID tidBase = pPce->IsFrontUp() ? pDef->GetBackTID() : pDef->GetFrontTID();

    if (!bWithFacing || pPce->GetFacing() == 0)
        return tidBase;

    // Handle rotated pieces...
    return GetFacedTileID(pid, tidBase,  pPce->GetFacing(), pPce->GetSide());
}

///////////////////////////////////////////////////////////////////////

TileID CPieceTable::GetFacedTileID(PieceID pid, TileID tidBase, int nFacing, int nSide)
{
    // Handle rotated pieces...
    ElementState state = MakePieceState(pid, nFacing, nSide);
    CTileFacingMap* pMapFacing = m_pDoc->GetFacingMap();
    TileID tidFacing = pMapFacing->GetFacingTileID(state);
    if (tidFacing != nullTid)
        return tidFacing;
    tidFacing = pMapFacing->CreateFacingTileID(state, tidBase);
    return tidFacing;
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::Clear()
{
    if (m_pPieceTbl != NULL)
        GlobalFreePtr(m_pPieceTbl);
    m_pPieceTbl = NULL;
    m_nTblSize = 0;
}

///////////////////////////////////////////////////////////////////////

Piece* CPieceTable::GetPiece(PieceID pid)
{
    ASSERT(m_pPieceTbl != NULL);
    ASSERT((UINT)pid < m_nTblSize);
    return &m_pPieceTbl[pid];
}

PieceDef* CPieceTable::GetPieceDef(PieceID pid)
{
    ASSERT(m_pPMgr != NULL);
    return m_pPMgr->GetPiece(pid);
}

void CPieceTable::GetPieceDefinitionPair(PieceID pid, Piece*& pPce,
    PieceDef*& pDef)
{
    pPce = GetPiece(pid);
    ASSERT(pPce != NULL);

    ASSERT(m_pPMgr != NULL);
    pDef = m_pPMgr->GetPiece(pid);
    ASSERT(pDef != NULL);
}

///////////////////////////////////////////////////////////////////////

CPieceTable* CPieceTable::Clone(CGamDoc *pDoc)
{
    CPieceTable* pTbl = new CPieceTable;
    pTbl->m_nTblSize = m_nTblSize;
    pTbl->m_pPieceTbl = (Piece*)GlobalAllocPtr(GHND,
        (long)m_nTblSize * sizeof(Piece));
    if (pTbl->m_pPieceTbl == NULL)
        AfxThrowMemoryException();
    for (UINT i = 0; i < m_nTblSize; i++)
        pTbl->m_pPieceTbl[i] = m_pPieceTbl[i];
    return pTbl;
}

void CPieceTable::Restore(CGamDoc *pDoc, CPieceTable* pTbl)
{
    Clear();
    m_nTblSize = pTbl->m_nTblSize;
    m_pPieceTbl = (Piece*)GlobalAllocPtr(GHND,
        (long)m_nTblSize * sizeof(Piece));
    if (m_pPieceTbl == NULL)
        AfxThrowMemoryException();
    for (UINT i = 0; i < m_nTblSize; i++)
        m_pPieceTbl[i] = pTbl->m_pPieceTbl[i];
}

BOOL CPieceTable::Compare(CPieceTable* pTbl)
{
    if (m_nTblSize != pTbl->m_nTblSize)
        return FALSE;
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        if (m_pPieceTbl[i] != pTbl->m_pPieceTbl[i])
            return FALSE;
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////////

void CPieceTable::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;

        ar << (WORD)m_nTblSize;

        for (UINT i = 0; i < m_nTblSize; i++)
            m_pPieceTbl[i].Serialize(ar);
    }
    else
    {
        Clear();

        m_pDoc = (CGamDoc*)ar.m_pDocument;
        m_pPMgr = m_pDoc->GetPieceManager();

        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;

        WORD wTmp;
        ar >> wTmp;
        if (wTmp > 0)
        {
            m_pPieceTbl = (Piece *)GlobalAllocPtr(GHND,
                (long)wTmp * sizeof(Piece));
            if (m_pPieceTbl == NULL)
                AfxThrowMemoryException();
            for (UINT i = 0; i < (UINT)wTmp; i++)
                m_pPieceTbl[i].Serialize(ar);
        }
        m_nTblSize = (UINT)wTmp;

        // Check for consistancy with game box piece table.

        ASSERT(m_pPMgr != NULL);
        UINT nDefSize = m_pPMgr->GetPieceTableSize();
        if (m_nTblSize < nDefSize)
        {
            // Need to increase the size of the playing piece table.
            if (m_nTblSize == 0)
            {
                m_pPieceTbl = (Piece *)GlobalAllocPtr(GHND,
                    (long)nDefSize * sizeof(Piece));
            }
            else
            {
                m_pPieceTbl = (Piece *)GlobalReAllocPtr(m_pPieceTbl,
                    (long)nDefSize * sizeof(Piece), GHND);
            }
            if (m_pPieceTbl == NULL)
                AfxThrowMemoryException();
            UINT nPrevSize = m_nTblSize;
            m_nTblSize = nDefSize;
            for (UINT i = nPrevSize; i < nDefSize; i++)
                GetPiece((PieceID)i)->SetUnused();
        }
        else if (m_nTblSize > nDefSize)
        {
            // Piece table in Game box was truncated. Probably
            // bad news.
            if (AfxMessageBox(IDS_ERR_PIECETBLSIZE, MB_OKCANCEL |
                    MB_ICONEXCLAMATION) != IDOK)
                AfxThrowArchiveException(CArchiveException::genericException);
            // Need to decrease the size of the playing piece table.
            m_pPieceTbl = (Piece*)GlobalReAllocPtr(m_pPieceTbl,
                (long)nDefSize * sizeof(Piece), GHND);
            if (m_pPieceTbl == NULL)
                AfxThrowMemoryException();
            UINT nOldTblSize = m_nTblSize;
            m_nTblSize = nDefSize;
            // Purge pieces in use that don't exist anymore
            CTrayManager* pYMgr = m_pDoc->GetTrayManager();
            CPBoardManager* pPBMgr = m_pDoc->GetPBoardManager();
            for (UINT i = m_nTblSize; i < nOldTblSize; i++)
            {
                pYMgr->RemovePieceIDFromTraySets((PieceID)i);
                CDrawObj* pObj = pPBMgr->RemoveObjectID((DWORD)i);
                if (pObj != NULL) delete pObj;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////

void Piece::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_nSide;
        ar << m_nFacing;
        ar << m_dwOwnerMask;
    }
    else
    {
        ar >> m_nSide;
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   // (support degrees)
        {
            // Convert pre 2.90 single byte (5 degree resolution) facings to degrees
            BYTE chVal;
            ar >> chVal;
            m_nFacing = chVal;
            m_nFacing *= 5;             // Convert to new degree format
        }
        else
            ar >> m_nFacing;
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 0))
            m_dwOwnerMask = 0;
        else if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10))
        {
            WORD wTmp;
            ar >> wTmp;
            m_dwOwnerMask = UPGRADE_OWNER_MASK(wTmp);
        }
        else
            ar >> m_dwOwnerMask;
    }
}

///////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CPieceTable::DumpToTextFile(CFile& file)
{
    static char szHead[] = "\r\nPiece Table\r\n-----------\r\n";
    file.Write(szHead, lstrlen(szHead));

    char szBfr[256];
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        Piece* pPce;
        PieceDef* pDef;
        GetPieceDefinitionPair((PieceID)i, pPce, pDef);
        wsprintf(szBfr, "PieceID %5.5d: m_nSide=%02X, m_nFacing=%3u, "
            "m_tidFront=%5u, m_tidBack=%5u\r\n", i,
            (UINT)pPce->m_nSide, (UINT)pPce->m_nFacing,
            (UINT)pDef->m_tidFront, (UINT)pDef->m_tidBack);
        file.Write(szBfr, lstrlen(szBfr));
    }
}
#endif

///////////////////////////////////////////////////////////////////////
// NOT INLINED SO WE CAN SET BREAK POINTS

void Piece::SetUnused()
{
    m_nSide = 0xFF;
    m_nFacing = 0;
}


