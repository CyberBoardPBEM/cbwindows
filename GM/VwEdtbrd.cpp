// VwEdtbrd.cpp : implementation of the CBrdEditView class
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "GMisc.h"
#include    "ClipBrd.h"
#include    "Tile.h"
#include    "LBoxTile.h"

#include    "ToolObjs.h"
#include    "SelObjs.h"

#include    "DlgTexto.h"
#include    "DlgSvisi.h"
#include    "FrmMain.h"
#include    "VwEdtbrd.h"
#include    "PalColor.h"
#include    "CDib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView

IMPLEMENT_DYNCREATE(CBrdEditViewContainer, CView)

wxBEGIN_EVENT_TABLE(CBrdEditView, wxScrolledCanvas)
#if 0
    ON_WM_MOUSEWHEEL()
#endif
    EVT_LEFT_DOWN(OnLButtonDown)
    EVT_LEFT_UP(OnLButtonUp)
    EVT_MOTION(OnMouseMove)
    EVT_LEFT_DCLICK(OnLButtonDblClk)
    EVT_MOUSE_CAPTURE_LOST(OnMouseCaptureLost)
#if 0
    ON_WM_TIMER()
    ON_WM_KEYDOWN()
    ON_WM_CHAR()
#endif
    EVT_SET_CURSOR(OnSetCursor)
#if 0
    ON_WM_ERASEBKGND()
#endif
    EVT_DRAGDROP(OnDragTileItem)
    EVT_SETCOLOR(OnSetColor)
    EVT_SETCUSTOMCOLOR(OnSetCustomColors)
    EVT_SETLINEWIDTH(OnSetLineWidth)
#if 0
    ON_COMMAND(ID_OFFSCREEN, OnOffscreen)
    ON_COMMAND(ID_VIEW_GRIDLINES, OnViewGridLines)
    ON_COMMAND(ID_DWG_TOBACK, OnDwgToBack)
    ON_COMMAND(ID_DWG_TOFRONT, OnDwgToFront)
#endif
    EVT_MENU(XRCID("ID_TOOL_ARROW"), OnToolPalette)
    EVT_MENU(XRCID("ID_EDIT_LAYER_BASE"), OnEditLayer)
#if 0
    ON_UPDATE_COMMAND_UI(ID_VIEW_GRIDLINES, OnUpdateViewGridLines)
    ON_UPDATE_COMMAND_UI(ID_OFFSCREEN, OnUpdateOffscreen)
#endif
    EVT_UPDATE_UI(XRCID("ID_EDIT_LAYER_BASE"), OnUpdateEditLayer)
    EVT_UPDATE_UI(XRCID("ID_COLOR_FOREGROUND"), OnUpdateColorForeground)
    EVT_UPDATE_UI(XRCID("ID_COLOR_BACKGROUND"), OnUpdateColorBackground)
    EVT_UPDATE_UI(XRCID("ID_COLOR_CUSTOM"), OnUpdateColorCustom)
    EVT_UPDATE_UI(XRCID("ID_LINE_WIDTH"), OnUpdateLineWidth)
    EVT_UPDATE_UI(XRCID("ID_TOOL_ARROW"), OnUpdateToolPalette)
#if 0
    ON_UPDATE_COMMAND_UI(ID_DWG_TOFRONT, OnUpdateDwgToFrontOrBack)
#endif
    EVT_UPDATE_UI(XRCID("ID_VIEW_FULLSCALE"), OnUpdateViewFullScale)
    EVT_UPDATE_UI(XRCID("ID_VIEW_HALFSCALE"), OnUpdateViewHalfScale)
#if 0
    ON_COMMAND(ID_DWG_FONT, OnDwgFont)
#endif
    EVT_MENU(XRCID("ID_VIEW_FULLSCALE"), OnViewFullScale)
    EVT_MENU(XRCID("ID_VIEW_HALFSCALE"), OnViewHalfScale)
    EVT_MENU(XRCID("ID_VIEW_SMALLSCALE"), OnViewSmallScale)
    EVT_UPDATE_UI(XRCID("ID_VIEW_SMALLSCALE"), OnUpdateViewSmallScale)
#if 0
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_CELLNUM, OnUpdateIndicatorCellNum)
    ON_COMMAND(ID_TOOLS_BRDSNAPGRID, OnToolsBrdSnapGrid)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_BRDSNAPGRID, OnUpdateToolsBrdSnapGrid)
    ON_COMMAND(ID_TOOLS_BRDPROPS, OnToolsBrdProps)
    ON_COMMAND(ID_TOOL_SETVISIBLESCALE, OnToolSetVisibleScale)
    ON_UPDATE_COMMAND_UI(ID_TOOL_SETVISIBLESCALE, OnUpdateToolSetVisibleScale)
    ON_COMMAND(ID_TOOL_SUSPENDSCALEVISIBILITY, OnToolSuspendScaleVisibility)
    ON_UPDATE_COMMAND_UI(ID_TOOL_SUSPENDSCALEVISIBILITY, OnUpdateToolSuspendScaleVsibility)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_COMMAND(ID_EDIT_PASTEBITMAPFROMFILE, OnEditPasteBitmapFromFile)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEBITMAPFROMFILE, OnUpdateEditPasteBitmapFromFile)
    ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
    ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_WM_SYSKEYDOWN()
    ON_COMMAND(ID_DWG_DRAWABOVEGRID, OnDwgDrawAboveGrid)
    ON_UPDATE_COMMAND_UI(ID_DWG_DRAWABOVEGRID, OnUpdateDwgDrawAboveGrid)
#endif
    EVT_MENU(XRCID("ID_EDIT_LAYER_TILE"), OnEditLayer)
    EVT_MENU(XRCID("ID_EDIT_LAYER_TOP"), OnEditLayer)
    EVT_MENU(XRCID("ID_TOOL_ERASER"), OnToolPalette)
    EVT_MENU(XRCID("ID_TOOL_TILE"), OnToolPalette)
    EVT_MENU(XRCID("ID_TOOL_TEXT"), OnToolPalette)
    EVT_MENU(XRCID("ID_TOOL_FILL"), OnToolPalette)
    EVT_MENU(XRCID("ID_TOOL_DROPPER"), OnToolPalette)
    EVT_MENU(XRCID("ID_TOOL_LINE"), OnToolPalette)
    EVT_MENU(XRCID("ID_TOOL_POLYGON"), OnToolPalette)
    EVT_MENU(XRCID("ID_TOOL_RECT"), OnToolPalette)
    EVT_MENU(XRCID("ID_TOOL_OVAL"), OnToolPalette)
    EVT_UPDATE_UI(XRCID("ID_EDIT_LAYER_TOP"), OnUpdateEditLayer)
    EVT_UPDATE_UI(XRCID("ID_EDIT_LAYER_TILE"), OnUpdateEditLayer)
    EVT_UPDATE_UI(XRCID("ID_TOOL_ERASER"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_TOOL_TILE"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_TOOL_TEXT"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_TOOL_FILL"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_TOOL_DROPPER"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_TOOL_LINE"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_TOOL_POLYGON"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_TOOL_RECT"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_TOOL_OVAL"), OnUpdateToolPalette)
#if 0
    ON_UPDATE_COMMAND_UI(ID_DWG_TOBACK, OnUpdateDwgToFrontOrBack)
#endif
    EVT_MENU(XRCID("ID_VIEW_TOGGLE_SCALE"), OnViewToggleScale)
    EVT_UPDATE_UI(XRCID("ID_VIEW_TOGGLE_SCALE"), OnUpdateEnable)
    EVT_SCROLLWIN_LINEDOWN(OnScrollWinLine)
    EVT_SCROLLWIN_LINEUP(OnScrollWinLine)
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CBrdEditViewContainer, CView)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView construction/destruction

CBrdEditView::CBrdEditView(CBrdEditViewContainer& p) :
    m_selList(*this),
    parent(&p),
    document(dynamic_cast<CGamDoc*>(parent->GetDocument()))
{
    m_bOffScreen = TRUE;
    m_pBoard = NULL;
    m_pBMgr = NULL;
    m_nCurToolID = XRCID("ID_TOOL_ARROW");       // ID_TOOL_ARROW tool (select)
    m_nLastToolID = XRCID("ID_TOOL_ARROW");      // ID_TOOL_ARROW tool (select)
    m_nZoom = fullScale;
    // use sizers for scrolling
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    sizer->Add(0, 0);
    wxScrolledCanvas::Create(*parent, 0);
}

CBrdEditView::~CBrdEditView()
{
}

#if 0
BOOL CBrdEditView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CScrollView::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)GetStockObject(DKGRAY_BRUSH));

    return TRUE;
}
#endif

void CBrdEditView::OnInitialUpdate()
{
    m_pBMgr = GetDocument().GetBoardManager();
    m_pBoard = static_cast<CBoard*>(GetDocument().GetCreateParameter());
    RecalcScrollLimits();
}

void CBrdEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    WORD wHint = LOWORD(lHint);
    if ((wHint == HINT_TILEMODIFIED && m_pBoard->IsTileInUse(static_cast<CGmBoxHint*>(pHint)->GetArgs<HINT_TILEMODIFIED>().m_tid)) ||
        wHint == HINT_TILEDELETED || wHint == HINT_TILESETDELETED)
    {
        Refresh(FALSE);          // Do redraw
        return;
    }
    else if (wHint == HINT_BOARDDELETED)
    {
        if (static_cast<CGmBoxHint*>(pHint)->GetArgs<HINT_BOARDDELETED>().m_pBoard == m_pBoard)
        {
            CFrameWnd* pFrm = parent->GetParentFrame();
            ASSERT(pFrm != NULL);
            pFrm->SendMessage(WM_CLOSE, 0, 0L);
        }
    }
    else if (wHint == HINT_BOARDPROPCHANGE)
    {
        if (static_cast<CGmBoxHint*>(pHint)->GetArgs<HINT_BOARDPROPCHANGE>().m_pBoard == m_pBoard)
        {
            RecalcScrollLimits();
            Refresh(FALSE);
        }
    }
    else if (wHint == HINT_ALWAYSUPDATE)
    {
        wxASSERT(!"untested code");
        parent->CView::OnUpdate(pSender, lHint, pHint);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView drawing

void CBrdEditView::OnDraw(wxDC& pDC)
{
    GetOverlay().Reset();
    wxMemoryDC dcMem;
    wxRect oRct;
    wxDC* pDrawDC = &pDC;

    CB_VERIFY(pDC.GetClippingBox(oRct));
    if (oRct.IsEmpty())
    {
        return;                 // Nothing to do
    }

    if (m_bOffScreen)
    {
        wxBitmap bmMem = wxBitmap(
            oRct.GetWidth(), oRct.GetHeight(), pDC);
        dcMem.SelectObject(bmMem);
        dcMem.SetDeviceOrigin(-oRct.GetLeft(), -oRct.GetTop());
        pDrawDC = &dcMem;
    }

    m_pBoard->Draw(*pDrawDC, oRct, m_nZoom);

    if (m_bOffScreen)
    {
        pDC.Blit(oRct.GetLeftTop(), oRct.GetSize(),
            &dcMem, oRct.GetLeftTop(), wxCOPY);
    }
    wxASSERT(!dynamic_cast<wxPrinterDC*>(&pDC));
    if (!dynamic_cast<wxPrinterDC*>(&pDC))
    {
        PrepareScaledDC(pDC);
        m_selList.OnDraw(pDC);
    }
}

#if 0
BOOL CBrdEditView::OnEraseBkgnd(CDC* pDC)
{
    return CScrollView::OnEraseBkgnd(pDC);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Coordinate space mappings

void CBrdEditView::PrepareScaledDC(wxDC& pDC) const
{
    wxSize wsize, vsize;
    m_pBoard->GetBoardArray().GetBoardScaling(m_nZoom, wsize, vsize);
    pDC.SetUserScale(double(vsize.x)/wsize.x, double(vsize.y)/wsize.y);
}

void CBrdEditView::OnPrepareScaledDC(wxDC& pDC)
{
    PrepareDC(pDC);
    PrepareScaledDC(pDC);
}

void CBrdEditView::ClientToWorkspace(wxPoint& point) const
{
    point = CalcUnscrolledPosition(point);
    wxSize wsize, vsize;
    m_pBoard->GetBoardArray().GetBoardScaling(m_nZoom, wsize, vsize);
    ScalePoint(point, wsize, vsize);
}

void CBrdEditView::ClientToWorkspace(wxRect& rect) const
{
    rect.SetTopLeft(CalcUnscrolledPosition(rect.GetTopLeft()));
    wxSize wsize, vsize;
    m_pBoard->GetBoardArray().GetBoardScaling(m_nZoom, wsize, vsize);
    ScaleRect(rect, wsize, vsize);
}

void CBrdEditView::WorkspaceToClient(wxPoint& point) const
{
    wxSize wsize, vsize;
    m_pBoard->GetBoardArray().GetBoardScaling(m_nZoom, wsize, vsize);
    ScalePoint(point, vsize, wsize);
    point = CalcScrolledPosition(point);
}

void CBrdEditView::WorkspaceToClient(wxRect& rect) const
{
    wxSize wsize, vsize;
    m_pBoard->GetBoardArray().GetBoardScaling(m_nZoom, wsize, vsize);
    ScaleRect(rect, vsize, wsize);
    rect.SetLeftTop(CalcScrolledPosition(rect.GetTopLeft()));
}

void CBrdEditView::InvalidateWorkspaceRect(const wxRect& pRect, BOOL bErase)
{
    wxRect rct = WorkspaceToClient(pRect);
    rct.Inflate(1, 1);
    RefreshRect(rct, bErase);
}

wxPoint CBrdEditView::GetWorkspaceDim() const
{
    // First get MM_TEXT size of board for this scaling mode.
    wxSize size = CB::Convert(m_pBoard->GetSize(m_nZoom));
    wxPoint pnt(size.x, size.y);
    // Translate to current scaling mode.
    pnt = CalcScrolledPosition(pnt);
    ClientToWorkspace(pnt);
    return pnt;
}

/////////////////////////////////////////////////////////////////////////////

void CBrdEditView::ResetToDefaultTool()
{
    if (!GetDocument().GetStickyDrawTools())
    {
        m_nCurToolID = m_nLastToolID = XRCID("ID_TOOL_ARROW");
    }
}

/////////////////////////////////////////////////////////////////////////////

CDrawObj* CBrdEditView::ObjectHitTest(wxPoint point)
{
    CDrawList* pDwg = GetDrawList(FALSE);
    if (pDwg != NULL)
        return pDwg->HitTest(CB::Convert(point), m_nZoom, m_pBoard->GetApplyVisible());
    else
        return NULL;
}

void CBrdEditView::SelectWithinRect(wxRect rctNet, BOOL bInclIntersects)
{
    CDrawList* pDwg = GetDrawList(FALSE);
    if (pDwg == NULL)
        return;

    for (CDrawList::iterator pos = pDwg->begin(); pos != pDwg->end(); ++pos)
    {
        CDrawObj& pObj = **pos;

        if (m_pBoard->GetApplyVisible() && (pObj.GetDObjFlags() && m_nZoom == 0))
            continue;                   // Doesn't qualify

        if (!m_selList.IsObjectSelected(pObj))
        {
            if ((!bInclIntersects &&
                ((CB::Convert(pObj.GetEnclosingRect()) + rctNet) == rctNet)) ||
                (bInclIntersects &&
                (!(CB::Convert(pObj.GetEnclosingRect()) * rctNet).IsEmpty())))
            {
                m_selList.AddObject(pObj, TRUE);
            }
        }
    }
}

void CBrdEditView::SelectAllUnderPoint(wxPoint point)
{
    CDrawList* pDwg = GetDrawList(FALSE);
    if (pDwg == NULL)
        return;

    std::vector<CB::not_null<CDrawObj*>> selLst;
    pDwg->DrillDownHitTest(CB::Convert(point), selLst, m_nZoom,
        m_pBoard->GetApplyVisible());

    for (size_t i = size_t(0) ; i < selLst.size() ; ++i)
    {
        CDrawObj& pObj = *selLst[i];
        if (!m_selList.IsObjectSelected(pObj))
            m_selList.AddObject(pObj, TRUE);
    }
}

#if 0
void CBrdEditView::DeleteObjsInSelectList(BOOL bInvalidate)
{
    CDrawList* pDwg = GetDrawList(FALSE);
    if (pDwg == NULL)
        return;

    if (bInvalidate)
        m_selList.InvalidateListHandles();

    CRect rct;
    while (!m_selList.empty())
    {
        OwnerPtr<CSelection> pSel = std::move(m_selList.front());
        m_selList.pop_front();
        pDwg->RemoveObject(*pSel->m_pObj);
        if (bInvalidate)
            pSel->Invalidate();
        delete pSel->m_pObj.get();
    }
    GetDocument().SetModifiedFlag();
}

void CBrdEditView::MoveObjsInSelectList(BOOL bToFront, BOOL bInvalidate)
{
    CDrawList* pDwg = GetDrawList(FALSE);
    if (pDwg == NULL)
        return;

    // Loop through the drawing list looking for objects that are
    // selected. Remove and add them to a local list. Then take
    // the objects and add them to the front or back. The reason for
    // the temp list is to maintain ordering of selected objects.
    std::vector<CB::not_null<CDrawObj*>> m_tmpLst;
    m_tmpLst.reserve(m_selList.size());

    for (CSelList::iterator pos = m_selList.begin() ; pos != m_selList.end() ; ++pos)
    {
        CSelection& pSel = **pos;
        pDwg->RemoveObject(*pSel.m_pObj);
        m_tmpLst.push_back(pSel.m_pObj.get());
    }
    if (bToFront)
    {
        for (auto pos = m_tmpLst.begin() ; pos != m_tmpLst.end() ; ++pos)
        {
            pDwg->AddToFront(pos->get());
        }
    }
    else
    {
        for (auto pos = m_tmpLst.rbegin() ; pos != m_tmpLst.rend() ; ++pos)
        {
            pDwg->AddToBack(pos->get());
        }
    }
    if (bInvalidate)
        m_selList.InvalidateList();
    GetDocument().SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView printing

void CBrdEditView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
    CScrollView::OnPrepareDC(pDC, pInfo);
//  if (pInfo != NULL)
//  {
//      CDC* pScrnDC = GetDC();
//      pDC->SetMapMode(MM_ANISOTROPIC);
//      pDC->SetWindowExt(pScrnDC->GetDeviceCaps(LOGPIXELSX),
//          pScrnDC->GetDeviceCaps(LOGPIXELSY));
//      pDC->SetViewportExt(pDC->GetDeviceCaps(LOGPIXELSX),
//          pDC->GetDeviceCaps(LOGPIXELSY));
//      ReleaseDC(pScrnDC);
//  }
}

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView diagnostics

#ifdef _DEBUG
void CBrdEditView::AssertValid() const
{
    CView::AssertValid();
}

void CBrdEditView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CBrdEditView::OnOffscreen()
{
    m_bOffScreen = !m_bOffScreen;
}

void CBrdEditView::OnUpdateOffscreen(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bOffScreen);
}

void CBrdEditView::OnViewGridLines()
{
    BeginWaitCursor();
    m_pBoard->SetCellBorder(!m_pBoard->GetCellBorder());
    Invalidate(FALSE);
    UpdateWindow();
    EndWaitCursor();

    CGmBoxHint hint;
    hint.GetArgs<HINT_BOARDPROPCHANGE>().m_pBoard = &*m_pBoard;
    GetDocument().UpdateAllViews(this, HINT_BOARDPROPCHANGE, &hint);
}

void CBrdEditView::OnUpdateViewGridLines(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_pBoard->GetCellBorder());
}
#endif

