// VwPbrd.cpp : implementation of the CPlayBoardView class
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
#if 0
#include    "DlgRot.h"
#endif
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

IMPLEMENT_DYNCREATE(CPlayBoardViewContainer, CView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

const int scrollZone = 16;      // From INI?

wxBEGIN_EVENT_TABLE(CPlayBoardView, CPlayBoardView::BASE)
    EVT_MENU(XRCID("ID_VIEW_FULLSCALEBRD"), OnViewFullScaleBrd)
    EVT_UPDATE_UI(XRCID("ID_VIEW_FULLSCALEBRD"), OnUpdateViewFullScaleBrd)
    EVT_MENU(XRCID("ID_VIEW_HALFSCALEBRD"), OnViewHalfScaleBrd)
    EVT_UPDATE_UI(XRCID("ID_VIEW_HALFSCALEBRD"), OnUpdateViewHalfScaleBrd)
    EVT_DRAGDROP(OnDragItem)
    EVT_ROTATEPIECE_DELTA(OnMessageRotateRelative)
    EVT_CENTERBOARDONPOINT(OnMessageCenterBoardOnPoint)
    EVT_LEFT_DOWN(OnLButtonDown)
    EVT_MOTION(OnMouseMove)
    EVT_LEFT_UP(OnLButtonUp)
    EVT_MOUSE_CAPTURE_LOST(OnMouseCaptureLost)
    EVT_TIMER(wxID_ANY, OnTimer)
    EVT_LEFT_DCLICK(OnLButtonDblClk)
    EVT_SET_CURSOR(OnSetCursor)
    EVT_KEY_DOWN(OnKeyDown)
    EVT_CHAR(OnChar)
    EVT_MENU(XRCID("ID_PTOOL_SELECT"), OnPlayTool)
    EVT_UPDATE_UI(XRCID("ID_PTOOL_SELECT"), OnUpdatePlayTool)
    EVT_MENU(XRCID("ID_ACT_STACK"), OnActStack)
    EVT_UPDATE_UI(XRCID("ID_ACT_STACK"), OnUpdateActStack)
    EVT_MENU(XRCID("ID_ACT_TOBACK"), OnActToBack)
    EVT_UPDATE_UI(XRCID("ID_ACT_TOBACK"), OnUpdateActToBack)
    EVT_MENU(XRCID("ID_ACT_TOFRONT"), OnActToFront)
    EVT_UPDATE_UI(XRCID("ID_ACT_TOFRONT"), OnUpdateActToFront)
    EVT_MENU(XRCID("ID_ACT_TURNOVER"), OnActTurnOver)
    EVT_MENU(XRCID("ID_ACT_TURNOVER_PREV"), OnActTurnOver)
    EVT_MENU(XRCID("ID_ACT_TURNOVER_RANDOM"), OnActTurnOver)
    EVT_UPDATE_UI(XRCID("ID_ACT_TURNOVER"), OnUpdateActTurnOver)
    EVT_UPDATE_UI(XRCID("ID_ACT_TURNOVER_PREV"), OnUpdateActTurnOver)
    EVT_UPDATE_UI(XRCID("ID_ACT_TURNOVER_RANDOM"), OnUpdateActTurnOver)
    EVT_MENU(XRCID("ID_PTOOL_PLOTMOVE"), OnActPlotMove)
    EVT_UPDATE_UI(XRCID("ID_PTOOL_PLOTMOVE"), OnUpdateActPlotMove)
    EVT_MENU(XRCID("ID_ACT_PLOTDONE"), OnActPlotDone)
    EVT_UPDATE_UI(XRCID("ID_ACT_PLOTDONE"), OnUpdateActPlotDone)
    EVT_MENU(XRCID("ID_ACT_PLOTDISCARD"), OnActPlotDiscard)
    EVT_UPDATE_UI(XRCID("ID_ACT_PLOTDISCARD"), OnUpdateActPlotDiscard)
    EVT_UPDATE_UI(XRCID("ID_INDICATOR_CELLNUM"), OnUpdateIndicatorCellNum)
    EVT_MENU(XRCID("ID_VIEW_SNAPGRID"), OnViewSnapGrid)
    EVT_UPDATE_UI(XRCID("ID_VIEW_SNAPGRID"), OnUpdateViewSnapGrid)
    EVT_MENU(XRCID("ID_EDIT_SELALLMARKERS"), OnEditSelAllMarkers)
    EVT_UPDATE_UI(XRCID("ID_EDIT_SELALLMARKERS"), OnUpdateEditSelAllMarkers)
#if 0
    ON_COMMAND(ID_ACT_ROTATE, OnActRotate)
    ON_UPDATE_COMMAND_UI(ID_ACT_ROTATE, OnUpdateActRotate)
#endif
    EVT_MENU(XRCID("ID_VIEW_TOGGLESCALE"), OnViewToggleScale)
    EVT_UPDATE_UI(XRCID("ID_VIEW_TOGGLESCALE"), OnUpdateViewToggleScale)
    EVT_MENU(XRCID("ID_VIEW_PIECES"), OnViewPieces)
    EVT_UPDATE_UI(XRCID("ID_VIEW_PIECES"), OnUpdateViewPieces)
    EVT_MENU(wxID_COPY, OnEditCopy)
    EVT_UPDATE_UI(wxID_COPY, OnUpdateEnable)
    EVT_MENU(XRCID("ID_EDIT_BRD2FILE"), OnEditBoardToFile)
    EVT_UPDATE_UI(XRCID("ID_EDIT_BRD2FILE"), OnUpdateEnable)
    EVT_MENU(XRCID("ID_EDIT_BRDPROP"), OnEditBoardProperties)
    EVT_UPDATE_UI(XRCID("ID_EDIT_BRDPROP"), OnUpdateEnable)
    EVT_MENU(XRCID("ID_ACT_ROTATEREL"), OnActRotateRelative)
    EVT_UPDATE_UI(XRCID("ID_ACT_ROTATEREL"), OnUpdateActRotateRelative)
    EVT_MENU(wxID_CLEAR, OnEditClear)
    EVT_UPDATE_UI(wxID_CLEAR, OnUpdateEditClear)
    EVT_CONTEXT_MENU(OnContextMenu)
    EVT_MENU(XRCID("ID_VIEW_DRAW_IND_ON_TOP"), OnViewDrawIndOnTop)
    EVT_UPDATE_UI(XRCID("ID_VIEW_DRAW_IND_ON_TOP"), OnUpdateViewDrawIndOnTop)
    EVT_MENU(XRCID("ID_EDIT_ELEMENT_TEXT"), OnEditElementText)
    EVT_UPDATE_UI(XRCID("ID_EDIT_ELEMENT_TEXT"), OnUpdateEditElementText)
    EVT_MENU(XRCID("ID_ACT_LOCKOBJECT"), OnActLockObject)
    EVT_UPDATE_UI(XRCID("ID_ACT_LOCKOBJECT"), OnUpdateActLockObject)
    EVT_MENU(XRCID("ID_ACT_LOCK_SUSPEND"), OnActLockSuspend)
    EVT_UPDATE_UI(XRCID("ID_ACT_LOCK_SUSPEND"), OnUpdateActLockSuspend)
    EVT_MENU(XRCID("ID_ACT_SHUFFLE_SELECTED"), OnActShuffleSelectedObjects)
    EVT_UPDATE_UI(XRCID("ID_ACT_SHUFFLE_SELECTED"), OnUpdateActShuffleSelectedObjects)
    EVT_MENU(XRCID("ID_ACT_AUTOSTACK_DECK"), OnActAutostackDeck)
    EVT_UPDATE_UI(XRCID("ID_ACT_AUTOSTACK_DECK"), OnUpdateActAutostackDeck)
    EVT_MENU(XRCID("ID_ACT_TAKE_OWNERSHIP"), OnActTakeOwnership)
    EVT_UPDATE_UI(XRCID("ID_ACT_TAKE_OWNERSHIP"), OnUpdateActTakeOwnership)
    EVT_MENU(XRCID("ID_ACT_RELEASE_OWNERSHIP"), OnActReleaseOwnership)
    EVT_UPDATE_UI(XRCID("ID_ACT_RELEASE_OWNERSHIP"), OnUpdateActReleaseOwnership)
    EVT_MENU(XRCID("ID_ACT_SET_OWNER"), OnActSetOwner)
    EVT_UPDATE_UI(XRCID("ID_ACT_SET_OWNER"), OnUpdateActSetOwner)
    EVT_MENU(XRCID("ID_VIEW_SMALLSCALEBRD"), OnViewSmallScaleBoard)
    EVT_UPDATE_UI(XRCID("ID_VIEW_SMALLSCALEBRD"), OnUpdateViewSmallScaleBoard)
    EVT_MENU(XRCID("ID_PTOOL_LINE"), OnPlayTool)
    EVT_MENU(XRCID("ID_PTOOL_TEXTBOX"), OnPlayTool)
    EVT_UPDATE_UI(XRCID("ID_PTOOL_LINE"), OnUpdatePlayTool)
    EVT_UPDATE_UI(XRCID("ID_PTOOL_TEXTBOX"), OnUpdatePlayTool)
#if 0
    ON_WM_MOUSEWHEEL()
#endif
    EVT_MENU(XRCID("ID_VIEW_BOARD_ROTATE180"), OnViewBoardRotate180)
    EVT_UPDATE_UI(XRCID("ID_VIEW_BOARD_ROTATE180"), OnUpdateViewBoardRotate180)
    EVT_MENU(XRCID("ID_ACT_ROTATEGROUP"), OnActRotateGroupRelative)
    EVT_UPDATE_UI(XRCID("ID_ACT_ROTATEGROUP"), OnUpdateActRotateGroupRelative)
    /* ID_ACT_ROTATE_0:  can't use wx event tables with range
        because XRCID doesn't allow controlling values, so use
        wxEvtHandler::Bind */
#if 0
    /* ID_MRKGROUP_FIRST involves modifying the menu item count,
        but the menus aren't ported to wx yet */
    ON_COMMAND_RANGE(ID_MRKGROUP_FIRST, ID_MRKGROUP_FIRST + 64, OnSelectGroupMarkers)
    ON_UPDATE_COMMAND_UI_RANGE(ID_MRKGROUP_FIRST, ID_MRKGROUP_FIRST + 64, OnUpdateSelectGroupMarkers)
#else
    /* ID_MRKGROUP_FIRST_0:  can't use wx event tables with range
        because XRCID doesn't allow controlling values, so use
        wxEvtHandler::Bind */
    /* wxUpdateEventUI doesn't support modifying menu,
        so use EVT_MENU_OPEN */
    EVT_MENU_OPEN(OnMenuOpen)
#endif
    EVT_WINSTATE(OnMessageWindowState)
    EVT_SELECT_BOARD_OBJLIST(OnMessageSelectBoardObjectList)
    EVT_SCROLLWIN_LINEDOWN(OnScrollWinLine)
    EVT_SCROLLWIN_LINEUP(OnScrollWinLine)
wxEND_EVENT_TABLE()

// helper for wxEvtHandler::Bind for ID_ACT_ROTATE_0
namespace {
    // avoid trying to use XRC before wxApp initialized
    const std::map<int /*id*/, int /*degrees*/>& GetRotateMap()
    {
        static const std::map<int /*id*/, int /*degrees*/> retval = []{
            std::map<int /*id*/, int /*degrees*/> retval;
            for (int degrees : {
                                    0,
                                    30,
                                    45,
                                    60,
                                    90,
                                    120,
                                    135,
                                    150,
                                    180,
                                    210,
                                    225,
                                    240,
                                    270,
                                    300,
                                    315,
                                    330,
                                })
            {
                wxString name = wxString::Format("ID_ACT_ROTATE_%d", degrees);
                retval[XRCID(name)] = degrees;
            }
            return retval;
        }();
        return retval;
    }
}

// helpers for wxEvtHandler::Bind for ID_MRKGROUP_FIRST_0
namespace {
    std::map<int /*index*/, int /*id*/> ids;
    std::map<int /*id*/, int /*index*/> indices;
    int MarkerIndexToXrcid(int index)
    {
        auto it = ids.lower_bound(index);
        if (it != ids.end() && it->first == index)
        {
            return it->second;
        }
        wxString name = wxString::Format("ID_MRKGROUP_FIRST_%d", index);
        int id = XRCID(name);
        ids.insert(it, std::make_pair(index, id));
        indices.insert(std::make_pair(id, index));
        return id;
    }
    int MarkerXrcidToIndex(int id)
    {
        return indices.at(id);
    }
}


BEGIN_MESSAGE_MAP(CPlayBoardViewContainer, CPlayBoardViewContainer::BASE)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
    ON_MESSAGE(WM_WINSTATE, OnMessageWindowState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardView construction/destruction

CPlayBoardView::CPlayBoardView(CPlayBoardViewContainer& p) :
    m_selList(*this),
    parent(&p),
    document(dynamic_cast<CGamDoc*>(parent->GetDocument())),
    m_pPBoard(static_cast<CPlayBoard*>(document->GetNewViewParameter()))
{
    EnableAutoScrollInside(scrollZone);
    DisableAutoScrollOutside();

    // use wxEvtHandler::Bind for ID_ACT_ROTATE_0
    for (auto pair : GetRotateMap())
    {
        Bind(wxEVT_MENU, &CPlayBoardView::OnRotatePiece, this, pair.first);
        Bind(wxEVT_UPDATE_UI, &CPlayBoardView::OnUpdateRotatePiece, this, pair.first);
    }

    m_nZoom = fullScale;
    m_nCurToolID = XRCID("ID_PTOOL_SELECT");
    m_bInDrag = FALSE;
    m_pDragSelList = NULL;
#if 0
    m_nTimerID = uintptr_t(0);
#endif

    // use sizers for scrolling
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    sizer->Add(0, 0);
    BASE::Create(*parent, 0);
    OnInitialUpdate();
}

CPlayBoardView::~CPlayBoardView()
{
}

#if 0
BOOL CPlayBoardView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CScrollView::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)GetStockObject(WHITE_BRUSH));

    return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnInitialUpdate()
{
    m_toolMsgTip.SetBalloonMode(true);
    m_toolMsgTip.SetMaxWidth(MAX_PLAYBOARD_TIP_WIDTH);
    m_toolHitTip.SetBalloonMode(true);
    m_toolHitTip.SetMaxWidth(MAX_PLAYBOARD_TIP_WIDTH);
    m_pCurTipObj = NULL;

#if 0
    m_pPBoard = (CPlayBoard*)GetDocument().GetNewViewParameter();

    if (m_pPBoard == NULL)
    {
        // See if we can get our playing board from the (0, 0) pane in the splitter.
        CSplitterWndEx* pParent = (CSplitterWndEx*)GetParent();
        CPlayBoardView* pPlay = (CPlayBoardView*)pParent->GetPane(0, 0);
        m_pPBoard = &pPlay->GetPlayBoard();
    }

    ASSERT(m_pPBoard != NULL);
#endif
    SetOurScrollSizes(m_nZoom);
}

