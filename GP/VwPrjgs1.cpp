// VwPrjgs1.cpp : Scenario View Support Routines
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "PPieces.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "Trays.h"
#include    "Player.h"

#include    "VwPrjgsn.h"

#include    "DlgScnp.h"
#include    "DlgYprop.h"
#include    "DlgSpece.h"
#include    "DlgPbprp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Scenario Info routine

void CGsnProjView::DoGsnProperty()
{
    GetDocument().DoScenarioProperties();
}

void CGsnProjView::DoUpdateGsnInfo()
{
    CGamDoc& pDoc = GetDocument();
    CB::string str;
    if (!pDoc.m_strScnTitle.empty())
        str += "TITLE: " + pDoc.m_strScnTitle + "\r\n\r\n";
    if (!pDoc.m_strScnAuthor.empty())
        str += "AUTHOR: " + pDoc.m_strScnAuthor + "\r\n\r\n";
    if (!pDoc.m_strScnDescr.empty())
        str += "DESCRIPTION:\r\n\r\n" + pDoc.m_strScnDescr;
    m_editInfo->SetValue(str);
}

/////////////////////////////////////////////////////////////////////////////
// Playing Board support routines

void CGsnProjView::DoBoardSelection()
{
    GetDocument().DoSelectBoards();
}

void CGsnProjView::DoBoardProperty()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel != wxNOT_FOUND);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpBrd);
    size_t nBrd = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));
    pDoc.DoBoardProperties(nBrd);
}

void CGsnProjView::DoBoardView()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel != wxNOT_FOUND);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpBrd);
    size_t nBrd = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CPlayBoard& pPBoard = pDoc.GetPBoardManager().GetPBoard(nBrd);
    CView* pView = pDoc.FindPBoardView(pPBoard);
    if (pView != NULL)
    {
        // This board already has an editor. Activate that view.
        CFrameWnd* pFrm = pView->GetParentFrame();
        wxASSERT(pFrm);
        pFrm->ActivateFrame();
    }
    else
    {
        CB::string strTitle = m_listProj->GetItemText(value_preserving_cast<size_t>(nSel));
        pDoc.CreateNewFrame(GetApp()->m_pBrdViewTmpl, strTitle, &pPBoard);
    }
}

void CGsnProjView::DoBoardRemove()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel != wxNOT_FOUND);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpBrd);
    size_t nBrd = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    pDoc.GetPBoardManager().DeletePBoard(nBrd);
    pDoc.SetModifiedFlag(TRUE);
    pDoc.UpdateAllViews(NULL, HINT_BOARDCHANGE);
}

void CGsnProjView::DoUpdateBoardHelpInfo()
{
    m_editInfo->Clear();
}

void CGsnProjView::DoUpdateBoardInfo()
{
    m_editInfo->Clear();
}

/////////////////////////////////////////////////////////////////////////////
// Piece tray support routines

void CGsnProjView::DoTrayCreate()
{
    GetDocument().DoCreateTray();
}

void CGsnProjView::DoTrayProperty()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel != wxNOT_FOUND);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTray);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));


    CTrayPropDialog dlg(pDoc.GetTrayManager(), pDoc.GetPlayerManager());
    dlg.m_nYSel = nGrp;

    CTraySet& pYGrp = pDoc.GetTrayManager().GetTraySet(nGrp);
    dlg.m_bRandomSel = pYGrp.IsRandomPiecePull();
    dlg.m_bRandomSide = pYGrp.IsRandomSidePull();
    dlg.SetTrayViz(pYGrp.GetTrayContentVisibility());
    dlg.m_nOwnerSel = CPlayerManager::GetPlayerNumFromMask(pYGrp.GetOwnerMask());
    dlg.m_bNonOwnerAccess = pYGrp.IsNonOwnerAccessAllowed();
    dlg.m_bEnforceVizForOwnerToo = pYGrp.IsEnforcingVisibilityForOwnerToo();

    if (dlg.ShowModal() == wxID_OK)
    {
        pYGrp.SetName(dlg.m_strName);

        pYGrp.SetRandPiecePull(dlg.m_bRandomSel);
        pYGrp.SetRandSidePull(dlg.m_bRandomSide);
        pYGrp.SetTrayContentVisibility(dlg.GetTrayViz());
        if (pDoc.GetPlayerManager() != NULL)
        {
            pYGrp.SetOwnerMask(CPlayerManager::GetMaskFromPlayerNum(dlg.m_nOwnerSel));
            pYGrp.PropagateOwnerMaskToAllPieces(&pDoc);
            pYGrp.SetNonOwnerAccess(dlg.m_bNonOwnerAccess);
            pYGrp.SetEnforceVisibilityForOwnerToo(dlg.m_bEnforceVizForOwnerToo);
        }

        CGamDocHint hint;
        hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = NULL;
        pDoc.UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
        pDoc.SetModifiedFlag();
    }
}

void CGsnProjView::DoTrayEdit()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel != wxNOT_FOUND);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTray);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));
    CTrayManager& pYMgr = pDoc.GetTrayManager();

    CSetPiecesDialog dlg(pDoc);
    dlg.m_nYSel = value_preserving_cast<int>(nGrp);

    m_listTrays->SetItemMap(NULL);       // Clear this since repaint may fail...
    pDoc.CloseTrayPalettes();          // ...Ditto that for tray palettes

    dlg.ShowModal();

    // Notify all visible trays
    CGamDocHint hint;
    hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = NULL;
    pDoc.UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
    pDoc.SetModifiedFlag();
}

void CGsnProjView::DoTrayDelete()
{
    CGamDoc& pDoc = GetDocument();
    CTrayManager& pYMgr = pDoc.GetTrayManager();
    CPieceTable& pPTbl = pDoc.GetPieceTable();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel != wxNOT_FOUND);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTray);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    if (!pYMgr.GetTraySet(nGrp).IsEmpty())
    {
        if (AfxMessageBox(IDS_ERR_TRAYHASPIECES,
                MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
            return;
        // Mark those pieces as unused.
        pPTbl.SetPieceListAsUnused(
            pYMgr.GetTraySet(nGrp).GetPieceIDTable());
    }
    pYMgr.DeleteTraySet(nGrp);
    CGamDocHint hint;
    hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = NULL;
    pDoc.UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
    pDoc.SetModifiedFlag();
}

void CGsnProjView::DoUpdateTrayHelpInfo()
{
    m_editInfo->Clear();
}

void CGsnProjView::DoUpdateTrayList()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel != wxNOT_FOUND);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTray);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CTraySet& pYGrp = pDoc.GetTrayManager().GetTraySet(nGrp);
    const std::vector<PieceID>& pLstMap = pYGrp.GetPieceIDTable();
    m_listTrays->SetItemMap(&pLstMap);
}


