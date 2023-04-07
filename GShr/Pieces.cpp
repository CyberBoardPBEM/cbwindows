// Pieces.cpp
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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

namespace {
    struct PieceDef_v310
    {
        TileID  m_tidFront;
        TileID  m_tidBack;
        WORD    m_flags;

        // for serialize in
        PieceDef_v310() = default;

        explicit PieceDef_v310(const PieceDef& rhs)
        {
            const std::vector<TileID>& tids = rhs.GetTIDs();
            switch (tids.size())
            {
                case 0:
                    // this can happen due to minimum piece table size 32
                    m_tidFront = nullTid;
                    m_tidBack = nullTid;
                    break;
                case 1:
                    m_tidFront = tids[size_t(0)];
                    m_tidBack = nullTid;
                    break;
                case 2:
                    m_tidFront = tids[size_t(0)];
                    m_tidBack = tids[size_t(1)];
                    break;
                default:
                    AfxThrowArchiveException(CArchiveException::badSchema);
            }
            m_flags = rhs.m_flags;
        }
        explicit operator PieceDef() const
        {
            PieceDef retval;
            std::vector<TileID> tids;
            // ASSERT(m_tidBack != nullTid --> m_tidFront != nullTid);
            ASSERT(m_tidBack == nullTid || m_tidFront != nullTid);
            if (m_tidFront != nullTid || m_tidBack != nullTid)
            {
                tids.push_back(m_tidFront);
            }
            if (m_tidBack != nullTid)
            {
                tids.push_back(m_tidBack);
            }
            retval.SetTIDs(std::move(tids));
            retval.m_flags = m_flags;
            return retval;
        }

        void SetEmpty() { m_tidFront = m_tidBack = nullTid; m_flags = 0; }
        void Serialize(CArchive& ar)
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
    };
}

