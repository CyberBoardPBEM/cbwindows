// GameBox.cpp
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
CTileManager*   CGameBox::c_pTileMgr = NULL;// Temp pointer to tile manager
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
    if (m_pTMgr != NULL) delete m_pTMgr;
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

    int nSaveLoadingVersion = CGamDoc::GetLoadingVersion();

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
        if (NumVersion(verMajor, verMinor) >
            NumVersion(fileGbxVerMajor, fileGbxVerMinor))
        {
            strErr.LoadString(IDS_ERR_GAMEBOXNEWER);
            return FALSE;
        }
        SetLoadingVersion(NumVersion(verMajor, verMinor));
        CGamDoc::SetLoadingVersion(NumVersion(verMajor, verMinor));

        ar >> cEatThis;         // Eat program version
        ar >> cEatThis;

        m_pTMgr = new CTileManager;
        m_pBMgr = new CBoardManager;

        m_pPMgr = new CPieceManager;
        m_pPMgr->SetTileManager(m_pTMgr);
        m_pMMgr = new CMarkManager;
        m_pMMgr->SetTileManager(m_pTMgr);

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
    c_pTileMgr = NULL;
    CGamDoc::SetLoadingVersion(nSaveLoadingVersion);
    return TRUE;
}
