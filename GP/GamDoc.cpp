// GamDoc.cpp
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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

#include    "FrmDockMark.h"
#include    "PalMark.h"
#include    "FrmDockTray.h"
#include    "PalTray.h"
#include    "PalReadMsg.h"
#include    "VwPrjgam.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

wxIMPLEMENT_DYNAMIC_CLASS(CGamDoc, wxDocument);
IMPLEMENT_DYNAMIC(CGamDocMfc, CDocument)

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

wxBEGIN_EVENT_TABLE(CGamDoc, wxDocument)
    EVT_UPDATE_UI(XRCID("ID_VIEW_TRAYA"), OnUpdateViewTrayA)
    EVT_UPDATE_UI(XRCID("ID_VIEW_TRAYB"), OnUpdateViewTrayB)
    EVT_UPDATE_UI(XRCID("ID_VIEW_MARKERPAL"), OnUpdateViewMarkPalette)
    EVT_MENU(XRCID("ID_VIEW_TRAYA"), OnViewTrayA)
    EVT_MENU(XRCID("ID_VIEW_TRAYB"), OnViewTrayB)
    EVT_UPDATE_UI(XRCID("ID_INDICATOR_COMPMOVE"), OnUpdateIndicatorCompoundMove)
    EVT_MENU(XRCID("ID_VIEW_MARKERPAL"), OnViewMarkPalette)
#if 0
    ON_COMMAND(ID_EDIT_SETBOOKMARK, OnEditSetBookMark)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SETBOOKMARK, OnUpdateEditSetBookMark)
    ON_COMMAND(ID_EDIT_RESTOREBOOKMARK, OnEditRestoreBookMark)
    ON_UPDATE_COMMAND_UI(ID_EDIT_RESTOREBOOKMARK, OnUpdateEditRestoreBookMark)
    ON_COMMAND(ID_EDIT_CLRBOOKMARK, OnEditClearBookMark)
#endif
    EVT_MENU(XRCID("ID_PBCK_DISCARD"), OnPbckDiscard)
    EVT_UPDATE_UI(XRCID("ID_PBCK_DISCARD"), OnUpdatePbckDiscard)
    EVT_MENU(XRCID("ID_PBCK_END"), OnPbckEnd)
    EVT_UPDATE_UI(XRCID("ID_PBCK_END"), OnUpdatePbckEnd)
    EVT_MENU(XRCID("ID_PBCK_FINISH"), OnPbckFinish)
    EVT_UPDATE_UI(XRCID("ID_PBCK_FINISH"), OnUpdatePbckFinish)
    EVT_MENU(XRCID("ID_PBCK_NEXT"), OnPbckNext)
    EVT_UPDATE_UI(XRCID("ID_PBCK_NEXT"), OnUpdatePbckNext)
    EVT_MENU(XRCID("ID_PBCK_START"), OnPbckStart)
    EVT_UPDATE_UI(XRCID("ID_PBCK_START"), OnUpdatePbckStart)
    EVT_MENU(XRCID("ID_ACT_DOMESSAGE"), OnActDoMessage)
#if 0
    ON_UPDATE_COMMAND_UI(ID_EDIT_CLRBOOKMARK, OnUpdateEditClearBookMark)
#endif
    EVT_UPDATE_UI(XRCID("ID_ACT_DOMESSAGE"), OnUpdateActDoMessage)
    EVT_MENU(XRCID("ID_PBCK_READMESSAGE"), OnPbckReadMessage)
    EVT_UPDATE_UI(XRCID("ID_PBCK_READMESSAGE"), OnUpdatePbckReadMessage)
    EVT_MENU(XRCID("ID_PBCK_NEXTHIST"), OnPbckNextHistory)
    EVT_UPDATE_UI(XRCID("ID_PBCK_NEXTHIST"), OnUpdatePbckNextHistory)
    EVT_MENU(XRCID("ID_PBCK_CLOSEHIST"), OnPbckCloseHistory)
    EVT_UPDATE_UI(XRCID("ID_PBCK_CLOSEHIST"), OnUpdatePbckCloseHistory)
    EVT_MENU(XRCID("ID_FILE_SENDMOVES2FILE"), OnFileSendRecording2File)
    EVT_UPDATE_UI(XRCID("ID_FILE_SENDMOVES2FILE"), OnUpdateFileSendRecording2File)
    EVT_MENU(XRCID("ID_FILE_DISCARDRECORDING"), OnFileDiscardRecordedMoves)
    EVT_UPDATE_UI(XRCID("ID_FILE_DISCARDRECORDING"), OnUpdateFileDiscardRecordedMoves)
    EVT_MENU(XRCID("ID_EDIT_CREATETRAY"), OnEditCreateTray)
    EVT_UPDATE_UI(XRCID("ID_EDIT_CREATETRAY"), OnUpdateEditCreateTray)
    EVT_MENU(XRCID("ID_EDIT_SCNPROPERTIES"), OnEditScenarioProperties)
    EVT_UPDATE_UI(XRCID("ID_EDIT_SCNPROPERTIES"), OnUpdateEditScenarioProperties)
    EVT_MENU(XRCID("ID_EDIT_SELECTBOARDS"), OnEditSelectBoards)
    EVT_UPDATE_UI(XRCID("ID_EDIT_SELECTBOARDS"), OnUpdateEditSelectBoards)
    EVT_MENU(XRCID("ID_EDIT_SELECTGAMEPIECES"), OnEditSelectGamePieces)
    EVT_UPDATE_UI(XRCID("ID_EDIT_SELECTGAMEPIECES"), OnUpdateEditSelectGamePieces)
    EVT_MENU(XRCID("ID_FILE_LOADMOVES"), OnFileLoadMoveFile)
    EVT_UPDATE_UI(XRCID("ID_FILE_LOADMOVES"), OnUpdateFileLoadMoveFile)
    EVT_MENU(XRCID("ID_ACT_COMPOUNDMOVE_BEGIN"), OnActCompoundMoveBegin)
    EVT_UPDATE_UI(XRCID("ID_ACT_COMPOUNDMOVE_BEGIN"), OnUpdateActCompoundMoveBegin)
    EVT_MENU(XRCID("ID_ACT_COMPOUNDMOVE_END"), OnActCompoundMoveEnd)
    EVT_UPDATE_UI(XRCID("ID_ACT_COMPOUNDMOVE_END"), OnUpdateActCompoundMoveEnd)
    EVT_MENU(XRCID("ID_ACT_COMPOUNDMOVE_DISCARD"), OnActCompoundMoveDiscard)
    EVT_UPDATE_UI(XRCID("ID_ACT_COMPOUNDMOVE_DISCARD"), OnUpdateActCompoundMoveDiscard)
    EVT_MENU(XRCID("ID_PBCK_PREVIOUS"), OnPbckPrevious)
    EVT_UPDATE_UI(XRCID("ID_PBCK_PREVIOUS"), OnUpdatePbckPrevious)
    EVT_MENU(XRCID("ID_FILE_SAVE_GAME_AS_SCENARIO"), OnFileSaveGameAsScenario)
    EVT_UPDATE_UI(XRCID("ID_FILE_SAVE_GAME_AS_SCENARIO"), OnUpdateFileSaveGameAsScenario)
    EVT_MENU(XRCID("ID_ACTIONS_ROLLDICE"), OnActRollDice)
    EVT_UPDATE_UI(XRCID("ID_ACTIONS_ROLLDICE"), OnUpdateActRollDice)
    EVT_MENU(XRCID("ID_PBCK_STEP_CMOVES"), OnPbckStepCompoundMoves)
    EVT_UPDATE_UI(XRCID("ID_PBCK_STEP_CMOVES"), OnUpdateStepCompoundMoves)
    EVT_MENU(XRCID("ID_EDIT_IMPORTPCEGROUPS"), OnEditImportPieceGroups)
    EVT_UPDATE_UI(XRCID("ID_EDIT_IMPORTPCEGROUPS"), OnUpdateEditImportPieceGroups)
    EVT_MENU(XRCID("ID_VIEW_SHOW_TIP_TEXT"), OnViewShowTipText)
    EVT_UPDATE_UI(XRCID("ID_VIEW_SHOW_TIP_TEXT"), OnUpdateViewShowTipText)
    EVT_MENU(XRCID("ID_VIEW_SAVE_WIN_STATE"), OnViewSaveWinState)
    EVT_UPDATE_UI(XRCID("ID_VIEW_SAVE_WIN_STATE"), OnUpdateViewSaveWinState)
    EVT_MENU(XRCID("ID_EDIT_CREATE_PLAYERS"), OnEditCreatePlayers)
    EVT_UPDATE_UI(XRCID("ID_EDIT_CREATE_PLAYERS"), OnUpdateEditCreatePlayers)
    EVT_MENU(XRCID("ID_EDIT_EDIT_PLAYERS"), OnEditEditPlayers)
    EVT_UPDATE_UI(XRCID("ID_EDIT_EDIT_PLAYERS"), OnUpdateEditEditPlayers)
    EVT_MENU(XRCID("ID_ACT_SIMULATE_SPECTATOR"), OnActSimulateSpectator)
    EVT_UPDATE_UI(XRCID("ID_ACT_SIMULATE_SPECTATOR"), OnUpdateActSimulateSpectator)
    EVT_MENU(XRCID("ID_EDIT_CREATE_GEOMORPHIC"), OnEditCreateGeomorphic)
    EVT_UPDATE_UI(XRCID("ID_EDIT_CREATE_GEOMORPHIC"), OnUpdateEditCreateGeomorphic)
    EVT_MENU(XRCID("ID_FILE_CREATE_REFEREE"), OnFileCreateReferee)
    EVT_UPDATE_UI(XRCID("ID_FILE_CREATE_REFEREE"), OnUpdateFileCreateReferee)
    EVT_MENU(XRCID("ID_FILE_CHANGE_GAME_OWNER"), OnFileChangeGameOwner)
    EVT_UPDATE_UI(XRCID("ID_FILE_CHANGE_GAME_OWNER"), OnUpdateFileChangeGameOwner)
    EVT_MENU(XRCID("ID_PBCK_STEP_TO_NEXT_HIST"), OnPbckStepToNextHist)
    EVT_UPDATE_UI(XRCID("ID_PBCK_STEP_TO_NEXT_HIST"), OnUpdatePbckStepToNextHist)
    EVT_MENU(XRCID("ID_PBCK_SKIP_KEEP_IND"), OnPbckSkipKeepIndicators)
    EVT_UPDATE_UI(XRCID("ID_PBCK_SKIP_KEEP_IND"), OnUpdatePbckSkipKeepIndicators)
    EVT_MENU(XRCID("ID_PBCK_AUTO_STEP"), OnPbckAutoStep)
    EVT_UPDATE_UI(XRCID("ID_PBCK_AUTO_STEP"), OnUpdatePbckAutoStep)
    EVT_MENU(XRCID("ID_VIEW_SHOW_TIP_OWNER"), OnViewShowTipOwner)
    EVT_UPDATE_UI(XRCID("ID_VIEW_SHOW_TIP_OWNER"), OnUpdateViewShowTipOwner)
