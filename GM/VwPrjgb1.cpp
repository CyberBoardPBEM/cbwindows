// VwPrjgb1.cpp : Support file for vwprjgbx.cpp
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
#include    "Gm.h"
#include    "GmDoc.h"
#include    "ResTbl.h"
#include    "Tile.h"
#include    "Board.h"
#include    "Pieces.h"
#include    "Marks.h"
#include    "DlgGboxp.h"
#include    "DlgMkbrd.h"
#include    "DlgMakts.h"
#include    "DlgNtile.h"
#include    "DlgTsetp.h"
#include    "DlgBrdp.h"
#include    "DlgPcep.h"
#include    "DlgMrkp.h"
#include    "DlgPgrpn.h"
#include    "DlgMgrpn.h"
#include    "DlgPnew.h"
#include    "DlgPedt.h"
#include    "DlgMnew.h"
#include    "DlgMedt.h"
#include    "DlgPEditMulti.h"
#include    "DlgMEditMulti.h"
#include    "VwPrjgbx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// Game Box info routine

void CGbxProjView::DoGbxProperty()
{
    GetDocument()->DoGbxProperties();
}

void CGbxProjView::DoUpdateGbxInfo()
{
    CGamDoc* pDoc = GetDocument();
    CString str;
    if (!pDoc->m_strTitle.IsEmpty())
        str += "TITLE: " + pDoc->m_strTitle + "\r\n\r\n";
    if (!pDoc->m_strAuthor.IsEmpty())
        str += "AUTHOR: " + pDoc->m_strAuthor + "\r\n\r\n";
    if (!pDoc->m_strDescr.IsEmpty())
        str += "DESCRIPTION:\r\n\r\n" + pDoc->m_strDescr;
    m_editInfo.SetWindowText(str);
}

/////////////////////////////////////////////////////////////////////
// Playing Board Support Methods

void CGbxProjView::DoBoardCreate()
{
    GetDocument()->DoCreateBoard();
}

void CGbxProjView::DoBoardProperty()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    size_t nBrd = m_listProj.GetItemSourceCode(nSel);

    CBoard& pBoard = pDoc->GetBoardManager()->GetBoard(nBrd);

    if (pDoc->DoBoardPropertyDialog(pBoard))
    {
        pDoc->SetModifiedFlag();
        DoUpdateProjectList();
    }
}

void CGbxProjView::DoBoardDelete()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    size_t nBrd = m_listProj.GetItemSourceCode(nSel);

    CString strTitle;
    m_listProj.GetItemText(nSel, strTitle);
    CString strPrompt;
    AfxFormatString1(strPrompt, IDP_DELETEBOARD, strTitle);
    if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        CGmBoxHint hint;
        hint.GetArgs<HINT_BOARDDELETED>().m_pBoard = &pDoc->GetBoardManager()->GetBoard(nBrd);

        pDoc->GetBoardManager()->DeleteBoard(nBrd);

        // Make sure any views on the board are closed.
        pDoc->UpdateAllViews(NULL, HINT_BOARDDELETED, &hint);

        pDoc->SetModifiedFlag();
        pDoc->IncrMajorRevLevel();
    }
}

