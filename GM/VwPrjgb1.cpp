// VwPrjgb1.cpp : Support file for vwprjgbx.cpp
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
#include    "FrmBited.h"
#include    "VwEdtbrd.h"
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
    GetDocument().DoGbxProperties();
}

void CGbxProjView::DoUpdateGbxInfo()
{
    CGamDoc& pDoc = GetDocument();
    CB::string str;
    if (!pDoc.m_strTitle.empty())
        str += "TITLE: " + pDoc.m_strTitle + "\r\n\r\n";
    if (!pDoc.m_strAuthor.empty())
        str += "AUTHOR: " + pDoc.m_strAuthor + "\r\n\r\n";
    if (!pDoc.m_strDescr.empty())
        str += "DESCRIPTION:\r\n\r\n" + pDoc.m_strDescr;
    m_editInfo->SetValue(str);
}

/////////////////////////////////////////////////////////////////////
// Playing Board Support Methods

void CGbxProjView::DoBoardCreate()
{
    GetDocument().DoCreateBoard();
}

void CGbxProjView::DoBoardProperty()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpBrd);
    size_t nBrd = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CBoard& pBoard = pDoc.GetBoardManager().GetBoard(nBrd);

    if (pDoc.DoBoardPropertyDialog(pBoard))
    {
        pDoc.SetModifiedFlag();
        DoUpdateProjectList();
    }
}

void CGbxProjView::DoBoardDelete()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpBrd);
    size_t nBrd = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CB::string strTitle = m_listProj->GetItemText(value_preserving_cast<size_t>(nSel));
    CB::string strPrompt = AfxFormatString1(IDP_DELETEBOARD, strTitle);
    if (wxMessageBox(strPrompt,
                        CB::GetAppName(),
                        wxYES_NO | wxICON_EXCLAMATION) == wxYES)
    {
        CGmBoxHint hint;
        hint.GetArgs<HINT_BOARDDELETED>().m_pBoard = &pDoc.GetBoardManager().GetBoard(nBrd);

        pDoc.GetBoardManager().DeleteBoard(nBrd);

        // Make sure any views on the board are closed.
        pDoc.UpdateAllViews(NULL, HINT_BOARDDELETED, &hint);

        pDoc.SetModifiedFlag();
        pDoc.IncrMajorRevLevel();
    }
}

void CGbxProjView::DoBoardClone()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel != wxNOT_FOUND);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpBrd);
    size_t nBrd = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CBoardManager& pBMgr = pDoc.GetBoardManager();

    wxASSERT(nBrd < pBMgr.GetNumBoards());
    CBoard& pOrigBoard = pBMgr.GetBoard(nBrd);

    TRY
    {
        CGamDoc::SetLoadingVersionGuard setLoadingVersionGuard(NumVersion(fileGbxVerMajor, fileGbxVerMinor));
        CMemFile file;
        CArchive arSave(&file, CArchive::store);
        arSave.m_pDocument = pDoc;
        SetFileFeaturesGuard setFileFeaturesGuard(arSave, GetCBFeatures());
        pOrigBoard.Serialize(arSave);      // Make a copy of the board
        arSave.Close();

        file.SeekToBegin();
        CArchive arRestore(&file, CArchive::load);
        arRestore.m_pDocument = pDoc;
        OwnerPtr<CBoard> pNewBoard = new CBoard();
        pNewBoard->Serialize(arRestore);

        // We have a copy of the board. Now just change it's name
        // and give it a new serial number.
        CB::string strNewName = CB::string::LoadString(IDS_COPY_OF);
        strNewName += pNewBoard->GetName();
        pNewBoard->SetName(strNewName);
        pNewBoard->SetSerialNumber(pBMgr.IssueSerialNumber());

        // Add it to the board list
        pBMgr.Add(std::move(pNewBoard));
    }
    CATCH_ALL(e)
    {
        return;
    }
    END_CATCH_ALL

    pDoc.UpdateAllViews(NULL, HINT_UPDATEPROJVIEW);
    pDoc.SetModifiedFlag();
}