#ifdef _DEBUG
    EVT_MENU(XRCID("ID_DEBUG_MOVELIST"), OnDebugMoveList)
    EVT_UPDATE_UI(XRCID("ID_DEBUG_MOVELIST"), OnUpdateEnable)
    EVT_MENU(XRCID("ID_DEBUG_PIECETABLE"), OnDebugPieceTable)
    EVT_UPDATE_UI(XRCID("ID_DEBUG_PIECETABLE"), OnUpdateEnable)
#endif
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////
// CGamDoc construction/destruction

CGamDoc::CGamDoc() :
    mfcDoc(new CGamDocMfc(*this))
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
    m_dwCurrentPlayer = OWNER_MASK_SPECTATOR;
    m_dwPlayerHash = 0;

    m_bSimulateSpectator = FALSE;
}

CGamDoc::~CGamDoc()
{
    DeleteContents();
}

bool CGamDoc::OnNewDocument()
{
    if (!mfcDoc->CDocument::OnNewDocument())
        return FALSE;
    if (!wxDocument::OnNewDocument())
    {
        return false;
    }

    SetThisDocumentType();

    bool retval;
    if (IsScenario())
        retval = OnNewScenario();
    else
        retval = OnNewGame();
    if (retval)
    {
        UpdateAllViews(nullptr, CGamDocHint(HINT_DOCREADY));
    }
    return retval;
}