void CPlayBoardView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CGamDocHint* ph = (CGamDocHint*)pHint;
    if (lHint == HINT_POINTINVIEW && ph->GetArgs<HINT_POINTINVIEW>().m_pPBoard == m_pPBoard)
    {
        ScrollWorkspacePointIntoView(ph->GetArgs<HINT_POINTINVIEW>().m_point);
    }
    else if (lHint == HINT_BOARDCHANGE)
    {
        // Make sure we still exist!
        if (GetDocument().GetPBoardManager().FindPBoardByRef(*m_pPBoard) == Invalid_v<size_t>)
        {
            CFrameWnd* pFrm = parent->GetParentFrame();
            wxASSERT(pFrm != NULL);
            pFrm->PostMessage(WM_CLOSE, 0, 0L);
        }
    }
    else if (lHint == HINT_UPDATEOBJECT && ph->GetArgs<HINT_UPDATEOBJECT>().m_pPBoard == m_pPBoard)
    {
        wxRect rct;
        rct = CB::Convert(ph->GetArgs<HINT_UPDATEOBJECT>().m_pDrawObj->GetEnclosingRect());   // In board coords.
        InvalidateWorkspaceRect(rct);
    }
    else if (lHint == HINT_UPDATEOBJLIST && ph->GetArgs<HINT_UPDATEOBJLIST>().m_pPBoard == m_pPBoard)
    {
        const std::vector<RefPtr<CDrawObj>>& pPtrList = CheckedDeref(ph->GetArgs<HINT_UPDATEOBJLIST>().m_pPtrList);
        for (size_t i = size_t(0); i < pPtrList.size(); ++i)
        {
            const CDrawObj& pDObj = *pPtrList[i];
            wxRect rct = CB::Convert(pDObj.GetEnclosingRect());  // In board coords.
            InvalidateWorkspaceRect(rct);
        }
    }
    else if (lHint == HINT_SELECTOBJ)
    {
        if (ph->GetArgs<HINT_SELECTOBJ>().m_pPBoard != NULL && ph->GetArgs<HINT_SELECTOBJ>().m_pPBoard != m_pPBoard)
            return;                     // Ignore in this case
        if (ph->GetArgs<HINT_SELECTOBJ>().m_pDrawObj == NULL)     // NULL means deselect all
        {
            m_selList.PurgeList(TRUE);
            return;
        }
        m_selList.AddObject(*ph->GetArgs<HINT_SELECTOBJ>().m_pDrawObj, TRUE);
        if (ph->GetArgs<HINT_SELECTOBJ>().m_pDrawObj->GetType() == CDrawObj::drawPieceObj ||
                ph->GetArgs<HINT_SELECTOBJ>().m_pDrawObj->GetType() == CDrawObj::drawMarkObj)
            NotifySelectListChange();
    }
    else if (lHint == HINT_SELECTOBJLIST && ph->GetArgs<HINT_SELECTOBJLIST>().m_pPBoard == m_pPBoard)
    {
        const std::vector<CB::not_null<CDrawObj*>>& pPtrList = CheckedDeref(ph->GetArgs<HINT_SELECTOBJLIST>().m_pPtrList);
        m_selList.PurgeList(TRUE);

        for (size_t i = size_t(0) ; i < pPtrList.size() ; ++i)
        {
            CDrawObj& pDObj = *pPtrList[i];
            m_selList.AddObject(pDObj, TRUE);
            NotifySelectListChange();
        }
    }
    else if (lHint == HINT_UPDATESELECTLIST && pSender != &*parent)
    {
        // Resync the select list to ensure that all objects still exist
        // and that the handles track objct movements for those that still
        // exist.
        std::vector<CB::not_null<CDrawObj*>> listSelectedObjs;
        for (CSelList::iterator pos = m_selList.begin() ; pos != m_selList.end() ; ++pos)
        {
            CDrawObj& pObj = *(*pos)->m_pObj;
            if (m_pPBoard->IsObjectOnBoard(pObj))
                listSelectedObjs.push_back(&pObj);
        }
        m_selList.PurgeList();          // Clear former selection indications
        // Reselect any object that are still on the board.
        SelectAllObjectsInTable(listSelectedObjs);
    }
    else if (lHint == HINT_INVALIDATERECT && ph->GetArgs<HINT_INVALIDATERECT>().m_pPBoard == m_pPBoard)
    {
        const CRect& rect = CheckedDeref(ph->GetArgs<HINT_INVALIDATERECT>().m_pRect);
        InvalidateWorkspaceRect(CB::Convert(rect), true);
    }
    else if (lHint == HINT_GAMESTATEUSED)
    {
        m_selList.PurgeList(TRUE);
        Refresh(FALSE);
        wxBusyCursor busyCursor;
        Update();
    }
    else if (lHint == HINT_ALWAYSUPDATE ||
        (lHint == HINT_UPDATEBOARD && ph->GetArgs<HINT_UPDATEBOARD>().m_pPBoard == m_pPBoard))
    {
        Refresh(FALSE);
        wxBusyCursor busyCursor;
        Update();
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
    hint.GetArgs<HINT_UPDATESELECT>().m_pPBoard = m_pPBoard.get();
    hint.GetArgs<HINT_UPDATESELECT>().m_pSelList = &m_selList;
    GetDocument().UpdateAllViews(&*parent, HINT_UPDATESELECT, &hint);
}

///////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
    if (bActivate && pActivateView != pDeactiveView)
        NotifySelectListChange();
}

///////////////////////////////////////////////////////////////////////
// This message is sent when a document is being saved.
// WPARAM = CPlayBoard*, LPARAM = const std::vector<CB::not_null<CDrawObj*>>*

void CPlayBoardView::OnMessageSelectBoardObjectList(SelectBoardObjListEvent& event)
{
    if (&event.GetBoard() != m_pPBoard)
    {
        event.Skip();
        return;
    }
    const std::vector<CB::not_null<CDrawObj*>>& pList = event.GetObjList();
    m_selList.PurgeList();                  // Deselect current set of selections
    SelectAllObjectsInList(pList);          // Select the new set
}

///////////////////////////////////////////////////////////////////////
// This message is sent when a document is being saved.
// WPARAM = CArchive*, LPARAM = 0 if save, 1 if restore

void CPlayBoardView::OnMessageWindowState(WinStateEvent& event)
{
    CArchive& ar = event.GetArchive();
    if (ar.IsStoring())
    {
        ar << static_cast<uint16_t>(m_nZoom);

        wxPoint pnt = GetViewStart();
        ar << value_preserving_cast<uint32_t>(pnt.x);
        ar << value_preserving_cast<uint32_t>(pnt.y);
        // Save the select list
        if (!m_selList.empty())
        {
            std::vector<RefPtr<CDrawObj>> tblObjPtrs;
            m_selList.LoadTableWithObjectPtrs(tblObjPtrs, CSelList::otPiecesMarks, TRUE);
            ar << value_preserving_cast<uint32_t>(tblObjPtrs.size());
            for (size_t i = size_t(0); i < tblObjPtrs.size(); i++)
            {
                CDrawObj& pObj = *tblObjPtrs.at(i);
                ar << GetDocument().GetGameElementCodeForObject(pObj);
            }
        }
        else
            ar << uint32_t(0);
    }
    else
    {
        wxPoint pnt;
        uint16_t wTmp;
        uint32_t dwTmp;

        ar >> wTmp; DoViewScaleBrd(static_cast<TileScale>(wTmp));

        ar >> dwTmp; pnt.x = value_preserving_cast<decltype(pnt.x)>(dwTmp);
        ar >> dwTmp; pnt.y = value_preserving_cast<decltype(pnt.y)>(dwTmp);

        Scroll(pnt);

        // Restore the select list.
        m_selList.PurgeList();
        uint32_t dwSelCount;
        ar >> dwSelCount;
        while (dwSelCount--)
        {
            GameElement elem;
            ar >> elem;
            CDrawObj* pObj = NULL;
            if (IsGameElementAPiece(elem))
                pObj = m_pPBoard->FindPieceID(GetPieceIDFromElement(elem));
            else
                pObj = m_pPBoard->FindObjectID(GetObjectIDFromElement(elem));
            if (pObj != NULL)
                m_selList.AddObject(*pObj, TRUE);
        }
    }
}

/* This view should support scrolling by individual pixels,
    but don't make the line-up and line-down scrolling that
    slow.  */
void CPlayBoardView::OnScrollWinLine(wxScrollWinEvent& event)
{
    int type = event.GetEventType();
    wxASSERT(type == static_cast<int>(wxEVT_SCROLLWIN_LINEDOWN) ||
                type == static_cast<int>(wxEVT_SCROLLWIN_LINEUP));

    int orient = event.GetOrientation();
    wxASSERT(orient == static_cast<int>(wxHORIZONTAL) ||
                orient == static_cast<int>(wxVERTICAL));

    int oldPos;
    int offset;
    if (orient == static_cast<int>(wxHORIZONTAL))
    {
        oldPos = GetViewStart().x;
        offset = m_xScrollPixelsPerLine;
    }
    else
    {
        oldPos = GetViewStart().y;
        offset = m_yScrollPixelsPerLine;
    }
    if (type == static_cast<int>(wxEVT_SCROLLWIN_LINEUP))
    {
        offset = -offset;
    }

    wxScrollWinEvent thumbEvent(wxEVT_SCROLLWIN_THUMBTRACK, oldPos + offset, orient);
    ProcessWindowEvent(thumbEvent);
}

/////////////////////////////////////////////////////////////////////////////

