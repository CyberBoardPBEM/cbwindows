// GmDoc.cpp : implementation of the CGamDoc class
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

#include    "stdafx.h"
#include    <io.h>
#include    "StrLib.h"
#include    "GMisc.h"
#include    "Gm.h"
#include    "GmDoc.h"
#include    "DibApi.h"
#include    "CDib.h"
#include    "GdiTools.h"
#include    "Board.h"
#include    "Pieces.h"
#include    "Marks.h"

#include    "FrmMain.h"
#include    "VwTilesl.h"
#include    "VwEdtbrd.h"

#include    "DlgBrdp.h"
#include    "DlgGboxp.h"
#include    "DlgMkbrd.h"
#include    "DlgMakts.h"
#include    "DlgMgrpn.h"
#include    "DlgPgrpn.h"
#include    "DlgGPass.h"
#include    "DlgSPass.h"

#include    <atlbase.h>
#include    <comutil.h>

CFontTbl CGamDoc::m_fontTbl;                // Global font table
int CGamDoc::c_fileVersion = 0;
Features CGamDoc::c_fileFeatures;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGamDoc, CDocument)
IMPLEMENT_DYNCREATE(CGmBoxHint, CObject)

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

// KLUDGE:  get access to CGamDoc::GetLoadingVersion
int GetLoadingVersion()
{
    return CGamDoc::GetLoadingVersion();
}

const Features& GetFileFeatures()
{
    return CGamDoc::GetFileFeatures();
}

void SetFileFeatures(Features&& fs)
{
    CGamDoc::SetFileFeatures(std::move(fs));
}

///////////////////////////////////////////////////////////////////////
// CGamDoc - message dispatch table

BEGIN_MESSAGE_MAP(CGamDoc, CDocument)
    ON_COMMAND(ID_EDIT_GBOXPROPERTIES, OnEditGbxProperties)
    ON_COMMAND(ID_EDIT_CREATEBOARD, OnEditCreateBoard)
    ON_COMMAND(ID_EDIT_CREATETILEGROUP, OnEditCreateTileGroup)
    ON_COMMAND(ID_EDIT_CREATEPIECEGROUP, OnEditCreatePieceGroup)
    ON_COMMAND(ID_EDIT_CREATEMARKGROUP, OnEditCreateMarkGroup)
    ON_COMMAND(ID_PROJECT_CHGID, OnProjectChangeFingerPrint)
    ON_COMMAND(ID_STICKY_DRAWTOOLS, OnStickyDrawTools)
    ON_UPDATE_COMMAND_UI(ID_STICKY_DRAWTOOLS, OnUpdateStickyDrawTools)
    ON_COMMAND(ID_DUMP_TILEDATA, OnDumpTileData)
    ON_COMMAND(ID_BUGFIX_DUMPBADTILES, OnBugFixDumpBadTiles)
    ON_COMMAND(ID_EXPORT_GAMEBOX, OnExportGamebox)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////

CGamDoc::CGamDoc()
{
    m_pBMgr = NULL;
    m_pTMgr = NULL;
    m_pPMgr = NULL;
    m_pMMgr = NULL;
    m_dwMajorRevs = 0;
    m_dwMinorRevs = 0;
    m_nBitsPerPixel = 0;
    m_bStickyDrawTools = FALSE;

    // Create a box ID incase needed.
    GUID  guid;
    CoCreateGuid(&guid);
    memcpy(m_abyteBoxID, (BYTE*)&guid, 16);

    ClearGameboxPasskey();

    m_wCompressLevel = 1;       // Best compression speed

    m_bMajorRevIncd = FALSE;    // So only increased once.

    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;

    m_pCustomColors = CColorPalette::CustomColorsAllocate();

    m_palTile.SetDocument(this);
}