bool CGamDoc::OnCloseDocument()
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
    if (!wxDocument::OnCloseDocument())
    {
        return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool CGamDoc::OnOpenDocument(const wxString& pszPathName)
{
    BOOL bRet = FALSE;
    SetThisDocumentType();
    // We make this call to ensure the sizes of the palette windows layout
    // don't "bloat" during document load. I can't really tell you why this
    // works but, unless you have a proper solution... Don't mess with it!
#if 0
    GetMainFrame()->ShowPalettePanes(TRUE);
#else
    CPP20_TRACE("TODO:  {}->{}\n", this, __func__);
#endif

    // This cheat is to have the filename being loaded available
    // to the Serialize routine
    m_strTmpPathName = pszPathName;
    bRet = wxDocument::OnOpenDocument(pszPathName);
    m_strTmpPathName.clear();

    if (bRet && !IsScenario())
    {
        // If multiplayer game, make sure the owner info hasn't been
        // tampered with.
        if (m_pPlayerMgr != NULL && !VerifyCurrentPlayerMask())
        {
            wxMessageBox(CB::string(IDS_ERR_PLAYER_TAMPER),
                            CB::GetAppName(),
                            wxOK | wxICON_STOP);
            bRet = FALSE;
        }
    }
    if (bRet)
    {
        UpdateAllViews(nullptr, CGamDocHint(HINT_DOCREADY));
    }
    else
    {
        // don't suggest saving doc
        SetModifiedFlag(false);
    }
    return bRet;
}

/////////////////////////////////////////////////////////////////////////////

bool CGamDoc::OnSaveDocument(const wxString& pszPathName)
{
    if (std::filesystem::exists(CB::string(pszPathName)))
    {
        if (m_bKeepGamBackup && IsScenario())
        {
            CB::string szTmp = pszPathName;
            if (IsScenario())
                szTmp = SetFileExt(szTmp, "gs_");
            else
                szTmp = SetFileExt(szTmp, "ga_");
            if (_access(szTmp, 0) != -1)    // Remove previous backup
                CFile::Remove(szTmp);
            CFile::Rename(pszPathName, szTmp);
        }
    }

    return wxDocument::OnSaveDocument(pszPathName);
}

/////////////////////////////////////////////////////////////////////////////

bool CGamDoc::DeleteContents()
{
    /* close may trigger paint of other windows,
        so close before delete */
    if (m_palTrayA)
    {
        CDockTrayPalette* pFrame = m_palTrayA->GetDockingFrame();
        if (pFrame)
        {
            pFrame->SetChild(NULL);         // Need to remove pointer from Tray's UI Frame.
        }
        m_palTrayA = nullptr;
    }
    if (m_palTrayB)
    {
        CDockTrayPalette* pFrame = m_palTrayB->GetDockingFrame();
        if (pFrame)
        {
            pFrame->SetChild(NULL);         // Need to remove pointer from Tray's UI Frame.
        }
        m_palTrayB = nullptr;
    }
    if (m_palMark)
    {
        CDockMarkPalette* pFrame = static_cast<CDockMarkPalette*>(m_palMark->GetDockingFrame());
        if (pFrame)
        {
            ASSERT_KINDOF(CDockMarkPalette, pFrame);
            pFrame->SetChild(NULL);         // Need to remove pointer from Marker's UI Frame.
        }
        m_palMark = nullptr;
    }

    // m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;

    if (m_file.m_hFile != CFile::hFileNull)
        m_file.Close();

    m_pTileFacingMap = NULL;

    m_pPBMgr = NULL;
    m_pGbx = NULL;
    m_pYMgr = NULL;
    m_pPTbl = NULL;

    m_pRcdMoves = nullptr;
    m_pHistMoves = nullptr;

    m_pPlayHist = NULL;

    m_pBookMark = NULL;
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

    DiscardWindowState();

    m_pPlayerMgr = NULL;
    m_dwCurrentPlayer = OWNER_MASK_SPECTATOR;
    m_dwPlayerHash = 0;
    m_strPlayerFileDescr.clear();

    m_bSimulateSpectator = FALSE;

    return true;
}


// wxDocument
// Called by OnSaveDocument and OnOpenDocument to implement standard
// Save/Load behaviour. Re-implement in derived class for custom
// behaviour.
bool CGamDoc::DoSaveDocument(const wxString& file)
{
    return mfcDoc->CDocument::OnSaveDocument(file);
}

bool CGamDoc::DoOpenDocument(const wxString& file)
{
    return mfcDoc->CDocument::OnOpenDocument(file);
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::OnIdle(BOOL bActive)
{
    if (bActive)
    {
        CMainFrame* pMFrame = GetMainFrame();

#if 0
        CDockMarkPalette& pDockMark = pMFrame->GetDockingMarkerWindow();
        pDockMark.SetChild(&*m_palMark);
        pMFrame->UpdatePaletteWindow(pDockMark, m_bMarkPalVisible);

        CDockTrayPalette& pDockTrayA = pMFrame->GetDockingTrayAWindow();
        pDockTrayA.SetChild(&*m_palTrayA);
        pMFrame->UpdatePaletteWindow(pDockTrayA, m_bTrayAVisible);

        CDockTrayPalette& pDockTrayB = pMFrame->GetDockingTrayBWindow();
        pDockTrayB.SetChild(&*m_palTrayB);
        pMFrame->UpdatePaletteWindow(pDockTrayB, m_bTrayBVisible);

        CReadMsgWnd& pDocMsg = pMFrame->GetMessageWindow();
        pMFrame->UpdatePaletteWindow(pDocMsg.GetParent(), m_bMsgWinVisible && !IsScenario());
        pDocMsg.SetText(this);
#else
        CPP20_TRACE("TODO:  {}->{}\n", this, __func__);
#endif
    }
}

/////////////////////////////////////////////////////////////////////////////

// Called my the project view to do what UpdateAllViews() would normally
// do. Since UpdateAllViews isn't virtual the MFC call that function
// will not call ours.

void CGamDoc::DoInitialUpdate()
{
#if 0
    (*m_palTrayA)->UpdatePaletteContents(NULL);
    (*m_palTrayB)->UpdatePaletteContents(NULL);
#else
    CPP20_TRACE("TODO:  {}->{}\n", this, __func__);
#endif
}

///////////////////////////////////////////////////////////////////////
// Support for new unique views on this document

void CGamDoc::CreateNewFrame(const CB::string& pszTitle,
    CPlayBoard& board)
{
    CDocTemplate* pTemplate = GetApp()->m_pBrdViewTmpl;
    class CreateParamManager
    {
    public:
        CreateParamManager(CGamDoc& d, CPlayBoard& board) :
            doc(d)
        {
            wxASSERT(!doc.m_pBoardParam);
            doc.m_pBoardParam = &board;
        }
        ~CreateParamManager()
        {
            doc.m_pBoardParam = nullptr;
        }
    private:
        CGamDoc& doc;
    } createParamMgr(*this, board);
    CMDIChildWndEx* pNewFrame
        = static_cast<CMDIChildWndEx*>(pTemplate->CreateNewFrame(*this, NULL));
    if (pNewFrame == NULL)
        AfxThrowMemoryException();               // Not created
    wxASSERT(pNewFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)));
    CB::string str = GetUserReadableName();
    str += " - ";
    str += pszTitle;
    pNewFrame->SetWindowText(str);
    pTemplate->InitialUpdateFrame(pNewFrame, *this);
}

/////////////////////////////////////////////////////////////////////////////

CGamProjView& CGamDoc::FindProjectView() const
{
    const wxList& views = GetViews();
    for (auto it = views.begin() ; it != views.end() ; ++it)
    {
        wxGamProjView* pView = dynamic_cast<wxGamProjView*>(*it);
        if (pView)
        {
            return *pView;
        }
    }
    wxASSERT(!"no project view");
    AfxThrowNotSupportedException();
}