#if 0
BOOL CPlayBoardView::PreTranslateMessage(MSG* pMsg)
{
    // RelayEvent is required for CToolTipCtrl objects -
    // it passes mouse messages on to the tool tip control
    // so it can decide when to show the tool tip
    m_toolHitTip.RelayEvent(pMsg);
    m_toolMsgTip.RelayEvent(pMsg);

    return CScrollView::PreTranslateMessage(pMsg);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::SetOurScrollSizes(TileScale nZoom)
{
    if (nZoom == fullScale)
    {
        /* KLUDGE:  without this, the optimization in
                    wxDC::SetUserScale() that returns immediately if
                    the new scale equals the old scale sometimes
                    leaves the wxDC with wrong scale behavior */
        overlay = MakeOwner<wxOverlay>();
    }
    CBoard* pBoard = m_pPBoard->GetBoard();

    wxSizer& sizer = CheckedDeref(GetSizer());
    wxSizerItemList& items = sizer.GetChildren();
    wxASSERT(items.size() == 1);
    wxSizerItem& item = CheckedDeref(items[0]);
    wxASSERT(item.IsSpacer());

    item.AssignSpacer(CB::Convert(pBoard->GetSize(m_nZoom)));
    SetScrollRate(1, 1);
    sizer.FitInside(this);

    // use size of cell as line increment
    const CCellForm& cf = pBoard->GetBoardArray().GetCellForm(m_nZoom);
    wxSize cellSize = CB::Convert(cf.GetCellSize());
    switch (cf.GetCellType())
    {
        case cformHexFlat:
            (cellSize.x *= 3) /= 4;
            break;
        case cformHexPnt:
            (cellSize.y *= 3) /= 4;
            break;
        default:
            ;   // do nothing
    }
    m_xScrollPixelsPerLine = cellSize.x;
    m_yScrollPixelsPerLine = cellSize.y;
}

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardView drawing

void CPlayBoardView::OnDraw(wxDC& pDC)
{
    GetOverlay().Reset();
    CBoard*     pBoard = m_pPBoard->GetBoard();
    wxMemoryDC  dcMem;
    wxRect      oRct;
    wxRect      oRctSave;

    pDC.GetClippingBox(oRct);

    if (oRct.IsEmpty())
        return;                 // Nothing to do

    wxBitmap bmMem(
        oRct.GetWidth(), oRct.GetHeight(), pDC);
    dcMem.SelectObject(bmMem);
    if (m_pPBoard->IsBoardRotated180())
    {
        oRctSave = oRct;
        wxSize sizeBrd = CB::Convert(pBoard->GetSize(m_nZoom));
        oRct = wxRect(wxPoint(sizeBrd.x - oRct.GetLeft() - oRct.GetWidth(),
            sizeBrd.y - oRct.GetTop() - oRct.GetHeight()), oRct.GetSize());
    }
    dcMem.SetDeviceOrigin(-oRct.GetLeft(), -oRct.GetTop());

    // Draw base board image...
    pBoard->SetMaxDrawLayer();          // Make sure all layers are drawn
    pBoard->Draw(dcMem, oRct, m_nZoom,
        m_nZoom == smallScale ? m_pPBoard->m_bSmallCellBorders : m_pPBoard->m_bCellBorders);

    // Draw pieces etc.....

    wxRect rct(oRct);
    {
    DCSetupDrawListDC setupDrawListDC(*this, dcMem, rct);

    m_pPBoard->Draw(dcMem, rct, m_nZoom);

    wxASSERT(!dynamic_cast<wxPrinterDC*>(&pDC));
    if (!dynamic_cast<wxPrinterDC*>(&pDC) && GetPlayBoard().GetPiecesVisible())
        m_selList.OnDraw(dcMem);       // Handle selections.

    }

    if (m_pPBoard->IsBoardRotated180())
    {
        // Xfer to output
        dcMem.SetDeviceOrigin(0, 0);
        pDC.StretchBlit(oRctSave.GetLeft(), oRctSave.GetTop(), oRctSave.GetWidth(), oRctSave.GetHeight(),
            &dcMem, oRctSave.GetWidth() - 1, oRctSave.GetHeight() - 1,
            -oRctSave.GetWidth(), -oRctSave.GetHeight());
    }
    else
    {
        // Xfer to output
        pDC.Blit(oRct.GetLeft(), oRct.GetTop(), oRct.GetWidth(), oRct.GetHeight(),
            &dcMem, oRct.GetLeft(), oRct.GetTop());
    }
}

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnDragItem(DragDropEvent& event)
{
    if (event.GetProcessId() != wxGetProcessId())
    {
        return;
    }
    if (GetDocument().IsPlaying())
        return;                       // Drags not supported during play

    const DragInfoWx& pdi = event.GetDragInfo();

    if (pdi.GetDragType() == DRAG_PIECE)
    {
        DoDragPiece(pdi);
        return;
    }

    if (pdi.GetDragType() == DRAG_PIECELIST)
    {
        DoDragPieceList(pdi);
        return;
    }

    if (pdi.GetDragType() == DRAG_MARKER)
    {
        DoDragMarker(pdi);
        return;
    }

    if (pdi.GetDragType() == DRAG_SELECTLIST)
    {
        DoDragSelectList(event);
        return;
    }
}

void CPlayBoardView::DoDragPiece(const DragInfoWx& pdi)
{
    wxASSERT(FALSE);      //!!!NOT USED???? //TODO: WHAT'S GOING ON HERE? 20200618
#if 0
    if (pdi.GetSubInfo<DRAG_PIECE>().m_gamDoc != &GetDocument())
        return -1;               // Only pieces from our document.

    // if piece can't fit on board, reject drop
    CSize limit = m_pPBoard->GetBoard()->GetSize(fullScale);
    if (pdi.GetSubInfo<DRAG_PIECE>().m_size.cx > limit.cx ||
        pdi.GetSubInfo<DRAG_PIECE>().m_size.cy > limit.cy)
    {
        return pdi.m_phase == PhaseDrag::Over ?
                    reinterpret_cast<LRESULT>(g_res.hcrNoDropTooBig)
                :
                    -1;
    }

    if (pdi.m_phase == PhaseDrag::Exit)
        DragKillAutoScroll();
    else if (pdi.m_phase == PhaseDrag::Over)
    {
        DragCheckAutoScroll();
        return (LRESULT)(LPVOID)pdi.m_hcsrSuggest;
    }
    else if (pdi.m_phase == PhaseDrag::Drop)
    {
        CPoint pnt = pdi.m_point;
        pnt = ClientToWorkspace(pnt);
        AddPiece(pnt, pdi.GetSubInfo<DRAG_PIECE>().m_pieceID);
        DragKillAutoScroll();
    }
    return 1;
#endif
}

void CPlayBoardView::DoDragPieceList(const DragInfoWx& pdi)
{
    if (pdi.GetSubInfo<DRAG_PIECELIST>().m_gamDoc != &GetDocument())
        return;               // Only pieces from our document.

#if 0
    // if piece can't fit on board, reject drop
    CSize limit = m_pPBoard->GetBoard()->GetSize(fullScale);
    if (pdi.GetSubInfo<DRAG_PIECELIST>().m_size.cx > limit.cx ||
        pdi.GetSubInfo<DRAG_PIECELIST>().m_size.cy > limit.cy)
    {
        return pdi.m_phase == PhaseDrag::Over ?
                    reinterpret_cast<LRESULT>(g_res.hcrNoDropTooBig)
                :
                    -1;
    }

    if (pdi.m_phase == PhaseDrag::Exit)
        DragKillAutoScroll();
    else if (pdi.m_phase == PhaseDrag::Over)
    {
        DragCheckAutoScroll();
        return (LRESULT)(LPVOID)pdi.m_hcsrSuggest;
    }
    else if (pdi.m_phase == PhaseDrag::Drop)
    {
        CGamDoc& pDoc = GetDocument();
        CPoint pnt = pdi.m_point;
        const std::vector<PieceID>& pTbl = CheckedDeref(pdi.GetSubInfo<DRAG_PIECELIST>().m_pieceIDList);
        pnt = ClientToWorkspace(pnt);

        // If the snap grid is on, adjust the point.
        CSize sz = GetDocument().GetPieceTable().GetStackedSize(pTbl,
            m_pPBoard->m_xStackStagger, m_pPBoard->m_yStackStagger);
        ASSERT(sz.cx != 0 && sz.cy != 0);
        CRect rct(CPoint(pnt.x - sz.cx/2, pnt.y - sz.cy/2), sz);
        rct = AdjustRect(rct);
        pnt = GetMidRect(rct);

        m_selList.PurgeList(TRUE);          // Purge former selections
        GetDocument().AssignNewMoveGroup();
        GetDocument().PlacePieceListOnBoard(pnt, pTbl,
            m_pPBoard->m_xStackStagger, m_pPBoard->m_yStackStagger, m_pPBoard.get());

        if (!pDoc.HasPlayers() || !m_pPBoard->IsOwned() ||
            m_pPBoard->IsNonOwnerAccessAllowed() ||
            m_pPBoard->IsOwnedBy(pDoc.GetCurrentPlayerMask()))
        {
            CDrawList* pDwg = m_pPBoard->GetPieceList();
            std::vector<CB::not_null<CPieceObj*>> pceList;
            pDwg->GetObjectListFromPieceIDTable(CheckedDeref(pdi.GetSubInfo<DRAG_PIECELIST>().m_pieceIDList), pceList);
            std::vector<CB::not_null<CDrawObj*>> temp;
            CB::Move(temp, std::move(pceList));
            SelectAllObjectsInList(temp);   // Reselect pieces dropped on board
        }

        DragKillAutoScroll();
    }
    return 1;
#else
    wxASSERT(!"TODO:");
#endif
}

#define MARKER_DROP_GAP_X     8

void CPlayBoardView::DoDragMarker(const DragInfoWx& pdi)
{
    wxASSERT(pdi.GetDragType() == DRAG_MARKER);
    CGamDoc& pDoc = GetDocument();
    if (pdi.GetSubInfo<DRAG_MARKER>().m_gamDoc != &pDoc)
        return;               // Only markers from our document.

#if 0
    // if marker can't fit on board, reject drop
    CSize limit = m_pPBoard->GetBoard()->GetSize(fullScale);
    if (pdi.GetSubInfo<DRAG_MARKER>().m_size.cx > limit.cx ||
        pdi.GetSubInfo<DRAG_MARKER>().m_size.cy > limit.cy)
    {
        return pdi.m_phase == PhaseDrag::Over ?
                    reinterpret_cast<LRESULT>(g_res.hcrNoDropTooBig)
                :
                    -1;
    }

    if (pdi.m_phase == PhaseDrag::Exit)
        DragKillAutoScroll();
    else if (pdi.m_phase == PhaseDrag::Over)
    {
        DragCheckAutoScroll();
        return (LRESULT)(LPVOID)pdi.m_hcsrSuggest;
    }
    else if (pdi.m_phase == PhaseDrag::Drop)
    {
        CMarkManager& pMMgr = pDoc.GetMarkManager();
        CPoint pnt = pdi.m_point;
        MarkID mid = pdi.GetSubInfo<DRAG_MARKER>().m_markID;
        pnt = ClientToWorkspace(pnt);

        // If Control is held and the marker tray is set to
        // deliver random markers, prompt for a count of markers
        // and randomly select that many of them. The snap grid is
        // ignored for this sort of placement.
        if (GetKeyState(VK_CONTROL) < 0)
        {
            // I'm going to cheat. I happen to know that marker drops
            // can only originate at the marker palette. I can find out
            // the current marker set this way.
            size_t nMrkGrp = pDoc.m_palMark.GetSelectedMarkerGroup();
            ASSERT(nMrkGrp != Invalid_v<size_t>);
            if (nMrkGrp == Invalid_v<size_t>)
                goto NASTY_GOTO_TARGET;
            CMarkSet& pMSet = pMMgr.GetMarkSet(nMrkGrp);

            CMarkerCountDialog dlg;
            dlg.m_nMarkerCount = 2;
            if (dlg.ShowModal() != wxID_OK)
                goto NASTY_GOTO_TARGET;
            wxASSERT(dlg.m_nMarkerCount > 0);

            m_selList.PurgeList();

            std::vector<MarkID> tblMarks;
            if (pMSet.IsRandomMarkerPull())
            {
                // Pull markers randomly from the marker group.
                tblMarks.reserve(1);
                tblMarks.push_back(mid);              // Add the first one that was dropped
                pMSet.GetRandomSelection(value_preserving_cast<size_t>(dlg.m_nMarkerCount - 1), tblMarks, &pDoc);
            }
            else
            {
                // Create duplicates of the one tile.
                tblMarks.reserve(value_preserving_cast<size_t>(dlg.m_nMarkerCount));
                for (int i = 0; i < dlg.m_nMarkerCount; i++)
                    tblMarks.push_back(mid);          // Add the first one that was dropped
            }
            // First figure out the minimum size required.
            CSize sizeMin(0, 0);
            int i;
            for (i = 0; i < dlg.m_nMarkerCount; i++)
            {
                CSize size = pMMgr.GetMarkSize(tblMarks[value_preserving_cast<size_t>(i)]);
                sizeMin.cx += size.cx;
                sizeMin.cy = CB::max(sizeMin.cy, size.cy);
                if (i < dlg.m_nMarkerCount - 1)
                    sizeMin += CSize(MARKER_DROP_GAP_X, 0);
            }
            CRect rct(CPoint(pnt.x - sizeMin.cx/2, pnt.y - sizeMin.cy), sizeMin);
            rct = LimitRect(rct);                    // Make sure stays on board.

            pDoc.AssignNewMoveGroup();
            int x = rct.right;
            int y = (rct.top + rct.bottom) / 2;
            // Load the list from right ot left so the objects
            // show up in the select list in top to bottom
            // corresponding to left to right.
            for (i = dlg.m_nMarkerCount - 1; i >= 0; i--)
            {
                CSize size = pMMgr.GetMarkSize(tblMarks[value_preserving_cast<size_t>(i)]);
                CDrawObj& pObj = pDoc.CreateMarkerObject(m_pPBoard.get(), tblMarks[value_preserving_cast<size_t>(i)],
                    CPoint(x - size.cx / 2, y), ObjectID());
                x -= size.cx + MARKER_DROP_GAP_X;
                m_selList.AddObject(pObj, TRUE);
            }
            NotifySelectListChange();
            return 1;
        }

NASTY_GOTO_TARGET:
        m_selList.PurgeList();
        // If the snap grid is on, adjust the point.
        CSize sz = pMMgr.GetMarkSize(mid);
        ASSERT(sz.cx != 0 && sz.cy != 0);
        CRect rct(CPoint(pnt.x - sz.cx/2, pnt.y - sz.cy/2), sz);
        rct = AdjustRect(rct);
        pnt = GetMidRect(rct);

        pDoc.AssignNewMoveGroup();
        CDrawObj& pObj = pDoc.CreateMarkerObject(m_pPBoard.get(), mid, pnt, ObjectID());

        // If marker is set to prompt for text on drop, show the
        // dialog.
        if (pMMgr.GetMark(mid).m_flags & MarkDef::flagPromptText)
        {
            CEditElementTextDialog dlg;

            dlg.m_strText = pDoc.GetGameElementString(MakeMarkerElement(mid));

            if (dlg.ShowModal() == wxID_OK)
            {
                GameElement elem = pDoc.GetGameElementCodeForObject(pObj);
                pDoc.SetObjectText(elem, dlg.m_strText.empty() ? NULL :
                    &dlg.m_strText);
            }
        }
    }
    return 1;
#else
    wxASSERT(!"TODO:");
#endif
}

void CPlayBoardView::DoDragSelectList(DragDropEvent& event)
{
    const DragInfoWx& pdi = event.GetDragInfo();
    if (pdi.GetSubInfo<DRAG_SELECTLIST>().m_gamDoc != &GetDocument())
        return;               // Only pieces from our document.

    // allow autoscroll while this is drag destination
    switch (pdi.m_phase)
    {
        case PhaseDrag::Enter:
            EnableAutoscrollWithoutCapture();
            break;
        case PhaseDrag::Exit:
        case PhaseDrag::Drop:
            DisableAutoscrollWithoutCapture();
            break;
    }

    wxPoint pdi_m_point = ClientToWorkspace(pdi.m_point);

    CSelList *pSLst = pdi.GetSubInfo<DRAG_SELECTLIST>().m_selectList;
    wxOverlayDC pDC(GetOverlay(), this);
    OnPrepareScaledDC(pDC, TRUE);
    pDC.Clear();

    if (pdi.m_phase == PhaseDrag::Exit || pdi.m_phase == PhaseDrag::Drop ||
        pdi.m_phase == PhaseDrag::Over)
    {
        // Remove previous drag image.
#if 0
        pSLst->DrawTracker(pDC, trkMoving);
#elif 0
        GetOverlay().Reset();
#endif
    }
#if 0
    if (pdi.m_phase == PhaseDrag::Exit)
        DragKillAutoScroll();
#endif

    wxRect rctSnapRef = CB::Convert(pSLst->GetSnapReferenceRect());
    wxPoint pntSnapRefTopLeft = rctSnapRef.GetTopLeft();

    wxPoint pntMseOff = pntSnapRefTopLeft + CB::Convert(pSLst->GetMouseOffset());
    wxPoint sizeDelta = pdi_m_point - pntMseOff; // Trial delta

    rctSnapRef.Offset(sizeDelta);    // Calc trial new position
    rctSnapRef = AdjustRect(rctSnapRef);         // Force onto grid.
    sizeDelta = rctSnapRef.GetTopLeft() - pntSnapRefTopLeft;   // Calc actual offset

    if (sizeDelta.x != 0 || sizeDelta.y != 0)
    {
        // We still have to make sure the larger rect hasn't left the
        // playing area.
        wxRect rctObjs = CB::Convert(pSLst->GetEnclosingRect());
        rctObjs.Offset(sizeDelta);               // Calc trial new position
        BOOL bXOK, bYOK;
        if (!IsRectFullyOnBoard(rctObjs, &bXOK, &bYOK))
        {
            wxRect temp = rctObjs;
            temp = LimitRect(temp);
            // if enclosing rect can't fit on board, reject drop
            if (!IsRectFullyOnBoard(temp, &bXOK, &bYOK))
            {
                if (pdi.m_phase == PhaseDrag::Over)
                {
                    event.SetCursor(g_res.hcrNoDropTooBigWx);
                }
                return;
            }
            sizeDelta += temp.GetTopLeft() - rctObjs.GetTopLeft();
        }
        if (sizeDelta.x != 0 || sizeDelta.y != 0) // Check 'em again (what a pain!)
            pSLst->Offset(CB::Convert(sizeDelta));
    }

    wxRect rctObjs = CB::Convert(pSLst->GetEnclosingRect());
    wxPoint pntTopLeft = rctObjs.GetTopLeft();

    if (pdi.m_phase == PhaseDrag::Over || pdi.m_phase == PhaseDrag::Enter)
    {
        m_pDragSelList = pSLst;
        // Draw new drag image.
        pSLst->DrawTracker(pDC, trkMoving);
    }

    if (pdi.m_phase == PhaseDrag::Over)
    {
#if 0
        DragCheckAutoScroll();
#endif
        event.SetCursor(pdi.m_hcsrSuggest);
        return;
    }
    else if (pdi.m_phase == PhaseDrag::Drop)
    {
        CGamDoc& pDoc = GetDocument();

        // Whoooopppp...Whoooopppp!!! Drop occurred here....
#if 0
        DragKillAutoScroll();
#endif
        std::vector<CB::not_null<CDrawObj*>> listObjs;
        pSLst->LoadTableWithObjectPtrs(listObjs, CSelList::otAll, FALSE);
        pSLst->PurgeList(FALSE);            // Purge source list

        pDoc.AssignNewMoveGroup();
        pDoc.PlaceObjectTableOnBoard(listObjs, CB::Convert(rctObjs.GetTopLeft()), m_pPBoard.get());

        m_selList.PurgeList(TRUE);          // Purge former selections

        if (!pDoc.HasPlayers() || !m_pPBoard->IsOwned() ||
            m_pPBoard->IsNonOwnerAccessAllowed() ||
            m_pPBoard->IsOwnedBy(pDoc.GetCurrentPlayerMask()))
        {
            SelectAllObjectsInTable(listObjs);  // Reselect on this board.
        }

        CFrameWnd* pFrame = parent->GetParentFrame();
        pFrame->SetActiveView(&*parent);

        pDoc.UpdateAllViews(&*parent, HINT_UPDATESELECTLIST);

        NotifySelectListChange();
    }
    event.SetResult(true);
    return;
}

#if 0
void CPlayBoardView::DragDoAutoScroll()
{
    CPoint ptBefore(0, 0);
    ptBefore = ClientToWorkspace(ptBefore);
    CDC *pDC = NULL;

    if (m_pDragSelList != NULL)
    {
        // Remove previous drag image.
        pDC = GetDC();
        pDC->SaveDC();
        OnPrepareScaledDC(*pDC, TRUE);
        m_pDragSelList->DrawTracker(*pDC, trkMoving);
        pDC->RestoreDC(-1);
    }
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);
    BOOL bScrolled = ProcessAutoScroll(point);
    if (m_pDragSelList != NULL && bScrolled)
    {
        CPoint ptAfter(0, 0);
        ptAfter = ClientToWorkspace(ptAfter);
        CSize sizeDelta = ptAfter - ptBefore;
        if (sizeDelta.cx != 0 || sizeDelta.cy != 0)
        {
            // We still have to make sure the larger rect hasn't left the
            // playing area.
            CRect rctObjs = m_pDragSelList->GetEnclosingRect();
            rctObjs += (CPoint)sizeDelta;               // Calc trial new position
            BOOL bXOK, bYOK;
            if (!IsRectFullyOnBoard(rctObjs, &bXOK, &bYOK))
            {
                sizeDelta.cx = bXOK ? sizeDelta.cx : 0;
                sizeDelta.cy = bYOK ? sizeDelta.cy : 0;
            }
        }
        m_pDragSelList->Offset((CPoint)sizeDelta);
    }
    if (m_pDragSelList != NULL)
    {
        pDC->SaveDC();
        OnPrepareScaledDC(*pDC, TRUE);
        // Restore drag image.
        m_pDragSelList->DrawTracker(*pDC, trkMoving);
        pDC->RestoreDC(-1);
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
        if (m_nTimerID == uintptr_t(0))
            m_nTimerID = SetTimer(timerIDAutoScroll, timerAutoScroll, NULL);
    }
    else if (m_nTimerID != uintptr_t(0))
    {
        if (m_nTimerID != uintptr_t(0))
            KillTimer(m_nTimerID);
        m_nTimerID = uintptr_t(0);
    }
}