void CGbxProjView::DoBoardEdit()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpBrd);
    size_t nBrd = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CBoard& pBoard = pDoc.GetBoardManager().GetBoard(nBrd);
    wxBrdEditView* pView = pDoc.FindBoardEditorView(pBoard);
    if (pView != NULL)
    {
        // This board already has an editor. Activate that view.
        pView->GetFrame().Activate();
    }
    else
    {
        wxBrdEditView::New(pDoc, pBoard);
    }
}

void CGbxProjView::DoUpdateBoardHelpInfo()
{
// m_editInfo.SetWindowText("Board Help Coming Soon....\r\nTo a theatre near you!");
    m_editInfo->SetValue(""_cbstring);
}

void CGbxProjView::DoUpdateBoardInfo()
{
// m_editInfo.SetWindowText("Board Info Coming Soon....\r\nTo a theatre near you!");
    m_editInfo->SetValue(""_cbstring);
}

/////////////////////////////////////////////////////////////////////
// Tile Image Support Methods

void CGbxProjView::DoTileManagerProperty()
{
    m_editInfo->SetValue(""_cbstring);
}

void CGbxProjView::DoTileGroupCreate()
{
    GetDocument().DoCreateTileGroup();
}

void CGbxProjView::DoTileGroupProperty()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CTSetPropDialog dlg;
    CTileManager& pTMgr = pDoc.GetTileManager();
    dlg.m_crTrans = CB::Convert(pTMgr.GetTransparentColor());
    dlg.m_strName = wxString(pTMgr.GetTileSet(nGrp).GetName());

    if (dlg.ShowModal() == wxID_OK)
    {
        pTMgr.SetTransparentColor(CB::Convert(dlg.m_crTrans));
        pTMgr.GetTileSet(nGrp).SetName(dlg.m_strName);
        pDoc.UpdateAllViews(NULL, HINT_TILESETPROPCHANGE, NULL);
        pDoc.SetModifiedFlag();
    }
}

void CGbxProjView::DoTileGroupDelete()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CB::string strTitle = m_listProj->GetItemText(value_preserving_cast<size_t>(nSel));
    CB::string strPrompt = AfxFormatString1(IDP_DELETETILESET, strTitle);

    if (wxMessageBox(strPrompt,
                    CB::GetAppName(),
                    wxYES_NO | wxICON_EXCLAMATION) == wxYES)
    {
        pDoc.GetTileManager().DeleteTileSet(nGrp);

        // Make sure no TileID references survive
        pDoc.NotifyTileDatabaseChange();

        // Make sure any views on the tileset informed.
        CGmBoxHint hint;
        hint.GetArgs<HINT_TILESETDELETED>().m_tileSet = nGrp;
        pDoc.UpdateAllViews(NULL, HINT_TILESETDELETED, &hint);
        pDoc.SetModifiedFlag();
        pDoc.IncrMajorRevLevel();
    }
}

void CGbxProjView::DoTileNew()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CNewTileDialog dlg;
    dlg.m_pBMgr = &pDoc.GetBoardManager();

    CTileManager& pTMgr = pDoc.GetTileManager();

    if (dlg.ShowModal() == wxID_OK)
    {
        TileID tidNew = pTMgr.CreateTile(nGrp,
            CSize(dlg.m_nWidth, dlg.m_nHeight),
            CSize(dlg.m_nHalfWidth, dlg.m_nHalfHeight),
            RGB(255, 255, 255));
        CGmBoxHint hint;
        hint.GetArgs<HINT_TILECREATED>().m_tid = tidNew;
        pDoc.UpdateAllViews(NULL, HINT_TILECREATED, &hint);
        wxBitEditView::New(pDoc, tidNew);
        pDoc.SetModifiedFlag();
    }
}