CGamDoc::~CGamDoc()
{
    CColorPalette::CustomColorsFree(m_pCustomColors);
    if (m_pTMgr) delete &*m_pTMgr;
    if (m_pBMgr) delete m_pBMgr;
    if (m_pPMgr) delete m_pPMgr;
    if (m_pMMgr) delete m_pMMgr;
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::OnCloseDocument()
{
    CDocument::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////

static CRuntimeClass *tblBrd[] = { RUNTIME_CLASS(CBrdEditView), NULL };

void CGamDoc::OnIdle(BOOL bActive)
{
    if (bActive)
    {
        CDockTilePalette* pDockTile = GetMainFrame()->GetDockingTileWindow();
        pDockTile->SetChild(&m_palTile);
        GetMainFrame()->UpdatePaletteWindow(pDockTile, tblBrd,
            GetMainFrame()->IsTilePaletteOn());
    }
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
    WORD wHint = LOWORD(lHint);
    if (
        ((wHint & HINT_TILEGROUP) && !(wHint & ~HINT_TILEGROUP)) ||
        wHint == HINT_TILESETDELETED ||
        wHint == HINT_TILESETPROPCHANGE ||
        wHint == HINT_ALWAYSUPDATE
        )
        m_palTile.UpdatePaletteContents();
    CDocument::UpdateAllViews(pSender, lHint, pHint);
}

///////////////////////////////////////////////////////////////////////

BOOL CGamDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;
    return SetupBlankBoard();
}

BOOL CGamDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    BOOL bOK = CDocument::OnOpenDocument(lpszPathName);
    // If the game loaded OK, check if it's password protected.
    if (bOK)
    {
        // Hash the box ID and check as the internal password for
        // a gamebox that doesn't require a password.
        std::array<std::byte, 16> bfr = Compute16ByteHash(m_abyteBoxID, sizeof(m_abyteBoxID));
        if (m_abytePass == bfr)
            return bOK;

        // Prompt and check the password...
        CDlgGetGameboxPassword dlg;
        if (dlg.DoModal() == IDOK)
        {
            bfr = ComputeGameboxPasskey(dlg.m_strPassword);
            if (bfr == m_abytePass)
                return TRUE;
            AfxMessageBox(IDS_ERR_BAD_PASSWORD);
        }
        else
            AfxMessageBox(IDS_ERR_NEED_PASSWORD);
        return FALSE;
    }
    return bOK;
}

BOOL CGamDoc::OnSaveDocument(const char* pszPathName)
{
    // Make sure tile edits are saved.
    UpdateAllViews(NULL, HINT_FORCETILEUPDATE, NULL);
    if (_access(pszPathName, 0) != -1)
    {
        char szTmp[_MAX_PATH];
        lstrcpy(szTmp, pszPathName);
        SetFileExt(szTmp, "gb_");
        if (_access(szTmp, 0) != -1)        // Remove previous backup
        {
            TRY
            {
                CFile::Remove(szTmp);
            }
            CATCH_ALL(e)
            {
                CString strErr;
                strErr.Format(IDS_ERR_SAVE_DELETE_FAIL, szTmp);
                AfxMessageBox(strErr);
                return FALSE;
            }
            END_CATCH_ALL
        }
        CFile::Rename(pszPathName, szTmp);
    }
    return CDocument::OnSaveDocument(pszPathName);
}

void CGamDoc::DeleteContents()
{
    if (m_pTMgr) delete &*m_pTMgr;
    m_pTMgr = NULL;
    if (m_pBMgr) delete m_pBMgr;
    m_pBMgr = NULL;
    if (m_pPMgr) delete m_pPMgr;
    m_pPMgr = NULL;
    if (m_pMMgr) delete m_pMMgr;
    m_pMMgr = NULL;
    if (m_palTile.m_hWnd != NULL)
    {
        CDockTilePalette* pFrame = (CDockTilePalette*)m_palTile.GetDockingFrame();
        if (pFrame != NULL)
        {
            ASSERT_KINDOF(CDockTilePalette, pFrame);
            pFrame->SetChild(NULL);         // Need to remove pointer from Tray's UI Frame.
        }
        m_palTile.DestroyWindow();
    }

    m_bMajorRevIncd = FALSE;
    CColorPalette::CustomColorsClear(m_pCustomColors);
}

///////////////////////////////////////////////////////////////////////

LPVOID CGamDoc::GetCustomColors()
{
    ASSERT(m_pCustomColors != NULL);
    return m_pCustomColors;
}

void CGamDoc::SetCustomColors(LPVOID pCustColors)
{
    ASSERT(m_pCustomColors != NULL);
    CColorPalette::CustomColorsSet(m_pCustomColors, pCustColors);
    SetModifiedFlag();
}