void CPlayBoardView::DragKillAutoScroll()
{
    m_bInDrag = FALSE;
    m_pDragSelList = NULL;
    if (m_nTimerID != uintptr_t(0))
        KillTimer(m_nTimerID);
    m_nTimerID = uintptr_t(0);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::AddPiece(wxPoint pnt, PieceID pid)
{
    wxASSERT(FALSE);      //!!!!NO LONGER USED?!!!!!
    GetDocument().PlacePieceOnBoard(CB::Convert(pnt), pid, m_pPBoard.get());
}

/////////////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnPrepareScaledDC(wxDC& pDC, BOOL bHonor180Flip)
{
    PrepareDC(pDC);
    PrepareScaledDC(pDC, NULL, bHonor180Flip);
}

CPlayBoardView::DCSetupDrawListDC::DCSetupDrawListDC(const CPlayBoardView& rThis, wxDC& pDC, wxRect& pRct)
{
    if (rThis.m_nZoom == fullScale)
        return;

    double xScale, yScale;
    pDC.GetUserScale(&xScale, &yScale);
    scaleChanger = CB::DCUserScaleChanger(pDC, xScale, yScale);
    logOrgChanger = CB::DCLogicalOriginChanger(pDC, pDC.GetLogicalOrigin());

    rThis.PrepareScaledDC(pDC, &pRct);
}

void CPlayBoardView::PrepareScaledDC(wxDC& pDC, wxRect* pRct, BOOL bHonor180Flip) const
{
    wxSize wsize, vsize;

    m_pPBoard->GetBoard()->GetBoardArray().
        GetBoardScaling(m_nZoom, wsize, vsize);

    if (bHonor180Flip && m_pPBoard->IsBoardRotated180())
    {
        pDC.SetLogicalOrigin(wsize.x, wsize.y);
        pDC.SetAxisOrientation(false, true);
    }
    pDC.SetUserScale(double(vsize.x)/wsize.x, double(vsize.y)/wsize.y);

    if (pRct != NULL)
        ScaleRect(*pRct, wsize, vsize);
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
const CGamDoc& CPlayBoardView::GetDocument() const // non-debug version is inline
{
    const CGamDoc& retval = *document;
    return retval;
}
#endif //_DEBUG

CFrameWnd* CPlayBoardView::GetParentFrame()
{
    return parent->GetParentFrame();
}

/////////////////////////////////////////////////////////////////////////////
// Right mouse button handler

void CPlayBoardView::OnContextMenu(wxContextMenuEvent& event)
{
    // Make sure window is active.
    GetParentFrame()->ActivateFrame();

    std::unique_ptr<wxMenuBar> bar(wxXmlResource::Get()->LoadMenuBar("IDR_MENU_PLAYER_POPUPS"));
    if (bar)
    {
        CB::string nMenuNum;
        if (GetDocument().IsPlaying())
            nMenuNum = "1=PV_PLAYMODE";
        else if (GetDocument().IsScenario())
            nMenuNum = "2=PV_SCNMODE";
        else
            nMenuNum = "0=PV_MOVEMODE";

        int index = bar->FindMenu(nMenuNum);
        wxASSERT(index != wxNOT_FOUND);
        std::unique_ptr<wxMenu> popup(bar->Remove(value_preserving_cast<size_t>(index)));

        // Make sure we clean up even if exception is tossed.
        try
        {
            PopupMenu(&*popup, ScreenToClient(event.GetPosition()));
        }
        catch (...)
        {
            wxASSERT(!"exception");
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Handlers associated with tools.

BOOL CPlayBoardView::IsBoardContentsAvailableToCurrentPlayer() const
{
    if (m_pPBoard->IsNonOwnerAccessAllowed() || !m_pPBoard->IsOwned() ||
            GetDocument().IsScenario())
        return TRUE;

    return m_pPBoard->IsOwnedBy(GetDocument().GetCurrentPlayerMask());
}

void CPlayBoardView::OnLButtonDown(wxMouseEvent& event)
{
    if (!IsBoardContentsAvailableToCurrentPlayer())
    {
        event.Skip();
        return;
    }

    PToolType eToolType = MapToolType(m_nCurToolID);
    CPlayTool& pTool = CPlayTool::GetTool(eToolType);
    // Allow pieces to be selected even during playback
    wxPoint point = ClientToWorkspace(event.GetPosition());
    pTool.OnLButtonDown(*this, event.GetModifiers(), point);
}

void CPlayBoardView::OnMouseMove(wxMouseEvent& event)
{
#if 1
    int nMods = event.GetModifiers();
    wxPoint point = event.GetPosition();
    if (!IsBoardContentsAvailableToCurrentPlayer())
    {
        event.Skip();
        return;
    }

    DoToolTipHitProcessing(point);

    if (!GetDocument().IsPlaying())
    {
        PToolType eToolType = MapToolType(m_nCurToolID);
        CPlayTool& pTool = CPlayTool::GetTool(eToolType);
        point = ClientToWorkspace(point);
        pTool.OnMouseMove(*this, nMods, point);
    }
    else
        event.Skip();
#else
    wxPoint client = event.GetPosition();
    wxPoint workspace = ClientToWorkspace(client);
    CPP20_TRACE("client {}, wkspc {}\n", client, workspace);
    wxOverlayDC dc(GetOverlay(), this);
    OnPrepareScaledDC(dc, true);
    dc.Clear();
    wxDCPenChanger setPen(dc, *wxBLACK_PEN);
    wxDCBrushChanger setBrush(dc, *wxBLACK_BRUSH);
    CB::DrawEllipse(dc, wxRect(wxPoint(workspace.x - 20, workspace.y - 20),
                                wxPoint(workspace.x + 20, workspace.y + 20)));
    event.Skip();
#endif
}

void CPlayBoardView::OnLButtonUp(wxMouseEvent& event)
{
    int nMods = event.GetModifiers();
    wxPoint point = event.GetPosition();
    if (!IsBoardContentsAvailableToCurrentPlayer())
    {
        event.Skip();
        return;
    }

    PToolType eToolType = MapToolType(m_nCurToolID);
    CPlayTool& pTool = CPlayTool::GetTool(eToolType);
    // Allow pieces to be selected even during playback
    point = ClientToWorkspace(point);
    bool rc = pTool.OnLButtonUp(*this, nMods, point);
    wxASSERT(rc || pTool.m_eToolType == ptypeSelect);
    if (!rc && pTool.m_eToolType == ptypeSelect)
    {
        // failed drop messed up selection list, so rebuild it
        std::vector<CB::not_null<CDrawObj*>> listObjs;
        m_selList.LoadTableWithObjectPtrs(listObjs, CSelList::otAll, FALSE);
        m_selList.PurgeList(TRUE);          // Purge former selections
        SelectAllObjectsInTable(listObjs);  // Reselect on this board.
    }
}

void CPlayBoardView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    PToolType eToolType = MapToolType(m_nCurToolID);
    CPlayTool& pTool = CPlayTool::GetTool(eToolType);
    pTool.OnMouseCaptureLost(*this);
}

void CPlayBoardView::OnLButtonDblClk(wxMouseEvent& event)
{
    int nMods = event.GetModifiers();
    wxPoint point = event.GetPosition();
    if (!IsBoardContentsAvailableToCurrentPlayer())
    {
        event.Skip();
        return;
    }

    if (!GetDocument().IsPlaying())
    {
        PToolType eToolType = MapToolType(m_nCurToolID);
        CPlayTool& pTool = CPlayTool::GetTool(eToolType);
        point = ClientToWorkspace(point);
        pTool.OnLButtonDblClk(*this, nMods, point);
    }
    else
        event.Skip();
}

void CPlayBoardView::OnTimer(wxTimerEvent& event)
{
#if 0
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
#endif
        if (!GetDocument().IsPlaying())
        {
            PToolType eToolType = MapToolType(m_nCurToolID);
            CPlayTool& pTool = CPlayTool::GetTool(eToolType);
            pTool.OnTimer(*this, event.GetId());
        }
        else
        {
            event.Skip();
        }
#if 0
    }
#endif
}

void CPlayBoardView::OnSetCursor(wxSetCursorEvent& event)
{
    if (GetDocument().IsPlaying())
    {
        event.Skip();
        return;
    }

    PToolType eToolType = MapToolType(m_nCurToolID);
    if (event.GetEventObject() == this && eToolType != ptypeUnknown)
    {
        CPlayTool& pTool = CPlayTool::GetTool(eToolType);
        wxPoint point(event.GetX(), event.GetY());
        if (GetClientRect().Contains(point))
        {
            point = ClientToWorkspace(point);
            wxCursor rc = pTool.OnSetCursor(*this, point);
            if (rc.IsOk())
            {
                event.SetCursor(rc);
                return;
            }
        }
    }
    if (GetDocument().IsRecordingCompoundMove())
    {
        event.SetCursor(g_res.hcrCompMoveActive);
        return;
    }
    else
    {
        event.Skip();
        return;
    }
}

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnChar(wxKeyEvent& event)
{
    int nChar = event.GetKeyCode();
    CGamDoc& pDoc = GetDocument();
    if (!pDoc.IsPlaying())
    {
        if (nChar == static_cast<int>(WXK_ESCAPE))
        {
            if (pDoc.IsRecordingCompoundMove())
                pDoc.RecordCompoundMoveDiscard();
            else if (m_pPBoard->GetPlotMoveMode())
                OnActPlotDiscard();
            else
                m_selList.PurgeList(TRUE);
        }
        else if (nChar == static_cast<int>(WXK_RETURN))
        {
            if (pDoc.IsRecordingCompoundMove())
                pDoc.RecordCompoundMoveEnd();
            else if (m_pPBoard->GetPlotMoveMode())
                OnActPlotDone();
        }
    }
    event.Skip();
}

void CPlayBoardView::OnKeyDown(wxKeyEvent& event)
{
    int nChar = event.GetKeyCode();
    if (!IsBoardContentsAvailableToCurrentPlayer())
        return;

    if (GetDocument().IsPlaying())
        return;

    if (nChar == WXK_DELETE ||
        nChar == WXK_NUMPAD_DELETE)
        OnEditClear();

    /* so other keys get to EVT_CHAR
        (see https://docs.wxwidgets.org/stable/classwx_key_event.html) */
    else
    {
        event.Skip();
    }
}

//////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnEditClear()
{
    if (!m_selList.HasMarkers())
        return;                                  // Nothing to do

    wxASSERT(!GetDocument().IsPlaying() && m_selList.HasMarkers());
    if (m_pPBoard->GetPlotMoveMode())
        OnActPlotDiscard();

    if (AfxMessageBox(IDS_WARN_DELETEMARKERS, MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;

    std::vector<RefPtr<CDrawObj>> listPtr;
    m_selList.LoadTableWithObjectPtrs(listPtr, CSelList::otAll, FALSE);
    m_selList.PurgeList(TRUE);                  // Purge selections
    GetDocument().AssignNewMoveGroup();
    GetDocument().DeleteObjectsInTable(listPtr);
}

void CPlayBoardView::OnUpdateEditClear(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!GetDocument().IsPlaying() && m_selList.HasMarkers());
}

//////////////////////////////////////////////////////////////////////

PToolType CPlayBoardView::MapToolType(int nToolResID) const
{
    // wx doesn't guarantee XRCID() value order
    static const std::unordered_map<int, PToolType > map {
        { XRCID("ID_PTOOL_SELECT"), ptypeSelect },
        { XRCID("ID_PTOOL_LINE"), ptypeLine },
        { XRCID("ID_PTOOL_TEXTBOX"), ptypeTextBox },
        { XRCID("ID_PTOOL_PLOTMOVE"), ptypeMPlot },
    };
    return map.at(nToolResID);
}

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardView message handlers

void CPlayBoardView::OnUpdateIndicatorCellNum(wxUpdateUIEvent& pCmdUI)
{
    CBoardArray& pba = m_pPBoard->GetBoard()->GetBoardArray();
    if (pba.GetCellNumTracking())
    {
        wxPoint point = wxGetMouseState().GetPosition();
        point = ScreenToClient(point);
        wxRect rct = GetClientRect();
        if (rct.Contains(point))
        {
            point = CalcUnscrolledPosition(point);
            CB::string str = m_pPBoard->GetCellNumberStr(CB::Convert(point), m_nZoom);
            pCmdUI.Enable(true);
            pCmdUI.SetText(str);
        }
    }
}

void CPlayBoardView::DoViewScaleBrd(TileScale nZoom)
{
    CBoard* pBoard = m_pPBoard->GetBoard();
    wxASSERT(pBoard != NULL);

    wxPoint pntMid;
    if (m_selList.IsAnySelects())
    {
        CRect rctSelection = m_selList.GetEnclosingRect();
        pntMid = CB::Convert(rctSelection.CenterPoint());
    }
    else
    {
        wxRect rctClient = GetClientRect();
        pntMid = GetMidRect(rctClient);
        pntMid = ClientToWorkspace(pntMid);
    }

    m_nZoom = nZoom;
    SetOurScrollSizes(m_nZoom);
    wxBusyCursor busyCursor;
    Refresh(FALSE);

    CenterViewOnWorkspacePoint(pntMid);

    Update();
}

void CPlayBoardView::OnViewFullScaleBrd(wxCommandEvent& /*event*/)
{
    DoViewScaleBrd(fullScale);
}

void CPlayBoardView::OnUpdateViewFullScaleBrd(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(m_nZoom == fullScale);
}

void CPlayBoardView::OnViewHalfScaleBrd(wxCommandEvent& /*event*/)
{
    DoViewScaleBrd(halfScale);
}

void CPlayBoardView::OnUpdateViewHalfScaleBrd(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(m_nZoom == halfScale);
}

void CPlayBoardView::OnViewSmallScaleBoard(wxCommandEvent& /*event*/)
{
    DoViewScaleBrd(smallScale);
}

void CPlayBoardView::OnUpdateViewSmallScaleBoard(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(m_nZoom == smallScale);
}

void CPlayBoardView::OnViewToggleScale(wxCommandEvent& /*event*/)
{
    if (wxGetKeyState(WXK_CONTROL))
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

void CPlayBoardView::OnUpdateViewToggleScale(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(TRUE);
}

void CPlayBoardView::OnViewBoardRotate180(wxCommandEvent& /*event*/)
{
   m_pPBoard->SetRotateBoard180(!m_pPBoard->IsBoardRotated180());
   if (m_nZoom == fullScale)
   {
       /* KLUDGE:  without this, the wxDC soemtimes has the
                    wrong axis behavior */
       overlay = MakeOwner<wxOverlay>();
   }
   CGamDocHint hint;
   hint.GetArgs<HINT_UPDATEBOARD>().m_pPBoard = m_pPBoard.get();
   GetDocument().UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);
}

void CPlayBoardView::OnUpdateViewBoardRotate180(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(m_pPBoard->IsBoardRotated180());
}

void CPlayBoardView::OnPlayTool(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id !=XRCID("ID_PTOOL_SELECT"))
        m_selList.PurgeList(TRUE);
    if (id != m_nCurToolID)
    {
        m_nCurToolID = id;
    }
}

void CPlayBoardView::OnUpdatePlayTool(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying())
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(pCmdUI.GetId() == m_nCurToolID);
    pCmdUI.Check(pCmdUI.GetId() == m_nCurToolID);
}

void CPlayBoardView::OnActStack(wxCommandEvent& /*event*/)
{
    DoAutostackOfSelectedObjects(m_pPBoard->m_xStackStagger,
        m_pPBoard->m_yStackStagger);
}

void CPlayBoardView::DoAutostackOfSelectedObjects(int xStagger, int yStagger)
{
    wxRect rct = CB::Convert(m_selList.GetPiecesEnclosingRect());
    if (rct.IsEmpty())
        return;

    wxPoint pntCenter(MidPnt(rct.GetLeft(), rct.GetRight()), MidPnt(rct.GetTop(), rct.GetBottom()));

    std::vector<CB::not_null<CDrawObj*>> tblObjs;
    m_selList.LoadTableWithObjectPtrs(tblObjs, CSelList::otPiecesMarks, TRUE);

    m_selList.PurgeList(TRUE);              // Purge former selections

    GetDocument().AssignNewMoveGroup();
    GetDocument().PlaceObjectTableOnBoard(CB::Convert(pntCenter), tblObjs,
        xStagger, yStagger, m_pPBoard.get());

    // Reselect the pieces.
    SelectAllObjectsInTable(tblObjs);      // Reselect objects
}

void CPlayBoardView::OnUpdateActStack(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying())
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(m_selList.IsMultipleSelects());
}

void CPlayBoardView::OnActAutostackDeck(wxCommandEvent& /*event*/)
{
    DoAutostackOfSelectedObjects(0, 0);
}

void CPlayBoardView::OnUpdateActAutostackDeck(wxUpdateUIEvent& pCmdUI)
{
    OnUpdateActStack(pCmdUI);
}

void CPlayBoardView::OnActShuffleSelectedObjects(wxCommandEvent& /*event*/)
{
    CRect rct = m_selList.GetPiecesEnclosingRect();
    if (rct.IsRectEmpty())
        return;

    CGamDoc& pDoc = GetDocument();          // Shorthand pointer

    CPoint pntCenter(MidPnt(rct.left, rct.right), MidPnt(rct.top, rct.bottom));

    std::vector<RefPtr<CDrawObj>> tblObjs;
    m_selList.LoadTableWithObjectPtrs(tblObjs, CSelList::otPiecesMarks, TRUE);

    m_selList.PurgeList(TRUE);              // Purge former selections

    // Generate a shuffled index vector for the number of selected items
    uint32_t nRandSeed = pDoc.GetRandomNumberSeed();
    size_t nNumIndices = tblObjs.size();
    std::vector<size_t> pnIndices = AllocateAndCalcRandomIndexVector(nNumIndices,
        nNumIndices, nRandSeed, &nRandSeed);
    pDoc.SetRandomNumberSeed(nRandSeed);

    // Create a shuffled table of objects...
    std::vector<CB::not_null<CDrawObj*>> tblRandObjs;
    tblRandObjs.reserve(tblObjs.size());
    for (size_t i = size_t(0); i < tblObjs.size(); i++)
        tblRandObjs.emplace_back(&*tblObjs[value_preserving_cast<size_t>(pnIndices[i])]);

    pDoc.AssignNewMoveGroup();

    if (pDoc.IsRecording())
    {
        // Insert a notification tip so there is some information
        // feedback during playback.
        CB::string strMsg = CB::string::Format(IDS_TIP_OBJS_SHUFFLED, tblRandObjs.size());
        pDoc.RecordEventMessage(strMsg, m_pPBoard->GetSerialNumber(),
            value_preserving_cast<int>(pntCenter.x), value_preserving_cast<int>(pntCenter.y));
    }

    pDoc.PlaceObjectTableOnBoard(tblRandObjs, m_pPBoard.get());

    // Reselect the pieces.
    SelectAllObjectsInTable(tblRandObjs);  // Reselect objects
}

void CPlayBoardView::OnUpdateActShuffleSelectedObjects(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying())
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(m_selList.IsMultipleSelects());
}

