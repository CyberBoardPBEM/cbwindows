// GamState.cpp
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
#include    "PBoard.h"
#include    "PPieces.h"
#include    "Trays.h"
#include    "GamState.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////

CGameState::CGameState()
{
    m_pDoc = NULL;
    m_pPBMgr = NULL;
    m_pPTbl = NULL;
}

CGameState::CGameState(CGamDoc* pDoc)
{
    m_pDoc = pDoc;
    m_pPBMgr = NULL;
    m_pPTbl = NULL;
}

BOOL CGameState::CompareState()
{
    ASSERT(m_pDoc != NULL);
    ASSERT(m_pPBMgr != NULL);
    ASSERT(m_pPTbl != NULL);
    // Compare the string tables

    // Compare the piece tables
    if (!m_pDoc->GetPieceTable()->Compare(*m_pPTbl))
        return FALSE;
    // Compare the play boards
    if (!m_pDoc->GetPBoardManager()->Compare(*m_pPBMgr))
        return FALSE;
    // Compare the trays
    if (!m_pDoc->GetTrayManager()->Compare(m_pYMgr))
        return FALSE;
    return TRUE;
}

BOOL CGameState::SaveState()
{
    ASSERT(m_pDoc != NULL);
    Clear();
    TRY
    {
        m_mapString.Clone(m_pDoc->GetGameStringMap());
        OwnerPtr<CPBoardManager> temp1 = m_pDoc->GetPBoardManager()->Clone(*m_pDoc);
        OwnerOrNullPtr<CPBoardManager> temp2 = CB::get_underlying(std::move(temp1));
        m_pPBMgr = CB::get_underlying(std::move(temp2)).release();
        m_pYMgr = m_pDoc->GetTrayManager()->Clone(m_pDoc);
        m_pPTbl = m_pDoc->GetPieceTable()->Clone(m_pDoc);
    }
    CATCH_ALL(e)
    {
        Clear();
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}

BOOL CGameState::RestoreState()
{
    ASSERT(m_pDoc != NULL);
    ASSERT(m_pPBMgr != NULL);
    ASSERT(m_pPTbl != NULL);

    TRY
    {
        m_pDoc->GetGameStringMap()->Clone(&m_mapString);
        m_pDoc->GetPBoardManager()->Restore(*m_pDoc, *m_pPBMgr);
        m_pDoc->GetTrayManager()->Restore(m_pDoc, m_pYMgr);
        m_pDoc->GetPieceTable()->Restore(m_pDoc, *m_pPTbl);
    }
    CATCH_ALL(e)
    {
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}

void CGameState::Clear()
{
    m_mapString.RemoveAll();

    m_pPBMgr = NULL;
    m_pYMgr.Clear();
    if (m_pPTbl != NULL) delete m_pPTbl;
    m_pPTbl = NULL;
}

void CGameState::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        m_mapString.Serialize(ar);
    }
    else
    {
        Clear();

        m_pPBMgr = new CPBoardManager(CheckedDeref(static_cast<CGamDoc*>(ar.m_pDocument)));
        m_pPTbl = new CPieceTable;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
            m_mapString.Serialize(ar);                  // V2.0
    }
    m_pPBMgr->Serialize(ar);
    m_pYMgr.Serialize(ar);
    m_pPTbl->Serialize(ar);
}

void CGameState::CPBoardManagerDelete::operator()(CPBoardManager* p) const
{
    delete p;
}
