// GamDoc.cpp
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

#include    "stdafx.h"
#include    <io.h>
#include    "Gp.h"
#include    "FrmMain.h"
#include    "GamDoc.h"
#include    "GameBox.h"
#include    "GMisc.h"
#include    "StrLib.h"
#include    "FileLib.h"

#include    "Board.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "Marks.h"
#include    "Trays.h"
#include    "MoveMgr.h"
#include    "GamState.h"
#include    "MoveHist.h"
#include    "WStateGp.h"
#include    "Player.h"
#include    "GeoBoard.h"

#include    "VwPbrd.h"
#include    "DlgScnp.h"
#include    "DlgYnew.h"
#include    "DlgSlbrd.h"
#include    "DlgSpece.h"
#include    "DlgPbprp.h"
#include    "DlgDice.h"
#include    "DlgItray.h"
#include    "DlgNply.h"
#include    "DlgEply.h"
#include    "DlgMply.h"
#include    "DlgPass.h"
#include    "DlgChgGameOwner.h"
#include    "DlgNewGeoBoard.h"
#include    "DlgSmsg.h"

#include    "VwPrjgam.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGamDoc, CDocument)
IMPLEMENT_DYNCREATE(CGamDocHint, CObject);

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

/////////////////////////////////////////////////////////////////////////////

int CGamDoc::c_fileVersion = 0;
Features CGamDoc::c_fileFeatures;

/////////////////////////////////////////////////////////////////////////////
// CGamDoc

BEGIN_MESSAGE_MAP(CGamDoc, CDocument)
    //{{AFX_MSG_MAP(CGamDoc)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TRAYA, OnUpdateViewTrayA)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TRAYB, OnUpdateViewTrayB)
    ON_UPDATE_COMMAND_UI(ID_VIEW_MARKERPAL, OnUpdateViewMarkPalette)
    ON_COMMAND(ID_VIEW_TRAYA, OnViewTrayA)
    ON_COMMAND(ID_VIEW_TRAYB, OnViewTrayB)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_COMPMOVE, OnUpdateIndicatorCompoundMove)
    ON_COMMAND(ID_VIEW_MARKERPAL, OnViewMarkPalette)
    ON_COMMAND(ID_EDIT_SETBOOKMARK, OnEditSetBookMark)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SETBOOKMARK, OnUpdateEditSetBookMark)
    ON_COMMAND(ID_EDIT_RESTOREBOOKMARK, OnEditRestoreBookMark)
    ON_UPDATE_COMMAND_UI(ID_EDIT_RESTOREBOOKMARK, OnUpdateEditRestoreBookMark)
    ON_COMMAND(ID_EDIT_CLRBOOKMARK, OnEditClearBookMark)
    ON_COMMAND(ID_PBCK_DISCARD, OnPbckDiscard)
    ON_UPDATE_COMMAND_UI(ID_PBCK_DISCARD, OnUpdatePbckDiscard)
    ON_COMMAND(ID_PBCK_END, OnPbckEnd)
    ON_UPDATE_COMMAND_UI(ID_PBCK_END, OnUpdatePbckEnd)
    ON_COMMAND(ID_PBCK_FINISH, OnPbckFinish)
    ON_UPDATE_COMMAND_UI(ID_PBCK_FINISH, OnUpdatePbckFinish)
    ON_COMMAND(ID_PBCK_NEXT, OnPbckNext)
    ON_UPDATE_COMMAND_UI(ID_PBCK_NEXT, OnUpdatePbckNext)
    ON_COMMAND(ID_PBCK_START, OnPbckStart)
    ON_UPDATE_COMMAND_UI(ID_PBCK_START, OnUpdatePbckStart)
    ON_COMMAND(ID_ACT_DOMESSAGE, OnActDoMessage)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CLRBOOKMARK, OnUpdateEditClearBookMark)
    ON_UPDATE_COMMAND_UI(ID_ACT_DOMESSAGE, OnUpdateActDoMessage)
    ON_COMMAND(ID_PBCK_READMESSAGE, OnPbckReadMessage)
    ON_UPDATE_COMMAND_UI(ID_PBCK_READMESSAGE, OnUpdatePbckReadMessage)
    ON_COMMAND(ID_PBCK_NEXTHIST, OnPbckNextHistory)
    ON_UPDATE_COMMAND_UI(ID_PBCK_NEXTHIST, OnUpdatePbckNextHistory)
    ON_COMMAND(ID_PBCK_CLOSEHIST, OnPbckCloseHistory)
    ON_UPDATE_COMMAND_UI(ID_PBCK_CLOSEHIST, OnUpdatePbckCloseHistory)
    ON_COMMAND(ID_FILE_SENDMOVES2FILE, OnFileSendRecording2File)
    ON_UPDATE_COMMAND_UI(ID_FILE_SENDMOVES2FILE, OnUpdateFileSendRecording2File)
    ON_COMMAND(ID_FILE_DISCARDRECORDING, OnFileDiscardRecordedMoves)
    ON_UPDATE_COMMAND_UI(ID_FILE_DISCARDRECORDING, OnUpdateFileDiscardRecordedMoves)
    ON_COMMAND(ID_EDIT_CREATETRAY, OnEditCreateTray)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CREATETRAY, OnUpdateEditCreateTray)
    ON_COMMAND(ID_EDIT_SCNPROPERTIES, OnEditScenarioProperties)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SCNPROPERTIES, OnUpdateEditScenarioProperties)
    ON_COMMAND(ID_EDIT_SELECTBOARDS, OnEditSelectBoards)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELECTBOARDS, OnUpdateEditSelectBoards)
    ON_COMMAND(ID_EDIT_SELECTGAMEPIECES, OnEditSelectGamePieces)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELECTGAMEPIECES, OnUpdateEditSelectGamePieces)
    ON_COMMAND(ID_FILE_LOADMOVES, OnFileLoadMoveFile)
    ON_UPDATE_COMMAND_UI(ID_FILE_LOADMOVES, OnUpdateFileLoadMoveFile)
    ON_COMMAND(ID_ACT_COMPOUNDMOVE_BEGIN, OnActCompoundMoveBegin)
    ON_UPDATE_COMMAND_UI(ID_ACT_COMPOUNDMOVE_BEGIN, OnUpdateActCompoundMoveBegin)
    ON_COMMAND(ID_ACT_COMPOUNDMOVE_END, OnActCompoundMoveEnd)
    ON_UPDATE_COMMAND_UI(ID_ACT_COMPOUNDMOVE_END, OnUpdateActCompoundMoveEnd)
    ON_COMMAND(ID_ACT_COMPOUNDMOVE_DISCARD, OnActCompoundMoveDiscard)
    ON_UPDATE_COMMAND_UI(ID_ACT_COMPOUNDMOVE_DISCARD, OnUpdateActCompoundMoveDiscard)
    ON_COMMAND(ID_PBCK_PREVIOUS, OnPbckPrevious)
    ON_UPDATE_COMMAND_UI(ID_PBCK_PREVIOUS, OnUpdatePbckPrevious)
    ON_COMMAND(ID_FILE_SAVE_GAME_AS_SCENARIO, OnFileSaveGameAsScenario)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_GAME_AS_SCENARIO, OnUpdateFileSaveGameAsScenario)
    ON_COMMAND(ID_ACTIONS_ROLLDICE, OnActRollDice)
    ON_UPDATE_COMMAND_UI(ID_ACTIONS_ROLLDICE, OnUpdateActRollDice)
    ON_COMMAND(ID_PBCK_STEP_CMOVES, OnPbckStepCompoundMoves)
    ON_UPDATE_COMMAND_UI(ID_PBCK_STEP_CMOVES, OnUpdateStepCompoundMoves)
    ON_COMMAND(ID_EDIT_IMPORTPCEGROUPS, OnEditImportPieceGroups)
    ON_UPDATE_COMMAND_UI(ID_EDIT_IMPORTPCEGROUPS, OnUpdateEditImportPieceGroups)
    ON_COMMAND(ID_VIEW_SHOW_TIP_TEXT, OnViewShowTipText)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_TIP_TEXT, OnUpdateViewShowTipText)
    ON_COMMAND(ID_VIEW_SAVE_WIN_STATE, OnViewSaveWinState)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SAVE_WIN_STATE, OnUpdateViewSaveWinState)
    ON_COMMAND(ID_EDIT_CREATE_PLAYERS, OnEditCreatePlayers)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CREATE_PLAYERS, OnUpdateEditCreatePlayers)
    ON_COMMAND(ID_EDIT_EDIT_PLAYERS, OnEditEditPlayers)
    ON_UPDATE_COMMAND_UI(ID_EDIT_EDIT_PLAYERS, OnUpdateEditEditPlayers)
    ON_COMMAND(ID_ACT_SIMULATE_SPECTATOR, OnActSimulateSpectator)
    ON_UPDATE_COMMAND_UI(ID_ACT_SIMULATE_SPECTATOR, OnUpdateActSimulateSpectator)
    ON_COMMAND(ID_EDIT_CREATE_GEOMORPHIC, OnEditCreateGeomorphic)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CREATE_GEOMORPHIC, OnUpdateEditCreateGeomorphic)
    ON_COMMAND(ID_FILE_CREATE_REFEREE, OnFileCreateReferee)
    ON_UPDATE_COMMAND_UI(ID_FILE_CREATE_REFEREE, OnUpdateFileCreateReferee)
    ON_COMMAND(ID_FILE_CHANGE_GAME_OWNER, OnFileChangeGameOwner)
    ON_UPDATE_COMMAND_UI(ID_FILE_CHANGE_GAME_OWNER, OnUpdateFileChangeGameOwner)
    ON_COMMAND(ID_PBCK_STEP_TO_NEXT_HIST, OnPbckStepToNextHist)
    ON_UPDATE_COMMAND_UI(ID_PBCK_STEP_TO_NEXT_HIST, OnUpdatePbckStepToNextHist)
    ON_COMMAND(ID_PBCK_SKIP_KEEP_IND, OnPbckSkipKeepIndicators)
    ON_UPDATE_COMMAND_UI(ID_PBCK_SKIP_KEEP_IND, OnUpdatePbckSkipKeepIndicators)
    ON_COMMAND(ID_PBCK_AUTO_STEP, OnPbckAutoStep)
    ON_UPDATE_COMMAND_UI(ID_PBCK_AUTO_STEP, OnUpdatePbckAutoStep)
    ON_COMMAND(ID_VIEW_SHOW_TIP_OWNER, OnViewShowTipOwner)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_TIP_OWNER, OnUpdateViewShowTipOwner)
    //}}AFX_MSG_MAP