namespace
{
    int IdToLayer(int id)
    {
        static const std::map<int, int> map = {
            { XRCID("ID_EDIT_LAYER_BASE"), LAYER_BASE },
            { XRCID("ID_EDIT_LAYER_TILE"), LAYER_GRID },
            { XRCID("ID_EDIT_LAYER_TOP"), LAYER_TOP },
        };
        return map.at(id);
    }
}

void CBrdEditView::OnEditLayer(wxCommandEvent& event)
{
    m_nCurToolID = XRCID("ID_TOOL_ARROW");       // Turn off tool with layer change
    m_nLastToolID = XRCID("ID_TOOL_ARROW");      // Turn off tool with layer change
    m_selList.PurgeList(TRUE);

    wxBusyCursor busyCursor;
    m_pBoard->SetMaxDrawLayer(IdToLayer(event.GetId()));
    Refresh(false);
    Update();
}

void CBrdEditView::OnUpdateEditLayer(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(TRUE);
    pCmdUI.Check(m_pBoard->GetMaxDrawLayer() == -1 ?
        pCmdUI.GetId() == XRCID("ID_EDIT_LAYER_TOP") :
        IdToLayer(pCmdUI.GetId()) ==
            m_pBoard->GetMaxDrawLayer());
}

/////////////////////////////////////////////////////////////////////////////

#if 0
void CBrdEditView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    // Make sure window is active.
    GetParentFrame()->ActivateFrame();

    CMenu bar;
    if (bar.LoadMenuW(IDR_MENU_DESIGN_POPUPS))
    {
        CMenu& popup = *bar.GetSubMenu(MENU_BV_DRAWING);
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
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView mouse and timer message handlers

void CBrdEditView::OnLButtonDown(wxMouseEvent& event)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    if (eToolType == ttypeUnknown)
    {
        OwnerPtr<wxEvent> moveEvent = event.Clone();
        moveEvent->SetEventType(wxEVT_MOTION);
        ProcessWindowEvent(*moveEvent);
        return;
    }
    CTool& pTool = CTool::GetTool(eToolType);
    wxPoint point = ClientToWorkspace(event.GetPosition());
    pTool.OnLButtonDown(*this, event.GetModifiers(), point);
}

void CBrdEditView::OnMouseMove(wxMouseEvent& event)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    if (eToolType == ttypeUnknown)
    {
        event.Skip();
        return;
    }
    CTool& pTool = CTool::GetTool(eToolType);
    wxPoint point = ClientToWorkspace(event.GetPosition());
    pTool.OnMouseMove(*this, event.GetModifiers(), CB::GetMouseButtons(event), point);
}

void CBrdEditView::OnLButtonUp(wxMouseEvent& event)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    if (eToolType == ttypeUnknown)
    {
        OwnerPtr<wxEvent> moveEvent = event.Clone();
        moveEvent->SetEventType(wxEVT_MOTION);
        ProcessWindowEvent(*moveEvent);
        return;
    }
    CTool& pTool = CTool::GetTool(eToolType);
    wxPoint point = ClientToWorkspace(event.GetPosition());
    pTool.OnLButtonUp(*this, event.GetModifiers(), point);
}

void CBrdEditView::OnLButtonDblClk(wxMouseEvent& event)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    if (eToolType == ttypeUnknown)
    {
        OwnerPtr<wxEvent> moveEvent = event.Clone();
        moveEvent->SetEventType(wxEVT_MOTION);
        ProcessWindowEvent(*moveEvent);
        return;
    }
    CTool& pTool = CTool::GetTool(eToolType);
    wxPoint point = ClientToWorkspace(event.GetPosition());
    pTool.OnLButtonDblClk(*this, event.GetModifiers(), point);
}

void CBrdEditView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    if (eToolType == ttypeUnknown)
    {
        return;
    }
    CTool& pTool = CTool::GetTool(eToolType);
    pTool.OnMouseCaptureLost(*this);
}

#if 0
void CBrdEditView::OnTimer(uintptr_t nIDEvent)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    CTool& pTool = CTool::GetTool(eToolType);
    pTool.OnTimer(*this, nIDEvent);
}
#endif

void CBrdEditView::OnSetCursor(wxSetCursorEvent& event)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    wxASSERT(event.GetEventObject() == this);
    if (event.GetEventObject() == this && eToolType != ttypeUnknown)
    {
        CTool& pTool = CTool::GetTool(eToolType);
        wxPoint point = ClientToWorkspace(wxPoint(event.GetX(), event.GetY()));
        wxCursor rc = pTool.OnSetCursor(*this, point);
        if (rc.IsOk())
        {
            event.SetCursor(rc);
            return;
        }
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////

#if 0
void CBrdEditView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    CTool& pTool = CTool::GetTool(eToolType);
    if (pTool.m_eToolType == ttypePolygon)
    {
        CPolyTool& pPolyTool = static_cast<CPolyTool&>(pTool);
        if (nChar == VK_ESCAPE)
        {
            pPolyTool.RemoveRubberBand(*this);
            pPolyTool.FinalizePolygon(*this, TRUE);
        }
        else if (nChar == VK_RETURN)
        {
            pPolyTool.RemoveRubberBand(*this);
            pPolyTool.FinalizePolygon(*this, FALSE);
        }
    }
    CScrollView::OnChar(nChar, nRepCnt, nFlags);
}

void CBrdEditView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_DOWN)
    {
        HandleKeyDown ();
    }

    else if (nChar == VK_UP)
    {
        HandleKeyUp ();
    }

    else if (nChar == VK_LEFT)
    {
        HandleKeyLeft ();
    }

    else if (nChar == VK_RIGHT)
    {
        HandleKeyRight ();
    }

    else if (nChar == VK_NEXT)
    {
        HandleKeyPageDown ();
    }

    else if (nChar == VK_PRIOR)
    {
        HandleKeyPageUp ();
    }

    else if (nChar == VK_HOME)
    {
        HandleKeyTop ();
    }

    else if (nChar == VK_END)
    {
        HandleKeyBottom ();
    }
    else
    {
        CScrollView::OnSysKeyDown(nChar, nRepCnt, nFlags);
    }
//  OnKeyDown (nChar, nRepCnt, nFlags);
}

void CBrdEditView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_DELETE && m_pBoard->GetMaxDrawLayer() != LAYER_GRID)
    {
        DeleteObjsInSelectList(TRUE);
    }

    //...DFM19991118
    else if (nChar == VK_DOWN)
    {
        HandleKeyDown ();
    }

    else if (nChar == VK_UP)
    {
        HandleKeyUp ();
    }

    else if (nChar == VK_LEFT)
    {
        HandleKeyLeft ();
    }

    else if (nChar == VK_RIGHT)
    {
        HandleKeyRight ();
    }

    else if (nChar == VK_NEXT)
    {
        HandleKeyPageDown ();
    }

    else if (nChar == VK_PRIOR)
    {
        HandleKeyPageUp ();
    }

    else if (nChar == VK_HOME)
    {
        HandleKeyTop ();
    }

    else if (nChar == VK_END)
    {
        HandleKeyBottom ();
    }
    //...DFM19991118
}

//////////////////////////////////////////////////////////////////////

void CBrdEditView::OnEditClear()
{
    ASSERT(m_pBoard->GetMaxDrawLayer() != LAYER_GRID &&
        m_selList.IsAnySelects());
    DeleteObjsInSelectList(TRUE);
}