CPlayBoardView* CGamDoc::FindPBoardView(const CPlayBoard& pPBoard) const
{
    if (!IsScenario() &&
        pPBoard.IsPrivate() &&
        pPBoard.IsOwnedButNotByCurrentPlayer(*this))
    {
        wxASSERT(!"private board");
        return nullptr;
    }

    POSITION pos = mfcDoc->GetFirstViewPosition();
    while (pos != NULL)
    {
        CView& pCont = CheckedDeref(mfcDoc->GetNextView(pos));
        if (pCont.IsKindOf(RUNTIME_CLASS(CPlayBoardViewContainer)))
        {
            CPlayBoardView& pView = static_cast<CPlayBoardViewContainer&>(pCont);
            if (&pView.GetPlayBoard() == &pPBoard)
                return &pView;
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::GetDocumentFrameList(std::vector<CB::not_null<CFrameWnd*>>& tblFrames) const
{
    tblFrames.clear();

    POSITION pos = mfcDoc->GetFirstViewPosition();
    while (pos != NULL)
    {
        CView* pView = mfcDoc->GetNextView(pos);
        CFrameWnd* pFrame = pView->GetParentFrame();
        wxASSERT(pFrame != NULL);
        size_t i;
        for (i = size_t(0); i < tblFrames.size(); i++)
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
    CB::string strFilter = CB::string::LoadString(IDS_GBOX_FILTER);
    CB::string strTitle = CB::string::LoadString(IDS_GBOX_SELECT);

    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,
        strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return FALSE;

    m_strGBoxFile = dlg.GetFileName();
    CB::string strGBoxPath = dlg.GetPathName();

    // Load gamebox using archival procedures.

    m_pGbx = new CGameBox;      // Create game box object
    CB::string strErr;
    if (!m_pGbx->Load(*this, strGBoxPath, strErr))
    {
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    // There must be at least one board in the Game Box file

    if (m_pGbx->GetBoardManager().IsEmpty())
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
    m_pPTbl = new CPieceTable(m_pGbx->GetPieceManager(), *this);
    m_pPTbl->CreatePlayingPieceTable();

    // Create the tray manager.

    m_pYMgr = new CTrayManager;
    m_pYMgr->SetTileManager(&m_pGbx->GetTileManager());

    // Finally set up the tray palettes
    wxASSERT(!m_palTrayA);
#if 0
    m_palTrayA = new CTrayPaletteContainer(*this, ID_VIEW_TRAYA);
    m_palTrayA->Create(GetMainFrame()->GetDockingTrayAWindow());
    wxASSERT(!m_palTrayB);
    m_palTrayB = new CTrayPaletteContainer(*this, ID_VIEW_TRAYB);
    m_palTrayB->Create(GetMainFrame()->GetDockingTrayBWindow());
    wxASSERT(!m_palMark);
    m_palMark = new CMarkerPaletteContainer(*this);
    m_palMark->Create(GetMainFrame()->GetDockingMarkerWindow());
#else
    CPP20_TRACE("TODO:  {}->{}\n", this, __func__);
#endif

    return TRUE;
}

BOOL CGamDoc::OnNewGame()
{
    CB::string strFilter = CB::string::LoadString(IDS_GSCN_FILTER);
    CB::string strTitle = CB::string::LoadString(IDS_GSCN_SELECT);

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
        CB::string strErr = AfxFormatString1(AFX_IDP_FAILED_TO_OPEN_DOC, dlg.GetPathName());
        wxMessageBox(strErr,
                        CB::GetAppName(),
                        wxOK | wxICON_EXCLAMATION);
        return FALSE;
    }

    CArchive ar(&file, CArchive::load | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = *this;
    ar.m_bForceFlat = FALSE;

    {
    wxBusyCursor busyCursor;
    TRY
    {
        m_strTmpPathName = dlg.GetPathName();
        m_bScenario = TRUE;             // Fake out shared code
        SerializeScenario(ar);
        m_bScenario = FALSE;
        m_strTmpPathName.clear();
    }
    CATCH_ALL(e)
    {
        file.Abort();       // Will not throw an exception
        return FALSE;
    }
    END_CATCH_ALL
    }

    SetGameState(stateRecording);

    // Check if we are starting a multi-player game. If we are, then
    // we need to prompt for a create a set of game files.
    if (m_pPlayerMgr != NULL)
    {
        // Generate a new Scenario ID. Only do this for multi-player
        // games.
        m_dwScenarioID = IssueScenarioID();

        // First prompt for name changes...

        CMultiplayerGameDialog dlgMultiplay(*m_pPlayerMgr);
        if (dlgMultiplay.ShowModal() != wxID_OK)
            return FALSE;

        // Prompt for a base file name...
        CB::string strExt = CB::string::LoadString(IDS_GAME_FILTER);
        CB::string strTitle = CB::string::LoadString(IDS_GAME_SELECT_ROOT_NAME);
        CFileDialog dlg(FALSE, "gam"_cbstring, NULL, OFN_HIDEREADONLY, strExt, NULL, 0);
        dlg.m_ofn.lpstrTitle = strTitle;

        if (dlg.DoModal() == IDOK)
        {
            CB::string strFileList;

            CB::string strBaseName = dlg.GetPathName();
            size_t nPos = strBaseName.rfind('.');
            strBaseName = strBaseName.substr(size_t(0), nPos);

            CB::string strExists;
            if (CheckIfPlayerFilesExist(strBaseName, dlg.GetFileExt(),
                dlgMultiplay.m_bCreateReferee, strExists))
            {
                CB::string strWarn = CB::string::Format(IDS_WARN_PLAYER_FILES_EXIST, strExists);
                if (wxMessageBox(strWarn,
                                    CB::GetAppName(),
                                    wxOK | wxCANCEL | wxCANCEL_DEFAULT | wxICON_EXCLAMATION) != wxOK)
                {
                    return FALSE;
                }
            }

            // Create an information string that will be placed in
            // all created games. This string will be displayed at the end of
            // the scenario description.

            CB::string strPlayers;
            for (const Player& player : *m_pPlayerMgr)
            {
                strPlayers += player.m_strName + "\r\n";
            }
            CB::string strGamInfo = CB::string::Format(IDS_INFO_MPLAY_CREATE, m_pPlayerMgr->size(),
                strPlayers);
            if (dlgMultiplay.m_bCreateReferee)
            {
                CB::string strReferee = CB::string::LoadString(IDS_INFO_MPLAY_CREATE_REF);
                strGamInfo += strReferee;
            }

            // First create each of the player game files...

            for (size_t i = size_t(0) ; i < m_pPlayerMgr->size() ; ++i)
            {
                PlayerId id(i);
                CB::string strPlayName;
                strPlayName = (*m_pPlayerMgr)[id].m_strName;
                CB::string strFName = strBaseName + "-" + strPlayName + "." + dlg.GetFileExt();

                m_strPlayerFileDescr = strGamInfo + "@" + strFName;  // For hash check and calc
                m_dwCurrentPlayer = CPlayerManager::GetMaskFromPlayerNum(id);
                m_dwPlayerHash = CalculateHashForCurrentPlayerMask();

                if (!DoSaveGameFile(strFName))
                    return FALSE;
                strFileList += strFName + "\n";
            }

            // Create the specator game file...

            CB::string strSpec = CB::string::LoadString(IDS_GAME_SPECTATOR);

            CB::string strFName = strBaseName + strSpec + dlg.GetFileExt();

            m_strPlayerFileDescr = strGamInfo + "@" + strFName; // For hash check and calc
            m_dwCurrentPlayer = OWNER_MASK_SPECTATOR;
            m_dwPlayerHash = CalculateHashForCurrentPlayerMask();

            if (!DoSaveGameFile(strFName))
                return FALSE;

            strFileList += strFName + "\n";

            // Optionally create the referee game file...

            if (dlgMultiplay.m_bCreateReferee)
            {
                CB::string strReferee = CB::string::LoadString(IDS_GAME_REFEREE);

                strFName = strBaseName + strReferee + dlg.GetFileExt();

                m_strPlayerFileDescr = strGamInfo + "@" + strFName;// For hash check and calc
                m_dwCurrentPlayer = PLAYER_REFEREE_FLAG | OWNER_MASK_REFEREE;
                m_dwPlayerHash = CalculateHashForCurrentPlayerMask();

                if (!DoSaveGameFile(strFName))
                    return FALSE;

                strFileList += strFName;
            }
            CB::string strMsg = CB::string::Format(IDS_MSG_PLAYER_FILES, strFileList);
            AfxMessageBox(strMsg, MB_OK | MB_ICONINFORMATION);

            return FALSE;
        }
        else
            return FALSE;
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

DWORD CGamDoc::CalculateHashForCurrentPlayerMask() const
{
    ASSERT(!m_strPlayerFileDescr.empty());
    std::array<std::byte, 18> bfr1 =
    /* use CP1252 (not wchar_t, not UTF8) to remain
        compatible with CB3 */
    Compute16ByteHash<18>(m_strPlayerFileDescr.a_str(),
        m_strPlayerFileDescr.a_size());
    bfr1[16] = static_cast<std::byte>((static_cast<uint32_t>(m_dwCurrentPlayer) >> 8));
    bfr1[17] = static_cast<std::byte>((static_cast<uint32_t>(m_dwCurrentPlayer) & uint32_t(0xFF)));
    std::array<std::byte, 16> bfr2 = Compute16ByteHash(bfr1.data(), bfr1.size());
    return *reinterpret_cast<DWORD*>(bfr2.data());
}

BOOL CGamDoc::VerifyCurrentPlayerMask() const
{
    DWORD dwCalcedKey = CalculateHashForCurrentPlayerMask();
    return m_dwPlayerHash == dwCalcedKey;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::CheckIfPlayerFilesExist(const CB::string& strBaseName, const CB::string& strFileExt, BOOL bCheckReferee,
    CB::string& strExist)
{
    BOOL bFilesExist = FALSE;

    for (const Player& player : *m_pPlayerMgr)
    {
        CB::string strPlayName = player.m_strName;
        CB::string strFName = strBaseName + "-" + strPlayName + "." + strFileExt;

        if (std::filesystem::exists(strFName))
        {
            strExist += strFName + "\n";
            bFilesExist = TRUE;
        }
    }
    CB::string strSpec = CB::string::LoadString(IDS_GAME_SPECTATOR);

    CB::string strFName = strBaseName + strSpec + strFileExt;
    if (std::filesystem::exists(strFName))
    {
        strExist += strFName + "\n";
        bFilesExist = TRUE;
    }

    if (!bCheckReferee)
        return bFilesExist;

    CB::string strReferee = CB::string::LoadString(IDS_GAME_REFEREE);

    strFName = strBaseName + strReferee + strFileExt;
    if (std::filesystem::exists(strFName))
    {
        strExist += strFName + "\n";
        bFilesExist = TRUE;
    }

    return bFilesExist;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGamDoc::DoSaveGameFile(const CB::string& pszFileName)
{
    CFile file;
    CFileException fe;

    if (!file.Open(pszFileName, CFile::modeCreate |
        CFile::modeReadWrite | CFile::shareExclusive, &fe))
    {
        mfcDoc->ReportSaveLoadException(pszFileName, &fe,
            FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
        return FALSE;
    }

    CArchive saveArchive(&file, CArchive::store | CArchive::bNoFlushOnDelete);
    saveArchive.m_pDocument = *this;
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
            mfcDoc->ReportSaveLoadException(pszFileName, e,
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

    m_pWinState->RestoreStateOfDocumentFrames();
    DiscardWindowState();                           // Discard used data
}

void CGamDoc::DiscardWindowState()
{
    m_pWinState = NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CGamDoc::SetThisDocumentType()
{
    wxDocTemplate& pDocTmpl = CheckedDeref(GetDocumentTemplate());
    CB::string str = pDocTmpl.GetDefaultExtension();
    m_bScenario = str.CompareNoCase("gsn") == 0;
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

const CTileManager& CGamDoc::GetTileManager() const
{
    return CheckedDeref(m_pGbx).GetTileManager();
}

const CMarkManager& CGamDoc::GetMarkManager() const
{
    return CheckedDeref(m_pGbx).GetMarkManager();
}

const CBoardManager& CGamDoc::GetBoardManager() const
{
    return CheckedDeref(m_pGbx).GetBoardManager();
}

const CPieceManager& CGamDoc::GetPieceManager() const
{
    return CheckedDeref(m_pGbx).GetPieceManager();
}

////////////////////////////////////////////////////////////////////////

CTileFacingMap& CGamDoc::GetFacingMap()
{
    wxASSERT(m_pGbx != NULL);
    if (m_pTileFacingMap != NULL)
        return *m_pTileFacingMap;
    else
    {
        m_pTileFacingMap = new CTileFacingMap(&GetTileManager());
        return *m_pTileFacingMap;
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
    CPlayBoard& pPBoard = GetPBoardManager().GetPBoard(nBrd);
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
    dlg.m_crPlotColor = CB::Convert(pPBoard.m_crPlotLineColor);
    dlg.m_nPlotWd = pPBoard.m_nPlotLineWidth;
    dlg.m_strBoardName = pPBoard.GetBoard()->GetName().wx_str();
    dlg.m_pPlayerMgr = GetPlayerManager();
    dlg.m_nOwnerSel = CPlayerManager::GetPlayerNumFromMask(pPBoard.GetOwnerMask());
    dlg.m_bOwnerInfoIsReadOnly = !IsScenario();
    dlg.m_bNonOwnerAccess = pPBoard.IsNonOwnerAccessAllowed();
    dlg.m_bPrivate = pPBoard.IsPrivate();
    dlg.m_bDrawLockedBeneath = pPBoard.GetDrawLockedBeneath();

    if (dlg.ShowModal() == wxID_OK)
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
        pPBoard.m_crPlotLineColor = CB::Convert(dlg.m_crPlotColor);
        pPBoard.m_nPlotLineWidth = dlg.m_nPlotWd;
        pPBoard.SetDrawLockedBeneath(dlg.m_bDrawLockedBeneath);

        if (dlg.m_pPlayerMgr && !dlg.m_bOwnerInfoIsReadOnly)
        {
            pPBoard.SetOwnerMask(CPlayerManager::GetMaskFromPlayerNum(dlg.m_nOwnerSel));
            pPBoard.PropagateOwnerMaskToAllPieces();
            pPBoard.SetNonOwnerAccess(dlg.m_bNonOwnerAccess);
            pPBoard.SetPrivate(dlg.m_bPrivate);
        }

        UpdateAllViews(NULL, CGamDocHint(HINT_BOARDCHANGE));
        UpdateAllViews(NULL, CGamDocHint(HINT_ALWAYSUPDATE));    // Repaint boards
        SetModifiedFlag();
    }
}

////////////////////////////////////////////////////////////////////////
// CGamDoc commands

void CGamDoc::OnUpdateViewTrayA(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
    pCmdUI.Check(m_bTrayAVisible);
}

void CGamDoc::OnUpdateViewTrayB(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
    pCmdUI.Check(m_bTrayBVisible);
}

void CGamDoc::OnUpdateViewMarkPalette(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
    pCmdUI.Check(m_bMarkPalVisible);
}

void CGamDoc::OnViewTrayA(wxCommandEvent& /*event*/)
{
    m_bTrayAVisible = !m_bTrayAVisible;
}

void CGamDoc::OnViewTrayB(wxCommandEvent& /*event*/)
{
    m_bTrayBVisible = !m_bTrayBVisible;
}

void CGamDoc::OnViewMarkPalette(wxCommandEvent& /*event*/)
{
    m_bMarkPalVisible = !m_bMarkPalVisible;
}

///////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CGamDoc::OnDebugMoveList(wxCommandEvent& /*event*/)
{
    CMoveList* pMoveList = NULL;
    if (m_pHistMoves != NULL)
        pMoveList = m_pHistMoves.get();
    else if (m_pRcdMoves != NULL)
        pMoveList = m_pRcdMoves.get();
    else
    {
        wxMessageBox("No move list exists!"_cbstring, CB::GetAppName());
        return;
    }
    wxFileDialog dlg(&CB::GetMainWndWx(),
                        wxFileSelectorPromptStr,
                        wxEmptyString, "movedump.txt"_cbstring,
                        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"_cbstring,
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK)
    {
        CFile file;
        if (file.Open(dlg.GetFilename(), CFile::modeWrite | CFile::modeCreate))
        {
            pMoveList->DumpToTextFile(*this, file);
            file.Close();
        }
    }
}

void CGamDoc::OnDebugPieceTable(wxCommandEvent& /*event*/)
{
    wxASSERT(m_pPTbl != NULL);
    if (m_pPTbl == NULL)
        return;
    wxFileDialog dlg(&CB::GetMainWndWx(),
                        wxFileSelectorPromptStr,
                        wxEmptyString, "piecedump.txt"_cbstring,
                        "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"_cbstring,
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK)
    {
        CFile file;
        if (file.Open(dlg.GetFilename(), CFile::modeWrite | CFile::modeCreate))
        {
            m_pPTbl->DumpToTextFile(file);
            file.Close();
        }
    }
}
#endif

void CGamDoc::OnUpdateEnable(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
}

///////////////////////////////////////////////////////////////////////

#if 0
void CGamDoc::OnEditSetBookMark()
{
    // Need WARNING MESSAGE regarding deleting mark
    m_pBookMark = new CGameState();
    if (!m_pBookMark->SaveState(*this))
    {
        // Memory low warning....
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
        UpdateAllViews(NULL, 0, CGamDocHint(HINT_GAMESTATEUSED));
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
    m_pBookMark = NULL;
}

void CGamDoc::OnUpdateEditClearBookMark(CCmdUI* pCmdUI)
{
    if (IsPlaying() || IsScenario())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_pBookMark != NULL);
}
#endif

void CGamDoc::OnPbckDiscard(wxCommandEvent& /*event*/)
{
    if (!IsPlayingMoves()) return;
    if (m_nMoveInterlock) return;       // Not reentrant!
    FlushAllIndicators();
    DiscardCurrentRecording(TRUE);
}

void CGamDoc::OnUpdatePbckDiscard(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(IsPlayingMoves());
}

void CGamDoc::OnPbckEnd(wxCommandEvent& /*event*/)
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
    UpdateAllViews(NULL, CGamDocHint(HINT_GAMESTATEUSED));
}

void CGamDoc::OnUpdatePbckEnd(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(IsPlaying() && m_nCurMove != Invalid_v<size_t>);
}

void CGamDoc::OnPbckFinish(wxCommandEvent& /*event*/)
{
    DoAcceptPlayback();
}

void CGamDoc::OnUpdatePbckFinish(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(IsPlayingMoves());
}

void CGamDoc::OnPbckNext(wxCommandEvent& /*event*/)
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
                    wxCommandEvent event(wxEVT_MENU, XRCID("ID_PBCK_NEXT"));
                    CB::GetMainWndWx().GetEventHandler()->AddPendingEvent(event);
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
            wxCommandEvent event(wxEVT_MENU, XRCID("ID_PBCK_NEXT"));
            CB::GetMainWndWx().GetEventHandler()->AddPendingEvent(event);
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

void CGamDoc::OnUpdatePbckNext(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario())
        pCmdUI.Enable(FALSE);
    else
    {
        pCmdUI.Enable((IsPlaying() && m_nCurMove != Invalid_v<size_t>) ||
            (m_bStepToNextHist && IsPlayingHistory() && !IsPlayingLastHistory()));
        pCmdUI.Check(m_bAutoPlayback);
    }
}

void CGamDoc::OnPbckStart(wxCommandEvent& /*event*/)
{
    if (!IsPlaying()) return;
    if (m_nMoveInterlock) return;       // Not reentrant!
    if (m_pMoves->IsDoMoveActive())
        return;                         // Must ignore since moves are still being played back

    MsgDialogCancel(TRUE);
    FlushAllIndicators();
    RestartMoves();
}

void CGamDoc::OnUpdatePbckStart(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI.Enable(FALSE);
    else
    {
        pCmdUI.Enable(IsPlaying() && ((m_nCurMove - size_t(1)) > m_nFirstMove ||
            (m_nCurMove == Invalid_v<size_t> &&
             m_pMoves->IsThisMovePossible(m_nFirstMove + size_t(1)))));
    }
}

void CGamDoc::OnPbckPrevious(wxCommandEvent& /*event*/)
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
            UpdateAllViews(NULL, CGamDocHint(HINT_GAMESTATEUSED)); // Sync up the images
        }

        // Clear out any strings we may have accumulated during the
        // search for the final visible move index.
        m_astrMsgHist.clear();

        while ((m_nCurMove = m_pMoves->DoMove(*this, m_nCurMove)) < nPrvMove &&
            m_nCurMove != Invalid_v<size_t>)
        {
            if (m_nCurMove < nPrvPrvMove)
                FlushAllIndicators();
            if (m_nCurMove == nPrvPrvMove)
            {
                m_bQuietPlayback = FALSE;   // Show last move
                UpdateAllViews(NULL, CGamDocHint(HINT_GAMESTATEUSED)); // Sync up the images
            }
        }
    }
    else
        UpdateAllViews(NULL, CGamDocHint(HINT_GAMESTATEUSED));

    wxASSERT(m_nCurMove == nPrvMove);
    m_nMoveInterlock--;
    m_bQuietPlayback = FALSE;
}

void CGamDoc::OnUpdatePbckPrevious(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario() ||  m_bAutoPlayback)
        pCmdUI.Enable(FALSE);
    else
    {
        pCmdUI.Enable(IsPlaying() && ((m_nCurMove - size_t(1)) > m_nFirstMove ||
            (m_nCurMove == Invalid_v<size_t> &&
             m_pMoves->IsThisMovePossible(m_nFirstMove + size_t(1)))));
    }
}

void CGamDoc::OnPbckStepToNextHist(wxCommandEvent& /*event*/)
{
    m_bStepToNextHist = !m_bStepToNextHist;
}

void CGamDoc::OnUpdatePbckStepToNextHist(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!IsScenario() && IsPlaying());
    pCmdUI.Check(m_bStepToNextHist);
}

void CGamDoc::OnPbckSkipKeepIndicators(wxCommandEvent& /*event*/)
{
    m_bKeepSkipInd = !m_bKeepSkipInd;
}

void CGamDoc::OnUpdatePbckSkipKeepIndicators(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!IsScenario() && IsPlaying());
    pCmdUI.Check(m_bKeepSkipInd);
}

void CGamDoc::OnPbckAutoStep(wxCommandEvent& /*event*/)
{
    m_bAutoStep = !m_bAutoStep;
}

void CGamDoc::OnUpdatePbckAutoStep(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!IsScenario() && IsPlaying());
    pCmdUI.Check(m_bAutoStep);
}

void CGamDoc::OnActDoMessage(wxCommandEvent& /*event*/)
{
    if (!IsPlaying())
        MsgSendDialogOpen();
}

void CGamDoc::OnUpdateActDoMessage(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!IsPlaying() && !IsScenario());
}

void CGamDoc::OnActRollDice(wxCommandEvent& /*event*/)
{
    if (!IsPlaying())
        MsgSendDialogOpen(TRUE);
}

void CGamDoc::OnUpdateActRollDice(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!IsPlaying() && !IsScenario());
}

void CGamDoc::OnPbckReadMessage(wxCommandEvent& /*event*/)
{
    m_bMsgWinVisible = !m_bMsgWinVisible;
}

void CGamDoc::OnUpdatePbckReadMessage(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!IsScenario());
    pCmdUI.Check(m_bMsgWinVisible);
}

void CGamDoc::OnPbckNextHistory(wxCommandEvent& /*event*/)
{
    wxASSERT(IsPlayingHistory());
    if (m_pMoves->IsDoMoveActive())
        return;                         // Must ignore since moves are still being played back

    size_t nCurHist = m_nCurHist;
    FinishHistoryPlayback();
    LoadAndActivateHistory(nCurHist + size_t(1));
    UpdateAllViews(NULL, CGamDocHint(HINT_GAMESTATEUSED));
}

void CGamDoc::OnUpdatePbckNextHistory(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(IsPlayingHistory() &&
            !IsPlayingLastHistory());
}

void CGamDoc::OnPbckCloseHistory(wxCommandEvent& /*event*/)
{
    wxASSERT(IsPlayingHistory());
    if (!IsPlayingHistory()) return;
    if (m_nMoveInterlock) return;       // We have to let it finish up.
    if (m_pMoves->IsDoMoveActive())
        return;                         // Must ignore since moves are still being played back

    FinishHistoryPlayback();
    UpdateAllViews(NULL, CGamDocHint(HINT_GAMESTATEUSED));
}

void CGamDoc::OnUpdatePbckCloseHistory(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario() || m_bAutoPlayback)
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(IsPlayingHistory());
}

