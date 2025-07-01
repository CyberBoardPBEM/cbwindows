// GamDoc2.cpp - Support code for recording user actions
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

void CGamDoc::SaveHistoryMovesInFile(size_t nHistRec)
{
    // Get history record.
    ASSERT(nHistRec < m_pHistTbl->GetNumHistRecords());
    CHistRecord& pHist = m_pHistTbl->GetHistRecord(nHistRec);

    // Get the output file name
    CB::string strFilter = CB::string::LoadString(IDS_GMOV_FILTER);
    CB::string strTitle = CB::string::LoadString(IDS_GMOV_ENTERNAME);

    CB::string szFName = GetPathName();
    if (!szFName.empty())
    {
        szFName = SetFileExt(szFName, "gmv");
    }

    CFileDialog dlg(FALSE, "gmv"_cbstring, !szFName.empty() ? szFName : NULL,
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
        CB::string strErr = AfxFormatString1(AFX_IDP_FAILED_TO_OPEN_DOC, dlg.GetPathName());
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    CArchive ar(&file, CArchive::store | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = this;
    ar.m_bForceFlat = FALSE;

    TRY
    {
        CHistRecord* temp = &pHist;
        SerializeMoveSet(ar, temp);
        ASSERT(temp == &pHist);

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
    CB::string strFilter = CB::string::LoadString(IDS_GMOV_FILTER);
    CB::string strTitle = CB::string::LoadString(IDS_GMOV_ENTERNAME);

    CB::string szFName = GetPathName();
    if (!szFName.empty())
    {
        szFName = SetFileExt(szFName, "gmv");
    }

    CFileDialog dlg(FALSE, "gmv"_cbstring, !szFName.empty() ? szFName : NULL,
        OFN_OVERWRITEPROMPT, strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return;

    // Get a description from user.
    CMovDescDialog dlg2;
    if (dlg2.ShowModal() != wxID_OK)
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
        CB::string strErr = AfxFormatString1(AFX_IDP_FAILED_TO_OPEN_DOC, dlg.GetPathName());
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    CArchive ar(&file, CArchive::store | CArchive::bNoFlushOnDelete);
    ar.m_pDocument = this;
    ar.m_bForceFlat = FALSE;

    // Force pending compound move to complete...
    if (m_pRcdMoves->IsRecordingCompoundMove())
        m_pRcdMoves->EndRecordingCompoundMove();

    pHist->m_pMList = std::move(m_pRcdMoves);

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

    m_strCurMsg.clear();
    m_astrMsgHist.clear();

    GetMainFrame()->GetMessageWindow()->SetText(NULL);

    // Save was ok..add to game history
    AddMovesToGameHistoryTable(pHist);

    // If no recording is under way make sure the random number seed
    // is new in case the first recorded move record is a random number
    // operation.
    m_nSeedCarryOver = (UINT)GetTickCount();
}

void CGamDoc::AddMovesToGameHistoryTable(OwnerPtr<CHistRecord> pHist)
{
    // If there are two game state records in the front of
    // the move list, remove the first since the second
    // contains the forced game state. There would be two in
    // the case of loading a move file so we can discard the
    // move file effects if the user didn't want the moves.
    if (pHist->m_pMList->GetCount() > size_t(1))
    {
        CMoveList::iterator pos = ++pHist->m_pMList->begin();
        ASSERT(pos != pHist->m_pMList->end());
        const CMoveRecord& pRcd = pHist->m_pMList->GetAt(pos);
        if (pRcd.GetType() == CMoveRecord::mrecState)
        {
            pHist->m_pMList->pop_front();
        }
    }

    if (m_pHistTbl == NULL)
        m_pHistTbl = new CHistoryTable;
    m_pHistTbl->AddNewHistRecord(std::move(pHist));

    m_eState = stateRecording;
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

    CMoveRecord& temp = *m_pRcdMoves->front();
    ASSERT(temp.GetType() == CMoveRecord::mrecState);
    CGameStateRcd& pMove = static_cast<CGameStateRcd&>(temp);

    if (!pMove.GetGameState().RestoreState(*this))
    {
        AfxMessageBox(IDS_ERR_FAILEDRESTORE, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    m_pRcdMoves = nullptr;

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
    OwnerPtr<CBoardPieceMove> pRcd = MakeOwner<CBoardPieceMove>(pPBrd->GetSerialNumber(),
        pid, pnt, ePos);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPlotList(CPlayBoard* pPBrd)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CMovePlotList> pRcd = MakeOwner<CMovePlotList>(pPBrd->GetSerialNumber(),
        *pPBrd->GetIndicatorList());
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPieceMoveToTray(const CTraySet& pYGrp, PieceID pid, size_t nPos)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    size_t nYGrp = GetTrayManager().FindTrayByRef(pYGrp);
    ASSERT(nYGrp != Invalid_v<size_t>);
    OwnerPtr<CTrayPieceMove> pRcd = MakeOwner<CTrayPieceMove>(nYGrp, pid, nPos);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPieceSetSide(PieceID pid, CPieceTable::Flip flip, size_t side, bool forceHide)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CPieceSetSide> pRcd = MakeOwner<CPieceSetSide>(pid, flip, side, forceHide);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPieceSetFacing(PieceID pid, uint16_t nFacingDegCW)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CPieceSetFacing> pRcd = MakeOwner<CPieceSetFacing>(pid, nFacingDegCW);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordPieceSetOwnership(PieceID pid, PlayerMask dwOwnerMask)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    wxASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CPieceSetOwnership> pRcd = MakeOwner<CPieceSetOwnership>(pid, dwOwnerMask);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordMarkerSetFacing(ObjectID dwObjID, MarkID mid, uint16_t nFacingDegCW)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CMarkerSetFacing> pRcd = MakeOwner<CMarkerSetFacing>(dwObjID, mid, nFacingDegCW);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordEventMessage(const CB::string& strMsg,
        BoardID nBoard, int x, int y)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CEventMessageRcd> pRcd =
        MakeOwner<CEventMessageRcd>(strMsg, nBoard, x, y);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

void CGamDoc::RecordEventMessage(const CB::string& strMsg,
        size_t nTray, PieceID pid)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CEventMessageRcd> pRcd =
        MakeOwner<CEventMessageRcd>(strMsg, nTray, pid);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordMarkMoveToBoard(CPlayBoard* pPBrd, ObjectID dwObjID,
    MarkID mid, CPoint pnt, PlacePos ePos /* = placeDefault */)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CBoardMarkerMove> pRcd = MakeOwner<CBoardMarkerMove>(pPBrd->GetSerialNumber(),
        dwObjID, mid, pnt, ePos);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordObjectDelete(ObjectID dwObjID)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CObjectDelete> pRcd = MakeOwner<CObjectDelete>(dwObjID);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordObjectSetText(GameElement elem, const CB::string& pszObjText)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CObjectSetText> pRcd = MakeOwner<CObjectSetText>(elem, &pszObjText);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordObjectLockdown(GameElement elem, BOOL bLockState)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CObjectLockdown> pRcd = MakeOwner<CObjectLockdown>(elem, bLockState);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordGameState()
{
    if (!IsRecording()) return;
    // Note: Don't Call CreateRecordListIfRequired() since it calls
    // us. Pretty nasty stuff!
    ASSERT(m_pRcdMoves != NULL);
    CGameState* pState = new CGameState();
    if (!pState->SaveState(*this))
    {
        // DISPLAY MEMORY LOW ERROR
        return;                     // Can't add the record.
    }
    OwnerPtr<CGameStateRcd> pRcd = MakeOwner<CGameStateRcd>(pState);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordMessage(const CB::string& strMsg)
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    OwnerPtr<CMessageRcd> pRcd = MakeOwner<CMessageRcd>(strMsg);
    m_pRcdMoves->AppendMoveRecord(std::move(pRcd));
}

////////////////////////////////////////////////////////////////////

void CGamDoc::RecordCompoundMoveBegin()
{
    if (!IsRecording()) return;
    CreateRecordListIfRequired();
    ASSERT(m_pRcdMoves != NULL);
    m_pRcdMoves->BeginRecordingCompoundMove(*this);
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
    m_pRcdMoves->CancelRecordingCompoundMove(*this);
}

////////////////////////////////////////////////////////////////////

void CGamDoc::CreateRecordListIfRequired()
{
    if (m_pRcdMoves != NULL)
        return;

    // Reseed the rendom number generator whenever a new set of
    // moves is started.
    m_nSeedCarryOver = (UINT)GetTickCount();

    m_pRcdMoves = MakeOwner<CMoveList>();
    if (m_eState == stateRecording)
        m_pMoves = m_pRcdMoves.get();         // Set up shadow pointer

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
    if (m_nCurMove != Invalid_v<size_t>)
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
    m_astrMsgHist.clear();
    TransferPlaybackToHistoryTable(bTruncateRemainingMoves);
}