CPieceManager::CPieceManager(CTileManager& pTMgr) :
    m_pTMgr(pTMgr)
{
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
        const std::vector<TileID>& tids = pDef.GetTIDs();
        if (std::find(tids.begin(), tids.end(), tid) != tids.end())
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CPieceManager::PurgeMissingTileIDs(CGameElementStringMap* pMapStrings /* = NULL */)
{
    BOOL bPieceRemoved = FALSE;
    for (size_t i = 0; i < m_pPieceTbl.GetSize(); i++)
    {
        PieceDef& pDef = GetPiece(static_cast<PieceID>(i));
        const std::vector<TileID>& tids = pDef.GetTIDs();
        if (std::find_if(tids.begin(), tids.end(),
                        [this](TileID tid)
                        {
                            ASSERT(tid != nullTid);
                            return tid != nullTid && !m_pTMgr.IsTileIDValid(tid);
                        }) != tids.end())
        {
            DeletePiece(static_cast<PieceID>(i), pMapStrings, TRUE);
            bPieceRemoved = TRUE;
        }
    }
    return bPieceRemoved;
}

///////////////////////////////////////////////////////////////////////

PieceID CPieceManager::CreatePiece(size_t nPSet, std::vector<TileID>&& tids)
{
    ASSERT(nPSet < m_PSetTbl.size());
    PieceID pid = m_pPieceTbl.CreateIDEntry(&PieceDef::SetEmpty);

    PieceDef* pDef = &m_pPieceTbl[pid];
    pDef->SetTIDs(std::move(tids));
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
        pMapStrings->RemoveKey(MakePieceElement(pid, unsigned(0)));
    if (pMapStrings != NULL && !pMapStrings->IsEmpty())
        pMapStrings->RemoveKey(MakePieceElement(pid, unsigned(1)));
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

size_t CPieceManager::CreatePieceSet(CB::string pszName)
{
    m_PSetTbl.resize(m_PSetTbl.size() + size_t(1));
    m_PSetTbl.back().SetName(std::move(pszName));
    return m_PSetTbl.size() - size_t(1);
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

bool CPieceManager::Needs100SidePieces() const
{
    for (size_t i = size_t(0) ; i < m_pPieceTbl.GetSize() ; ++i)
    {
        if (m_pPieceTbl[static_cast<PieceID>(i)].GetSides() > size_t(2))
        {
            return true;
        }
    }
    return false;
}

void CPieceManager::Serialize(CArchive& ar)
{
    typedef XxxxIDTable<PieceID, PieceDef_v310,
                        pieceTblBaseSize, pieceTblIncrSize,
                        true,
                        CalcAllocSize> PieceIDTable_v310;

    if (ar.IsStoring())
    {
        if (Needs100SidePieces())
        {
            if (!GetCBFeatures().Check(ftrPiece100Sides))
            {
                AfxThrowArchiveException(CArchiveException::badSchema);
            }
            CB::AddFeature(ar, ftrPiece100Sides);
        }
        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;
        if (!CB::GetFeatures(ar).Check(ftrPiece100Sides))
        {
            PieceIDTable_v310 temp;
            temp.ResizeTable(m_pPieceTbl.GetSize(), &PieceDef_v310::SetEmpty);
            for (size_t i = size_t(0) ; i < temp.GetSize() ; ++i)
            {
                PieceID pid = static_cast<PieceID>(i);
                temp[pid] = static_cast<PieceDef_v310>(m_pPieceTbl[pid]);
            }
            ar << temp;
        }
        else
        {
            ar << m_pPieceTbl;
        }
    }
    else
    {
        Clear();
        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;
        if (!CB::GetFeatures(ar).Check(ftrPiece100Sides))
        {
            PieceIDTable_v310 temp;
            ar >> temp;
            m_pPieceTbl.ResizeTable(temp.GetSize(), &PieceDef::SetEmpty);
            for (size_t i = size_t(0) ; i < temp.GetSize() ; ++i)
            {
                PieceID pid = static_cast<PieceID>(i);
                m_pPieceTbl[pid] = static_cast<PieceDef>(temp[pid]);
            }
        }
        else
        {
            ar >> m_pPieceTbl;
        }
    }
    SerializePieceSets(ar);
}

void CPieceManager::SerializePieceSets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(GetNumPieceSets());
        }
        else
        {
            CB::WriteCount(ar, GetNumPieceSets());
        }
        for (size_t i = size_t(0); i < GetNumPieceSets(); i++)
            GetPieceSet(i).Serialize(ar);
    }
    else
    {
        size_t wSize;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            WORD temp;
            ar >> temp;
            wSize = temp;
        }
        else
        {
            wSize = CB::ReadCount(ar);
        }
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
        if (!CB::GetFeatures(ar).Check(ftrPiece100Sides))
        {
            ASSERT(!"dead code");
            PieceDef_v310 temp(*this);
            temp.Serialize(ar);
        }
        else
        {
            ar << m_tids;
            ar << m_flags;
        }
    }
    else
    {
        if (!CB::GetFeatures(ar).Check(ftrPiece100Sides))
        {
            ASSERT(!"dead code");
            PieceDef_v310 temp;
            temp.Serialize(ar);
            *this = static_cast<PieceDef>(temp);
        }
        else
        {
            ar >> m_tids;
            ar >> m_flags;
        }
    }
}

TileID PieceDef::GetFrontTID() const
{
    if (m_tids.size() < size_t(1))
    {
        AfxThrowMemoryException();
    }
    return m_tids[size_t(0)];
}

void PieceDef::SetFrontTID(TileID tid)
{
    ASSERT(tid != nullTid);
    if (m_tids.size() < size_t(1))
    {
        m_tids.resize(size_t(1));
    }
    m_tids[size_t(0)] = tid;
}

void PieceDef::SetTIDs(std::vector<TileID>&& tids)
{
    if (tids.size() > maxSides)
    {
        AfxThrowMemoryException();
    }
    m_tids = std::move(tids);
}

void PieceDef::SetSides(size_t sides)
{
    if (sides > maxSides)
    {
        AfxThrowMemoryException();
    }
    m_tids.resize(sides);
}


