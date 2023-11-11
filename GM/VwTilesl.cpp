// VwTilesl.cpp : implementation file
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "CDib.h"
#include    "VwTilesl.h"
#include    "VwBitedt.h"
#include    "DlgTilsz.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int xGap = 10;
const int yGap = 10;
const int nBorderWidth = 5;

/////////////////////////////////////////////////////////////////////////////
// CTileSelView

IMPLEMENT_DYNCREATE(CTileSelView, CScrollView)

CTileSelView::CTileSelView() :
    m_bmFull(MakeOwner<CBitmap>()),
    m_bmHalf(MakeOwner<CBitmap>()),
    m_bmSmall(MakeOwner<CBitmap>())
{
    m_pTileMgr = NULL;
    m_pEditView = NULL;
    m_tid = nullTid;
    m_bNoUpdate = FALSE;
    m_pBmFullUndo = NULL;
    m_pBmHalfUndo = NULL;
}

BEGIN_MESSAGE_MAP(CTileSelView, CScrollView)
    //{{AFX_MSG_MAP(CTileSelView)
    ON_MESSAGE(WM_SETCOLOR, OnSetColor)
    ON_MESSAGE(WM_SETCUSTOMCOLOR, OnSetCustomColors)
    ON_MESSAGE(WM_SETLINEWIDTH, OnSetLineWidth)
    ON_UPDATE_COMMAND_UI(ID_COLOR_FOREGROUND, OnUpdateColorForeground)
    ON_UPDATE_COMMAND_UI(ID_COLOR_BACKGROUND, OnUpdateColorBackground)
    ON_UPDATE_COMMAND_UI(ID_COLOR_TRANSPARENT, OnUpdateColorTransparent)
    ON_UPDATE_COMMAND_UI(ID_COLOR_CUSTOM, OnUpdateColorCustom)
    ON_UPDATE_COMMAND_UI(ID_LINE_WIDTH, OnUpdateLineWidth)
    ON_WM_LBUTTONDOWN()
    ON_WM_DESTROY()
    ON_UPDATE_COMMAND_UI(ID_ITOOL_PENCIL, OnUpdateToolPalette)
    ON_COMMAND_EX(ID_ITOOL_SELECT, OnToolPalette)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND_EX(ID_ITOOL_BRUSH, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_FILL, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_TEXT, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_LINE, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_RECT, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_OVAL, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_FILLRECT, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_FILLOVAL, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_DROPPER, OnToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_SELECT, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_BRUSH, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_FILL, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_TEXT, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_LINE, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_RECT, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_OVAL, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_FILLRECT, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_FILLOVAL, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_DROPPER, OnUpdateToolPalette)
    ON_COMMAND(ID_VIEW_TOGGLE_SCALE, OnViewToggleScale)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

BOOL CTileSelView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CScrollView::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)(COLOR_BTNFACE + 1));
    return TRUE;
}

void CTileSelView::OnInitialUpdate()
{
    m_pTileMgr = GetDocument().GetTileManager();
    ASSERT(m_pTileMgr != NULL);
    CScrollView::OnInitialUpdate();
    m_tid = static_cast<TileID>(reinterpret_cast<uintptr_t>(GetDocument().GetCreateParameter()));
    ASSERT(m_tid != nullTid);

    // Fetch full scale tile
    CTile tile = m_pTileMgr->GetTile(m_tid, fullScale);
    m_bmFull = tile.CreateBitmapOfTile();
    m_sizeFull = tile.GetSize();

    // Fetch half scale tile
    tile = m_pTileMgr->GetTile(m_tid, halfScale);
    m_bmHalf = tile.CreateBitmapOfTile();
    m_sizeHalf = tile.GetSize();

    // Fetch small scale color and create bogus tile for color editing
    tile = m_pTileMgr->GetTile(m_tid, smallScale);
    m_crSmall = tile.GetSmallColor();
    m_sizeSmall = CSize(8, 8);
    m_bmSmall = CreateColorBitmap(m_sizeSmall, m_crSmall);

    // Setup rectangles
    CalcViewLayout();

    // Finally hand the full size tile to the bit editor.
    ASSERT(m_pEditView != NULL);
    SelectCurrentBitmap(fullScale);
    m_pEditView->SetCurrentBitmap(m_tid, &*m_bmFull);

    SetScrollSizes(MM_TEXT, m_sizeSelArea, CSize(64, 64), CSize(8, 8));
}