///////////////////////////////////////////////////////////////////////
// Support for new unique views on this document

BOOL CGamDoc::CreateNewFrame(CDocTemplate* pTemplate, LPCSTR pszTitle,
    LPVOID lpvCreateParam)
{
    CMDIChildWndEx* pNewFrame
        = (CMDIChildWndEx*)(pTemplate->CreateNewFrame(this, NULL));
    if (pNewFrame == NULL)
        return FALSE;               // Not created
    ASSERT(pNewFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)));
    CString str = GetTitle();
    str += " - ";
    str += pszTitle;
    pNewFrame->SetWindowText(str);
    m_lpvCreateParam = lpvCreateParam;
    pTemplate->InitialUpdateFrame(pNewFrame, this);
    // KLUDGE:  work around https://github.com/CyberBoardPBEM/cbwindows/issues/23
    GetMainFrame()->RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE);
    m_lpvCreateParam = NULL;
    return TRUE;
}

///////////////////////////////////////////////////////////////////////
// Returns TRUE if views may have been affected

BOOL CGamDoc::NotifyTileDatabaseChange(BOOL bDelScan /* = TRUE */)
{
//  GetMainFrame()->GetTilePalWnd()->SynchronizeTileToolPalette();
    m_palTile.UpdatePaletteContents();

    // Verify the boards etc...aren't using nonexistant TileID's.
    if (bDelScan)
        return PurgeMissingTileIDs();
    else
        return FALSE;
}

BOOL CGamDoc::PurgeMissingTileIDs()
{
    BOOL bRet = m_pBMgr->PurgeMissingTileIDs();
    bRet |= m_pPMgr->PurgeMissingTileIDs(&GetGameStringMap());
    bRet |= m_pMMgr->PurgeMissingTileIDs();
    return bRet;
}

BOOL CGamDoc::QueryTileInUse(TileID tid) const
{
    BOOL bRet = m_pBMgr->IsTileInUse(tid);
    bRet |= m_pPMgr->IsTileInUse(tid);
    bRet |= m_pMMgr->IsTileInUse(tid);
    return bRet;
}

BOOL CGamDoc::QueryAnyOfTheseTilesInUse(const std::vector<TileID>& tbl) const
{
    for (size_t i = 0; i < tbl.size(); i++)
    {
        if (QueryTileInUse(tbl[i]))
            return TRUE;
    }
    return FALSE;
}

void CGamDoc::IncrMajorRevLevel()
{
    if (m_bMajorRevIncd)
        return;
    m_dwMajorRevs++;
    m_bMajorRevIncd = TRUE;
//  m_dwMinorRevs = 0;          // ALLOW IT TO INCREMENT
}

///////////////////////////////////////////////////////////////////////
// The game box ID is a combination of a time based random
// number and the lower WORD of the amount of millisecs that
// have transpired since Windows was loaded on this machine.

DWORD CGamDoc::IssueGameBoxID()
{
    return (DWORD)MAKELONG(GetTimeBasedRandomNumber(FALSE),
        (WORD)GetTickCount());
}

///////////////////////////////////////////////////////////////////////

CView* CGamDoc::FindTileEditorView(TileID tid)
{
    POSITION pos = GetFirstViewPosition();
    while (pos != NULL)
    {
        CTileSelView* pView = (CTileSelView*)GetNextView(pos);
        if (pView->IsKindOf(RUNTIME_CLASS(CTileSelView)))
        {
            if (pView->GetTileID() == tid)
                return pView;
        }
    }
    return NULL;
}

