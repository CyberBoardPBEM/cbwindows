// VwPrjga1.cpp : Game Project View Support Routines
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
#include    "Gamdoc.h"
#include    "Restbl.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "Trays.h"
#include    "MoveHist.h"

#include    "VwPrjgam.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Scenario Info routine

void CGamProjView::DoGamProperty()
{
}

void CGamProjView::DoUpdateGamInfo()
{
    CString strTitle;
    CString strAuthor;
    CString strDescr;
    CString strMultiInfo;
    strTitle.LoadString(IDS_PRJINFO_TITLE);
    strAuthor.LoadString(IDS_PRJINFO_AUTHOR);
    strDescr.LoadString(IDS_PRJINFO_DESCRIPTION);
    strMultiInfo.LoadString(IDS_PRJINFO_MPLAYER);

    CGamDoc* pDoc = GetDocument();
    CString str;
    if (!pDoc->m_strScnTitle.IsEmpty())
        str += strTitle + pDoc->m_strScnTitle + "\r\n\r\n";
    if (!pDoc->m_strScnAuthor.IsEmpty())
        str += strAuthor + pDoc->m_strScnAuthor + "\r\n\r\n";
    if (pDoc->HasPlayers() && !pDoc->IsScenario())
    {
        CString strInfo;
        int nPos = pDoc->m_strPlayerFileDescr.ReverseFind('@');
        if (nPos >= 0)
            strInfo = pDoc->m_strPlayerFileDescr.Left(nPos);
        else
            strInfo = pDoc->m_strPlayerFileDescr;
        str += strMultiInfo + strInfo;
    }
    if (!pDoc->m_strScnDescr.IsEmpty())
        str += strDescr + pDoc->m_strScnDescr;
    m_editInfo.SetWindowText(str);
}

/////////////////////////////////////////////////////////////////////////////
// Playing Board support routines

void CGamProjView::DoBoardProperty()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    int nBrd = m_listProj.GetItemSourceCode(nSel);
    pDoc->DoBoardProperties(nBrd);
}

void CGamProjView::DoBoardView()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    int nBrd = m_listProj.GetItemSourceCode(nSel);

    CPlayBoard* pPBoard = pDoc->GetPBoardManager()->GetPBoard(nBrd);
    CView* pView = pDoc->FindPBoardView(pPBoard);
    if (pView != NULL)
    {
        // This board already has an editor. Activate that view.
        CFrameWnd* pFrm = pView->GetParentFrame();
        ASSERT(pFrm);
        pFrm->ActivateFrame();
    }
    else
    {
        CString strTitle;
        m_listProj.GetItemText(nSel, strTitle);
        pDoc->CreateNewFrame(GetApp()->m_pBrdViewTmpl, strTitle, pPBoard);
    }
}

void CGamProjView::DoUpdateBoardHelpInfo()
{
    m_editInfo.SetWindowText("");
}

void CGamProjView::DoUpdateBoardInfo()
{
    m_editInfo.SetWindowText("");
}

/////////////////////////////////////////////////////////////////////////////
// Game History support routines

// Save current moves
void CGamProjView::DoHistorySave()
{
    CGamDoc* pDoc = GetDocument();
    if (pDoc->IsPlaying())
    {
        AfxMessageBox(IDS_ERR_NOSAVEWHENPLAY, MB_OK | MB_ICONINFORMATION);
        return;
    }
    pDoc->SaveRecordedMoves();
}

// Load a move file and setup playback
void CGamProjView::DoHistoryLoad()
{
    GetDocument()->DoLoadMoveFile();
}

// Finished with the move file play back
void CGamProjView::DoHistoryDone()
{
    CGamDoc* pDoc = GetDocument();
    pDoc->DoAcceptPlayback();
 }

// Discard the current recording
void CGamProjView::DoHistoryDiscard()
{
    CGamDoc* pDoc = GetDocument();
    if (pDoc->GetGameState() == CGamDoc::stateHistPlay)
    {
        AfxMessageBox(IDS_ERR_NODISCARDWHENPLAY, MB_OK | MB_ICONINFORMATION);
        return;
    }
    GetDocument()->DiscardCurrentRecording(TRUE);
}

// Load and replay a history entry. The current move recording
// is not affected.
void CGamProjView::DoHistoryReplay()
{
    CGamDoc* pDoc = GetDocument();
    if (pDoc->IsPlayingMoves())
    {
        AfxMessageBox(IDS_ERR_NOHISTWHENMOVE, MB_OK | MB_ICONINFORMATION);
        return;
    }
    int nSel = m_listProj.GetCurSel();
    if (pDoc->IsPlaying())
    {
        DoHistoryReplayDone();
        nSel--;                     // Compensate for lost line "<hist...>"
    }
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpHist);
    int nHist = m_listProj.GetItemSourceCode(nSel);
    m_listProj.MarkGroupItem(grpHist, nHist);   // Pre mark line
    if (!pDoc->LoadAndActivateHistory(nHist))
    {
        m_listProj.MarkGroupItem();
        m_listProj.Invalidate();
    }
    // Move the current selection down one to track the
    // history record that was selected.
    m_listProj.SetCurSel(nSel+1);
}

// Finished with the move file play back
void CGamProjView::DoHistoryReplayDone()
{
    CGamDoc* pDoc = GetDocument();
    m_listProj.MarkGroupItem();
    GetDocument()->FinishHistoryPlayback();
}

void CGamProjView::DoHistoryExport()
{
    CGamDoc* pDoc = GetDocument();
    if (pDoc->IsPlaying())
    {
        AfxMessageBox(IDS_ERR_NOSAVEWHENPLAY, MB_OK | MB_ICONINFORMATION);
        return;
    }
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpHist);
    int nHist = m_listProj.GetItemSourceCode(nSel);
    pDoc->SaveHistoryMovesInFile(nHist);
}

void CGamProjView::DoUpdateHistoryHelpInfo()
{
    m_editInfo.SetWindowText("");
}

void CGamProjView::DoUpdateCurPlayInfo()
{
    CGamDoc* pDoc = GetDocument();
    CString str = "";
    if (pDoc->GetGameState() == CGamDoc::stateMovePlay)
    {
        if (!pDoc->m_pPlayHist->m_strDescr.IsEmpty())
            str = pDoc->m_pPlayHist->m_strDescr;
    }
    m_editInfo.SetWindowText(str);
}

void CGamProjView::DoUpdateHistoryInfo()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpHist);
    int nHist = m_listProj.GetItemSourceCode(nSel);
    CHistoryTable* pHistTbl = pDoc->GetHistoryTable();
    ASSERT(pHistTbl);
    CHistRecord* pHRec = pHistTbl->GetHistRecord(nHist);

    m_editInfo.SetWindowText(pHRec->m_strDescr);
}



