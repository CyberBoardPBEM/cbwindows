// VwPbrd.cpp : implementation of the CPlayBoardView class
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
#include    "ResTbl.h"
#include    "GamDoc.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "PPieces.h"
#include    "ToolPlay.h"
#include    "SelOPlay.h"
#include    "VwpBrd.h"
#include    "GMisc.h"
#include    "LibMfc.h"
#include    "DlgRot.h"
#include    "DlgRotpc.h"
#include    "DlgEdtel.h"
#include    "DlgSelOwner.h"
#include    "DlgMarkCount.h"
#include    "Player.h"
#include    "CDib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CPlayBoardView, CScrollView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CPlayBoardView, CScrollView)
    //{{AFX_MSG_MAP(CPlayBoardView)
    ON_COMMAND(ID_VIEW_FULLSCALEBRD, OnViewFullScaleBrd)
    ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCALEBRD, OnUpdateViewFullScaleBrd)
    ON_COMMAND(ID_VIEW_HALFSCALEBRD, OnViewHalfScaleBrd)
    ON_UPDATE_COMMAND_UI(ID_VIEW_HALFSCALEBRD, OnUpdateViewHalfScaleBrd)
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragItem)
    ON_MESSAGE(WM_ROTATEPIECE_DELTA, OnMessageRotateRelative)
    ON_MESSAGE(WM_CENTERBOARDONPOINT, OnMessageCenterBoardOnPoint)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_SETCURSOR()
    ON_WM_KEYDOWN()
    ON_WM_CHAR()
    ON_COMMAND_EX(ID_PTOOL_SELECT, OnPlayTool)
    ON_UPDATE_COMMAND_UI(ID_PTOOL_SELECT, OnUpdatePlayTool)
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
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_CELLNUM, OnUpdateIndicatorCellNum)
    ON_COMMAND(ID_VIEW_SNAPGRID, OnViewSnapGrid)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SNAPGRID, OnUpdateViewSnapGrid)
    ON_COMMAND(ID_EDIT_SELALLMARKERS, OnEditSelAllMarkers)
    ON_UPDATE_COMMAND_UI(ID_EDIT_SELALLMARKERS, OnUpdateEditSelAllMarkers)
    ON_COMMAND(ID_ACT_ROTATE, OnActRotate)
    ON_UPDATE_COMMAND_UI(ID_ACT_ROTATE, OnUpdateActRotate)
    ON_COMMAND(ID_VIEW_TOGGLESCALE, OnViewToggleScale)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TOGGLESCALE, OnUpdateViewToggleScale)
    ON_COMMAND(ID_VIEW_PIECES, OnViewPieces)
    ON_UPDATE_COMMAND_UI(ID_VIEW_PIECES, OnUpdateViewPieces)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_BRD2FILE, OnEditBoardToFile)
    ON_COMMAND(ID_EDIT_BRDPROP, OnEditBoardProperties)
    ON_COMMAND(ID_ACT_ROTATEREL, OnActRotateRelative)
    ON_UPDATE_COMMAND_UI(ID_ACT_ROTATEREL, OnUpdateActRotateRelative)
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_VIEW_DRAW_IND_ON_TOP, OnViewDrawIndOnTop)
    ON_UPDATE_COMMAND_UI(ID_VIEW_DRAW_IND_ON_TOP, OnUpdateViewDrawIndOnTop)
    ON_COMMAND(ID_EDIT_ELEMENT_TEXT, OnEditElementText)
    ON_UPDATE_COMMAND_UI(ID_EDIT_ELEMENT_TEXT, OnUpdateEditElementText)
    ON_COMMAND(ID_ACT_LOCKOBJECT, OnActLockObject)
    ON_UPDATE_COMMAND_UI(ID_ACT_LOCKOBJECT, OnUpdateActLockObject)
    ON_COMMAND(ID_ACT_LOCK_SUSPEND, OnActLockSuspend)
    ON_UPDATE_COMMAND_UI(ID_ACT_LOCK_SUSPEND, OnUpdateActLockSuspend)
    ON_COMMAND(ID_ACT_SHUFFLE_SELECTED, OnActShuffleSelectedObjects)
    ON_UPDATE_COMMAND_UI(ID_ACT_SHUFFLE_SELECTED, OnUpdateActShuffleSelectedObjects)
    ON_COMMAND(ID_ACT_AUTOSTACK_DECK, OnActAutostackDeck)
    ON_UPDATE_COMMAND_UI(ID_ACT_AUTOSTACK_DECK, OnUpdateActAutostackDeck)
    ON_COMMAND(ID_ACT_TAKE_OWNERSHIP, OnActTakeOwnership)
    ON_UPDATE_COMMAND_UI(ID_ACT_TAKE_OWNERSHIP, OnUpdateActTakeOwnership)
    ON_COMMAND(ID_ACT_RELEASE_OWNERSHIP, OnActReleaseOwnership)
    ON_UPDATE_COMMAND_UI(ID_ACT_RELEASE_OWNERSHIP, OnUpdateActReleaseOwnership)
    ON_COMMAND(ID_ACT_SET_OWNER, OnActSetOwner)
    ON_UPDATE_COMMAND_UI(ID_ACT_SET_OWNER, OnUpdateActSetOwner)
    ON_COMMAND(ID_VIEW_SMALLSCALEBRD, OnViewSmallScaleBoard)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SMALLSCALEBRD, OnUpdateViewSmallScaleBoard)
    ON_COMMAND_EX(ID_PTOOL_LINE, OnPlayTool)
    ON_COMMAND_EX(ID_PTOOL_TEXTBOX, OnPlayTool)
    ON_UPDATE_COMMAND_UI(ID_PTOOL_LINE, OnUpdatePlayTool)
    ON_UPDATE_COMMAND_UI(ID_PTOOL_TEXTBOX, OnUpdatePlayTool)
    ON_WM_MOUSEWHEEL()
    ON_COMMAND(ID_VIEW_BOARD_ROTATE180, OnViewBoardRotate180)
    ON_UPDATE_COMMAND_UI(ID_VIEW_BOARD_ROTATE180, OnUpdateViewBoardRotate180)
    ON_COMMAND(ID_ACT_ROTATEGROUP, OnActRotateGroupRelative)
    ON_UPDATE_COMMAND_UI(ID_ACT_ROTATEGROUP, OnUpdateActRotateGroupRelative)
    //}}AFX_MSG_MAP
    ON_COMMAND_RANGE(ID_ACT_ROTATE_0, (ID_ACT_ROTATE_0 + 360 / 5), OnRotatePiece)
    ON_UPDATE_COMMAND_UI_RANGE(ID_ACT_ROTATE_0, (ID_ACT_ROTATE_0 + 360 / 5), OnUpdateRotatePiece)
    ON_COMMAND_RANGE(ID_MRKGROUP_FIRST, ID_MRKGROUP_FIRST + 64, OnSelectGroupMarkers)
    ON_UPDATE_COMMAND_UI_RANGE(ID_MRKGROUP_FIRST, ID_MRKGROUP_FIRST + 64, OnUpdateSelectGroupMarkers)
    // Standard printing commands
    ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
    ON_MESSAGE(WM_WINSTATE, OnMessageWindowState)
    ON_MESSAGE(WM_SELECT_BOARD_OBJLIST, OnMessageSelectBoardObjectList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardView construction/destruction

CPlayBoardView::CPlayBoardView()
{
    m_selList.SetView(this);
    m_pPBoard = NULL;
    m_nZoom = fullScale;
    m_nCurToolID = ID_PTOOL_SELECT;
    m_bInDrag = FALSE;
    m_pDragSelList = NULL;
    m_nTimerID = 0;
}

CPlayBoardView::~CPlayBoardView()
{
}

BOOL CPlayBoardView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CScrollView::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)GetStockObject(WHITE_BRUSH));

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnInitialUpdate()
{
    m_toolMsgTip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX);
    m_toolMsgTip.SetMaxTipWidth(MAX_PLAYBOARD_TIP_WIDTH);
    m_toolHitTip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX);
    m_toolHitTip.SetMaxTipWidth(MAX_PLAYBOARD_TIP_WIDTH);
    m_pCurTipObj = NULL;

    m_pPBoard = (CPlayBoard*)GetDocument()->GetNewViewParameter();

    if (m_pPBoard == NULL)
    {
        // See if we can get our playing board from the (0, 0) pane in the splitter.
        CSplitterWndEx* pParent = (CSplitterWndEx*)GetParent();
        CPlayBoardView* pPlay = (CPlayBoardView*)pParent->GetPane(0, 0);
        m_pPBoard = pPlay->GetPlayBoard();
    }

    ASSERT(m_pPBoard != NULL);
    SetOurScrollSizes(m_nZoom);
    CScrollView::OnInitialUpdate();
}

void CPlayBoardView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CGamDocHint* ph = (CGamDocHint*)pHint;
    if (lHint == HINT_POINTINVIEW && ph->m_pPBoard == m_pPBoard)
    {
        ScrollWorkspacePointIntoView(ph->m_point);
    }
    else if (lHint == HINT_BOARDCHANGE)
    {
        // Make sure we still exist!
        if (GetDocument()->GetPBoardManager()->FindPBoardByPtr(m_pPBoard) == -1)
        {
            CFrameWnd* pFrm = GetParentFrame();
            ASSERT(pFrm != NULL);
            pFrm->PostMessage(WM_CLOSE, 0, 0L);
        }
    }
    else if (lHint == HINT_UPDATEOBJECT && ph->m_pPBoard == m_pPBoard)
    {
        CRect rct;
        rct = ph->m_pDrawObj->GetEnclosingRect();   // In board coords.
        InvalidateWorkspaceRect(&rct);
    }
    else if (lHint == HINT_UPDATEOBJLIST && ph->m_pPBoard == m_pPBoard)
    {
        POSITION pos;
        for (pos = ph->m_pPtrList->GetHeadPosition(); pos != NULL; )
        {
            CDrawObj* pDObj = (CDrawObj*)ph->m_pPtrList->GetNext(pos);
            ASSERT(pDObj != NULL);
            CRect rct = pDObj->GetEnclosingRect();  // In board coords.
            InvalidateWorkspaceRect(&rct);
        }
    }
    else if (lHint == HINT_SELECTOBJ)
    {
        if (ph->m_pPBoard != NULL && ph->m_pPBoard != m_pPBoard)
            return;                     // Ignore in this case
        if (ph->m_pDrawObj == NULL)     // NULL means deselect all
        {
            m_selList.PurgeList(TRUE);
            return;
        }
        m_selList.AddObject(ph->m_pDrawObj, TRUE);
        if (ph->m_pDrawObj->GetType() == CDrawObj::drawPieceObj ||
                ph->m_pDrawObj->GetType() == CDrawObj::drawMarkObj)
            NotifySelectListChange();
    }
    else if (lHint == HINT_SELECTOBJLIST && ph->m_pPBoard == m_pPBoard)
    {
        ASSERT(ph->m_pPtrList != NULL);
        m_selList.PurgeList(TRUE);

        POSITION pos;
        for (pos = ph->m_pPtrList->GetHeadPosition(); pos != NULL; )
        {
            CDrawObj* pDObj = (CDrawObj*)ph->m_pPtrList->GetNext(pos);
            ASSERT(pDObj != NULL);
            m_selList.AddObject(pDObj, TRUE);
            NotifySelectListChange();
        }
    }
    else if (lHint == HINT_UPDATESELECTLIST && pSender != this)
    {
        // Resync the select list to ensure that all objects still exist
        // and that the handles track objct movements for those that still
        // exist.
        CPtrList listSelectedObjs;
        POSITION pos = m_selList.GetHeadPosition();
        while (pos != NULL)
        {
            CDrawObj* pObj = ((CSelection*)m_selList.GetNext(pos))->m_pObj;
            if (m_pPBoard->IsObjectOnBoard(pObj))
                listSelectedObjs.AddTail(pObj);
        }
        m_selList.PurgeList();          // Clear former selection indications
        // Reselect any object that are still on the board.
        SelectAllObjectsInList(&listSelectedObjs);
    }
    else if (lHint == HINT_GAMESTATEUSED)
    {
        m_selList.PurgeList(TRUE);
        Invalidate(FALSE);
        BeginWaitCursor();
        UpdateWindow();
        EndWaitCursor();
    }
    else if (lHint == HINT_ALWAYSUPDATE ||
        (lHint == HINT_UPDATEBOARD && ph->m_pPBoard == m_pPBoard))
    {
        Invalidate(FALSE);
        BeginWaitCursor();
        UpdateWindow();
        EndWaitCursor();
    }
    else if (lHint == HINT_CLEARINDTIP)
    {
        ClearNotificationTip();
    }
}

