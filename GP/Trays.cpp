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
    m_eTrayViz = trayVizTwoSide;
    m_bEnforceVizForOwnerToo = FALSE;
}

BOOL CTraySet::HasPieceID(PieceID pid)
{
    for (int i = 0; i < m_pidTbl.GetSize(); i++)
    {
        if ((PieceID)m_pidTbl.GetAt(i) == pid)
            return TRUE;
    }
    return FALSE;
}

int CTraySet::GetPieceIDIndex(PieceID pid)
{
    for (int i = 0; i < m_pidTbl.GetSize(); i++)
    {
        if ((PieceID)m_pidTbl.GetAt(i) == pid)
            return i;
    }
    return -1;
}

void CTraySet::RemovePieceID(PieceID pid)
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

void CTraySet::AddPieceID(PieceID pid, int nPos /* = -1 */)
{
    if (nPos < 0)
        m_pidTbl.Add((WORD)pid);
    else
    {
        if (nPos > m_pidTbl.GetSize())          // Prevent indexing of the list
            nPos = m_pidTbl.GetSize();
        m_pidTbl.InsertAt(nPos, (WORD)pid);
    }
}

void CTraySet::AddPieceList(CWordArray *pTbl, int nPos)
{
    for (int i = 0; i < pTbl->GetSize(); i++)
    {
        AddPieceID((PieceID)pTbl->GetAt(i), nPos);
        if (nPos > 0) nPos++;                   // Move insertion position
    }
}

void CTraySet::RemovePieceList(CWordArray *pTbl)
{
    for (int i = 0; i < pTbl->GetSize(); i++)
        RemovePieceID((PieceID)pTbl->GetAt(i));
}

CTraySet* CTraySet::Clone(CGamDoc *pDoc)
{
    CTraySet* pSet = new CTraySet;
    pSet->m_pidTbl.InsertAt(0, &m_pidTbl);
    // Don't need to save the name.
    return pSet;
}

void CTraySet::Restore(CGamDoc *pDoc, CTraySet* pSet)
{
    m_pidTbl.RemoveAll();
    m_pidTbl.InsertAt(0, &pSet->m_pidTbl);
}

BOOL CTraySet::Compare(CTraySet* pYGrp)
{
    if (m_pidTbl.GetSize() != pYGrp->m_pidTbl.GetSize())
        return FALSE;

    for (int i = 0; i < m_pidTbl.GetSize(); i++)
    {
        if (m_pidTbl.GetAt(i) != pYGrp->m_pidTbl.GetAt(i))
            return FALSE;
    }
    return TRUE;
}

BOOL CTraySet::IsOwnedButNotByCurrentPlayer(CGamDoc* pDoc)
{
    return IsOwned() && !IsOwnedBy(pDoc->GetCurrentPlayerMask());
}

void CTraySet::PropagateOwnerMaskToAllPieces(CGamDoc* pDoc)
{
    for (int i = 0; i < m_pidTbl.GetSize(); i++)
    {
        pDoc->GetPieceTable()->SetOwnerMask(
            (PieceID)m_pidTbl[i], GetOwnerMask());
    }
}

void CTraySet::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_strName;
        ar << (WORD)m_bRandomPull;
        WORD wTmp = (WORD)m_eTrayViz;
        if (m_bEnforceVizForOwnerToo)
            wTmp |= trayVizOwnEnforceFlag;   // Piggy back this option so don't rev file ver
        ar << wTmp;
        ar << m_dwOwnerMask;
        ar << (WORD)m_bNonOwnerAccess;

        m_pidTbl.Serialize(ar);
    }
    else
    {
        ar >> m_strName;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
        {
            WORD  wTmp;
            ar >> wTmp; m_bRandomPull = (BOOL)wTmp;
            // Special handle tray visibility rules
            ar >> wTmp;
            if (wTmp & trayVizOwnEnforceFlag)
            {
                m_bEnforceVizForOwnerToo = TRUE;
                wTmp &= ~(WORD)trayVizOwnEnforceFlag;
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
        m_pidTbl.Serialize(ar);
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

CTrayManager::~CTrayManager()
{
    Clear();
}

void CTrayManager::Clear()
{
    for (int i = 0; i < m_YSetTbl.GetSize(); i++)
        delete (CTraySet*)m_YSetTbl.GetAt(i);
}

int CTrayManager::CreateTraySet(const char* pszName)
{
    CTraySet* pYSet = new CTraySet;
    pYSet->SetName(pszName);
    m_YSetTbl.Add(pYSet);
    return m_YSetTbl.GetSize() - 1;
}

void CTrayManager::DeleteTraySet(int nYSet)
{
    CTraySet* pYSet = GetTraySet(nYSet);
    m_YSetTbl.RemoveAt(nYSet);
    delete pYSet;
}

CTraySet* CTrayManager::FindPieceIDInTraySet(PieceID pid)
{
    for (int i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet* pYSet = GetTraySet(i);
        if (pYSet->HasPieceID(pid))
            return pYSet;
    }
    return NULL;
}

int CTrayManager::FindTrayByName(const char* strName)
{
    for (int i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet* pYSet = GetTraySet(i);
        if (lstrcmp(pYSet->GetName(), strName) == 0)
            return i;
    }
    return -1;
}

void CTrayManager::ClearAllOwnership()
{
    for (int i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet* pYSet = GetTraySet(i);
        pYSet->SetOwnerMask(0);
    }
}

void CTrayManager::PropagateOwnerMaskToAllPieces(CGamDoc* pDoc)
{
    for (int i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet* pYSet = GetTraySet(i);
        pYSet->PropagateOwnerMaskToAllPieces(pDoc);
    }
}

int CTrayManager::FindTrayByPtr(CTraySet* pYSet)
{
    for (int i = 0; i < GetNumTraySets(); i++)
    {
        if (GetTraySet(i) == pYSet)
            return i;
    }
    return -1;
}

void CTrayManager::RemovePieceIDFromTraySets(PieceID pid)
{
    for (int i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet* pYSet = GetTraySet(i);
        if (pYSet->HasPieceID(pid))
        {
            pYSet->RemovePieceID(pid);
            return;
        }
    }
}

CTrayManager* CTrayManager::Clone(CGamDoc *pDoc)
{
    CTrayManager* pMgr = new CTrayManager;

    for (int i = 0; i < GetNumTraySets(); i++)
    {
        CTraySet* pSet = GetTraySet(i)->Clone(pDoc);
        pMgr->m_YSetTbl.Add(pSet);
    }
    return pMgr;
}

void CTrayManager::Restore(CGamDoc *pDoc, CTrayManager* pMgr)
{
    int nTrayLimit = min(GetNumTraySets(), pMgr->GetNumTraySets());
    for (int i = 0; i < nTrayLimit; i++)
        GetTraySet(i)->Restore(pDoc, pMgr->GetTraySet(i));
}

BOOL CTrayManager::Compare(CTrayManager* pYMgr)
{
    if (pYMgr->GetNumTraySets() != GetNumTraySets())
        return FALSE;

    for (int i = 0; i < GetNumTraySets(); i++)
    {
        if (!GetTraySet(i)->Compare(pYMgr->GetTraySet(i)))
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
        ar << (WORD)GetNumTraySets();
        for (int i = 0; i < GetNumTraySets(); i++)
            GetTraySet(i)->Serialize(ar);
    }
    else
    {
        WORD wSize;
        ar >> wSize;
        for (int i = 0; i < (int)wSize; i++)
        {
            CTraySet* pYSet = new CTraySet;
            pYSet->Serialize(ar);
            m_YSetTbl.Add(pYSet);
        }
    }
}