void CTileSelView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    WORD wHint = LOWORD(lHint);

    if (wHint == HINT_TILEDELETED)
    {
        if (static_cast<CGmBoxHint*>(pHint)->GetArgs<HINT_TILEDELETED>().m_tid == m_tid)
        {
            m_bNoUpdate = TRUE;
            CFrameWnd* pFrm = GetParentFrame();
            ASSERT(pFrm != NULL);
            pFrm->PostMessage(WM_CLOSE, 0, 0L);
        }
    }
    else if (wHint == HINT_TILESETDELETED && !m_pTileMgr->IsTileIDValid(m_tid))
    {
        m_bNoUpdate = TRUE;
        CFrameWnd* pFrm = GetParentFrame();
        ASSERT(pFrm != NULL);
        pFrm->PostMessage(WM_CLOSE, 0, 0L);
    }
    else if (wHint == HINT_FORCETILEUPDATE)
    {
        UpdateDocumentTiles();
    }
    else if (wHint == HINT_ALWAYSUPDATE)
        CScrollView::OnUpdate(pSender, lHint, pHint);
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::SetBitEditor(CBitEditView& pEditView)
{
    m_pEditView = &pEditView;
}

/////////////////////////////////////////////////////////////////////////////
// CTileSelView drawing

void CTileSelView::DrawTile(CDC& pDC, CBitmap& pBMap, CRect rct)
{
    CBrush brBlack;
    brBlack.CreateStockObject(BLACK_BRUSH);
    g_gt.mDC1.SelectObject(&pBMap);
    pDC.BitBlt(rct.left, rct.top, rct.Width(), rct.Height(),
        &g_gt.mDC1, 0, 0, SRCCOPY);
    rct.InflateRect(1, 1);
    pDC.FrameRect(&rct, &brBlack);
}