///////////////////////////////////////////////////////////////////////

void CPlayBoardView::NotifySelectListChange()
{
    CGamDocHint hint;
    hint.m_pPBoard = m_pPBoard;
    hint.m_pSelList = &m_selList;
    GetDocument()->UpdateAllViews(this, HINT_UPDATESELECT, &hint);
}

///////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
    CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);
    if (bActivate && pActivateView != pDeactiveView)
        NotifySelectListChange();
}

///////////////////////////////////////////////////////////////////////
// This message is sent when a document is being saved.
// WPARAM = CPlayBoard*, LPARAM = CPtrList* of CDrawObj*

LRESULT CPlayBoardView::OnMessageSelectBoardObjectList(WPARAM wParam, LPARAM lParam)
{
    if ((CPlayBoard*)wParam != m_pPBoard)
        return (LRESULT)0;
    CPtrList* pList = (CPtrList*)lParam;
    m_selList.PurgeList();                  // Deselect current set of selections
    SelectAllObjectsInList(pList);          // Select the new set
    return (LRESULT)1;
}

///////////////////////////////////////////////////////////////////////
// This message is sent when a document is being saved.
// WPARAM = CArchive*, LPARAM = 0 if save, 1 if restore

LRESULT CPlayBoardView::OnMessageWindowState(WPARAM wParam, LPARAM lParam)
{
    ASSERT(wParam != NULL);
    CArchive& ar = *((CArchive*)wParam);
    if (ar.IsStoring())
    {
        ar << (WORD)m_nZoom;

        CPoint pnt = GetScrollPosition();
        ar << (DWORD)pnt.x;
        ar << (DWORD)pnt.y;
        // Save the select list
        if (m_selList.GetCount() > 0)
        {
            CPtrArray tblObjPtrs;
            m_selList.LoadTableWithObjectPtrs(&tblObjPtrs);
            ar << (DWORD)tblObjPtrs.GetSize();
            for (int i = 0; i < tblObjPtrs.GetSize(); i++)
            {
                CDrawObj* pObj = (CDrawObj*)tblObjPtrs.GetAt(i);
                ar << (DWORD)GetDocument()->GetGameElementCodeForObject(pObj);
            }
        }
        else
            ar << (DWORD)0;
    }
    else
    {
        CPoint pnt;
        WORD wTmp;
        DWORD dwTmp;

        ar >> wTmp; DoViewScaleBrd((TileScale)wTmp);

        ar >> dwTmp; pnt.x = (LONG)dwTmp;
        ar >> dwTmp; pnt.y = (LONG)dwTmp;

        ScrollToPosition(pnt);

        // Restore the select list.
        m_selList.PurgeList();
        DWORD dwSelCount;
        ar >> dwSelCount;
        while (dwSelCount--)
        {
            GameElement elem;
            ar >> dwTmp; elem = (GameElement)dwTmp;
            CDrawObj* pObj = NULL;
            if (IsGameElementAPiece(elem))
                pObj = m_pPBoard->FindPieceID(GetPieceIDFromElement(elem));
            else
                pObj = m_pPBoard->FindObjectID(GetObjectIDFromElement(elem));
            if (pObj != NULL)
                m_selList.AddObject(pObj, TRUE);
        }
    }
    return (LRESULT)1;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CPlayBoardView::PreTranslateMessage(MSG* pMsg)
{
    // RelayEvent is required for CToolTipCtrl objects -
    // it passes mouse messages on to the tool tip control
    // so it can decide when to show the tool tip
    m_toolHitTip.RelayEvent(pMsg);
    m_toolMsgTip.RelayEvent(pMsg);

    return CScrollView::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::SetOurScrollSizes(TileScale nZoom)
{
    CBoard* pBoard = m_pPBoard->GetBoard();
    HDC hDC = ::GetDC(NULL);
    int xWidth = GetDeviceCaps(hDC, HORZRES);
    int yHeight = GetDeviceCaps(hDC, VERTRES);

    CSize sizeCell = pBoard->GetCellSize(nZoom);
    int nPageX = sizeCell.cx > xWidth / 8 ? xWidth / 32 : sizeCell.cx;
    int nPageY = sizeCell.cy > yHeight / 8 ? yHeight / 32 : sizeCell.cy;

    SetScrollSizes(MM_TEXT, pBoard->GetSize(m_nZoom), sizeDefault,
        CSize(nPageX, nPageY));
}

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardView drawing

void CPlayBoardView::OnDraw(CDC* pDC)
{
    CBoard*     pBoard = m_pPBoard->GetBoard();
    CDC         dcMem;
    CBitmap     bmMem;
    CRect       oRct;
    CRect       oRctSave;
    CBitmap*    pPrvBMap;

    pDC->GetClipBox(&oRct);
    SetupPalette(pDC);

    if (oRct.IsRectEmpty())
        return;                 // Nothing to do

    bmMem.Attach(Create16BitDIBSection(pDC->m_hDC,
        oRct.Width(), oRct.Height()));
    dcMem.CreateCompatibleDC(pDC);
    pPrvBMap = dcMem.SelectObject(&bmMem);
    if (m_pPBoard->IsBoardRotated180())
    {
        oRctSave = oRct;
        CSize sizeBrd = pBoard->GetSize(m_nZoom);
        oRct = CRect(CPoint(sizeBrd.cx - oRct.left - oRct.Width(),
            sizeBrd.cy - oRct.top - oRct.Height()), oRct.Size());
    }
    dcMem.SetViewportOrg(-oRct.left, -oRct.top);

    SetupPalette(&dcMem);

    // Draw base board image...
    pBoard->SetMaxDrawLayer();          // Make sure all layers are drawn
    pBoard->Draw(&dcMem, &oRct, m_nZoom,
        m_nZoom == smallScale ? m_pPBoard->m_bSmallCellBorders : m_pPBoard->m_bCellBorders);

    // Draw pieces etc.....

    CRect rct(&oRct);
    SetupDrawListDC(&dcMem, &rct);

    m_pPBoard->Draw(&dcMem, &rct, m_nZoom);

    if (!pDC->IsPrinting() && GetPlayBoard()->GetPiecesVisible())
        m_selList.OnDraw(&dcMem);       // Handle selections.

    RestoreDrawListDC(&dcMem);

    if (m_pPBoard->IsBoardRotated180())
    {
        // Xfer to output
        dcMem.SetViewportOrg(0, 0);
        pDC->StretchBlt(oRctSave.left, oRctSave.top, oRctSave.Width(), oRctSave.Height(),
            &dcMem, oRctSave.Width() - 1, oRctSave.Height() - 1,
            -oRctSave.Width(), -oRctSave.Height(), SRCCOPY);
    }
    else
    {
        // Xfer to output
        pDC->BitBlt(oRct.left, oRct.top, oRct.Width(), oRct.Height(),
            &dcMem, oRct.left, oRct.top, SRCCOPY);
    }

    ResetPalette(&dcMem);
    dcMem.SelectObject(pPrvBMap);

    ResetPalette(pDC);
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CPlayBoardView::OnDragItem(WPARAM wParam, LPARAM lParam)
{
    if (GetDocument()->IsPlaying())
        return 0;                       // Drags not supported during play

    DragInfo* pdi = (DragInfo*)lParam;

    if (pdi->m_dragType == DRAG_PIECE)
        return DoDragPiece(wParam, pdi);

    if (pdi->m_dragType == DRAG_PIECELIST)
        return DoDragPieceList(wParam, pdi);

    if (pdi->m_dragType == DRAG_MARKER)
        return DoDragMarker(wParam, pdi);

    if (pdi->m_dragType == DRAG_SELECTLIST)
        return DoDragSelectList(wParam, pdi);

    return 0;
}

LRESULT CPlayBoardView::DoDragPiece(WPARAM wParam, DragInfo* pdi)
{
    ASSERT(FALSE);      //!!!NOT USED???? //TODO: WHAT'S GOING ON HERE? 20200618
    if (pdi->m_pObj != (void*)GetDocument())
        return 0;               // Only pieces from our document.

    if (wParam == phaseDragExit)
        DragKillAutoScroll();
    else if (wParam == phaseDragOver)
    {
        DragCheckAutoScroll();
        return (LRESULT)(LPVOID)pdi->m_hcsrSuggest;
    }
    else if (wParam == phaseDragDrop)
    {
        CPoint pnt = pdi->m_point;
        ClientToWorkspace(pnt);
        AddPiece(pnt, (PieceID)pdi->m_dwVal);
        DragKillAutoScroll();
    }
    return 0;
}

LRESULT CPlayBoardView::DoDragPieceList(WPARAM wParam, DragInfo* pdi)
{
    if (pdi->m_pObj != (void*)GetDocument())
        return 0;               // Only pieces from our document.

    if (wParam == phaseDragExit)
        DragKillAutoScroll();
    else if (wParam == phaseDragOver)
    {
        DragCheckAutoScroll();
        return (LRESULT)(LPVOID)pdi->m_hcsrSuggest;
    }
    else if (wParam == phaseDragDrop)
    {
        CGamDoc* pDoc = GetDocument();
        CPoint pnt = pdi->m_point;
        CWordArray* pTbl = (CWordArray *)pdi->m_dwVal;
        ClientToWorkspace(pnt);

        // If the snap grid is on, adjust the point.
        CSize sz = GetDocument()->GetPieceTable()->GetStackedSize(pTbl,
            m_pPBoard->m_xStackStagger, m_pPBoard->m_yStackStagger);
        ASSERT(sz.cx != 0 && sz.cy != 0);
        CRect rct(CPoint(pnt.x - sz.cx/2, pnt.y - sz.cy/2), sz);
        AdjustRect(rct);
        pnt = GetMidRect(rct);

        m_selList.PurgeList(TRUE);          // Purge former selections
        GetDocument()->AssignNewMoveGroup();
        GetDocument()->PlacePieceListOnBoard(pnt, pTbl,
            m_pPBoard->m_xStackStagger, m_pPBoard->m_yStackStagger, m_pPBoard);

        if (!pDoc->HasPlayers() || !m_pPBoard->IsOwned() ||
            m_pPBoard->IsNonOwnerAccessAllowed() ||
            m_pPBoard->IsOwnedBy(pDoc->GetCurrentPlayerMask()))
        {
            CDrawList* pDwg = m_pPBoard->GetPieceList();
            CPtrList pceList;
            pDwg->GetObjectListFromPieceIDTable((CWordArray *)pdi->m_dwVal, &pceList);
            SelectAllObjectsInList(&pceList);   // Reselect pieces dropped on board
        }

        DragKillAutoScroll();
    }
    return 0;
}

#define MARKER_DROP_GAP_X     8

LRESULT CPlayBoardView::DoDragMarker(WPARAM wParam, DragInfo* pdi)
{
    CGamDoc* pDoc = GetDocument();
    if (pdi->m_pObj != (void*)pDoc)
        return 0;               // Only markers from our document.

    if (wParam == phaseDragExit)
        DragKillAutoScroll();
    else if (wParam == phaseDragOver)
    {
        DragCheckAutoScroll();
        return (LRESULT)(LPVOID)pdi->m_hcsrSuggest;
    }
    else if (wParam == phaseDragDrop)
    {
        CMarkManager* pMMgr = pDoc->GetMarkManager();
        CPoint pnt = pdi->m_point;
        MarkID mid = (MarkID)pdi->m_dwVal;
        ClientToWorkspace(pnt);

        // If Control is held and the marker tray is set to
        // deliver random markers, prompt for a count of markers
        // and randomly select that many of them. The snap grid is
        // ignored for this sort of placement.
        if (GetKeyState(VK_CONTROL) < 0)
        {
            // I'm going to cheat. I happen to know that marker drops
            // can only originate at the marker palette. I can find out
            // the current marker set this way.
            int nMrkGrp = pDoc->m_palMark.GetSelectedMarkerGroup();
            ASSERT(nMrkGrp >= 0);
            if (nMrkGrp < 0)
                goto NASTY_GOTO_TARGET;
            CMarkSet* pMSet = pMMgr->GetMarkSet(nMrkGrp);

            CMarkerCountDialog dlg;
            dlg.m_nMarkerCount = 2;
            if (dlg.DoModal() != IDOK)
                goto NASTY_GOTO_TARGET;
            ASSERT(dlg.m_nMarkerCount > 0);

            m_selList.PurgeList();

            CWordArray tblMarks;
            if (pMSet->IsRandomMarkerPull())
            {
                // Pull markers randomly from the marker group.
                tblMarks.Add(mid);              // Add the first one that was dropped
                pMSet->GetRandomSelection(dlg.m_nMarkerCount - 1, tblMarks, pDoc);
            }
            else
            {
                // Create duplicates of the one tile.
                for (int i = 0; i < dlg.m_nMarkerCount; i++)
                    tblMarks.Add(mid);          // Add the first one that was dropped
            }
            // First figure out the minimum size required.
            CSize sizeMin(0, 0);
            int i;
            for (i = 0; i < dlg.m_nMarkerCount; i++)
            {
                CSize size = pMMgr->GetMarkSize((MarkID)tblMarks[i]);
                sizeMin.cx += size.cx;
                sizeMin.cy = max(sizeMin.cy, size.cy);
                if (i < dlg.m_nMarkerCount - 1)
                    sizeMin += CSize(MARKER_DROP_GAP_X, 0);
            }
            CRect rct(CPoint(pnt.x - sizeMin.cx/2, pnt.y - sizeMin.cy), sizeMin);
            LimitRect(&rct);                    // Make sure stays on board.

            pDoc->AssignNewMoveGroup();
            int x = rct.right;
            int y = (rct.top + rct.bottom) / 2;
            // Load the list from right ot left so the objects
            // show up in the select list in top to bottom
            // corresponding to left to right.
            for (i = dlg.m_nMarkerCount - 1; i >= 0; i--)
            {
                CSize size = pMMgr->GetMarkSize((MarkID)tblMarks[i]);
                CDrawObj* pObj = pDoc->CreateMarkerObject(m_pPBoard, (MarkID)tblMarks[i],
                    CPoint(x - size.cx / 2, y), ObjectID());
                x -= size.cx + MARKER_DROP_GAP_X;
                m_selList.AddObject(pObj, TRUE);
            }
            NotifySelectListChange();
            return 0;
        }

NASTY_GOTO_TARGET:
        m_selList.PurgeList();
        // If the snap grid is on, adjust the point.
        CSize sz = pMMgr->GetMarkSize(mid);
        ASSERT(sz.cx != 0 && sz.cy != 0);
        CRect rct(CPoint(pnt.x - sz.cx/2, pnt.y - sz.cy/2), sz);
        AdjustRect(rct);
        pnt = GetMidRect(rct);

        pDoc->AssignNewMoveGroup();
        CDrawObj* pObj = pDoc->CreateMarkerObject(m_pPBoard, mid, pnt, ObjectID());

        // If marker is set to prompt for text on drop, show the
        // dialog.
        if (pMMgr->GetMark(mid)->m_flags & MarkDef::flagPromptText)
        {
            CEditElementTextDialog dlg;

            dlg.m_strText = pDoc->GetGameElementString(MakeMarkerElement(mid));

            if (dlg.DoModal() == IDOK)
            {
                GameElement elem = pDoc->GetGameElementCodeForObject(pObj);
                pDoc->SetObjectText(elem, dlg.m_strText.IsEmpty() ? NULL :
                    (LPCTSTR)dlg.m_strText);
            }
        }
    }
    return 0;
}

LRESULT CPlayBoardView::DoDragSelectList(WPARAM wParam, DragInfo* pdi)
{
    if (pdi->m_pObj != (void*)GetDocument())
        return 0;               // Only pieces from our document.

    ClientToWorkspace(pdi->m_point);

    CSelList *pSLst = (CSelList *)pdi->m_dwVal;
    CDC *pDC = GetDC();
    OnPrepareScaledDC(pDC, TRUE);

    if (wParam == phaseDragExit || wParam == phaseDragDrop ||
        wParam == phaseDragOver)
    {
        // Remove previous drag image.
        pSLst->DrawTracker(pDC, trkMoving);
    }
    if (wParam == phaseDragExit)
        DragKillAutoScroll();

    CRect rctSnapRef = pSLst->GetSnapReferenceRect();
    CPoint pntSnapRefTopLeft = rctSnapRef.TopLeft();

    CPoint pntMseOff = pntSnapRefTopLeft + pSLst->GetMouseOffset();
    CSize sizeDelta = pdi->m_point - pntMseOff; // Trial delta

    rctSnapRef += (CPoint)sizeDelta;    // Calc trial new position
    AdjustRect(rctSnapRef);         // Force onto grid.
    sizeDelta = rctSnapRef.TopLeft() - pntSnapRefTopLeft;   // Calc actual offset

    if (sizeDelta.cx != 0 || sizeDelta.cy != 0)
    {
        // We still have to make sure the larger rect hasn't left the
        // playing area.
        CRect rctObjs = pSLst->GetEnclosingRect();
        rctObjs += (CPoint)sizeDelta;               // Calc trial new position
        BOOL bXOK, bYOK;
        if (!IsRectFullyOnBoard(&rctObjs, &bXOK, &bYOK))
        {
            sizeDelta.cx = bXOK ? sizeDelta.cx : 0;
            sizeDelta.cy = bYOK ? sizeDelta.cy : 0;
        }
        if (sizeDelta.cx != 0 || sizeDelta.cy != 0) // Check 'em again (what a pain!)
            pSLst->Offset((CPoint)sizeDelta);
    }

    CRect rctObjs = pSLst->GetEnclosingRect();
    CPoint pntTopLeft = rctObjs.TopLeft();

    if (wParam == phaseDragOver || wParam == phaseDragEnter)
    {
        m_pDragSelList = pSLst;
        // Draw new drag image.
        pSLst->DrawTracker(pDC, trkMoving);
    }
    ReleaseDC(pDC);

    if (wParam == phaseDragOver)
    {
        DragCheckAutoScroll();
        return (LRESULT)(LPVOID)pdi->m_hcsrSuggest;
    }
    else if (wParam == phaseDragDrop)
    {
        CGamDoc* pDoc = GetDocument();

        // Whoooopppp...Whoooopppp!!! Drop occurred here....
        DragKillAutoScroll();
        CPtrList listObjs;
        pSLst->LoadListWithObjectPtrs(&listObjs);
        pSLst->PurgeList(FALSE);            // Purge source list

        pDoc->AssignNewMoveGroup();
        pDoc->PlaceObjectListOnBoard(&listObjs, rctObjs.TopLeft(), m_pPBoard);

        m_selList.PurgeList(TRUE);          // Purge former selections

        if (!pDoc->HasPlayers() || !m_pPBoard->IsOwned() ||
            m_pPBoard->IsNonOwnerAccessAllowed() ||
            m_pPBoard->IsOwnedBy(pDoc->GetCurrentPlayerMask()))
        {
            SelectAllObjectsInList(&listObjs);  // Reselect on this board.
        }

        CFrameWnd* pFrame = GetParentFrame();
        pFrame->SetActiveView(this);

        CGamDocHint hint;
        hint.m_pPBoard = m_pPBoard;
        hint.m_pSelList = &m_selList;
        pDoc->UpdateAllViews(this, HINT_UPDATESELECTLIST);

        NotifySelectListChange();
    }
    return 0;
}

void CPlayBoardView::DragDoAutoScroll()
{
    CPoint ptBefore(0, 0);
    ClientToWorkspace(ptBefore);
    CDC *pDC = NULL;

    if (m_pDragSelList != NULL)
    {
        // Remove previous drag image.
        pDC = GetDC();
        pDC->SaveDC();
        OnPrepareScaledDC(pDC, TRUE);
        m_pDragSelList->DrawTracker(pDC, trkMoving);
        pDC->RestoreDC(-1);
    }
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);
    BOOL bScrolled = ProcessAutoScroll(point);
    if (m_pDragSelList != NULL && bScrolled)
    {
        OnPrepareScaledDC(pDC, TRUE);
        CPoint ptAfter(0, 0);
        ClientToWorkspace(ptAfter);
        CSize sizeDelta = ptAfter - ptBefore;
        if (sizeDelta.cx != 0 || sizeDelta.cy != 0)
        {
            // We still have to make sure the larger rect hasn't left the
            // playing area.
            CRect rctObjs = m_pDragSelList->GetEnclosingRect();
            rctObjs += (CPoint)sizeDelta;               // Calc trial new position
            BOOL bXOK, bYOK;
            if (!IsRectFullyOnBoard(&rctObjs, &bXOK, &bYOK))
            {
                sizeDelta.cx = bXOK ? sizeDelta.cx : 0;
                sizeDelta.cy = bYOK ? sizeDelta.cy : 0;
            }
        }
        m_pDragSelList->Offset((CPoint)sizeDelta);
    }
    if (m_pDragSelList != NULL)
    {
        // Restore drag image.
        m_pDragSelList->DrawTracker(pDC, trkMoving);
    }
    if (pDC != NULL)
        ReleaseDC(pDC);
}

void CPlayBoardView::DragCheckAutoScroll()
{
    m_bInDrag = TRUE;
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);
    if (CheckAutoScroll(point))
    {
        if (m_nTimerID == 0)
            m_nTimerID = SetTimer(timerIDAutoScroll, timerAutoScroll, NULL);
    }
    else if (m_nTimerID != 0)
    {
        if (m_nTimerID != 0)
            KillTimer(m_nTimerID);
        m_nTimerID = 0;
    }
}

void CPlayBoardView::DragKillAutoScroll()
{
    m_bInDrag = FALSE;
    m_pDragSelList = NULL;
    if (m_nTimerID != 0)
        KillTimer(m_nTimerID);
    m_nTimerID = 0;
}

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::AddPiece(CPoint pnt, PieceID pid)
{
    ASSERT(FALSE);      //!!!!NO LONGER USED?!!!!!
    GetDocument()->PlacePieceOnBoard(pnt, pid, m_pPBoard);
}

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnPrepareScaledDC(CDC *pDC, BOOL bHonor180Flip)
{
    OnPrepareDC(pDC, NULL);
    PrepareScaledDC(pDC, NULL, bHonor180Flip);
}

void CPlayBoardView::SetupDrawListDC(CDC* pDC, CRect* pRct)
{
    if (m_nZoom == fullScale)
        return;

    pDC->SaveDC();
    PrepareScaledDC(pDC, pRct);
}

void CPlayBoardView::PrepareScaledDC(CDC *pDC, CRect* pRct, BOOL bHonor180Flip)
{
    CSize wsize, vsize;

    m_pPBoard->GetBoard()->GetBoardArray()->
        GetBoardScaling(m_nZoom, wsize, vsize);

    pDC->SetMapMode(MM_ANISOTROPIC);
    if (bHonor180Flip && m_pPBoard->IsBoardRotated180())
    {
        pDC->SetWindowExt(-wsize);
        pDC->SetWindowOrg(wsize.cx, wsize.cy);
    }
    else
        pDC->SetWindowExt(wsize);
    pDC->SetViewportExt(vsize);

    if (pRct != NULL)
        ScaleRect(*pRct, wsize, vsize);
}

void CPlayBoardView::RestoreDrawListDC(CDC *pDC)
{
    if (m_nZoom != fullScale)
        pDC->RestoreDC(-1);
}

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardView printing

BOOL CPlayBoardView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    return DoPreparePrinting(pInfo);
}