void CPlayBoardView::OnActToFront(wxCommandEvent& /*event*/)
{
    CRect rct = m_selList.GetEnclosingRect();
    if (rct.IsRectEmpty())
        return;

    std::vector<CB::not_null<CDrawObj*>> listObjs;
    m_selList.LoadTableWithObjectPtrs(listObjs, CSelList::otAll, FALSE);

    m_selList.PurgeList(TRUE);          // Purge former selections

    GetDocument().AssignNewMoveGroup();
    GetDocument().PlaceObjectTableOnBoard(listObjs, rct.TopLeft(),
        m_pPBoard.get(), placeTop);

    SelectAllObjectsInTable(listObjs);  // Reselect pieces
}

void CPlayBoardView::OnUpdateActToFront(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying())
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(m_selList.IsAnySelects());
}

void CPlayBoardView::OnActToBack(wxCommandEvent& /*event*/)
{
    CRect rct = m_selList.GetEnclosingRect();
    if (rct.IsRectEmpty())
        return;

    std::vector<CB::not_null<CDrawObj*>> listObjs;
    m_selList.LoadTableWithObjectPtrs(listObjs, CSelList::otAll, FALSE);

    m_selList.PurgeList(TRUE);          // Purge former selections

    GetDocument().AssignNewMoveGroup();
    GetDocument().PlaceObjectTableOnBoard(listObjs, rct.TopLeft(),
        m_pPBoard.get(), placeBack);

    SelectAllObjectsInTable(listObjs);  // Reselect pieces
}

