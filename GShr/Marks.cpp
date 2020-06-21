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

const UINT tblBaseSize = 32;            // MarkDef table allocation strategy
const UINT tblIncrSize = 8;

///////////////////////////////////////////////////////////////////////

CMarkManager::CMarkManager()
{
    m_pMarkTbl = NULL;
    m_nTblSize = 0;
    m_pTMgr = NULL;
    // --------- //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

CMarkManager::~CMarkManager()
{
    if (m_pMarkTbl != NULL) GlobalFreePtr(m_pMarkTbl);
    for (int i = 0; i < m_MSetTbl.GetSize(); i++)
        delete (CMarkSet*)m_MSetTbl.GetAt(i);
}

void CMarkManager::Clear()
{
    if (m_pMarkTbl != NULL) GlobalFreePtr(m_pMarkTbl);
    m_pMarkTbl = NULL;
    m_nTblSize = 0;
    for (int i = 0; i < m_MSetTbl.GetSize(); i++)
        delete (CMarkSet*)m_MSetTbl.GetAt(i);
}

///////////////////////////////////////////////////////////////////////

MarkDef* CMarkManager::GetMark(MarkID mid)
{
    ASSERT(m_pMarkTbl != NULL);
    ASSERT(mid < m_nTblSize);
    return &m_pMarkTbl[mid];
}

///////////////////////////////////////////////////////////////////////

CSize CMarkManager::GetMarkSize(MarkID mid)
{
    MarkDef* pDef = GetMark(mid);
    CTile tile;
    m_pTMgr->GetTile(pDef->m_tid, &tile);
    return tile.GetSize();
}

///////////////////////////////////////////////////////////////////////

BOOL CMarkManager::IsTileInUse(TileID tid)
{
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        MarkDef* pDef = GetMark((MarkID)i);
        if (pDef->m_tid == tid)
            return TRUE;
    }
    return FALSE;
}

BOOL CMarkManager::PurgeMissingTileIDs(CGameElementStringMap* pMapString)
{
    BOOL bMarkRemoved = FALSE;
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        MarkDef* pDef = GetMark((MarkID)i);
        TileID tid = pDef->m_tid;
        if ((tid != nullTid && !m_pTMgr->IsTileIDValid(tid)))
        {
            DeleteMark((MarkID)i, pMapString, TRUE);
            bMarkRemoved = TRUE;
        }
    }
    return bMarkRemoved;
}

///////////////////////////////////////////////////////////////////////

MarkID CMarkManager::CreateMark(int nMSet, TileID tid, WORD wFlags /* = 0 */)
{
    ASSERT(nMSet < m_MSetTbl.GetSize());
    MarkID mid = CreateMarkIDEntry();

    MarkDef* pDef = &m_pMarkTbl[mid];
    pDef->m_tid = tid;
    pDef->m_flags = wFlags;
    GetMarkSet(nMSet)->AddMarkID(mid);
    return mid;
}

void CMarkManager::DeleteMark(MarkID mid, CGameElementStringMap* pMapString /* = NULL */,
    BOOL bFromSetAlso /* = TRUE */)
{
    ASSERT(m_pMarkTbl != NULL);
    ASSERT(mid < m_nTblSize);
    ASSERT(!m_pMarkTbl[mid].IsEmpty());
    MarkDef* pDef = &m_pMarkTbl[mid];

    if (bFromSetAlso)
        RemoveMarkIDFromMarkSets(mid);

    pDef->SetEmpty();

    if (pMapString != NULL && !pMapString->IsEmpty())
        pMapString->RemoveKey(MakeMarkerElement(mid));
}

BOOL CMarkManager::IsMarkIDValid(MarkID mid)
{
    for (int i = 0; i < GetNumMarkSets(); i++)
    {
        CMarkSet* pMSet = GetMarkSet(i);
        if (pMSet->HasMarkID(mid))
            return TRUE;
    }
    return FALSE;
}

int CMarkManager::FindMarkerSetFromPieceID(MarkID mid)
{
    for (int i = 0; i < GetNumMarkSets(); i++)
    {
        CMarkSet* pMSet = GetMarkSet(i);
        if (pMSet->HasMarkID(mid))
            return i;
    }
    return -1;
}

BOOL CMarkManager::IsMarkerInGroup(int nGroup, MarkID mid)
{
    int nGroupActual = FindMarkInMarkSet(mid);
    return nGroup == nGroupActual;
}

int CMarkManager::FindMarkInMarkSet(MarkID mid)
{
    for (int i = 0; i < GetNumMarkSets(); i++)
    {
        CMarkSet* pMSet = GetMarkSet(i);
        if (pMSet->HasMarkID(mid))
            return i;
    }
    return -1;
}

int CMarkManager::CreateMarkSet(const char* pszName)
{
    CMarkSet* pMSet = new CMarkSet;
    pMSet->SetName(pszName);
    m_MSetTbl.Add(pMSet);
    return m_MSetTbl.GetSize() - 1;
}