void CGbxProjView::DoBoardClone()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    size_t nBrd = m_listProj.GetItemSourceCode(nSel);

    CBoardManager* pBMgr = pDoc->GetBoardManager();

    ASSERT(nBrd < pBMgr->GetNumBoards());
    CBoard& pOrigBoard = pBMgr->GetBoard(nBrd);

    CBoard* pNewBoard = NULL;
    TRY
    {
        CGamDoc::SetLoadingVersionGuard setLoadingVersionGuard(NumVersion(fileGbxVerMajor, fileGbxVerMinor));
        CMemFile file;
        CArchive arSave(&file, CArchive::store);
        arSave.m_pDocument = pDoc;
        pOrigBoard.Serialize(arSave);      // Make a copy of the board
        arSave.Close();

        file.SeekToBegin();
        CArchive arRestore(&file, CArchive::load);
        arRestore.m_pDocument = pDoc;
        pNewBoard = new CBoard();
        pNewBoard->Serialize(arRestore);

        // We have a copy of the board. Now just change it's name
        // and give it a new serial number.
        CString strNewName;
        strNewName.LoadString(IDS_COPY_OF);
        strNewName += pNewBoard->GetName();
        pNewBoard->SetName(strNewName);
        pNewBoard->SetSerialNumber(pBMgr->IssueSerialNumber());

        // Add it to the board list
        pBMgr->Add(pNewBoard);
    }
    CATCH_ALL(e)
    {
        if (pNewBoard != NULL)
            delete pNewBoard;
        return;
    }
    END_CATCH_ALL

    pDoc->UpdateAllViews(NULL, HINT_UPDATEPROJVIEW);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoBoardEdit()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpBrd);
    size_t nBrd = m_listProj.GetItemSourceCode(nSel);

    CBoard& pBoard = pDoc->GetBoardManager()->GetBoard(nBrd);
    CView* pView = pDoc->FindBoardEditorView(pBoard);
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
        pDoc->CreateNewFrame(GetApp()->m_pMapViewTmpl, strTitle, &pBoard);
    }
}

void CGbxProjView::DoUpdateBoardHelpInfo()
{
// m_editInfo.SetWindowText("Board Help Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoUpdateBoardInfo()
{
// m_editInfo.SetWindowText("Board Info Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

/////////////////////////////////////////////////////////////////////
// Tile Image Support Methods

void CGbxProjView::DoTileManagerProperty()
{
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoTileGroupCreate()
{
    GetDocument()->DoCreateTileGroup();
}

void CGbxProjView::DoTileGroupProperty()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CTSetPropDialog dlg;
    CTileManager* pTMgr = pDoc->GetTileManager();
    dlg.m_crTrans = pTMgr->GetTransparentColor();
    dlg.m_strName = pTMgr->GetTileSet(nGrp).GetName();

    if (dlg.DoModal() == IDOK)
    {
        pTMgr->SetTransparentColor(dlg.m_crTrans);
        pTMgr->GetTileSet(nGrp).SetName(dlg.m_strName);
        pDoc->UpdateAllViews(NULL, HINT_TILESETPROPCHANGE, NULL);
        pDoc->SetModifiedFlag();
    }
}

void CGbxProjView::DoTileGroupDelete()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CString strTitle;
    m_listProj.GetItemText(nSel, strTitle);
    CString strPrompt;
    AfxFormatString1(strPrompt, IDP_DELETETILESET, strTitle);

    if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        pDoc->GetTileManager()->DeleteTileSet(nGrp);

        // Make sure no TileID references survive
        pDoc->NotifyTileDatabaseChange();

        // Make sure any views on the tileset informed.
        CGmBoxHint hint;
        hint.GetArgs<HINT_TILESETDELETED>().m_tileSet = nGrp;
        pDoc->UpdateAllViews(NULL, HINT_TILESETDELETED, &hint);
        pDoc->SetModifiedFlag();
        pDoc->IncrMajorRevLevel();
    }
}

void CGbxProjView::DoTileNew()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CNewTileDialog dlg;
    dlg.m_pBMgr = pDoc->GetBoardManager();

    CTileManager* pTMgr = pDoc->GetTileManager();

    if (dlg.DoModal() == IDOK)
    {
        TileID tidNew = pTMgr->CreateTile(nGrp,
            CSize(dlg.m_nWidth, dlg.m_nHeight),
            CSize(dlg.m_nHalfWidth, dlg.m_nHalfHeight),
            RGB(255, 255, 255));
        CGmBoxHint hint;
        hint.GetArgs<HINT_TILECREATED>().m_tid = tidNew;
        pDoc->UpdateAllViews(NULL, HINT_TILECREATED, &hint);
        pDoc->CreateNewFrame(GetApp()->m_pTileEditTmpl, "Tile Editor",
            reinterpret_cast<LPVOID>(value_preserving_cast<uintptr_t>(tidNew)));
        pDoc->SetModifiedFlag();
    }
}

