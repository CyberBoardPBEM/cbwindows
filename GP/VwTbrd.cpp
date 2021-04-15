// VwTbrd.cpp : Small scale playing board view.
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

IMPLEMENT_DYNCREATE(CTinyBoardView, CScrollView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTinyBoardView, CScrollView)
    //{{AFX_MSG_MAP(CTinyBoardView)
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_MOUSEACTIVATE()
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_WINSTATE, OnMessageWindowState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardView

CTinyBoardView::CTinyBoardView()
{
    m_pPBoard = NULL;
    m_pBMap = NULL;
}

CTinyBoardView::~CTinyBoardView()
{
    if (m_pBMap != NULL) delete m_pBMap;
}

BOOL CTinyBoardView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CScrollView::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)GetStockObject(WHITE_BRUSH));

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CTinyBoardView::OnInitialUpdate()
{
    CScrollView::OnInitialUpdate();

    m_pPBoard = (CPlayBoard*)GetDocument()->GetNewViewParameter();
    CBoard* pBoard = m_pPBoard->GetBoard();
    SetScrollSizes(MM_TEXT, pBoard->GetSize(smallScale));
}

void CTinyBoardView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CGamDocHint* ph = (CGamDocHint*)pHint;
    if (lHint == HINT_UPDATEOBJECT && ph->GetArgs<HINT_UPDATEOBJECT>().m_pPBoard == m_pPBoard)
    {
        CRect rct;
        rct = ph->GetArgs<HINT_UPDATEOBJECT>().m_pDrawObj->GetEnclosingRect();   // In board coords.
        InvalidateWorkspaceRect(&rct);
    }
    else if (lHint == HINT_UPDATEOBJLIST && ph->GetArgs<HINT_UPDATEOBJLIST>().m_pPBoard == m_pPBoard)
    {
        POSITION pos;
        for (pos = ph->GetArgs<HINT_UPDATEOBJLIST>().m_pPtrList->GetHeadPosition(); pos != NULL; )
        {
            CDrawObj* pDObj = (CDrawObj*)ph->GetArgs<HINT_UPDATEOBJLIST>().m_pPtrList->GetNext(pos);
            ASSERT(pDObj != NULL);
            CRect rct = pDObj->GetEnclosingRect();  // In board coords.
            InvalidateWorkspaceRect(&rct);
        }
    }
    else if (lHint == HINT_UPDATEBOARD && ph->GetArgs<HINT_UPDATEBOARD>().m_pPBoard == m_pPBoard)
    {
        if (m_pBMap != NULL) delete m_pBMap;
        m_pBMap = NULL;
        Invalidate();
    }
    else if (lHint == HINT_ALWAYSUPDATE || lHint == HINT_GAMESTATEUSED)
        CScrollView::OnUpdate(pSender, lHint, pHint);
}

///////////////////////////////////////////////////////////////////////
// This message is sent when a document is being saved.
// WPARAM = CArchive*, LPARAM = 0 if save, 1 if restore

LRESULT CTinyBoardView::OnMessageWindowState(WPARAM wParam, LPARAM lParam)
{
    ASSERT(wParam != NULL);
    CArchive& ar = *((CArchive*)wParam);
    if (ar.IsStoring())
    {
        CPoint pnt = GetScrollPosition();
        ar << (DWORD)pnt.x;
        ar << (DWORD)pnt.y;
    }
    else
    {
        CPoint pnt;
        DWORD dwTmp;
        ar >> dwTmp; pnt.x = (LONG)dwTmp;
        ar >> dwTmp; pnt.y = (LONG)dwTmp;
        ScrollToPosition(pnt);
    }
    return (LRESULT)1;
}

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardView drawing

void CTinyBoardView::OnDraw(CDC* pDC)
{
    SetupPalette(pDC);          // (moved to top)
    if (m_pBMap == NULL)
        RegenCachedMap(pDC);
    ASSERT(m_pBMap != NULL);

    CDC      dcMem;
    CBitmap  bmMem;
    CRect    oRct;
    CRect    oRctSave;
    CBitmap* pPrvBMap;

    pDC->GetClipBox(&oRct);
    if (oRct.IsRectEmpty())
        return;                 // Nothing to do

    bmMem.Attach(Create16BitDIBSection(pDC->m_hDC, oRct.Width(), oRct.Height()));
    dcMem.CreateCompatibleDC(pDC);
    pPrvBMap = dcMem.SelectObject(&bmMem);
    dcMem.PatBlt(0, 0, oRct.Width(), oRct.Height(), WHITENESS);

    if (m_pPBoard->IsBoardRotated180())
    {
        oRctSave = oRct;
        CSize sizeBrd = m_pPBoard->GetBoard()->GetSize(smallScale);
        oRct = CRect(CPoint(sizeBrd.cx - oRct.left - oRct.Width(),
            sizeBrd.cy - oRct.top - oRct.Height()), oRct.Size());
    }

    dcMem.SetViewportOrg(-oRct.left, -oRct.top);
    SetupPalette(&dcMem);

    // Draw updated part of board image
    BitmapBlt(&dcMem, CPoint(0, 0), m_pBMap);

    // Draw pieces etc. (Need to rescale the DC and the update rect)

    CRect rct(&oRct);
    SetupDrawListDC(&dcMem, rct);

    m_pPBoard->Draw(&dcMem, &rct, smallScale);

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
        // Copy to output area
        pDC->BitBlt(oRct.left, oRct.top, oRct.Width(), oRct.Height(),
            &dcMem, oRct.left, oRct.top, SRCCOPY);
    }

    ResetPalette(&dcMem);
    dcMem.SelectObject(pPrvBMap);
}