void CPlayBoardView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add extra initialization before printing
}

void CPlayBoardView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
CGamDoc* CPlayBoardView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGamDoc)));
    return (CGamDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Right mouse button handler

void CPlayBoardView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    // Make sure window is active.
    GetParentFrame()->ActivateFrame();

    CMenu bar;
    if (bar.LoadMenu(IDR_MENU_PLAYER_POPUPS))
    {
        UINT nMenuNum;
        if (GetDocument()->IsPlaying())
            nMenuNum = MENU_PV_PLAYMODE;
        else if (GetDocument()->IsScenario())
            nMenuNum = MENU_PV_SCNMODE;
        else
            nMenuNum = MENU_PV_MOVEMODE;

        CMenu& popup = *bar.GetSubMenu(nMenuNum);
        ASSERT(popup.m_hMenu != NULL);

        // Make sure we clean up even if exception is tossed.
        TRY
        {
            popup.TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y,
                AfxGetMainWnd()); // Route commands through main window
            // Make sure command is dispatched BEFORE we clear m_bInRightMouse.
            GetApp()->DispatchMessages();
        }
        END_TRY
    }
}

/////////////////////////////////////////////////////////////////////////////
// Handlers associated with tools.

BOOL CPlayBoardView::IsBoardContentsAvailableToCurrentPlayer()
{
    if (m_pPBoard->IsNonOwnerAccessAllowed() || !m_pPBoard->IsOwned() ||
            GetDocument()->IsScenario())
        return TRUE;

    return m_pPBoard->IsOwnedBy(GetDocument()->GetCurrentPlayerMask());
}