void CBrdEditView::OnUpdateEditClear(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_pBoard->GetMaxDrawLayer() != LAYER_GRID &&
        m_selList.IsAnySelects());
}
#endif

//////////////////////////////////////////////////////////////////////

ToolType CBrdEditView::MapToolType(int nToolResID) const
{
    // wx doesn't guarantee XRCID() value order
    static const std::unordered_map<int, ToolType> tblDrawTools
    {
        { XRCID("ID_TOOL_ARROW"), ttypeSelect },
        { XRCID("ID_TOOL_ERASER"), ttypeUnknown },
        { XRCID("ID_TOOL_TILE"), ttypeTile },
        { XRCID("ID_TOOL_TEXT"), ttypeText },
        { XRCID("ID_TOOL_FILL"), ttypePaintBack },
        { XRCID("ID_TOOL_DROPPER"), ttypeColorPick },
        { XRCID("ID_TOOL_LINE"), ttypeLine },
        { XRCID("ID_TOOL_POLYGON"), ttypePolygon },
        { XRCID("ID_TOOL_RECT"), ttypeRect },
        { XRCID("ID_TOOL_OVAL"), ttypeEllipse },
    };

    static const std::unordered_map<int, ToolType> tblGridTools
    {
        { XRCID("ID_TOOL_ARROW"), ttypeUnknown },
        { XRCID("ID_TOOL_ERASER"), ttypeCellEraser },
        { XRCID("ID_TOOL_TILE"), ttypeTile },
        { XRCID("ID_TOOL_TEXT"), ttypeUnknown },
        { XRCID("ID_TOOL_FILL"), ttypeCellPaint },
        { XRCID("ID_TOOL_DROPPER"), ttypeColorPick },
        { XRCID("ID_TOOL_LINE"), ttypeUnknown },
        { XRCID("ID_TOOL_POLYGON"), ttypeUnknown },
        { XRCID("ID_TOOL_RECT"), ttypeUnknown },
        { XRCID("ID_TOOL_OVAL"), ttypeUnknown },
    };

    return m_pBoard->GetMaxDrawLayer() == LAYER_GRID ?
        tblGridTools.at(nToolResID) :
        tblDrawTools.at(nToolResID);
}

//////////////////////////////////////////////////////////////////////
// Tile drag and drop code.

void CBrdEditView::OnDragTileItem(DragDropEvent& event)
{
    if (event.GetProcessId() != wxGetProcessId())
    {
        return;
    }
    m_nCurToolID = XRCID("ID_TOOL_ARROW");       // Not valid with drag over.
    m_nLastToolID = XRCID("ID_TOOL_ARROW");      // Not valid with drag over.

    const DragInfoWx& pdi = event.GetDragInfo();

    if (pdi.GetDragType() != DRAG_TILE)
        return;               // Only tile drops allowed
    if (pdi.GetSubInfo<DRAG_TILE>().m_gamDoc != &GetDocument())
        return;               // Only tiles from our document.

    // if tile can't fit on board, reject drop
    wxSize limit = CB::Convert(m_pBoard->GetSize(fullScale));
    if (pdi.GetSubInfo<DRAG_TILE>().m_size.x > limit.x ||
        pdi.GetSubInfo<DRAG_TILE>().m_size.y > limit.y)
    {
        if (pdi.m_phase == PhaseDrag::Over)
        {
            event.SetCursor(g_res.hcrNoDropTooBigWx);
        }
        return;
    }

    if (pdi.m_phase == PhaseDrag::Over)
    {
        event.SetCursor(pdi.m_hcsrSuggest);
    }
    else if (pdi.m_phase == PhaseDrag::Drop)
    {
        CDrawList* pDwg;
        // Process a tile drop....
        wxPoint pnt(pdi.m_point);
        ClientToWorkspace(pnt);
        switch (m_pBoard->GetMaxDrawLayer())
        {
            case LAYER_BASE:
                pDwg = m_pBoard->GetBaseDrawing(TRUE);
                SetDrawingTile(CheckedDeref(pDwg), pdi.GetSubInfo<DRAG_TILE>().m_tileID, pnt, TRUE);
                break;
            case LAYER_GRID:
                SetCellTile(pdi.GetSubInfo<DRAG_TILE>().m_tileID, pnt, TRUE);
                break;
            case LAYER_TOP:
                pDwg = m_pBoard->GetTopDrawing(TRUE);
                SetDrawingTile(CheckedDeref(pDwg), pdi.GetSubInfo<DRAG_TILE>().m_tileID, pnt, TRUE);
                break;
            default: ;
        }
    }
}

void CBrdEditView::SetDrawingTile(CDrawList& pDwg, TileID tid, wxPoint pnt,
    BOOL bUpdate)
{
    CTileManager* pTMgr = GetDocument().GetTileManager();
    {
        OwnerPtr<CTileImage> pTileImage(MakeOwner<CTileImage>(pTMgr));

        // Center the image on the drop point.
        CTile tile = pTMgr->GetTile(tid);
        wxRect rct(pnt, CB::Convert(tile.GetSize()));
        rct.Offset(-rct.GetWidth() / 2, -rct.GetHeight() / 2);
        AdjustRect(rct);

        pTileImage->SetTile(rct.GetLeft(), rct.GetTop(), tid);

        pDwg.AddToFront(std::move(pTileImage));
    }
    if (bUpdate)
    {
        wxRect rct = CB::Convert(pDwg.Front().GetEnclosingRect());   // In board coords.
        InvalidateWorkspaceRect(rct);
    }
    GetDocument().SetModifiedFlag();
}

void CBrdEditView::SetCellTile(TileID tid, wxPoint pnt, BOOL bUpdate)
{
    size_t row, col;

    CBoardArray& pBa = m_pBoard->GetBoardArray();

    WorkspaceToClient(pnt);
    pnt = CalcUnscrolledPosition(pnt);

    if (!pBa.FindCell(pnt.x, pnt.y, row, col, m_nZoom))
        return;                                 // Not a valid cell hit
    if (tid != pBa.GetCellTile(value_preserving_cast<size_t>(row), value_preserving_cast<size_t>(col)))
    {
        pBa.SetCellTile(row, col, tid);
        if (bUpdate)
        {
            wxRect rct = CB::Convert(pBa.GetCellRect(row, col, m_nZoom));// In board coords
            rct.SetTopLeft(CalcScrolledPosition(rct.GetTopLeft()));
            RefreshRect(rct, FALSE);
        }
        GetDocument().SetModifiedFlag();
    }
}

void CBrdEditView::SetCellColor(wxColour crCell, wxPoint pnt, BOOL bUpdate)
{
    size_t row, col;

    CBoardArray& pBa = m_pBoard->GetBoardArray();
    WorkspaceToClient(pnt);
    pnt = CalcUnscrolledPosition(pnt);

    if (!pBa.FindCell(pnt.x, pnt.y, row, col, m_nZoom))
        return;                                 // Not a valid cell hit
    if (crCell != pBa.GetCellColor(row, col))
    {
        pBa.SetCellColor(row, col, CB::Convert(crCell));
        if (bUpdate)
        {
            wxRect rct = CB::Convert(pBa.GetCellRect(row, col, m_nZoom));// In board coords
            rct.SetTopLeft(CalcScrolledPosition(rct.GetTopLeft()));
            RefreshRect(rct, FALSE);
        }
        GetDocument().SetModifiedFlag();
    }
}

void CBrdEditView::SetBoardBackColor(wxColour cr, BOOL bUpdate)
{
    m_pBoard->SetBkColor(m_pBMgr->GetForeColor());
    if (bUpdate)
        Refresh();
    GetDocument().SetModifiedFlag();
}

CDrawList* CBrdEditView::GetDrawList(BOOL bCanCreateList)
{
    wxASSERT(m_pBoard);

    if (m_pBoard->GetMaxDrawLayer() == LAYER_TOP)
        return m_pBoard->GetTopDrawing(bCanCreateList);
    else if (m_pBoard->GetMaxDrawLayer() == LAYER_BASE)
        return m_pBoard->GetBaseDrawing(bCanCreateList);
    else
        return NULL;
}

void CBrdEditView::AddDrawObject(CDrawObj::OwnerPtr pObj)
{
    CDrawList* pDwg = GetDrawList(TRUE);
    if (pDwg != NULL)
    {
        pDwg->AddToFront(std::move(pObj));
        GetDocument().SetModifiedFlag();
    }
}

void CBrdEditView::DeleteDrawObject(CDrawObj::OwnerPtr pObj)
{
    CDrawList* pDwg = NULL;

    if (m_pBoard->GetMaxDrawLayer() == LAYER_TOP)
        pDwg = m_pBoard->GetTopDrawing();
    else if (m_pBoard->GetMaxDrawLayer() == LAYER_BASE)
        pDwg = m_pBoard->GetBaseDrawing();

    if (pDwg != NULL)
    {
        pDwg->RemoveObject(*pObj);
        GetDocument().SetModifiedFlag();
    }
}

