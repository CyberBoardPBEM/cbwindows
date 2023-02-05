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
    m_pPBMgr = NULL;
    m_pPTbl = NULL;
}

BOOL CGameState::CompareState(const CGamDoc& doc) const
{
    ASSERT(m_pPBMgr != NULL);
    ASSERT(m_pPTbl != NULL);
    // Compare the string tables

    // Compare the piece tables
    if (!doc.GetPieceTable()->Compare(*m_pPTbl))
        return FALSE;
    // Compare the play boards
    if (!doc.GetPBoardManager()->Compare(*m_pPBMgr))
        return FALSE;
    // Compare the trays
    if (!doc.GetTrayManager()->Compare(m_pYMgr))
        return FALSE;
    return TRUE;
}

BOOL CGameState::SaveState(CGamDoc& doc)
{
    Clear();
    TRY
    {
        m_mapString.Clone(doc.GetGameStringMap());
        OwnerPtr<CPBoardManager> temp1 = doc.GetPBoardManager()->Clone(doc);
        OwnerOrNullPtr<CPBoardManager> temp2 = std::move(temp1);
        m_pPBMgr = CB::get_underlying(std::move(temp2)).release();
        m_pYMgr = doc.GetTrayManager()->Clone(&doc);
        m_pPTbl = doc.GetPieceTable()->Clone();
    }
    CATCH_ALL(e)
    {
        Clear();
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}

BOOL CGameState::RestoreState(CGamDoc& doc) const
{
    ASSERT(m_pPBMgr != NULL);
    ASSERT(m_pPTbl != NULL);

    TRY
    {
        doc.GetGameStringMap().Clone(m_mapString);
        doc.GetPBoardManager()->Restore(doc, *m_pPBMgr);
        doc.GetTrayManager()->Restore(&doc, m_pYMgr);
        doc.GetPieceTable()->Restore(*m_pPTbl);
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

        CGamDoc& doc = CheckedDeref(static_cast<CGamDoc*>(ar.m_pDocument));
        m_pPBMgr = new CPBoardManager(doc);
        m_pPTbl = new CPieceTable(*doc.GetPieceManager(), doc);

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
