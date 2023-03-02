// Trays.cpp
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
#include    "Gp.h"
#include    "GamDoc.h"
#include    "GMisc.h"
#include    "Trays.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////

CTraySet::CTraySet()
{
    m_dwOwnerMask = 0;
    m_bNonOwnerAccess = FALSE;
    m_bRandomPull = FALSE;
    m_bRandomSidePull = false;
    m_eTrayViz = trayVizAllSides;
    m_bEnforceVizForOwnerToo = FALSE;
}

BOOL CTraySet::HasPieceID(PieceID pid) const
{
    for (size_t i = 0; i < m_pidTbl.size(); i++)
    {
        if (m_pidTbl.at(i) == pid)
            return TRUE;
    }
    return FALSE;
}

size_t CTraySet::GetPieceIDIndex(PieceID pid) const
{
    for (size_t i = 0; i < m_pidTbl.size(); i++)
    {
        if (m_pidTbl.at(i) == pid)
            return i;
    }
    return Invalid_v<size_t>;
}

void CTraySet::RemovePieceID(PieceID pid)
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

void CTraySet::AddPieceID(PieceID pid, size_t nPos /* = Invalid_v<size_t> */)
{
    if (nPos == Invalid_v<size_t>)
        m_pidTbl.push_back(pid);
    else
    {
        if (nPos > m_pidTbl.size())          // Prevent indexing of the list
            nPos = m_pidTbl.size();
        m_pidTbl.insert(m_pidTbl.begin() + value_preserving_cast<ptrdiff_t>(nPos), pid);
    }
}

void CTraySet::AddPieceList(const std::vector<PieceID>& pTbl, size_t nPos)
{
    for (size_t i = 0; i < pTbl.size(); i++)
    {
        AddPieceID(pTbl.at(i), nPos);
        ASSERT(nPos == Invalid_v<size_t> || !"untested code");
        if (nPos != Invalid_v<size_t>) nPos++;                   // Move insertion position
    }
}

void CTraySet::RemovePieceList(const std::vector<PieceID>& pTbl)
{
    for (size_t i = 0; i < pTbl.size(); i++)
        RemovePieceID(pTbl.at(i));
}

CTraySet CTraySet::Clone(CGamDoc *pDoc) const
{
    CTraySet pSet;
    pSet.m_pidTbl = m_pidTbl;
    // Don't need to save the name.
    return pSet;
}

void CTraySet::Restore(CGamDoc *pDoc, const CTraySet& pSet)
{
    m_pidTbl = pSet.m_pidTbl;
}

BOOL CTraySet::Compare(const CTraySet& pYGrp) const
{
    if (m_pidTbl.size() != pYGrp.m_pidTbl.size())
        return FALSE;

    for (size_t i = 0; i < m_pidTbl.size(); i++)
    {
        if (m_pidTbl.at(i) != pYGrp.m_pidTbl.at(i))
            return FALSE;
    }
    return TRUE;
}

BOOL CTraySet::IsOwnedButNotByCurrentPlayer(const CGamDoc& pDoc) const
{
    return IsOwned() && !IsOwnedBy(pDoc.GetCurrentPlayerMask());
}

void CTraySet::PropagateOwnerMaskToAllPieces(CGamDoc* pDoc)
{
    for (size_t i = 0; i < m_pidTbl.size(); i++)
    {
        pDoc->GetPieceTable()->SetOwnerMask(
            m_pidTbl[i], GetOwnerMask());
    }
}

void CTraySet::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_strName;
        ar << static_cast<uint16_t>(m_bRandomPull);
        if (!CB::GetFeatures(ar).Check(ftrPiece100Sides))
        {
            if (m_bRandomSidePull)
            {
                AfxThrowArchiveException(CArchiveException::badSchema);
            }
        }
        else
        {
            ar << static_cast<uint16_t>(m_bRandomSidePull);
        }
        uint16_t wTmp = static_cast<uint16_t>(m_eTrayViz);
        if (m_bEnforceVizForOwnerToo)
            wTmp |= trayVizOwnEnforceFlag;   // Piggy back this option so don't rev file ver
        ar << wTmp;
        ar << m_dwOwnerMask;
        ar << static_cast<uint16_t>(m_bNonOwnerAccess);

        ar << m_pidTbl;
    }
    else
    {
        ar >> m_strName;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
        {
            uint16_t  wTmp;
            ar >> wTmp; m_bRandomPull = static_cast<BOOL>(wTmp);
            if (!CB::GetFeatures(ar).Check(ftrPiece100Sides))
            {
                m_bRandomSidePull = false;
            }
            else
            {
                ar >> wTmp; m_bRandomSidePull = static_cast<bool>(wTmp);
            }
            // Special handle tray visibility rules
            ar >> wTmp;
            if (wTmp & trayVizOwnEnforceFlag)
            {
                m_bEnforceVizForOwnerToo = TRUE;
                wTmp &= ~(uint16_t)trayVizOwnEnforceFlag;
            }
            else
                m_bEnforceVizForOwnerToo;
            m_eTrayViz = (TrayViz)wTmp;

            if (CGamDoc::GetLoadingVersion() < NumVersion(3, 10))
            {
                ar >> wTmp;
                m_dwOwnerMask = UPGRADE_OWNER_MASK(wTmp);
            }
            else
                ar >> m_dwOwnerMask;
            ar >> wTmp; m_bNonOwnerAccess = (BOOL)wTmp;
        }
        ar >> m_pidTbl;
    }
}