void CGbxProjView::DoTileEdit()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);

    if (m_listTiles.GetSelCount() <= 0) return;

    std::vector<TileID> tidtbl;
    m_listTiles.GetCurMappedItemList(tidtbl);
    CTileManager* pTMgr = pDoc->GetTileManager();

    for (size_t i = 0; i < tidtbl.size(); i++)
    {
        TileID tid = tidtbl[i];

        CView *pView = pDoc->FindTileEditorView(tid);
        if (pView)
        {
            // Already has an editor. Activate that view.
            CFrameWnd* pFrm = pView->GetParentFrame();
            ASSERT(pFrm);
            pFrm->ActivateFrame();
        }
        else
        {
            pDoc->CreateNewFrame(GetApp()->m_pTileEditTmpl, "Tile Editor",
                reinterpret_cast<LPVOID>(value_preserving_cast<uintptr_t>(tid)));
        }
    }
}

void CGbxProjView::DoTileClone()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    if (m_listTiles.GetSelCount() <= 0) return;

    std::vector<TileID> tidtbl;
    m_listTiles.GetCurMappedItemList(tidtbl);
    CTileManager* pTMgr = pDoc->GetTileManager();

    for (size_t i = 0; i < tidtbl.size(); i++)
    {
        TileID tid = tidtbl[i];

        CTileUpdatable tileFull = pTMgr->GetTile(tid, fullScale);
        CTileUpdatable tileHalf = pTMgr->GetTile(tid, halfScale);
        CTile tileSmall = pTMgr->GetTile(tid, smallScale);

        TileID tidNew = pTMgr->CreateTile(nGrp, tileFull.GetSize(),
            tileHalf.GetSize(), tileSmall.GetSmallColor());

        OwnerPtr<CBitmap> bmap = tileFull.CreateBitmapOfTile();
        tileFull = pTMgr->GetTile(tidNew, fullScale);
        tileFull.Update(*bmap);

        bmap = tileHalf.CreateBitmapOfTile();
        tileHalf = pTMgr->GetTile(tidNew, halfScale);
        tileHalf.Update(*bmap);

        CGmBoxHint hint;
        hint.GetArgs<HINT_TILECREATED>().m_tid = tidNew;
        pDoc->UpdateAllViews(NULL, HINT_TILECREATED, &hint);

        pDoc->CreateNewFrame(GetApp()->m_pTileEditTmpl, "Tile Editor",
            reinterpret_cast<LPVOID>(value_preserving_cast<uintptr_t>(tidNew)));
    }
    pDoc->SetModifiedFlag();

}

void CGbxProjView::DoTileDelete()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);

    if (m_listTiles.GetSelCount() <= 0) return;

    std::vector<TileID> tidtbl;
    m_listTiles.GetCurMappedItemList(tidtbl);
    BOOL bTilesInUse = pDoc->QueryAnyOfTheseTilesInUse(tidtbl);
    if (bTilesInUse)
    {
        if (AfxMessageBox(IDS_TILEINUSE,
                MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2) != IDYES)
            return;
    }
    CTileManager* pTMgr = pDoc->GetTileManager();
    for (size_t i = 0; i < tidtbl.size(); i++)
    {
        pTMgr->DeleteTile(tidtbl[i]);
        CGmBoxHint hint;
        hint.GetArgs<HINT_TILEDELETED>().m_tid = tidtbl[i];
        pDoc->UpdateAllViews(NULL, HINT_TILEDELETED, &hint);
    }
    pDoc->NotifyTileDatabaseChange();
    pDoc->SetModifiedFlag();
    if (bTilesInUse)                        // (don't increase if tiles weren't used)
        pDoc->IncrMajorRevLevel();
}

void CGbxProjView::DoUpdateTileHelpInfo()
{
    // m_editInfo.SetWindowText("Tile Tips Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoUpdateTileList()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpTile);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    const CTileSet& pTSet = pDoc->GetTileManager()->GetTileSet(nGrp);
    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    m_listTiles.SetItemMap(&pLstMap);
}

/////////////////////////////////////////////////////////////////////
// Playing Piece Support Methods