void CBrdEditView::DoCreateTextDrawingObject(wxPoint point)
{
    if (m_pBoard->GetMaxDrawLayer() == LAYER_GRID)
        return;                     // No text here!

    CTextObjDialog dlg;
    dlg.m_pFontMgr = CGamDoc::GetFontManager();
    dlg.SetFontID(m_pBMgr->GetFontID());

    if (dlg.ShowModal() == wxID_OK)
    {
        if (!dlg.m_strText.empty())
        {
            CreateTextDrawingObject(point, dlg.m_fontID,
                m_pBMgr->GetForeColor(), dlg.m_strText, TRUE);
            GetDocument().SetModifiedFlag();
        }
    }
}

void CBrdEditView::DoEditTextDrawingObject(CText& pDObj)
{
    CTextObjDialog dlg;
    dlg.m_strText = wxString(pDObj.m_text);
    dlg.m_pFontMgr = CGamDoc::GetFontManager();
    dlg.SetFontID(pDObj.m_fontID);

    if (dlg.ShowModal() == wxID_OK)
    {
        if (!dlg.m_strText.empty())
        {
            wxRect rct = CB::Convert(pDObj.GetEnclosingRect());
            InvalidateWorkspaceRect(rct);

            pDObj.m_text = dlg.m_strText;
            pDObj.SetFont(dlg.m_fontID);   // Also resyncs the extent rect

            rct = CB::Convert(pDObj.GetEnclosingRect());
            InvalidateWorkspaceRect(rct);
            GetDocument().SetModifiedFlag();
        }
    }
}

void CBrdEditView::CreateTextDrawingObject(wxPoint pnt, FontID fid,
    wxColour crText, const CB::string& strText, BOOL bInvalidate /* = TRUE */)
{
    {
        OwnerPtr<CText> pText = MakeOwner<CText>();
        AdjustPoint(pnt);
        pText->SetText(pnt.x, pnt.y, strText, fid, CB::Convert(crText));

        wxRect rct = CB::Convert(pText->GetRect());
        AdjustRect(rct);
        pText->SetRect(CB::Convert(rct));

        GetDrawList(TRUE)->AddToFront(std::move(pText));
    }
    GetDocument().SetModifiedFlag();
    if (bInvalidate)
    {
        wxRect rct = CB::Convert(GetDrawList(TRUE)->Front().GetEnclosingRect());
        InvalidateWorkspaceRect(rct);
    }
}

//////////////////////////////////////////////////////////////////////

BOOL CBrdEditView::IsGridizeActive() const
{
    BOOL bGridSnap = m_pBoard->m_bGridSnap;
    int bControl = GetAsyncKeyState(VK_CONTROL) < 0;
    return !(bControl && bGridSnap || !bControl && !bGridSnap);
}

void CBrdEditView::GridizeX(decltype(wxPoint::x)& xPos) const
{
    if (IsGridizeActive())
    {
        xPos = GridizeClosest1000(value_preserving_cast<int32_t>(xPos),
            value_preserving_cast<int32_t>(m_pBoard->m_xGridSnap), value_preserving_cast<int32_t>(m_pBoard->m_xGridSnapOff));
    }
}

void CBrdEditView::GridizeY(decltype(wxPoint::y)& yPos) const
{
    if (IsGridizeActive())
    {
        yPos = GridizeClosest1000(value_preserving_cast<int32_t>(yPos),
            value_preserving_cast<int32_t>(m_pBoard->m_yGridSnap), value_preserving_cast<int32_t>(m_pBoard->m_yGridSnapOff));
    }
}

void CBrdEditView::LimitPoint(wxPoint& pPnt) const
{
    if (pPnt.x < 0) pPnt.x = 0;
    if (pPnt.x > m_pBoard->GetWidth(fullScale))
        pPnt.x = m_pBoard->GetWidth(fullScale);
    if (pPnt.y < 0) pPnt.y = 0;
    if (pPnt.y > m_pBoard->GetHeight(fullScale))
        pPnt.y = m_pBoard->GetHeight(fullScale);

#ifdef WE_WANT_THIS_CRAP_940130
    CSize size = m_pBoard->GetSize(m_nZoom);
    PixelToWorkspace((CPoint)size);

    if (pPnt->x < 0) pPnt->x = 0;
    if (pPnt->x > size.cx)
        pPnt->x = size.cx;
    if (pPnt->y < 0) pPnt->y = 0;
    if (pPnt->y > size.cy)
        pPnt->y = size.cy;
#endif
}

void CBrdEditView::LimitRect(wxRect& pRct) const
{
    wxRect rct(pRct);
    if (rct.GetLeft() < 0)
        rct.Offset(-rct.GetLeft(), 0);
    if (rct.GetTop() < 0)
        rct.Offset(0, -rct.GetTop());
    if (rct.GetRight() > m_pBoard->GetWidth(fullScale))
        rct.Offset(m_pBoard->GetWidth(fullScale) - rct.GetRight(), 0);
    if (rct.GetBottom() > m_pBoard->GetHeight(fullScale))
        rct.Offset(0, m_pBoard->GetHeight(fullScale) - rct.GetBottom());
    pRct = rct;
#ifdef WANT_THIS_CRAP_940130
    CSize size = m_pBoard->GetSize(m_nZoom);
    PixelToWorkspace((CPoint)size);

    CRect rct(pRct);
    if (rct.left < 0)
        rct.OffsetRect(-rct.left, 0);
    if (rct.top < 0)
        rct.OffsetRect(0, -rct.top);
    if (rct.right > size.cx)
        rct.OffsetRect(size.cx - rct.right, 0);
    if (rct.bottom > size.cy)
        rct.OffsetRect(0, size.cy - rct.bottom);
    *pRct = rct;
#endif
}

void CBrdEditView::AdjustPoint(wxPoint& pnt) const
{
    GridizeX(pnt.x);
    GridizeY(pnt.y);
    LimitPoint(pnt);
}

void CBrdEditView::AdjustRect(wxRect& rct) const
{
    wxPoint pnt = rct.GetTopLeft();
    GridizeX(pnt.x);
    GridizeY(pnt.y);
    LimitPoint(pnt);
    if (pnt != rct.GetTopLeft())
        rct.Offset(pnt - rct.GetTopLeft());
    LimitRect(rct);
}

#if 0
void CBrdEditView::PixelToWorkspace(CPoint& point) const
{
    CSize wsize, vsize;
    m_pBoard->GetBoardArray().GetBoardScaling(m_nZoom, wsize, vsize);
    ScalePoint(point, wsize, vsize);
}
#endif

//////////////////////////////////////////////////////////////////////

void CBrdEditView::OnUpdateColorForeground(wxUpdateUIEvent& pCmdUI)
{
    CCmdUI* cmdui = static_cast<CCmdUI*>(pCmdUI.GetClientData());
    CColorCmdUI& colorCmdUI = CheckedDeref(dynamic_cast<CColorCmdUI*>(cmdui));
    colorCmdUI.SetColor(CB::Convert(m_pBMgr->GetForeColor()));
}

void CBrdEditView::OnUpdateColorBackground(wxUpdateUIEvent& pCmdUI)
{
    CCmdUI* cmdui = static_cast<CCmdUI*>(pCmdUI.GetClientData());
    CColorCmdUI& colorCmdUI = CheckedDeref(dynamic_cast<CColorCmdUI*>(cmdui));
    colorCmdUI.SetColor(CB::Convert(m_pBMgr->GetBackColor()));
}

void CBrdEditView::OnUpdateColorCustom(wxUpdateUIEvent& pCmdUI)
{
    CCmdUI* cmdui = static_cast<CCmdUI*>(pCmdUI.GetClientData());
    CColorCmdUI& colorCmdUI = CheckedDeref(dynamic_cast<CColorCmdUI*>(cmdui));
    colorCmdUI.SetCustomColors(GetDocument().GetCustomColors());
}

void CBrdEditView::OnUpdateLineWidth(wxUpdateUIEvent& pCmdUI)
{
    CCmdUI* cmdui = static_cast<CCmdUI*>(pCmdUI.GetClientData());
    CColorCmdUI& colorCmdUI = CheckedDeref(dynamic_cast<CColorCmdUI*>(cmdui));
    colorCmdUI.SetLineWidth(value_preserving_cast<UINT>(m_pBMgr->GetLineWidth()));
}

//////////////////////////////////////////////////////////////////////


