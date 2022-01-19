// VwEdtbrd.cpp : implementation of the CBrdEditView class
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

IMPLEMENT_DYNCREATE(CBrdEditView, CScrollView)

BEGIN_MESSAGE_MAP(CBrdEditView, CScrollView)
    //{{AFX_MSG_MAP(CBrdEditView)
    ON_WM_MOUSEWHEEL()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_TIMER()
    ON_WM_KEYDOWN()
    ON_WM_CHAR()
    ON_WM_SETCURSOR()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragTileItem)
    ON_MESSAGE(WM_SETCOLOR, OnSetColor)
    ON_MESSAGE(WM_SETCUSTOMCOLOR, OnSetCustomColors)
    ON_MESSAGE(WM_SETLINEWIDTH, OnSetLineWidth)
    ON_COMMAND(ID_OFFSCREEN, OnOffscreen)
    ON_COMMAND(ID_VIEW_GRIDLINES, OnViewGridLines)
    ON_COMMAND(ID_DWG_TOBACK, OnDwgToBack)
    ON_COMMAND(ID_DWG_TOFRONT, OnDwgToFront)
    ON_COMMAND_EX(ID_TOOL_ARROW, OnToolPalette)
    ON_COMMAND_EX(ID_EDIT_LAYER_BASE, OnEditLayer)
    ON_UPDATE_COMMAND_UI(ID_VIEW_GRIDLINES, OnUpdateViewGridLines)
    ON_UPDATE_COMMAND_UI(ID_OFFSCREEN, OnUpdateOffscreen)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LAYER_BASE, OnUpdateEditLayer)
    ON_UPDATE_COMMAND_UI(ID_COLOR_FOREGROUND, OnUpdateColorForeground)
    ON_UPDATE_COMMAND_UI(ID_COLOR_BACKGROUND, OnUpdateColorBackground)
    ON_UPDATE_COMMAND_UI(ID_COLOR_CUSTOM, OnUpdateColorCustom)
    ON_UPDATE_COMMAND_UI(ID_LINE_WIDTH, OnUpdateLineWidth)
    ON_UPDATE_COMMAND_UI(ID_TOOL_ARROW, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_DWG_TOFRONT, OnUpdateDwgToFrontOrBack)
    ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCALE, OnUpdateViewFullScale)
    ON_UPDATE_COMMAND_UI(ID_VIEW_HALFSCALE, OnUpdateViewHalfScale)
    ON_COMMAND(ID_DWG_FONT, OnDwgFont)
    ON_COMMAND(ID_VIEW_FULLSCALE, OnViewFullScale)
    ON_COMMAND(ID_VIEW_HALFSCALE, OnViewHalfScale)
    ON_COMMAND(ID_VIEW_SMALLSCALE, OnViewSmallScale)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SMALLSCALE, OnUpdateViewSmallScale)
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
    ON_COMMAND_EX(ID_EDIT_LAYER_TILE, OnEditLayer)
    ON_COMMAND_EX(ID_EDIT_LAYER_TOP, OnEditLayer)
    ON_COMMAND_EX(ID_TOOL_ERASER, OnToolPalette)
    ON_COMMAND_EX(ID_TOOL_TILE, OnToolPalette)
    ON_COMMAND_EX(ID_TOOL_TEXT, OnToolPalette)
    ON_COMMAND_EX(ID_TOOL_FILL, OnToolPalette)
    ON_COMMAND_EX(ID_TOOL_DROPPER, OnToolPalette)
    ON_COMMAND_EX(ID_TOOL_LINE, OnToolPalette)
    ON_COMMAND_EX(ID_TOOL_POLYGON, OnToolPalette)
    ON_COMMAND_EX(ID_TOOL_RECT, OnToolPalette)
    ON_COMMAND_EX(ID_TOOL_OVAL, OnToolPalette)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LAYER_TOP, OnUpdateEditLayer)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LAYER_TILE, OnUpdateEditLayer)
    ON_UPDATE_COMMAND_UI(ID_TOOL_ERASER, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_TOOL_TILE, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_TOOL_TEXT, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_TOOL_FILL, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_TOOL_DROPPER, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_TOOL_LINE, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_TOOL_POLYGON, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_TOOL_RECT, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_TOOL_OVAL, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_DWG_TOBACK, OnUpdateDwgToFrontOrBack)
    ON_COMMAND(ID_VIEW_TOGGLE_SCALE, OnViewToggleScale)
    //}}AFX_MSG_MAP
    // Standard printing commands
    ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView construction/destruction

CBrdEditView::CBrdEditView() :
    m_selList(*this)
{
    m_bOffScreen = TRUE;
    m_pBoard = NULL;
    m_pBMgr = NULL;
    m_nCurToolID = ID_TOOL_ARROW;       // ID_TOOL_ARROW tool (select)
    m_nLastToolID = ID_TOOL_ARROW;      // ID_TOOL_ARROW tool (select)
    m_nZoom = fullScale;
}

CBrdEditView::~CBrdEditView()
{
}

BOOL CBrdEditView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CScrollView::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)GetStockObject(DKGRAY_BRUSH));

    return TRUE;
}