#ifdef _DEBUG
    ON_COMMAND(ID_DEBUG_MOVELIST, OnDebugMoveList)
    ON_COMMAND(ID_DEBUG_PIECETABLE, OnDebugPieceTable)
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGamDoc construction/destruction

CGamDoc::CGamDoc() :
    m_palTrayA(*this),
    m_palTrayB(*this)
{
    m_nSeedCarryOver = (UINT)GetTickCount();

    m_bScenario = FALSE;
    m_pGbx = NULL;
    m_eState = stateNotRecording;
    m_pMoves = NULL;
    m_nCurMove = Invalid_v<size_t>;
    m_nFirstMove = Invalid_v<size_t>;
    m_nCurHist = Invalid_v<size_t>;
    m_posCurMove = NULL;
    m_pPlayHist = NULL;
    m_pHistTbl = NULL;
    m_pPBMgr = NULL;
    m_pYMgr = NULL;
    m_pPTbl = NULL;
    m_pntMsgReadPos = CPoint(INT_MIN, INT_MIN);
    m_nMoveIdxAtBookMark = size_t(0);
    m_pBookMark = NULL;
    m_bTrayAVisible = FALSE;
    m_bTrayBVisible = FALSE;
    m_bMarkPalVisible = FALSE;
    m_bMsgWinVisible = FALSE;
    m_bShowObjTipText = TRUE;
    m_bDisableOwnerTips = FALSE;            // (was m_wReserved1)
    m_bSaveWindowPositions = TRUE;
    m_bKeepGamBackup = TRUE;
    m_bKeepMoveHist = TRUE;
    m_bVrfyGameState = TRUE;
    m_bVrfySaveState = TRUE;
    m_palTrayA.SetPaletteID(0);
    m_palTrayB.SetPaletteID(1);
    m_palMark.SetDocument(this);
    m_wDocRand = GetTimeBasedRandomNumber(FALSE);  // Non zero random number
    m_nMoveInterlock = 0;
    m_bQuietPlayback = FALSE;
    m_bStepToNextHist = FALSE;
    m_bKeepSkipInd = TRUE;
    m_bAutoStep = FALSE;
    m_bAutoPlayback = FALSE;
    // m_wReserved1 = 0;                    // It's now m_bDisableOwnerTips
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
    m_pTileFacingMap = NULL;

    m_pWinState = NULL;

    m_pPlayerMgr = NULL;
    m_dwCurrentPlayer = 0;
    m_dwPlayerHash = 0;

    m_bSimulateSpectator = FALSE;
}

CGamDoc::~CGamDoc()
{
    DeleteContents();
}

BOOL CGamDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    SetThisDocumentType();

    if (IsScenario())
        return OnNewScenario();
    else
        return OnNewGame();
}