void CPlayBoardView::OnUpdateActToBack(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying())
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(m_selList.IsAnySelects());
}

void CPlayBoardView::OnActTurnOver(wxCommandEvent& event)
{
    CPieceTable::Flip flip;
    if (event.GetId() == XRCID("ID_ACT_TURNOVER"))
    {
        flip = CPieceTable::fNext;
    }
    else if (event.GetId() == XRCID("ID_ACT_TURNOVER_PREV"))
    {
        flip = CPieceTable::fPrev;
    }
    else if (event.GetId() == XRCID("ID_ACT_TURNOVER_RANDOM"))
    {
        flip = CPieceTable::fRandom;
    }
    else
    {
        throw std::invalid_argument("unknown command id");
    }
    std::vector<CB::not_null<CDrawObj*>> listObjs;
    m_selList.LoadTableWithObjectPtrs(listObjs, CSelList::otAll, FALSE);

    wxPoint pntCenter;
    if (flip == CPieceTable::fRandom)
    {
        wxRect rct = CB::Convert(m_selList.GetPiecesEnclosingRect(FALSE));
        wxASSERT(!rct.IsEmpty());
        pntCenter = GetMidRect(rct);
    }

    m_selList.PurgeList(TRUE);          // Purge former selections

    CGamDoc& pDoc = GetDocument();
    pDoc.AssignNewMoveGroup();

    if (pDoc.IsRecording() && flip == CPieceTable::fRandom)
    {
        // Insert a notification tip so there is some information
        // feedback during playback.
        CB::string strMsg = CB::string::LoadString(IDS_TIP_FLIP_RANDOM);
        pDoc.RecordEventMessage(strMsg, m_pPBoard->GetSerialNumber(),
            pntCenter.x, pntCenter.y);
    }

    pDoc.InvertPlayingPieceTableOnBoard(listObjs, *m_pPBoard, flip);

    SelectAllObjectsInTable(listObjs);  // Reselect pieces
}

void CPlayBoardView::OnUpdateActTurnOver(wxUpdateUIEvent& pCmdUI)
{
    bool bEnabled = false;
    CGamDoc& pDoc = GetDocument();
    if (pDoc.IsPlaying() || !pDoc.IsScenario() &&
        m_selList.HasOwnedPiecesNotMatching(pDoc.GetCurrentPlayerMask()))
        bEnabled = false;
    else
        bEnabled = m_selList.HasFlippablePieces();
    pCmdUI.Enable(bEnabled);
#if 0
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // Need to handle menu that the submenu is connected to.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (bEnabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
#endif
}

void CPlayBoardView::OnActPlotMove(wxCommandEvent& /*event*/)
{
    // Do this call so we don't record the plot list in the
    // restoration record.
    GetDocument().CreateRecordListIfRequired();
    // Ok...finish plot setup
    m_pPBoard->SetPlotMoveMode(TRUE);
    m_pPBoard->InitPlotStartPoint();
    m_nCurToolID = XRCID("ID_PTOOL_PLOTMOVE");
}

void CPlayBoardView::OnUpdateActPlotMove(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying() || (!m_selList.HasPieces() &&
        !m_selList.HasMarkers()) || GetDocument().IsScenario())
    {
#if 0
        if (pCmdUI->m_pSubMenu != NULL)
        {
            pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
                MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        }
        else
            pCmdUI->Enable(FALSE);
#else
        pCmdUI.Enable(FALSE);
#endif
        return;
    }
#if 0
    else if (pCmdUI->m_pSubMenu != NULL)
    {
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | MF_ENABLED);
    }
#endif

    pCmdUI.Enable(!m_pPBoard->GetPlotMoveMode());
    pCmdUI.Check(m_pPBoard->GetPlotMoveMode());
}

void CPlayBoardView::OnActPlotDone()
{
    if (m_pPBoard->GetPrevPlotPoint() != CPoint(-1, -1))
    {
        wxRect rct = CB::Convert(m_selList.GetPiecesEnclosingRect());
        wxPoint ptPrev = CB::Convert(m_pPBoard->GetPrevPlotPoint());
        ptPrev -= wxSize(rct.GetWidth() / 2, rct.GetHeight() / 2);
        ptPrev = AdjustPoint(ptPrev);

        std::vector<CB::not_null<CDrawObj*>> listObjs;
        m_selList.LoadTableWithObjectPtrs(listObjs, CSelList::otAll, FALSE);
        m_selList.PurgeList(TRUE);

        GetDocument().AssignNewMoveGroup();

        // Note that PlaceObjectListOnBoard() automatically detects the
        // plotted move case and records that fact.
        GetDocument().PlaceObjectTableOnBoard(listObjs, CB::Convert(ptPrev), m_pPBoard.get());
        m_selList.PurgeList(TRUE);          // Purge former selections
        SelectAllObjectsInTable(listObjs);  // Select on this board.
    }
    m_pPBoard->SetPlotMoveMode(FALSE);
    GetDocument().UpdateAllBoardIndicators(*m_pPBoard);
    m_pPBoard->FlushAllIndicators();
    m_nCurToolID = XRCID("ID_PTOOL_SELECT");
}

void CPlayBoardView::OnUpdateActPlotDone(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying() || GetDocument().IsScenario())
    {
        pCmdUI.Enable(FALSE);
        return;
    }
    pCmdUI.Enable(m_pPBoard->GetPlotMoveMode());
}

void CPlayBoardView::OnActPlotDiscard()
{
    m_pPBoard->SetPlotMoveMode(FALSE);
    GetDocument().UpdateAllBoardIndicators(*m_pPBoard);
    m_pPBoard->FlushAllIndicators();
    m_nCurToolID = XRCID("ID_PTOOL_SELECT");
}

void CPlayBoardView::OnUpdateActPlotDiscard(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying() || GetDocument().IsScenario())
    {
        pCmdUI.Enable(FALSE);
        return;
    }
    pCmdUI.Enable(m_pPBoard->GetPlotMoveMode());
}

void CPlayBoardView::OnViewSnapGrid(wxCommandEvent& /*event*/)
{
    m_pPBoard->m_bGridSnap = !m_pPBoard->m_bGridSnap;
}

void CPlayBoardView::OnUpdateViewSnapGrid(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(!GetDocument().IsPlaying());
    pCmdUI.Check(m_pPBoard->m_bGridSnap);
}

void CPlayBoardView::OnEditSelAllMarkers(wxCommandEvent& /*event*/)
{
    SelectAllMarkers();
}

void CPlayBoardView::OnUpdateEditSelAllMarkers(wxUpdateUIEvent& pCmdUI)
{
    CDrawList* pDwg = m_pPBoard->GetPieceList();
    wxASSERT(pDwg);
    pCmdUI.Enable(!GetDocument().IsPlaying() && pDwg->HasMarker());
}

#if 0
void CPlayBoardView::OnActRotate()      // ** TEST CODE ** //
{
    std::vector<PieceID> tbl;
    CGamDoc& pDoc = GetDocument();
    m_selList.LoadTableWithPieceIDs(tbl);
    TileID tid = pDoc.GetPieceTable().GetActiveTileID(tbl.front());
    CTile tile = pDoc.GetTileManager().GetTile(tid);
    OwnerPtr<CBitmap> bmap = tile.CreateBitmapOfTile();
    wxASSERT(!"dead code");
#if 0
    CRotateDialog dlg;
    dlg.m_pBMap = &*bmap;
    dlg.m_crTrans = pDoc->GetTileManager().GetTransparentColor();
    dlg.DoModal();
#endif
}

void CPlayBoardView::OnUpdateActRotate(CCmdUI* pCmdUI) // ** TEST CODE ** //
{
    CGamDoc& pDoc = GetDocument();
    if (pDoc.IsPlaying() || m_pPBoard->GetPlotMoveMode() || !pDoc.IsScenario() &&
            m_selList.HasOwnedPiecesNotMatching(pDoc.GetCurrentPlayerMask()))
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(m_selList.HasPieces());
}
#endif

///////////////////////////////////////////////////////////////////////
// Handle rotation requests. The ID's for tile rotations
// start at: ID_ACT_ROTATE_0 with the ID number being stepped at
// five degree increments. For example: if ID_ACT_ROTATE_0 is 42000, then
// ID_ACT_ROTATE_90 must be 42009. This makes the angle easy to compute.

void CPlayBoardView::OnRotatePiece(wxCommandEvent& event)
{
    int nFacingDegCW = GetRotateMap().at(event.GetId());
    std::vector<CB::not_null<CDrawObj*>> listObjs;
    m_selList.LoadTableWithObjectPtrs(listObjs, CSelList::otAll, FALSE);

    m_selList.PurgeList(TRUE);          // Purge former selections

    GetDocument().AssignNewMoveGroup();
    GetDocument().ChangePlayingPieceFacingTableOnBoard(listObjs, m_pPBoard.get(),
        value_preserving_cast<uint16_t>(nFacingDegCW));

    SelectAllObjectsInTable(listObjs);  // Reselect pieces
}

void CPlayBoardView::OnUpdateRotatePiece(wxUpdateUIEvent& pCmdUI)
{
    CGamDoc& pDoc = GetDocument();
    BOOL bEnabled = (m_selList.HasPieces() || m_selList.HasMarkers()) && !pDoc.IsPlaying();
    if (bEnabled && !pDoc.IsScenario() &&
        m_selList.HasOwnedPiecesNotMatching(pDoc.GetCurrentPlayerMask()))
    {
        bEnabled = FALSE;
    }

#if 0
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // Need to handle menu that the submenu is connected to.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (bEnabled ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
#endif
    pCmdUI.Enable(bEnabled);
}

///////////////////////////////////////////////////////////////////////
// This method handles messages typically sent by the tiny map view.
// WPARAM = POINT*

void CPlayBoardView::OnMessageCenterBoardOnPoint(CenterBoardOnPointEvent& event)
{
    CenterViewOnWorkspacePoint(event.GetPoint());
}

///////////////////////////////////////////////////////////////////////
// This method handles notifications of changes of relative piece
// rotation sent from the CRotatePieceDialog dialog.