void CBrdEditView::OnInitialUpdate()
{
    CScrollView::OnInitialUpdate();
    m_pBMgr = GetDocument()->GetBoardManager();
    m_pBoard = (CBoard*)GetDocument()->GetCreateParameter();
    SetScrollSizes(MM_TEXT, m_pBoard->GetSize(m_nZoom));
}

void CBrdEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    WORD wHint = LOWORD(lHint);
    if ((wHint == HINT_TILEMODIFIED && m_pBoard->IsTileInUse(static_cast<CGmBoxHint*>(pHint)->GetArgs<HINT_TILEMODIFIED>().m_tid)) ||
        wHint == HINT_TILEDELETED || wHint == HINT_TILESETDELETED)
    {
        Invalidate(FALSE);          // Do redraw
        return;
    }
    else if (wHint == HINT_BOARDDELETED)
    {
        if (static_cast<CGmBoxHint*>(pHint)->GetArgs<HINT_BOARDDELETED>().m_pBoard == m_pBoard)
        {
            CFrameWnd* pFrm = GetParentFrame();
            ASSERT(pFrm != NULL);
            pFrm->SendMessage(WM_CLOSE, 0, 0L);
        }
    }
    else if (wHint == HINT_BOARDPROPCHANGE)
    {
        if (static_cast<CGmBoxHint*>(pHint)->GetArgs<HINT_BOARDPROPCHANGE>().m_pBoard == m_pBoard)
        {
            SetScrollSizes(MM_TEXT, m_pBoard->GetSize(m_nZoom));
            Invalidate(FALSE);
        }
    }
    else if (wHint == HINT_ALWAYSUPDATE)
        CScrollView::OnUpdate(pSender, lHint, pHint);
}

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView drawing

void CBrdEditView::OnDraw(CDC* pDC)
{
    CDC dcMem;
    CBitmap bmMem;
    CRect oRct;
    CDC* pDrawDC = pDC;
    CBitmap* pPrvBMap = nullptr;

    pDC->GetClipBox(&oRct);
    if (oRct.IsRectEmpty())
        return;                 // Nothing to do

    SetupPalette(pDC);

    if (m_bOffScreen)
    {
        bmMem.Attach(Create16BitDIBSection(pDC->m_hDC,
            oRct.Width(), oRct.Height()));
        dcMem.CreateCompatibleDC(pDC);
        pPrvBMap = dcMem.SelectObject(&bmMem);
        dcMem.SetViewportOrg(-oRct.left, -oRct.top);
        dcMem.SetStretchBltMode(COLORONCOLOR);
        SetupPalette(&dcMem);
        pDrawDC = &dcMem;
    }

    m_pBoard->Draw(pDrawDC, &oRct, m_nZoom);

    if (m_bOffScreen)
    {
        pDC->BitBlt(oRct.left, oRct.top, oRct.Width(), oRct.Height(),
            &dcMem, oRct.left, oRct.top, SRCCOPY);
        ResetPalette(&dcMem);
        dcMem.SelectObject(pPrvBMap);
    }
    if (!pDC->IsPrinting())
    {
        PrepareScaledDC(pDC);
        m_selList.OnDraw(*pDC);
    }
    ResetPalette(pDC);
}

BOOL CBrdEditView::OnEraseBkgnd(CDC* pDC)
{
    return CScrollView::OnEraseBkgnd(pDC);
}

void CBrdEditView::SetupPalette(CDC *pDC)
{
    CPalette* pPal = GetMainFrame()->GetMasterPalette();

    if (pPal == NULL || pPal->m_hObject == NULL)
        pPal = CPalette::FromHandle(
            (HPALETTE)::GetStockObject(DEFAULT_PALETTE));
    else
        pDC->SelectPalette(pPal, FALSE);
    pDC->RealizePalette();
}

