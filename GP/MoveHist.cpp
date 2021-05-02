// MoveHist.cpp
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
#include    "MoveHist.h"
#include    "MoveMgr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////

void CHistoryTable::Clear()
{
    clear();
}

void CHistoryTable::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << value_preserving_cast<WORD>(size());
        for (size_t i = 0; i < size(); i++)
            GetHistRecord(i).Serialize(ar);
    }
    else
    {
        Clear();
        WORD wSize;
        ar >> wSize;
        for (WORD i = 0; i < wSize; i++)
        {
            OwnerPtr<CHistRecord> pRcd = new CHistRecord;
            pRcd->Serialize(ar);
            AddNewHistRecord(std::move(pRcd));
        }
    }
}

////////////////////////////////////////////////////////////////////////

CHistRecord::CHistRecord()
{
    m_nGamFileVersion = NumVersion(fileGamVerMajor, fileGamVerMinor);
    m_dwFilePos = 0;
    m_pMList = nullptr;
}

void CHistRecord::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_timeCreated;
        ar << m_timeAbsorbed;
        ar << m_strTitle;
        ar << m_strDescr;
        if (m_pMList)
        {
            ar << (BYTE)1;                  // Write out move list existance flag
            m_pMList->Serialize(ar);
        }
        else
            ar << (BYTE)0;
    }
    else
    {
        ar >> m_timeCreated;
        ar >> m_timeAbsorbed;
        ar >> m_strTitle;
        ar >> m_strDescr;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))
        {
            m_pMList = nullptr;
            BYTE fMoveExist;
            ar >> fMoveExist;
            if (fMoveExist)
            {
                m_pMList = MakeOwner<CMoveList>();
                m_pMList->Serialize(ar);
            }
        }
        else
        {
            m_pMList = nullptr;
            ar >> m_dwFilePos;
            if (CGamDoc::GetLoadingVersion() >= NumVersion(0, 59))
            {
                DWORD dwTmp;
                ar >> dwTmp; m_nGamFileVersion = (int)dwTmp;
            }
            else
            {
                // Assume that the game file version is the version
                // being loaded right now.
                m_nGamFileVersion = CGamDoc::GetLoadingVersion();
            }
        }
    }
}