void CGbxProjView::DoPieceGroupCreate()
{
    GetDocument()->DoCreatePieceGroup();
}

void CGbxProjView::DoPieceGroupProperty()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpPce);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CPieceManager* pPMgr = pDoc->GetPieceManager();

    CPiecePropDialog dlg;
    dlg.m_strName = pPMgr->GetPieceSet(nGrp).GetName();

    if (dlg.DoModal() == IDOK)
    {
        pPMgr->GetPieceSet(nGrp).SetName(dlg.m_strName);
        pDoc->UpdateAllViews(NULL, HINT_PIECESETPROPCHANGE, NULL);
        pDoc->SetModifiedFlag();
    }
}

void CGbxProjView::DoPieceGroupDelete()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpPce);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CString strTitle;
    m_listProj.GetItemText(nSel, strTitle);
    CString strPrompt;
    AfxFormatString1(strPrompt, IDP_DELETEPIECESET, strTitle);
    if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        pDoc->GetPieceManager()->DeletePieceSet(nGrp, &pDoc->GetGameStringMap());

        // Make sure any views on the tileset informed.
        CGmBoxHint hint;
        hint.GetArgs<HINT_PIECESETDELETED>().m_pieceSet = nGrp;
        pDoc->UpdateAllViews(NULL, HINT_PIECESETDELETED, &hint);
        pDoc->SetModifiedFlag();
        pDoc->IncrMajorRevLevel();
    }
}