void CBrdEditView::ResetPalette(CDC *pDC)
{
    pDC->SelectPalette(CPalette::FromHandle(
        (HPALETTE)::GetStockObject(DEFAULT_PALETTE)), TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Coordinate space mappings

void CBrdEditView::PrepareScaledDC(CDC *pDC)
{
    CSize wsize, vsize;
    m_pBoard->GetBoardArray()->GetBoardScaling(m_nZoom, wsize, vsize);
    pDC->SetMapMode(MM_ANISOTROPIC);
    pDC->SetWindowExt(wsize);
    pDC->SetViewportExt(vsize);
}

void CBrdEditView::OnPrepareScaledDC(CDC *pDC)
{
    OnPrepareDC(pDC, NULL);
    PrepareScaledDC(pDC);
}

void CBrdEditView::ClientToWorkspace(CPoint& point) const
{
    CPoint dpnt = GetDeviceScrollPosition();
    point += (CSize)dpnt;
    CSize wsize, vsize;
    m_pBoard->GetBoardArray()->GetBoardScaling(m_nZoom, wsize, vsize);
    ScalePoint(point, wsize, vsize);
}

void CBrdEditView::ClientToWorkspace(CRect& rect) const
{
    CPoint dpnt = GetDeviceScrollPosition();
    rect += dpnt;
    CSize wsize, vsize;
    m_pBoard->GetBoardArray()->GetBoardScaling(m_nZoom, wsize, vsize);
    ScaleRect(rect, wsize, vsize);
}

void CBrdEditView::WorkspaceToClient(CPoint& point) const
{
    CPoint dpnt = GetDeviceScrollPosition();
    CSize wsize, vsize;
    m_pBoard->GetBoardArray()->GetBoardScaling(m_nZoom, wsize, vsize);
    ScalePoint(point, vsize, wsize);
    point -= (CSize)dpnt;
}

void CBrdEditView::WorkspaceToClient(CRect& rect) const
{
    CPoint dpnt = GetDeviceScrollPosition();
    CSize wsize, vsize;
    m_pBoard->GetBoardArray()->GetBoardScaling(m_nZoom, wsize, vsize);
    ScaleRect(rect, vsize, wsize);
    rect -= dpnt;
}

void CBrdEditView::InvalidateWorkspaceRect(const CRect* pRect, BOOL bErase)
{
    CRect rct(pRect);
    WorkspaceToClient(rct);
    rct.InflateRect(1, 1);
    InvalidateRect(&rct, bErase);
}

CPoint CBrdEditView::GetWorkspaceDim()
{
    // First get MM_TEXT size of board for this scaling mode.
    CPoint pnt = (CPoint)m_pBoard->GetSize(m_nZoom);
    // Translate to current scaling mode.
    pnt -= (CSize)GetDeviceScrollPosition();
    ClientToWorkspace(pnt);
    return pnt;
}

/////////////////////////////////////////////////////////////////////////////

void CBrdEditView::ResetToDefaultTool()
{
    if (!GetDocument()->GetStickyDrawTools())
    {
        m_nCurToolID = m_nLastToolID = ID_TOOL_ARROW;
    }
}

/////////////////////////////////////////////////////////////////////////////

CDrawObj* CBrdEditView::ObjectHitTest(CPoint point)
{
    CDrawList* pDwg = GetDrawList(FALSE);
    if (pDwg != NULL)
        return pDwg->HitTest(point, m_nZoom, m_pBoard->GetApplyVisible());
    else
        return NULL;
}

void CBrdEditView::SelectWithinRect(CRect rctNet, BOOL bInclIntersects)
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
                ((pObj.GetEnclosingRect() | rctNet) == rctNet)) ||
                (bInclIntersects &&
                (!(pObj.GetEnclosingRect() & rctNet).IsRectEmpty())))
            {
                m_selList.AddObject(pObj, TRUE);
            }
        }
    }
}

void CBrdEditView::SelectAllUnderPoint(CPoint point)
{
    CDrawList* pDwg = GetDrawList(FALSE);
    if (pDwg == NULL)
        return;

    std::vector<CB::not_null<CDrawObj*>> selLst;
    pDwg->DrillDownHitTest(point, selLst, m_nZoom,
        m_pBoard->GetApplyVisible());

    for (size_t i = size_t(0) ; i < selLst.size() ; ++i)
    {
        CDrawObj& pObj = *selLst[i];
        if (!m_selList.IsObjectSelected(pObj))
            m_selList.AddObject(pObj, TRUE);
    }
}

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
    GetDocument()->SetModifiedFlag();
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
    GetDocument()->SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView printing

BOOL CBrdEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    return DoPreparePrinting(pInfo);
}

void CBrdEditView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add extra initialization before printing
}

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

void CBrdEditView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
    CRect oRct;
    pDC->GetClipBox(&oRct);
    m_pBoard->Draw(pDC, &oRct, m_nZoom);
}

void CBrdEditView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add cleanup after printing
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

CGamDoc* CBrdEditView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGamDoc)));
    return (CGamDoc*) m_pDocument;
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
    hint.GetArgs<HINT_BOARDPROPCHANGE>().m_pBoard = m_pBoard;
    GetDocument()->UpdateAllViews(this, HINT_BOARDPROPCHANGE, &hint);
}

void CBrdEditView::OnUpdateViewGridLines(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_pBoard->GetCellBorder());
}

BOOL CBrdEditView::OnEditLayer(UINT id)
{
    m_nCurToolID = ID_TOOL_ARROW;       // Turn off tool with layer change
    m_nLastToolID = ID_TOOL_ARROW;      // Turn off tool with layer change
    m_selList.PurgeList(TRUE);

    BeginWaitCursor();
    m_pBoard->SetMaxDrawLayer((int)((UINT)id - ID_EDIT_LAYER_BASE + 1));
    Invalidate(FALSE);
    UpdateWindow();
    EndWaitCursor();
    return TRUE;
}

void CBrdEditView::OnUpdateEditLayer(CCmdUI* pCmdUI)
{
    // NOTE!!: The control ID's are assumed to be consecutive and
    // in the same order as the tool codes defined in MAINFRM.CPP
    pCmdUI->Enable(TRUE);
    pCmdUI->SetCheck(m_pBoard->GetMaxDrawLayer() == -1 ?
        pCmdUI->m_nID == ID_EDIT_LAYER_TOP :
        pCmdUI->m_nID - ID_EDIT_LAYER_BASE + 1 ==
            (unsigned)m_pBoard->GetMaxDrawLayer());
}

/////////////////////////////////////////////////////////////////////////////

void CBrdEditView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    // Make sure window is active.
    GetParentFrame()->ActivateFrame();

    CMenu bar;
    if (bar.LoadMenu(IDR_MENU_DESIGN_POPUPS))
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

/////////////////////////////////////////////////////////////////////////////
// CBrdEditView mouse and timer message handlers