///////////////////////////////////////////////////////////////////////

CTrayManager::CTrayManager()
{
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
    m_pTMgr = NULL;
}

void CTrayManager::Clear()
{
    m_YSetTbl.clear();
}

size_t CTrayManager::CreateTraySet(const char* pszName)
{
    m_YSetTbl.resize(m_YSetTbl.size() + 1);
    m_YSetTbl.back().SetName(pszName);
    return m_YSetTbl.size() - 1;
}

void CTrayManager::DeleteTraySet(size_t nYSet)
{
    m_YSetTbl.erase(m_YSetTbl.begin() + value_preserving_cast<ptrdiff_t>(nYSet));
}

CTraySet* CTrayManager::FindPieceIDInTraySet(PieceID pid)
{
    for (size_t i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet& pYSet = GetTraySet(i);
        if (pYSet.HasPieceID(pid))
            return &pYSet;
    }
    return NULL;
}

size_t CTrayManager::FindTrayByName(const char* strName) const
{
    for (size_t i = 0; i < GetNumTraySets(); i++)
    {
        const CTraySet& pYSet = GetTraySet(i);
        if (lstrcmp(pYSet.GetName(), strName) == 0)
            return i;
    }
    return Invalid_v<size_t>;
}

void CTrayManager::ClearAllOwnership()
{
    for (size_t i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet& pYSet = GetTraySet(i);
        pYSet.SetOwnerMask(0);
    }
}

void CTrayManager::PropagateOwnerMaskToAllPieces(CGamDoc* pDoc)
{
    for (size_t i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet& pYSet = GetTraySet(i);
        pYSet.PropagateOwnerMaskToAllPieces(pDoc);
    }
}

size_t CTrayManager::FindTrayByRef(const CTraySet& pYSet) const
{
    for (size_t i = 0; i < GetNumTraySets(); i++)
    {
        if (&GetTraySet(i) == &pYSet)
            return i;
    }
    return Invalid_v<size_t>;
}

void CTrayManager::RemovePieceIDFromTraySets(PieceID pid)
{
    for (size_t i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet& pYSet = GetTraySet(i);
        if (pYSet.HasPieceID(pid))
        {
            pYSet.RemovePieceID(pid);
            return;
        }
    }
}

CTrayManager CTrayManager::Clone(CGamDoc *pDoc) const
{
    CTrayManager pMgr;

    pMgr.m_YSetTbl.reserve(GetNumTraySets());
    for (size_t i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet pSet = GetTraySet(i).Clone(pDoc);
        pMgr.m_YSetTbl.push_back(std::move(pSet));
    }
    return pMgr;
}

void CTrayManager::Restore(CGamDoc *pDoc, const CTrayManager& pMgr)
{
    size_t nTrayLimit = CB::min(GetNumTraySets(), pMgr.GetNumTraySets());
    for (size_t i = 0; i < nTrayLimit; i++)
        GetTraySet(i).Restore(pDoc, pMgr.GetTraySet(i));
}

BOOL CTrayManager::Compare(const CTrayManager& pYMgr) const
{
    if (pYMgr.GetNumTraySets() != GetNumTraySets())
        return FALSE;

    for (size_t i = 0; i < GetNumTraySets(); i++)
    {
        if (!GetTraySet(i).Compare(pYMgr.GetTraySet(i)))
            return FALSE;
    }
    return TRUE;
}

void CTrayManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;
    }
    else
    {
        Clear();
        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;
    }
    SerializeTraySets(ar);
}

void CTrayManager::SerializeTraySets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(GetNumTraySets());
        }
        else
        {
            CB::WriteCount(ar, GetNumTraySets());
        }
        for (size_t i = size_t(0); i < GetNumTraySets(); i++)
            GetTraySet(i).Serialize(ar);
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
        m_YSetTbl.reserve(wSize);
        for (size_t i = 0; i < wSize; i++)
        {
            CTraySet pYSet;
            pYSet.Serialize(ar);
            m_YSetTbl.push_back(std::move(pYSet));
        }
    }
}


