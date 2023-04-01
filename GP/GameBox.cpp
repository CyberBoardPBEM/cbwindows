// GameBox.cpp
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

// Game Box File Format:
//      File Signature (4 BYTEs):  See GM.H
//      File Version Major (BYTE): See GM.H
//      File Version Minor (BYTE): See GM.H
//      Prog Version Major (BYTE): See GM.H
//      Prog Version Minor (BYTE): See GM.H
//
//      Object Serializations...
//

#include    "stdafx.h"
#include    "Gp.h"
#include    "GamDoc.h"
#include    "FrmMain.h"

#include    "GdiTools.h"
#include    "Board.h"
#include    "Pieces.h"
#include    "Marks.h"
#include    "GameBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////

CFontTbl        CGameBox::m_fontTbl;        // Global font table
int             CGameBox::c_gbxFileVersion = 0;

//////////////////////////////////////////////////////////////////

CGameBox::CGameBox()
{
    m_pBMgr = NULL;
    m_pTMgr = NULL;
    m_pPMgr = NULL;
    m_pMMgr = NULL;
    m_dwMajorRevs = 0;
    m_dwMinorRevs = 0;
    m_nBitsPerPixel = 0;
}

CGameBox::~CGameBox()
{
    if (m_pTMgr != NULL) delete &*m_pTMgr;
    if (m_pBMgr != NULL) delete m_pBMgr;
    if (m_pPMgr != NULL) delete m_pPMgr;
    if (m_pMMgr != NULL) delete m_pMMgr;
}

//////////////////////////////////////////////////////////////////

BOOL CGameBox::Load(CGamDoc* pDoc, LPCSTR pszPathName, CString& strErr,
    DWORD dwGbxID)
{
    CFile file;
    CFileException fe;

    if (!file.Open(pszPathName, CFile::modeRead | CFile::shareDenyWrite, &fe))
    {
        AfxFormatString1(strErr, AFX_IDP_FAILED_TO_OPEN_DOC, pszPathName);
        return FALSE;
    }

    CArchive ar(&file, CArchive::load | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = pDoc;
    ar.m_bForceFlat = FALSE;

    TRY
    {
        GetMainFrame()->BeginWaitCursor();
        // File Header
        char str[5];
        ar.Read(&str, 4);
        str[4] = 0;
        if (strcmp(str, FILEGBXSIGNATURE) != 0)
        {
            strErr.LoadString(IDS_ERR_NOTAGAMEBOX);
            return FALSE;
        }

        BYTE cEatThis;
        BYTE verMajor, verMinor;
        ar >> verMajor;
        ar >> verMinor;

        Features fileFeatures;
        if (NumVersion(verMajor, verMinor) >= NumVersion(5, 0))
        {
            try
            {
                ar.Flush();     // ensure GetPosition() is current
                uint64_t offsetOffsetFeatureTable = ar.GetFile()->GetPosition();
                uint64_t offsetFeatureTable;
                ar >> offsetFeatureTable;
                ar.Flush();
                ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetFeatureTable), CFile::begin);
                ar >> fileFeatures;
                ar.Flush();
                ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetOffsetFeatureTable), CFile::begin);
                uint64_t dummy;
                ar >> dummy;
                ASSERT(dummy == offsetFeatureTable);
            }
            catch (...)
            {
                ASSERT(!"exception");
                // report file too new
                verMajor = value_preserving_cast<BYTE>(fileGbxVerMajor + 1);
            }
        }
        else if (NumVersion(verMajor, verMinor) == NumVersion(4, 0))
        {
            fileFeatures = GetCBFile4Features();
        }
        else
        {
            ASSERT(NumVersion(verMajor, verMinor) <= NumVersion(3, 90));
        }

        if (NumVersion(verMajor, verMinor) >
            NumVersion(fileGbxVerMajor, fileGbxVerMinor) &&
            // file 3.90 is the same as 3.10
            NumVersion(verMajor, verMinor) != NumVersion(3, 90))
        {
            strErr.LoadString(IDS_ERR_GAMEBOXNEWER);
            return FALSE;
        }
        SetLoadingVersion(NumVersion(verMajor, verMinor));
        CGamDoc::SetLoadingVersionGuard setLoadingVersionGuard(NumVersion(verMajor, verMinor));
        SetFileFeaturesGuard setFileFeaturesGuard(ar, fileFeatures);

        ar >> cEatThis;         // Eat program version
        ar >> cEatThis;

        m_pTMgr = new CTileManager;
        m_pBMgr = new CBoardManager(*pDoc);

        m_pPMgr = new CPieceManager(*m_pTMgr);
        m_pMMgr = new CMarkManager;
        m_pMMgr->SetTileManager(&*m_pTMgr);

        // Main serialization
        WORD wEatThis;
        CString strEatThis;

        ar >> m_nBitsPerPixel;
        if (m_nBitsPerPixel > (WORD)GetCurrentVideoResolution())
            AfxMessageBox(IDS_ERR_GBXVIDEORES, MB_OK | MB_ICONINFORMATION);

        ar >> m_dwMajorRevs;
        ar >> m_dwMinorRevs;
        ar >> m_dwGameID;

        if (dwGbxID != 0 && dwGbxID != m_dwGameID)
        {
            strErr.LoadString(IDS_ERR_WRONGGAMEBOX);
            return FALSE;
        }

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // V2.0
        {
            BYTE bfrEatThis[16];
            ar.Read(bfrEatThis, 16);    // m_abyteBoxID
        }

        ar >> strEatThis;           // m_strAuthor;
        ar >> strEatThis;           // m_strTitle;
        ar >> strEatThis;           // m_strDescr;

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // V2.0
        {
            BYTE bfrEatThis[16];
            ar.Read(bfrEatThis, 16);// m_abytePass
        }

        ar >> wEatThis;             // m_bStickyDrawTools

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // V2.0
            ar >> wEatThis;         // m_wCompressLevel

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // V2.0
            ar >> wEatThis;         // m_wReserved1
        ar >> wEatThis;             // m_wReserved2
        ar >> wEatThis;             // m_wReserved3
        ar >> wEatThis;             // m_wReserved4

        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
            m_mapStrings.Serialize(ar);                         // V2.0

        m_pTMgr->Serialize(ar);     // Tiles
        m_pBMgr->Serialize(ar);     // Boards
        m_pPMgr->Serialize(ar);     // Pieces
        m_pMMgr->Serialize(ar);     // Markers

        // We don't need what follows the markers so just close the gamebox file.

        ar.Close();
        file.Close();
        GetMainFrame()->EndWaitCursor();
    }
    CATCH(CMemoryException, e)
    {
        file.Abort();       // Will not throw an exception
        GetMainFrame()->EndWaitCursor();
        strErr.LoadString(IDS_ERR_GBXNOMEM);
        return FALSE;
    }
    AND_CATCH_ALL(e)
    {
        file.Abort();       // Will not throw an exception
        GetMainFrame()->EndWaitCursor();
        strErr.LoadString(IDS_ERR_GBXREAD);
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}