void CGamDoc::OnFileSendRecording2File(wxCommandEvent& /*event*/)
{
    if (IsPlayingHistory())
    {
        wxMessageBox(CB::string::LoadString(IDS_ERR_NOSAVEWHENPLAY),
                        CB::GetAppName(),
                        wxOK | wxICON_INFORMATION);
        return;
    }
    SaveRecordedMoves();
}

void CGamDoc::OnUpdateFileSendRecording2File(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario())
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(IsAnyRecorded());
}

void CGamDoc::OnFileDiscardRecordedMoves(wxCommandEvent& /*event*/)
{
    if (IsPlayingHistory())
    {
        wxMessageBox(CB::string::LoadString(IDS_ERR_NODISCARDWHENPLAY),
                        CB::GetAppName(),
                        wxOK | wxICON_INFORMATION);
        return;
    }
    DiscardCurrentRecording(TRUE);
}

void CGamDoc::OnUpdateFileDiscardRecordedMoves(wxUpdateUIEvent& pCmdUI)
{
    if (IsScenario())
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(IsAnyRecorded());
}

void CGamDoc::OnEditCreateTray()
{
    wxASSERT(IsScenario());
    CTrayNewDialog dlg(GetTrayManager());
    if (dlg.ShowModal() == wxID_OK)
    {
        GetTrayManager().CreateTraySet(dlg.m_strName);

        CGamDocHint hint;
        hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = NULL;
        UpdateAllViews(NULL, hint);
        SetModifiedFlag();
    }
}