void CPlayBoardView::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (!IsBoardContentsAvailableToCurrentPlayer())
    {
        CScrollView::OnLButtonDown(nFlags, point);
        return;
    }

    PToolType eToolType = MapToolType(m_nCurToolID);
    CPlayTool* pTool = CPlayTool::GetTool(eToolType);
    // Allow pieces to be selected even during playback
    if (pTool != NULL)
    {
        ClientToWorkspace(point);
        pTool->OnLButtonDown(this, nFlags, point);
    }
    else
        CScrollView::OnLButtonDown(nFlags, point);
}

void CPlayBoardView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!IsBoardContentsAvailableToCurrentPlayer())
    {
        CScrollView::OnMouseMove(nFlags, point);
        return;
    }

    DoToolTipHitProcessing(point);

    PToolType eToolType = MapToolType(m_nCurToolID);
    CPlayTool* pTool = CPlayTool::GetTool(eToolType);
    if (!GetDocument()->IsPlaying() && pTool != NULL)
    {
        ClientToWorkspace(point);
        pTool->OnMouseMove(this, nFlags, point);
    }
    else
        CScrollView::OnMouseMove(nFlags, point);
}

void CPlayBoardView::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (!IsBoardContentsAvailableToCurrentPlayer())
    {
        CScrollView::OnLButtonUp(nFlags, point);
        return;
    }

    PToolType eToolType = MapToolType(m_nCurToolID);
    CPlayTool* pTool = CPlayTool::GetTool(eToolType);
    // Allow pieces to be selected even during playback
    if (pTool != NULL)
    {
        ClientToWorkspace(point);
        pTool->OnLButtonUp(this, nFlags, point);
    }
    else
        CScrollView::OnLButtonUp(nFlags, point);
}

void CPlayBoardView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if (!IsBoardContentsAvailableToCurrentPlayer())
    {
        CScrollView::OnLButtonDblClk(nFlags, point);
        return;
    }

    PToolType eToolType = MapToolType(m_nCurToolID);
    CPlayTool* pTool = CPlayTool::GetTool(eToolType);
    if (!GetDocument()->IsPlaying() && pTool != NULL)
    {
        ClientToWorkspace(point);
        pTool->OnLButtonDblClk(this, nFlags, point);
    }
    else
        CScrollView::OnLButtonDblClk(nFlags, point);
}

void CPlayBoardView::OnTimer(UINT nIDEvent)
{
    if (m_nTimerID == nIDEvent)
    {
        CPoint point;
        GetCursorPos(&point);
        ScreenToClient(&point);
        if (!m_bInDrag || !CheckAutoScroll(point))
        {
            KillTimer(m_nTimerID);
            return;
        }
        DragDoAutoScroll();
        return;
    }
    else
    {
        PToolType eToolType = MapToolType(m_nCurToolID);
        CPlayTool* pTool = CPlayTool::GetTool(eToolType);
        if (!GetDocument()->IsPlaying() && pTool != NULL)
            pTool->OnTimer(this, nIDEvent);
        else
            CScrollView::OnTimer(nIDEvent);
    }
}

BOOL CPlayBoardView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if (GetDocument()->IsPlaying())
        return CScrollView::OnSetCursor(pWnd, nHitTest, message);

    PToolType eToolType = MapToolType(m_nCurToolID);
    if (pWnd == this && eToolType != ptypeUnknown)
    {
        CPlayTool* pTool = CPlayTool::GetTool(eToolType);
        if (pTool != NULL && pTool->OnSetCursor(this, nHitTest))
            return TRUE;
    }
    if (GetDocument()->IsRecordingCompoundMove())
    {
        ::SetCursor(g_res.hcrCompMoveActive);
        return TRUE;
    }
    else
        return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CGamDoc* pDoc = GetDocument();
    if (!pDoc->IsPlaying())
    {
        if (nChar == VK_ESCAPE)
        {
            if (pDoc->IsRecordingCompoundMove())
                pDoc->RecordCompoundMoveDiscard();
            else if (m_pPBoard->GetPlotMoveMode())
                OnActPlotDiscard();
            else
                m_selList.PurgeList(TRUE);
        }
        else if (nChar == VK_RETURN)
        {
            if (pDoc->IsRecordingCompoundMove())
                pDoc->RecordCompoundMoveEnd();
            else if (m_pPBoard->GetPlotMoveMode())
                OnActPlotDone();
        }
    }
    CScrollView::OnChar(nChar, nRepCnt, nFlags);
}

void CPlayBoardView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (!IsBoardContentsAvailableToCurrentPlayer())
        return;

    if (GetDocument()->IsPlaying())
        return;

    if (nChar == VK_DELETE)
        OnEditClear();
}

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnEditClear()
{
    if (!m_selList.HasMarkers())
        return;                                  // Nothing to do

    ASSERT(!GetDocument()->IsPlaying() && m_selList.HasMarkers());
    if (m_pPBoard->GetPlotMoveMode())
        OnActPlotDiscard();

    if (AfxMessageBox(IDS_WARN_DELETEMARKERS, MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;

    CPtrList listPtr;
    m_selList.LoadListWithObjectPtrs(&listPtr);
    m_selList.PurgeList(TRUE);                  // Purge selections
    GetDocument()->AssignNewMoveGroup();
    GetDocument()->DeleteObjectsInList(&listPtr);
}

void CPlayBoardView::OnUpdateEditClear(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!GetDocument()->IsPlaying() && m_selList.HasMarkers());
}

//////////////////////////////////////////////////////////////////////

static PToolType tblTools[] =
{
    ptypeSelect,        // ID_PTOOL_SELECT
    ptypeLine,          // ID_PTOOL_LINE
    ptypeTextBox,       // ID_PTOOL_TEXTBOX
    ptypeMPlot,         // ID_PTOOL_PLOTMOVE
};

PToolType CPlayBoardView::MapToolType(UINT nToolResID)
{
    return tblTools[nToolResID - ID_PTOOL_SELECT];
}

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardView message handlers

void CPlayBoardView::OnUpdateIndicatorCellNum(CCmdUI* pCmdUI)
{
    CBoardArray* pba = m_pPBoard->GetBoard()->GetBoardArray();
    if (pba->GetCellNumTracking())
    {
        CPoint point;
        GetCursorPos(&point);
        ScreenToClient(&point);
        CRect rct;
        GetClientRect(&rct);
        if (rct.PtInRect(point))
        {
            point += (CSize)GetDeviceScrollPosition();
            CString str;
            pba->GetCellNumberStr(point, str, m_nZoom);
            pCmdUI->Enable();
            pCmdUI->SetText(str);
        }
    }
}

void CPlayBoardView::DoViewScaleBrd(TileScale nZoom)
{
    ASSERT(m_pPBoard != NULL);
    CBoard* pBoard = m_pPBoard->GetBoard();
    ASSERT(pBoard != NULL);

    CPoint pntMid;
    if (m_selList.IsAnySelects())
    {
        CRect rctSelection = m_selList.GetEnclosingRect();
        pntMid = rctSelection.CenterPoint();
    }
    else
    {
        CRect rctClient;
        GetClientRect(&rctClient);
        pntMid = rctClient.CenterPoint();
        ClientToWorkspace(pntMid);
    }

    m_nZoom = nZoom;
    SetOurScrollSizes(m_nZoom);
    BeginWaitCursor();
    Invalidate(FALSE);

    CenterViewOnWorkspacePoint(pntMid);

    UpdateWindow();
    EndWaitCursor();
}

void CPlayBoardView::OnViewFullScaleBrd()
{
    DoViewScaleBrd(fullScale);
}

void CPlayBoardView::OnUpdateViewFullScaleBrd(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_nZoom == fullScale);
}