void CGbxProjView::DoTileEdit()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile);

    if (m_listTiles->GetSelectedCount() <= 0) return;

    std::vector<TileID> tidtbl = m_listTiles->GetCurMappedItemList();

    for (size_t i = 0; i < tidtbl.size(); i++)
    {
        TileID tid = tidtbl[i];

        wxBitEditView* pView = pDoc.FindTileEditorView(tid);
        if (pView != NULL)
        {
            // Already has an editor. Activate that view.
            pView->GetFrame().Activate();
        }
        else
        {
            wxBitEditView::New(pDoc, tid);
        }
    }
}

void CGbxProjView::DoTileClone()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    if (m_listTiles->GetSelectedCount() <= 0) return;

    std::vector<TileID> tidtbl = m_listTiles->GetCurMappedItemList();
    CTileManager& pTMgr = pDoc.GetTileManager();

    for (size_t i = 0; i < tidtbl.size(); i++)
    {
        TileID tid = tidtbl[i];

        CTileUpdatable tileFull = pTMgr.GetTile(tid, fullScale);
        CTileUpdatable tileHalf = pTMgr.GetTile(tid, halfScale);
        CTile tileSmall = pTMgr.GetTile(tid, smallScale);

        TileID tidNew = pTMgr.CreateTile(nGrp, tileFull.GetSize(),
            tileHalf.GetSize(), tileSmall.GetSmallColor());

        OwnerPtr<CBitmap> bmap = tileFull.CreateBitmapOfTile();
        tileFull = pTMgr.GetTile(tidNew, fullScale);
        tileFull.Update(*bmap);

        bmap = tileHalf.CreateBitmapOfTile();
        tileHalf = pTMgr.GetTile(tidNew, halfScale);
        tileHalf.Update(*bmap);

        CGmBoxHint hint;
        hint.GetArgs<HINT_TILECREATED>().m_tid = tidNew;
        pDoc.UpdateAllViews(NULL, HINT_TILECREATED, &hint);

        wxBitEditView::New(pDoc, tidNew);
    }
    pDoc.SetModifiedFlag();

}

void CGbxProjView::DoTileDelete()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile);

    if (m_listTiles->GetSelectedCount() <= 0) return;

    std::vector<TileID> tidtbl = m_listTiles->GetCurMappedItemList();
    BOOL bTilesInUse = pDoc.QueryAnyOfTheseTilesInUse(tidtbl);
    if (bTilesInUse)
    {
        if (wxMessageBox(CB::string::LoadString(IDS_TILEINUSE),
                            CB::GetAppName(),
                            wxYES_NO | wxICON_EXCLAMATION | wxNO_DEFAULT) != wxYES)
            return;
    }
    CTileManager& pTMgr = pDoc.GetTileManager();
    for (size_t i = size_t(0); i < tidtbl.size(); i++)
    {
        pTMgr.DeleteTile(tidtbl[i]);
        CGmBoxHint hint;
        hint.GetArgs<HINT_TILEDELETED>().m_tid = tidtbl[i];
        pDoc.UpdateAllViews(NULL, HINT_TILEDELETED, &hint);
    }
    pDoc.NotifyTileDatabaseChange();
    pDoc.SetModifiedFlag();
    if (bTilesInUse)                        // (don't increase if tiles weren't used)
        pDoc.IncrMajorRevLevel();
}

void CGbxProjView::DoUpdateTileHelpInfo()
{
    // m_editInfo.SetWindowText("Tile Tips Coming Soon....\r\nTo a theatre near you!");
    m_editInfo->SetValue(""_cbstring);
}

void CGbxProjView::DoUpdateTileList()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    const CTileSet& pTSet = pDoc.GetTileManager().GetTileSet(nGrp);
    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    m_listTiles->SetItemMap(&pLstMap);
}

/////////////////////////////////////////////////////////////////////
// Playing Piece Support Methods