void CTileSelView::OnDraw(CDC* pDC)
{
    SetupPalette(*pDC);
    CRect rctActive;

    if (m_eCurTile != fullScale)
        DrawTile(*pDC, *m_bmFull, m_rctFull);
    else
        rctActive = m_rctFull;

    if (m_eCurTile != halfScale)
        DrawTile(*pDC, *m_bmHalf, m_rctHalf);
    else
        rctActive = m_rctHalf;

    if (m_eCurTile != smallScale)
        DrawTile(*pDC, *m_bmSmall, m_rctSmall);
    else
        rctActive = m_rctSmall;

    CBitmap& pbm = *m_pEditView->GetCurrentViewBitmap();
    ASSERT(pbm.m_hObject != NULL);

    DrawTile(*pDC, pbm, rctActive);

    rctActive.InflateRect(nBorderWidth + 1, nBorderWidth + 1);
    Draw25PctPatBorder(*this, *pDC, rctActive, nBorderWidth);

    ResetPalette(*pDC);
    g_gt.SelectSafeObjectsForDC1();
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::UpdateViewPixel(CPoint pt, UINT nBrushSize, const CBrush *pBrush)
{
    int nSize;
    int nSizeX;
    int nSizeY;

    CDC* pDC = GetDC();
    SetupPalette(*pDC);

    CPoint pnt = GetActiveTileLoc();
    CSize size = m_pEditView->GetBitmapSize();
    CBrush* pPrvBrush = CBrush::FromHandle(static_cast<HBRUSH>(pDC->SelectObject(*pBrush)));

    nSizeX = pt.x - nBrushSize / 2;
    nSizeY = pt.y - nBrushSize / 2;

    CPoint ptScroll = GetDeviceScrollPosition();

    pDC->PatBlt(pnt.x + (nSizeX >= 0 ? nSizeX : 0) - ptScroll.x,
        pnt.y + (nSizeY >=  0 ? nSizeY :  0) - ptScroll.y,
        ((nSize = size.cx - nSizeX) >= (int)nBrushSize ? nBrushSize : nSize),
        ((nSize = size.cy - nSizeY) >= (int)nBrushSize ? nBrushSize : nSize),
        PATCOPY);

    pDC->SelectObject(pPrvBrush);
    ResetPalette(*pDC);
    ReleaseDC(pDC);
}

void CTileSelView::UpdateViewImage(CRect* pRct, BOOL bImmed /* = FALSE */)
{
    CSize size = m_pEditView->GetBitmapSize();
    CRect rct = GetActiveTileRect();
    if (pRct)
        rct = *pRct;
    GetDocument().SetModifiedFlag();
    InvalidateRect(&rct, FALSE);
    if (bImmed)
        UpdateWindow();
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::UpdateDocumentTiles()
{
    CGamDoc& pDoc = GetDocument();
    // Make sure our bitmaps are up to date.
    GetActiveBitmap() = CloneBitmap(*m_pEditView->GetCurrentViewBitmap());

    // Get small scale color from tiny bitmap and update tile
    g_gt.mDC1.SelectObject(&*m_bmSmall);
    SetupPalette(g_gt.mDC1);
    COLORREF crSmall = g_gt.mDC1.GetPixel(0, 0);
    g_gt.SelectSafeObjectsForDC1();

    // The update happens here...
    m_pTileMgr->UpdateTile(m_tid, *m_bmFull, *m_bmHalf, crSmall);

    // Finally handle various notifications
    CGmBoxHint hint;
    hint.GetArgs<HINT_TILEMODIFIED>().m_tid = m_tid;
    pDoc.UpdateAllViews(this, HINT_TILEMODIFIED, &hint);
    pDoc.SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::DoTileResizeDialog()
{
    CResizeTileDialog dlg;
    GetActiveBitmap() = CloneBitmap(*m_pEditView->GetCurrentViewBitmap());
    dlg.m_pBMgr = GetDocument().GetBoardManager();
    dlg.m_bRescaleBMaps = TRUE;
    dlg.m_nWidth = m_sizeFull.cx;
    dlg.m_nHeight = m_sizeFull.cy;
    if (dlg.DoModal() == IDOK)
    {
        // Copy current bitmaps for undo
        PurgeUndo();            // Make sure no mem leaks.
        m_pBmFullUndo = CloneBitmap(*m_bmFull);
        m_pBmHalfUndo = CloneBitmap(*m_bmHalf);

        OwnerOrNullPtr<CBitmap> bmFull(MakeOwner<CBitmap>());
        OwnerOrNullPtr<CBitmap> bmHalf(MakeOwner<CBitmap>());
        m_sizeFull = CSize(dlg.m_nWidth, dlg.m_nHeight);
        m_sizeHalf = CSize(dlg.m_nHalfWidth, dlg.m_nHalfHeight);

        if (dlg.m_bRescaleBMaps)
        {
            bmFull = CloneScaledBitmap(*m_bmFull, m_sizeFull);
            bmHalf = CloneScaledBitmap(*m_bmHalf, m_sizeHalf);
        }
        else
        {
            bmFull = CreateColorBitmap(m_sizeFull, RGB(255, 255, 255));
            bmHalf = CreateColorBitmap(m_sizeHalf, RGB(255, 255, 255));
            MergeBitmap(*bmFull, *m_bmFull, CPoint(0,0), noColor);
            MergeBitmap(*bmHalf, *m_bmHalf, CPoint(0,0), noColor);
        }
        m_bmFull = std::move(bmFull);
        m_bmHalf = std::move(bmHalf);
        // Recompute selection window layout.
        CalcViewLayout();

        // Finally hand the full size tile to the bit editor.
        SelectCurrentBitmap(fullScale);
        m_pEditView->SetCurrentBitmap(m_tid, &*m_bmFull);
        Invalidate();
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::DoTileRotation(int nAngle)
{
    CDib dibFull(*m_bmFull, GetAppPalette());
    CDib dibHalf(*m_bmHalf, GetAppPalette());
    GetActiveBitmap() = CloneBitmap(*m_pEditView->GetCurrentViewBitmap());
    if (!dibFull)
        return;             // MEMORY ERROR
    if(!dibHalf)
        return;             // MEMORY ERROR

    CDib pDib = Rotate16BitDib(dibFull, nAngle, RGB(255, 255, 255));
    OwnerOrNullPtr<CBitmap> pbmFull = pDib.DIBToBitmap(GetAppPalette());

    pDib = Rotate16BitDib(dibHalf, nAngle, RGB(255, 255, 255));
    OwnerOrNullPtr<CBitmap> pbmHalf = pDib.DIBToBitmap(GetAppPalette());

    m_bmFull = std::move(pbmFull);
    m_bmHalf = std::move(pbmHalf);
    // Recompute selection window layout.
    CalcViewLayout();
    // Finally hand the full size tile to the bit editor.
    SelectCurrentBitmap(fullScale);
    m_pEditView->SetCurrentBitmap(m_tid, &*m_bmFull);
    Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CTileSelView::IsUndoAvailable() const
{
    return m_pBmFullUndo != NULL;
}

void CTileSelView::PurgeUndo()
{
    m_pBmFullUndo = NULL;
    m_pBmHalfUndo = NULL;
}

void CTileSelView::RestoreFromUndo()
{
    ASSERT(m_pBmFullUndo != NULL);
    ASSERT(m_pBmHalfUndo != NULL);
    if (m_pBmFullUndo == NULL)
        return;

    m_bmFull = std::move(m_pBmFullUndo);
    m_bmHalf = std::move(m_pBmHalfUndo);

    PurgeUndo();            // Dump empty objects.

    // Recompute selection window layout.
    CalcViewLayout();

    // Finally hand the full size tile to the bit editor.
    SelectCurrentBitmap(fullScale);
    m_pEditView->SetCurrentBitmap(m_tid, &*m_bmFull);
    Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::CalcViewLayout()
{
    // Redo sizes since some callers count on it.

    BITMAP bmi;
    m_bmFull->GetObject(sizeof(bmi), &bmi);
    m_sizeFull = CSize(bmi.bmWidth, bmi.bmHeight);
    m_bmHalf->GetObject(sizeof(bmi), &bmi);
    m_sizeHalf = CSize(bmi.bmWidth, bmi.bmHeight);

    m_rctFull.SetRect(0, 0, m_sizeFull.cx, m_sizeFull.cy);
    m_rctFull.OffsetRect(xGap, yGap);
    m_rctHalf.SetRect(0, 0, m_sizeHalf.cx, m_sizeHalf.cy);
    m_rctHalf.OffsetRect(xGap + (m_sizeFull.cx - m_sizeHalf.cx) / 2,
        yGap + m_rctFull.bottom + 2);
    m_rctSmall.SetRect(0, 0, m_sizeSmall.cx, m_sizeSmall.cy);
    m_rctSmall.OffsetRect(xGap + (m_sizeFull.cx - m_sizeSmall.cx) / 2,
        yGap + m_rctHalf.bottom + 2);

    m_sizeSelArea.cx = m_rctFull.right + nBorderWidth;
    m_sizeSelArea.cy = m_rctSmall.bottom + nBorderWidth;

#ifdef WANTHORIZ_VERSION
    m_rctFull.SetRect(0, 0, m_sizeFull.cx, m_sizeFull.cy);
    m_rctFull.OffsetRect(xGap, yGap);
    m_rctHalf.SetRect(0, 0, m_sizeHalf.cx, m_sizeHalf.cy);
    m_rctHalf.OffsetRect(xGap + m_rctFull.right + 2,
        yGap + (m_sizeFull.cy - m_sizeHalf.cy) / 2);
    m_rctSmall.SetRect(0, 0, m_sizeSmall.cx, m_sizeSmall.cy);
    m_rctSmall.OffsetRect(xGap + m_rctHalf.right + 2,
        yGap + (m_sizeFull.cy - m_sizeSmall.cy) / 2);
#endif
}

/////////////////////////////////////////////////////////////////////////////

CRect CTileSelView::GetActiveTileRect() const
{
    if (m_eCurTile == fullScale)
        return m_rctFull;
    else if (m_eCurTile == halfScale)
        return m_rctHalf;
    else
        return m_rctSmall;
}

CPoint CTileSelView::GetActiveTileLoc() const
{
    if (m_eCurTile == fullScale)
        return m_rctFull.TopLeft();
    else if (m_eCurTile == halfScale)
        return m_rctHalf.TopLeft();
    else
        return m_rctSmall.TopLeft();
}

OwnerPtr<CBitmap>& CTileSelView::GetActiveBitmap()
{
    if (m_eCurTile == fullScale)
        return m_bmFull;
    else if (m_eCurTile == halfScale)
        return m_bmHalf;
    else
        return m_bmSmall;
}

void CTileSelView::SelectCurrentBitmap(TileScale eScale)
{
    if (eScale == fullScale)
        m_pEditView->SetCurrentBitmap(m_tid, &*m_bmFull);
    else if (eScale == halfScale)
        m_pEditView->SetCurrentBitmap(m_tid, &*m_bmHalf);
    else
        m_pEditView->SetCurrentBitmap(m_tid, &*m_bmSmall, TRUE);
    m_eCurTile = eScale;
}

/////////////////////////////////////////////////////////////////////////////
// CTileSelView message handlers

void CTileSelView::OnLButtonDown(UINT nFlags, CPoint point)
{
    point += GetDeviceScrollPosition();
    TileScale eScale;
    if (m_rctFull.PtInRect(point))
        eScale = fullScale;
    else if (m_rctHalf.PtInRect(point))
        eScale = halfScale;
    else if (m_rctSmall.PtInRect(point))
        eScale = smallScale;
    else
    {
        CScrollView::OnLButtonDown(nFlags, point);
        GetParentFrame()->SetActiveView(m_pEditView);
        return;
    }
    GetActiveBitmap() = CloneBitmap(*m_pEditView->GetCurrentViewBitmap());

    SelectCurrentBitmap(eScale);
    Invalidate();
    GetParentFrame()->SetActiveView(m_pEditView);
}

void CTileSelView::OnDestroy()
{
    if (!m_bNoUpdate)
    {
        // Update master copies...
        UpdateDocumentTiles();
    }
    // Let destroy continue...
    CScrollView::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CTileSelView command handlers
// Hot potato most of these to the editor view.

LRESULT CTileSelView::OnSetColor(WPARAM wParam, LPARAM lParam)
{
    return m_pEditView->OnSetColor(wParam, lParam);
}

LRESULT CTileSelView::OnSetCustomColors(WPARAM wParam, LPARAM lParam)
{
    return m_pEditView->OnSetCustomColors(wParam, lParam);
}

LRESULT CTileSelView::OnSetLineWidth(WPARAM wParam, LPARAM lParam)
{
    return m_pEditView->OnSetLineWidth(wParam, lParam);
}

void CTileSelView::OnUpdateColorForeground(CCmdUI* pCmdUI)
{
    m_pEditView->OnUpdateColorForeground(pCmdUI);
}

void CTileSelView::OnUpdateColorBackground(CCmdUI* pCmdUI)
{
    m_pEditView->OnUpdateColorBackground(pCmdUI);
}

void CTileSelView::OnUpdateColorTransparent(CCmdUI* pCmdUI)
{
    m_pEditView->OnUpdateColorTransparent(pCmdUI);
}

void CTileSelView::OnUpdateColorCustom(CCmdUI* pCmdUI)
{
    m_pEditView->OnUpdateColorCustom(pCmdUI);
}

void CTileSelView::OnUpdateLineWidth(CCmdUI* pCmdUI)
{
    m_pEditView->OnUpdateLineWidth(pCmdUI);
}

void CTileSelView::OnUpdateToolPalette(CCmdUI* pCmdUI)
{
    m_pEditView->OnUpdateToolPalette(pCmdUI);
}

BOOL CTileSelView::OnToolPalette(UINT id)
{
    return m_pEditView->OnToolPalette(id);
}

void CTileSelView::OnEditPaste()
{
    m_pEditView->OnEditPaste();
}

void CTileSelView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
    m_pEditView->OnUpdateEditPaste(pCmdUI);
}

void CTileSelView::OnEditCopy()
{
    m_pEditView->OnEditCopy();
}

void CTileSelView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
    m_pEditView->OnUpdateEditCopy(pCmdUI);
}

void CTileSelView::OnEditUndo()
{
    if (IsUndoAvailable())      // Us first.
        RestoreFromUndo();
    else
        m_pEditView->OnEditUndo();
}

void CTileSelView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
    if (IsUndoAvailable())      // Us first.
        pCmdUI->Enable(TRUE);
    else
        m_pEditView->OnUpdateEditUndo(pCmdUI);
}

void CTileSelView::OnViewToggleScale()
{
    m_pEditView->OnViewToggleScale();
}

