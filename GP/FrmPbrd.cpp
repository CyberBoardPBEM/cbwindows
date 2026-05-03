// FrmPbrd.cpp : implementation file
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
#include    "Gp.h"
#include    "GamDoc.h"
#include    "FrmPbrd.h"
#include    "VwPbrd.h"
#include    "VwTbrd.h"
#include    "VwSelpce.h"
#include    "WinState.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "Player.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if 0
IMPLEMENT_DYNCREATE(CPlayBoardFrameContainer, CMDIChildWndEx)
#endif
wxIMPLEMENT_DYNAMIC_CLASS(CBPlayBoardFrameView, CB::View);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CPlayBoardFrame, wxPanel)
#if 0
#if 0
    ON_COMMAND(ID_VIEW_HALFSCALEBRD, OnViewHalfScaleBrd)
    ON_UPDATE_COMMAND_UI(ID_VIEW_HALFSCALEBRD, OnUpdateViewHalfScaleBrd)
    ON_COMMAND(ID_VIEW_FULLSCALEBRD, OnViewFullScaleBrd)
    ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCALEBRD, OnUpdateViewFullScaleBrd)
#endif
    ON_WM_CLOSE()
#if 0
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
    ON_COMMAND_EX(ID_ACT_TURNOVER, OnActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_PREV, OnActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_RANDOM, OnActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER, OnUpdateActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_PREV, OnUpdateActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_RANDOM, OnUpdateActTurnOver)
#endif
    ON_COMMAND(ID_PTOOL_PLOTMOVE, OnActPlotMove)
    ON_UPDATE_COMMAND_UI(ID_PTOOL_PLOTMOVE, OnUpdateActPlotMove)
    ON_COMMAND(ID_ACT_PLOTDONE, OnActPlotDone)
    ON_UPDATE_COMMAND_UI(ID_ACT_PLOTDONE, OnUpdateActPlotDone)
    ON_COMMAND(ID_ACT_PLOTDISCARD, OnActPlotDiscard)
    ON_UPDATE_COMMAND_UI(ID_ACT_PLOTDISCARD, OnUpdateActPlotDiscard)
#endif
    EVT_MENU(XRCID("ID_VIEW_SPLITBOARDROWS"), OnViewSplitBoardRows)
    EVT_UPDATE_UI(XRCID("ID_VIEW_SPLITBOARDROWS"), OnUpdateViewSplitBoardRows)
    EVT_MENU(XRCID("ID_VIEW_SPLITBOARDCOLS"), OnViewSplitBoardCols)
    EVT_UPDATE_UI(XRCID("ID_VIEW_SPLITBOARDCOLS"), OnUpdateViewSplitBoardCols)
#if 0
    ON_COMMAND_RANGE(ID_MRKGROUP_FIRST, ID_MRKGROUP_FIRST + 64, OnSelectGroupMarkers)
    ON_UPDATE_COMMAND_UI_RANGE(ID_MRKGROUP_FIRST, ID_MRKGROUP_FIRST + 64, OnUpdateSelectGroupMarkers)
#endif
    EVT_CENTERBOARDONPOINT(OnMessageCenterBoardOnPoint)
#if 0
    ON_MESSAGE(WM_WINSTATE, OnMessageWindowState)
    ON_WM_SIZE()
#endif
wxEND_EVENT_TABLE()

#if 0
BEGIN_MESSAGE_MAP(CPlayBoardFrameContainer, CPlayBoardFrameContainer::BASE)
    ON_MESSAGE(WM_CENTERBOARDONPOINT, OnMessageCenterBoardOnPoint)
    ON_MESSAGE(WM_WINSTATE, OnMessageWindowState)
END_MESSAGE_MAP()
#endif

/* KLUDGE:  wxDocManager passes events to
            (non-virtual) wxView::ProcessEventLocally(), which
            means TryAfter() doesn't get checked, so use this
            class to also give CPlayBoardFrame a chance at
            event */
bool DocChildBoardFrame::ProcessEvent(wxEvent& event)
{
    if (CB::DocChildFrame::ProcessEvent(event))
    {
        return true;
    }

    // only process wxChildFocusEvent for (strict) descendants
    wxChildFocusEvent* cfe = dynamic_cast<wxChildFocusEvent*>(&event);
    if (cfe)
    {
        if (this == cfe->GetWindow() ||
            !IsDescendant(cfe->GetWindow()))
        {
            return false;
        }
    }

    wxView* view = GetView();
    if (!view)
    {
        return false;
    }
    CBPlayBoardFrameView& boardFrameView = dynamic_cast<CBPlayBoardFrameView&>(*view);
    return boardFrameView.GetFramePanel().ProcessWindowEventLocally(event);
}

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardFrame

CPlayBoardFrame::~CPlayBoardFrame()
{
}

#if 0
BOOL CPlayBoardFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)GetStockObject(LTGRAY_BRUSH),
        AfxGetApp()->LoadIconW(IDR_GP_BOARDVIEW));

    cs.style |= WS_CLIPCHILDREN;
    cs.style &= ~(DWORD)FWS_ADDTOTITLE;
    return TRUE;
}
#endif

#if 0
void CPlayBoardFrameContainer::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    CGamDoc& pDoc = CheckedDeref(CB::ToCGamDoc(GetActiveDocument()));
    CB::string str = pDoc.GetUserReadableName();

    CB::string strBoardName = child->m_pPBoard->GetBoard()->GetName();
    str += " - " + strBoardName;

    if (child->m_pPBoard->IsOwned())
    {
        str += " - ";
        CB::string strOwnerName = pDoc.GetPlayerManager()->
            GetPlayerUsingMask(child->m_pPBoard->GetOwnerMask()).m_strName;
        CB::string strOwnedBy = CB::string::Format(IDS_TIP_OWNED_BY_UC, strOwnerName);
        str += strOwnedBy;
    }

    SetWindowText(str);
}
#endif

/////////////////////////////////////////////////////////////////////////////

CPlayBoardFrame::CPlayBoardFrame(wxWindow& parent,
                                    CGamDoc& doc) :
    CB_XRC_BEGIN_CTRLS_DEFN(&parent, CPlayBoardFrame)
        CB_XRC_CTRL(m_wndSplitter1)
        CB_XRC_CTRL(m_wndSplitter2)
        CB_XRC_CTRL(m_wndSplitBoards)
        CB_XRC_CTRL(m_vwBoard1)
        CB_XRC_CTRL(m_vwBoard2)
    CB_XRC_END_CTRLS_DEFN()
{
    m_pPBoard = NULL;
#if 0
    CRect rect;
    parent.GetClientRect(rect);
    if (!Create(nullptr, nullptr, 0, rect, &parent, AFX_IDW_PANE_FIRST, &pContext))
    {
        AfxThrowMemoryException();
    }
    CGamDoc* pDoc = CB::ToCGamDoc(pContext.m_pCurrentDoc);
#endif
    m_pPBoard = &doc.GetNewViewBoard();

#if 0
    // Create a splitter with 1 row, 2 columns
    if (!m_wndSplitter1.CreateStatic(this, 1, 2))
    {
        TRACE("Failed to create static splitter\n");
        AfxThrowMemoryException();
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
        AfxThrowMemoryException();
    }

    if (!m_wndSplitBoards.CreateView(0, 0,
        pContext.m_pNewViewClass, CSize(xSize / 2, ySize / 2), &pContext))
    {
        TRACE("Failed to create first board pane\n");
        AfxThrowMemoryException();
    }

    if (!m_wndSplitBoards.CreateView(0, 1,
        pContext.m_pNewViewClass, CSize(xSize / 2, ySize / 2), &pContext))
    {
        TRACE("Failed to create second board pane\n");
        AfxThrowMemoryException();
    }
    if (!m_wndSplitBoards.CreateView(1, 0,
        pContext.m_pNewViewClass, CSize(xSize / 2, ySize / 2), &pContext))
    {
        TRACE("Failed to create third board pane\n");
        AfxThrowMemoryException();
    }
    if (!m_wndSplitBoards.CreateView(1, 1,
        pContext.m_pNewViewClass, CSize(xSize / 2, ySize / 2), &pContext))
    {
        TRACE("Failed to create fourth board pane\n");
        AfxThrowMemoryException();
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
        AfxThrowMemoryException();
    }

    if (!m_wndSplitter2.CreateView(0, 0,
        RUNTIME_CLASS(CSelectedPieceViewContainer),
        CSize(rct.Width() - xSize, ySize), &pContext))
    {
        TRACE("Failed to create second Selected Piece pane\n");
        AfxThrowMemoryException();
    }
    if (!m_wndSplitter2.CreateView(1, 0,
        RUNTIME_CLASS(CTinyBoardViewContainer),
        CSize(rct.Width() - xSize, rct.Height() - ySize), &pContext))
    {
        TRACE("Failed to create small scale map pane\n");
        AfxThrowMemoryException();
    }

    if (!m_pPBoard->m_bShowSelListAndTinyMap)
        m_wndSplitter1.HideColumn(1);

    ShowWindow(SW_SHOW);
#else
    RefPtr<CSelectedPieceView> m_vwSelect = XRCCTRL(*m_wndSplitter2, "m_vwSelect", CSelectedPieceView);
    RefPtr<CTinyBoardView> m_vwTiny = XRCCTRL(*m_wndSplitter2, "m_vwTiny", CTinyBoardView);

    GetParent()->Layout();
    wxRect rct = GetClientRect();
    int xSize = value_preserving_cast<int>((85L * rct.GetWidth()) / 100);
    int ySize = rct.GetHeight() / 2;
    m_wndSplitter1->SetSashPosition(xSize);
    m_wndSplitter2->SetSashPosition(ySize);
    /* KLUDGE:  want to start with the board unsplit, but I
                don't know how to do that w/ wxFormBuilder */
    m_wndSplitBoards->Unsplit();
    m_wndSplitBoards->SetLastSplitPosition(wxPoint(0, 0));

    m_vwBoard1->OnInitialUpdate(doc);
    m_vwBoard2->OnInitialUpdate(doc);
    m_vwSelect->OnInitialUpdate(doc);
    m_vwTiny->OnInitialUpdate(doc);

    if (!m_pPBoard->m_bShowSelListAndTinyMap)
        m_wndSplitter1->Unsplit();

    static_cast<wxView&>(*m_vwBoard1).Activate(true);
#endif
}