void CGbxProjView::DoPieceGroupCreate()
{
    GetDocument().DoCreatePieceGroup();
}

void CGbxProjView::DoPieceGroupProperty()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpPce);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CPieceManager& pPMgr = pDoc.GetPieceManager();

    CPiecePropDialog dlg;
    dlg.m_strName = wxString(pPMgr.GetPieceSet(nGrp).GetName());

    if (dlg.ShowModal() == wxID_OK)
    {
        pPMgr.GetPieceSet(nGrp).SetName(dlg.m_strName);
        pDoc.UpdateAllViews(NULL, HINT_PIECESETPROPCHANGE, NULL);
        pDoc.SetModifiedFlag();
    }
}

void CGbxProjView::DoPieceGroupDelete()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpPce);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CB::string strTitle = m_listProj->GetItemText(value_preserving_cast<size_t>(nSel));
    CB::string strPrompt = AfxFormatString1(IDP_DELETEPIECESET, strTitle);
    if (wxMessageBox(strPrompt,
                        CB::GetAppName(),
                        wxYES_NO | wxICON_EXCLAMATION) == wxYES)
    {
        pDoc.GetPieceManager().DeletePieceSet(nGrp, &pDoc.GetGameStringMap());

        // Make sure any views on the tileset informed.
        CGmBoxHint hint;
        hint.GetArgs<HINT_PIECESETDELETED>().m_pieceSet = nGrp;
        pDoc.UpdateAllViews(NULL, HINT_PIECESETDELETED, &hint);
        pDoc.SetModifiedFlag();
        pDoc.IncrMajorRevLevel();
    }
}

void CGbxProjView::DoPieceNew()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpPce);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CPieceNewDialog dlg(pDoc, nGrp);

    dlg.ShowModal();
    pDoc.UpdateAllViews(NULL, HINT_PIECESETPROPCHANGE, NULL);
    pDoc.SetModifiedFlag();
}

void CGbxProjView::DoPieceEdit()
{
    CGamDoc& pDoc = GetDocument();

    if (m_listPieces->GetSelectedCount() == 1)
    {
        // Do full piece edit.
        std::vector<size_t> nSel = m_listPieces->GetSelections();
        PieceID pid = m_listPieces->MapIndexToItem(nSel.front());

        CPieceEditDialog dlg(pDoc, pid);

        if (dlg.ShowModal() != wxID_OK)
            return;
    }
    else
    {
        // Multiple selected. Do special piece edit.
        size_t nNumSelected = m_listPieces->GetSelectedCount();
        if (!nNumSelected)
        {
            return;
        }
        std::vector<size_t> tblSel = m_listPieces->GetSelections();

        std::vector<PieceID> pids(tblSel.size());
        std::vector<RefPtr<PieceDef>> pDefs;
        pDefs.reserve(tblSel.size());
        size_t sides = size_t(0);
        for (size_t i = size_t(0) ; i < tblSel.size() ; ++i)
        {
            pids[i] = m_listPieces->MapIndexToItem(value_preserving_cast<size_t>(tblSel[i]));
            pDefs.push_back(&pDoc.GetPieceManager().GetPiece(pids[i]));
            sides = CB::max(sides, pDefs[i]->GetSides());
        }

        CPieceEditMultipleDialog dlg(sides);
        if (dlg.ShowModal() != wxID_OK)
            return;

        for (size_t i = size_t(0) ; i < tblSel.size() ; ++i)
        {
            PieceID pid = pids[i];
            PieceDef& pDef = *pDefs[i];
            // Process "top only visible" change
            if (pDef.GetSides() >= size_t(2) && dlg.m_bSetTopOnlyVisible)
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
            // Process piece text changes
            for (size_t j = size_t(0) ; j < pDef.GetSides() ; ++j)
            {
                if (dlg.m_bSetTexts[j])
                {
                    GameElement elem = MakePieceElement(pid, value_preserving_cast<unsigned>(j));
                    if (!dlg.m_strs[j].empty())
                        pDoc.GetGameStringMap().SetAt(elem, dlg.m_strs[j]);
                    else
                        pDoc.GetGameStringMap().RemoveKey(elem);
                }
            }
        }
    }
    pDoc.UpdateAllViews(NULL, HINT_PIECESETPROPCHANGE, NULL);
    pDoc.SetModifiedFlag();
}

