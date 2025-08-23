// VwTbrd.cpp : Small scale playing board view.
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
#include    "GamDoc.h"
#include    "FrmMain.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "VwTbrd.h"
#include    "WinPoptb.h"
#include    "GMisc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTinyBoardViewContainer, CView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CTinyBoardView, CB::ProcessEventOverride<wxScrolledCanvas>)
    EVT_LEFT_DOWN(OnLButtonDown)
    EVT_RIGHT_DOWN(OnRButtonDown)
#if 0
    ON_WM_MOUSEACTIVATE()
#endif
    EVT_WINSTATE(OnMessageWindowState)
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CTinyBoardViewContainer, CTinyBoardViewContainer::BASE)
    ON_WM_CREATE()
#if 0
    ON_WM_SIZE()
#endif
    ON_WM_MOUSEACTIVATE()
    ON_MESSAGE(WM_WINSTATE, OnMessageWindowState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardView

CTinyBoardView::CTinyBoardView(CTinyBoardViewContainer& p) :
    parent(&p),
    document(dynamic_cast<CGamDoc*>(parent->GetDocument())),
    m_pPBoard(static_cast<CPlayBoard*>(document->GetNewViewParameter()))
{
    // use sizers for scrolling
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    sizer->Add(0, 0);
    BASE::Create(*parent, 0);
    OnInitialUpdate();
}

#if 0
BOOL CTinyBoardView::PreCreateWindow(CREATESTRUCT& cs)
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

void CTinyBoardView::OnInitialUpdate()
{
    RecalcScrollLimits();
}

void CTinyBoardView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CGamDocHint* ph = (CGamDocHint*)pHint;
    if (lHint == HINT_UPDATEOBJECT && ph->GetArgs<HINT_UPDATEOBJECT>().m_pPBoard == m_pPBoard)
    {
        wxRect rct;
        rct = CB::Convert(ph->GetArgs<HINT_UPDATEOBJECT>().m_pDrawObj->GetEnclosingRect());   // In board coords.
        InvalidateWorkspaceRect(rct);
    }
    else if (lHint == HINT_UPDATEOBJLIST && ph->GetArgs<HINT_UPDATEOBJLIST>().m_pPBoard == m_pPBoard)
    {
        wxASSERT(!"untested code");
        const std::vector<CB::not_null<CDrawObj*>>& pPtrList = *ph->GetArgs<HINT_UPDATEOBJLIST>().m_pPtrList;
        for (size_t i = size_t(0); i < pPtrList.size(); ++i)
        {
            CDrawObj& pDObj = *pPtrList[i];
            wxRect rct = CB::Convert(pDObj.GetEnclosingRect());  // In board coords.
            InvalidateWorkspaceRect(rct);
        }
    }
    else if (lHint == HINT_UPDATEBOARD && ph->GetArgs<HINT_UPDATEBOARD>().m_pPBoard == m_pPBoard)
    {
        m_pBMap = wxBitmap();
        Refresh();
    }
    else if (lHint == HINT_ALWAYSUPDATE || lHint == HINT_GAMESTATEUSED)
    {
        parent->CTinyBoardViewContainer::BASE::OnUpdate(pSender, lHint, pHint);
    }
}

///////////////////////////////////////////////////////////////////////
// This message is sent when a document is being saved.