void CBrdEditView::OnLButtonDown(UINT nFlags, CPoint point)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    CTool& pTool = CTool::GetTool(eToolType);
    ClientToWorkspace(point);
    pTool.OnLButtonDown(this, nFlags, point);
}

void CBrdEditView::OnMouseMove(UINT nFlags, CPoint point)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    CTool& pTool = CTool::GetTool(eToolType);
    ClientToWorkspace(point);
    pTool.OnMouseMove(this, nFlags, point);
}

void CBrdEditView::OnLButtonUp(UINT nFlags, CPoint point)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    CTool& pTool = CTool::GetTool(eToolType);
    ClientToWorkspace(point);
    pTool.OnLButtonUp(this, nFlags, point);
}

void CBrdEditView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    CTool& pTool = CTool::GetTool(eToolType);
    ClientToWorkspace(point);
    pTool.OnLButtonDblClk(this, nFlags, point);
}

void CBrdEditView::OnTimer(UINT nIDEvent)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    CTool& pTool = CTool::GetTool(eToolType);
    pTool.OnTimer(this, nIDEvent);
}

BOOL CBrdEditView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    if (pWnd == this && eToolType != ttypeUnknown)
    {
        CTool& pTool = CTool::GetTool(eToolType);
        if(pTool.OnSetCursor(this, nHitTest))
            return TRUE;
    }
    return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

//////////////////////////////////////////////////////////////////////