void CPlayBoardView::OnMessageRotateRelative(RotatePieceDeltaEvent& event)
{
    CGamDoc& pDoc = GetDocument();
    int nRelativeRotation = event.GetDelta();
    wxASSERT(!m_tblCurPieces.empty());
    wxASSERT(m_tblCurAngles.size() == m_tblCurPieces.size());

    for (size_t i = size_t(0) ; i < m_tblCurPieces.size() ; ++i)
    {
        int nAngle = m_tblCurAngles[i] + nRelativeRotation;
        nAngle += nAngle < 0 ? 360 : 0;     // Shift to positive values
        nAngle %= 360;                      // Keep to within a circle

        CDrawObj& pDObj = *m_tblCurPieces[i];
        if (pDObj.GetType() == CDrawObj::drawPieceObj)
            pDoc.ChangePlayingPieceFacingOnBoard(static_cast<CPieceObj&>(pDObj), m_pPBoard.get(), value_preserving_cast<uint16_t>(nAngle));
        else if (pDObj.GetType() == CDrawObj::drawMarkObj)
            pDoc.ChangeMarkerFacingOnBoard(static_cast<CMarkObj&>(pDObj), m_pPBoard.get(), value_preserving_cast<uint16_t>(nAngle));
        if (m_bWheelRotation &&
            (pDObj.GetType() == CDrawObj::drawPieceObj || pDObj.GetType() == CDrawObj::drawMarkObj))
        {
            // Calculate new rotated mid-point for object.
            wxPoint pntRotate = RotatePointAroundPoint(m_pntWheelMid,
                m_tblMidPnt[i], nRelativeRotation);
            wxPoint sizeDelta = pntRotate - GetMidRect(CB::Convert(pDObj.GetEnclosingRect()));
            pDoc.PlaceObjectOnBoard(m_pPBoard.get(), &pDObj, CB::Convert(sizeDelta));
        }
    }
}

void CPlayBoardView::OnActRotateRelative(wxCommandEvent& /*event*/)
{
    DoRotateRelative(FALSE);
}

void CPlayBoardView::OnActRotateGroupRelative(wxCommandEvent& /*event*/)
{
    DoRotateRelative(TRUE);
}

void CPlayBoardView::DoRotateRelative(BOOL bWheelRotation)
{
    m_bWheelRotation = bWheelRotation;

    CRotatePieceDialog dlg(*this);
    CGamDoc& pDoc = GetDocument();
    CPieceTable& pPTbl = pDoc.GetPieceTable();

    // Get a list of the selected pieces and save their current
    // rotations.
    m_tblCurAngles.clear();
    m_tblCurPieces.clear();
    m_tblMidPnt.clear();

    m_selList.LoadTableWithObjectPtrs(m_tblCurPieces, CSelList::otAll, FALSE);

    wxRect rctGroupRect = CB::Convert(m_selList.GetPiecesEnclosingRect());
    m_pntWheelMid = GetMidRect(rctGroupRect);

    for (size_t i = size_t(0) ; i < m_tblCurPieces.size() ; ++i)
    {
        CDrawObj& pDObj = *m_tblCurPieces[i];
        if (pDObj.GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj& pObj = static_cast<CPieceObj&>(pDObj);
            m_tblCurAngles.push_back(pPTbl.GetPieceFacing(pObj.m_pid));
        }
        else if (pDObj.GetType() == CDrawObj::drawMarkObj)
        {
            CMarkObj& pObj = static_cast<CMarkObj&>(pDObj);
            m_tblCurAngles.push_back(pObj.GetFacing());
        }
        if (m_bWheelRotation &&
            (pDObj.GetType() == CDrawObj::drawPieceObj || pDObj.GetType() == CDrawObj::drawMarkObj))
        {
            wxPoint midPoint = GetMidRect(CB::Convert(pDObj.GetEnclosingRect()));
            m_tblMidPnt.push_back(midPoint);
        }
    }
    // If we're recording moves right now, suspend it for the moment.
    int nDlgResult;
    {
    class SuspendRecording
    {
    public:
        SuspendRecording(CGamDoc& d) : doc(d)
        {
            if (bRecording)
            {
                doc.SetGameState(CGamDoc::stateNotRecording);
            }
        }
        ~SuspendRecording()
        {
            if (bRecording)
            {
                doc.SetGameState(CGamDoc::stateRecording);
            }
        }
    private:
        CGamDoc& doc;
        BOOL bRecording = doc.IsRecording();
    } suspendRecording(pDoc);

    // Show the rotation dialog
    nDlgResult = dlg.ShowModal();

    // Restore angles before possibly recording the operation.
    for (size_t i = size_t(0) ; i < m_tblCurPieces.size() ; ++i)
    {
        CDrawObj& pDObj = *m_tblCurPieces[i];
        if (pDObj.GetType() == CDrawObj::drawPieceObj)
        {
            pDoc.ChangePlayingPieceFacingOnBoard(static_cast<CPieceObj&>(pDObj),
                m_pPBoard.get(), m_tblCurAngles[i]);
        }
        else if (pDObj.GetType() == CDrawObj::drawMarkObj)
        {
            pDoc.ChangeMarkerFacingOnBoard(static_cast<CMarkObj&>(pDObj), m_pPBoard.get(),
                m_tblCurAngles[i]);
        }
        if (m_bWheelRotation &&
            (pDObj.GetType() == CDrawObj::drawPieceObj || pDObj.GetType() == CDrawObj::drawMarkObj))
        {
            // Restore original position
            wxPoint sizeDelta = m_tblMidPnt[i] -
                GetMidRect(CB::Convert(pDObj.GetEnclosingRect()));
            pDoc.PlaceObjectOnBoard(m_pPBoard.get(), &pDObj, CB::Convert(sizeDelta));
        }
    }
    // Restore recording mode if it was active.
    }
    if (nDlgResult == wxID_OK)
    {
        // Rotation was accepted. Make the final changes.
        pDoc.AssignNewMoveGroup();
        for (size_t i = size_t(0) ; i < m_tblCurPieces.size() ; ++i)
        {
            int nAngle = m_tblCurAngles[i] + dlg.m_nRelativeRotation;
            nAngle += nAngle < 0 ? 360 : 0;
            nAngle %= 360;                      // Keep to within a circle

            CDrawObj& pDObj = *m_tblCurPieces[i];

            if (pDObj.GetType() == CDrawObj::drawPieceObj)
            {
                pDoc.ChangePlayingPieceFacingOnBoard(static_cast<CPieceObj&>(pDObj), m_pPBoard.get(), value_preserving_cast<uint16_t>(nAngle));
            }
            else if (pDObj.GetType() == CDrawObj::drawMarkObj)
            {
                pDoc.ChangeMarkerFacingOnBoard(static_cast<CMarkObj&>(pDObj), m_pPBoard.get(), value_preserving_cast<uint16_t>(nAngle));
            }
            if (m_bWheelRotation &&
                (pDObj.GetType() == CDrawObj::drawPieceObj || pDObj.GetType() == CDrawObj::drawMarkObj))
            {
                // Calculate new rotated mid-point for object.
                wxPoint pntRotate = RotatePointAroundPoint(m_pntWheelMid,
                    m_tblMidPnt[i], dlg.m_nRelativeRotation);
                wxPoint sizeDelta = pntRotate - GetMidRect(CB::Convert(pDObj.GetEnclosingRect()));
                pDoc.PlaceObjectOnBoard(m_pPBoard.get(), &pDObj, CB::Convert(sizeDelta));
            }
        }
        m_selList.UpdateObjects(TRUE, FALSE);
        // Make sure we erase old handles.
        rctGroupRect.Inflate(16, 16);
        RefreshRect(rctGroupRect, FALSE);
    }
    m_tblCurAngles.clear();
    m_tblCurPieces.clear();
    m_tblMidPnt.clear();
}

void CPlayBoardView::OnUpdateActRotateRelative(wxUpdateUIEvent& pCmdUI)
{
    CGamDoc& pDoc = GetDocument();
    if (pDoc.IsPlaying() || !pDoc.IsScenario() &&
            m_selList.HasOwnedPiecesNotMatching(pDoc.GetCurrentPlayerMask()))
        pCmdUI.Enable(FALSE);
    else
        pCmdUI.Enable(m_selList.HasPieces() || m_selList.HasMarkers());
}

void CPlayBoardView::OnUpdateActRotateGroupRelative(wxUpdateUIEvent& pCmdUI)
{
    CGamDoc& pDoc = GetDocument();
    if (pDoc.IsPlaying() || !pDoc.IsScenario() &&
            m_selList.HasOwnedPiecesNotMatching(pDoc.GetCurrentPlayerMask()))
        pCmdUI.Enable(FALSE);
    else
    {
        pCmdUI.Enable(m_selList.IsMultipleSelects() &&
            (m_selList.HasPieces() || m_selList.HasMarkers()));
    }
}

///////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnViewPieces(wxCommandEvent& /*event*/)
{
    GetPlayBoard().SetPiecesVisible(!GetPlayBoard().GetPiecesVisible());
    CGamDocHint hint;
    hint.GetArgs<HINT_UPDATEBOARD>().m_pPBoard = m_pPBoard.get();
    GetDocument().UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);
}

void CPlayBoardView::OnUpdateViewPieces(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(!GetPlayBoard().GetPiecesVisible());
}

///////////////////////////////////////////////////////////////////////

void CPlayBoardView::OnEditCopy(wxCommandEvent& /*event*/)
{
    CBoard* pBoard = m_pPBoard->GetBoard();

    wxSize size = CB::Convert(pBoard->GetSize(m_nZoom));

    wxBitmap bmap(size.x, size.y);
    {
    wxMemoryDC dcMem;
    dcMem.SelectObject(bmap);

    wxRect rct(wxPoint(0, 0), size);

    // Draw base board image...
    pBoard->Draw(dcMem, rct, m_nZoom, m_pPBoard->m_bCellBorders);

    // Draw pieces etc.....
    DCSetupDrawListDC setupDrawListDC(*this, dcMem, rct);
    m_pPBoard->Draw(dcMem, rct, m_nZoom);

    }

    LockWxClipboard lockClipbd(std::try_to_lock);
    if (lockClipbd)
    {
        wxBusyCursor busyCursor;

        wxTheClipboard->SetData(new wxBitmapDataObject(bmap));
    }
}

void CPlayBoardView::OnEditBoardToFile(wxCommandEvent& /*event*/)
{
    CB::string strFilter = CB::string::LoadString(IDS_BMP_FILTER);
    CB::string strTitle = CB::string::LoadString(IDS_SEL_BITMAPFILE);

    wxFileDialog dlg(this, strTitle,
                        wxEmptyString, wxEmptyString,
                        strFilter,
                        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxBusyCursor busyCursor;
    try
    {
        CBoard* pBoard = m_pPBoard->GetBoard();

        wxSize size = CB::Convert(pBoard->GetSize(m_nZoom));

        wxBitmap bmap(size.x, size.y);
        {
        wxMemoryDC dcMem;
        dcMem.SelectObject(bmap);

        wxRect rct(wxPoint(0, 0), size);

        // Draw base board image...
        pBoard->Draw(dcMem, rct, m_nZoom, m_pPBoard->m_bCellBorders);

        // Draw pieces etc.....
        DCSetupDrawListDC setupDrawListDC(*this, dcMem, rct);
        m_pPBoard->Draw(dcMem, rct, m_nZoom);

        }

        wxImage img = bmap.ConvertToImage();

        if (!img.SaveFile(dlg.GetPath()))
        {
            wxMessageBox(CB::string::LoadString(IDP_ERR_BMPCREATE),
                            CB::GetAppName(),
                            wxICON_EXCLAMATION);
            return;
        }
    }
    catch (...)
    {
        wxMessageBox(CB::string::LoadString(IDP_ERR_BMPWRITE),
                        CB::GetAppName(),
                        wxICON_EXCLAMATION);
    }
}

void CPlayBoardView::OnEditBoardProperties(wxCommandEvent& /*event*/)
{
    GetDocument().DoBoardProperties(GetPlayBoard());
}

void CPlayBoardView::OnSelectGroupMarkers(wxCommandEvent& event)
{
    SelectMarkersInGroup(value_preserving_cast<size_t>(MarkerXrcidToIndex(event.GetId())));
}

void CPlayBoardView::OnMenuOpen(wxMenuEvent& event)
{
    if (event.GetMenu())
    {
        wxMenu* menu;
        wxMenuItem* markers = event.GetMenu()->FindItem(XRCID("ID_MRKGROUP_FIRST_0"), &menu);
        if (markers)
        {
            OnUpdateSelectGroupMarkers(CheckedDeref(menu));
            return;
        }
    }
    event.Skip();
}

void CPlayBoardView::OnUpdateSelectGroupMarkers(wxMenu& menu)
{
    CMarkManager& pMgr = GetDocument().GetMarkManager();
    if (pMgr.IsEmpty())
        return;
    std::vector<CB::string> tbl;
    tbl.reserve(pMgr.GetNumMarkSets());
    for (size_t i = size_t(0) ; i < pMgr.GetNumMarkSets() ; ++i)
    {
        tbl.push_back(pMgr.GetMarkSet(i).GetName());
    }
    wxASSERT(menu.GetMenuItemCount() == 1);
    menu.Delete(XRCID("ID_MRKGROUP_FIRST_0"));
    wxASSERT(menu.GetMenuItemCount() == 0);

    for (size_t i = size_t(0) ; i < tbl.size() ; ++i)
    {
        int intI = value_preserving_cast<int>(i);
        int xrcid = MarkerIndexToXrcid(intI);
        menu.Append(xrcid, tbl[i]);
        if (intI >= m_bindEnd)
        {
            Bind(wxEVT_MENU, &CPlayBoardView::OnSelectGroupMarkers, this, xrcid);
            ++m_bindEnd;
        }
    }
}

void CPlayBoardView::OnViewDrawIndOnTop(wxCommandEvent& /*event*/)
{
    GetPlayBoard().SetIndicatorsOnTop(!GetPlayBoard().GetIndicatorsOnTop());
    CGamDocHint hint;
    hint.GetArgs<HINT_UPDATEBOARD>().m_pPBoard = m_pPBoard.get();
    GetDocument().UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);
}