void CBrdEditView::OnSetColor(SetColorEvent& event)
{
    struct ColorSetting
    {
        COLORREF m_cr;              // Color to set
        BOOL     m_bColorAccepted;  // TRUE if at least one object accepted the color
    };
    ColorSetting cset = { RGB(0,0,0), FALSE };

    if (event.GetSubId() == XRCID("ID_COLOR_FOREGROUND"))
    {
        m_pBMgr->SetForeColor(CB::Convert(event.GetColor()));
        cset.m_cr = m_pBMgr->GetForeColor();
        m_selList.ForAllSelections([&cset](CDrawObj& pObj) { cset.m_bColorAccepted |= pObj.SetForeColor(cset.m_cr); });
        m_selList.UpdateObjects();
    }
    else if (event.GetSubId() == XRCID("ID_COLOR_BACKGROUND"))
    {
        m_pBMgr->SetBackColor(CB::Convert(event.GetColor()));
        cset.m_cr = m_pBMgr->GetBackColor();
        m_selList.ForAllSelections([&cset](CDrawObj& pObj) { cset.m_bColorAccepted |= pObj.SetBackColor(cset.m_cr); });
        m_selList.UpdateObjects();
    }
    else
    {
        event.Skip();
        return;
    }
    if (cset.m_bColorAccepted)
        GetDocument().SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////

void CBrdEditView::OnSetCustomColors(SetCustomColorEvent& event)
{
    const std::vector<wxColour>& pCustomColors = event.GetColors();
    GetDocument().SetCustomColors(pCustomColors);
}

//////////////////////////////////////////////////////////////////////

void CBrdEditView::OnSetLineWidth(SetLineWidthEvent& event)
{
    struct WidthSetting
    {
        UINT m_nWidth;          // Width to set
        BOOL m_bWidthAccepted;  // TRUE if at least one object accepted the color
    };
    WidthSetting wset = { 1, FALSE };

    m_pBMgr->SetLineWidth(event.GetWidth());
    m_selList.UpdateObjects();
    wset.m_nWidth = m_pBMgr->GetLineWidth();
    m_selList.ForAllSelections([&wset](CDrawObj& pObj) { wset.m_bWidthAccepted |= pObj.SetLineWidth(wset.m_nWidth); });
    m_selList.UpdateObjects(TRUE, FALSE);
    if (wset.m_bWidthAccepted)
        GetDocument().SetModifiedFlag();
}

// ------------------------------------------------------ //

#if 0
void CBrdEditView::OnDwgFont()
{
    if (m_pBMgr->DoBoardFontDialog())
    {
        m_selList.UpdateObjects();
        FontID fontID = m_pBMgr->GetFontID();
        BOOL fontAccepted = false;
        m_selList.ForAllSelections([&fontID, &fontAccepted](CDrawObj& pObj) { fontAccepted |= pObj.SetFont(fontID); });
        m_selList.UpdateObjects(TRUE, FALSE);
        if (fontAccepted)
        {
            GetDocument().SetModifiedFlag();
        }
    }
}
#endif

// ------------------------------------------------------ //

void CBrdEditView::OnToolPalette(wxCommandEvent& event)
{
    if (event.GetId() != XRCID("ID_TOOL_ARROW"))
        m_selList.PurgeList(TRUE);
    if (event.GetId() != m_nCurToolID)
    {
        if (m_nCurToolID == XRCID("ID_TOOL_POLYGON"))
        {
            // If we're changing away from the polygon tool
            // we need to act as if the escape key was hit.
            ToolType eToolType = MapToolType(m_nCurToolID);
            CTool& pTool = CTool::GetTool(eToolType);
            if (pTool.m_eToolType == ttypePolygon)
            {
                CPolyTool& pPolyTool = static_cast<CPolyTool&>(pTool);
                pPolyTool.RemoveRubberBand(*this);
                pPolyTool.FinalizePolygon(*this, TRUE);
            }
        }
        m_nLastToolID = m_nCurToolID;
        m_nCurToolID = event.GetId();
    }
}

// ---------------------------------------------------- //

void CBrdEditView::OnUpdateToolPalette(wxUpdateUIEvent& pCmdUI)
{
    TileID tid;
    int iLayer = m_pBoard->GetMaxDrawLayer();
    BOOL bEnable;

    if (pCmdUI.GetId() == XRCID("ID_TOOL_TILE"))
    {
        tid = GetDocument().GetTilePalWnd()->GetCurrentTileID();
        bEnable = tid != nullTid;
        if (tid == nullTid && m_nCurToolID == XRCID("ID_TOOL_TILE"))
        {
            m_nCurToolID = XRCID("ID_TOOL_ARROW");
            m_nLastToolID = XRCID("ID_TOOL_ARROW");
        }
    }
    else if (pCmdUI.GetId() == XRCID("ID_TOOL_ARROW") ||
            pCmdUI.GetId() == XRCID("ID_TOOL_DROPPER"))
    {
        bEnable = TRUE;
    }
    else if (pCmdUI.GetId() == XRCID("ID_TOOL_FILL") ||
            pCmdUI.GetId() == XRCID("ID_TOOL_ERASER"))
    {
        bEnable = iLayer == LAYER_BASE || iLayer == LAYER_GRID;
    }
    else if (pCmdUI.GetId() == XRCID("ID_TOOL_TEXT") ||
            pCmdUI.GetId() == XRCID("ID_TOOL_LINE") ||
            pCmdUI.GetId() == XRCID("ID_TOOL_POLYGON") ||
            pCmdUI.GetId() == XRCID("ID_TOOL_RECT") ||
            pCmdUI.GetId() == XRCID("ID_TOOL_OVAL"))
    {
        bEnable = iLayer == LAYER_BASE || iLayer == LAYER_TOP;
    }
    else
    {
        ASSERT(!"unexpected tool id");
        bEnable = false;
    }

    pCmdUI.Enable(bEnable);
    pCmdUI.Check(pCmdUI.GetId()  == m_nCurToolID);
}

#if 0
void CBrdEditView::OnUpdateDwgToFrontOrBack(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!m_selList.empty());
}

void CBrdEditView::OnDwgToBack()
{
    MoveObjsInSelectList(FALSE, TRUE);
}

void CBrdEditView::OnDwgToFront()
{
    MoveObjsInSelectList(TRUE, TRUE);
}

void CBrdEditView::OnUpdateDwgDrawAboveGrid(CCmdUI* pCmdUI)
{
    if (m_pBoard->GetMaxDrawLayer() == LAYER_TOP && !m_selList.empty())
    {
        pCmdUI->Enable(TRUE);
        if (m_selList.IsDObjFlagSetInAllSelectedObjects(dobjFlgDrawPass2))
            pCmdUI->SetCheck(1);
        else if (m_selList.IsDObjFlagSetInSomeSelectedObjects(dobjFlgDrawPass2))
            pCmdUI->SetCheck(2);
        else
            pCmdUI->SetCheck(0);
    }
    else
    {
        pCmdUI->Enable(FALSE);
        pCmdUI->SetCheck(0);
    }

}

void CBrdEditView::OnDwgDrawAboveGrid()
{
    ASSERT(!m_selList.empty());
    if (m_selList.IsDObjFlagSetInAllSelectedObjects(dobjFlgDrawPass2))
        m_selList.ClearDObjFlagInAllSelectedObjects(dobjFlgDrawPass2);
    else
        m_selList.SetDObjFlagInAllSelectedObjects(dobjFlgDrawPass2);
    m_selList.InvalidateList(TRUE);
}
#endif

void CBrdEditView::CenterViewOnWorkspacePoint(wxPoint point)
{
    WorkspaceToClient(point);
    wxRect rct = GetClientRect();
    wxPoint pt = GetMidRect(rct);
    wxPoint size = point - pt;
    wxPoint newUpLeft = CalcUnscrolledPosition(size);
    // If the axis being scrolled is entirely visible then set
    // that scroll position to zero.
    wxSize sizeTotal = GetVirtualSize();   // Logical is in device units for us
    if (rct.GetWidth() >= sizeTotal.x)
        newUpLeft.x = 0;
    if (rct.GetHeight() >= sizeTotal.y)
        newUpLeft.y = 0;
    Scroll(newUpLeft);
    Update();
}

void CBrdEditView::DoViewScale(TileScale nZoom)
{
    wxRect rctClient = GetClientRect();
    wxPoint pntMid = GetMidRect(rctClient);
    ClientToWorkspace(pntMid);

    m_nZoom = nZoom;
    RecalcScrollLimits();
    wxBusyCursor busyCursor;
    Refresh(false);
    CenterViewOnWorkspacePoint(pntMid);
}

void CBrdEditView::OnViewFullScale(wxCommandEvent& /*event*/)
{
    DoViewScale(fullScale);
}

void CBrdEditView::OnUpdateViewFullScale(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
    pCmdUI.Check(m_nZoom == fullScale);
}

void CBrdEditView::OnViewHalfScale(wxCommandEvent& /*event*/)
{
    DoViewScale(halfScale);
}

void CBrdEditView::OnUpdateViewHalfScale(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
    pCmdUI.Check(m_nZoom == halfScale);
}

void CBrdEditView::OnViewSmallScale(wxCommandEvent& /*event*/)
{
    DoViewScale(smallScale);
}

void CBrdEditView::OnUpdateViewSmallScale(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
    pCmdUI.Check(m_nZoom == smallScale);
}

void CBrdEditView::OnViewToggleScale(wxCommandEvent& /*event*/)
{
    if (m_nZoom == fullScale)
         DoViewScale(halfScale);
    else if (m_nZoom == halfScale)
         DoViewScale(smallScale);
    else // smallScale
         DoViewScale(fullScale);
}

void CBrdEditView::OnUpdateEnable(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
}