void CGbxProjView::DoPieceDelete()
{
    CGamDoc& pDoc = GetDocument();

    size_t nNumSelected = m_listPieces->GetSelectedCount();
    if (nNumSelected == size_t(0))
        return;

    std::vector<size_t> tblSel = m_listPieces->GetSelections();

    std::vector<PieceID> pieces(value_preserving_cast<size_t>(tblSel.size()));
    for (size_t i = size_t(0); i < tblSel.size(); ++i)                  // Map them all to piece ID's
        pieces[i] = m_listPieces->MapIndexToItem(tblSel[i]);

    m_listPieces->SetItemMap(NULL);
    for (size_t i = size_t(0); i < pieces.size(); i++)
        pDoc.GetPieceManager().DeletePiece(pieces[i], &pDoc.GetGameStringMap());

    pDoc.UpdateAllViews(NULL, HINT_PIECEDELETED, NULL);
    pDoc.SetModifiedFlag();
    pDoc.IncrMajorRevLevel();
}

void CGbxProjView::DoUpdatePieceHelpInfo()
{
    // m_editInfo.SetWindowText("Piece Help Coming Soon....\r\nTo a theatre near you!");
    m_editInfo->SetValue(""_cbstring);
}

void CGbxProjView::DoUpdatePieceList()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpPce);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CPieceSet& pPSet = pDoc.GetPieceManager().GetPieceSet(nGrp);
    const std::vector<PieceID>& pLstMap = pPSet.GetPieceIDTable();
    m_listPieces->SetItemMap(&pLstMap);
}

/////////////////////////////////////////////////////////////////////
// Marker Support Methods

void CGbxProjView::DoMarkGroupCreate()
{
    GetDocument().DoCreateMarkGroup();
}

void CGbxProjView::DoMarkGroupProperty()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpMark);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CMarkManager& pMMgr = pDoc.GetMarkManager();
    CMarkSet& pMSet = pMMgr.GetMarkSet(nGrp);

    CMarkerPropDialog dlg;
    dlg.m_strName = wxString(pMSet.GetName());
    dlg.m_nMarkerViz = pMSet.GetMarkerTrayContentVisibility(); // zero based enum

    if (dlg.ShowModal() == wxID_OK)
    {
        pMSet.SetName(dlg.m_strName);
        pMSet.SetMarkerTrayContentVisibility(dlg.m_nMarkerViz);

        pDoc.UpdateAllViews(NULL, HINT_MARKERSETPROPCHANGE, NULL);
        pDoc.SetModifiedFlag();
    }
}

void CGbxProjView::DoMarkGroupDelete()
{
    CGamDoc& pDoc = GetDocument();
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpMark);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CB::string strTitle = m_listProj->GetItemText(value_preserving_cast<size_t>(nSel));
    CB::string strPrompt = AfxFormatString1(IDP_DELETEMARKSET, strTitle);
    if (wxMessageBox(strPrompt,
                        CB::GetAppName(),
                        wxYES_NO | wxICON_EXCLAMATION) == wxYES)
    {
        pDoc.GetMarkManager().DeleteMarkSet(nGrp, &pDoc.GetGameStringMap());

        CGmBoxHint hint;
        hint.GetArgs<HINT_MARKSETDELETED>().m_markSet = nGrp;
        pDoc.UpdateAllViews(NULL, HINT_MARKSETDELETED, &hint);
        pDoc.SetModifiedFlag();
        pDoc.IncrMajorRevLevel();
    }
}

