// VwPrjgs1.cpp : Scenario View Support Routines
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
    GetDocument()->DoScenarioProperties();
}

void CGsnProjView::DoUpdateGsnInfo()
{
    CGamDoc* pDoc = GetDocument();
    CString str;
    if (!pDoc->m_strScnTitle.IsEmpty())
        str += "TITLE: " + pDoc->m_strScnTitle + "\r\n\r\n";
    if (!pDoc->m_strScnAuthor.IsEmpty())
        str += "AUTHOR: " + pDoc->m_strScnAuthor + "\r\n\r\n";
    if (!pDoc->m_strScnDescr.IsEmpty())
        str += "DESCRIPTION:\r\n\r\n" + pDoc->m_strScnDescr;
    m_editInfo.SetWindowText(str);
}

/////////////////////////////////////////////////////////////////////////////
// Playing Board support routines

void CGsnProjView::DoBoardSelection()
{
    GetDocument()->DoSelectBoards();
}

void CGsnProjView::DoBoardProperty()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    int nBrd = m_listProj.GetItemSourceCode(nSel);
    pDoc->DoBoardProperties(nBrd);
}

void CGsnProjView::DoBoardView()
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

void CGsnProjView::DoBoardRemove()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    int nBrd = m_listProj.GetItemSourceCode(nSel);

    pDoc->GetPBoardManager()->DeletePBoard(nBrd);
    pDoc->SetModifiedFlag(TRUE);
    pDoc->UpdateAllViews(NULL, HINT_BOARDCHANGE);
}

void CGsnProjView::DoUpdateBoardHelpInfo()
{
    m_editInfo.SetWindowText("");
}

void CGsnProjView::DoUpdateBoardInfo()
{
    m_editInfo.SetWindowText("");
}

/////////////////////////////////////////////////////////////////////////////
// Piece tray support routines

void CGsnProjView::DoTrayCreate()
{
    GetDocument()->DoCreateTray();
}

void CGsnProjView::DoTrayProperty()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTray);
    int nGrp = m_listProj.GetItemSourceCode(nSel);


    CTrayPropDialog dlg;
    dlg.m_nYSel = nGrp;
    dlg.m_pYMgr = pDoc->GetTrayManager();
    dlg.m_pPlayerMgr = pDoc->GetPlayerManager();

    CTraySet* pYGrp = pDoc->GetTrayManager()->GetTraySet(nGrp);
    dlg.m_bRandomSel = pYGrp->IsRandomPiecePull();
    dlg.SetTrayViz(pYGrp->GetTrayContentVisibility());
    dlg.m_nOwnerSel = CPlayerManager::GetPlayerNumFromMask(pYGrp->GetOwnerMask());
    dlg.m_bNonOwnerAccess = pYGrp->IsNonOwnerAccessAllowed();
    dlg.m_bEnforceVizForOwnerToo = pYGrp->IsEnforcingVisibilityForOwnerToo();

    if (dlg.DoModal() == IDOK)
    {
        pYGrp->SetName(dlg.m_strName);

        pYGrp->SetRandPiecePull(dlg.m_bRandomSel);
        pYGrp->SetTrayContentVisibility(dlg.GetTrayViz());
        if (dlg.m_pPlayerMgr != NULL)
        {
            pYGrp->SetOwnerMask(CPlayerManager::GetMaskFromPlayerNum(dlg.m_nOwnerSel));
            pYGrp->PropagateOwnerMaskToAllPieces(pDoc);
            pYGrp->SetNonOwnerAccess(dlg.m_bNonOwnerAccess);
            pYGrp->SetEnforceVisibilityForOwnerToo(dlg.m_bEnforceVizForOwnerToo);
        }

        CGamDocHint hint;
        hint.m_pTray = NULL;
        pDoc->UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
        pDoc->SetModifiedFlag();
    }
}

void CGsnProjView::DoTrayEdit()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTray);
    int nGrp = m_listProj.GetItemSourceCode(nSel);
    CTrayManager* pYMgr = pDoc->GetTrayManager();

    CSetPiecesDialog dlg;
    dlg.m_pDoc = pDoc;
    dlg.m_nYSel = nGrp;

    m_listTrays.SetItemMap(NULL);       // Clear this since repaint may fail...
    pDoc->CloseTrayPalettes();          // ...Ditto that for tray palettes

    dlg.DoModal();

    // Notify all visible trays
    CGamDocHint hint;
    hint.m_pTray = NULL;
    pDoc->UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
    pDoc->SetModifiedFlag();
}

void CGsnProjView::DoTrayDelete()
{
    CGamDoc* pDoc = GetDocument();
    CTrayManager *pYMgr = pDoc->GetTrayManager();
    CPieceTable *pPTbl = pDoc->GetPieceTable();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTray);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    if (!pYMgr->GetTraySet(nGrp)->IsEmpty())
    {
        if (AfxMessageBox(IDS_ERR_TRAYHASPIECES,
                MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
            return;
        // Mark those pieces as unused.
        pPTbl->SetPieceListAsUnused(
            pYMgr->GetTraySet(nGrp)->GetPieceIDTable());
    }
    pYMgr->DeleteTraySet(nGrp);
    CGamDocHint hint;
    hint.m_pTray = NULL;
    pDoc->UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
    pDoc->SetModifiedFlag();
}

void CGsnProjView::DoUpdateTrayHelpInfo()
{
    m_editInfo.SetWindowText("");
}

void CGsnProjView::DoUpdateTrayList()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTray);
    int nGrp = m_listProj.GetItemSourceCode(nSel);

    CTraySet* pYGrp = pDoc->GetTrayManager()->GetTraySet(nGrp);
    if (pYGrp == NULL)
    {
        m_listTrays.SetItemMap(NULL);
        return;
    }
    CWordArray* pLstMap = pYGrp->GetPieceIDTable();
    m_listTrays.SetItemMap(pLstMap);
}