void CGbxProjView::DoPieceNew()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpPce);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CPieceNewDialog dlg;
    dlg.m_pDoc = (CGamDoc*)GetDocument();
    dlg.m_nPSet = nGrp;

    dlg.DoModal();
    GetDocument()->UpdateAllViews(NULL, HINT_PIECESETPROPCHANGE, NULL);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoPieceEdit()
{
    CGamDoc* pDoc = GetDocument();

    if (m_listPieces.GetSelCount() == 1)
    {
        // Do full piece edit.
        int nSel;
        m_listPieces.GetSelItems(1, &nSel);
        if (nSel < 0)
            return;
        PieceID pid = m_listPieces.MapIndexToItem(value_preserving_cast<size_t>(nSel));

        CPieceEditDialog dlg;
        dlg.m_pDoc = pDoc;
        dlg.m_pid = pid;

        if (dlg.DoModal() != IDOK)
            return;
    }
    else
    {
        // Multiple selected. Do special piece edit.
        CArray<int, int> tblSel;
        int nNumSelected = m_listPieces.GetSelCount();
        tblSel.SetSize(nNumSelected);
        m_listPieces.GetSelItems(nNumSelected, tblSel.GetData());

        CPieceEditMultipleDialog dlg;
        if (dlg.DoModal() != IDOK)
            return;

        for (int i = 0; i < tblSel.GetSize(); i++)
        {
            PieceID pid = m_listPieces.MapIndexToItem(value_preserving_cast<size_t>(tblSel[i]));
            PieceDef& pDef = pDoc->GetPieceManager()->GetPiece(pid);
            // Process "top only visible" change
            if (pDef.Is2Sided() && dlg.m_bSetTopOnlyVisible)
            {
                pDef.m_flags &= ~PieceDef::flagShowOnlyVisibleSide;      // Initially clear the flag
                pDef.m_flags &= ~PieceDef::flagShowOnlyOwnersToo;
                if (dlg.m_bTopOnlyVisible)
                {
                    pDef.m_flags |= PieceDef::flagShowOnlyVisibleSide;   // Set the flag
                    if (dlg.m_bTopOnlyOwnersToo)
                        pDef.m_flags |= PieceDef::flagShowOnlyOwnersToo; // Set this flag too
                }
            }
            // Process front piece text change
            if (dlg.m_bSetFrontText)
            {
                GameElement elem = MakePieceElement(pid, 0);
                if (!dlg.m_strFront.IsEmpty())
                    pDoc->GetGameStringMap().SetAt(elem, dlg.m_strFront);
                else
                    pDoc->GetGameStringMap().RemoveKey(elem);
            }
            // Process back piece text change
            if (pDef.Is2Sided() && dlg.m_bSetBackText)
            {
                GameElement elem = MakePieceElement(pid, 1);
                if (!dlg.m_strBack.IsEmpty())
                    pDoc->GetGameStringMap().SetAt(elem, dlg.m_strBack);
                else
                    pDoc->GetGameStringMap().RemoveKey(elem);
            }
        }
    }
    pDoc->UpdateAllViews(NULL, HINT_PIECESETPROPCHANGE, NULL);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoPieceDelete()
{
    CGamDoc* pDoc = GetDocument();

    CArray<int, int> tblSel;
    int nNumSelected = m_listPieces.GetSelCount();
    if (nNumSelected == 0)
        return;

    tblSel.SetSize(nNumSelected);
    m_listPieces.GetSelItems(nNumSelected, tblSel.GetData());

    std::vector<PieceID> pieces(value_preserving_cast<size_t>(tblSel.GetSize()));
    for (int i = 0; i < tblSel.GetSize(); i++)                  // Map them all to piece ID's
        pieces[value_preserving_cast<size_t>(i)] = m_listPieces.MapIndexToItem(value_preserving_cast<size_t>(tblSel[i]));

    m_listPieces.SetItemMap(NULL);
    for (size_t i = 0; i < pieces.size(); i++)
        pDoc->GetPieceManager()->DeletePiece(pieces[i], &pDoc->GetGameStringMap());

    pDoc->UpdateAllViews(NULL, HINT_PIECEDELETED, NULL);
    pDoc->SetModifiedFlag();
    pDoc->IncrMajorRevLevel();
}

void CGbxProjView::DoUpdatePieceHelpInfo()
{
    // m_editInfo.SetWindowText("Piece Help Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoUpdatePieceList()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpPce);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CPieceSet& pPSet = pDoc->GetPieceManager()->GetPieceSet(nGrp);
    const std::vector<PieceID>& pLstMap = pPSet.GetPieceIDTable();
    m_listPieces.SetItemMap(&pLstMap);
}

/////////////////////////////////////////////////////////////////////
// Marker Support Methods

void CGbxProjView::DoMarkGroupCreate()
{
    GetDocument()->DoCreateMarkGroup();
}

void CGbxProjView::DoMarkGroupProperty()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpMark);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CMarkManager* pMMgr = pDoc->GetMarkManager();
    CMarkSet& pMSet = pMMgr->GetMarkSet(nGrp);

    CMarkerPropDialog dlg;
    dlg.m_strName = pMSet.GetName();
    dlg.m_nMarkerViz = (int)pMSet.GetMarkerTrayContentVisibility(); // zero based enum

    if (dlg.DoModal() == IDOK)
    {
        pMSet.SetName(dlg.m_strName);
        pMSet.SetMarkerTrayContentVisibility((MarkerTrayViz)dlg.m_nMarkerViz);

        pDoc->UpdateAllViews(NULL, HINT_MARKERSETPROPCHANGE, NULL);
        pDoc->SetModifiedFlag();
    }
}

void CGbxProjView::DoMarkGroupDelete()
{
    CGamDoc* pDoc = GetDocument();
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpMark);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CString strTitle;
    m_listProj.GetItemText(nSel, strTitle);
    CString strPrompt;
    AfxFormatString1(strPrompt, IDP_DELETEMARKSET, strTitle);
    if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {
        pDoc->GetMarkManager()->DeleteMarkSet(nGrp, &pDoc->GetGameStringMap());

        CGmBoxHint hint;
        hint.GetArgs<HINT_MARKSETDELETED>().m_markSet = nGrp;
        pDoc->UpdateAllViews(NULL, HINT_MARKSETDELETED, &hint);
        pDoc->SetModifiedFlag();
        pDoc->IncrMajorRevLevel();
    }
}