void CGbxProjView::DoMarkNew()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpMark);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CMarkerCreateDialog dlg;
    dlg.m_pDoc = &pDoc;
    dlg.m_nMSet = nGrp;

    dlg.ShowModal();
    pDoc.UpdateAllViews(NULL, HINT_MARKERSETPROPCHANGE, NULL);
    pDoc.SetModifiedFlag();
}

void CGbxProjView::DoMarkEdit()
{
    CGamDoc& pDoc = GetDocument();

    if (m_listMarks->GetSelectedCount() == 1)
    {
        // Do full marker edit.
        std::vector<size_t> nSel = m_listMarks->GetSelections();
        MarkID mid = m_listMarks->MapIndexToItem(nSel.front());

        CMarkerEditDialog dlg;
        dlg.m_pDoc = &pDoc;
        dlg.m_mid = mid;

        if (dlg.ShowModal() != wxID_OK)
            return;
    }
    else
    {
        // Multiple selected. Do special marker edit.
        std::vector<size_t> tblSel = m_listMarks->GetSelections();

        CMarkerEditMultipleDialog dlg;
        if (dlg.ShowModal() != wxID_OK)
            return;

        for (size_t i = size_t(0) ; i < tblSel.size() ; ++i)
        {
            MarkID mid = m_listMarks->MapIndexToItem(tblSel[i]);
            MarkDef& pDef = pDoc.GetMarkManager().GetMark(mid);
            // Process "prompt for text" change
            if (dlg.m_bPromptForText != wxCHK_UNDETERMINED)
            {
                pDef.m_flags &= ~MarkDef::flagPromptText;          // Initially clear the flag
                if (dlg.m_bPromptForText == wxCHK_CHECKED)
                    pDef.m_flags |= MarkDef::flagPromptText;       // Set the flag
            }
            // Process marker text change
            if (dlg.m_bSetText)
            {
                GameElement elem = MakeMarkerElement(mid);
                if (!dlg.m_strText.empty())
                    pDoc.GetGameStringMap().SetAt(elem, CB::string(dlg.m_strText));
                else
                    pDoc.GetGameStringMap().RemoveKey(elem);
            }
        }
    }
    pDoc.UpdateAllViews(NULL, HINT_MARKERSETPROPCHANGE, NULL);
    pDoc.SetModifiedFlag();
}

void CGbxProjView::DoMarkDelete()
{
    CGamDoc& pDoc = GetDocument();

    size_t nNumSelected = m_listMarks->GetSelectedCount();
    if (nNumSelected == size_t(0))
        return;

    std::vector<size_t> tblSel = m_listMarks->GetSelections();

    std::vector<MarkID> markers(tblSel.size());
    for (size_t i = size_t(0); i < tblSel.size(); ++i)                  // Map them all to piece ID's
        markers[i] = m_listMarks->MapIndexToItem(tblSel[i]);

    m_listMarks->SetItemMap(NULL);
    for (size_t i = size_t(0); i < markers.size(); i++)
        pDoc.GetMarkManager().DeleteMark(markers[i], &pDoc.GetGameStringMap());

    pDoc.UpdateAllViews(NULL, HINT_MARKERDELETED, NULL);
    pDoc.SetModifiedFlag();
    pDoc.IncrMajorRevLevel();
}

void CGbxProjView::DoUpdateMarkHelpInfo()
{
    // m_editInfo.SetWindowText("Marker Help Coming Soon....\r\nTo a theatre near you!");
    m_editInfo->SetValue(""_cbstring);
}

void CGbxProjView::DoUpdateMarkList()
{
    CGamDoc& pDoc = GetDocument();

    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0);
    wxASSERT(m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpMark);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CMarkSet& pMSet = pDoc.GetMarkManager().GetMarkSet(nGrp);
    const std::vector<MarkID>& pLstMap = pMSet.GetMarkIDTable();
    m_listMarks->SetItemMap(&pLstMap);
}