CView* CGamDoc::FindBoardEditorView(const CBoard& pBoard)
{
    POSITION pos = GetFirstViewPosition();
    while (pos != NULL)
    {
        CBrdEditView* pView = (CBrdEditView*)GetNextView(pos);
        if (pView->IsKindOf(RUNTIME_CLASS(CBrdEditView)))
        {
            if (pView->GetBoard() == &pBoard)
                return pView;
        }
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////
// Support for strings associated with game elements (pieces, markers)

CString CGamDoc::GetGameElementString(GameElement gelem) const
{
    CString str;
    m_mapStrings.Lookup(gelem, str);
    return str;
}

BOOL CGamDoc::HasGameElementString(GameElement gelem) const
{
    CString str;
    return m_mapStrings.Lookup(gelem, str);
}

///////////////////////////////////////////////////////////////////////

BOOL CGamDoc::SetupBlankBoard()
{
    CGmBoxPropsDialog dlg;

    dlg.m_bPropEdit = FALSE;            // Sets focus to ID field

    if (dlg.DoModal() != IDOK)
        return FALSE;

    m_dwGameID = IssueGameBoxID();
    m_strAuthor = dlg.m_strAuthor;
    m_strTitle = dlg.m_strTitle;
    m_strDescr = dlg.m_strDescr;

    m_pTMgr = new CTileManager;
    m_pBMgr = new CBoardManager(*this);

    m_pPMgr = new CPieceManager(*m_pTMgr);
    m_pMMgr = new CMarkManager;
    m_pMMgr->SetTileManager(&*m_pTMgr);

    // Setup tile palette.
    m_palTile.Create(GetMainFrame()->GetDockingTileWindow());
    return TRUE;
}

///////////////////////////////////////////////////////////////////////
// This is here for sharing. Probably could go in BOARD.CPP too.
// returns TRUE if board was changed.

BOOL CGamDoc::DoBoardPropertyDialog(CBoard& pBoard)
{
    CBoardPropDialog dlg;
    CBoardArray& pBrdAry = pBoard.GetBoardArray();

    dlg.m_strName = pBoard.GetName().mfc_str();
    dlg.m_bGridSnap = pBoard.m_bGridSnap;
    dlg.m_xGridSnap = pBoard.m_xGridSnap;
    dlg.m_yGridSnap = pBoard.m_yGridSnap;
    dlg.m_xGridSnapOff = pBoard.m_xGridSnapOff;
    dlg.m_yGridSnapOff = pBoard.m_yGridSnapOff;
    dlg.m_bCellLines = pBoard.GetCellBorder();
    dlg.m_bCellBorderOnTop = pBoard.GetCellBorderOnTop();
    dlg.m_nStyleNum = (int)pBrdAry.GetCellNumStyle();
    dlg.m_bTrackCellNum = pBrdAry.GetCellNumTracking();
    dlg.m_nRowTrkOffset = pBrdAry.GetRowCellTrackingOffset();
    dlg.m_nColTrkOffset = pBrdAry.GetColCellTrackingOffset();
    dlg.m_bRowTrkInvert = pBrdAry.GetRowCellTrackingInvert();
    dlg.m_bColTrkInvert = pBrdAry.GetColCellTrackingInvert();
    dlg.m_crCellFrame = pBrdAry.GetCellFrameColor();
    dlg.m_bEnableXParentCells = pBrdAry.IsTransparentCellTilesEnabled();

    dlg.m_nRows = pBrdAry.GetRows();
    dlg.m_nCols = pBrdAry.GetCols();
    CSize size = pBrdAry.GetCellSize(fullScale);
    dlg.m_nCellHt = size.cy;
    dlg.m_nCellWd = size.cx;
    const CCellForm& pcf = pBrdAry.GetCellForm(fullScale);
    dlg.m_eCellStyle = pcf.GetCellType();
    dlg.m_bStagger = pcf.GetCellStagger();

    if (dlg.DoModal() == IDOK)
    {
        pBoard.SetName(dlg.m_strName);
        pBoard.m_bGridSnap = dlg.m_bGridSnap;
        pBoard.m_xGridSnap = dlg.m_xGridSnap;
        pBoard.m_yGridSnap = dlg.m_yGridSnap;
        pBoard.m_xGridSnapOff = dlg.m_xGridSnapOff;
        pBoard.m_yGridSnapOff = dlg.m_yGridSnapOff;
        pBoard.SetCellBorder(dlg.m_bCellLines);
        pBoard.SetCellBorderOnTop(dlg.m_bCellBorderOnTop);
        pBrdAry.SetCellNumStyle((CellNumStyle)dlg.m_nStyleNum);
        pBrdAry.SetCellNumTracking(dlg.m_bTrackCellNum);
        pBrdAry.SetRowCellTrackingOffset(dlg.m_nRowTrkOffset);
        pBrdAry.SetColCellTrackingOffset(dlg.m_nColTrkOffset);
        pBrdAry.SetRowCellTrackingInvert(dlg.m_bRowTrkInvert);
        pBrdAry.SetColCellTrackingInvert(dlg.m_bColTrkInvert);
        pBrdAry.SetCellFrameColor(dlg.m_crCellFrame);
        pBrdAry.SetTransparentCellTilesEnabled(dlg.m_bEnableXParentCells);

        if (dlg.m_bShapeChanged)
        {
            if (dlg.m_eCellStyle == cformRect)
            {
                dlg.m_bStagger = CellStagger::Invalid;
            }
            if (dlg.m_eCellStyle == cformHexPnt) // Only first param is used
                dlg.m_nCellHt = dlg.m_nCellWd;
            pBrdAry.ReshapeBoard(dlg.m_nRows, dlg.m_nCols,
                dlg.m_nCellHt, dlg.m_nCellWd, dlg.m_bStagger);
            pBoard.ForceObjectsOntoBoard();
        }
        CGmBoxHint hint;
        hint.GetArgs<HINT_BOARDPROPCHANGE>().m_pBoard = &pBoard;
        UpdateAllViews(NULL, HINT_BOARDPROPCHANGE, &hint);
        return TRUE;
    }
    return FALSE;
}

///////////////////////////////////////////////////////////////////////

TileID CGamDoc::CreateTileFromDib(CDib* pDib, size_t nTSet)
{
    int xTile = pDib->Width();
    int yTile = pDib->Height();
    TileID tid = m_pTMgr->CreateTile(nTSet, CSize(xTile, yTile),
        CSize(xTile/2, yTile/2), RGB(255, 255, 255));
    OwnerPtr<CBitmap> pBMap = pDib->DIBToBitmap(GetAppPalette());
    OwnerPtr<CBitmap> bmHalf = CloneScaledBitmap(*pBMap, CSize(xTile/2, yTile/2),
        COLORONCOLOR);

    CTileUpdatable tile = m_pTMgr->GetTile(tid, fullScale);
    tile.Update(*pBMap);
    tile = m_pTMgr->GetTile(tid, halfScale);
    tile.Update(*bmHalf);

    return tid;
}

///////////////////////////////////////////////////////////////////////
// CGamDoc serialization

void CGamDoc::Serialize(CArchive& ar)
{
    ar.m_pDocument = this;
    if (ar.IsStoring())
    {
        // File Header
        ar.Write(FILEGBXSIGNATURE, 4);
        ar << (BYTE)fileGbxVerMajor;
        ar << (BYTE)fileGbxVerMinor;

        // leave space for pointer to feature list at end of file
        uint64_t offsetOffsetFeatureTable = UINT64_MAX;
        if (NumVersion(fileGbxVerMajor, fileGbxVerMinor) >= NumVersion(5, 0))
        {
            ar.Flush();     // ensure GetPosition() is current
            offsetOffsetFeatureTable = ar.GetFile()->GetPosition();
            ar << uint64_t(0);

            // initialize feature list
            c_fileFeatures = GetCBForcedFeatures();
            /* these features are global in the sense that, if
                any ID needs 32 bits, then we store all IDs in
                32 bit format */
            if (GetBoardManager()->Needs32BitIDs() ||
                GetTileManager()->Needs32BitIDs() ||
                GetPieceManager()->Needs32BitIDs() ||
                GetMarkManager()->Needs32BitIDs())
            {
                if (!GetCBFeatures().Check(ftrId32Bit))
                {
                    AfxThrowArchiveException(CArchiveException::badSchema);
                }
                c_fileFeatures.Add(ftrId32Bit);
            }
            /* TODO:  Check all size_t for 64bit vals.  For now,
                        use unless forbidden */
            if (GetCBFeatures().Check(ftrSizet64Bit))
            {
                c_fileFeatures.Add(ftrSizet64Bit);
            }
        }
        else if (NumVersion(fileGbxVerMajor, fileGbxVerMinor) == NumVersion(4, 0)) {
            c_fileFeatures = GetCBFile4Features();
        }
        else
        {
            ASSERT(NumVersion(fileGmvVerMajor, fileGmvVerMinor) <= NumVersion(3, 90));
            c_fileFeatures = Features();
        }

        ar << (BYTE)progVerMajor;
        ar << (BYTE)progVerMinor;

        // Main serialization
        ar << (WORD)GetCurrentVideoResolution();    // m_nBitsPerPixel
        ar << m_dwMajorRevs;
        ar << m_dwMinorRevs;

        ar << m_dwGameID;

        ar.Write(m_abyteBoxID, 16);

        ar << m_strAuthor;
        ar << m_strTitle;
        ar << m_strDescr;

        ar.Write(m_abytePass.data(), value_preserving_cast<UINT>(m_abytePass.size()));

        ar << (WORD)m_bStickyDrawTools;
        ar << m_wCompressLevel;
        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;

        m_mapStrings.Serialize(ar);                 // Ver2.0

        ASSERT(m_pTMgr);
        m_pTMgr->Serialize(ar);     // Tiles
        ASSERT(m_pBMgr);
        m_pBMgr->Serialize(ar);     // Boards
        ASSERT(m_pPMgr);
        m_pPMgr->Serialize(ar);     // Pieces
        ASSERT(m_pMMgr);
        m_pMMgr->Serialize(ar);     // Markers

        // Serialize stuff that the game player program
        // doesn't need here...

        CColorPalette::CustomColorsSerialize(ar, m_pCustomColors);

        // serialize done, so write features now
        if (NumVersion(fileGbxVerMajor, fileGbxVerMinor) >= NumVersion(5, 0))
        {
            ar.Flush();     // ensure GetPosition() is current
            uint64_t offsetFeatureTable = ar.GetFile()->GetPosition();
            ar << c_fileFeatures;
            // write data at current file pointer before changing it
            ar.Flush();
            ar.GetFile()->Seek(value_preserving_cast<LONGLONG>(offsetOffsetFeatureTable), CFile::begin);
            ar << offsetFeatureTable;
        }
    }
    else
    {
        TRY
        {
            // File Header
            char str[5];
            ar.Read(&str, 4);
            str[4] = 0;
            if (strcmp(str, FILEGBXSIGNATURE) != 0)
            {
                AfxMessageBox(IDS_ERR_NOTAGAMEBOX, MB_OK | MB_ICONEXCLAMATION);
                AfxThrowArchiveException(CArchiveException::genericException);
            }
            BYTE bucket;
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
                    /* drop read-buffered data since next read
                        will be at a different file offset */
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
                fileFeatures = Features();
            }

            if (NumVersion(verMajor, verMinor) >
                NumVersion(fileGbxVerMajor, fileGbxVerMinor) &&
                // file 3.90 is the same as 3.10
                NumVersion(verMajor, verMinor) != NumVersion(3, 90))
            {
                AfxMessageBox(IDS_ERR_GAMEBOXNEWER, MB_OK | MB_ICONEXCLAMATION);
                AfxThrowArchiveException(CArchiveException::genericException);
            }
            if (NumVersion(verMajor, verMinor) < NumVersion(fileGbxVerMajor, fileGbxVerMinor))
            {
                if (AfxMessageBox(IDS_WARN_FILE_UPGRADE,
                    MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING) != IDOK)
                {
                    AfxThrowArchiveException(CArchiveException::genericException);
                }
            }
            SetLoadingVersionGuard setLoadingVersionGuard(NumVersion(verMajor, verMinor),
                                                            NumVersion(fileGbxVerMajor, fileGbxVerMinor));
            SetFileFeaturesGuard setFileFeaturesGuard(ar, std::move(fileFeatures),
                                                            Features(GetCBFeatures()));
            ar >> bucket;           // Eat program version
            ar >> bucket;

            m_pTMgr = new CTileManager;
            m_pBMgr = new CBoardManager(*this);

            m_pPMgr = new CPieceManager(*m_pTMgr);
            m_pMMgr = new CMarkManager;
            m_pMMgr->SetTileManager(&*m_pTMgr);

            // Main serialization
            ar >> m_nBitsPerPixel;
            if (m_nBitsPerPixel > (WORD)GetCurrentVideoResolution())
            {
                if (AfxMessageBox(IDS_ERR_VIDEORES,
                    MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
                {
                    AfxThrowArchiveException(CArchiveException::genericException);
                }
            }
            ar >> m_dwMajorRevs;
            ar >> m_dwMinorRevs;
            m_dwMinorRevs++;            // New rev level.

            if (CGamDoc::GetLoadingVersion() > NumVersion(0, 55))
                ar >> m_dwGameID;
            else
            {
                CString str;            // Create an ID based on old name
                ar >> str;
                m_dwGameID = GetStringHash(str);
            }

            if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // V2.0
                ar.Read(m_abyteBoxID, 16);

            ar >> m_strAuthor;
            ar >> m_strTitle;
            ar >> m_strDescr;

            if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // V2.0
                ar.Read(m_abytePass.data(), value_preserving_cast<UINT>(m_abytePass.size()));

            WORD wTmp;
            ar >> wTmp; m_bStickyDrawTools = (BOOL)wTmp;

            if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))   // V2.0
                ar >> m_wCompressLevel;

            if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
                ar >> m_wReserved1;
            ar >> m_wReserved2;
            ar >> m_wReserved3;
            ar >> m_wReserved4;

            if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
                m_mapStrings.Serialize(ar);                        // V2.0

            m_pTMgr->Serialize(ar);     // Tiles
            m_pBMgr->Serialize(ar);     // Boards
            m_pPMgr->Serialize(ar);     // Pieces
            m_pMMgr->Serialize(ar);     // Markers

            if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))// Ver 2.90
                CColorPalette::CustomColorsSerialize(ar, m_pCustomColors);
        }
        CATCH(CArchiveException, e)
        {
            THROW_LAST();       // Just toss this up
        }
        AND_CATCH_ALL(e)
        {
            AfxThrowArchiveException(CArchiveException::genericException);
        }
        END_CATCH_ALL

        // Setup tile palette.
        m_palTile.Create(GetMainFrame()->GetDockingTileWindow());
    }
}