void CGamDoc::OnUpdateEditCreateTray(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsScenario());
}

void CGamDoc::OnEditScenarioProperties()
{
    wxASSERT(IsScenario());
    CScnPropDialog dlg;

    dlg.m_strAuthor = m_strScnAuthor.wx_str();
    dlg.m_strTitle = m_strScnTitle.wx_str();
    dlg.m_strDescr = m_strScnDescr.wx_str();

    if (dlg.ShowModal() == wxID_OK)
    {
        m_strScnAuthor = dlg.m_strAuthor;
        m_strScnTitle  = dlg.m_strTitle;
        m_strScnDescr  = dlg.m_strDescr;
        UpdateAllViews(NULL, CGamDocHint(HINT_GSNPROPCHANGE));
        SetModifiedFlag();
    }
}

void CGamDoc::OnUpdateEditScenarioProperties(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsScenario());
}

void CGamDoc::OnEditSelectBoards()
{
    wxASSERT(IsScenario());

    CPBoardManager& pPBMgr = GetPBoardManager();

    CSelectBoardsDialog dlg(CheckedDeref(pPBMgr.GetBoardManager()));
    pPBMgr.GetPBoardList(dlg.m_tblBrds);

    if (dlg.ShowModal() == wxID_OK)
    {
        // HINT_BOARDCHANGE closes removed board views

        // Then change the play list.
        pPBMgr.SetPBoardList(dlg.m_tblBrds);
        UpdateAllViews(NULL, CGamDocHint(HINT_BOARDCHANGE));
        SetModifiedFlag();
    }
}