///////////////////////////////////////////////////////////////////////
// The WM_WINSTATE message is sent when a document is being saved.
// WPARAM = CArchive*, LPARAM = 0 if save, 1 if restore

#define SCHEMA_BRDVIEW_SAVE     1

#if 0
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
        GetActiveBoardView().NotifySelectListChange();
    }

    return (LRESULT)1;
}
#endif

/////////////////////////////////////////////////////////////////////////////

bool CPlayBoardFrame::SendMessageToActiveBoardPane(wxEvent& event)
{
    CPlayBoardView& pWnd = GetActiveBoardView();
    wxASSERT(dynamic_cast<SelectBoardObjListEvent*>(&event));
    return pWnd.ProcessWindowEvent(event);
}

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardFrame message handlers

void CPlayBoardFrame::OnMessageCenterBoardOnPoint(CenterBoardOnPointEvent& event)
{
    // Route the message to the active board view.
    CPlayBoardView& pView = GetActiveBoardView();
    pView.ProcessWindowEvent(event);
}

#if 0
// Send these on to the main view so they can be process no
// matter what frame view is active.

void CPlayBoardFrame::OnClose()
{
    CWnd::OnClose();            // Short circuit frame's doc close code
}
#endif

#if 0
void CPlayBoardFrame::OnViewHalfScaleBrd()
{
    GetActiveBoardView().OnViewHalfScaleBrd();
}

void CPlayBoardFrame::OnUpdateViewHalfScaleBrd(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateViewHalfScaleBrd(pCmdUI);
}

void CPlayBoardFrame::OnViewFullScaleBrd()
{
    GetActiveBoardView().OnViewFullScaleBrd();
}

void CPlayBoardFrame::OnUpdateViewFullScaleBrd(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateViewFullScaleBrd(pCmdUI);
}

void CPlayBoardFrame::OnViewSnapGrid()
{
    GetActiveBoardView().OnViewSnapGrid();
}

void CPlayBoardFrame::OnUpdateViewSnapGrid(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateViewSnapGrid(pCmdUI);
}

void CPlayBoardFrame::OnEditSelAllMarkers()
{
    GetActiveBoardView().OnEditSelAllMarkers();
}

void CPlayBoardFrame::OnUpdateEditSelAllMarkers(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateEditSelAllMarkers(pCmdUI);
}

void CPlayBoardFrame::OnEditBoardProperties()
{
    GetActiveBoardView().OnEditBoardProperties();
}

void CPlayBoardFrame::OnViewPieces()
{
    GetActiveBoardView().OnViewPieces();
}

void CPlayBoardFrame::OnUpdateViewPieces(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateViewPieces(pCmdUI);
}

void CPlayBoardFrame::OnSelectGroupMarkers(UINT nID)
{
    GetActiveBoardView().OnSelectGroupMarkers(nID);
}

void CPlayBoardFrame::OnUpdateSelectGroupMarkers(CCmdUI* pCmdUI, UINT nID)
{
    GetActiveBoardView().OnUpdateSelectGroupMarkers(pCmdUI, nID);
}

void CPlayBoardFrame::OnViewToggleScale()
{
    GetActiveBoardView().OnViewToggleScale();
}

