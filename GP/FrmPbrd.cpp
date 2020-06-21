// FrmPbrd.cpp : implementation file
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
#include    "GamDoc.h"
#include    "FrmPbrd.h"
#include    "VwPbrd.h"
#include    "VwTbrd.h"
#include    "VwSelpce.h"
#include    "WinState.h"
#include    "Board.h"
#include    "Player.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPlayBoardFrame, CMDIChildWndEx)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CPlayBoardFrame, CMDIChildWndEx)
    //{{AFX_MSG_MAP(CPlayBoardFrame)
    ON_COMMAND(ID_VIEW_HALFSCALEBRD, OnViewHalfScaleBrd)
    ON_UPDATE_COMMAND_UI(ID_VIEW_HALFSCALEBRD, OnUpdateViewHalfScaleBrd)
    ON_COMMAND(ID_VIEW_FULLSCALEBRD, OnViewFullScaleBrd)
    ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCALEBRD, OnUpdateViewFullScaleBrd)
    ON_WM_CLOSE()
    ON_COMMAND(ID_VIEW_SNAPGRID, OnViewSnapGrid)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SNAPGRID, OnUpdateViewSnapGrid)
    ON_COMMAND(ID_EDIT_SELALLMARKERS, OnEditSelAllMarkers)
    ON_COMMAND(ID_EDIT_BRDPROP, OnEditBoardProperties)
    ON_COMMAND(ID_VIEW_PIECES, OnViewPieces)
    ON_UPDATE_COMMAND_UI(ID_VIEW_PIECES, OnUpdateViewPieces)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELALLMARKERS, OnUpdateEditSelAllMarkers)
    ON_COMMAND(ID_VIEW_TOGGLESCALE, OnViewToggleScale)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TOGGLESCALE, OnUpdateViewToggleScale)
    ON_COMMAND(ID_ACT_STACK, OnActStack)
    ON_UPDATE_COMMAND_UI(ID_ACT_STACK, OnUpdateActStack)
    ON_COMMAND(ID_ACT_TOBACK, OnActToBack)
    ON_UPDATE_COMMAND_UI(ID_ACT_TOBACK, OnUpdateActToBack)
    ON_COMMAND(ID_ACT_TOFRONT, OnActToFront)
    ON_UPDATE_COMMAND_UI(ID_ACT_TOFRONT, OnUpdateActToFront)
    ON_COMMAND(ID_ACT_TURNOVER, OnActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER, OnUpdateActTurnOver)
    ON_COMMAND(ID_PTOOL_PLOTMOVE, OnActPlotMove)
    ON_UPDATE_COMMAND_UI(ID_PTOOL_PLOTMOVE, OnUpdateActPlotMove)
    ON_COMMAND(ID_ACT_PLOTDONE, OnActPlotDone)
    ON_UPDATE_COMMAND_UI(ID_ACT_PLOTDONE, OnUpdateActPlotDone)
    ON_COMMAND(ID_ACT_PLOTDISCARD, OnActPlotDiscard)
    ON_UPDATE_COMMAND_UI(ID_ACT_PLOTDISCARD, OnUpdateActPlotDiscard)
    ON_COMMAND(ID_VIEW_SPLITBOARDROWS, OnViewSplitBoardRows)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SPLITBOARDROWS, OnUpdateViewSplitBoardRows)
    ON_COMMAND(ID_VIEW_SPLITBOARDCOLS, OnViewSplitBoardCols)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SPLITBOARDCOLS, OnUpdateViewSplitBoardCols)
    //}}AFX_MSG_MAP
    ON_COMMAND_RANGE(ID_MRKGROUP_FIRST, ID_MRKGROUP_FIRST + 64, OnSelectGroupMarkers)
    ON_UPDATE_COMMAND_UI_RANGE(ID_MRKGROUP_FIRST, ID_MRKGROUP_FIRST + 64, OnUpdateSelectGroupMarkers)
    // Other messages
    ON_MESSAGE(WM_CENTERBOARDONPOINT, OnMessageCenterBoardOnPoint)
    ON_MESSAGE(WM_WINSTATE, OnMessageWindowState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardFrame

CPlayBoardFrame::CPlayBoardFrame()
{
    m_pPBoard = NULL;
}

CPlayBoardFrame::~CPlayBoardFrame()
{
}

BOOL CPlayBoardFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CMDIChildWndEx::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)GetStockObject(LTGRAY_BRUSH),
        AfxGetApp()->LoadIcon(IDR_GP_BOARDVIEW));

    cs.style |= WS_CLIPCHILDREN;
    cs.style &= ~(DWORD)FWS_ADDTOTITLE;
    return TRUE;
}

void CPlayBoardFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    CGamDoc* pDoc = (CGamDoc*)GetActiveDocument();
    CString str = pDoc->GetTitle();

    CString strBoardName = m_pPBoard->GetBoard()->GetName();
    str += " - " + strBoardName;

    if (m_pPBoard->IsOwned())
    {
        str += " - ";
        CString strOwnerName = pDoc->GetPlayerManager()->
            GetPlayerUsingMask(m_pPBoard->GetOwnerMask()).m_strName;
        CString strOwnedBy;
        strOwnedBy.Format(IDS_TIP_OWNED_BY_UC, (LPCTSTR)strOwnerName);
        str += strOwnedBy;
    }

    SetWindowText(str);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CPlayBoardFrame::OnCreateClient(LPCREATESTRUCT lpcs,
     CCreateContext* pContext)
{
    CGamDoc* pDoc = (CGamDoc*)pContext->m_pCurrentDoc;
    m_pPBoard = (CPlayBoard*)pDoc->GetNewViewParameter();
    ASSERT(m_pPBoard != NULL);

    // Create a splitter with 1 row, 2 columns
    if (!m_wndSplitter1.CreateStatic(this, 1, 2))
    {
        TRACE("Failed to create static splitter\n");
        return FALSE;
    }

    CRect rct;
    GetClientRect(&rct);
    int xSize = (int)((85L * rct.Width()) / 100);
    int ySize = rct.Height() / 2;

    m_wndSplitter1.SetColumnInfo(0, xSize, 0);
    m_wndSplitter1.SetColumnInfo(1, rct.Width() - xSize, 0);

    if (!m_wndSplitBoards.CreateStatic(&m_wndSplitter1, 2, 2, WS_CHILD | WS_VISIBLE,
        m_wndSplitter1.IdFromRowCol(0, 0)))
    {
        TRACE("Failed to create board view splitter\n");
        return FALSE;
    }

    if (!m_wndSplitBoards.CreateView(0, 0,
        pContext->m_pNewViewClass, CSize(xSize / 2, ySize / 2), pContext))
    {
        TRACE("Failed to create first board pane\n");
        return FALSE;
    }

    if (!m_wndSplitBoards.CreateView(0, 1,
        pContext->m_pNewViewClass, CSize(xSize / 2, ySize / 2), pContext))
    {
        TRACE("Failed to create second board pane\n");
        return FALSE;
    }
    if (!m_wndSplitBoards.CreateView(1, 0,
        pContext->m_pNewViewClass, CSize(xSize / 2, ySize / 2), pContext))
    {
        TRACE("Failed to create third board pane\n");
        return FALSE;
    }
    if (!m_wndSplitBoards.CreateView(1, 1,
        pContext->m_pNewViewClass, CSize(xSize / 2, ySize / 2), pContext))
    {
        TRACE("Failed to create fourth board pane\n");
        return FALSE;
    }

    m_wndSplitBoards.HideColumn(1);
    m_wndSplitBoards.HideRow(1);

    // Add the second splitter pane - which is a nested splitter with 2 rows
    if (!m_wndSplitter2.CreateStatic(
        &m_wndSplitter1,        // Our parent window is the first splitter
        2, 1,                   // The new splitter is 2 rows, 1 column
        WS_CHILD | WS_VISIBLE | WS_BORDER,  // Style, WS_BORDER is needed
        m_wndSplitter1.IdFromRowCol(0, 1)
            // New splitter is in the first row, 2nd column of first splitter
       ))
    {
        TRACE("Failed to create nested splitter\n");
        return FALSE;
    }

    if (!m_wndSplitter2.CreateView(0, 0,
        RUNTIME_CLASS(CSelectedPieceView),
        CSize(rct.Width() - xSize, ySize), pContext))
    {
        TRACE("Failed to create second Selected Piece pane\n");
        return FALSE;
    }
    if (!m_wndSplitter2.CreateView(1, 0,
        RUNTIME_CLASS(CTinyBoardView),
        CSize(rct.Width() - xSize, rct.Height() - ySize), pContext))
    {
        TRACE("Failed to create small scale map pane\n");
        return FALSE;
    }

    if (!m_pPBoard->m_bShowSelListAndTinyMap)
        m_wndSplitter1.HideColumn(1);

    return TRUE;
}