void CPlayBoardView::OnViewHalfScaleBrd()
{
    DoViewScaleBrd(halfScale);
}

void CPlayBoardView::OnUpdateViewHalfScaleBrd(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_nZoom == halfScale);
}

void CPlayBoardView::OnViewSmallScaleBoard()
{
    DoViewScaleBrd(smallScale);
}

void CPlayBoardView::OnUpdateViewSmallScaleBoard(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_nZoom == smallScale);
}

void CPlayBoardView::OnViewToggleScale()
{
    if (GetKeyState(VK_CONTROL) < 0)
    {
        // Zoom out and around
        if (m_nZoom == fullScale)
            DoViewScaleBrd(smallScale);
        else if (m_nZoom == halfScale)
            DoViewScaleBrd(fullScale);
        else
            DoViewScaleBrd(halfScale);
    }
    else
    {
        // Zoom in and around
        if (m_nZoom == fullScale)
            DoViewScaleBrd(halfScale);
        else if (m_nZoom == halfScale)
            DoViewScaleBrd(smallScale);
        else
            DoViewScaleBrd(fullScale);
    }
}

void CPlayBoardView::OnUpdateViewToggleScale(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void CPlayBoardView::OnViewBoardRotate180()
{
   m_pPBoard->SetRotateBoard180(!m_pPBoard->IsBoardRotated180());
   CGamDocHint hint;
   hint.m_pPBoard = m_pPBoard;
   GetDocument()->UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);
}

void CPlayBoardView::OnUpdateViewBoardRotate180(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_pPBoard->IsBoardRotated180());
}

BOOL CPlayBoardView::OnPlayTool(UINT id)
{
    if (id !=ID_PTOOL_SELECT)
        m_selList.PurgeList(TRUE);
    if (id != m_nCurToolID)
    {
        m_nCurToolID = id;
    }
    return TRUE;
}

void CPlayBoardView::OnUpdatePlayTool(CCmdUI* pCmdUI)
{
    // NOTE!!: The control ID's are assumed to be consecutive and
    // in the same order as the tool codes defined in MAINFRM.C
    if (GetDocument()->IsPlaying())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(pCmdUI->m_nID == m_nCurToolID);
    pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurToolID);
}

void CPlayBoardView::OnActStack()
{
    DoAutostackOfSelectedObjects(m_pPBoard->m_xStackStagger,
        m_pPBoard->m_yStackStagger);
}

void CPlayBoardView::DoAutostackOfSelectedObjects(int xStagger, int yStagger)
{
    CRect rct = m_selList.GetPiecesEnclosingRect();
    if (rct.IsRectEmpty())
        return;

    CPoint pntCenter(MidPnt(rct.left, rct.right), MidPnt(rct.top, rct.bottom));

    CPtrArray tblObjs;
    m_selList.LoadTableWithObjectPtrs(&tblObjs);

    m_selList.PurgeList(TRUE);              // Purge former selections

    GetDocument()->AssignNewMoveGroup();
    GetDocument()->PlaceObjectTableOnBoard(pntCenter, &tblObjs,
        xStagger, yStagger, m_pPBoard);

    // Reselect the pieces.
    SelectAllObjectsInTable(&tblObjs);      // Reselect objects
}

void CPlayBoardView::OnUpdateActStack(CCmdUI* pCmdUI)
{
    if (GetDocument()->IsPlaying())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_selList.IsMultipleSelects());
}

void CPlayBoardView::OnActAutostackDeck()
{
    DoAutostackOfSelectedObjects(0, 0);
}

void CPlayBoardView::OnUpdateActAutostackDeck(CCmdUI* pCmdUI)
{
    OnUpdateActStack(pCmdUI);
}

void CPlayBoardView::OnActShuffleSelectedObjects()
{
    CRect rct = m_selList.GetPiecesEnclosingRect();
    if (rct.IsRectEmpty())
        return;

    CGamDoc* pDoc = GetDocument();          // Shorthand pointer

    CPoint pntCenter(MidPnt(rct.left, rct.right), MidPnt(rct.top, rct.bottom));

    CPtrArray tblObjs;
    m_selList.LoadTableWithObjectPtrs(&tblObjs);

    m_selList.PurgeList(TRUE);              // Purge former selections

    // Generate a shuffled index vector for the number of selected items
    UINT nRandSeed = pDoc->GetRandomNumberSeed();
    int nNumIndices = tblObjs.GetSize();
    int* pnIndices = AllocateAndCalcRandomIndexVector(nNumIndices,
        nNumIndices, nRandSeed, &nRandSeed);
    pDoc->SetRandomNumberSeed(nRandSeed);

    // Create a shuffled table of objects...
    CPtrArray tblRandObjs;
    for (int i = 0; i < tblObjs.GetSize(); i++)
        tblRandObjs.Add(tblObjs[pnIndices[i]]);

    pDoc->AssignNewMoveGroup();

    if (pDoc->IsRecording())
    {
        // Insert a notification tip so there is some information
        // feedback during playback.
        CString strMsg;
        strMsg.Format(IDS_TIP_OBJS_SHUFFLED, tblRandObjs.GetSize());
        pDoc->RecordEventMessage(strMsg, TRUE, m_pPBoard->GetSerialNumber(),
            pntCenter.x, pntCenter.y);
    }

    pDoc->PlaceObjectTableOnBoard(&tblRandObjs, m_pPBoard);

    // Reselect the pieces.
    SelectAllObjectsInTable(&tblRandObjs);  // Reselect objects
}

void CPlayBoardView::OnUpdateActShuffleSelectedObjects(CCmdUI* pCmdUI)
{
    if (GetDocument()->IsPlaying())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_selList.IsMultipleSelects());
}

void CPlayBoardView::OnActToFront()
{
    CRect rct = m_selList.GetEnclosingRect();
    if (rct.IsRectEmpty())
        return;

    CPtrList listObjs;
    m_selList.LoadListWithObjectPtrs(&listObjs);

    m_selList.PurgeList(TRUE);          // Purge former selections

    GetDocument()->AssignNewMoveGroup();
    GetDocument()->PlaceObjectListOnBoard(&listObjs, rct.TopLeft(),
        m_pPBoard, placeTop);

    SelectAllObjectsInList(&listObjs);  // Reselect pieces
}

void CPlayBoardView::OnUpdateActToFront(CCmdUI* pCmdUI)
{
    if (GetDocument()->IsPlaying())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_selList.IsAnySelects());
}

void CPlayBoardView::OnActToBack()
{
    CRect rct = m_selList.GetEnclosingRect();
    if (rct.IsRectEmpty())
        return;

    CPtrList listObjs;
    m_selList.LoadListWithObjectPtrs(&listObjs);

    m_selList.PurgeList(TRUE);          // Purge former selections

    GetDocument()->AssignNewMoveGroup();
    GetDocument()->PlaceObjectListOnBoard(&listObjs, rct.TopLeft(),
        m_pPBoard, placeBack);

    SelectAllObjectsInList(&listObjs);  // Reselect pieces
}

void CPlayBoardView::OnUpdateActToBack(CCmdUI* pCmdUI)
{
    if (GetDocument()->IsPlaying())
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_selList.IsAnySelects());
}

void CPlayBoardView::OnActTurnOver()
{
    CPtrList listObjs;
    m_selList.LoadListWithObjectPtrs(&listObjs);

    m_selList.PurgeList(TRUE);          // Purge former selections

    GetDocument()->AssignNewMoveGroup();
    GetDocument()->InvertPlayingPieceListOnBoard(&listObjs, m_pPBoard);

    SelectAllObjectsInList(&listObjs);  // Reselect pieces
}

void CPlayBoardView::OnUpdateActTurnOver(CCmdUI* pCmdUI)
{
    CGamDoc* pDoc = GetDocument();
    if (pDoc->IsPlaying() || !pDoc->IsScenario() &&
            m_selList.HasOwnedPiecesNotMatching(pDoc->GetCurrentPlayerMask()))
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_selList.Has2SidedPieces());
}

void CPlayBoardView::OnActPlotMove()
{
    // Do this call so we don't record the plot list in the
    // restoration record.
    GetDocument()->CreateRecordListIfRequired();
    // Ok...finish plot setup
    m_pPBoard->SetPlotMoveMode(TRUE);
    m_pPBoard->InitPlotStartPoint();
    m_nCurToolID = ID_PTOOL_PLOTMOVE;
}

void CPlayBoardView::OnUpdateActPlotMove(CCmdUI* pCmdUI)
{
    if (GetDocument()->IsPlaying() || (!m_selList.HasPieces() &&
        !m_selList.HasMarkers()) || GetDocument()->IsScenario())
    {
        if (pCmdUI->m_pSubMenu != NULL)
        {
            pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
                MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        }
        else
            pCmdUI->Enable(FALSE);
        return;
    }
    else if (pCmdUI->m_pSubMenu != NULL)
    {
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | MF_ENABLED);
    }

    pCmdUI->Enable(!m_pPBoard->GetPlotMoveMode());
    pCmdUI->SetCheck(m_pPBoard->GetPlotMoveMode());
}

void CPlayBoardView::OnActPlotDone()
{
    if (m_pPBoard->GetPrevPlotPoint() != CPoint(-1, -1))
    {
        CRect rct = m_selList.GetPiecesEnclosingRect();
        CPoint ptPrev = m_pPBoard->GetPrevPlotPoint();
        ptPrev -= CSize(rct.Width() / 2, rct.Height() / 2);
        AdjustPoint(ptPrev);

        CPtrList listObjs;
        m_selList.LoadListWithObjectPtrs(&listObjs);
        m_selList.PurgeList(TRUE);

        GetDocument()->AssignNewMoveGroup();

        // Note that PlaceObjectListOnBoard() automatically detects the
        // plotted move case and records that fact.
        GetDocument()->PlaceObjectListOnBoard(&listObjs, ptPrev, m_pPBoard);
        m_selList.PurgeList(TRUE);          // Purge former selections
        SelectAllObjectsInList(&listObjs);  // Select on this board.
    }
    m_pPBoard->SetPlotMoveMode(FALSE);
    GetDocument()->UpdateAllBoardIndicators(m_pPBoard);
    m_pPBoard->FlushAllIndicators();
    m_nCurToolID = ID_PTOOL_SELECT;
}