void CBrdEditView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    ToolType eToolType = MapToolType(m_nCurToolID);
    CTool& pTool = CTool::GetTool(eToolType);
    if (pTool.m_eToolType == ttypePolygon)
    {
        CPolyTool& pPolyTool = static_cast<CPolyTool&>(pTool);
        if (nChar == VK_ESCAPE)
        {
            pPolyTool.RemoveRubberBand(this);
            pPolyTool.FinalizePolygon(this, TRUE);
        }
        else if (nChar == VK_RETURN)
        {
            pPolyTool.RemoveRubberBand(this);
            pPolyTool.FinalizePolygon(this, FALSE);
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

//////////////////////////////////////////////////////////////////////

static ToolType tblDrawTools[] =
{
    ttypeSelect,    // ID_TOOL_ARROW
    ttypeUnknown,   // ID_TOOL_ERASER
    ttypeTile,      // ID_TOOL_TILE
    ttypeText,      // ID_TOOL_TEXT
    ttypePaintBack, // ID_TOOL_FILL
    ttypeColorPick, // ID_TOOL_DROPPER
    ttypeLine,      // ID_TOOL_LINE
    ttypePolygon,   // ID_TOOL_POLYGON
    ttypeRect,      // ID_TOOL_RECT
    ttypeEllipse,   // ID_TOOL_OVAL
};

static ToolType tblGridTools[] =
{
    ttypeUnknown,   // ID_TOOL_ARROW
    ttypeCellEraser,// ID_TOOL_ERASER
    ttypeTile,      // ID_TOOL_TILE
    ttypeUnknown,   // ID_TOOL_TEXT
    ttypeCellPaint, // ID_TOOL_FILL
    ttypeColorPick, // ID_TOOL_DROPPER
    ttypeUnknown,   // ID_TOOL_LINE
    ttypeUnknown,   // ID_TOOL_POLYGON
    ttypeUnknown,   // ID_TOOL_RECT
    ttypeUnknown,   // ID_TOOL_OVAL
};

ToolType CBrdEditView::MapToolType(UINT nToolResID)
{
    return m_pBoard->GetMaxDrawLayer() == LAYER_GRID ?
        tblGridTools[nToolResID - ID_TOOL_ARROW] :
        tblDrawTools[nToolResID - ID_TOOL_ARROW];
}

//////////////////////////////////////////////////////////////////////
// Tile drag and drop code.

LRESULT CBrdEditView::OnDragTileItem(WPARAM wParam, LPARAM lParam)
{
    m_nCurToolID = ID_TOOL_ARROW;       // Not valid with drag over.
    m_nLastToolID = ID_TOOL_ARROW;      // Not valid with drag over.

    DragInfo* pdi = (DragInfo*)lParam;

    if (pdi->m_dragType != DRAG_TILE)
        return 0;               // Only tile drops allowed
    if (pdi->GetSubInfo<DRAG_TILE>().m_gamDoc != GetDocument())
        return 0;               // Only tiles from our document.

    if (wParam == phaseDragOver)
        return (LRESULT)(LPVOID)pdi->m_hcsrSuggest;
    else if (wParam == phaseDragDrop)
    {
        CDrawList* pDwg;
        // Process a tile drop....
        CPoint pnt = pdi->m_point;
        ClientToWorkspace(pnt);
        switch (m_pBoard->GetMaxDrawLayer())
        {
            case LAYER_BASE:
                pDwg = m_pBoard->GetBaseDrawing(TRUE);
                SetDrawingTile(pDwg, pdi->GetSubInfo<DRAG_TILE>().m_tileID, pnt, TRUE);
                break;
            case LAYER_GRID:
                SetCellTile(pdi->GetSubInfo<DRAG_TILE>().m_tileID, pnt, TRUE);
                break;
            case LAYER_TOP:
                pDwg = m_pBoard->GetTopDrawing(TRUE);
                SetDrawingTile(pDwg, pdi->GetSubInfo<DRAG_TILE>().m_tileID, pnt, TRUE);
                break;
            default: ;
        }
    }
    return 0;
}

void CBrdEditView::SetDrawingTile(CDrawList* pDwg, TileID tid, CPoint pnt,
    BOOL bUpdate)
{
    CTileManager* pTMgr = GetDocument()->GetTileManager();
    {
        OwnerPtr<CTileImage> pTileImage(MakeOwner<CTileImage>(pTMgr));

        // Center the image on the drop point.
        CTile tile;
        pTMgr->GetTile(tid, &tile);
        CRect rct(CPoint(pnt.x, pnt.y), tile.GetSize());
        rct.OffsetRect(-rct.Width() / 2, -rct.Height() / 2);
        AdjustRect(rct);

        pTileImage->SetTile(rct.left, rct.top, tid);

        pDwg->AddToFront(std::move(pTileImage));
    }
    if (bUpdate)
    {
        CRect rct;
        rct = pDwg->Front().GetEnclosingRect();   // In board coords.
        InvalidateWorkspaceRect(&rct);
    }
    GetDocument()->SetModifiedFlag();
}

void CBrdEditView::SetCellTile(TileID tid, CPoint pnt, BOOL bUpdate)
{
    int row, col;

    CBoardArray* pBa = m_pBoard->GetBoardArray();
    ASSERT(pBa != NULL);

    WorkspaceToClient(pnt);
    pnt += (CSize)GetDeviceScrollPosition();

    if (!pBa->FindCell(pnt.x, pnt.y, row, col, m_nZoom))
        return;                                 // Not a valid cell hit
    if (tid != pBa->GetCellTile(row, col))
    {
        pBa->SetCellTile(row, col, tid);
        if (bUpdate)
        {
            CRect rct;
            pBa->GetCellRect(row, col, &rct, m_nZoom);// In board coords
            rct -= GetDeviceScrollPosition();
            InvalidateRect(&rct, FALSE);
        }
        GetDocument()->SetModifiedFlag();
    }
}

void CBrdEditView::SetCellColor(COLORREF crCell, CPoint pnt, BOOL bUpdate)
{
    int row, col;

    CBoardArray* pBa = m_pBoard->GetBoardArray();
    ASSERT(pBa != NULL);
    WorkspaceToClient(pnt);
    pnt += (CSize)GetDeviceScrollPosition();

    if (!pBa->FindCell(pnt.x, pnt.y, row, col, m_nZoom))
        return;                                 // Not a valid cell hit
    if (crCell != pBa->GetCellColor(row, col))
    {
        pBa->SetCellColor(row, col, crCell);
        if (bUpdate)
        {
            CRect rct;
            pBa->GetCellRect(row, col, &rct, m_nZoom);// In board coords
            rct -= GetDeviceScrollPosition();
            InvalidateRect(&rct, FALSE);
        }
        GetDocument()->SetModifiedFlag();
    }
}

void CBrdEditView::SetBoardBackColor(COLORREF cr, BOOL bUpdate)
{
    m_pBoard->SetBkColor(m_pBMgr->GetForeColor());
    if (bUpdate)
        Invalidate();
    GetDocument()->SetModifiedFlag();
}

CDrawList* CBrdEditView::GetDrawList(BOOL bCanCreateList)
{
    ASSERT(m_pBoard != NULL);

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
        GetDocument()->SetModifiedFlag();
    }
}

void CBrdEditView::DeleteDrawObject(CDrawObj* pObj)
{
    CDrawList* pDwg = NULL;

    if (m_pBoard->GetMaxDrawLayer() == LAYER_TOP)
        pDwg = m_pBoard->GetTopDrawing();
    else if (m_pBoard->GetMaxDrawLayer() == LAYER_BASE)
        pDwg = m_pBoard->GetBaseDrawing();

    if (pDwg != NULL)
    {
        pDwg->RemoveObject(CheckedDeref(pObj));
        delete pObj;
        GetDocument()->SetModifiedFlag();
    }
}

void CBrdEditView::DoCreateTextDrawingObject(CPoint point)
{
    if (m_pBoard->GetMaxDrawLayer() == LAYER_GRID)
        return;                     // No text here!

    CTextObjDialog dlg;
    dlg.m_pFontMgr = CGamDoc::GetFontManager();
    dlg.SetFontID(m_pBMgr->GetFontID());

    if (dlg.DoModal() == IDOK)
    {
        if (!dlg.m_strText.IsEmpty())
        {
            CreateTextDrawingObject(point, dlg.m_fontID,
                m_pBMgr->GetForeColor(), dlg.m_strText, TRUE);
            GetDocument()->SetModifiedFlag();
        }
    }
}

void CBrdEditView::DoEditTextDrawingObject(CText* pDObj)
{
    CTextObjDialog dlg;
    dlg.m_strText = pDObj->m_text;
    dlg.m_pFontMgr = CGamDoc::GetFontManager();
    dlg.SetFontID(pDObj->m_fontID);

    if (dlg.DoModal() == IDOK)
    {
        if (!dlg.m_strText.IsEmpty())
        {
            CRect rct = pDObj->GetEnclosingRect();
            InvalidateWorkspaceRect(&rct);

            pDObj->m_text = dlg.m_strText;
            pDObj->SetFont(dlg.m_fontID);   // Also resyncs the extent rect

            rct = pDObj->GetEnclosingRect();
            InvalidateWorkspaceRect(&rct);
            GetDocument()->SetModifiedFlag();
        }
    }
}

void CBrdEditView::CreateTextDrawingObject(CPoint pnt, FontID fid,
    COLORREF crText, CString& strText, BOOL bInvalidate /* = TRUE */)
{
    {
        OwnerPtr<CText> pText = MakeOwner<CText>();
        AdjustPoint(pnt);
        pText->SetText(pnt.x, pnt.y, strText, fid, crText);

        CRect rct = pText->GetRect();
        AdjustRect(rct);
        pText->SetRect(rct);

        GetDrawList(TRUE)->AddToFront(std::move(pText));
    }
    GetDocument()->SetModifiedFlag();
    if (bInvalidate)
    {
        CRect rct = GetDrawList(TRUE)->Front().GetEnclosingRect();
        InvalidateWorkspaceRect(&rct);
    }
}

//////////////////////////////////////////////////////////////////////

BOOL CBrdEditView::IsGridizeActive()
{
    BOOL bGridSnap = m_pBoard->m_bGridSnap;
    int bControl = GetAsyncKeyState(VK_CONTROL) < 0;
    return !(bControl && bGridSnap || !bControl && !bGridSnap);
}

void CBrdEditView::GridizeX(long& xPos)
{
    if (IsGridizeActive())
    {
        xPos = GridizeClosest1000(xPos,
            m_pBoard->m_xGridSnap, m_pBoard->m_xGridSnapOff);
    }
}

void CBrdEditView::GridizeY(long& yPos)
{
    if (IsGridizeActive())
    {
        yPos = GridizeClosest1000(yPos,
            m_pBoard->m_yGridSnap, m_pBoard->m_yGridSnapOff);
    }
}

void CBrdEditView::LimitPoint(POINT* pPnt)
{
    if (pPnt->x < 0) pPnt->x = 0;
    if (pPnt->x > m_pBoard->GetWidth(fullScale))
        pPnt->x = m_pBoard->GetWidth(fullScale);
    if (pPnt->y < 0) pPnt->y = 0;
    if (pPnt->y > m_pBoard->GetHeight(fullScale))
        pPnt->y = m_pBoard->GetHeight(fullScale);

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

void CBrdEditView::LimitRect(RECT* pRct)
{
    CRect rct(pRct);
    if (rct.left < 0)
        rct.OffsetRect(-rct.left, 0);
    if (rct.top < 0)
        rct.OffsetRect(0, -rct.top);
    if (rct.right > m_pBoard->GetWidth(fullScale))
        rct.OffsetRect(m_pBoard->GetWidth(fullScale) - rct.right, 0);
    if (rct.bottom > m_pBoard->GetHeight(fullScale))
        rct.OffsetRect(0, m_pBoard->GetHeight(fullScale) - rct.bottom);
    *pRct = rct;
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

void CBrdEditView::AdjustPoint(CPoint& pnt)
{
    GridizeX(pnt.x);
    GridizeY(pnt.y);
    LimitPoint(&pnt);
}

void CBrdEditView::AdjustRect(CRect& rct)
{
    CPoint pnt = rct.TopLeft();
    GridizeX(pnt.x);
    GridizeY(pnt.y);
    LimitPoint(&pnt);
    if (pnt != rct.TopLeft())
        rct.OffsetRect(pnt - rct.TopLeft());
    LimitRect(&rct);
}

void CBrdEditView::PixelToWorkspace(CPoint& point)
{
    CSize wsize, vsize;
    m_pBoard->GetBoardArray()->GetBoardScaling(m_nZoom, wsize, vsize);
    ScalePoint(point, wsize, vsize);
}

//////////////////////////////////////////////////////////////////////

void CBrdEditView::OnUpdateColorForeground(CCmdUI* pCmdUI)
{
    ((CColorCmdUI*)pCmdUI)->SetColor(m_pBMgr->GetForeColor());
}

void CBrdEditView::OnUpdateColorBackground(CCmdUI* pCmdUI)
{
    ((CColorCmdUI*)pCmdUI)->SetColor(m_pBMgr->GetBackColor());
}

void CBrdEditView::OnUpdateColorCustom(CCmdUI* pCmdUI)
{
    ((CColorCmdUI*)pCmdUI)->SetCustomColors(GetDocument()->GetCustomColors());
}

void CBrdEditView::OnUpdateLineWidth(CCmdUI* pCmdUI)
{
    ((CColorCmdUI*)pCmdUI)->SetLineWidth(m_pBMgr->GetLineWidth());
}

//////////////////////////////////////////////////////////////////////


LRESULT CBrdEditView::OnSetColor(WPARAM wParam, LPARAM lParam)
{
    struct ColorSetting
    {
        COLORREF m_cr;              // Color to set
        BOOL     m_bColorAccepted;  // TRUE if at least one object accepted the color
    };
    ColorSetting cset = { RGB(0,0,0), FALSE };

    if ((UINT)wParam == ID_COLOR_FOREGROUND)
    {
        m_pBMgr->SetForeColor((COLORREF)lParam);
        cset.m_cr = m_pBMgr->GetForeColor();
        m_selList.ForAllSelections([&cset](CDrawObj& pObj) { cset.m_bColorAccepted |= pObj.SetForeColor(cset.m_cr); });
        m_selList.UpdateObjects();
    }
    else if ((UINT)wParam == ID_COLOR_BACKGROUND)
    {
        m_pBMgr->SetBackColor((COLORREF)lParam);
        cset.m_cr = m_pBMgr->GetBackColor();
        m_selList.ForAllSelections([&cset](CDrawObj& pObj) { cset.m_bColorAccepted |= pObj.SetBackColor(cset.m_cr); });
        m_selList.UpdateObjects();
    }
    else
        return 0;
    if (cset.m_bColorAccepted)
        GetDocument()->SetModifiedFlag();
    return 1;
}

//////////////////////////////////////////////////////////////////////

LRESULT CBrdEditView::OnSetCustomColors(WPARAM wParam, LPARAM lParam)
{
    LPVOID pCustomColors = (LPVOID)wParam;
    GetDocument()->SetCustomColors(pCustomColors);
    return (LRESULT)0;
}

//////////////////////////////////////////////////////////////////////

LRESULT CBrdEditView::OnSetLineWidth(WPARAM wParam, LPARAM lParam)
{
    struct WidthSetting
    {
        UINT m_nWidth;          // Width to set
        BOOL m_bWidthAccepted;  // TRUE if at least one object accepted the color
    };
    WidthSetting wset = { 1, FALSE };

    m_pBMgr->SetLineWidth((UINT)wParam);
    m_selList.UpdateObjects();
    wset.m_nWidth = m_pBMgr->GetLineWidth();
    m_selList.ForAllSelections([&wset](CDrawObj& pObj) { wset.m_bWidthAccepted |= pObj.SetLineWidth(wset.m_nWidth); });
    m_selList.UpdateObjects(TRUE, FALSE);
    if (wset.m_bWidthAccepted)
        GetDocument()->SetModifiedFlag();
    return 1;
}

// ------------------------------------------------------ //

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
            GetDocument()->SetModifiedFlag();
        }
    }
}