void CPlayBoardView::OnUpdateViewDrawIndOnTop(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
    pCmdUI.Check(GetPlayBoard().GetIndicatorsOnTop());
}

void CPlayBoardView::OnEditElementText(wxCommandEvent& /*event*/)
{
    ASSERT(m_selList.IsSingleSelect() && (m_selList.HasMarkers() || m_selList.HasPieces()));

    CDrawObj& pDObj = *m_selList.front()->m_pObj;
    GetDocument().DoEditObjectText(pDObj);
    NotifySelectListChange();       // Make sure indicators are updated
}

void CPlayBoardView::OnUpdateEditElementText(wxUpdateUIEvent& pCmdUI)
{
    CGamDoc& pDoc = GetDocument();
    if (pDoc.IsPlaying() || !pDoc.IsScenario() &&
            m_selList.HasOwnedPiecesNotMatching(pDoc.GetCurrentPlayerMask()))
        pCmdUI.Enable(FALSE);
    else
    {
        pCmdUI.Enable(m_selList.IsSingleSelect() &&
            (m_selList.HasMarkers() || m_selList.HasPieces()));
    }
}

void CPlayBoardView::OnActLockObject(wxCommandEvent& /*event*/)
{
    int nSet;
    int nClear;
    m_selList.CountDObjFlags(dobjFlgLockDown, nSet, nClear);

    // If any are locked then force an unlock. All must be unlocked
    // to do a lock.
    BOOL bLockState = nSet != 0 ? FALSE : TRUE;

    std::vector<CB::not_null<CDrawObj*>> listObjs;
    m_selList.LoadTableWithObjectPtrs(listObjs, CSelList::otAll, FALSE);

    GetDocument().AssignNewMoveGroup();
    GetDocument().SetObjectLockdownTable(listObjs, bLockState);

    if (m_pPBoard->GetLocksEnforced() && bLockState)
        m_selList.PurgeList(TRUE);          // Purge former selections
}

void CPlayBoardView::OnUpdateActLockObject(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying())
    {
        pCmdUI.Enable(FALSE);
        return;
    }

    int nSet;
    int nClear;

    m_selList.CountDObjFlags(dobjFlgLockDown, nSet, nClear);

    if (nSet != 0 && nClear != 0)
        pCmdUI.Set3StateValue(wxCHK_UNDETERMINED);
    else if (nSet != 0 && nClear == 0)
        pCmdUI.Set3StateValue(wxCHK_CHECKED);
    else
        pCmdUI.Set3StateValue(wxCHK_UNCHECKED);
    pCmdUI.Enable(!m_selList.empty());
}

void CPlayBoardView::OnActLockSuspend(wxCommandEvent& /*event*/)
{
    m_pPBoard->SetLocksEnforced(!m_pPBoard->GetLocksEnforced());
    // If enforcement is on and objects are locked, deselect them
    if (m_pPBoard->GetLocksEnforced())
        m_selList.DeselectIfDObjFlagsSet(dobjFlgLockDown);
}

void CPlayBoardView::OnUpdateActLockSuspend(wxUpdateUIEvent& pCmdUI)
{
    if (GetDocument().IsPlaying())
        pCmdUI.Enable(FALSE);
    else
    {
        pCmdUI.Enable(TRUE);
        pCmdUI.Check(!m_pPBoard->GetLocksEnforced());
    }
}


void CPlayBoardView::OnActTakeOwnership(wxCommandEvent& /*event*/)
{
    CRect rct = m_selList.GetPiecesEnclosingRect(FALSE);
    if (rct.IsRectEmpty())
        return;

    CPoint pntCenter(MidPnt(rct.left, rct.right), MidPnt(rct.top, rct.bottom));

    CGamDoc& pDoc = GetDocument();

    std::vector<PieceID> tblPieces;

    m_selList.LoadTableWithOwnerStatePieceIDs(tblPieces, m_selList.LF_NOTOWNED);

    pDoc.AssignNewMoveGroup();

    if (pDoc.IsRecording())
    {
        // Insert a notification tip so there is some information
        // feedback during playback.
        CB::string strMsg = CB::string::LoadString(IDS_TIP_OWNER_ACQUIRED);
        pDoc.RecordEventMessage(strMsg, m_pPBoard->GetSerialNumber(),
            value_preserving_cast<int>(pntCenter.x), value_preserving_cast<int>(pntCenter.y));
    }

    pDoc.SetPieceOwnershipTable(tblPieces, pDoc.GetCurrentPlayerMask());

    CGamDocHint hint;
    hint.GetArgs<HINT_UPDATEBOARD>().m_pPBoard = m_pPBoard.get();
    pDoc.UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);

    NotifySelectListChange();
}

void CPlayBoardView::OnUpdateActTakeOwnership(wxUpdateUIEvent& pCmdUI)
{
    CGamDoc& pDoc = GetDocument();
    // Can't take ownership while residing on an owned board.
    if (pDoc.IsPlaying() || m_pPBoard->IsOwned())
        pCmdUI.Enable(FALSE);
    else if (pDoc.IsCurrentPlayerReferee())
        pCmdUI.Enable(FALSE);      // No owner to acquire. He's the Referee!
    else
    {
        pCmdUI.Enable(pDoc.HasPlayers() && m_selList.HasNonOwnedPieces() &&
            pDoc.GetCurrentPlayerMask() != OWNER_MASK_SPECTATOR);
    }
}

void CPlayBoardView::OnActReleaseOwnership(wxCommandEvent& /*event*/)
{
    CRect rct = m_selList.GetPiecesEnclosingRect(FALSE);
    if (rct.IsRectEmpty())
        return;

    CPoint pntCenter(MidPnt(rct.left, rct.right), MidPnt(rct.top, rct.bottom));

    CGamDoc& pDoc = GetDocument();

    std::vector<PieceID> tblPieces;

    m_selList.LoadTableWithOwnerStatePieceIDs(tblPieces, m_selList.LF_OWNED);

    pDoc.AssignNewMoveGroup();

    if (pDoc.IsRecording())
    {
        // Insert a notification tip so there is some information
        // feedback during playback.
        CB::string strMsg = CB::string::LoadString(IDS_TIP_OWNER_RELEASED);
        pDoc.RecordEventMessage(strMsg, m_pPBoard->GetSerialNumber(),
            value_preserving_cast<int>(pntCenter.x), value_preserving_cast<int>(pntCenter.y));
    }

    pDoc.SetPieceOwnershipTable(tblPieces, OWNER_MASK_SPECTATOR);

    CGamDocHint hint;
    hint.GetArgs<HINT_UPDATEBOARD>().m_pPBoard = m_pPBoard.get();
    pDoc.UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);

    NotifySelectListChange();
}

void CPlayBoardView::OnUpdateActReleaseOwnership(wxUpdateUIEvent& pCmdUI)
{
    CGamDoc& pDoc = GetDocument();
    // Can't release ownership while residing on an owned board.
    if (pDoc.IsPlaying() || m_pPBoard->IsOwned())
        pCmdUI.Enable(FALSE);
    else if (pDoc.IsCurrentPlayerReferee() && m_selList.HasPieces())
        pCmdUI.Enable(TRUE);
    else
    {
        pCmdUI.Enable(pDoc.HasPlayers() && m_selList.HasOwnedPieces() &&
            pDoc.GetCurrentPlayerMask() != OWNER_MASK_SPECTATOR);
    }
}

void CPlayBoardView::OnActSetOwner(wxCommandEvent& /*event*/)
{
    CGamDoc& pDoc = GetDocument();
    CRect rct = m_selList.GetPiecesEnclosingRect(FALSE);
    if (rct.IsRectEmpty())
        return;

    CSelectNewOwnerDialog dlg(CheckedDeref(pDoc.GetPlayerManager()));
    if (!pDoc.IsCurrentPlayerReferee())
        dlg.m_nPlayer = CPlayerManager::GetPlayerNumFromMask(pDoc.GetCurrentPlayerMask());

    if (dlg.ShowModal() != wxID_OK)
        return;

    PlayerMask dwNewOwnerMask = CPlayerManager::GetMaskFromPlayerNum(dlg.m_nPlayer);

    CPoint pntCenter(MidPnt(rct.left, rct.right), MidPnt(rct.top, rct.bottom));

    std::vector<PieceID> tblPieces;
    m_selList.LoadTableWithOwnerStatePieceIDs(tblPieces, m_selList.LF_BOTH);

    pDoc.AssignNewMoveGroup();

    if (pDoc.IsRecording())
    {
        // Insert a notification tip so there is some information
        // feedback during playback.
        CB::string strMsg = CB::string::LoadString(IDS_TIP_OWNER_ACQUIRED);
        pDoc.RecordEventMessage(strMsg, m_pPBoard->GetSerialNumber(),
            value_preserving_cast<int>(pntCenter.x), value_preserving_cast<int>(pntCenter.y));
    }

    pDoc.SetPieceOwnershipTable(tblPieces, dwNewOwnerMask);

    CGamDocHint hint;
    hint.GetArgs<HINT_UPDATEBOARD>().m_pPBoard = m_pPBoard.get();
    pDoc.UpdateAllViews(NULL, HINT_UPDATEBOARD, &hint);

    NotifySelectListChange();
}

void CPlayBoardView::OnUpdateActSetOwner(wxUpdateUIEvent& pCmdUI)
{
    CGamDoc& pDoc = GetDocument();
    // Can't take ownership while residing on an owned board.
    if (pDoc.IsPlaying() || m_pPBoard->IsOwned())
        pCmdUI.Enable(FALSE);
    else if (pDoc.IsCurrentPlayerReferee() && m_selList.HasPieces())
        pCmdUI.Enable(TRUE);
    else
    {
        pCmdUI.Enable(pDoc.HasPlayers() &&
            (m_selList.HasPieces() && pDoc.GetCurrentPlayerMask() != OWNER_MASK_SPECTATOR));
    }
}

#if 0
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
        static unsigned uWheelScrollLines = unsigned(-1);
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
                nDisplacement = CB::min(nDisplacement, m_pageDev.cy);
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
                nDisplacement = CB::min(nDisplacement, m_pageDev.cx);
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
            int yEdge = CB::min(point.y, rctClient.bottom - point.y);
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
                nDisplacement = CB::min(nDisplacement, m_pageDev.cy);
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
                nDisplacement = CB::min(nDisplacement, m_pageDev.cx);
            }
            bResult = OnScrollBy(CSize(nDisplacement, 0), TRUE);
        }

        if (bResult)
            UpdateWindow();

        return bResult;
    }
}
#endif

void CPlayBoardView::OnUpdateEnable(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
}

void CPlayBoardViewContainer::OnDraw(CDC* pDC)
{
    // do nothing because child covers entire client rect
}

void CPlayBoardViewContainer::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    child->OnUpdate(pSender, lHint, pHint);

    BASE::OnUpdate(pSender, lHint, pHint);
}

void CPlayBoardViewContainer::OnActivateView(BOOL bActivate, CView* pActivateView,
                CView* pDeactiveView)
{
    BASE::OnActivateView(bActivate, pActivateView, pDeactiveView);
    child->OnActivateView(bActivate, pActivateView, pDeactiveView);
}

CPlayBoardViewContainer::CPlayBoardViewContainer() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

int CPlayBoardViewContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (BASE::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    child = new CPlayBoardView(*this);

    return 0;
}

// MFC puts the focus here, so move it to the useful window
void CPlayBoardViewContainer::OnSetFocus(CWnd* pOldWnd)
{
    BASE::OnSetFocus(pOldWnd);
    child->SetFocus();
}

void CPlayBoardViewContainer::OnSize(UINT nType, int cx, int cy)
{
    child->SetSize(0, 0, cx, cy);
    return BASE::OnSize(nType, cx, cy);
}

LRESULT CPlayBoardViewContainer::OnMessageWindowState(WPARAM wParam, LPARAM lParam)
{
    WinStateEvent event(*reinterpret_cast<CArchive*>(wParam), bool(lParam));
    child->ProcessWindowEvent(event);
    return (LRESULT)1;
}