void CGamDoc::OnUpdateEditSelectBoards(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsScenario());
}

void CGamDoc::OnEditImportPieceGroups(wxCommandEvent& /*event*/)
{
    wxASSERT(IsScenario());

    CImportTraysDlg dlg(*this);

    if (dlg.ShowModal() == wxID_OK)
    {
        CGamDocHint hint;
        hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = NULL;
        UpdateAllViews(NULL, hint);
        SetModifiedFlag();
    }
}

void CGamDoc::OnUpdateEditImportPieceGroups(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsScenario());
}

void CGamDoc::OnEditSelectGamePieces(wxCommandEvent& /*event*/)
{
    wxASSERT(IsScenario() && GetTrayManager().GetNumTraySets() > size_t(0));

    CSetPiecesDialog dlg(*this);
    dlg.m_nYSel = 0;                    // Default is first tray

    CloseTrayPalettes();                // ...Ditto that for tray palettes

    dlg.ShowModal();

    // Notify all visible trays
    CGamDocHint hint;
    hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = NULL;
    UpdateAllViews(NULL, hint);
    SetModifiedFlag();
}

void CGamDoc::OnUpdateEditSelectGamePieces(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsScenario() && GetTrayManager().GetNumTraySets() > size_t(0));
}

void CGamDoc::OnFileLoadMoveFile()
{
    if (IsPlaying())
    {
        wxMessageBox(CB::string::LoadString(IDS_ERR_NOLOADWHENPLAY),
                        CB::GetAppName(),
                        wxOK | wxICON_INFORMATION);
        return;
    }

    CB::string strFilter = CB::string::LoadString(IDS_GMOV_FILTER);
    CB::string strTitle = CB::string::LoadString(IDS_GMOV_ENTERNAME);

    wxFileDialog dlg(&CB::GetMainWndWx(),
                    strTitle,
                    wxEmptyString,
                    wxEmptyString,
                    strFilter,
                    wxFD_OPEN);

    if (dlg.ShowModal() != wxID_OK)
        return;

    if (!DiscardCurrentRecording(TRUE))
        return;
    LoadAndActivateMoveFile(dlg.GetFilename());
}

void CGamDoc::OnUpdateFileLoadMoveFile(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!IsScenario() && !IsPlaying());
}

///////////////////////////////////////////////////////////////////////

BOOL CGamDoc::IsRecordingCompoundMove() const
{
    return IsRecording() && m_pRcdMoves != NULL &&
        m_pRcdMoves->IsRecordingCompoundMove();
}

void CGamDoc::OnActCompoundMoveBegin(wxCommandEvent& /*event*/)
{
    RecordCompoundMoveBegin();
}

void CGamDoc::OnUpdateActCompoundMoveBegin(wxUpdateUIEvent& pCmdUI)
{
#if 0
    if (pCmdUI->m_pSubMenu != NULL)
    {
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION |
            (IsRecording() ?  MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    else
#endif
        pCmdUI.Enable(IsRecording());
    pCmdUI.Check(IsRecordingCompoundMove());
}

void CGamDoc::OnActCompoundMoveEnd(wxCommandEvent& /*event*/)
{
    RecordCompoundMoveEnd();
}

void CGamDoc::OnUpdateActCompoundMoveEnd(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsRecordingCompoundMove());
}

void CGamDoc::OnActCompoundMoveDiscard(wxCommandEvent& /*event*/)
{
    if (wxMessageBox(CB::string::LoadString(IDS_WARN_DISCARD_COMP_MOVE),
                        CB::GetAppName(),
                        wxYES_NO | wxICON_QUESTION | wxNO_DEFAULT) == wxYES)
        RecordCompoundMoveDiscard();
}

void CGamDoc::OnUpdateActCompoundMoveDiscard(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsRecordingCompoundMove());
}

void CGamDoc::OnUpdateIndicatorCompoundMove(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsRecordingCompoundMove());
}

void CGamDoc::OnFileSaveGameAsScenario(wxCommandEvent& /*event*/)
{
    wxASSERT(!IsScenario());
    BOOL bModified = IsModified();          // Cache the modified flag

    // Save various variables...
    m_bScenario = TRUE;
    CB::string strPlayerFileDescr = m_strPlayerFileDescr;
    DWORD dwPlayerHash = m_dwPlayerHash;
    PlayerMask dwCurrentPlayer = m_dwCurrentPlayer;
    DWORD dwScenarioID = m_dwScenarioID;    // Cache scenario ID

    m_strPlayerFileDescr.clear();
    m_dwPlayerHash = 0;
    m_dwCurrentPlayer = OWNER_MASK_SPECTATOR;
    m_dwScenarioID = IssueScenarioID();     // Create new scenario ID

#if 0
    std::unique_ptr<CB::string> fileName = CB::string::DoPromptFileName(*GetApp(), IDS_SAVEGAMEASSCENARIO,
        OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE,
        GetApp()->GetScnenarioDocTemplate());
    if (fileName)
    {
        TRY
        {
            FlushAllIndicators();
            OnSaveDocument(*fileName);
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
#else
    AfxThrowNotSupportedException();
#endif
}

void CGamDoc::OnUpdateFileSaveGameAsScenario(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!IsScenario() && (!HasPlayers() ||
        HasPlayers() && IsCurrentPlayerReferee()));
}

void CGamDoc::OnPbckStepCompoundMoves(wxCommandEvent& /*event*/)
{
    if (!IsPlaying()) return;
    m_pMoves->SetCompoundSingleStep(!m_pMoves->IsSingleSteppingCompoundMoves());
}

void CGamDoc::OnUpdateStepCompoundMoves(wxUpdateUIEvent& pCmdUI)
{
    BOOL bEnable = !IsScenario() && IsPlaying();
    if (bEnable && IsPlaying())
    {
        wxASSERT(m_pMoves != NULL);
        pCmdUI.Check(m_pMoves->IsSingleSteppingCompoundMoves());
    }
    else
        pCmdUI.Check(FALSE);

    pCmdUI.Enable(bEnable);
}

void CGamDoc::OnViewShowTipText(wxCommandEvent& /*event*/)
{
    m_bShowObjTipText = !m_bShowObjTipText;
}

void CGamDoc::OnUpdateViewShowTipText(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(m_bShowObjTipText);
    pCmdUI.Enable(TRUE);
}

void CGamDoc::OnViewShowTipOwner(wxCommandEvent& /*event*/)
{
    m_bDisableOwnerTips = !m_bDisableOwnerTips;
}

void CGamDoc::OnUpdateViewShowTipOwner(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(!m_bDisableOwnerTips);
    pCmdUI.Enable(TRUE);
}

void CGamDoc::OnViewSaveWinState(wxCommandEvent& /*event*/)
{
    m_bSaveWindowPositions = !m_bSaveWindowPositions;
}

void CGamDoc::OnUpdateViewSaveWinState(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
    pCmdUI.Check(m_bSaveWindowPositions);
}

void CGamDoc::OnEditCreatePlayers(wxCommandEvent& /*event*/)
{
    CCreatePlayersDialog dlg;
    dlg.m_nPlayerCount = m_pPlayerMgr != NULL ? m_pPlayerMgr->size() : size_t(0);
    if (dlg.ShowModal() != wxID_OK)
        return;

    m_pPlayerMgr = NULL;
    ClearAllOwnership();            // Start with clean slate
    if (dlg.m_nPlayerCount > size_t(0))
    {
        m_pPlayerMgr = new CPlayerManager;
        for (size_t i = size_t(0) ; i < dlg.m_nPlayerCount ; ++i)
        {
            CB::string szBfr = value_preserving_cast<CB::string::value_type>(size_t('A') + i);
            CB::string str = CB::string::Format(IDS_BASE_PLAYER_NAME, szBfr);
            m_pPlayerMgr->AddPlayer(str);
        }
    }
    SetModifiedFlag();
    UpdateAllViews(NULL, nullptr);
}

void CGamDoc::OnUpdateEditCreatePlayers(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsScenario());
}