void CPlayBoardFrame::OnUpdateViewToggleScale(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateViewToggleScale(pCmdUI);
}

void CPlayBoardFrame::OnActStack()
{
    GetActiveBoardView().OnActStack();
}

void CPlayBoardFrame::OnUpdateActStack(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateActStack(pCmdUI);
}

void CPlayBoardFrame::OnActToFront()
{
    GetActiveBoardView().OnActToFront();
}

void CPlayBoardFrame::OnUpdateActToFront(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateActToFront(pCmdUI);
}

void CPlayBoardFrame::OnActToBack()
{
    GetActiveBoardView().OnActToBack();
}

void CPlayBoardFrame::OnUpdateActToBack(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateActToBack(pCmdUI);
}

BOOL CPlayBoardFrame::OnActTurnOver(UINT id)
{
    return GetActiveBoardView().OnActTurnOver(id);
}

void CPlayBoardFrame::OnUpdateActTurnOver(CCmdUI* pCmdUI)
{
    GetActiveBoardView().OnUpdateActTurnOver(pCmdUI);
}

void CPlayBoardFrame::OnActPlotMove()
{
    wxASSERT(!"dead code?");
    wxCommandEvent dummy;
    GetActiveBoardView().OnActPlotMove(dummy);
}

void CPlayBoardFrame::OnUpdateActPlotMove(CCmdUI* pCmdUI)
{
    wxASSERT(!"dead code?");
    CB_VERIFY(CB::RelayOnCmdMsg(GetActiveBoardView(), ID_PTOOL_PLOTMOVE, CN_UPDATE_COMMAND_UI, pCmdUI, nullptr));
}

void CPlayBoardFrame::OnActPlotDone()
{
    wxASSERT(!"dead code?");
    wxCommandEvent dummy;
    GetActiveBoardView().OnActPlotDone(dummy);
}

void CPlayBoardFrame::OnUpdateActPlotDone(CCmdUI* pCmdUI)
{
    wxASSERT(!"dead code?");
    CB_VERIFY(CB::RelayOnCmdMsg(GetActiveBoardView(), ID_ACT_PLOTDONE, CN_UPDATE_COMMAND_UI, pCmdUI, nullptr));
}

void CPlayBoardFrame::OnActPlotDiscard()
{
    wxASSERT(!"dead code?");
    wxCommandEvent dummy;
    GetActiveBoardView().OnActPlotDiscard(dummy);
}

void CPlayBoardFrame::OnUpdateActPlotDiscard(CCmdUI* pCmdUI)
{
    wxASSERT(!"dead code?");
    CB_VERIFY(CB::RelayOnCmdMsg(GetActiveBoardView(), ID_ACT_PLOTDISCARD, CN_UPDATE_COMMAND_UI, pCmdUI, nullptr));
}
#endif

void CPlayBoardFrame::OnViewSplitBoardRows(wxCommandEvent& /*event*/)
{
    wxSplitterWindow& pSplitWnd = GetBoardSplitter();
    if (!pSplitWnd.IsSplit() ||
        pSplitWnd.GetSplitMode() == wxSPLIT_VERTICAL)
    {
        if (pSplitWnd.IsSplit())
        {
            pSplitWnd.Unsplit();
        }
        pSplitWnd.SplitHorizontally(&*m_vwBoard1, &*m_vwBoard2);
    }
    else
        pSplitWnd.Unsplit();
}

void CPlayBoardFrame::OnUpdateViewSplitBoardRows(wxUpdateUIEvent& pCmdUI)
{
    wxSplitterWindow& pSplitWnd = GetBoardSplitter();
    pCmdUI.Enable(true);
    pCmdUI.Check(pSplitWnd.IsSplit() &&
                    pSplitWnd.GetSplitMode() == wxSPLIT_HORIZONTAL);
}

void CPlayBoardFrame::OnViewSplitBoardCols(wxCommandEvent& /*event*/)
{
    wxSplitterWindow& pSplitWnd = GetBoardSplitter();
    if (!pSplitWnd.IsSplit() ||
        pSplitWnd.GetSplitMode() == wxSPLIT_HORIZONTAL)
    {
        if (pSplitWnd.IsSplit())
        {
            pSplitWnd.Unsplit();
        }
        pSplitWnd.SplitVertically(&*m_vwBoard1, &*m_vwBoard2);
    }
    else
        pSplitWnd.Unsplit();
}