/////////////////////////////////////////////////////////////////////////////
// CGamDoc diagnostics

#ifdef _DEBUG
void CGamDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CGamDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGamDoc commands

void CGamDoc::OnEditGbxProperties()
{
    CGmBoxPropsDialog dlg;

    dlg.m_strAuthor = m_strAuthor;
    dlg.m_strTitle = m_strTitle;
    dlg.m_strDescr = m_strDescr;
    dlg.m_nCompressLevel = (int)m_wCompressLevel;

    if (dlg.DoModal() == IDOK)
    {
        m_strAuthor = dlg.m_strAuthor;
        m_strTitle = dlg.m_strTitle;
        m_strDescr = dlg.m_strDescr;
        m_wCompressLevel = (WORD)dlg.m_nCompressLevel;
        if (dlg.m_bPassSet)
        {
            // Update the gamebox password.
            if (dlg.m_strPassword.IsEmpty())
                ClearGameboxPasskey();      // Disable the password (if any)
            else
                m_abytePass = ComputeGameboxPasskey(dlg.m_strPassword);// Set the password
        }
        SetModifiedFlag();
    }
}

// Do an MD5 hash of the password, append the box ID and
// hash it again. This value is the password key used to
// allow editing of the gamebox.
std::array<std::byte, 16> CGamDoc::ComputeGameboxPasskey(LPCTSTR pszPassword)
{
    CString strPassword = pszPassword;
    strPassword += KEY_PASS_POSTFIX;

    std::array<std::byte, 32> bfr = Compute16ByteHash<32>((LPCTSTR)strPassword, value_preserving_cast<size_t>(strPassword.GetLength()));
    memcpy(&bfr[16], m_abyteBoxID, 16);
    return Compute16ByteHash(bfr.data(), bfr.size());
}