void CPlayBoardView::OnUpdateActPlotDone(CCmdUI* pCmdUI)
{
    if (GetDocument()->IsPlaying() || GetDocument()->IsScenario())
    {
        pCmdUI->Enable(FALSE);
        return;
    }
    pCmdUI->Enable(m_pPBoard->GetPlotMoveMode());
}

void CPlayBoardView::OnActPlotDiscard()
{
    m_pPBoard->SetPlotMoveMode(FALSE);
    GetDocument()->UpdateAllBoardIndicators(m_pPBoard);
    m_pPBoard->FlushAllIndicators();
    m_nCurToolID = ID_PTOOL_SELECT;
}

void CPlayBoardView::OnUpdateActPlotDiscard(CCmdUI* pCmdUI)
{
    if (GetDocument()->IsPlaying() || GetDocument()->IsScenario())
    {
        pCmdUI->Enable(FALSE);
        return;
    }
    pCmdUI->Enable(m_pPBoard->GetPlotMoveMode());
}

void CPlayBoardView::OnViewSnapGrid()
{
    m_pPBoard->m_bGridSnap = !m_pPBoard->m_bGridSnap;
}

void CPlayBoardView::OnUpdateViewSnapGrid(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!GetDocument()->IsPlaying());
    pCmdUI->SetCheck(m_pPBoard->m_bGridSnap);
}

void CPlayBoardView::OnEditSelAllMarkers()
{
    SelectAllMarkers();
}

void CPlayBoardView::OnUpdateEditSelAllMarkers(CCmdUI* pCmdUI)
{
    CDrawList* pDwg = m_pPBoard->GetPieceList();
    ASSERT(pDwg);
    pCmdUI->Enable(!GetDocument()->IsPlaying() && pDwg->HasMarker());
}

void CPlayBoardView::OnActRotate()      // ** TEST CODE ** //
{
    CWordArray tbl;
    CGamDoc* pDoc = GetDocument();
    m_selList.LoadTableWithPieceIDs(&tbl);
    TileID tid = pDoc->GetPieceTable()->GetActiveTileID((PieceID)tbl[0]);
    CTile tile;
    pDoc->GetTileManager()->GetTile(tid, &tile);
    CBitmap bmap;
    tile.CreateBitmapOfTile(&bmap);
    CRotateDialog dlg;
    dlg.m_pBMap = &bmap;
    dlg.m_crTrans = pDoc->GetTileManager()->GetTransparentColor();
    dlg.DoModal();
}

void CPlayBoardView::OnUpdateActRotate(CCmdUI* pCmdUI) // ** TEST CODE ** //
{
    CGamDoc* pDoc = GetDocument();
    if (pDoc->IsPlaying() || m_pPBoard->GetPlotMoveMode() || !pDoc->IsScenario() &&
            m_selList.HasOwnedPiecesNotMatching(pDoc->GetCurrentPlayerMask()))
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_selList.HasPieces());
}

///////////////////////////////////////////////////////////////////////
// Handle rotation requests. The ID's for tile rotations
// start at: ID_ACT_ROTATE_0 with the ID number being stepped at
// five degree increments. For example: if ID_ACT_ROTATE_0 is 42000, then
// ID_ACT_ROTATE_90 must be 42009. This makes the angle easy to compute.

void CPlayBoardView::OnRotatePiece(UINT nID)
{
    int nFacing5DegCW = nID - ID_ACT_ROTATE_0;
    CPtrList listObjs;
    m_selList.LoadListWithObjectPtrs(&listObjs);

    m_selList.PurgeList(TRUE);          // Purge former selections

    GetDocument()->AssignNewMoveGroup();
    GetDocument()->ChangePlayingPieceFacingListOnBoard(&listObjs, m_pPBoard,
        5 * nFacing5DegCW);             // Convert to degrees

    SelectAllObjectsInList(&listObjs);  // Reselect pieces
}

void CPlayBoardView::OnUpdateRotatePiece(CCmdUI* pCmdUI, UINT nID)
{
    CGamDoc* pDoc = GetDocument();
    BOOL bEnabled = (m_selList.HasPieces() || m_selList.HasMarkers()) && !pDoc->IsPlaying();
    if (bEnabled && !pDoc->IsScenario() &&
        m_selList.HasOwnedPiecesNotMatching(pDoc->GetCurrentPlayerMask()))
    {
        bEnabled = FALSE;
    }

    if (pCmdUI->m_pSubMenu != NULL)
    {
        // Need to handle menu that the submenu is connected to.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (bEnabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    pCmdUI->Enable(bEnabled);
}

///////////////////////////////////////////////////////////////////////
// This method handles messages typically sent by the tiny map view.
// WPARAM = POINT*

LRESULT CPlayBoardView::OnMessageCenterBoardOnPoint(WPARAM wParam, LPARAM lParam)
{
    CPoint pnt = *((POINT*)wParam);
    CenterViewOnWorkspacePoint(pnt);
    return (LRESULT)0;
}

///////////////////////////////////////////////////////////////////////
// This method handles notifications of changes of relative piece
// rotation sent from the CRotatePieceDialog dialog.

LRESULT CPlayBoardView::OnMessageRotateRelative(WPARAM wParam, LPARAM lParam)
{
    CGamDoc* pDoc = GetDocument();
    int nRelativeRotation = (int)wParam;
    ASSERT(m_tblCurPieces.GetCount() > 0);
    ASSERT(m_tblCurAngles.GetSize() == m_tblCurPieces.GetCount());

    POSITION pos = m_tblCurPieces.GetHeadPosition();
    for (int i = 0; pos != NULL; i++)
    {
        int nAngle = m_tblCurAngles[i] + nRelativeRotation;
        nAngle += nAngle < 0 ? 360 : 0;     // Shift to positive values
        nAngle %= 360;                      // Keep to within a circle

        CDrawObj* pDObj = (CDrawObj*)m_tblCurPieces.GetNext(pos);
        if (pDObj->GetType() == CDrawObj::drawPieceObj)
            pDoc->ChangePlayingPieceFacingOnBoard((CPieceObj*)pDObj, m_pPBoard, nAngle);
        else if (pDObj->GetType() == CDrawObj::drawMarkObj)
            pDoc->ChangeMarkerFacingOnBoard((CMarkObj*)pDObj, m_pPBoard, nAngle);
        if (m_bWheelRotation &&
            (pDObj->GetType() == CDrawObj::drawPieceObj || pDObj->GetType() == CDrawObj::drawMarkObj))
        {
            // Calculate new rotated mid-point for object.
            CPoint pntRotate = RotatePointAroundPoint(m_pntWheelMid,
                CPoint(m_tblXMidPnt[i], m_tblYMidPnt[i]), nRelativeRotation);
            CSize sizeDelta = pntRotate - GetMidRect(pDObj->GetEnclosingRect());
            pDoc->PlaceObjectOnBoard(m_pPBoard, pDObj, sizeDelta);
        }
    }
    return (LRESULT)0;
}

void CPlayBoardView::OnActRotateRelative()
{
    DoRotateRelative(FALSE);
}

void CPlayBoardView::OnActRotateGroupRelative()
{
    DoRotateRelative(TRUE);
}

void CPlayBoardView::DoRotateRelative(BOOL bWheelRotation)
{
    m_bWheelRotation = bWheelRotation;

    CRotatePieceDialog dlg(this);
    CGamDoc* pDoc = GetDocument();
    CPieceTable* pPTbl = pDoc->GetPieceTable();

    // Get a list of the selected pieces and save their current
    // rotations.
    m_tblCurAngles.RemoveAll();
    m_tblCurPieces.RemoveAll();
    m_tblXMidPnt.RemoveAll();
    m_tblYMidPnt.RemoveAll();

    m_selList.LoadListWithObjectPtrs(&m_tblCurPieces);

    CRect rctGroupRect = m_selList.GetPiecesEnclosingRect();
    m_pntWheelMid = GetMidRect(rctGroupRect);

    POSITION pos = m_tblCurPieces.GetHeadPosition();
    for (int i = 0; pos != NULL; i++)
    {
        CDrawObj* pDObj = (CDrawObj*)m_tblCurPieces.GetNext(pos);
        if (pDObj->GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj* pObj = (CPieceObj*)pDObj;
            m_tblCurAngles.Add(pPTbl->GetPieceFacing(pObj->m_pid));
        }
        else if (pDObj->GetType() == CDrawObj::drawMarkObj)
        {
            CMarkObj* pObj = (CMarkObj*)pDObj;
            m_tblCurAngles.Add(pObj->GetFacing());
        }
        if (m_bWheelRotation &&
            (pDObj->GetType() == CDrawObj::drawPieceObj || pDObj->GetType() == CDrawObj::drawMarkObj))
        {
            CPoint midPoint = GetMidRect(pDObj->GetEnclosingRect());
            m_tblXMidPnt.Add((UINT)midPoint.x);
            m_tblYMidPnt.Add((UINT)midPoint.y);
        }
    }
    // If we're recording moves right now, suspend it for the moment.
    BOOL bRecording = pDoc->IsRecording();
    if (bRecording)
        pDoc->SetGameState(CGamDoc::stateNotRecording);

    // Show the rotation dialog
    INT_PTR nDlgResult = dlg.DoModal();

    // Restore angles before possibly recording the operation.
    pos = m_tblCurPieces.GetHeadPosition();
    for (int i = 0; pos != NULL; i++)
    {
        CDrawObj* pDObj = (CDrawObj*)m_tblCurPieces.GetNext(pos);
        if (pDObj->GetType() == CDrawObj::drawPieceObj)
        {
            pDoc->ChangePlayingPieceFacingOnBoard((CPieceObj*)pDObj,
                m_pPBoard, m_tblCurAngles[i]);
        }
        else if (pDObj->GetType() == CDrawObj::drawMarkObj)
        {
            pDoc->ChangeMarkerFacingOnBoard((CMarkObj*)pDObj, m_pPBoard,
                m_tblCurAngles[i]);
        }
        if (m_bWheelRotation &&
            (pDObj->GetType() == CDrawObj::drawPieceObj || pDObj->GetType() == CDrawObj::drawMarkObj))
        {
            // Restore original position
            CSize sizeDelta = CPoint(m_tblXMidPnt[i], m_tblYMidPnt[i]) -
                GetMidRect(pDObj->GetEnclosingRect());
            pDoc->PlaceObjectOnBoard(m_pPBoard, pDObj, sizeDelta);
        }
    }
    // Restore recording mode if it was active.
    if (bRecording)
        pDoc->SetGameState(CGamDoc::stateRecording);
    if (nDlgResult == IDOK)
    {
        // Rotation was accepted. Make the final changes.
        pDoc->AssignNewMoveGroup();
        POSITION pos = m_tblCurPieces.GetHeadPosition();
        for (int i = 0; pos != NULL; i++)
        {
            int nAngle = m_tblCurAngles[i] + dlg.m_nRelativeRotation;
            nAngle += nAngle < 0 ? 360 : 0;
            nAngle %= 360;                      // Keep to within a circle

            CDrawObj* pDObj = (CDrawObj*)m_tblCurPieces.GetNext(pos);

            if (pDObj->GetType() == CDrawObj::drawPieceObj)
            {
                pDoc->ChangePlayingPieceFacingOnBoard((CPieceObj*)pDObj, m_pPBoard, nAngle);
            }
            else if (pDObj->GetType() == CDrawObj::drawMarkObj)
            {
                pDoc->ChangeMarkerFacingOnBoard((CMarkObj*)pDObj, m_pPBoard, nAngle);
            }
            if (m_bWheelRotation &&
                (pDObj->GetType() == CDrawObj::drawPieceObj || pDObj->GetType() == CDrawObj::drawMarkObj))
            {
                // Calculate new rotated mid-point for object.
                CPoint pntRotate = RotatePointAroundPoint(m_pntWheelMid,
                    CPoint(m_tblXMidPnt[i], m_tblYMidPnt[i]), dlg.m_nRelativeRotation);
                CSize sizeDelta = pntRotate - GetMidRect(pDObj->GetEnclosingRect());
                pDoc->PlaceObjectOnBoard(m_pPBoard, pDObj, sizeDelta);
            }
        }
        m_selList.UpdateObjects(TRUE, FALSE);
        // Make sure we erase old handles.
        rctGroupRect.InflateRect(16, 16);
        InvalidateRect(rctGroupRect, FALSE);
    }
    m_tblCurAngles.RemoveAll();
    m_tblCurPieces.RemoveAll();
    m_tblXMidPnt.RemoveAll();
    m_tblYMidPnt.RemoveAll();
}

void CPlayBoardView::OnUpdateActRotateRelative(CCmdUI* pCmdUI)
{
    CGamDoc* pDoc = GetDocument();
    if (pDoc->IsPlaying() || !pDoc->IsScenario() &&
            m_selList.HasOwnedPiecesNotMatching(pDoc->GetCurrentPlayerMask()))
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_selList.HasPieces() || m_selList.HasMarkers());
}