void CGamDoc::OnCloseDocument()
{
    if (!IsScenario())
    {
        TRY
        {
            // Make sure game file gets closed prior to shut down.
            if (m_file.m_hFile != CFile::hFileNull)
                m_file.Close();
        }
        END_TRY
    }
    CDocument::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::OnOpenDocument(const char* pszPathName)
{
    BOOL bRet = FALSE;
    SetThisDocumentType();
    // We make this call to ensure the sizes of the palette windows layout
    // don't "bloat" during document load. I can't really tell you why this
    // works but, unless you have a proper solution... Don't mess with it!
    GetMainFrame()->ShowPalettePanes(TRUE);

    // This cheat is to have the filename being loaded available
    // to the Serialize routine
    m_strTmpPathName = pszPathName;
    bRet = CDocument::OnOpenDocument(pszPathName);
    m_strTmpPathName.Empty();

    if (bRet && !IsScenario())
    {
        // If multiplayer game, make sure the owner info hasn't been
        // tampered with.
        if (m_pPlayerMgr != NULL && !VerifyCurrentPlayerMask())
        {
            AfxMessageBox(IDS_ERR_PLAYER_TAMPER, MB_OK | MB_ICONSTOP);
            return FALSE;
        }
    }
    return bRet;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::OnSaveDocument(const char* pszPathName)
{
    if (_access(pszPathName, 0) != -1)
    {
        if (m_bKeepGamBackup && IsScenario())
        {
            char szTmp[_MAX_PATH];
            lstrcpy(szTmp, pszPathName);
            if (IsScenario())
                SetFileExt(szTmp, "gs_");
            else
                SetFileExt(szTmp, "ga_");
            if (_access(szTmp, 0) != -1)    // Remove previous backup
                CFile::Remove(szTmp);
            CFile::Rename(pszPathName, szTmp);
        }
    }
    // Before we do the actual save see if the user desires to have
    // the state of the windows saved. If they do, get the state information.
    // The state data will be saved during the serialization
    // process.
    if (m_pWinState != NULL)
        delete m_pWinState;         // Delete old data
    m_pWinState = NULL;
    if (m_bSaveWindowPositions)
    {
        m_pWinState = new CGpWinStateMgr;
        m_pWinState->SetDocument(this);
        m_pWinState->GetStateOfOpenDocumentFrames();
    }

    return CDocument::OnSaveDocument(pszPathName);
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::DeleteContents()
{
    // m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;

    if (m_file.m_hFile != CFile::hFileNull)
        m_file.Close();

    if (m_pTileFacingMap != NULL)
        delete m_pTileFacingMap;
    m_pTileFacingMap = NULL;

    if (m_pGbx != NULL) delete m_pGbx;
    m_pGbx = NULL;
    if (m_pPBMgr != NULL) delete m_pPBMgr;
    m_pPBMgr = NULL;
    if (m_pYMgr != NULL) delete m_pYMgr;
    m_pYMgr = NULL;
    if (m_pPTbl != NULL) delete m_pPTbl;
    m_pPTbl = NULL;

    m_pRcdMoves = nullptr;
    m_pHistMoves = nullptr;

    if (m_pPlayHist != NULL) delete m_pPlayHist;
    m_pPlayHist = NULL;

    if (m_pBookMark != NULL) delete m_pBookMark;
    m_pBookMark = NULL;
    if (m_pHistTbl != NULL) delete m_pHistTbl;
    m_pHistTbl = NULL;
    m_pMsgDialog = NULL;

    m_pRollState = NULL;

    m_pMoves = NULL;
    m_eState = stateNotRecording;
    m_posCurMove = NULL;
    m_nCurMove = Invalid_v<size_t>;
    m_nFirstMove = Invalid_v<size_t>;
    m_nCurHist = Invalid_v<size_t>;
    m_nMoveInterlock = 0;
    m_bQuietPlayback = FALSE;

    if (m_palTrayA.m_hWnd != NULL)
    {
        CDockTrayPalette* pFrame = (CDockTrayPalette*)m_palTrayA.GetDockingFrame();
        if (pFrame)
        {
            ASSERT_KINDOF(CDockTrayPalette, pFrame);
            pFrame->SetChild(NULL);         // Need to remove pointer from Tray's UI Frame.
        }
        m_palTrayA.DestroyWindow();
    }
    if (m_palTrayB.m_hWnd != NULL)
    {
        CDockTrayPalette* pFrame = (CDockTrayPalette*)m_palTrayB.GetDockingFrame();
        if (pFrame)
        {
            ASSERT_KINDOF(CDockTrayPalette, pFrame);
            pFrame->SetChild(NULL);         // Need to remove pointer from Tray's UI Frame.
        }
        m_palTrayB.DestroyWindow();
    }
    if (m_palMark.m_hWnd != NULL)
    {
        CDockMarkPalette* pFrame = (CDockMarkPalette*)m_palMark.GetDockingFrame();
        if (pFrame)
        {
            ASSERT_KINDOF(CDockMarkPalette, pFrame);
            pFrame->SetChild(NULL);         // Need to remove pointer from Marker's UI Frame.
        }
        m_palMark.DestroyWindow();
    }

    DiscardWindowState();

    if (m_pPlayerMgr != NULL) delete m_pPlayerMgr;
    m_pPlayerMgr = NULL;
    m_dwCurrentPlayer = 0;
    m_dwPlayerHash = 0;
    m_strPlayerFileDescr.Empty();

    m_bSimulateSpectator = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::OnIdle(BOOL bActive)
{
    if (bActive)
    {
        CMainFrame* pMFrame = GetMainFrame();

        CDockMarkPalette* pDockMark = pMFrame->GetDockingMarkerWindow();
        pDockMark->SetChild(&m_palMark);
        pMFrame->UpdatePaletteWindow(pDockMark, m_bMarkPalVisible);

        CDockTrayPalette* pDockTrayA = pMFrame->GetDockingTrayAWindow();
        pDockTrayA->SetChild(&m_palTrayA);
        pMFrame->UpdatePaletteWindow(pDockTrayA, m_bTrayAVisible);

        CDockTrayPalette* pDockTrayB = pMFrame->GetDockingTrayBWindow();
        pDockTrayB->SetChild(&m_palTrayB);
        pMFrame->UpdatePaletteWindow(pDockTrayB, m_bTrayBVisible);

        CReadMsgWnd* pDocMsg = pMFrame->GetMessageWindow();
        pMFrame->UpdatePaletteWindow(pDocMsg, m_bMsgWinVisible && !IsScenario());
        pDocMsg->SetText(this);
    }
}

/////////////////////////////////////////////////////////////////////////////

// Called my the project view to do what UpdateAllViews() would normally
// do. Since UpdateAllViews isn't virtual the MFC call that function
// will not call ours.

void CGamDoc::DoInitialUpdate()
{
    m_palTrayA.UpdatePaletteContents(NULL);
    m_palTrayB.UpdatePaletteContents(NULL);
}

void CGamDoc::UpdateAllViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CGamDocHint* ph = static_cast<CGamDocHint*>(pHint);
    if (lHint == HINT_TRAYCHANGE)
    {
        m_palTrayA.UpdatePaletteContents(ph->GetArgs<HINT_TRAYCHANGE>().m_pTray);
        m_palTrayB.UpdatePaletteContents(ph->GetArgs<HINT_TRAYCHANGE>().m_pTray);
    }
    else if (lHint == HINT_GAMESTATEUSED)
    {
        m_palTrayA.UpdatePaletteContents();
        m_palTrayB.UpdatePaletteContents();
    }
    CDocument::UpdateAllViews(pSender, lHint, pHint);
}

///////////////////////////////////////////////////////////////////////
// Support for new unique views on this document

BOOL CGamDoc::CreateNewFrame(CDocTemplate* pTemplate, LPCSTR pszTitle,
    LPVOID lpvCreateParam)
{
    m_pvParam = lpvCreateParam;
    CMDIChildWndEx* pNewFrame
        = (CMDIChildWndEx*)(pTemplate->CreateNewFrame(this, NULL));
    if (pNewFrame == NULL)
        return FALSE;               // Not created
    ASSERT(pNewFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)));
    CString str = GetTitle();
    str += " - ";
    str += pszTitle;
    pNewFrame->SetWindowText(str);
    pTemplate->InitialUpdateFrame(pNewFrame, this);
    m_pvParam = NULL;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CGamProjView& CGamDoc::FindProjectView()
{
    POSITION pos = GetFirstViewPosition();
    while (pos != NULL)
    {
        CView& pView = CheckedDeref(GetNextView(pos));
        if (pView.IsKindOf(RUNTIME_CLASS(CGamProjView)))
        {
            return static_cast<CGamProjView&>(pView);
        }
    }
    ASSERT(!"no project view");
    AfxThrowNotSupportedException();
}

CView* CGamDoc::FindPBoardView(const CPlayBoard& pPBoard)
{
    if (!IsScenario() &&
        pPBoard.IsPrivate() &&
        pPBoard.IsOwnedButNotByCurrentPlayer(*this))
    {
        ASSERT(!"private board");
        return nullptr;
    }

    POSITION pos = GetFirstViewPosition();
    while (pos != NULL)
    {
        CPlayBoardView* pView = (CPlayBoardView*)GetNextView(pos);
        if (pView->IsKindOf(RUNTIME_CLASS(CPlayBoardView)))
        {
            if (pView->GetPlayBoard() == &pPBoard)
                return pView;
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::GetDocumentFrameList(std::vector<CB::not_null<CFrameWnd*>>& tblFrames)
{
    tblFrames.clear();

    POSITION pos = GetFirstViewPosition();
    while (pos != NULL)
    {
        CView* pView = GetNextView(pos);
        CFrameWnd* pFrame = pView->GetParentFrame();
        ASSERT(pFrame != NULL);
        size_t i;
        for (i = 0; i < tblFrames.size(); i++)
        {
            if (pFrame == tblFrames.at(i))
                break;
        }
        if (i == tblFrames.size())
            tblFrames.push_back(pFrame);          // Add new frame
    }
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::OnNewScenario()
{
    CString strFilter;
    strFilter.LoadString(IDS_GBOX_FILTER);
    CString strTitle;
    strTitle.LoadString(IDS_GBOX_SELECT);

    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,
        strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return FALSE;

    m_strGBoxFile = dlg.GetFileName();
    CString strGBoxPath = dlg.GetPathName();

    // Load gamebox using archival procedures.

    m_pGbx = new CGameBox;      // Create game box object
    CString strErr;
    if (!m_pGbx->Load(this, strGBoxPath, strErr))
    {
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    // There must be at least one board in the Game Box file

    if (m_pGbx->GetBoardManager()->IsEmpty())
    {
        AfxMessageBox(IDS_ERR_NEEDABOARD, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    // Setup initial version info and Game box ID info

    m_dwGBoxID = m_pGbx->m_dwGameID;
    m_dwMajorRevs = m_pGbx->m_dwMajorRevs;
    m_dwMinorRevs = m_pGbx->m_dwMinorRevs;

    // Generate the Scenario ID...
    m_dwScenarioID = IssueScenarioID();

    // ....Create the scenario....

    m_pPBMgr = new CPBoardManager(*this);

    // Create the playing piece table...
    m_pPTbl = new CPieceTable(*m_pGbx->GetPieceManager(), *this);
    m_pPTbl->CreatePlayingPieceTable();

    // Create the tray manager.

    m_pYMgr = new CTrayManager;
    m_pYMgr->SetTileManager(m_pGbx->GetTileManager());

    // Finally set up the tray palettes
    m_palTrayA.Create(GetMainFrame()->GetDockingTrayAWindow());
    m_palTrayB.Create(GetMainFrame()->GetDockingTrayBWindow());
    m_palMark.Create(GetMainFrame()->GetDockingMarkerWindow());

    return TRUE;
}

BOOL CGamDoc::OnNewGame()
{
    CString strFilter;
    strFilter.LoadString(IDS_GSCN_FILTER);
    CString strTitle;
    strTitle.LoadString(IDS_GSCN_SELECT);

    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,
        strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return FALSE;

    // Load game scenario using serialize procedures.

    CFile file;
    CFileException fe;

    if (!file.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite,
        &fe))
    {
        CString strErr;
        AfxFormatString1(strErr, AFX_IDP_FAILED_TO_OPEN_DOC, dlg.GetPathName());
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    CArchive ar(&file, CArchive::load | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = this;
    ar.m_bForceFlat = FALSE;

    TRY
    {
        GetMainFrame()->BeginWaitCursor();
        m_strTmpPathName = dlg.GetPathName();
        m_bScenario = TRUE;             // Fake out shared code
        SerializeScenario(ar);
        m_bScenario = FALSE;
        m_strTmpPathName.Empty();
        GetMainFrame()->EndWaitCursor();
    }
    CATCH_ALL(e)
    {
        file.Abort();       // Will not throw an exception
        GetMainFrame()->EndWaitCursor();
        return FALSE;
    }
    END_CATCH_ALL

    SetGameState(stateRecording);

    // Check if we are starting a multi-player game. If we are, then
    // we need to prompt for a create a set of game files.
    if (m_pPlayerMgr != NULL)
    {
        // Generate a new Scenario ID. Only do this for multi-player
        // games.
        m_dwScenarioID = IssueScenarioID();

        // First prompt for name changes...

        CMultiplayerGameDialog dlgMultiplay;
        dlgMultiplay.m_pPlayerMgr = m_pPlayerMgr;
        if (dlgMultiplay.DoModal() != IDOK)
            return FALSE;

        // Prompt for a base file name...
        CString strExt;
        strExt.LoadString(IDS_GAME_FILTER);
        CString strTitle;
        strTitle.LoadString(IDS_GAME_SELECT_ROOT_NAME);
        CFileDialog dlg(FALSE, "gam", NULL, OFN_HIDEREADONLY, strExt, NULL, 0);
        dlg.m_ofn.lpstrTitle = strTitle;

        if (dlg.DoModal() == IDOK)
        {
            CString strFileList;

            CString strBaseName = dlg.GetPathName();
            int nPos = strBaseName.ReverseFind('.');
            strBaseName = strBaseName.Left(nPos);

            CString strExists;
            if (CheckIfPlayerFilesExist(strBaseName, dlg.GetFileExt(),
                dlgMultiplay.m_bCreateReferee, strExists))
            {
                CString strWarn;
                strWarn.Format(IDS_WARN_PLAYER_FILES_EXIST, (LPCTSTR)strExists);
                if (AfxMessageBox(strWarn,
                    MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) != IDOK)
                {
                    return FALSE;
                }
            }

            // Create an information string that will be placed in
            // all created games. This string will be displayed at the end of
            // the scenario description.

            CString strPlayers;
            for (int i = 0; i < m_pPlayerMgr->GetSize(); i++)
                strPlayers += m_pPlayerMgr->ElementAt(i).m_strName + "\r\n";
            CString strGamInfo;
            strGamInfo.Format(IDS_INFO_MPLAY_CREATE, m_pPlayerMgr->GetSize(),
                (LPCTSTR)strPlayers);
            if (dlgMultiplay.m_bCreateReferee)
            {
                CString strReferee;
                strReferee.LoadString(IDS_INFO_MPLAY_CREATE_REF);
                strGamInfo += strReferee;
            }

            // First create each of the player game files...

            for (int i = 0; i < m_pPlayerMgr->GetSize(); i++)
            {
                CString strPlayName;
                strPlayName = m_pPlayerMgr->ElementAt(i).m_strName;
                CString strFName = strBaseName + "-" + strPlayName + "." + dlg.GetFileExt();

                m_strPlayerFileDescr = strGamInfo + "@" + strFName;  // For hash check and calc
                m_dwCurrentPlayer = CPlayerManager::GetMaskFromPlayerNum(i);
                m_dwPlayerHash = CalculateHashForCurrentPlayerMask();

                if (!DoSaveGameFile(strFName))
                    return FALSE;
                strFileList += strFName + "\n";
            }

            // Create the specator game file...

            CString strSpec;
            strSpec.LoadString(IDS_GAME_SPECTATOR);

            CString strFName = strBaseName + strSpec + dlg.GetFileExt();

            m_strPlayerFileDescr = strGamInfo + "@" + strFName; // For hash check and calc
            m_dwCurrentPlayer = 0;
            m_dwPlayerHash = CalculateHashForCurrentPlayerMask();

            if (!DoSaveGameFile(strFName))
                return FALSE;

            strFileList += strFName + "\n";

            // Optionally create the referee game file...

            if (dlgMultiplay.m_bCreateReferee)
            {
                CString strReferee;
                strReferee.LoadString(IDS_GAME_REFEREE);

                strFName = strBaseName + strReferee + dlg.GetFileExt();

                m_strPlayerFileDescr = strGamInfo + "@" + strFName;// For hash check and calc
                m_dwCurrentPlayer = PLAYER_REFEREE_FLAG | OWNER_MASK_REFEREE;
                m_dwPlayerHash = CalculateHashForCurrentPlayerMask();

                if (!DoSaveGameFile(strFName))
                    return FALSE;

                strFileList += strFName;
            }
            CString strMsg;
            strMsg.Format(IDS_MSG_PLAYER_FILES, strFileList.GetString());
            AfxMessageBox(strMsg, MB_OK | MB_ICONINFORMATION);

            return FALSE;
        }
        else
            return FALSE;
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CGamDoc::CalculateHashForCurrentPlayerMask()
{
    ASSERT(!m_strPlayerFileDescr.IsEmpty());
    BYTE bfr1[18];
    BYTE bfr2[16];
    Compute16ByteHash((LPBYTE)(LPCTSTR)m_strPlayerFileDescr,
        m_strPlayerFileDescr.GetLength(), bfr1);
    bfr1[16] = (BYTE)(m_dwCurrentPlayer >> 8);
    bfr1[17] = (BYTE)(m_dwCurrentPlayer & 0xFF);
    Compute16ByteHash(bfr1, 18, bfr2);
    return *((DWORD*)(&bfr2));
}

BOOL CGamDoc::VerifyCurrentPlayerMask()
{
    DWORD dwCalcedKey = CalculateHashForCurrentPlayerMask();
    return m_dwPlayerHash == dwCalcedKey;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::CheckIfPlayerFilesExist(LPCTSTR pszBaseName, LPCTSTR pszExt, BOOL bCheckReferee,
    CString& strExist)
{
    CString strBaseName = pszBaseName;
    CString strFileExt = pszExt;
    BOOL bFilesExist = FALSE;

    for (int i = 0; i < m_pPlayerMgr->GetSize(); i++)
    {
        CString strPlayName;
        strPlayName = m_pPlayerMgr->ElementAt(i).m_strName;
        CString strFName = strBaseName + "-" + strPlayName + "." + strFileExt;

        if (_access(strFName, 0) != -1)
        {
            strExist += strFName + "\n";
            bFilesExist = TRUE;
        }
    }
    CString strSpec;
    strSpec.LoadString(IDS_GAME_SPECTATOR);

    CString strFName = strBaseName + strSpec + strFileExt;
    if (_access(strFName, 0) != -1)
    {
        strExist += strFName + "\n";
        bFilesExist = TRUE;
    }

    if (!bCheckReferee)
        return bFilesExist;

    CString strReferee;
    strReferee.LoadString(IDS_GAME_REFEREE);

    strFName = strBaseName + strReferee + strFileExt;
    if (_access(strFName, 0) != -1)
    {
        strExist += strFName + "\n";
        bFilesExist = TRUE;
    }

    return bFilesExist;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::DoSaveGameFile(LPCTSTR pszFileName)
{
    CFile file;
    CFileException fe;

    if (!file.Open(pszFileName, CFile::modeCreate |
        CFile::modeReadWrite | CFile::shareExclusive, &fe))
    {
        ReportSaveLoadException(pszFileName, &fe,
            FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
        return FALSE;
    }

    CArchive saveArchive(&file, CArchive::store | CArchive::bNoFlushOnDelete);
    saveArchive.m_pDocument = this;
    saveArchive.m_bForceFlat = FALSE;
    TRY
    {
        // Make sure we start at the start of the file
        saveArchive.Flush();
        file.Seek(0, CFile::begin);
        Serialize(saveArchive);     // save me
        saveArchive.Close();
    }
    CATCH_ALL(e)
    {
        TRY
            ReportSaveLoadException(pszFileName, e,
                TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
        END_TRY
        return FALSE;
    }
    END_CATCH_ALL
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::RestoreWindowState()
{
    if (m_pWinState == NULL)
        return;

    // If a window state payload was delivered to us during deserialize,
    // attempt to restore all the windows to their former glory.

    m_pWinState->SetDocument(this);
    m_pWinState->RestoreStateOfDocumentFrames();
    DiscardWindowState();                           // Discard used data
}

void CGamDoc::DiscardWindowState()
{
    if (m_pWinState != NULL)
        delete m_pWinState;
    m_pWinState = NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SetThisDocumentType()
{
    CDocTemplate *pDocTmpl = GetDocTemplate();
    CString str;
    VERIFY(pDocTmpl->GetDocString(str, CDocTemplate::filterExt));
    m_bScenario = str.CompareNoCase(".gsn") == 0;
}

///////////////////////////////////////////////////////////////////////
// The Scenario ID is a combination of a time based random
// number and the lower WORD of the amount of millisecs that
// have transpired since Windows was loaded on this machine.

DWORD CGamDoc::IssueScenarioID()
{
    return (DWORD)MAKELONG(GetTimeBasedRandomNumber(FALSE),
        (WORD)GetTickCount());
}

/////////////////////////////////////////////////////////////////////////////

const CTileManager* CGamDoc::GetTileManager() const
{
    if (m_pGbx == NULL) return NULL;
    return m_pGbx->GetTileManager();
}

CMarkManager* CGamDoc::GetMarkManager()
{
    if (m_pGbx == NULL) return NULL;
    return m_pGbx->GetMarkManager();
}

const CBoardManager* CGamDoc::GetBoardManager() const
{
    if (m_pGbx == NULL) return NULL;
    return m_pGbx->GetBoardManager();
}

const CPieceManager* CGamDoc::GetPieceManager() const
{
    if (m_pGbx == NULL) return NULL;
    return m_pGbx->GetPieceManager();
}

////////////////////////////////////////////////////////////////////////

CTileFacingMap* CGamDoc::GetFacingMap()
{
    ASSERT(m_pGbx != NULL);
    ASSERT(m_pGbx->GetTileManager());
    if (m_pTileFacingMap != NULL)
        return m_pTileFacingMap;
    else
    {
        m_pTileFacingMap = new CTileFacingMap(GetTileManager());
        return m_pTileFacingMap;
    }
}

////////////////////////////////////////////////////////////////////////

void CGamDoc::CloseTrayPalettes()
{
    if (m_bTrayAVisible)
        OnViewTrayA();          // Toggle it off
    if (m_bTrayBVisible)
        OnViewTrayB();          // Toggle it off
}

////////////////////////////////////////////////////////////////////////

void CGamDoc::DoBoardProperties(size_t nBrd)
{
    CPlayBoard& pPBoard = GetPBoardManager()->GetPBoard(nBrd);
    DoBoardProperties(pPBoard);
}

void CGamDoc::DoBoardProperties(CPlayBoard& pPBoard)
{
    CPBrdPropDialog dlg;

    dlg.m_bGridSnap = pPBoard.m_bGridSnap;
    dlg.m_bGridRectCenters = pPBoard.m_bGridRectCenters;
    dlg.m_xGridSnap = pPBoard.m_xGridSnap;
    dlg.m_yGridSnap = pPBoard.m_yGridSnap;
    dlg.m_xGridSnapOff = pPBoard.m_xGridSnapOff;
    dlg.m_yGridSnapOff = pPBoard.m_yGridSnapOff;
    dlg.m_bSnapMovePlot = pPBoard.m_bSnapMovePlot;
    dlg.m_bSmallCellBorders = pPBoard.m_bSmallCellBorders;
    dlg.m_bCellBorders = pPBoard.m_bCellBorders;
    dlg.m_bOpenBoardOnLoad = pPBoard.m_bOpenBoardOnLoad;
    dlg.m_bShowSelListAndTinyMap = pPBoard.m_bShowSelListAndTinyMap;
    dlg.m_xStackStagger = pPBoard.m_xStackStagger;
    dlg.m_yStackStagger = pPBoard.m_yStackStagger;
    dlg.m_crPlotColor = pPBoard.m_crPlotLineColor;
    dlg.m_nPlotWd = pPBoard.m_nPlotLineWidth;
    dlg.m_strBoardName = pPBoard.GetBoard()->GetName();
    dlg.m_pPlayerMgr = GetPlayerManager();
    dlg.m_nOwnerSel = CPlayerManager::GetPlayerNumFromMask(pPBoard.GetOwnerMask());
    dlg.m_bOwnerInfoIsReadOnly = !IsScenario();
    dlg.m_bNonOwnerAccess = pPBoard.IsNonOwnerAccessAllowed();
    dlg.m_bPrivate = pPBoard.IsPrivate();
    dlg.m_bDrawLockedBeneath = pPBoard.GetDrawLockedBeneath();

    if (dlg.DoModal() == IDOK)
    {
        pPBoard.m_bGridSnap = dlg.m_bGridSnap;
        pPBoard.m_bGridRectCenters = dlg.m_bGridRectCenters;
        pPBoard.m_xGridSnap = dlg.m_xGridSnap;
        pPBoard.m_yGridSnap= dlg.m_yGridSnap;
        pPBoard.m_xGridSnapOff = dlg.m_xGridSnapOff;
        pPBoard.m_yGridSnapOff = dlg.m_yGridSnapOff;
        pPBoard.m_bSnapMovePlot = dlg.m_bSnapMovePlot;
        pPBoard.m_bSmallCellBorders = dlg.m_bSmallCellBorders;
        pPBoard.m_bCellBorders = dlg.m_bCellBorders;
        pPBoard.m_bOpenBoardOnLoad = dlg.m_bOpenBoardOnLoad;
        pPBoard.m_bShowSelListAndTinyMap = dlg.m_bShowSelListAndTinyMap;
        pPBoard.m_xStackStagger = dlg.m_xStackStagger;
        pPBoard.m_yStackStagger = dlg.m_yStackStagger;
        pPBoard.m_crPlotLineColor = dlg.m_crPlotColor;
        pPBoard.m_nPlotLineWidth = dlg.m_nPlotWd;
        pPBoard.SetDrawLockedBeneath(dlg.m_bDrawLockedBeneath);

        if (dlg.m_pPlayerMgr && !dlg.m_bOwnerInfoIsReadOnly)
        {
            pPBoard.SetOwnerMask(CPlayerManager::GetMaskFromPlayerNum(dlg.m_nOwnerSel));
            pPBoard.PropagateOwnerMaskToAllPieces();
            pPBoard.SetNonOwnerAccess(dlg.m_bNonOwnerAccess);
            pPBoard.SetPrivate(dlg.m_bPrivate);
        }

        UpdateAllViews(NULL, HINT_BOARDCHANGE);
        UpdateAllViews(NULL, HINT_ALWAYSUPDATE);    // Repaint boards
        SetModifiedFlag();
    }
}

////////////////////////////////////////////////////////////////////////
// CGamDoc commands

void CGamDoc::OnUpdateViewTrayA(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bTrayAVisible);
}

void CGamDoc::OnUpdateViewTrayB(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bTrayBVisible);
}

void CGamDoc::OnUpdateViewMarkPalette(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bMarkPalVisible);
}

void CGamDoc::OnViewTrayA()
{
    m_bTrayAVisible = !m_bTrayAVisible;
}

void CGamDoc::OnViewTrayB()
{
    m_bTrayBVisible = !m_bTrayBVisible;
}

void CGamDoc::OnViewMarkPalette()
{
    m_bMarkPalVisible = !m_bMarkPalVisible;
}

///////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CGamDoc::OnDebugMoveList()
{
    CMoveList* pMoveList = NULL;
    if (m_pHistMoves != NULL)
        pMoveList = m_pHistMoves.get();
    else if (m_pRcdMoves != NULL)
        pMoveList = m_pRcdMoves.get();
    else
    {
        AfxMessageBox("No move list exists!");
        return;
    }
    CFileDialog dlg(FALSE, "txt", "movedump.txt",
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||", NULL, 0);
    if (dlg.DoModal() == IDOK)
    {
        CFile file;
        if (file.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
        {
            pMoveList->DumpToTextFile(*this, file);
            file.Close();
        }
    }
}

void CGamDoc::OnDebugPieceTable()
{
    ASSERT(m_pPTbl != NULL);
    if (m_pPTbl == NULL)
        return;
    CFileDialog dlg(FALSE, "txt", "piecedump.txt",
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||", NULL, 0);
    if (dlg.DoModal() == IDOK)
    {
        CFile file;
        if (file.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
        {
            m_pPTbl->DumpToTextFile(file);
            file.Close();
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////

void CGamDoc::OnEditSetBookMark()
{
    if (m_pBookMark != NULL)
    {
        // Need WARNING MESSAGE regarding deleting mark
        delete m_pBookMark;
    }
    m_pBookMark = new CGameState();
    if (!m_pBookMark->SaveState(*this))
    {
        // Memory low warning....
        delete m_pBookMark;
        m_pBookMark = NULL;
        return;
    }
    m_nMoveIdxAtBookMark = m_pRcdMoves != NULL ? m_pRcdMoves->GetCount() : size_t(0);
}

void CGamDoc::OnUpdateEditSetBookMark(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsPlaying() && !IsScenario());
}

void CGamDoc::OnEditRestoreBookMark()
{
    if (m_pBookMark != NULL)
    {
        SetLoadingVersion(NumVersion(fileGamVerMajor, fileGamVerMinor));
        SetFileFeatures(GetCBFeatures());
        if (!m_pBookMark->RestoreState(*this))
        {
            // Add memory error message
            return;
        }
        // TODO: Should set up a REDO index. For now just discard
        // TODO: all moves after where this bookmark was created.
        // TODO: I'll fix this when UNDO is added.
        if (m_pRcdMoves != NULL)
            m_pRcdMoves->PurgeAfter(m_nMoveIdxAtBookMark);
        UpdateAllViews(NULL, HINT_GAMESTATEUSED);
    }
}

void CGamDoc::OnUpdateEditRestoreBookMark(CCmdUI* pCmdUI)
{
    if (IsPlaying() || IsScenario())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_pBookMark != NULL);
}

void CGamDoc::OnEditClearBookMark()
{
    if (m_pBookMark != NULL)
    {
        delete m_pBookMark;
        m_pBookMark = NULL;
    }
}

void CGamDoc::OnUpdateEditClearBookMark(CCmdUI* pCmdUI)
{
    if (IsPlaying() || IsScenario())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_pBookMark != NULL);
}

void CGamDoc::OnPbckDiscard()
{
    if (!IsPlayingMoves()) return;
    if (m_nMoveInterlock) return;       // Not reentrant!
    FlushAllIndicators();
    DiscardCurrentRecording(TRUE);
}

void CGamDoc::OnUpdatePbckDiscard(CCmdUI* pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(IsPlayingMoves());
}

void CGamDoc::OnPbckEnd()
{
    if (!IsPlaying()) return;
    if (m_nMoveInterlock) return;       // No reentrant calls allowed
    m_nMoveInterlock++;
    m_bQuietPlayback = TRUE;
    FlushAllIndicators();
    while ((m_nCurMove = m_pMoves->DoMove(*this, m_nCurMove)) != Invalid_v<size_t>)
    {
        FlushAllIndicators();
    }
    m_nMoveInterlock--;
    m_bQuietPlayback = FALSE;
    UpdateAllViews(NULL, HINT_GAMESTATEUSED);
}

void CGamDoc::OnUpdatePbckEnd(CCmdUI* pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(IsPlaying() && m_nCurMove != Invalid_v<size_t>);
}

void CGamDoc::OnPbckFinish()
{
    DoAcceptPlayback();
}

void CGamDoc::OnUpdatePbckFinish(CCmdUI* pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(IsPlayingMoves());
}

void CGamDoc::OnPbckNext()
{
    if (!IsPlaying()) return;
    if (!m_nMoveInterlock)                   // No reentrant calls allowed
    {
        m_nMoveInterlock++;
        FlushAllIndicators();
        if (m_nCurMove != Invalid_v<size_t>)
        {
            if (m_bAutoStep)
                m_bAutoPlayback = TRUE;     // Will be cleared during DoMove if aborted

            m_nCurMove = m_pMoves->DoMove(*this, m_nCurMove);

            if (m_bAutoStep && m_bAutoPlayback)
            {
                if (m_nCurMove != Invalid_v<size_t> ||
                    (m_bStepToNextHist && IsPlayingHistory() && !IsPlayingLastHistory()))
                {
                    GetMainFrame()->PostMessage(WM_COMMAND, MAKEWPARAM(uint16_t(ID_PBCK_NEXT), uint16_t(0)));
                }
                else
                    m_bAutoPlayback = FALSE;    // Make sure FALSE in case auto step turned off
            }
            else
                m_bAutoPlayback = FALSE;    // Make sure FALSE in case auto step turned off
        }
        else if (m_bStepToNextHist && IsPlayingHistory() && !IsPlayingLastHistory())
        {
            // Force switch to next history record.
            OnPbckNextHistory();
            // Queue up the next move command
            GetMainFrame()->PostMessage(WM_COMMAND, MAKEWPARAM(uint16_t(ID_PBCK_NEXT), uint16_t(0)));
        }

        m_nMoveInterlock--;
    }
    else
    {
        if (m_bAutoPlayback)
            m_bAutoPlayback = FALSE;        // Will stop auto step when above DoMove returns
        else
            m_pMoves->IncrementSkipCount(m_bKeepSkipInd);// Force skip of current move
    }
}

void CGamDoc::OnUpdatePbckNext(CCmdUI* pCmdUI)
{
    if (IsScenario())
        pCmdUI->Enable(FALSE);
    else
    {
        pCmdUI->Enable((IsPlaying() && m_nCurMove != Invalid_v<size_t>) ||
            (m_bStepToNextHist && IsPlayingHistory() && !IsPlayingLastHistory()));
        pCmdUI->SetCheck(m_bAutoPlayback);
    }
}

void CGamDoc::OnPbckStart()
{
    if (!IsPlaying()) return;
    if (m_nMoveInterlock) return;       // Not reentrant!
    if (m_pMoves->IsDoMoveActive())
        return;                         // Must ignore since moves are still being played back

    MsgDialogCancel(TRUE);
    FlushAllIndicators();
    RestartMoves();
}

void CGamDoc::OnUpdatePbckStart(CCmdUI* pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI->Enable(FALSE);
    else
    {
        pCmdUI->Enable(IsPlaying() && ((m_nCurMove - size_t(1)) > m_nFirstMove ||
            (m_nCurMove == Invalid_v<size_t> &&
             m_pMoves->IsThisMovePossible(m_nFirstMove + size_t(1)))));
    }
}

void CGamDoc::OnPbckPrevious()
{
    if (!IsPlaying()) return;
    if (m_nMoveInterlock) return;       // Not reentrant!
    if (m_pMoves->IsDoMoveActive())
        return;                         // Must ignore since moves are still being played back

    m_nMoveInterlock++;
    m_bQuietPlayback = TRUE;

    size_t nPrvMove = m_pMoves->FindPreviousMove(*this, m_nCurMove);

    MsgDialogCancel(TRUE);
    FlushAllIndicators();
    RestartMoves();

    if (nPrvMove < m_nCurMove)
        nPrvMove = m_nCurMove;

    if (m_nCurMove < nPrvMove)      // Check if not already at start of moves
    {
        // OK...we know there is at least one move to do PRIOR to the move
        // we are stepping up to. Find it so we can turn off silent mode for
        // that move.
        size_t nPrvPrvMove = m_pMoves->FindPreviousMove(*this, nPrvMove);
        if (m_nCurMove == nPrvPrvMove)
        {
            m_bQuietPlayback = FALSE;
            UpdateAllViews(NULL, HINT_GAMESTATEUSED); // Sync up the images
        }

        // Clear out any strings we may have accumulated during the
        // search for the final visible move index.
        m_astrMsgHist.RemoveAll();

        while ((m_nCurMove = m_pMoves->DoMove(*this, m_nCurMove)) < nPrvMove &&
            m_nCurMove != Invalid_v<size_t>)
        {
            if (m_nCurMove < nPrvPrvMove)
                FlushAllIndicators();
            if (m_nCurMove == nPrvPrvMove)
            {
                m_bQuietPlayback = FALSE;   // Show last move
                UpdateAllViews(NULL, HINT_GAMESTATEUSED); // Sync up the images
            }
        }
    }
    else
        UpdateAllViews(NULL, HINT_GAMESTATEUSED);

    ASSERT(m_nCurMove == nPrvMove);
    m_nMoveInterlock--;
    m_bQuietPlayback = FALSE;
}

void CGamDoc::OnUpdatePbckPrevious(CCmdUI* pCmdUI)
{
    if (IsScenario() ||  m_bAutoPlayback)
        pCmdUI->Enable(FALSE);
    else
    {
        pCmdUI->Enable(IsPlaying() && ((m_nCurMove - size_t(1)) > m_nFirstMove ||
            (m_nCurMove == Invalid_v<size_t> &&
             m_pMoves->IsThisMovePossible(m_nFirstMove + size_t(1)))));
    }
}

void CGamDoc::OnPbckStepToNextHist()
{
    m_bStepToNextHist = !m_bStepToNextHist;
}

void CGamDoc::OnUpdatePbckStepToNextHist(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsScenario() && IsPlaying());
    pCmdUI->SetCheck(m_bStepToNextHist);
}

void CGamDoc::OnPbckSkipKeepIndicators()
{
    m_bKeepSkipInd = !m_bKeepSkipInd;
}

void CGamDoc::OnUpdatePbckSkipKeepIndicators(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsScenario() && IsPlaying());
    pCmdUI->SetCheck(m_bKeepSkipInd);
}

void CGamDoc::OnPbckAutoStep()
{
    m_bAutoStep = !m_bAutoStep;
}

void CGamDoc::OnUpdatePbckAutoStep(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsScenario() && IsPlaying());
    pCmdUI->SetCheck(m_bAutoStep);
}

void CGamDoc::OnActDoMessage()
{
    if (!IsPlaying())
        MsgSendDialogOpen();
}

void CGamDoc::OnUpdateActDoMessage(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsPlaying() && !IsScenario());
}

void CGamDoc::OnActRollDice()
{
    if (!IsPlaying())
        MsgSendDialogOpen(TRUE);
}

void CGamDoc::OnUpdateActRollDice(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsPlaying() && !IsScenario());
}

void CGamDoc::OnPbckReadMessage()
{
    m_bMsgWinVisible = !m_bMsgWinVisible;
}

void CGamDoc::OnUpdatePbckReadMessage(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsScenario());
    pCmdUI->SetCheck(m_bMsgWinVisible);
}

void CGamDoc::OnPbckNextHistory()
{
    ASSERT(IsPlayingHistory());
    if (m_pMoves->IsDoMoveActive())
        return;                         // Must ignore since moves are still being played back

    size_t nCurHist = m_nCurHist;
    FinishHistoryPlayback();
    LoadAndActivateHistory(nCurHist + 1);
    UpdateAllViews(NULL, HINT_GAMESTATEUSED);
}

void CGamDoc::OnUpdatePbckNextHistory(CCmdUI* pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(IsPlayingHistory() &&
            !IsPlayingLastHistory());
}

void CGamDoc::OnPbckCloseHistory()
{
    ASSERT(IsPlayingHistory());
    if (!IsPlayingHistory()) return;
    if (m_nMoveInterlock) return;       // We have to let it finish up.
    if (m_pMoves->IsDoMoveActive())
        return;                         // Must ignore since moves are still being played back

    FinishHistoryPlayback();
    UpdateAllViews(NULL, HINT_GAMESTATEUSED);
}

void CGamDoc::OnUpdatePbckCloseHistory(CCmdUI* pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(IsPlayingHistory());
}

void CGamDoc::OnFileSendRecording2File()
{
    if (IsPlayingHistory())
    {
        AfxMessageBox(IDS_ERR_NOSAVEWHENPLAY, MB_OK | MB_ICONINFORMATION);
        return;
    }
    SaveRecordedMoves();
}

void CGamDoc::OnUpdateFileSendRecording2File(CCmdUI* pCmdUI)
{
    if (IsScenario())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(IsAnyRecorded());
}

void CGamDoc::OnFileDiscardRecordedMoves()
{
    if (IsPlayingHistory())
    {
        AfxMessageBox(IDS_ERR_NODISCARDWHENPLAY, MB_OK | MB_ICONINFORMATION);
        return;
    }
    DiscardCurrentRecording(TRUE);
}

void CGamDoc::OnUpdateFileDiscardRecordedMoves(CCmdUI* pCmdUI)
{
    if (IsScenario())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(IsAnyRecorded());
}

void CGamDoc::OnEditCreateTray()
{
    ASSERT(IsScenario());
    CTrayNewDialog dlg;
    dlg.m_pYMgr =GetTrayManager();
    if (dlg.DoModal() == IDOK)
    {
        dlg.m_pYMgr->CreateTraySet(dlg.m_strName);

        CGamDocHint hint;
        hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = NULL;
        UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
        SetModifiedFlag();
    }
}

void CGamDoc::OnUpdateEditCreateTray(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsScenario());
}

void CGamDoc::OnEditScenarioProperties()
{
    ASSERT(IsScenario());
    CScnPropDialog dlg;

    dlg.m_strAuthor = m_strScnAuthor;
    dlg.m_strTitle = m_strScnTitle;
    dlg.m_strDescr = m_strScnDescr;

    if (dlg.DoModal() == IDOK)
    {
        m_strScnAuthor = dlg.m_strAuthor;
        m_strScnTitle  = dlg.m_strTitle;
        m_strScnDescr  = dlg.m_strDescr;
        UpdateAllViews(NULL, HINT_GSNPROPCHANGE);
        SetModifiedFlag();
    }
}

void CGamDoc::OnUpdateEditScenarioProperties(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsScenario());
}

void CGamDoc::OnEditSelectBoards()
{
    ASSERT(IsScenario());

    CPBoardManager* pPBMgr = GetPBoardManager();
    ASSERT(pPBMgr);

    CSelectBoardsDialog dlg;

    dlg.m_pBMgr = pPBMgr->GetBoardManager();
    pPBMgr->GetPBoardList(dlg.m_tblBrds);

    if (dlg.DoModal() == IDOK)
    {
        // First close all the views of boards that are going
        // to be removed from the play list.
        std::vector<CB::not_null<CPlayBoard*>> tblNotInList;
        pPBMgr->FindPBoardsNotInList(dlg.m_tblBrds, tblNotInList);
        for (size_t i = 0; i < tblNotInList.size(); i++)
        {
            CView* pView = FindPBoardView(*tblNotInList.at(i));
            if (pView != NULL)
            {
                CFrameWnd* pFrame = pView->GetParentFrame();
                ASSERT(pFrame != NULL);
                pFrame->SendMessage(WM_CLOSE);
            }
        }

        // Then change the play list.
        pPBMgr->SetPBoardList(dlg.m_tblBrds);
        UpdateAllViews(NULL, HINT_BOARDCHANGE);
        SetModifiedFlag();
    }
}

void CGamDoc::OnUpdateEditSelectBoards(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsScenario());
}

void CGamDoc::OnEditImportPieceGroups()
{
    ASSERT(IsScenario());

    CImportTraysDlg dlg;
    dlg.m_pDoc = this;

    if (dlg.DoModal() == IDOK)
    {
        CGamDocHint hint;
        hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = NULL;
        UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
        SetModifiedFlag();
    }
}

void CGamDoc::OnUpdateEditImportPieceGroups(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsScenario());
}

void CGamDoc::OnEditSelectGamePieces()
{
    ASSERT(IsScenario() && GetTrayManager()->GetNumTraySets() > 0);
    CTrayManager* pYMgr = GetTrayManager();

    CSetPiecesDialog dlg(*this);
    dlg.m_nYSel = 0;                    // Default is first tray

    CloseTrayPalettes();                // ...Ditto that for tray palettes

    dlg.DoModal();

    // Notify all visible trays
    CGamDocHint hint;
    hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = NULL;
    UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
    SetModifiedFlag();
}

void CGamDoc::OnUpdateEditSelectGamePieces(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsScenario() && GetTrayManager()->GetNumTraySets() > 0);
}

void CGamDoc::OnFileLoadMoveFile()
{
    if (IsPlaying())
    {
        AfxMessageBox(IDS_ERR_NOLOADWHENPLAY, MB_OK | MB_ICONINFORMATION);
        return;
    }

    CString strFilter;
    strFilter.LoadString(IDS_GMOV_FILTER);
    CString strTitle;
    strTitle.LoadString(IDS_GMOV_ENTERNAME);

    CFileDialog dlg(TRUE, "gmv", NULL, OFN_HIDEREADONLY, strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return;

    if (!DiscardCurrentRecording(TRUE))
        return;
    LoadAndActivateMoveFile(dlg.GetPathName());
}

void CGamDoc::OnUpdateFileLoadMoveFile(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsScenario() && !IsPlaying());
}

///////////////////////////////////////////////////////////////////////

BOOL CGamDoc::IsRecordingCompoundMove()
{
    return IsRecording() && m_pRcdMoves != NULL &&
        m_pRcdMoves->IsRecordingCompoundMove();
}

void CGamDoc::OnActCompoundMoveBegin()
{
    RecordCompoundMoveBegin();
}

void CGamDoc::OnUpdateActCompoundMoveBegin(CCmdUI* pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION |
            (IsRecording() ?  MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
        pCmdUI->Enable(IsRecording());
    pCmdUI->SetCheck(IsRecordingCompoundMove());
}

void CGamDoc::OnActCompoundMoveEnd()
{
    RecordCompoundMoveEnd();
}

void CGamDoc::OnUpdateActCompoundMoveEnd(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsRecordingCompoundMove());
}

void CGamDoc::OnActCompoundMoveDiscard()
{
    if (AfxMessageBox(IDS_WARN_DISCARD_COMP_MOVE,
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
        RecordCompoundMoveDiscard();
}

void CGamDoc::OnUpdateActCompoundMoveDiscard(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsRecordingCompoundMove());
}

void CGamDoc::OnUpdateIndicatorCompoundMove(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsRecordingCompoundMove());
}

void CGamDoc::OnFileSaveGameAsScenario()
{
    ASSERT(!IsScenario());
    BOOL bModified = IsModified();          // Cache the modified flag

    // Save various variables...
    m_bScenario = TRUE;
    CString strPlayerFileDescr = m_strPlayerFileDescr;
    DWORD dwPlayerHash = m_dwPlayerHash;
    DWORD dwCurrentPlayer = m_dwCurrentPlayer;
    DWORD dwScenarioID = m_dwScenarioID;    // Cache scenario ID

    m_strPlayerFileDescr.Empty();
    m_dwPlayerHash = 0;
    m_dwCurrentPlayer = 0;
    m_dwScenarioID = IssueScenarioID();     // Create new scenario ID

    CString fileName;
    if (GetApp()->DoPromptFileName(fileName, IDS_SAVEGAMEASSCENARIO,
        OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE,
        GetApp()->GetScnenarioDocTemplate()))
    {
        TRY
        {
            FlushAllIndicators();
            OnSaveDocument(fileName);
        }
        END_TRY
    }

    // Restore saved variables.
    m_bScenario = FALSE;
    m_strPlayerFileDescr = strPlayerFileDescr;
    m_dwPlayerHash = dwPlayerHash;
    m_dwCurrentPlayer = dwCurrentPlayer;
    m_dwScenarioID = dwScenarioID;

    SetModifiedFlag(bModified);
}

void CGamDoc::OnUpdateFileSaveGameAsScenario(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!IsScenario() && (!HasPlayers() ||
        HasPlayers() && IsCurrentPlayerReferee()));
}

void CGamDoc::OnPbckStepCompoundMoves()
{
    if (!IsPlaying()) return;
    m_pMoves->SetCompoundSingleStep(!m_pMoves->IsSingleSteppingCompoundMoves());
}

void CGamDoc::OnUpdateStepCompoundMoves(CCmdUI* pCmdUI)
{
    BOOL bEnable = !IsScenario() && IsPlaying();
    if (bEnable && IsPlaying())
    {
        ASSERT(m_pMoves != NULL);
        pCmdUI->SetCheck(m_pMoves->IsSingleSteppingCompoundMoves());
    }
    else
        pCmdUI->SetCheck(FALSE);

    pCmdUI->Enable(bEnable);
}

void CGamDoc::OnViewShowTipText()
{
    m_bShowObjTipText = !m_bShowObjTipText;
}

void CGamDoc::OnUpdateViewShowTipText(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bShowObjTipText);
    pCmdUI->Enable(TRUE);
}

void CGamDoc::OnViewShowTipOwner()
{
    m_bDisableOwnerTips = !m_bDisableOwnerTips;
}

void CGamDoc::OnUpdateViewShowTipOwner(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(!m_bDisableOwnerTips);
    pCmdUI->Enable(TRUE);
}

void CGamDoc::OnViewSaveWinState()
{
    m_bSaveWindowPositions = !m_bSaveWindowPositions;
}

void CGamDoc::OnUpdateViewSaveWinState(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bSaveWindowPositions ? 1 : 0);
}

void CGamDoc::OnEditCreatePlayers()
{
    CCreatePlayersDialog dlg;
    dlg.m_nPlayerCount = value_preserving_cast<size_t>(m_pPlayerMgr != NULL ? m_pPlayerMgr->GetSize() : 0);
    if (dlg.DoModal() != IDOK)
        return;

    if (m_pPlayerMgr != NULL)
    {
        delete m_pPlayerMgr;
        m_pPlayerMgr = NULL;
    }
    ClearAllOwnership();            // Start with clean slate
    if (dlg.m_nPlayerCount > size_t(0))
    {
        m_pPlayerMgr = new CPlayerManager;
        char szBfr[2];
        szBfr[1] = 0;
        for (size_t i = size_t(0) ; i < dlg.m_nPlayerCount ; ++i)
        {
            szBfr[0] = value_preserving_cast<char>(size_t('A') + i);
            CString str;
            str.Format(IDS_BASE_PLAYER_NAME, szBfr);
            m_pPlayerMgr->AddPlayer(str);
        }
    }
    SetModifiedFlag();
    UpdateAllViews(NULL);
}

void CGamDoc::OnUpdateEditCreatePlayers(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsScenario());
}

void CGamDoc::OnEditEditPlayers()
{
    CEditPlayersDialog dlg;
    dlg.SetDialogsPlayerNames(m_pPlayerMgr);
    if (dlg.DoModal() != IDOK)
        return;
    dlg.GetPlayerNamesFromDialog(m_pPlayerMgr);
    SetModifiedFlag();
    UpdateAllViews(NULL);
}

void CGamDoc::OnUpdateEditEditPlayers(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsScenario() && m_pPlayerMgr != NULL &&
        m_pPlayerMgr->GetSize() > 0);
}

void CGamDoc::OnActSimulateSpectator()
{
    m_bSimulateSpectator = !m_bSimulateSpectator;
    UpdateAllViews(NULL, HINT_GAMESTATEUSED);       // So trays sync up
}

void CGamDoc::OnUpdateActSimulateSpectator(CCmdUI* pCmdUI)
{
    if (!IsScenario() && m_dwCurrentPlayer != 0)
    {
        pCmdUI->Enable(TRUE);
        pCmdUI->SetCheck(m_bSimulateSpectator ? 1 : 0);
    }
    else
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(0);
    }
}

void CGamDoc::OnEditCreateGeomorphic()
{
    CCreateGeomorphicBoardDialog dlg(*this);
    if (dlg.DoModal() != IDOK)
        return;
    OwnerPtr<CGeomorphicBoard> pGeoBoard = dlg.DetachGeomorphicBoard();

    GetPBoardManager()->AddBoard(std::move(pGeoBoard));     // Add to list of active boards

    UpdateAllViews(NULL, HINT_BOARDCHANGE);
    SetModifiedFlag();
}

void CGamDoc::OnUpdateEditCreateGeomorphic(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsScenario());
}

static BYTE szPassWord[16] =
{
    0x16, 0xd2, 0x19, 0xe1,
    0x64, 0x7a, 0xf2, 0x21,
    0xec, 0x9b, 0x43, 0x87,
    0xe4, 0x7d, 0xf6, 0x6d
};

void CGamDoc::OnFileCreateReferee()
{
    CPasswordDialog dlg;
    if (dlg.DoModal() != IDOK)
        return;
    MD5_CTX md5Context;
    MD5Calc(&md5Context, (BYTE*)(LPCTSTR)dlg.m_strPassword,
        dlg.m_strPassword.GetLength());
    if (memcmp(md5Context.digest, szPassWord, 16) != 0)
    {
        AfxMessageBox(IDS_ERR_INVALID_PASSWORD);
        return;
    }

    // Save so we can later restore.
    CString strPlayerFileDescr = m_strPlayerFileDescr;
    DWORD dwCurrentPlayer = m_dwCurrentPlayer;
    DWORD dwPlayerHash = m_dwPlayerHash;

    CString strReferee;
    strReferee.LoadString(IDS_GAME_REFEREE);

    // Rip apart current player string to get the raw stuff
    // we need to create a referee file.
    CString strBaseName = m_strPathName;
    int nPos = strBaseName.ReverseFind('-');
    if (nPos >= 0)
        strBaseName = strBaseName.Left(nPos);
    else
    {
        // Just remove the extension
        int nPos = strBaseName.ReverseFind('.');
        if (nPos >= 0)
            strBaseName = strBaseName.Left(nPos);
    }
    CString strGamInfo;
    nPos = m_strPlayerFileDescr.ReverseFind('@');
    if (nPos >= 0)
        strGamInfo = m_strPlayerFileDescr.Left(nPos);

    CString strFName = strBaseName + strReferee + "gam";

    if (_access(strFName, 0) != -1)
    {
        // File already exists. Prompt for overwrite.
        CString str;
        str.Format(IDS_WARN_REF_EXISTS, strFName.GetString());
        if (AfxMessageBox(str, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2) != IDOK)
            return;
    }

    m_strPlayerFileDescr = strGamInfo + "@" + strFName;// For hash check and calc
    m_dwCurrentPlayer = PLAYER_REFEREE_FLAG | OWNER_MASK_REFEREE;
    m_dwPlayerHash = CalculateHashForCurrentPlayerMask();

    BOOL bOK = DoSaveGameFile(strFName);

    // Restore variables...
    m_strPlayerFileDescr = strPlayerFileDescr;
    m_dwCurrentPlayer = dwCurrentPlayer;
    m_dwPlayerHash = dwPlayerHash;

    if (bOK)
    {
        CString str;
        str.Format(IDS_INFO_REF_CREATED, strFName.GetString());
        AfxMessageBox(str);
    }
}

void CGamDoc::OnUpdateFileCreateReferee(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(HasPlayers() && !IsCurrentPlayerReferee());
}

void CGamDoc::OnFileChangeGameOwner()
{
    CPasswordDialog dlg;
    if (dlg.DoModal() != IDOK)
        return;
    MD5_CTX md5Context;
    MD5Calc(&md5Context, (BYTE*)(LPCTSTR)dlg.m_strPassword,
        dlg.m_strPassword.GetLength());
    if (memcmp(md5Context.digest, szPassWord, 16) != 0)
    {
        AfxMessageBox(IDS_ERR_INVALID_PASSWORD);
        return;
    }

    CChgGameOwnerDialog dlg2;
    dlg2.m_pPlayerMgr = GetPlayerManager();
    if (dlg2.DoModal() != IDOK)
        return;

    if (dlg2.m_nPlayer == -1)
        return;                         // No player selected

    DWORD dwPlayerMask = CPlayerManager::GetMaskFromPlayerNum(dlg2.m_nPlayer);
    CString strPlayerName = m_pPlayerMgr->ElementAt(dlg2.m_nPlayer).m_strName;

    // Save so we can later restore.
    CString strPlayerFileDescr = m_strPlayerFileDescr;
    DWORD dwCurrentPlayer = m_dwCurrentPlayer;
    DWORD dwPlayerHash = m_dwPlayerHash;

    // Rip apart current player string to get the raw stuff
    // we need to create a referee file.
    CString strBaseName = m_strPathName;
    int nPos = strBaseName.ReverseFind('-');
    if (nPos >= 0)
        strBaseName = strBaseName.Left(nPos);
    else
    {
        // Just remove the extension
        int nPos = strBaseName.ReverseFind('.');
        if (nPos >= 0)
            strBaseName = strBaseName.Left(nPos);
    }
    CString strGamInfo;
    nPos = m_strPlayerFileDescr.ReverseFind('@');
    if (nPos >= 0)
        strGamInfo = m_strPlayerFileDescr.Left(nPos);

    CString strFName = strBaseName + "-" + strPlayerName + ".gam";

    if (_access(strFName, 0) != -1)
    {
        // File already exists. Prompt for overwrite.
        CString str;
        str.Format(IDS_WARN_REF_EXISTS, strFName.GetString());
        if (AfxMessageBox(str, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2) != IDOK)
            return;
    }

    m_strPlayerFileDescr = strGamInfo + "@" + strFName;// For hash check and calc
    m_dwCurrentPlayer = dwPlayerMask;
    m_dwPlayerHash = CalculateHashForCurrentPlayerMask();

    BOOL bOK = DoSaveGameFile(strFName);

    // Restore variables...
    m_strPlayerFileDescr = strPlayerFileDescr;
    m_dwCurrentPlayer = dwCurrentPlayer;
    m_dwPlayerHash = dwPlayerHash;

    if (bOK)
    {
        CString str;
        str.Format(IDS_INFO_GAME_CREATED, strFName.GetString());
        AfxMessageBox(str);
    }
}

void CGamDoc::OnUpdateFileChangeGameOwner(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(HasPlayers() && !IsCurrentPlayerReferee());
}