void CGamDoc::ClearGameboxPasskey()
{
    // Disable the password (if any)
    // Hash the box ID and use it as the internal password for
    // a gamebox that doesn't require a password.
    m_abytePass = Compute16ByteHash(m_abyteBoxID, sizeof(m_abyteBoxID));
}

void CGamDoc::OnEditCreateBoard()
{
    CGridType dlg;

    dlg.m_iCellWd = 55;
    dlg.m_iCellHt = 55;
    dlg.m_nBoardType = (int)cformHexPnt;
    dlg.m_iRows = size_t(30);
    dlg.m_iCols = size_t(30);

    if (dlg.DoModal() == IDOK)
    {
        // Create a Game Board
        CBoardManager* pBMgr = GetBoardManager();
        CBoard* pBoard = new CBoard;
        pBoard->SetSerialNumber(pBMgr->IssueSerialNumber());
        pBoard->SetName(dlg.m_strBoardName);

        OwnerOrNullPtr<CBoardArray> pBrdAry = MakeOwner<CBoardArray>(CheckedDeref(GetTileManager()));

        if (dlg.m_nBoardType == cformRect)
        {
            dlg.m_bStagger = CellStagger::Invalid;
        }
        if (dlg.m_nBoardType == cformHexPnt)    // Only first param is used
            dlg.m_iCellHt = dlg.m_iCellWd;

        pBrdAry->CreateBoard((CellFormType)dlg.m_nBoardType,
            dlg.m_iRows, dlg.m_iCols, dlg.m_iCellHt, dlg.m_iCellWd,
            dlg.m_bStagger);

        pBoard->SetBoardArray(std::move(pBrdAry));
        pBoard->SetBkColor(RGB(255, 255, 255));
        pBMgr->Add(pBoard);
        pBoard->SetMaxDrawLayer(LAYER_TOP);

        SetModifiedFlag();

        UpdateAllViews(NULL, HINT_UPDATEPROJVIEW);
    }
}