void CGbxProjView::DoMarkNew()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpMark);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CMarkerCreateDialog dlg;
    dlg.m_pDoc = (CGamDoc*)GetDocument();
    dlg.m_nMSet = nGrp;

    dlg.DoModal();
    pDoc->UpdateAllViews(NULL, HINT_MARKERSETPROPCHANGE, NULL);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoMarkEdit()
{
    CGamDoc* pDoc = GetDocument();

    if (m_listMarks.GetSelCount() == 1)
    {
        // Do full marker edit.
        int nSel;
        m_listMarks.GetSelItems(1, &nSel);
        if (nSel < 0)
            return;
        MarkID mid = m_listMarks.MapIndexToItem(value_preserving_cast<size_t>(nSel));

        CMarkerEditDialog dlg;
        dlg.m_pDoc = (CGamDoc*)GetDocument();
        dlg.m_mid = mid;

        if (dlg.DoModal() != IDOK)
            return;
    }
    else
    {
        // Multiple selected. Do special marker edit.
        CArray<int, int> tblSel;
        int nNumSelected = m_listMarks.GetSelCount();
        tblSel.SetSize(nNumSelected);
        m_listMarks.GetSelItems(nNumSelected, tblSel.GetData());

        CMarkerEditMultipleDialog dlg;
        if (dlg.DoModal() != IDOK)
            return;

        for (int i = 0; i < tblSel.GetSize(); i++)
        {
            MarkID mid = m_listMarks.MapIndexToItem(value_preserving_cast<size_t>(tblSel[i]));
            MarkDef& pDef = pDoc->GetMarkManager()->GetMark(mid);
            // Process "prompt for text" change
            if (dlg.m_bSetPromptForText)
            {
                pDef.m_flags &= ~MarkDef::flagPromptText;          // Initially clear the flag
                if (dlg.m_bPromptForText)
                    pDef.m_flags |= MarkDef::flagPromptText;       // Set the flag
            }
            // Process marker text change
            if (dlg.m_bSetText)
            {
                GameElement elem = MakeMarkerElement(mid);
                if (!dlg.m_strText.IsEmpty())
                    pDoc->GetGameStringMap().SetAt(elem, dlg.m_strText);
                else
                    pDoc->GetGameStringMap().RemoveKey(elem);
            }
        }
    }
    pDoc->UpdateAllViews(NULL, HINT_MARKERSETPROPCHANGE, NULL);
    pDoc->SetModifiedFlag();
}

void CGbxProjView::DoMarkDelete()
{
    CGamDoc* pDoc = GetDocument();

    CArray<int, int> tblSel;
    int nNumSelected = m_listMarks.GetSelCount();
    if (nNumSelected == 0)
        return;

    tblSel.SetSize(nNumSelected);
    m_listMarks.GetSelItems(nNumSelected, tblSel.GetData());

    std::vector<MarkID> markers(value_preserving_cast<size_t>(tblSel.GetSize()));
    for (int i = 0; i < tblSel.GetSize(); i++)                  // Map them all to piece ID's
        markers[value_preserving_cast<size_t>(i)] = m_listMarks.MapIndexToItem(value_preserving_cast<size_t>(tblSel[i]));

    m_listMarks.SetItemMap(NULL);
    for (size_t i = 0; i < markers.size(); i++)
        pDoc->GetMarkManager()->DeleteMark(markers[i], &pDoc->GetGameStringMap());

    pDoc->UpdateAllViews(NULL, HINT_MARKERDELETED, NULL);
    pDoc->SetModifiedFlag();
    pDoc->IncrMajorRevLevel();
}

void CGbxProjView::DoUpdateMarkHelpInfo()
{
    // m_editInfo.SetWindowText("Marker Help Coming Soon....\r\nTo a theatre near you!");
    m_editInfo.SetWindowText("");
}

void CGbxProjView::DoUpdateMarkList()
{
    CGamDoc* pDoc = GetDocument();

    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0);
    ASSERT(m_listProj.GetItemGroupCode(nSel) == grpMark);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CMarkSet& pMSet = pDoc->GetMarkManager()->GetMarkSet(nGrp);
    const std::vector<MarkID>& pLstMap = pMSet.GetMarkIDTable();
    m_listMarks.SetItemMap(&pLstMap);
}