void CPlayBoardView::OnUpdateActRotateGroupRelative(CCmdUI *pCmdUI)
{
    CGamDoc* pDoc = GetDocument();
    if (pDoc->IsPlaying() || !pDoc->IsScenario() &&
            m_selList.HasOwnedPiecesNotMatching(pDoc->GetCurrentPlayerMask()))
        pCmdUI->Enable(FALSE);
    else
    {
        pCmdUI->Enable(m_selList.IsMultipleSelects() &&
            (m_selList.HasPieces() || m_selList.HasMarkers()));
    }
}

///////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnViewPieces()
{
    GetPlayBoard()->SetPiecesVisible(!GetPlayBoard()->GetPiecesVisible());
    CGamDocHint hint;
    hint.m_pPBoard = m_pPBoard;
    GetDocument()->UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);
}

void CPlayBoardView::OnUpdateViewPieces(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(!GetPlayBoard()->GetPiecesVisible());
}

///////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnEditCopy()
{
    CBoard* pBoard = m_pPBoard->GetBoard();
    CWindowDC scrnDC(this);

    SetupPalette(&scrnDC);
    CSize size = pBoard->GetSize(m_nZoom);

    CBitmap bmap;
    bmap.Attach(Create16BitDIBSection(scrnDC.m_hDC, size.cx, size.cy));
    CDC dcMem;
    dcMem.CreateCompatibleDC(&scrnDC);
    CBitmap* pPrvBMap = (CBitmap*)dcMem.SelectObject(&bmap);
    SetupPalette(&dcMem);

    CRect rct(0, 0, size.cx, size.cy);

    // Draw base board image...
    pBoard->Draw(&dcMem, &rct, m_nZoom, m_pPBoard->m_bCellBorders);

    // Draw pieces etc.....
    SetupDrawListDC(&dcMem, &rct);
    m_pPBoard->Draw(&dcMem, &rct, m_nZoom);
    RestoreDrawListDC(&dcMem);

    GdiFlush();
    dcMem.SelectObject(pPrvBMap);

    if (OpenClipboard())
    {
        BeginWaitCursor();
        EmptyClipboard();

        CDib dib;
        dib.BitmapToDIB(&bmap, GetAppPalette());
        SetClipboardData(CF_DIB, CopyHandle((HANDLE)dib.m_hDib));

        CloseClipboard();
        EndWaitCursor();
    }
}

void CPlayBoardView::OnEditBoardToFile()
{
    CString strFilter;
    strFilter.LoadString(IDS_BMP_FILTER);
    CString strTitle;
    strTitle.LoadString(IDS_SEL_BITMAPFILE);

    CFileDialog dlg(FALSE, "bmp", NULL, OFN_HIDEREADONLY |
        OFN_OVERWRITEPROMPT, strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return;

    BeginWaitCursor();
    TRY
    {
        CFile file;
        CFileException fe;

        if (!file.Open(dlg.GetPathName(),
            CFile::modeCreate | CFile::modeWrite, &fe))
        {
            EndWaitCursor();
            AfxMessageBox(IDP_ERR_BMPCREATE, MB_ICONEXCLAMATION);
            EndWaitCursor();
            return;
        }

        CBoard* pBoard = m_pPBoard->GetBoard();
        CWindowDC scrnDC(this);

        SetupPalette(&scrnDC);
        CSize size = pBoard->GetSize(m_nZoom);

        CBitmap bmap;
        bmap.Attach(Create16BitDIBSection(scrnDC.m_hDC,
            size.cx, size.cy));
        CDC dcMem;
        dcMem.CreateCompatibleDC(&scrnDC);
        CBitmap* pPrvBMap = (CBitmap*)dcMem.SelectObject(&bmap);
        SetupPalette(&dcMem);

        CRect rct(0, 0, size.cx, size.cy);

        // Draw base board image...
        pBoard->Draw(&dcMem, &rct, m_nZoom, m_pPBoard->m_bCellBorders);

        // Draw pieces etc.....
        SetupDrawListDC(&dcMem, &rct);
        m_pPBoard->Draw(&dcMem, &rct, m_nZoom);
        RestoreDrawListDC(&dcMem);

        GdiFlush();
        dcMem.SelectObject(pPrvBMap);

        CDib dib;
        dib.BitmapToDIB(&bmap, GetAppPalette(), 24);

        if (!dib.WriteDIBFile(file))
            AfxThrowMemoryException();

        EndWaitCursor();
    }
    CATCH_ALL (e)
    {
        EndWaitCursor();
        AfxMessageBox(IDP_ERR_BMPWRITE, MB_ICONEXCLAMATION);
    }
    END_CATCH_ALL
}

void CPlayBoardView::OnEditBoardProperties()
{
    GetDocument()->DoBoardProperties(GetPlayBoard());
}

void CPlayBoardView::OnSelectGroupMarkers(UINT nID)
{
    SelectMarkersInGroup(nID - ID_MRKGROUP_FIRST);
}

void CPlayBoardView::OnUpdateSelectGroupMarkers(CCmdUI* pCmdUI, UINT nID)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        CMarkManager* pMgr = GetDocument()->GetMarkManager();
        if (pMgr->GetNumMarkSets() == 0)
            return;
        CStringArray tbl;
        for (int i = 0; i < pMgr->GetNumMarkSets(); i++)
            tbl.Add(pMgr->GetMarkSet(i)->GetName());
        CMenu menu;
        VERIFY(menu.CreatePopupMenu());

        CreateSequentialSubMenuIDs(menu, ID_MRKGROUP_FIRST, tbl);

        CString str;
        VERIFY(pCmdUI->m_pMenu->GetMenuString(pCmdUI->m_nIndex, str,
            MF_BYPOSITION));
        VERIFY(pCmdUI->m_pMenu->ModifyMenu(pCmdUI->m_nIndex,
            MF_BYPOSITION | MF_ENABLED | MF_POPUP | MF_STRING,
            (UINT)menu.Detach(), str));
    }
    else
        pCmdUI->Enable();
}

void CPlayBoardView::OnViewDrawIndOnTop()
{
    GetPlayBoard()->SetIndicatorsOnTop(!GetPlayBoard()->GetIndicatorsOnTop());
    CGamDocHint hint;
    hint.m_pPBoard = m_pPBoard;
    GetDocument()->UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);
}

void CPlayBoardView::OnUpdateViewDrawIndOnTop(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(GetPlayBoard()->GetIndicatorsOnTop());
}

void CPlayBoardView::OnEditElementText()
{
    ASSERT(m_selList.IsSingleSelect() && (m_selList.HasMarkers() || m_selList.HasPieces()));

    CDrawObj* pDObj = m_selList.GetHead()->m_pObj;
    GetDocument()->DoEditObjectText(pDObj);
    NotifySelectListChange();       // Make sure indicators are updated
}

void CPlayBoardView::OnUpdateEditElementText(CCmdUI* pCmdUI)
{
    CGamDoc* pDoc = GetDocument();
    if (pDoc->IsPlaying() || !pDoc->IsScenario() &&
            m_selList.HasOwnedPiecesNotMatching(pDoc->GetCurrentPlayerMask()))
        pCmdUI->Enable(FALSE);
    else
    {
        pCmdUI->Enable(m_selList.IsSingleSelect() &&
            (m_selList.HasMarkers() || m_selList.HasPieces()));
    }
}

void CPlayBoardView::OnActLockObject()
{
    int nSet;
    int nClear;
    m_selList.CountDObjFlags(dobjFlgLockDown, nSet, nClear);

    // If any are locked then force an unlock. All must be unlocked
    // to do a lock.
    BOOL bLockState = nSet != 0 ? FALSE : TRUE;

    CPtrList listObjs;
    m_selList.LoadListWithObjectPtrs(&listObjs);

    GetDocument()->AssignNewMoveGroup();
    GetDocument()->SetObjectLockdownList(&listObjs, bLockState);

    if (m_pPBoard->GetLocksEnforced() && bLockState)
        m_selList.PurgeList(TRUE);          // Purge former selections
}

void CPlayBoardView::OnUpdateActLockObject(CCmdUI* pCmdUI)
{
    if (GetDocument()->IsPlaying())
    {
        pCmdUI->Enable(FALSE);
        return;
    }

    int nSet;
    int nClear;

    m_selList.CountDObjFlags(dobjFlgLockDown, nSet, nClear);

    if (nSet != 0 && nClear != 0)
        pCmdUI->SetCheck(2);
    else if (nSet != 0 && nClear == 0)
        pCmdUI->SetCheck(1);
    else
        pCmdUI->SetCheck(0);
    pCmdUI->Enable(m_selList.GetCount() > 0);
}

