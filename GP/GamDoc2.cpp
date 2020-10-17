// GamDoc2.cpp - Support code for recording user actions
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
#include    "GMisc.h"
#include    "StrLib.h"
#include    "FrmMain.h"
#include    "GamDoc.h"

#include    "Board.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "Marks.h"
#include    "Trays.h"
#include    "GameBox.h"
#include    "MoveMgr.h"
#include    "MoveHist.h"
#include    "GamState.h"

#include    "DlgMdesc.h"
#include    "DlgTrunc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////

void CGamDoc::SaveHistoryMovesInFile(int nHistRec)
{
    // Get history record.
    ASSERT(nHistRec < m_pHistTbl->GetNumHistRecords());
    CHistRecord* pHist = m_pHistTbl->GetHistRecord(nHistRec);
    ASSERT(pHist != NULL);

    // Get the output file name
    CString strFilter;
    strFilter.LoadString(IDS_GMOV_FILTER);
    CString strTitle;
    strTitle.LoadString(IDS_GMOV_ENTERNAME);

    char szFName[MAX_PATH];
    strcpy(szFName, GetPathName());
    if (*szFName != 0)
        SetFileExt(szFName, "gmv");

    CFileDialog dlg(FALSE, "gmv", *szFName != 0 ? szFName : NULL,
        OFN_OVERWRITEPROMPT, strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return;

    CFile file;
    CFileException fe;

    if (!file.Open(dlg.GetPathName(),
        CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive,
        &fe))
    {
        CString strErr;
        AfxFormatString1(strErr, AFX_IDP_FAILED_TO_OPEN_DOC, dlg.GetPathName());
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    CArchive ar(&file, CArchive::store | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = this;
    ar.m_bForceFlat = FALSE;

    TRY
    {
        SerializeMoveSet(ar, pHist);

        ar.Close();
        file.Close();
    }
    CATCH_ALL(e)
    {
        file.Abort();       // Will not throw an exception
    }
    END_CATCH_ALL
}

////////////////////////////////////////////////////////////////////

void CGamDoc::SaveRecordedMoves()
{
    if (m_pRcdMoves == NULL)
        return;                     // Nothing to save.

    // Dump all select lists and move lot lists.
    FlushAllIndicators();

    // Get the output file name
    CString strFilter;
    strFilter.LoadString(IDS_GMOV_FILTER);
    CString strTitle;
    strTitle.LoadString(IDS_GMOV_ENTERNAME);

    char szFName[_MAX_PATH];
    strcpy(szFName, GetPathName());
    if (*szFName != 0)
        SetFileExt(szFName, "gmv");

    CFileDialog dlg(FALSE, "gmv", *szFName != 0 ? szFName : NULL,
        OFN_OVERWRITEPROMPT, strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return;

    // Get a description from user.
    CMovDescDialog dlg2;
    if (dlg2.DoModal() != IDOK)
        return;

    // Set up history record.
    CHistRecord* pHist = new CHistRecord;
    pHist->m_strTitle = dlg2.m_strTitle;
    pHist->m_strDescr = dlg2.m_strDesc;
    pHist->m_timeCreated = CTime::GetCurrentTime();
    pHist->m_timeAbsorbed = pHist->m_timeCreated;

    CFile file;
    CFileException fe;

    if (!file.Open(dlg.GetPathName(),
        CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive,
        &fe))
    {
        CString strErr;
        AfxFormatString1(strErr, AFX_IDP_FAILED_TO_OPEN_DOC, dlg.GetPathName());
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    CArchive ar(&file, CArchive::store | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = this;
    ar.m_bForceFlat = FALSE;

    // Force pending compound move to complete...
    if (m_pRcdMoves->IsRecordingCompoundMove())
        m_pRcdMoves->EndRecordingCompoundMove();

    pHist->m_pMList = m_pRcdMoves;

    TRY
    {
        SerializeMoveSet(ar, pHist);

        ar.Close();
        file.Close();
    }
    CATCH_ALL(e)
    {
        file.Abort();       // Will not throw an exception
        return;
    }
    END_CATCH_ALL

    m_strCurMsg.Empty();
    m_astrMsgHist.RemoveAll();

    GetMainFrame()->GetMessageWindow()->SetText(NULL);

    // Save was ok..add to game history
    AddMovesToGameHistoryTable(pHist);

    // If no recording is under way make sure the random number seed
    // is new in case the first recorded move record is a random number
    // operation.
    m_nSeedCarryOver = (UINT)GetTickCount();
}

void CGamDoc::AddMovesToGameHistoryTable(CHistRecord* pHist)
{
    // If there are two game state records in the front of
    // the move list, remove the first since the second
    // contains the forced game state. There would be two in
    // the case of loading a move file so we can discard the
    // move file effects if the user didn't want the moves.
    if (pHist->m_pMList->GetCount() > 1)
    {
        POSITION pos = pHist->m_pMList->FindIndex(1);
        ASSERT(pos != NULL);
        CMoveRecord* pRcd = pHist->m_pMList->GetAt(pos);
        if (pRcd->GetType() == CMoveRecord::mrecState)
        {
            CMoveRecord* pRcd = (CMoveRecord*)m_pRcdMoves->RemoveHead();
            delete pRcd;
        }
    }

    if (m_pHistTbl == NULL)
        m_pHistTbl = new CHistoryTable;
    m_pHistTbl->AddNewHistRecord(pHist);

    m_eState = stateRecording;
    m_pRcdMoves = NULL;
    m_pMoves = NULL;            // Clear shadow pointer

    if (m_pBookMark)
        delete m_pBookMark;

    m_pBookMark = NULL;

    SetModifiedFlag();
    UpdateAllViews(NULL, HINT_GAMPROPCHANGE);

    MsgDialogCancel(TRUE);
}

////////////////////////////////////////////////////////////////////

BOOL CGamDoc::DiscardCurrentRecording(BOOL bPrompt /* = TRUE */)
{
    if (m_pRcdMoves == NULL)
        return TRUE;
    if (bPrompt)
    {
        if (AfxMessageBox(IDS_PMT_RCD_DISCARD, MB_YESNO |
                MB_ICONQUESTION) != IDYES)
            return FALSE;
    }

    // Dump all select lists and move lot lists.
    FlushAllIndicators();

    // Get the first record which contains the starting positions
    // of the recording. Restore it.

    CGameStateRcd* pMove = (CGameStateRcd*)m_pRcdMoves->GetFirstRecord();
    ASSERT(pMove != NULL);
    ASSERT(pMove->GetType() == CMoveRecord::mrecState);

    if (!pMove->GetGameState()->RestoreState())
    {
        AfxMessageBox(IDS_ERR_FAILEDRESTORE, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    delete m_pRcdMoves;
    m_pRcdMoves = NULL;

    m_eState = stateRecording;
    m_pMoves = NULL;        // Clear shadow pointer

    if (m_pBookMark != NULL)
        delete m_pBookMark;
    m_pBookMark = NULL;
    if (m_pPlayHist != NULL)
        delete m_pPlayHist;
    m_pPlayHist = NULL;

    SetModifiedFlag();

    MsgDialogCancel(TRUE);
    UpdateAllViews(NULL, HINT_GAMESTATEUSED);
    return TRUE;
}

////////////////////////////////////////////////////////////////////
// Allow moves to be grouped. (Example: A move of a stack of pieces
//      should be considered a single move even though it is made
//      up of several primative moves.)

void CGamDoc::AssignNewMoveGroup()
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    m_pRcdMoves->AssignNewMoveGroup();
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPieceMoveToBoard(CPlayBoard* pPBrd, PieceID pid, CPoint pnt,
    PlacePos ePos /* = placeDefault */)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CBoardPieceMove* pRcd = new CBoardPieceMove(pPBrd->GetSerialNumber(),
        pid, pnt, ePos);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPlotList(CPlayBoard* pPBrd)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CMovePlotList* pRcd = new CMovePlotList(pPBrd->GetSerialNumber(),
        pPBrd->GetIndicatorList());
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPieceMoveToTray(CTraySet* pYGrp, PieceID pid, int nPos)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    int nYGrp = GetTrayManager()->FindTrayByPtr(pYGrp);
    ASSERT(nYGrp != -1);
    CTrayPieceMove* pRcd = new CTrayPieceMove(nYGrp, pid, nPos);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPieceSetSide(PieceID pid, BOOL bTopUp)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CPieceSetSide* pRcd = new CPieceSetSide(pid, bTopUp);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPieceSetFacing(PieceID pid, int nFacingDegCW)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CPieceSetFacing* pRcd = new CPieceSetFacing(pid, nFacingDegCW);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPieceSetOwnership(PieceID pid, DWORD dwOwnerMask)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CPieceSetOwnership* pRcd = new CPieceSetOwnership(pid, dwOwnerMask);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordMarkerSetFacing(ObjectID dwObjID, MarkID mid, int nFacingDegCW)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CMarkerSetFacing* pRcd = new CMarkerSetFacing(dwObjID, mid, nFacingDegCW);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordEventMessage(CString strMsg, BOOL bIsBoardEvent,
        int nID, int nVal1, int nVal2 /* = 0*/)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CEventMessageRcd* pRcd =
        new CEventMessageRcd(strMsg, bIsBoardEvent, nID, nVal1, nVal2);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordMarkMoveToBoard(CPlayBoard* pPBrd, ObjectID dwObjID,
    MarkID mid, CPoint pnt, PlacePos ePos /* = placeDefault */)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CBoardMarkerMove* pRcd = new CBoardMarkerMove(pPBrd->GetSerialNumber(),
        dwObjID, mid, pnt, ePos);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordObjectDelete(ObjectID dwObjID)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CObjectDelete* pRcd = new CObjectDelete(dwObjID);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordObjectSetText(GameElement elem, LPCTSTR pszObjText)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CObjectSetText* pRcd = new CObjectSetText(elem, pszObjText);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordObjectLockdown(GameElement elem, BOOL bLockState)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CObjectLockdown* pRcd = new CObjectLockdown(elem, bLockState);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordGameState()
{
    if (!IsRecording()) return;
    // Note: Don't Call CreateRecordListIfRequired() since it calls
    // us. Pretty nasty stuff!
    ASSERT(m_pRcdMoves != NULL);
    CGameState* pState = new CGameState(this);
    if (!pState->SaveState())
    {
        // DISPLAY MEMORY LOW ERROR
        return;                     // Can't add the record.
    }
    CGameStateRcd* pRcd = new CGameStateRcd(pState);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordMessage(CString strMsg)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    CMessageRcd* pRcd = new CMessageRcd(strMsg);
    m_pRcdMoves->AppendMoveRecord(pRcd);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordCompoundMoveBegin()
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    m_pRcdMoves->BeginRecordingCompoundMove(this);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordCompoundMoveEnd()
{
    if (!IsRecording()) return;
    ASSERT(m_pRcdMoves != NULL);
    ASSERT(m_pRcdMoves->IsRecordingCompoundMove());
    m_pRcdMoves->EndRecordingCompoundMove();
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordCompoundMoveDiscard()
{
    if (!IsRecording()) return;
    ASSERT(m_pRcdMoves != NULL);
    ASSERT(m_pRcdMoves->IsRecordingCompoundMove());
    m_pRcdMoves->CancelRecordingCompoundMove(this);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::CreateRecordListIfRequired()
{
    if (m_pRcdMoves != NULL)
        return;

    // Reseed the rendom number generator whenever a new set of
    // moves is started.
    m_nSeedCarryOver = (UINT)GetTickCount();

    m_pRcdMoves = new CMoveList;
    if (m_eState == stateRecording)
        m_pMoves = m_pRcdMoves;         // Set up shadow pointer

    // First record is the board state. Required so the
    // move file can be used to reconstitute the game state and
    // start play. The person doing playback may choose not to
    // apply this record.

    RecordGameState();

    UpdateAllViews(NULL, HINT_GAMPROPCHANGE);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::DoAcceptPlayback()    // (exposed for project window access)
{
    if (!IsPlayingMoves()) return;
    if (m_nMoveInterlock) return;       // Not reentrant!

    BOOL bTruncateRemainingMoves = FALSE;
    if (m_nCurMove != -1)
    {
        CTruncatePlaybackDlg dlg;
        if (dlg.DoModal() != IDOK)
            return;
        bTruncateRemainingMoves = dlg.m_nTruncateSel == 1;
    }
    else
    {
        if (AfxMessageBox(IDS_PMT_FINISHPLAY, MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;
    }

    FlushAllIndicators();
    m_astrMsgHist.RemoveAll();
    TransferPlaybackToHistoryTable(bTruncateRemainingMoves);
}