// ------------------------------------------------------ //

BOOL CBrdEditView::OnToolPalette(UINT id)
{
    if (id != ID_TOOL_ARROW)
        m_selList.PurgeList(TRUE);
    if (id != m_nCurToolID)
    {
        if (m_nCurToolID == ID_TOOL_POLYGON)
        {
            // If we're changing away from the polygon tool
            // we need to act as if the escape key was hit.
            ToolType eToolType = MapToolType(m_nCurToolID);
            CTool& pTool = CTool::GetTool(eToolType);
            if (pTool.m_eToolType == ttypePolygon)
            {
                CPolyTool& pPolyTool = static_cast<CPolyTool&>(pTool);
                pPolyTool.RemoveRubberBand(this);
                pPolyTool.FinalizePolygon(this, TRUE);
            }
        }
        m_nLastToolID = m_nCurToolID;
        m_nCurToolID = id;
    }
    return TRUE;
}

// ---------------------------------------------------- //

void CBrdEditView::OnUpdateToolPalette(CCmdUI* pCmdUI)
{
    TileID tid;
    int iLayer = m_pBoard->GetMaxDrawLayer();
    BOOL bEnable;

    switch (pCmdUI->m_nID)
    {
        case ID_TOOL_TILE:
            tid = GetDocument()->GetTilePalWnd()->GetCurrentTileID();
            bEnable = tid != nullTid;
            if (tid == nullTid && m_nCurToolID == ID_TOOL_TILE)
            {
                m_nCurToolID = ID_TOOL_ARROW;
                m_nLastToolID = ID_TOOL_ARROW;
            }
            break;
        case ID_TOOL_ARROW:
        case ID_TOOL_DROPPER:
            bEnable = TRUE;
            break;
        case ID_TOOL_FILL:
        case ID_TOOL_ERASER:
            bEnable = iLayer == LAYER_BASE || iLayer == LAYER_GRID;
            break;
        case ID_TOOL_TEXT:
        case ID_TOOL_LINE:
        case ID_TOOL_POLYGON:
        case ID_TOOL_RECT:
        case ID_TOOL_OVAL:
            bEnable = iLayer == LAYER_BASE || iLayer == LAYER_TOP;
            break;
        default:
            ASSERT(!"unexpected tool id");
            bEnable = false;
    }
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // Need to handle menu that the submenu is connected to.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (bEnable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }

    pCmdUI->Enable(bEnable);
    // NOTE!!: The control ID's are assumed to be consecutive and
    // in the same order as the tool codes defined in MAINFRM.C
    pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurToolID);
}

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

