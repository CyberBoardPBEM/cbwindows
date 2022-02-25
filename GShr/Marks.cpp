// Marks.cpp
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
#ifdef      GPLAY
    #include    "Gp.h"
    #include    "GameBox.h"
    #include    "GamDoc.h"
#else
    #include    "Gm.h"
    #include    "GmDoc.h"
#endif
#include    "WinExt.h"
#include    "Marks.h"
#include    "GMisc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////

CMarkManager::CMarkManager()
{
    m_pTMgr = NULL;
    // --------- //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

void CMarkManager::Clear()
{
    m_pMarkTbl.Clear();
    m_MSetTbl.clear();
}

///////////////////////////////////////////////////////////////////////

MarkDef& CMarkManager::GetMark(MarkID mid)
{
    ASSERT(m_pMarkTbl != NULL);
    ASSERT(m_pMarkTbl.Valid(mid));
    return m_pMarkTbl[mid];
}

///////////////////////////////////////////////////////////////////////

CSize CMarkManager::GetMarkSize(MarkID mid)
{
    MarkDef& pDef = GetMark(mid);
    CTile tile = m_pTMgr->GetTile(pDef.m_tid);
    return tile.GetSize();
}

///////////////////////////////////////////////////////////////////////

BOOL CMarkManager::IsTileInUse(TileID tid)
{
    for (size_t i = 0; i < m_pMarkTbl.GetSize(); i++)
    {
        MarkDef& pDef = GetMark(static_cast<MarkID>(i));
        if (pDef.m_tid == tid)
            return TRUE;
    }
    return FALSE;
}

BOOL CMarkManager::PurgeMissingTileIDs(CGameElementStringMap* pMapString)
{
    BOOL bMarkRemoved = FALSE;
    for (size_t i = 0; i < m_pMarkTbl.GetSize(); i++)
    {
        MarkDef& pDef = GetMark(static_cast<MarkID>(i));
        TileID tid = pDef.m_tid;
        if ((tid != nullTid && !m_pTMgr->IsTileIDValid(tid)))
        {
            DeleteMark(static_cast<MarkID>(i), pMapString, TRUE);
            bMarkRemoved = TRUE;
        }
    }
    return bMarkRemoved;
}

///////////////////////////////////////////////////////////////////////

MarkID CMarkManager::CreateMark(size_t nMSet, TileID tid, WORD wFlags /* = 0 */)
{
    ASSERT(nMSet < m_MSetTbl.size());
    MarkID mid = m_pMarkTbl.CreateIDEntry(&MarkDef::SetEmpty);

    MarkDef& pDef = m_pMarkTbl[mid];
    pDef.m_tid = tid;
    pDef.m_flags = wFlags;
    GetMarkSet(nMSet).AddMarkID(mid);
    return mid;
}

void CMarkManager::DeleteMark(MarkID mid, CGameElementStringMap* pMapString /* = NULL */,
    BOOL bFromSetAlso /* = TRUE */)
{
    ASSERT(m_pMarkTbl != NULL);
    ASSERT(m_pMarkTbl.Valid(mid));
    ASSERT(!m_pMarkTbl[mid].IsEmpty());
    MarkDef* pDef = &m_pMarkTbl[mid];

    if (bFromSetAlso)
        RemoveMarkIDFromMarkSets(mid);

    pDef->SetEmpty();

    if (pMapString != NULL && !pMapString->IsEmpty())
        pMapString->RemoveKey(MakeMarkerElement(mid));
}

BOOL CMarkManager::IsMarkIDValid(MarkID mid) const
{
    for (size_t i = 0; i < GetNumMarkSets(); i++)
    {
        const CMarkSet& pMSet = GetMarkSet(i);
        if (pMSet.HasMarkID(mid))
            return TRUE;
    }
    return FALSE;
}

bool CMarkManager::IsMarkerInGroup(size_t nGroup, MarkID mid) const
{
    size_t nGroupActual = FindMarkInMarkSet(mid);
    return nGroup == nGroupActual;
}

size_t CMarkManager::FindMarkInMarkSet(MarkID mid) const
{
    for (size_t i = 0; i < GetNumMarkSets(); i++)
    {
        const CMarkSet& pMSet = GetMarkSet(i);
        if (pMSet.HasMarkID(mid))
            return i;
    }
    return Invalid_v<size_t>;
}

size_t CMarkManager::CreateMarkSet(const char* pszName)
{
    m_MSetTbl.resize(m_MSetTbl.size() + 1);
    m_MSetTbl.back().SetName(pszName);
    return m_MSetTbl.size() - 1;
}

void CMarkManager::DeleteMarkSet(size_t nMSet, CGameElementStringMap* pMapString /* = NULL */)
{
    CMarkSet& pMSet = GetMarkSet(nMSet);
    const std::vector<MarkID>& pMids = pMSet.GetMarkIDTable();
    for (size_t i = 0; i < pMids.size(); i++)
        DeleteMark(pMids.at(i), pMapString, FALSE);
    m_MSetTbl.erase(m_MSetTbl.begin() + value_preserving_cast<ptrdiff_t>(nMSet));
}

///////////////////////////////////////////////////////////////////////

void CMarkManager::RemoveMarkIDFromMarkSets(MarkID mid)
{
    for (size_t i = 0; i < GetNumMarkSets(); i++)
    {
        CMarkSet& pMSet = GetMarkSet(i);
        if (pMSet.HasMarkID(mid))
        {
            pMSet.RemoveMarkID(mid);
            return;
        }
    }
    ASSERT(FALSE);
}

void CMarkManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;
        ar << m_pMarkTbl;
    }
    else
    {
        Clear();
        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;
        ar >> m_pMarkTbl;
    }