void CGamDoc::OnEditEditPlayers(wxCommandEvent& /*event*/)
{
    CEditPlayersDialog dlg;
    dlg.SetDialogsPlayerNames(*m_pPlayerMgr);
    if (dlg.ShowModal() != wxID_OK)
        return;
    dlg.GetPlayerNamesFromDialog(*m_pPlayerMgr);
    SetModifiedFlag();
    UpdateAllViews(NULL, nullptr);
}

void CGamDoc::OnUpdateEditEditPlayers(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsScenario() && m_pPlayerMgr != NULL &&
        !m_pPlayerMgr->empty());
}

void CGamDoc::OnActSimulateSpectator(wxCommandEvent& /*event*/)
{
    m_bSimulateSpectator = !m_bSimulateSpectator;
    UpdateAllViews(NULL, CGamDocHint(HINT_GAMESTATEUSED));       // So trays sync up
}

void CGamDoc::OnUpdateActSimulateSpectator(wxUpdateUIEvent& pCmdUI)
{
    if (!IsScenario() && bool(m_dwCurrentPlayer))
    {
        pCmdUI.Enable(TRUE);
        pCmdUI.Check(m_bSimulateSpectator);
    }
    else
    {
        pCmdUI.Enable(FALSE);
        pCmdUI.Check(false);
    }
}

void CGamDoc::OnEditCreateGeomorphic(wxCommandEvent& /*event*/)
{
    CCreateGeomorphicBoardDialog dlg(*this);
    if (dlg.ShowModal() != wxID_OK)
        return;
    OwnerPtr<CGeomorphicBoard> pGeoBoard = dlg.DetachGeomorphicBoard();

    GetPBoardManager().AddBoard(std::move(pGeoBoard));     // Add to list of active boards

    UpdateAllViews(NULL, CGamDocHint(HINT_BOARDCHANGE));
    SetModifiedFlag();
}

void CGamDoc::OnUpdateEditCreateGeomorphic(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsScenario());
}

static BYTE szPassWord[16] =
{
    0x16, 0xd2, 0x19, 0xe1,
    0x64, 0x7a, 0xf2, 0x21,
    0xec, 0x9b, 0x43, 0x87,
    0xe4, 0x7d, 0xf6, 0x6d
};

void CGamDoc::OnFileCreateReferee(wxCommandEvent& /*event*/)
{
    CPasswordDialog dlg;
    if (dlg.ShowModal() != wxID_OK)
        return;
    MD5_CTX md5Context;
    /* use CP1252 (not wchar_t, not UTF8) to remain
        compatible with CB3 */
    MD5Calc(&md5Context, dlg.m_strPassword.a_str(),
        dlg.m_strPassword.a_size());
    if (memcmp(md5Context.digest, szPassWord, 16) != 0)
    {
        wxMessageBox(CB::string::LoadString(IDS_ERR_INVALID_PASSWORD),
                        CB::GetAppName());
        return;
    }

    // Save so we can later restore.
    CB::string strPlayerFileDescr = m_strPlayerFileDescr;
    PlayerMask dwCurrentPlayer = m_dwCurrentPlayer;
    DWORD dwPlayerHash = m_dwPlayerHash;

    CB::string strReferee = CB::string::LoadString(IDS_GAME_REFEREE);

    // Rip apart current player string to get the raw stuff
    // we need to create a referee file.
    CB::string strBaseName = GetFilename();
    size_t nPos = strBaseName.rfind('-');
    if (nPos != CB::string::npos)
        strBaseName = strBaseName.substr(size_t(0), nPos);
    else
    {
        // Just remove the extension
        size_t nPos = strBaseName.rfind('.');
        if (nPos != CB::string::npos)
            strBaseName = strBaseName.substr(size_t(0), nPos);
    }
    CB::string strGamInfo;
    nPos = m_strPlayerFileDescr.rfind('@');
    if (nPos != CB::string::npos)
        strGamInfo = m_strPlayerFileDescr.substr(size_t(0), nPos);

    CB::string strFName = strBaseName + strReferee + "gam";

    if (std::filesystem::exists(strFName))
    {
        // File already exists. Prompt for overwrite.
        CB::string str = CB::string::Format(IDS_WARN_REF_EXISTS, strFName);
        if (wxMessageBox(str,
                            CB::GetAppName(),
                            wxOK | wxCANCEL | wxICON_EXCLAMATION | wxCANCEL_DEFAULT) != IDOK)
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
        CB::string str = CB::string::Format(IDS_INFO_REF_CREATED, strFName);
        wxMessageBox(str, CB::GetAppName());
    }
}

void CGamDoc::OnUpdateFileCreateReferee(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(HasPlayers() && !IsCurrentPlayerReferee());
}

void CGamDoc::OnFileChangeGameOwner(wxCommandEvent& /*event*/)
{
    CPasswordDialog dlg;
    if (dlg.ShowModal() != wxID_OK)
        return;
    MD5_CTX md5Context;
    /* use CP1252 (not wchar_t, not UTF8) to remain
        compatible with CB3 */
    MD5Calc(&md5Context, dlg.m_strPassword.a_str(),
        dlg.m_strPassword.a_size());
    if (memcmp(md5Context.digest, szPassWord, 16) != 0)
    {
        wxMessageBox(CB::string::LoadString(IDS_ERR_INVALID_PASSWORD),
            CB::GetAppName());
        return;
    }

    CChgGameOwnerDialog dlg2(CheckedDeref(GetPlayerManager()));
    if (dlg2.ShowModal() != wxID_OK)
        return;

    if (dlg2.m_nPlayer == INVALID_PLAYER)
        return;                         // No player selected

    PlayerMask dwPlayerMask = CPlayerManager::GetMaskFromPlayerNum(dlg2.m_nPlayer);
    CB::string strPlayerName = (*m_pPlayerMgr)[dlg2.m_nPlayer].m_strName;

    // Save so we can later restore.
    CB::string strPlayerFileDescr = m_strPlayerFileDescr;
    PlayerMask dwCurrentPlayer = m_dwCurrentPlayer;
    DWORD dwPlayerHash = m_dwPlayerHash;

    // Rip apart current player string to get the raw stuff
    // we need to create a referee file.
    CB::string strBaseName = GetFilename();
    size_t nPos = strBaseName.rfind('-');
    if (nPos != CB::string::npos)
        strBaseName = strBaseName.substr(size_t(0), nPos);
    else
    {
        // Just remove the extension
        size_t nPos = strBaseName.rfind('.');
        if (nPos != CB::string::npos)
            strBaseName = strBaseName.substr(size_t(0), nPos);
    }
    CB::string strGamInfo;
    nPos = m_strPlayerFileDescr.rfind('@');
    if (nPos != CB::string::npos)
        strGamInfo = m_strPlayerFileDescr.substr(size_t(0), nPos);

    CB::string strFName = strBaseName + "-" + strPlayerName + ".gam";

    if (std::filesystem::exists(strFName))
    {
        // File already exists. Prompt for overwrite.
        CB::string str = CB::string::Format(IDS_WARN_REF_EXISTS, strFName);
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
        CB::string str = CB::string::Format(IDS_INFO_GAME_CREATED, strFName);
        wxMessageBox(str, CB::GetAppName());
    }
}

void CGamDoc::OnUpdateFileChangeGameOwner(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(HasPlayers() && !IsCurrentPlayerReferee());
}

