// Pieces.cpp
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
#include    "Pieces.h"
#include    "GMisc.h"
#include    "MapStrng.h"
#include    "versions.h"
#ifdef      GPLAY
#include    "GamDoc.h"
#else
#include    "GmDoc.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////

const UINT tblBaseSize = 32;            // PieceDef table allocation strategy
const UINT tblIncrSize = 8;

///////////////////////////////////////////////////////////////////////

CPieceManager::CPieceManager()
{
    m_pPieceTbl = NULL;
    m_nTblSize = 0;
    m_pTMgr = NULL;
    // --------- //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

CPieceManager::~CPieceManager()
{
    Clear();
}

void CPieceManager::Clear()
{
    if (m_pPieceTbl != NULL) GlobalFreePtr(m_pPieceTbl);
    m_pPieceTbl = NULL;
    m_nTblSize = 0;
    for (int i = 0; i < m_PSetTbl.GetSize(); i++)
        delete (CPieceSet*)m_PSetTbl.GetAt(i);
}

///////////////////////////////////////////////////////////////////////

PieceDef* CPieceManager::GetPiece(PieceID pid)
{
    ASSERT(m_pPieceTbl != NULL);
    ASSERT(pid < m_nTblSize);
    return &m_pPieceTbl[pid];
}

///////////////////////////////////////////////////////////////////////

BOOL CPieceManager::IsTileInUse(TileID tid)
{
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        PieceDef* pDef = GetPiece((PieceID)i);
        if (pDef->m_tidFront == tid || pDef->m_tidBack == tid)
            return TRUE;
    }
    return FALSE;
}

BOOL CPieceManager::PurgeMissingTileIDs(CGameElementStringMap* pMapStrings /* = NULL */)
{
    BOOL bPieceRemoved = FALSE;
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        PieceDef* pDef = GetPiece((PieceID)i);
        TileID tid1 = pDef->m_tidFront;
        TileID tid2 = pDef->m_tidBack;
        if ((tid1 != nullTid && !m_pTMgr->IsTileIDValid(tid1)) ||
            (tid2 != nullTid && !m_pTMgr->IsTileIDValid(tid2)))
        {
            DeletePiece((PieceID)i, pMapStrings, TRUE);
            bPieceRemoved = TRUE;
        }
    }
    return bPieceRemoved;
}

///////////////////////////////////////////////////////////////////////

PieceID CPieceManager::CreatePiece(int nPSet, TileID tidFront, TileID tidBack)
{
    ASSERT(nPSet < m_PSetTbl.GetSize());
    PieceID pid = CreatePieceIDEntry();

    PieceDef* pDef = &m_pPieceTbl[pid];
    pDef->m_tidFront = tidFront;
    pDef->m_tidBack = tidBack;
    GetPieceSet(nPSet)->AddPieceID(pid);
    return pid;
}

void CPieceManager::DeletePiece(PieceID pid, CGameElementStringMap* pMapStrings /* = NULL */,
    BOOL bFromSetAlso /* = TRUE */)
{
    ASSERT(m_pPieceTbl != NULL);
    ASSERT(pid < m_nTblSize);
    ASSERT(!m_pPieceTbl[pid].IsEmpty());
    PieceDef* pDef = &m_pPieceTbl[pid];

    if (bFromSetAlso)
        RemovePieceIDFromPieceSets(pid);

    pDef->SetEmpty();

    // Also delete any associated piece strings
    if (pMapStrings != NULL && !pMapStrings->IsEmpty())
        pMapStrings->RemoveKey(MakePieceElement(pid, 0));
    if (pMapStrings != NULL && !pMapStrings->IsEmpty())
        pMapStrings->RemoveKey(MakePieceElement(pid, 1));
}

BOOL CPieceManager::IsPieceIDValid(PieceID pid)
{
    for (int i = 0; i < GetNumPieceSets(); i++)
    {
        CPieceSet* pPSet = GetPieceSet(i);
        if (pPSet->HasPieceID(pid))
            return TRUE;
    }
    return FALSE;
}

int CPieceManager::FindPieceSetFromPieceID(PieceID pid)
{
    for (int i = 0; i < GetNumPieceSets(); i++)
    {
        CPieceSet* pPSet = GetPieceSet(i);
        if (pPSet->HasPieceID(pid))
            return i;
    }
    return -1;
}

int CPieceManager::CreatePieceSet(const char* pszName)
{
    CPieceSet* pPSet = new CPieceSet;
    pPSet->SetName(pszName);
    m_PSetTbl.Add(pPSet);
    return m_PSetTbl.GetSize() - 1;
}