#ifdef GPLAY
    if (CGameBox::GetLoadingVersion() > NumVersion(0, 53))
#else
    if (ar.IsStoring() ||
        CGamDoc::GetLoadingVersion() > NumVersion(0, 53))
#endif
        SerializeMarkSets(ar);
    else
    {
        /* N.B.:  https://docs.microsoft.com/en-us/cpp/mfc/reference/cptrarray-class?view=msvc-160
                    says CPtrArray can't be serialized */
        ASSERT(!"untested code");
        if (ar.IsStoring())
        {
            CPtrArray temp;
            for (size_t i = 0 ; i < m_MSetTbl.size() ; ++i)
            {
                temp.Add(&m_MSetTbl[i]);
            }
            temp.Serialize(ar);
        }
        else
        {
            CPtrArray temp;
            temp.Serialize(ar);
            m_MSetTbl.resize(value_preserving_cast<size_t>(temp.GetSize()));
            for (INT_PTR i = 0 ; i < temp.GetSize() ; ++i)
            {
                CMarkSet* pSet = static_cast<CMarkSet*>(temp[i]);
                m_MSetTbl[value_preserving_cast<size_t>(i)] = std::move(CheckedDeref(pSet));
                delete pSet;
            }
        }
    }
}

void CMarkManager::SerializeMarkSets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<WORD>(GetNumMarkSets());
        }
        else
        {
            CB::WriteCount(ar, GetNumMarkSets());
        }
        for (size_t i = size_t(0); i < GetNumMarkSets(); i++)
            GetMarkSet(i).Serialize(ar);
    }
    else
    {
        size_t wSize;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            WORD temp;
            ar >> temp;
            wSize = temp;
        }
        else
        {
            wSize = CB::ReadCount(ar);
        }
        m_MSetTbl.resize(wSize);
        for (size_t i = size_t(0); i < m_MSetTbl.size(); i++)
        {
            m_MSetTbl[i].Serialize(ar);
        }
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

BOOL CMarkSet::HasMarkID(MarkID mid) const
{
    for (size_t i = 0; i < m_midTbl.size(); i++)
    {
        if (m_midTbl.at(i) == mid)
            return TRUE;
    }
    return FALSE;
}

void CMarkSet::RemoveMarkID(MarkID mid)
{
    for (size_t i = 0; i < m_midTbl.size(); i++)
    {
        if (m_midTbl.at(i) == mid)
        {
            m_midTbl.erase(m_midTbl.begin() + value_preserving_cast<ptrdiff_t>(i));
            return;
        }
    }
}

void CMarkSet::AddMarkID(MarkID mid, size_t nPos /* = Invalid_v<size_t> */)
{
    if (nPos == Invalid_v<size_t>)
        m_midTbl.push_back(mid);
    else
    {
        ASSERT(nPos <= m_midTbl.size());
        m_midTbl.insert(m_midTbl.begin() + value_preserving_cast<ptrdiff_t>(nPos), mid);
    }
}

#ifdef GPLAY
// Entries are appended to the table.
void CMarkSet::GetRandomSelection(size_t nCount, std::vector<MarkID>& tblIDs, CGamDoc* pDoc)
{
    UINT nRandSeed = pDoc->GetRandomNumberSeed();
    tblIDs.reserve(tblIDs.size() + nCount);
    while (nCount--)
    {
        int nRandNum = CalcRandomNumberUsingSeed(0, value_preserving_cast<UINT>(GetMarkIDTable().size()),
            nRandSeed, &nRandSeed);
        tblIDs.push_back(GetMarkIDTable().at(value_preserving_cast<size_t>(nRandNum)));
    }
    pDoc->SetRandomNumberSeed(nRandSeed);
}
#endif

void CMarkSet::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_strName;
        ar << (WORD)m_eMarkViz;         // File Ver 2.0
        ar << m_midTbl;
    }
    else
    {
        ar >> m_strName;
#ifdef GPLAY
        if (CGameBox::GetLoadingVersion() >= NumVersion(2, 0))  // File Ver 2.0
#else
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // File Ver 2.0
#endif
        {
            WORD wTmp;
            ar >> wTmp;
            m_eMarkViz = (MarkerTrayViz)wTmp;
        }
        ar >> m_midTbl;
    }
}

///////////////////////////////////////////////////////////////////////

void MarkDef::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_tid;
        ar << m_flags;
    }
    else
    {
        ar >> m_tid;
#ifdef GPLAY
        if (CGameBox::GetLoadingVersion() >= NumVersion(2, 0)) // V2.0
#else
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))  // V2.0
#endif
            ar >> m_flags;
        else
            m_flags = 0;
    }
}