void CTinyBoardView::OnMessageWindowState(WinStateEvent& event)
{
    CArchive& ar = event.GetArchive();
    if (ar.IsStoring())
    {
        wxPoint pnt = GetViewStart();
        ar << value_preserving_cast<uint32_t>(pnt.x);
        ar << value_preserving_cast<uint32_t>(pnt.y);
    }
    else
    {
        wxPoint pnt;
        uint32_t dwTmp;
        ar >> dwTmp; pnt.x = value_preserving_cast<decltype(pnt.x)>(dwTmp);
        ar >> dwTmp; pnt.y = value_preserving_cast<decltype(pnt.y)>(dwTmp);
        Scroll(pnt);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardView drawing

void CTinyBoardView::OnDraw(wxDC& pDC)
{
    if (!m_pBMap.IsOk())
        RegenCachedMap(pDC);
    wxASSERT(m_pBMap.IsOk());

    wxMemoryDC dcMem;
    wxRect    oRct;
    wxRect    oRctSave;

    pDC.GetClippingBox(oRct);
    if (oRct.IsEmpty())
        return;                 // Nothing to do

    wxBitmap bmMem(oRct.GetWidth(), oRct.GetHeight(), pDC);
    dcMem.SelectObject(bmMem);
    {
    wxDCBrushChanger setBrush(dcMem, *wxWHITE_BRUSH);
    wxDCPenChanger setPen(dcMem, *wxTRANSPARENT_PEN);
    dcMem.DrawRectangle(0, 0, oRct.GetWidth(), oRct.GetHeight());
    }

    if (m_pPBoard->IsBoardRotated180())
    {
        oRctSave = oRct;
        wxSize sizeBrd = CB::Convert(m_pPBoard->GetBoard()->GetSize(smallScale));
        oRct = wxRect(wxPoint(sizeBrd.x - oRct.GetLeft() - oRct.GetWidth(),
            sizeBrd.y - oRct.GetTop() - oRct.GetHeight()), oRct.GetSize());
    }

    dcMem.SetDeviceOrigin(-oRct.GetLeft(), -oRct.GetTop());

    // Draw updated part of board image
    dcMem.DrawBitmap(m_pBMap, 0, 0);

    // Draw pieces etc. (Need to rescale the DC and the update rect)

    wxRect rct(oRct);
    {
    DCSetupDrawListDC setupDrawListDC(*this, dcMem, rct);

    m_pPBoard->Draw(dcMem, rct, smallScale);

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
        // Copy to output area
        pDC.Blit(oRct.GetLeft(), oRct.GetTop(), oRct.GetWidth(), oRct.GetHeight(),
            &dcMem, oRct.GetLeft(), oRct.GetTop(), wxCOPY);
    }
}

wxBitmap CTinyBoardView::DrawFullMap()
{
    wxBitmap bmap;
    wxMemoryDC dcMem;

    wxSize size = CB::Convert(m_pPBoard->GetBoard()->GetSize(smallScale));

    bmap = wxBitmap(size.x, size.y);
    dcMem.SelectObject(bmap);

    // Draw updated part of board image
    dcMem.DrawBitmap(m_pBMap, 0, 0);

    // Draw pieces etc. (Need to rescale the DC and the update rect)

    wxRect rct(wxPoint(0,0), size);
    {
    DCSetupDrawListDC setupDrawList(*this, dcMem, rct);

    m_pPBoard->Draw(dcMem, rct, smallScale);

    }

    return bmap;
}

void CTinyBoardView::RegenCachedMap(wxDC& pDC)
{
    wxBusyCursor waitCursor;

    CBoard* pBoard = m_pPBoard->GetBoard();
    wxSize size = CB::Convert(pBoard->GetSize(smallScale));   // Get pixel size of board
    m_pBMap = wxBitmap(size.x, size.y, pDC);

    wxMemoryDC dcMem;
    dcMem.SelectObject(m_pBMap);

    wxRect rct(wxPoint(0, 0), size);
    pBoard->SetMaxDrawLayer();                  // Make sure all layers are drawn
    pBoard->Draw(dcMem, rct, smallScale, m_pPBoard->m_bSmallCellBorders);
}

/////////////////////////////////////////////////////////////////////////////

CTinyBoardView::DCSetupDrawListDC::DCSetupDrawListDC(const CTinyBoardView& rThis, wxDC& pDC, wxRect& rct)
{
    wxSize wsize, vsize;
    rThis.m_pPBoard->GetBoard()->GetBoardArray().
        GetBoardScaling(smallScale, wsize, vsize);

    scaleChanger = CB::DCUserScaleChanger(pDC, double(vsize.x)/wsize.x, double(vsize.y)/wsize.y);

    ScaleRect(rct, wsize, vsize);
}

/////////////////////////////////////////////////////////////////////////////

void CTinyBoardView::WorkspaceToClient(wxRect& rect) const
{
    wxSize wsize, vsize;
    m_pPBoard->GetBoard()->GetBoardArray().
        GetBoardScaling(smallScale, wsize, vsize);
    if (m_pPBoard->IsBoardRotated180())
    {
        rect = wxRect(wxPoint(wsize.x - rect.GetLeft(), wsize.y - rect.GetTop()),
            wxPoint(wsize.x - rect.GetRight(), wsize.y - rect.GetBottom()));
        CB::Normalize(rect);
    }
    ScaleRect(rect, vsize, wsize);
    rect.SetLeftTop(CalcScrolledPosition(rect.GetTopLeft()));
}

void CTinyBoardView::InvalidateWorkspaceRect(const wxRect& pRect, BOOL bErase)
{
    wxRect rct(pRect);
    WorkspaceToClient(rct);
    RefreshRect(rct, bErase);
}

void CTinyBoardView::ClientToWorkspace(wxPoint& pnt) const
{
    pnt = CalcUnscrolledPosition(pnt);
    wxSize wsize, vsize;
    m_pPBoard->GetBoard()->GetBoardArray().
        GetBoardScaling(smallScale, wsize, vsize);
    ScalePoint(pnt, wsize, vsize);
    if (m_pPBoard->IsBoardRotated180())
        pnt = wxPoint(wsize.x - pnt.x, wsize.y - pnt.y);
}

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardView message handlers

void CTinyBoardView::OnLButtonDown(wxMouseEvent& event)
{
    wxPoint point = event.GetPosition();
    ClientToWorkspace(point);
    // TODO:  convert to wx when frame handles wxEvent
    CFrameWnd* pFrame = parent->GetParentFrame();
    CPoint cpoint = CB::Convert(point);
    pFrame->SendMessage(WM_CENTERBOARDONPOINT, reinterpret_cast<WPARAM>(&cpoint));
}

void CTinyBoardView::OnRButtonDown(wxMouseEvent& event)
{
    if (!m_pBMap.IsOk())
        return;

    // owned by MFC
    RefPtr<CTinyBoardPopup> pTBrd(new CTinyBoardPopup(CheckedDeref(parent->GetParentFrame())));

    pTBrd->m_bmap = DrawFullMap();

    m_pPBoard->GetBoard()->GetBoardArray().
        GetBoardScaling(smallScale, pTBrd->m_wsize, pTBrd->m_vsize);
    pTBrd->m_bRotate180 = m_pPBoard->IsBoardRotated180();
    wxRect rct = GetScreenRect();
    pTBrd->Create(CheckedDeref(GetMainFrame()), GetMidRect(rct));

    event.Skip();
}

#if 0
int CTinyBoardView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    // We don't want the frame to ever consider this view to be the
    // "active" view.
    return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}
#endif

void CTinyBoardView::RecalcScrollLimits()
{
    wxSizer& sizer = CheckedDeref(GetSizer());
    wxSizerItemList& items = sizer.GetChildren();
    wxASSERT(items.size() == 1);
    wxSizerItem& item = CheckedDeref(items[0]);
    wxASSERT(item.IsSpacer());

    CBoard* pBoard = m_pPBoard->GetBoard();
    wxSize size = CB::Convert(pBoard->GetSize(smallScale));
    item.AssignSpacer(size);
    SetScrollRate(size.x/100, size.y/100);
    sizer.FitInside(this);
}

void CTinyBoardViewContainer::OnDraw(CDC* pDC)
{
    // do nothing because child covers entire client rect
}

void CTinyBoardViewContainer::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    child->OnUpdate(pSender, lHint, pHint);

    BASE::OnUpdate(pSender, lHint, pHint);
}

CTinyBoardViewContainer::CTinyBoardViewContainer() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

int CTinyBoardViewContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (BASE::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    child = new CTinyBoardView(*this);

    return 0;
}

#if 0
void CTinyBoardViewContainer::OnSize(UINT nType, int cx, int cy)
{
    child->MoveWindow(0, 0, cx, cy);
    return CView::OnSize(nType, cx, cy);
}
#endif

int CTinyBoardViewContainer::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    // We don't want the frame to ever consider this view to be the
    // "active" view.
    return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

LRESULT CTinyBoardViewContainer::OnMessageWindowState(WPARAM wParam, LPARAM lParam)
{
    WinStateEvent event(*reinterpret_cast<CArchive*>(wParam), bool(lParam));
    child->ProcessWindowEvent(event);
    return (LRESULT)1;
}