void CPieceManager::DeletePieceSet(int nPSet, CGameElementStringMap* pMapStrings /* = NULL */)
{
    CPieceSet* pPSet = GetPieceSet(nPSet);
    CWordArray* pPids = pPSet->GetPieceIDTable();
    for (int i = 0; i < pPids->GetSize(); i++)
        DeletePiece((PieceID)pPids->GetAt(i), pMapStrings, FALSE);
    m_PSetTbl.RemoveAt(nPSet);
    delete pPSet;
}

///////////////////////////////////////////////////////////////////////

void CPieceManager::ResizePieceTable(UINT nEntsNeeded)
{
    UINT nNewSize = CalcAllocSize(nEntsNeeded, tblBaseSize, tblIncrSize);
    if (m_pPieceTbl != NULL)
    {
        PieceDef * pNewTbl = (PieceDef *)GlobalReAllocPtr(
            m_pPieceTbl, (DWORD)nNewSize * sizeof(PieceDef), GHND);
        if (pNewTbl == NULL)
            AfxThrowMemoryException();
        m_pPieceTbl = pNewTbl;
    }
    else
    {
        m_pPieceTbl = (PieceDef *)GlobalAllocPtr(GHND,
            (DWORD)nNewSize * sizeof(PieceDef));
        if (m_pPieceTbl == NULL)
            AfxThrowMemoryException();
    }
    for (UINT i = m_nTblSize; i < nNewSize; i++)
        m_pPieceTbl[i].SetEmpty();
    m_nTblSize = nNewSize;
}

///////////////////////////////////////////////////////////////////////

PieceID CPieceManager::CreatePieceIDEntry()
{
    // Allocate from empty entry if possible
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        if (m_pPieceTbl[i].IsEmpty())
            return (PieceID)i;
    }
    // Get TileID from end of table.
    PieceID newPid = m_nTblSize;
    ResizePieceTable(m_nTblSize + 1);
    return newPid;
}

void CPieceManager::RemovePieceIDFromPieceSets(PieceID pid)
{
    for (int i = 0; i < GetNumPieceSets(); i++)
    {
        CPieceSet* pPSet = GetPieceSet(i);
        if (pPSet->HasPieceID(pid))
        {
            pPSet->RemovePieceID(pid);
            return;
        }
    }
    ASSERT(FALSE);
}

void CPieceManager::Serialize(CArchive& ar)
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
        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;
        WORD wTmp;
        ar >> wTmp;
        if (wTmp > 0)
        {
            ResizePieceTable((UINT)wTmp);
            for (UINT i = 0; i < (UINT)wTmp; i++)
                m_pPieceTbl[i].Serialize(ar);
        }
    }
    SerializePieceSets(ar);
}

void CPieceManager::SerializePieceSets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (WORD)GetNumPieceSets();
        for (int i = 0; i < GetNumPieceSets(); i++)
            GetPieceSet(i)->Serialize(ar);
    }
    else
    {
        WORD wSize;
        ar >> wSize;
        for (int i = 0; i < (int)wSize; i++)
        {
            CPieceSet* pPSet = new CPieceSet;
            pPSet->Serialize(ar);
            m_PSetTbl.Add(pPSet);
        }
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

BOOL CPieceSet::HasPieceID(PieceID pid)
{
    for (int i = 0; i < m_pidTbl.GetSize(); i++)
    {
        if ((PieceID)m_pidTbl.GetAt(i) == pid)
            return TRUE;
    }
    return FALSE;
}

void CPieceSet::RemovePieceID(PieceID pid)
{
    for (int i = 0; i < m_pidTbl.GetSize(); i++)
    {
        if ((PieceID)m_pidTbl.GetAt(i) == pid)
        {
            m_pidTbl.RemoveAt(i);
            return;
        }
    }
}

void CPieceSet::AddPieceID(PieceID pid, int nPos /* = -1 */)
{
    if (nPos < 0)
        m_pidTbl.Add((WORD)pid);
    else
    {
        ASSERT(nPos <= m_pidTbl.GetSize());
        m_pidTbl.InsertAt(nPos, (WORD)pid);
    }
}

void CPieceSet::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
        ar << m_strName;
    else
        ar >> m_strName;
    m_pidTbl.Serialize(ar);
}

///////////////////////////////////////////////////////////////////////

void PieceDef::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_tidFront;
        ar << m_tidBack;
        ar << m_flags;
    }
    else
    {
        WORD wTmp;
        ar >> wTmp; m_tidFront = (TileID)wTmp;
        ar >> wTmp; m_tidBack = (TileID)wTmp;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // V2.0
            ar >> m_flags;
        else
            m_flags = 0;
    }
}