void CTinyBoardView::DrawFullMap(CDC* pDC, CBitmap& bmap)
{
    CDC dcMem;

    CSize size = m_pPBoard->GetBoard()->GetSize(smallScale);

    bmap.Attach(Create16BitDIBSection(pDC->m_hDC, size.cx, size.cy));
    dcMem.CreateCompatibleDC(pDC);
    CBitmap* pPrvBMap = dcMem.SelectObject(&bmap);
    SetupPalette(&dcMem);

    // Draw updated part of board image
    BitmapBlt(&dcMem, CPoint(0, 0), m_pBMap);

    // Draw pieces etc. (Need to rescale the DC and the update rect)

    CRect rct(CPoint(0,0), size);
    SetupDrawListDC(&dcMem, rct);

    m_pPBoard->Draw(&dcMem, &rct, smallScale);

    RestoreDrawListDC(&dcMem);
    dcMem.SelectObject(pPrvBMap);
}

void CTinyBoardView::RegenCachedMap(CDC* pDC)
{
    BeginWaitCursor();
    SetupPalette(pDC);

    if (m_pBMap != NULL) delete m_pBMap;
    m_pBMap = new CBitmap;

    CBoard* pBoard = m_pPBoard->GetBoard();
    CSize size = pBoard->GetSize(smallScale);   // Get pixel size of board
    m_pBMap->Attach(Create16BitDIBSection(pDC->m_hDC, size.cx, size.cy));

    CDC dcMem;
    dcMem.CreateCompatibleDC(pDC);
    CBitmap* pPrvBMap = dcMem.SelectObject(m_pBMap);
    SetupPalette(&dcMem);

    CRect rct(CPoint(0, 0), size);
    pBoard->SetMaxDrawLayer();                  // Make sure all layers are drawn
    pBoard->Draw(&dcMem, &rct, smallScale, m_pPBoard->m_bSmallCellBorders);

    ResetPalette(&dcMem);
    dcMem.SelectObject(pPrvBMap);
    EndWaitCursor();
}

/////////////////////////////////////////////////////////////////////////////

void CTinyBoardView::SetupDrawListDC(CDC* pDC, CRect& rct)
{
    CSize wsize, vsize;
    m_pPBoard->GetBoard()->GetBoardArray()->
        GetBoardScaling(smallScale, wsize, vsize);

    pDC->SaveDC();
    pDC->SetMapMode(MM_ANISOTROPIC);
    pDC->SetWindowExt(wsize);
    pDC->SetViewportExt(vsize);

    ScaleRect(rct, wsize, vsize);
}

void CTinyBoardView::RestoreDrawListDC(CDC *pDC)
{
    pDC->RestoreDC(-1);
}

/////////////////////////////////////////////////////////////////////////////

void CTinyBoardView::WorkspaceToClient(CRect& rect)
{
    CPoint dpnt = GetDeviceScrollPosition();
    CSize wsize, vsize;
    m_pPBoard->GetBoard()->GetBoardArray()->
        GetBoardScaling(smallScale, wsize, vsize);
    if (m_pPBoard->IsBoardRotated180())
    {
        rect = CRect(wsize.cx - rect.left, wsize.cy - rect.top,
            wsize.cx - rect.right, wsize.cy - rect.bottom);
        rect.NormalizeRect();
    }
    ScaleRect(rect, vsize, wsize);
    rect -= dpnt;
}

void CTinyBoardView::InvalidateWorkspaceRect(const CRect* pRect, BOOL bErase)
{
    CRect rct(pRect);
    WorkspaceToClient(rct);
    InvalidateRect(&rct, bErase);
}

void CTinyBoardView::ClientToWorkspace(CPoint& pnt)
{
    pnt += GetDeviceScrollPosition();
    CSize wsize, vsize;
    m_pPBoard->GetBoard()->GetBoardArray()->
        GetBoardScaling(smallScale, wsize, vsize);
    ScalePoint(pnt, wsize, vsize);
    if (m_pPBoard->IsBoardRotated180())
        pnt = CPoint(wsize.cx - pnt.x, wsize.cy - pnt.y);
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
CGamDoc* CTinyBoardView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGamDoc)));
    return (CGamDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardView message handlers

void CTinyBoardView::OnLButtonDown(UINT nFlags, CPoint point)
{
    ClientToWorkspace(point);
    CFrameWnd* pFrame = GetParentFrame();
    pFrame->SendMessage(WM_CENTERBOARDONPOINT, (WPARAM)&point);
}

void CTinyBoardView::OnRButtonDown(UINT nFlags, CPoint point)
{
    if (m_pBMap == NULL)
        return;

    CTinyBoardPopup* pTBrd = new CTinyBoardPopup;

    {
        CWindowDC dcRef(NULL);
        SetupPalette(&dcRef);
        DrawFullMap(&dcRef, pTBrd->m_bmap);
        ResetPalette(&dcRef);
    }

    pTBrd->m_pWnd = GetParentFrame();
    m_pPBoard->GetBoard()->GetBoardArray()->
        GetBoardScaling(smallScale, pTBrd->m_wsize, pTBrd->m_vsize);
    pTBrd->m_bRotate180 = m_pPBoard->IsBoardRotated180();
    CRect rct;
    GetWindowRect(&rct);
    pTBrd->Create(GetMainFrame(), rct.CenterPoint());

    CScrollView::OnRButtonDown(nFlags, point);
}

int CTinyBoardView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    // We don't want the frame to ever consider this view to be the
    // "active" view.
    return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}