void CBrdEditView::CenterViewOnWorkspacePoint(CPoint point)
{
    WorkspaceToClient(point);
    CRect rct;
    GetClientRect(&rct);
    CPoint pt = GetMidRect(rct);
    CSize size = point - pt;
    CPoint newUpLeft = GetDeviceScrollPosition() + size;
    // If the axis being scrolled is entirely visible then set
    // that scroll position to zero.
    CSize sizeTotal = GetTotalSize();   // Logical is in device units for us
    if (rct.Width() >= sizeTotal.cx)
        newUpLeft.x = 0;
    if (rct.Height() >= sizeTotal.cy)
        newUpLeft.y = 0;
    ScrollToPosition(newUpLeft);
    UpdateWindow();
}

void CBrdEditView::DoViewScale(TileScale nZoom)
{
    CRect rctClient;
    GetClientRect(&rctClient);
    CPoint pntMid = rctClient.CenterPoint();
    ClientToWorkspace(pntMid);

    m_nZoom = nZoom;
    SetScrollSizes(MM_TEXT, m_pBoard->GetSize(m_nZoom));
    BeginWaitCursor();
    Invalidate(FALSE);
    CenterViewOnWorkspacePoint(pntMid);
    EndWaitCursor();
}

void CBrdEditView::OnViewFullScale()
{
    DoViewScale(fullScale);
}

