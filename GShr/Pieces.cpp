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

CPieceManager::CPieceManager()
{
    m_pTMgr = NULL;
    // --------- //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

void CPieceManager::Clear()
{
    m_pPieceTbl.Clear();
    m_PSetTbl.clear();
}

///////////////////////////////////////////////////////////////////////

const PieceDef& CPieceManager::GetPiece(PieceID pid) const
{
    ASSERT(m_pPieceTbl != NULL);
    ASSERT(m_pPieceTbl.Valid(pid));
    return m_pPieceTbl[pid];
}

///////////////////////////////////////////////////////////////////////

BOOL CPieceManager::IsTileInUse(TileID tid) const
{
    for (size_t i = 0; i < m_pPieceTbl.GetSize(); i++)
    {
        const PieceDef& pDef = GetPiece(static_cast<PieceID>(i));
        if (pDef.m_tidFront == tid || pDef.m_tidBack == tid)
            return TRUE;
    }
    return FALSE;
}

BOOL CPieceManager::PurgeMissingTileIDs(CGameElementStringMap* pMapStrings /* = NULL */)
{
    BOOL bPieceRemoved = FALSE;
    for (size_t i = 0; i < m_pPieceTbl.GetSize(); i++)
    {
        PieceDef& pDef = GetPiece(static_cast<PieceID>(i));
        TileID tid1 = pDef.m_tidFront;
        TileID tid2 = pDef.m_tidBack;
        if ((tid1 != nullTid && !m_pTMgr->IsTileIDValid(tid1)) ||
            (tid2 != nullTid && !m_pTMgr->IsTileIDValid(tid2)))
        {
            DeletePiece(static_cast<PieceID>(i), pMapStrings, TRUE);
            bPieceRemoved = TRUE;
        }
    }
    return bPieceRemoved;
}

///////////////////////////////////////////////////////////////////////

PieceID CPieceManager::CreatePiece(size_t nPSet, TileID tidFront, TileID tidBack)
{
    ASSERT(nPSet < m_PSetTbl.size());
    PieceID pid = m_pPieceTbl.CreateIDEntry(&PieceDef::SetEmpty);

    PieceDef* pDef = &m_pPieceTbl[pid];
    pDef->m_tidFront = tidFront;
    pDef->m_tidBack = tidBack;
    GetPieceSet(nPSet).AddPieceID(pid);
    return pid;
}

void CPieceManager::DeletePiece(PieceID pid, CGameElementStringMap* pMapStrings /* = NULL */,
    BOOL bFromSetAlso /* = TRUE */)
{
    ASSERT(m_pPieceTbl != NULL);
    ASSERT(m_pPieceTbl.Valid(pid));
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

BOOL CPieceManager::IsPieceIDValid(PieceID pid) const
{
    for (size_t i = 0; i < GetNumPieceSets(); i++)
    {
        const CPieceSet& pPSet = GetPieceSet(i);
        if (pPSet.HasPieceID(pid))
            return TRUE;
    }
    return FALSE;
}

size_t CPieceManager::FindPieceSetFromPieceID(PieceID pid) const
{
    for (size_t i = 0; i < GetNumPieceSets(); i++)
    {
        const CPieceSet& pPSet = GetPieceSet(i);
        if (pPSet.HasPieceID(pid))
            return i;
    }
    return Invalid_v<size_t>;
}

size_t CPieceManager::CreatePieceSet(const char* pszName)
{
    m_PSetTbl.resize(m_PSetTbl.size() + 1);
    m_PSetTbl.back().SetName(pszName);
    return m_PSetTbl.size() - 1;
}

void CPieceManager::DeletePieceSet(size_t nPSet, CGameElementStringMap* pMapStrings /* = NULL */)
{
    CPieceSet& pPSet = GetPieceSet(nPSet);
    const std::vector<PieceID>& pPids = pPSet.GetPieceIDTable();
    for (size_t i = 0; i < pPids.size(); i++)
        DeletePiece(pPids.at(i), pMapStrings, FALSE);
    m_PSetTbl.erase(m_PSetTbl.begin() + value_preserving_cast<ptrdiff_t>(nPSet));
}

///////////////////////////////////////////////////////////////////////

void CPieceManager::RemovePieceIDFromPieceSets(PieceID pid)
{
    for (size_t i = 0; i < GetNumPieceSets(); i++)
    {
        CPieceSet& pPSet = GetPieceSet(i);
        if (pPSet.HasPieceID(pid))
        {
            pPSet.RemovePieceID(pid);
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
        ar << m_pPieceTbl;
    }
    else
    {
        Clear();
        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;
        ar >> m_pPieceTbl;
    }
    SerializePieceSets(ar);
}

void CPieceManager::SerializePieceSets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << value_preserving_cast<WORD>(GetNumPieceSets());
        for (size_t i = 0; i < GetNumPieceSets(); i++)
            GetPieceSet(i).Serialize(ar);
    }
    else
    {
        WORD wSize;
        ar >> wSize;
        m_PSetTbl.resize(wSize);
        for (size_t i = 0; i < m_PSetTbl.size(); i++)
        {
            m_PSetTbl[i].Serialize(ar);
        }
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

BOOL CPieceSet::HasPieceID(PieceID pid) const
{
    for (size_t i = 0; i < m_pidTbl.size(); i++)
    {
        if (m_pidTbl.at(i) == pid)
            return TRUE;
    }
    return FALSE;
}

void CPieceSet::RemovePieceID(PieceID pid)
{
    for (size_t i = 0; i < m_pidTbl.size(); i++)
    {
        if (m_pidTbl.at(i) == pid)
        {
            m_pidTbl.erase(m_pidTbl.begin() + value_preserving_cast<ptrdiff_t>(i));
            return;
        }
    }
}

void CPieceSet::AddPieceID(PieceID pid, size_t nPos /* = Invalid_v<size_t> */)
{
    if (nPos == Invalid_v<size_t>)
        m_pidTbl.push_back(pid);
    else
    {
        ASSERT(nPos <= m_pidTbl.size());
        m_pidTbl.insert(m_pidTbl.begin() + value_preserving_cast<ptrdiff_t>(nPos), pid);
    }
}

void CPieceSet::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_strName;
        ar << m_pidTbl;
    }
    else
    {
        ar >> m_strName;
        ar >> m_pidTbl;
    }
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
        ar >> m_tidFront;
        ar >> m_tidBack;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // V2.0
            ar >> m_flags;
        else
            m_flags = 0;
    }
}