///////////////////////////////////////////////////////////////////////
// The WM_WINSTATE message is sent when a document is being saved.
// WPARAM = CArchive*, LPARAM = 0 if save, 1 if restore

#define SCHEMA_BRDVIEW_SAVE     1

LRESULT CPlayBoardFrame::OnMessageWindowState(WPARAM wParam, LPARAM lParam)
{
    ASSERT(wParam != NULL);
    CArchive& ar = *((CArchive*)wParam);

    int nRowCount;
    int nColCount;
    int nRow;
    int nCol;
    int nCur;
    int nMin;

    // Process splitter windows...
    if (ar.IsStoring())
    {
        ar << (WORD)SCHEMA_BRDVIEW_SAVE;        // Object versioning

        // The outermost splitter info.
        m_wndSplitter1.GetColumnInfo(0, nCur, nMin);
        ar << (DWORD)nCur;
        ar << (DWORD)nMin;

        nColCount = m_wndSplitter1.GetColumnCount();
        ar << (WORD)nColCount;

        // The right splitter area info
        if (nColCount > 1)
        {
            m_wndSplitter1.GetColumnInfo(1, nCur, nMin);    // Ver 2.90
            ar << (DWORD)nCur;
            ar << (DWORD)nMin;

            m_wndSplitter2.GetRowInfo(0, nCur, nMin);
            ar << (DWORD)nCur;
            ar << (DWORD)nMin;
        }

        // Dimensions of board splits
        nRowCount = m_wndSplitBoards.GetRowCount();
        ASSERT(nRowCount <= 2);
        nColCount = m_wndSplitBoards.GetColumnCount();
        ASSERT(nColCount <= 2);

        ar << (WORD)nRowCount;
        ar << (WORD)nColCount;

        m_wndSplitBoards.GetActivePane(&nRow, &nCol);
        ar << (WORD)nRow;
        ar << (WORD)nCol;

        // Row info for top row
        m_wndSplitBoards.GetRowInfo(0, nCur, nMin);
        ar << (DWORD)nCur;
        ar << (DWORD)nMin;

        // Column info for left column
        m_wndSplitBoards.GetColumnInfo(0, nCur, nMin);
        ar << (DWORD)nCur;
        ar << (DWORD)nMin;
    }
    else
    {
        WORD  wSchema;
        WORD  wTmp;
        DWORD dwTmp;

        ar >> wSchema;                      // Get object schema
        if (wSchema > SCHEMA_BRDVIEW_SAVE)
        {
            ASSERT(wSchema <= SCHEMA_BRDVIEW_SAVE);
            return (LRESULT)0;              // Don't understand object. Ignore
        }

        // The outermost splitter info.
        ar >> dwTmp; nCur = (int)dwTmp;
        ar >> dwTmp; nMin = (int)dwTmp;
        m_wndSplitter1.SetColumnInfo(0, nCur, nMin);
        m_wndSplitter1.RecalcLayout();

        ar >> wTmp; nColCount = (int)wTmp;

        // The right splitter area info
        if (nColCount > 1)
        {
            ar >> dwTmp; nCur = (int)dwTmp;
            ar >> dwTmp; nMin = (int)dwTmp;
            m_wndSplitter1.SetColumnInfo(1, nCur, nMin);
            m_wndSplitter1.RecalcLayout();
            ar >> dwTmp; nCur = (int)dwTmp;
            ar >> dwTmp; nMin = (int)dwTmp;
            m_wndSplitter2.SetRowInfo(0, nCur, nMin);
            m_wndSplitter2.RecalcLayout();
        }

        // Dimensions of board splits
        ar >> wTmp; nRowCount = (int)wTmp;
        ar >> wTmp; nColCount = (int)wTmp;

        if (nRowCount > 1)
            m_wndSplitBoards.ShowRow();
        if (nColCount > 1)
            m_wndSplitBoards.ShowColumn();

        // The active pane restoration is the last thing done.
        ar >> wTmp; nRow = (int)wTmp;
        ar >> wTmp; nCol = (int)wTmp;

        // Row info for top row
        ar >> dwTmp; nCur = (int)dwTmp;
        ar >> dwTmp; nMin = (int)dwTmp;
        m_wndSplitBoards.SetRowInfo(0, nCur, nMin);

        // Column info for left column
        ar >> dwTmp; nCur = (int)dwTmp;
        ar >> dwTmp; nMin = (int)dwTmp;
        m_wndSplitBoards.SetColumnInfo(0, nCur, nMin);

        m_wndSplitBoards.RecalcLayout();
    }

    // ... ROW 0 PROCESSING ...
    // Upper-left board view..
    CWnd* pWnd = m_wndSplitBoards.GetPane(0, 0);
    ASSERT(pWnd != NULL);
    pWnd->SendMessage(WM_WINSTATE, wParam, lParam);

    if (nColCount > 1)
    {
        // Upper-right board view...
        pWnd = m_wndSplitBoards.GetPane(0, 1);
        ASSERT(pWnd != NULL);
        pWnd->SendMessage(WM_WINSTATE, wParam, lParam);
    }

    // ... ROW 1 PROCESSING ...
    if (nRowCount > 1)
    {
        // Lower-left board view...
        pWnd = m_wndSplitBoards.GetPane(1, 0);
        ASSERT(pWnd != NULL);
        pWnd->SendMessage(WM_WINSTATE, wParam, lParam);

        if (nColCount > 1)
        {
            // Lower-right board view...
            pWnd = m_wndSplitBoards.GetPane(1, 1);
            ASSERT(pWnd != NULL);
            pWnd->SendMessage(WM_WINSTATE, wParam, lParam);
        }
    }

    // Select list view...
    pWnd = m_wndSplitter2.GetPane(0, 0);
    pWnd->SendMessage(WM_WINSTATE, wParam, lParam);

    // Tiny map view...
    pWnd = m_wndSplitter2.GetPane(1, 0);
    pWnd->SendMessage(WM_WINSTATE, wParam, lParam);

    // Finally sync up the select list
    if (ar.IsLoading())
    {
        m_wndSplitBoards.SetActivePane(nRow, nCol);
        GetActiveBoardView()->NotifySelectListChange();
    }

    return (LRESULT)1;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CPlayBoardFrame::SendMessageToActiveBoardPane(UINT nMsg, WPARAM wParam,
    LPARAM lParam)
{
    CWnd* pWnd = GetActiveBoardView();
    if (pWnd != NULL)
        return pWnd->SendMessage(nMsg, wParam, lParam);
    return (LRESULT)0;
}

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardFrame message handlers

LRESULT CPlayBoardFrame::OnMessageCenterBoardOnPoint(WPARAM wParam, LPARAM lParam)
{
    // Route the message to the active board view.
    CPlayBoardView* pView = GetActiveBoardView();
    ASSERT(pView != NULL);
    return pView->SendMessage(WM_CENTERBOARDONPOINT, wParam, lParam);
}

// Send these on to the main view so they can be process no
// matter what frame view is active.

void CPlayBoardFrame::OnClose()
{
    CWnd::OnClose();            // Short circuit frame's doc close code
}

CCbSplitterWnd* CPlayBoardFrame::GetBoardSplitter()
{
    return (CCbSplitterWnd*)m_wndSplitter1.GetPane(0, 0);
}

CPlayBoardView* CPlayBoardFrame::GetActiveBoardView()
{
    CCbSplitterWnd* pSplitWnd = (CCbSplitterWnd*)m_wndSplitter1.GetPane(0, 0);
    return (CPlayBoardView*)pSplitWnd->GetActivePane();
}

void CPlayBoardFrame::OnViewHalfScaleBrd()
{
    GetActiveBoardView()->OnViewHalfScaleBrd();
}

void CPlayBoardFrame::OnUpdateViewHalfScaleBrd(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateViewHalfScaleBrd(pCmdUI);
}

void CPlayBoardFrame::OnViewFullScaleBrd()
{
    GetActiveBoardView()->OnViewFullScaleBrd();
}

void CPlayBoardFrame::OnUpdateViewFullScaleBrd(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateViewFullScaleBrd(pCmdUI);
}

void CPlayBoardFrame::OnViewSnapGrid()
{
    GetActiveBoardView()->OnViewSnapGrid();
}

void CPlayBoardFrame::OnUpdateViewSnapGrid(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateViewSnapGrid(pCmdUI);
}

void CPlayBoardFrame::OnEditSelAllMarkers()
{
    GetActiveBoardView()->OnEditSelAllMarkers();
}

void CPlayBoardFrame::OnUpdateEditSelAllMarkers(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateEditSelAllMarkers(pCmdUI);
}

void CPlayBoardFrame::OnEditBoardProperties()
{
    GetActiveBoardView()->OnEditBoardProperties();
}

void CPlayBoardFrame::OnViewPieces()
{
    GetActiveBoardView()->OnViewPieces();
}

void CPlayBoardFrame::OnUpdateViewPieces(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateViewPieces(pCmdUI);
}

void CPlayBoardFrame::OnSelectGroupMarkers(UINT nID)
{
    GetActiveBoardView()->OnSelectGroupMarkers(nID);
}

void CPlayBoardFrame::OnUpdateSelectGroupMarkers(CCmdUI* pCmdUI, UINT nID)
{
    GetActiveBoardView()->OnUpdateSelectGroupMarkers(pCmdUI, nID);
}

void CPlayBoardFrame::OnViewToggleScale()
{
    GetActiveBoardView()->OnViewToggleScale();
}

void CPlayBoardFrame::OnUpdateViewToggleScale(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateViewToggleScale(pCmdUI);
}

void CPlayBoardFrame::OnActStack()
{
    GetActiveBoardView()->OnActStack();
}

void CPlayBoardFrame::OnUpdateActStack(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateActStack(pCmdUI);
}

void CPlayBoardFrame::OnActToFront()
{
    GetActiveBoardView()->OnActToFront();
}

void CPlayBoardFrame::OnUpdateActToFront(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateActToFront(pCmdUI);
}

void CPlayBoardFrame::OnActToBack()
{
    GetActiveBoardView()->OnActToBack();
}

void CPlayBoardFrame::OnUpdateActToBack(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateActToBack(pCmdUI);
}

void CPlayBoardFrame::OnActTurnOver()
{
    GetActiveBoardView()->OnActTurnOver();
}

void CPlayBoardFrame::OnUpdateActTurnOver(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateActTurnOver(pCmdUI);
}

void CPlayBoardFrame::OnActPlotMove()
{
    GetActiveBoardView()->OnActPlotMove();
}

void CPlayBoardFrame::OnUpdateActPlotMove(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateActPlotMove(pCmdUI);
}

void CPlayBoardFrame::OnActPlotDone()
{
    GetActiveBoardView()->OnActPlotDone();
}

void CPlayBoardFrame::OnUpdateActPlotDone(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateActPlotDone(pCmdUI);
}

void CPlayBoardFrame::OnActPlotDiscard()
{
    GetActiveBoardView()->OnActPlotDiscard();
}

void CPlayBoardFrame::OnUpdateActPlotDiscard(CCmdUI* pCmdUI)
{
    GetActiveBoardView()->OnUpdateActPlotDiscard(pCmdUI);
}

void CPlayBoardFrame::OnViewSplitBoardRows()
{
    CCbSplitterWnd* pSplitWnd = GetBoardSplitter();
    if (pSplitWnd->IsRowHidden())
    {
        CRect rct;
        pSplitWnd->GetWindowRect(&rct);
        pSplitWnd->SetRowInfo(0, rct.Height() / 2, 0);
        pSplitWnd->ShowRow();
    }
    else
        pSplitWnd->HideRow(1);
}

void CPlayBoardFrame::OnUpdateViewSplitBoardRows(CCmdUI* pCmdUI)
{
    CCbSplitterWnd* pSplitWnd = GetBoardSplitter();
    pCmdUI->Enable();
    pCmdUI->SetCheck(!pSplitWnd->IsRowHidden());
}

void CPlayBoardFrame::OnViewSplitBoardCols()
{
    CCbSplitterWnd* pSplitWnd = GetBoardSplitter();
    if (pSplitWnd->IsColHidden())
    {
        CRect rct;
        pSplitWnd->GetWindowRect(&rct);
        pSplitWnd->SetColumnInfo(0, rct.Width() / 2, 0);
        pSplitWnd->ShowColumn();
    }
    else
        pSplitWnd->HideColumn(1);
}

void CPlayBoardFrame::OnUpdateViewSplitBoardCols(CCmdUI* pCmdUI)
{
    CCbSplitterWnd* pSplitWnd = GetBoardSplitter();
    pCmdUI->Enable();
    pCmdUI->SetCheck(!pSplitWnd->IsColHidden());
}