#if 0
void CBrdEditView::OnUpdateIndicatorCellNum(CCmdUI* pCmdUI)
{
    CBoardArray& pba = m_pBoard->GetBoardArray();
    if (pba.GetCellNumTracking())
    {
        CPoint point;
        GetCursorPos(&point);
        ScreenToClient(&point);
        CRect rct;
        GetClientRect(&rct);
        if (rct.PtInRect(point))
        {
            point = CalcUnscrolledPosition(point);
            CB::string str = pba.GetCellNumberStr(point, m_nZoom);
            pCmdUI->Enable();
            pCmdUI->SetText(str);
        }
    }
}

void CBrdEditView::OnToolsBrdSnapGrid()
{
    m_pBoard->m_bGridSnap = !m_pBoard->m_bGridSnap;
}

void CBrdEditView::OnUpdateToolsBrdSnapGrid(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(TRUE);
    pCmdUI->SetCheck(m_pBoard->m_bGridSnap);
}

void CBrdEditView::OnToolsBrdProps()
{
    GetDocument().DoBoardPropertyDialog(*m_pBoard);
}

void CBrdEditView::OnToolSetVisibleScale()
{
    CSetScaleVisibilityDialog dlg;
    dlg.m_bFullScale = TRUE;
    dlg.m_bHalfScale = TRUE;
    dlg.m_bSmallScale = TRUE;
    dlg.m_bNaturalScale = FALSE;

    if (dlg.ShowModal() == wxID_OK)
    {
        int mask =
            (dlg.m_bFullScale ? fullScale : 0) |
            (dlg.m_bHalfScale ? halfScale : 0) |
            (dlg.m_bSmallScale ? smallScale : 0);

        m_selList.ForAllSelections([mask](CDrawObj& pObj) { pObj.SetScaleVisibility(mask); });
        if (dlg.m_bNaturalScale)
            m_selList.ForAllSelections([](CDrawObj& pObj) { pObj.SetDObjFlags(dobjFlgLayerNatural); });
        else
            m_selList.ForAllSelections([](CDrawObj& pObj) { pObj.ClearDObjFlags(dobjFlgLayerNatural); });

        m_selList.UpdateObjects(TRUE, FALSE);
    }
}

void CBrdEditView::OnUpdateToolSetVisibleScale(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_selList.IsAnySelects());
}

void CBrdEditView::OnToolSuspendScaleVisibility()
{
    m_pBoard->SetApplyVisible(!m_pBoard->GetApplyVisible());
    Invalidate(FALSE);
}

void CBrdEditView::OnUpdateToolSuspendScaleVsibility(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(!m_pBoard->GetApplyVisible());
}

void CBrdEditView::OnEditPaste()
{
    OwnerPtr<CBitmap> pBMap = GetClipboardBitmap(this);

    {
        OwnerPtr<CBitmapImage> pDObj = MakeOwner<CBitmapImage>();
        pDObj->SetBitmap(0, 0, (HBITMAP)pBMap->Detach(), fullScale);

        GetSelectList().PurgeList(TRUE);           // Clear current select list
        AddDrawObject(std::move(pDObj));
    }
    CDrawObj& pDObj = GetDrawList(FALSE)->Front();
    GetSelectList().AddObject(pDObj, TRUE);
    CRect rct = pDObj.GetEnclosingRect();
    InvalidateWorkspaceRect(rct);
}

void CBrdEditView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
    int nMaxLayer = m_pBoard->GetMaxDrawLayer();
    pCmdUI->Enable(IsClipboardBitmap() &&
        (nMaxLayer < 0 || nMaxLayer == LAYER_BASE || nMaxLayer == LAYER_TOP));
}

void CBrdEditView::OnEditCopy()
{
    GetSelectList().CopyToClipboard();
}

void CBrdEditView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetSelectList().IsCopyToClipboardPossible());
}

void CBrdEditView::OnEditPasteBitmapFromFile()
{
    CB::string strFilter = CB::string::LoadString(IDS_BMP_FILTER);
    CB::string strTitle = CB::string::LoadString(IDS_SEL_BITMAPFILE);

    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,
        strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return;

    try
    {
        wxImage img(CB::string(dlg.GetPathName()));
        OwnerPtr<CBitmap> pBMap = ToBitmap(img);

        OwnerPtr<CBitmapImage> pDObj = MakeOwner<CBitmapImage>();
        pDObj->SetBitmap(0, 0, (HBITMAP)pBMap->Detach(), fullScale);

        GetSelectList().PurgeList(TRUE);           // Clear current select list
        AddDrawObject(std::move(pDObj));
    }
    catch (...)
    {
        AfxMessageBox(IDP_ERR_LOADBITMAP, MB_ICONEXCLAMATION);
        return;
    }
    CDrawObj& pDObj = GetDrawList(FALSE)->Front();
    GetSelectList().AddObject(pDObj, TRUE);
    CRect rct = pDObj.GetEnclosingRect();
    InvalidateWorkspaceRect(&rct);
}

void CBrdEditView::OnUpdateEditPasteBitmapFromFile(CCmdUI* pCmdUI)
{
    int nMaxLayer = m_pBoard->GetMaxDrawLayer();
    pCmdUI->Enable(nMaxLayer < 0 || nMaxLayer == LAYER_BASE ||
        nMaxLayer == LAYER_TOP);
}

//DFM 19991014...
//GetKeyState()
static const short KEY_STATE_VALUE = static_cast<short>(static_cast<unsigned short>(0x8000));

void CBrdEditView::HandleKeyDown ()
{
    int modifierState = 4 * (GetKeyState (VK_SHIFT) & KEY_STATE_VALUE) +
                        2 * (GetKeyState (VK_CONTROL) & KEY_STATE_VALUE) +
                            (GetKeyState (VK_MENU) & KEY_STATE_VALUE);
    modifierState /= KEY_STATE_VALUE;

    switch (modifierState)
    {
        case 0:
            NudgeDown ();
            break;
        case 1:
            FastNudgeDown ();
            break;
        case 2:
            NudgeScrollDown ();
            break;
        case 3:
            FastNudgeScrollDown ();
            break;
        case 4:
            ScrollDown ();
            break;
        case 5:
            FastScrollDown ();
            break;
        case 6:
            break;
        case 7:
            break;
    }
}

void CBrdEditView::HandleKeyUp ()
{
    int modifierState = 4 * (GetKeyState (VK_SHIFT) & KEY_STATE_VALUE) +
                        2 * (GetKeyState (VK_CONTROL) & KEY_STATE_VALUE) +
                            (GetKeyState (VK_MENU) & KEY_STATE_VALUE);
    modifierState /= KEY_STATE_VALUE;

    switch (modifierState)
    {
        case 0:
            NudgeUp ();
            break;
        case 1:
            FastNudgeUp ();
            break;
        case 2:
            NudgeScrollUp ();
            break;
        case 3:
            FastNudgeScrollUp ();
            break;
        case 4:
            ScrollUp ();
            break;
        case 5:
            FastScrollUp ();
            break;
        case 6:
            break;
        case 7:
            break;
    }
}

void CBrdEditView::HandleKeyLeft ()
{
    int modifierState = 4 * (GetKeyState (VK_SHIFT) & KEY_STATE_VALUE) +
                        2 * (GetKeyState (VK_CONTROL) & KEY_STATE_VALUE) +
                            (GetKeyState (VK_MENU) & KEY_STATE_VALUE);
    modifierState /= KEY_STATE_VALUE;

    switch (modifierState)
    {
        case 0:
            NudgeLeft ();
            break;
        case 1:
            FastNudgeLeft ();
            break;
        case 2:
            NudgeScrollLeft ();
            break;
        case 3:
            FastNudgeScrollLeft ();
            break;
        case 4:
            ScrollLeft ();
            break;
        case 5:
            FastScrollLeft ();
            break;
        case 6:
            break;
        case 7:
            break;
    }
}

void CBrdEditView::HandleKeyRight ()
{
    int modifierState = 4 * (GetKeyState (VK_SHIFT) & KEY_STATE_VALUE) +
                        2 * (GetKeyState (VK_CONTROL) & KEY_STATE_VALUE) +
                            (GetKeyState (VK_MENU) & KEY_STATE_VALUE);
    modifierState /= KEY_STATE_VALUE;

    switch (modifierState)
    {
        case 0:
            NudgeRight ();
            break;
        case 1:
            FastNudgeRight ();
            break;
        case 2:
            NudgeScrollRight ();
            break;
        case 3:
            FastNudgeScrollRight ();
            break;
        case 4:
            ScrollRight ();
            break;
        case 5:
            FastScrollRight ();
            break;
        case 6:
            break;
        case 7:
            break;
    }
}

void CBrdEditView::HandleKeyPageUp ()
{
    if ((GetKeyState (VK_CONTROL) & 0x8000) != 0)
    {
        PageLeft ();
    }
    else
    {
        PageUp ();
    }
}

void CBrdEditView::HandleKeyPageDown ()
{
    if ((GetKeyState (VK_CONTROL) & 0x8000) != 0)
    {
        PageRight ();
    }
    else
    {
        PageDown ();
    }
}