void CPlayBoardView::OnActLockSuspend()
{
    m_pPBoard->SetLocksEnforced(!m_pPBoard->GetLocksEnforced());
    // If enforcement is on and objects are locked, deselect them
    if (m_pPBoard->GetLocksEnforced())
        m_selList.DeselectIfDObjFlagsSet(dobjFlgLockDown);
}

void CPlayBoardView::OnUpdateActLockSuspend(CCmdUI* pCmdUI)
{
    if (GetDocument()->IsPlaying())
        pCmdUI->Enable(FALSE);
    else
    {
        pCmdUI->Enable(TRUE);
        pCmdUI->SetCheck(!m_pPBoard->GetLocksEnforced());
    }
}


void CPlayBoardView::OnActTakeOwnership()
{
    CRect rct = m_selList.GetPiecesEnclosingRect(FALSE);
    if (rct.IsRectEmpty())
        return;

    CPoint pntCenter(MidPnt(rct.left, rct.right), MidPnt(rct.top, rct.bottom));

    CGamDoc* pDoc = GetDocument();

    CWordArray tblPieces;

    m_selList.LoadTableWithOwnerStatePieceIDs(&tblPieces, m_selList.LF_NOTOWNED);

    pDoc->AssignNewMoveGroup();

    if (pDoc->IsRecording())
    {
        // Insert a notification tip so there is some information
        // feedback during playback.
        CString strMsg;
        strMsg.LoadString(IDS_TIP_OWNER_ACQUIRED);
        pDoc->RecordEventMessage(strMsg, TRUE, m_pPBoard->GetSerialNumber(),
            pntCenter.x, pntCenter.y);
    }

    pDoc->SetPieceOwnershipTable(&tblPieces, pDoc->GetCurrentPlayerMask());

    CGamDocHint hint;
    hint.m_pPBoard = m_pPBoard;
    pDoc->UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);

    NotifySelectListChange();
}

void CPlayBoardView::OnUpdateActTakeOwnership(CCmdUI* pCmdUI)
{
    CGamDoc* pDoc = GetDocument();
    // Can't take ownership while residing on an owned board.
    if (pDoc->IsPlaying() || m_pPBoard->IsOwned())
        pCmdUI->Enable(FALSE);
    else if (pDoc->IsCurrentPlayerReferee())
        pCmdUI->Enable(FALSE);      // No owner to acquire. He's the Referee!
    else
    {
        pCmdUI->Enable(pDoc->HasPlayers() && m_selList.HasNonOwnedPieces() &&
            pDoc->GetCurrentPlayerMask() != 0);
    }
}

void CPlayBoardView::OnActReleaseOwnership()
{
    CRect rct = m_selList.GetPiecesEnclosingRect(FALSE);
    if (rct.IsRectEmpty())
        return;

    CPoint pntCenter(MidPnt(rct.left, rct.right), MidPnt(rct.top, rct.bottom));

    CGamDoc* pDoc = GetDocument();

    CWordArray tblPieces;

    m_selList.LoadTableWithOwnerStatePieceIDs(&tblPieces, m_selList.LF_OWNED);

    pDoc->AssignNewMoveGroup();

    if (pDoc->IsRecording())
    {
        // Insert a notification tip so there is some information
        // feedback during playback.
        CString strMsg;
        strMsg.LoadString(IDS_TIP_OWNER_RELEASED);
        pDoc->RecordEventMessage(strMsg, TRUE, m_pPBoard->GetSerialNumber(),
            pntCenter.x, pntCenter.y);
    }

    pDoc->SetPieceOwnershipTable(&tblPieces, 0);

    CGamDocHint hint;
    hint.m_pPBoard = m_pPBoard;
    pDoc->UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);

    NotifySelectListChange();
}

void CPlayBoardView::OnUpdateActReleaseOwnership(CCmdUI* pCmdUI)
{
    CGamDoc* pDoc = GetDocument();
    // Can't release ownership while residing on an owned board.
    if (pDoc->IsPlaying() || m_pPBoard->IsOwned())
        pCmdUI->Enable(FALSE);
    else if (pDoc->IsCurrentPlayerReferee() && m_selList.HasPieces())
        pCmdUI->Enable(TRUE);
    else
    {
        pCmdUI->Enable(pDoc->HasPlayers() && m_selList.HasOwnedPieces() &&
            pDoc->GetCurrentPlayerMask() != 0);
    }
}

void CPlayBoardView::OnActSetOwner()
{
    CGamDoc* pDoc = GetDocument();
    CRect rct = m_selList.GetPiecesEnclosingRect(FALSE);
    if (rct.IsRectEmpty())
        return;

    CSelectNewOwnerDialog dlg;
    dlg.m_pPlayerMgr = pDoc->GetPlayerManager();
    if (!pDoc->IsCurrentPlayerReferee())
        dlg.m_nPlayer = CPlayerManager::GetPlayerNumFromMask(pDoc->GetCurrentPlayerMask());

    if (dlg.DoModal() != IDOK)
        return;

    DWORD dwNewOwnerMask = CPlayerManager::GetMaskFromPlayerNum(dlg.m_nPlayer);

    CPoint pntCenter(MidPnt(rct.left, rct.right), MidPnt(rct.top, rct.bottom));

    CWordArray tblPieces;
    m_selList.LoadTableWithOwnerStatePieceIDs(&tblPieces, m_selList.LF_BOTH);

    pDoc->AssignNewMoveGroup();

    if (pDoc->IsRecording())
    {
        // Insert a notification tip so there is some information
        // feedback during playback.
        CString strMsg;
        strMsg.LoadString(IDS_TIP_OWNER_ACQUIRED);
        pDoc->RecordEventMessage(strMsg, TRUE, m_pPBoard->GetSerialNumber(),
            pntCenter.x, pntCenter.y);
    }

    pDoc->SetPieceOwnershipTable(&tblPieces, dwNewOwnerMask);

    CGamDocHint hint;
    hint.m_pPBoard = m_pPBoard;
    pDoc->UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);

    NotifySelectListChange();
}

void CPlayBoardView::OnUpdateActSetOwner(CCmdUI* pCmdUI)
{
    CGamDoc* pDoc = GetDocument();
    // Can't take ownership while residing on an owned board.
    if (pDoc->IsPlaying() || m_pPBoard->IsOwned())
        pCmdUI->Enable(FALSE);
    else if (pDoc->IsCurrentPlayerReferee() && m_selList.HasPieces())
        pCmdUI->Enable(TRUE);
    else
    {
        pCmdUI->Enable(pDoc->HasPlayers() &&
            (m_selList.HasPieces() && pDoc->GetCurrentPlayerMask() != 0));
    }
}

/////////////////////////////////////////////////////////////////////////////
// Fix MFC problems with mouse wheel handling in Win98 and WinME systems

BOOL CPlayBoardView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    return DoMouseWheelFix(nFlags, zDelta, pt);
}

BOOL CPlayBoardView::DoMouseWheelFix(UINT fFlags, short zDelta, CPoint point)
{
    static BOOL bInitOsType = TRUE;
    static BOOL bWin98;
    static BOOL bWinME;

    if (bInitOsType)
    {
        bInitOsType = FALSE;
        bWin98 = FALSE;
        bWinME = FALSE;
    }

    if (bWin98 || bWinME)
    {
        static UINT uWheelScrollLines = -1;
        if ((int)uWheelScrollLines < 0)
            ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uWheelScrollLines, 0);

        // CODE CULLED FROM MFC SOURCE...
        // if we have a vertical scroll bar, the wheel scrolls that
        // if we have _only_ a horizontal scroll bar, the wheel scrolls that
        // otherwise, don't do any work at all

        BOOL bHasHorzBar, bHasVertBar;
        CheckScrollBars(bHasHorzBar, bHasVertBar);
        if (!bHasVertBar && !bHasHorzBar)
            return FALSE;

        BOOL bResult = FALSE;
        int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
        int nDisplacement;

        if (bHasVertBar)
        {
            if (uWheelScrollLines == WHEEL_PAGESCROLL)
            {
                nDisplacement = m_pageDev.cy;
                if (zDelta > 0)
                    nDisplacement = -nDisplacement;
            }
            else
            {
                nDisplacement = nToScroll * m_lineDev.cy;
                nDisplacement = min(nDisplacement, m_pageDev.cy);
            }
            bResult = OnScrollBy(CSize(0, nDisplacement), TRUE);
        }
        else if (bHasHorzBar)
        {
            if (uWheelScrollLines == WHEEL_PAGESCROLL)
            {
                nDisplacement = m_pageDev.cx;
                if (zDelta > 0)
                    nDisplacement = -nDisplacement;
            }
            else
            {
                nDisplacement = nToScroll * m_lineDev.cx;
                nDisplacement = min(nDisplacement, m_pageDev.cx);
            }
            bResult = OnScrollBy(CSize(nDisplacement, 0), TRUE);
        }

        if (bResult)
            UpdateWindow();

        return bResult;
    }
    else
    {
        // If both scroll bars are visible, choose the direction based
        // on what client window edge the mouse is closest to.
        BOOL bHasHorzBar, bHasVertBar;
        CheckScrollBars(bHasHorzBar, bHasVertBar);
        if (!bHasVertBar && !bHasHorzBar)
            return FALSE;

        BOOL bScrollVert;               // Otherwise scroll horz.
        if (bHasHorzBar && bHasVertBar)
        {
            // If the mouse is within 5% of vertical size then a
            // horizontal scroll will be performed. Otherwise
            // a vertical scroll is done.
            ScreenToClient(&point);
            CRect rctClient;
            GetClientRect(&rctClient);
            int yEdge = min(point.y, rctClient.bottom - point.y);
            int ySize = rctClient.Height();
            bScrollVert = yEdge > ((ySize * 5) / 100);
        }
        else if (bHasVertBar)
            bScrollVert = TRUE;
        else    // Must have horizontal bar
            bScrollVert = FALSE;

        BOOL bResult = FALSE;
        int nDisplacement;

        UINT uWheelScrollLines;
        ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uWheelScrollLines, 0);

        int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);

        if (bScrollVert)
        {
            if (uWheelScrollLines == WHEEL_PAGESCROLL)
            {
                nDisplacement = m_pageDev.cy;
                if (zDelta > 0)
                    nDisplacement = -nDisplacement;
            }
            else
            {
                nDisplacement = nToScroll * m_lineDev.cy;
                nDisplacement = min(nDisplacement, m_pageDev.cy);
            }
            bResult = OnScrollBy(CSize(0, nDisplacement), TRUE);
        }
        else
        {
            if (uWheelScrollLines == WHEEL_PAGESCROLL)
            {
                nDisplacement = m_pageDev.cx;
                if (zDelta > 0)
                    nDisplacement = -nDisplacement;
            }
            else
            {
                nDisplacement = nToScroll * m_lineDev.cx;
                nDisplacement = min(nDisplacement, m_pageDev.cx);
            }
            bResult = OnScrollBy(CSize(nDisplacement, 0), TRUE);
        }

        if (bResult)
            UpdateWindow();

        return bResult;
    }
}