void CMarkManager::DeleteMarkSet(int nMSet, CGameElementStringMap* pMapString /* = NULL */)
{
    CMarkSet* pMSet = GetMarkSet(nMSet);
    CWordArray* pMids = pMSet->GetMarkIDTable();
    for (int i = 0; i < pMids->GetSize(); i++)
        DeleteMark((MarkID)pMids->GetAt(i), pMapString, FALSE);
    m_MSetTbl.RemoveAt(nMSet);
    delete pMSet;
}

///////////////////////////////////////////////////////////////////////

void CMarkManager::ResizeMarkTable(UINT nEntsNeeded)
{
    UINT nNewSize = CalcAllocSize(nEntsNeeded, tblBaseSize, tblIncrSize);
    if (m_pMarkTbl != NULL)
    {
        MarkDef * pNewTbl = (MarkDef *)GlobalReAllocPtr(
            m_pMarkTbl, (DWORD)nNewSize * sizeof(MarkDef), GHND);
        if (pNewTbl == NULL)
            AfxThrowMemoryException();
        m_pMarkTbl = pNewTbl;
    }
    else
    {
        m_pMarkTbl = (MarkDef *)GlobalAllocPtr(GHND,
            (DWORD)nNewSize * sizeof(MarkDef));
        if (m_pMarkTbl == NULL)
            AfxThrowMemoryException();
    }
    for (UINT i = m_nTblSize; i < nNewSize; i++)
        m_pMarkTbl[i].SetEmpty();
    m_nTblSize = nNewSize;
}

///////////////////////////////////////////////////////////////////////

MarkID CMarkManager::CreateMarkIDEntry()
{
    // Allocate from empty entry if possible
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        if (m_pMarkTbl[i].IsEmpty())
            return (MarkID)i;
    }
    // Get TileID from end of table.
    MarkID newMid = m_nTblSize;
    ResizeMarkTable(m_nTblSize + 1);
    return newMid;
}

void CMarkManager::RemoveMarkIDFromMarkSets(MarkID mid)
{
    for (int i = 0; i < GetNumMarkSets(); i++)
    {
        CMarkSet* pMSet = GetMarkSet(i);
        if (pMSet->HasMarkID(mid))
        {
            pMSet->RemoveMarkID(mid);
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
        ar << (WORD)m_nTblSize;
        for (UINT i = 0; i < m_nTblSize; i++)
            m_pMarkTbl[i].Serialize(ar);
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
            ResizeMarkTable((UINT)wTmp);
            for (UINT i = 0; i < (UINT)wTmp; i++)
                m_pMarkTbl[i].Serialize(ar);
        }
    }
#ifdef GPLAY
    if (CGameBox::GetLoadingVersion() > NumVersion(0, 53))
#else
    if (ar.IsStoring() ||
        CGamDoc::GetLoadingVersion() > NumVersion(0, 53))
#endif
        SerializeMarkSets(ar);
    else
        m_MSetTbl.Serialize(ar);
}

void CMarkManager::SerializeMarkSets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (WORD)GetNumMarkSets();
        for (int i = 0; i < GetNumMarkSets(); i++)
            GetMarkSet(i)->Serialize(ar);
    }
    else
    {
        WORD wSize;
        ar >> wSize;
        for (int i = 0; i < (int)wSize; i++)
        {
            CMarkSet* pMSet = new CMarkSet;
            pMSet->Serialize(ar);
            m_MSetTbl.Add(pMSet);
        }
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

BOOL CMarkSet::HasMarkID(MarkID mid)
{
    for (int i = 0; i < m_midTbl.GetSize(); i++)
    {
        if ((MarkID)m_midTbl.GetAt(i) == mid)
            return TRUE;
    }
    return FALSE;
}

void CMarkSet::RemoveMarkID(MarkID mid)
{
    for (int i = 0; i < m_midTbl.GetSize(); i++)
    {
        if ((MarkID)m_midTbl.GetAt(i) == mid)
        {
            m_midTbl.RemoveAt(i);
            return;
        }
    }
}

void CMarkSet::AddMarkID(MarkID mid, int nPos /* = -1 */)
{
    if (nPos < 0)
        m_midTbl.Add((WORD)mid);
    else
    {
        ASSERT(nPos <= m_midTbl.GetSize());
        m_midTbl.InsertAt(nPos, (WORD)mid);
    }
}

#ifdef GPLAY
// Entries are appended to the table.
void CMarkSet::GetRandomSelection(int nCount, CWordArray& tblIDs, CGamDoc* pDoc)
{
    UINT nRandSeed = pDoc->GetRandomNumberSeed();
    while (nCount--)
    {
        int nRandNum = CalcRandomNumberUsingSeed(0, GetMarkIDTable()->GetSize(),
            nRandSeed, &nRandSeed);
        tblIDs.Add(GetMarkIDTable()->GetAt(nRandNum));
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
        m_midTbl.Serialize(ar);
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
        m_midTbl.Serialize(ar);
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
        WORD wTmp;
        ar >> wTmp; m_tid = (TileID)wTmp;
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