void CPlayBoardFrame::OnUpdateViewSplitBoardCols(wxUpdateUIEvent& pCmdUI)
{
    wxSplitterWindow& pSplitWnd = GetBoardSplitter();
    pCmdUI.Enable(true);
    pCmdUI.Check(pSplitWnd.IsSplit() &&
                    pSplitWnd.GetSplitMode() == wxSPLIT_VERTICAL);
}

#if 0
void CPlayBoardFrame::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (m_wndSplitter1.m_hWnd)
    {
        wxASSERT(m_wndSplitter1.GetParent() == this);
        CRect rect;
        GetClientRect(rect);
        wxASSERT(rect.Width() == cx && rect.Height() == cy);
        m_wndSplitter1.SetWindowPos(&wndBottom, 0, 0, cx, cy, SWP_NOACTIVATE);
    }
}
#endif

#if 0
CDocument* CPlayBoardFrameContainer::GetActiveDocument()
{
    AfxThrowNotSupportedException();
}

BOOL CPlayBoardFrameContainer::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
    wxWindow = MakeOwner<CB::wxNativeContainerWindowMixin>(*this);
    CGamDoc& pDoc = CheckedDeref(CB::ToCGamDoc(pContext->m_pCurrentDoc));
    child = new CPlayBoardFrame(*wxWindow, pDoc);
    return true;
}

LRESULT CPlayBoardFrameContainer::OnMessageCenterBoardOnPoint(WPARAM wParam, LPARAM lParam)
{
    const POINT* point = reinterpret_cast<POINT*>(wParam);
    CenterBoardOnPointEvent event(CB::Convert(CheckedDeref(point)));
    child->ProcessWindowEvent(event);
    return 0;
}

LRESULT CPlayBoardFrameContainer::OnMessageWindowState(WPARAM wParam, LPARAM lParam)
{
    WinStateEvent event(*reinterpret_cast<CArchive*>(wParam), bool(lParam));
    child->ProcessWindowEvent(event);
    return (LRESULT)1;
}
#endif

void CBPlayBoardFrameView::New(CGamDoc& doc)
{
    wxDocTemplate& templ = CB::FindDocTemplateByView(*wxCLASSINFO(CBPlayBoardFrameView));
    templ.CreateView(&doc);
}

const CB::DocChildFrame& CBPlayBoardFrameView::GetFrameFrame() const
{
    return BASE::GetFrame();
}

const CPlayBoardFrame& CBPlayBoardFrameView::GetFramePanel() const
{
    const CB::DocChildFrame& frame = GetFrameFrame();
    const wxWindowList& children = frame.GetChildren();
    wxASSERT(children.size() == 1);
    const wxWindow& wnd = CheckedDeref(children.front());
    wxASSERT(dynamic_cast<const CPlayBoardFrame*>(&wnd));
    return static_cast<const CPlayBoardFrame&>(wnd);
}

void CBPlayBoardFrameView::Activate(bool activate)
{
    CB::View::Activate(activate);
    // if possible, move activation to an actual board
    if (activate &&
        !GetFrameFrame().GetChildren().empty())
    {
        wxView& boardView = GetFramePanel().GetActiveBoardView();
        boardView.Activate(true);
    }
}

bool CBPlayBoardFrameView::OnClose(bool deleteWindow)
{
    wxASSERT(!deleteWindow);
    /* doc's life determined by wxGsnProjView, not this,
        so bypass wxView::OnClose() */
    return true;
}

bool CBPlayBoardFrameView::OnCreate(wxDocument* doc, long flags)
{
    wxASSERT(doc == &GetDocument());
    if (!wxView::OnCreate(doc, flags))
    {
        return false;
    }
    CGamDoc& pDoc = GetDocument();
    CPlayBoard& board = pDoc.GetNewViewBoard();

    CB::string str = pDoc.GetUserReadableName();

    CB::string strBoardName = board.GetBoard()->GetName();
    str += " - " + strBoardName;

    if (board.IsOwned())
    {
        str += " - ";
        CB::string strOwnerName = pDoc.GetPlayerManager()->
            GetPlayerUsingMask(board.GetOwnerMask()).m_strName;
        CB::string strOwnedBy = CB::string::Format(IDS_TIP_OWNED_BY_UC, strOwnerName);
        str += strOwnedBy;
    }

    CB::DocChildFrame* frame = new DocChildBoardFrame(pDoc,
                    *this,
                    CheckedDeref(GetMainFrame()),
                    str,
                    wxIcon(std::format("#{}", IDR_GP_BOARDVIEW),
                            wxBITMAP_TYPE_ICO_RESOURCE,
                            16, 16),
                    pDoc.GetMenuName());
    new CPlayBoardFrame(*frame, pDoc);
    frame->Show();

    return true;
}