void CGamDoc::OnEditCreateTileGroup()
{
    CCreateTSet dlg;
    if (dlg.DoModal() == IDOK)
    {
        GetTileManager()->CreateTileSet(dlg.m_strTSName);
        UpdateAllViews(NULL, HINT_UPDATEPROJVIEW);
        SetModifiedFlag();
    }
}

void CGamDoc::OnEditCreatePieceGroup()
{
    CPieceGroupNameDialog dlg;
    if (dlg.DoModal() == IDOK)
    {
        GetPieceManager()->CreatePieceSet(dlg.m_strName);
        UpdateAllViews(NULL, HINT_UPDATEPROJVIEW);
        SetModifiedFlag();
    }
}

void CGamDoc::OnEditCreateMarkGroup()
{
    CMarkGroupNewDialog dlg;
    if (dlg.DoModal() == IDOK)
    {
        GetMarkManager()->CreateMarkSet(dlg.m_strName);
        UpdateAllViews(NULL, HINT_UPDATEPROJVIEW);
        SetModifiedFlag();
    }
}


void CGamDoc::OnProjectChangeFingerPrint()
{
    if (AfxMessageBox(IDP_WARN_CHGID, MB_YESNO | MB_DEFBUTTON2 |
            MB_ICONEXCLAMATION) != IDYES)
        return;
    if (AfxMessageBox(IDP_WARN_CHGID2, MB_YESNO | MB_DEFBUTTON2 |
            MB_ICONEXCLAMATION) != IDYES)
        return;

    m_dwGameID = IssueGameBoxID();
    SetModifiedFlag();
}

void CGamDoc::OnStickyDrawTools()
{
    m_bStickyDrawTools = !m_bStickyDrawTools;
}

void CGamDoc::OnUpdateStickyDrawTools(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bStickyDrawTools);
}

void CGamDoc::OnDumpTileData()
{
    CFileDialog dlg(FALSE, "txt", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||", NULL, 0);
    if (dlg.DoModal() == IDOK)
    {
        GetTileManager()->DumpTileDatabaseInfoToFile(dlg.GetPathName(), TRUE);
    }
}

void CGamDoc::OnBugFixDumpBadTiles()
{
    if (AfxMessageBox("This is a secret function that will attempt to remove illegal tiles "
        "assigned to tile sheet 255. It can also trash your GameBox. Therefore, only Dale Larson and his "
        "appropriately blessed holy minions should use it. You have been WARNED!",
        MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
    {
        if (GetTileManager()->PruneTilesOnSheet255())
            SetModifiedFlag();
    }
}

void CGamDoc::OnExportGamebox()
{
    //TBD??? Formally used as experimental XML exporter.
}