void CBrdEditView::HandleKeyTop ()
{
    if ((GetKeyState (VK_CONTROL) & 0x8000) != 0)
    {
        PageFarLeft ();
    }
    else
    {
        PageTop ();
    }
}

void CBrdEditView::HandleKeyBottom ()
{
    if ((GetKeyState (VK_CONTROL) & 0x8000) != 0)
    {
        PageFarRight ();
    }
    else
    {
        PageBottom ();
    }
}

void CBrdEditView::ScrollDown()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.y += 5;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::ScrollUp()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.y -= 5;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::ScrollLeft()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.x -= 5;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::ScrollRight()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.x += 5;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::FastScrollDown()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.y += 25;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::FastScrollUp()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.y -= 25;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::FastScrollLeft()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.x -= 25;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::FastScrollRight()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.x += 25;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::PageDown()
{
    CRect windowSize;
    GetClientRect(windowSize);

    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.y += windowSize.Height();
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::PageUp()
{
    CRect windowSize;
    GetClientRect(windowSize);

    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.y -= windowSize.Height();
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::PageLeft()
{
    CRect windowSize;
    GetClientRect(windowSize);

    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.x -= windowSize.Width();
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::PageRight()
{
    CRect windowSize;
    GetClientRect(windowSize);

    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.x += windowSize.Width();
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::PageBottom()
{
    CSize boardSize = GetTotalSize();

    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.y = boardSize.cy;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::PageTop()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.y = 0;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::PageFarLeft()
{
    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.x = 0;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::PageFarRight()
{
    CSize boardSize = GetTotalSize();

    CPoint newUpLeft = GetScrollPosition();
    newUpLeft.x += boardSize.cx;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::NudgeDown()
{
    NudgeObjsInSelectList(0,1, false);
}

void CBrdEditView::NudgeUp()
{
    NudgeObjsInSelectList(0,-1, false);
}

void CBrdEditView::NudgeLeft()
{
    NudgeObjsInSelectList(-1, 0, false);
}

void CBrdEditView::NudgeRight()
{
    NudgeObjsInSelectList(1, 0, false);
}

void CBrdEditView::FastNudgeDown()
{
    NudgeObjsInSelectList(0,5, false);
}

void CBrdEditView::FastNudgeUp()
{
    NudgeObjsInSelectList(0,-5, false);
}

void CBrdEditView::FastNudgeLeft()
{
    NudgeObjsInSelectList(-5, 0, false);
}

void CBrdEditView::FastNudgeRight()
{
    NudgeObjsInSelectList(5, 0, false);
}

void CBrdEditView::NudgeScrollDown()
{
    NudgeObjsInSelectList(0,1, true);
}

void CBrdEditView::NudgeScrollUp()
{
    NudgeObjsInSelectList(0,-1, true);
}

void CBrdEditView::NudgeScrollLeft()
{
    NudgeObjsInSelectList(-1, 0, true);
}

void CBrdEditView::NudgeScrollRight()
{
    NudgeObjsInSelectList(1, 0, true);
}

void CBrdEditView::FastNudgeScrollDown()
{
    NudgeObjsInSelectList(0,5, true);
}

void CBrdEditView::FastNudgeScrollUp()
{
    NudgeObjsInSelectList(0,-5, true);
}

void CBrdEditView::FastNudgeScrollLeft()
{
    NudgeObjsInSelectList(-5, 0, true);
}

void CBrdEditView::FastNudgeScrollRight()
{
    NudgeObjsInSelectList(5, 0, true);
}

void CBrdEditView::NudgeObjsInSelectList(int dX, int dY, BOOL forceScroll)
{
    if (!m_selList.IsAnySelects())
    {
        return;
    }

    //----------------------------------------------------------------
    //  Figure out how far we're going to move the objects
    //----------------------------------------------------------------
    if (IsGridizeActive())
    {
        //  get the approximate snap and adjust later
        dX *= value_preserving_cast<int>(m_pBoard->m_xGridSnap) / 1000;
        dY *= value_preserving_cast<int>(m_pBoard->m_yGridSnap) / 1000;
    }

    CPoint offset (dX, dY);

    //---------------------------------------------------------------
    //  First we are going to make sure that the region spanned by
    //  the selected items isn't going to bump into the edge of the
    //  board
    //---------------------------------------------------------------
    CSelList::iterator pos = m_selList.begin();
    CSelection& pSel = **pos;
    CRect selectRegion(pSel.m_rect);
    for ( ; pos != m_selList.end() ; ++pos)
    {
        CSelection& pSel = **pos;
        selectRegion.UnionRect (pSel.m_rect,selectRegion);
    }

    AdjustRect (selectRegion);
    CRect originalRect (selectRegion);
    selectRegion.OffsetRect(offset);
    AdjustRect (selectRegion);

    if (selectRegion == originalRect)
    {
        return;
    }

    //----------------------------------------------------------------
    //  Now we actually move the selected items.
    //----------------------------------------------------------------
    m_selList.InvalidateList();
    m_selList.InvalidateListHandles();

    for (pos = m_selList.begin() ; pos != m_selList.end() ; ++pos)
    {
        CSelection& pSel = **pos;

        pSel.Offset(offset);
        AdjustRect(pSel.m_rect);

        pSel.m_pObj->OffsetObject(offset);
        CRect temp = pSel.m_pObj->GetRect();
        AdjustRect(temp);
        pSel.m_pObj->SetRect(temp);
    }

    m_selList.InvalidateList();
    m_selList.InvalidateListHandles();
    GetDocument().SetModifiedFlag();

    //----------------------------------------------------------------
    //  Now we're going to decide if we want to scroll the window
    //  to keep objects in view.
    //----------------------------------------------------------------
    BOOL doScroll = forceScroll;
    CPoint scrollPos = GetScrollPosition();

    if (!doScroll)
    {
        CRect windowSize;
        GetClientRect(windowSize);
        windowSize.OffsetRect(scrollPos);

        CRect beforeRect;
        beforeRect.UnionRect(windowSize,originalRect);

        CRect afterRect;
        afterRect.UnionRect(windowSize,selectRegion);

        if (beforeRect != afterRect)
        {
            doScroll = true;
        }
    }

    if (doScroll)
    {
        scrollPos.Offset(offset);
        ScrollToPosition(scrollPos);
        UpdateWindow();
    }
}
//...DFM 19991014

/////////////////////////////////////////////////////////////////////////////
// Fix MFC problems with mouse wheel handling in Win98 and WinME systems

BOOL CBrdEditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    return DoMouseWheelFix(nFlags, zDelta, pt);
}

BOOL CBrdEditView::DoMouseWheelFix(UINT fFlags, short zDelta, CPoint point)
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
        static UINT uWheelScrollLines = UINT(-1);
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
        // Don't need the fix so call normal processing.
        return DoMouseWheel(fFlags, zDelta, point);
    }
}
#endif

/* This view should support scrolling by individual pixels,
    but don't make the line-up and line-down scrolling that
    slow.  */
void CBrdEditView::OnScrollWinLine(wxScrollWinEvent& event)
{
    int type = event.GetEventType();
    wxASSERT(type == wxEVT_SCROLLWIN_LINEDOWN ||
                type == wxEVT_SCROLLWIN_LINEUP);

    int orient = event.GetOrientation();
    wxASSERT(orient == wxHORIZONTAL || orient == wxVERTICAL);

    int oldPos;
    int offset;
    if (orient == wxHORIZONTAL)
    {
        oldPos = GetViewStart().x;
        offset = m_xScrollPixelsPerLine;
    }
    else
    {
        oldPos = GetViewStart().y;
        offset = m_yScrollPixelsPerLine;
    }
    if (type == wxEVT_SCROLLWIN_LINEUP)
    {
        offset = -offset;
    }

    wxScrollWinEvent thumbEvent(wxEVT_SCROLLWIN_THUMBTRACK, oldPos + offset, orient);
    ProcessWindowEvent(thumbEvent);
}

void CBrdEditView::RecalcScrollLimits()
{
    wxSizer& sizer = CheckedDeref(GetSizer());
    wxSizerItemList& items = sizer.GetChildren();
    wxASSERT(items.size() == 1);
    wxSizerItem& item = CheckedDeref(items[0]);
    wxASSERT(item.IsSpacer());

    item.AssignSpacer(CB::Convert(m_pBoard->GetSize(m_nZoom)));
    SetScrollRate(1, 1);
    sizer.FitInside(this);

    // use size of cell as line increment
    const CCellForm& cf = m_pBoard->GetBoardArray().GetCellForm(m_nZoom);
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

void CBrdEditViewContainer::OnDraw(CDC* /*pDC*/)
{
    // do nothing because child covers entire client rect
}

void CBrdEditViewContainer::OnInitialUpdate()
{
    child->OnInitialUpdate();
}

void CBrdEditViewContainer::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    child->OnUpdate(pSender, lHint, pHint);
}

CBrdEditViewContainer::CBrdEditViewContainer() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

int CBrdEditViewContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CB::OnCmdMsgOverride<CView>::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    child = new CBrdEditView(*this);

    return 0;
}