void CBrdEditView::OnUpdateViewFullScale(CCmdUI* pCmdUI)
{
    pCmdUI->Enable();
    pCmdUI->SetCheck(m_nZoom == fullScale);
}

void CBrdEditView::OnViewHalfScale()
{
    DoViewScale(halfScale);
}

void CBrdEditView::OnUpdateViewHalfScale(CCmdUI* pCmdUI)
{
    pCmdUI->Enable();
    pCmdUI->SetCheck(m_nZoom == halfScale);
}

void CBrdEditView::OnViewSmallScale()
{
    DoViewScale(smallScale);
}

void CBrdEditView::OnUpdateViewSmallScale(CCmdUI* pCmdUI)
{
    pCmdUI->Enable();
    pCmdUI->SetCheck(m_nZoom == smallScale);
}

void CBrdEditView::OnViewToggleScale()
{
    if (m_nZoom == fullScale)
         DoViewScale(halfScale);
    else if (m_nZoom == halfScale)
         DoViewScale(smallScale);
    else // smallScale
         DoViewScale(fullScale);
}

void CBrdEditView::OnUpdateIndicatorCellNum(CCmdUI* pCmdUI)
{
    CBoardArray* pba = m_pBoard->GetBoardArray();
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
    GetDocument()->DoBoardPropertyDialog(*m_pBoard);
}

void CBrdEditView::OnToolSetVisibleScale()
{
    CSetScaleVisibilityDialog dlg;
    dlg.m_bFullScale = TRUE;
    dlg.m_bHalfScale = TRUE;
    dlg.m_bSmallScale = TRUE;
    dlg.m_bNaturalScale = FALSE;

    if (dlg.DoModal())
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
    OwnerPtr<CBitmap> pBMap = GetClipboardBitmap(this, GetAppPalette());

    {
        OwnerPtr<CBitmapImage> pDObj = MakeOwner<CBitmapImage>();
        pDObj->SetBitmap(0, 0, (HBITMAP)pBMap->Detach(), fullScale);

        GetSelectList()->PurgeList(TRUE);           // Clear current select list
        AddDrawObject(std::move(pDObj));
    }
    CDrawObj& pDObj = GetDrawList(FALSE)->Front();
    GetSelectList()->AddObject(pDObj, TRUE);
    CRect rct = pDObj.GetEnclosingRect();
    InvalidateWorkspaceRect(&rct);
}

void CBrdEditView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
    int nMaxLayer = m_pBoard->GetMaxDrawLayer();
    pCmdUI->Enable(IsClipboardBitmap() &&
        (nMaxLayer < 0 || nMaxLayer == LAYER_BASE || nMaxLayer == LAYER_TOP));
}

void CBrdEditView::OnEditCopy()
{
    GetSelectList()->CopyToClipboard();
}

void CBrdEditView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetSelectList()->IsCopyToClipboardPossible());
}

void CBrdEditView::OnEditPasteBitmapFromFile()
{
    CString strFilter;
    strFilter.LoadString(IDS_BMP_FILTER);
    CString strTitle;
    strTitle.LoadString(IDS_SEL_BITMAPFILE);

    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,
        strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return;

    CFile file;
    CFileException fe;

    if (!file.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite,
        &fe))
    {
        CString strErr;
        AfxFormatString1(strErr, AFX_IDP_FAILED_TO_OPEN_DOC, dlg.GetPathName());
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return;
    }
    CDib dib;
    try
    {
        dib.ReadDIBFile(file);
    }
    catch (...)
    {
        AfxMessageBox(IDP_ERR_LOADBITMAP, MB_ICONEXCLAMATION);
        return;
    }
    OwnerPtr<CBitmap> pBMap = dib.DIBToBitmap(GetAppPalette());

    {
        OwnerPtr<CBitmapImage> pDObj = MakeOwner<CBitmapImage>();
        pDObj->SetBitmap(0, 0, (HBITMAP)pBMap->Detach(), fullScale);

        GetSelectList()->PurgeList(TRUE);           // Clear current select list
        AddDrawObject(std::move(pDObj));
    }
    CDrawObj& pDObj = GetDrawList(FALSE)->Front();
    GetSelectList()->AddObject(pDObj, TRUE);
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
        dX *= m_pBoard->m_xGridSnap / 1000;
        dY *= m_pBoard->m_yGridSnap / 1000;
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
        AdjustRect(pSel.m_pObj->GetRect());
    }

    m_selList.InvalidateList();
    m_selList.InvalidateListHandles();
    GetDocument()->SetModifiedFlag();

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


