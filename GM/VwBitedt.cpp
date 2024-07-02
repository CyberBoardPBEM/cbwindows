// VwBitedt.cpp : implementation file
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
#include    "CellForm.h"
#include    "VwBitedt.h"
#include    "VwTilesl.h"
#include    "PalColor.h"
#include    "ClipBrd.h"
#include    "DlgBmask.h"
#include    "DlgPaste.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

const int xBorder = 6;
const int yBorder = 6;

const size_t maxUndoLevels = size_t(8);

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CBitEditView, CScrollView)
    //{{AFX_MSG_MAP(CBitEditView)
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_IMAGE_GRIDLINES, OnImageGridLines)
    ON_COMMAND_EX(ID_ITOOL_PENCIL, OnToolPalette)
    ON_MESSAGE(WM_SETCOLOR, OnSetColor)
    ON_MESSAGE(WM_SETCUSTOMCOLOR, OnSetCustomColors)
    ON_MESSAGE(WM_SETLINEWIDTH, OnSetLineWidth)
    ON_UPDATE_COMMAND_UI(ID_IMAGE_GRIDLINES, OnUpdateImageGridLines)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_PENCIL, OnUpdateToolPalette)
    ON_UPDATE_COMMAND_UI(ID_COLOR_FOREGROUND, OnUpdateColorForeground)
    ON_UPDATE_COMMAND_UI(ID_COLOR_BACKGROUND, OnUpdateColorBackground)
    ON_UPDATE_COMMAND_UI(ID_COLOR_TRANSPARENT, OnUpdateColorTransparent)
    ON_UPDATE_COMMAND_UI(ID_COLOR_CUSTOM, OnUpdateColorCustom)
    ON_UPDATE_COMMAND_UI(ID_LINE_WIDTH, OnUpdateLineWidth)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_COMMAND(ID_IMAGE_BOARDMASK, OnImageBoardMask)
    ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomIn)
    ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMIN, OnUpdateViewZoomIn)
    ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomOut)
    ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMOUT, OnUpdateViewZoomOut)
    ON_UPDATE_COMMAND_UI(ID_IMAGE_BOARDMASK, OnUpdateImageBoardMask)
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_COMMAND(ID_DWG_FONT, OnDwgFont)
    ON_WM_CHAR()
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_CELLNUM, OnUpdateIndicatorCellNum)
    ON_WM_CONTEXTMENU()
    ON_COMMAND_EX(ID_ITOOL_SELECT, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_BRUSH, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_FILL, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_TEXT, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_LINE, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_RECT, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_OVAL, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_FILLRECT, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_FILLOVAL, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_DROPPER, OnToolPalette)
    ON_COMMAND_EX(ID_ITOOL_COLORCHANGE, OnToolPalette)
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
    ON_UPDATE_COMMAND_UI(ID_ITOOL_COLORCHANGE, OnUpdateToolPalette)
    ON_COMMAND(ID_VIEW_TOGGLE_SCALE, OnViewToggleScale)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBitEditView

IMPLEMENT_DYNCREATE(CBitEditView, CScrollView)

CBitEditView::CBitEditView() :
    m_bmMaster(MakeOwner<CBitmap>()),
    m_bmView(MakeOwner<CBitmap>()),
    m_bmPaste(MakeOwner<CBitmap>())
{
    m_pSelView = NULL;
    m_nZoom = 6;
    m_bGridVisible = TRUE;
    m_tid = nullTid;
    m_bFillOnly = FALSE;
    // ------- //
    m_nTxtExtent = 0;
    m_fontID = 0;
    // ------- //
    m_nCurToolID = ID_ITOOL_PENCIL;
    m_nLastToolID = ID_ITOOL_PENCIL;
    m_bSelectCapture = FALSE;
    m_rctPaste.SetRectEmpty();
}

CBitEditView::~CBitEditView()
{
    PurgeUndo();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CBitEditView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CScrollView::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)(COLOR_BTNFACE + 1));

    return TRUE;
}

void CBitEditView::OnInitialUpdate()
{
    CScrollView::OnInitialUpdate();
    m_pTMgr = GetDocument().GetTileManager();
    UpdateFontInfo();
    RecalcScrollLimits();
}

void CBitEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    WORD wHint = LOWORD(lHint);
    // Update for nonspecific notification only
    if (wHint == HINT_ALWAYSUPDATE)
        CScrollView::OnUpdate(pSender, lHint, pHint);
}

/////////////////////////////////////////////////////////////////////////////
// CBitEditView drawing

void CBitEditView::OnDraw(CDC* pDC)
{
    if (m_bmView->m_hObject == NULL)
        return;                 // Nothing to draw.

    CSize size(m_size.cx * m_nZoom, m_size.cy * m_nZoom);

    if (m_nCurToolID != ID_ITOOL_SELECT || m_rctPaste.IsRectEmpty())
    {
        // Handle fancy focus rect
        if (GetParentFrame()->GetActiveView() == this)
        {
            CRect rct(0, 0, 2 * xBorder + size.cx, 2 * yBorder + size.cy);
            Draw25PctPatBorder(*this, *pDC, rct, xBorder);
        }
    }

    // Now handle the actual bitmap

    g_gt.mDC1.SelectObject(*m_bmView);
    pDC->SetStretchBltMode(COLORONCOLOR);

    if (m_bGridVisible && m_nZoom > 2)
    {
        OwnerPtr<CBitmap> bmTmp = CreateRGBDIBSection(
            size.cx + 1, size.cy + 1);
        g_gt.mDC2.SelectObject(&*bmTmp);
        g_gt.mDC2.StretchBlt(0, 0, size.cx, size.cy, &g_gt.mDC1, 0, 0,
            m_size.cx, m_size.cy, SRCCOPY);
        for (int i = 0; i <= size.cx; i += m_nZoom)
            g_gt.mDC2.PatBlt(i, 0, 1, size.cy + 1, BLACKNESS);
        for (int i = 0; i <= size.cy; i += m_nZoom)
            g_gt.mDC2.PatBlt(0, i, size.cx + 1, 1, BLACKNESS);
        pDC->BitBlt(xBorder - 1, yBorder - 1, size.cx + 1, size.cy + 1, &g_gt.mDC2,
            0, 0, SRCCOPY);
        g_gt.SelectSafeObjectsForDC2();
    }
    else
    {
        pDC->StretchBlt(xBorder, yBorder, size.cx, size.cy, &g_gt.mDC1, 0, 0,
            m_size.cx, m_size.cy, SRCCOPY);
        pDC->PatBlt(xBorder-1, yBorder-1, 1, size.cy + 2, BLACKNESS);
        pDC->PatBlt(xBorder-1, yBorder-1, size.cx + 2, 1, BLACKNESS);
        pDC->PatBlt(xBorder + size.cx, yBorder-1, 1, size.cy + 2, BLACKNESS);
        pDC->PatBlt(xBorder-1, yBorder + size.cy, size.cx + 2, 1, BLACKNESS);
    }
    if (m_nCurToolID == ID_ITOOL_SELECT && !m_bSelectCapture &&
        m_bmPaste->m_hObject != NULL && !m_rctPaste.IsRectEmpty())
    {
        // Handle fancy focus rect
        if (GetParentFrame()->GetActiveView() == this)
        {
            // Convert paste rect to view coordinates.
            CRect rct = GetZoomedSelectBorderRect();
            Draw25PctPatBorder(*this, *pDC, rct, xBorder);
        }
    }
    g_gt.SelectSafeObjectsForDC1();  // This does ResetPalette for mDC1
}

void CBitEditView::OnActivateView(BOOL bActivate, CView *pActivateView,
    CView* pDeactivateView)
{
    CScrollView::OnActivateView(bActivate, pActivateView, pDeactivateView);
    if (pActivateView == pDeactivateView)
        return;
//  if (m_nCurToolID == ID_ITOOL_TEXT)
//  {
//      SetFocus();
//      SetTextCaretPos(m_ptCaret);
//  }
    Invalidate();       // Later only mess with focus rect.
}

/////////////////////////////////////////////////////////////////////////////

void CBitEditView::ClientToWorkspace(CPoint& pnt) const
{
    pnt += (CSize)GetDeviceScrollPosition();
}

void CBitEditView::WorkspaceToClient(CPoint& pnt) const
{
    pnt -= (CSize)GetDeviceScrollPosition();
}

void CBitEditView::WorkspaceToClient(CRect& rct) const
{
    rct -= GetDeviceScrollPosition();
}

/////////////////////////////////////////////////////////////////////////////
// Draw tool support.
// points are in bit image coordinates

void CBitEditView::DrawImageLine(CPoint startPt, CPoint curPt, UINT nSize)
{
    SetViewImageFromMasterImage();          // Get fresh original

    g_gt.mDC1.SelectObject(*m_bmView);
    CBrush* pPrvBrush = CBrush::FromHandle(static_cast<HBRUSH>(g_gt.mDC1.SelectObject(m_pTMgr->GetForeBrush())));

    if (nSize == 0)
        nSize = 1;          // Zero width is same as one width

    int x = startPt.x;
    int y = startPt.y;
    DELTAGEN dg;

    DeltaGenInit(&dg, startPt.x, startPt.y, curPt.x, curPt.y, &x, &y);
    do
    {
        if (x < m_size.cx && y < m_size.cy)
        {
            g_gt.mDC1.PatBlt(x - nSize / 2, y - nSize / 2,
                nSize, nSize, PATCOPY);
        }
    }   while (DeltaGen(&dg, &x, &y));

    g_gt.mDC1.SelectObject(pPrvBrush);
    g_gt.SelectSafeObjectsForDC1();

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage(NULL, TRUE);
}

// If nSize is Zero. The rect is filled. Otherwise it is a frame.
void CBitEditView::DrawImageSelectRect(CPoint startPt, CPoint curPt)
{
    SetViewImageFromMasterImage();          // Get fresh original

    g_gt.mDC1.SelectObject(*m_bmView);

    CRect rct(startPt.x, startPt.y, curPt.x, curPt.y);
    rct.NormalizeRect();
    if (rct.Width() != 0 && rct.Height() != 0)
    {
        g_gt.mDC1.PatBlt(rct.left, rct.top, rct.Width(), 1, DSTINVERT);
        g_gt.mDC1.PatBlt(rct.right, rct.top, 1, rct.Height(), DSTINVERT);
        g_gt.mDC1.PatBlt(rct.left+1, rct.bottom, rct.Width(), 1, DSTINVERT);
        g_gt.mDC1.PatBlt(rct.left, rct.top+1, 1, rct.Height(), DSTINVERT);
    }
    g_gt.SelectSafeObjectsForDC1();

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage(NULL, TRUE);
}

void CBitEditView::DrawPastedImage()
{
    ASSERT(m_bmPaste->m_hObject != NULL);
    SetViewImageFromMasterImage();          // Get fresh original
    MergeBitmap(*m_bmView, *m_bmPaste, m_rctPaste.TopLeft());

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage(NULL, TRUE);
}

// If nSize is Zero. The rect is filled. Otherwise it is a frame.
void CBitEditView::DrawImageRect(CPoint startPt, CPoint curPt, UINT nSize)
{
    SetViewImageFromMasterImage();          // Get fresh original

    g_gt.mDC1.SelectObject(*m_bmView);

    CRect rct(startPt.x, startPt.y, curPt.x, curPt.y);
    rct.NormalizeRect();
    if (nSize == 0)
    {
        // const_cast would probably work, but this is guaranteed safe
        OwnerPtr<CBrush> temp = Clone(m_pTMgr->GetForeBrush());
        g_gt.mDC1.FillRect(&rct, &*temp);
        g_gt.mDC1.FrameRect(&rct, &*temp);
    }
    else
    {
        CPen pen(PS_SOLID, nSize, m_pTMgr->GetForeColor());
        CPen* pPrvPen = g_gt.mDC1.SelectObject(&pen);
        CBrush* pPrvBrsh = (CBrush*)g_gt.mDC1.SelectStockObject(NULL_BRUSH);

        g_gt.mDC1.Rectangle(&rct);

        g_gt.mDC1.SelectObject(pPrvPen);
        g_gt.mDC1.SelectObject(pPrvBrsh);
    }

    g_gt.SelectSafeObjectsForDC1();

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage(NULL, TRUE);
}

// If nSize is Zero. The ellipse is filled. Otherwise it is a frame.
void CBitEditView::DrawImageEllipse(CPoint startPt, CPoint curPt, UINT nSize)
{
    SetViewImageFromMasterImage();          // Get fresh original

    g_gt.mDC1.SelectObject(*m_bmView);

    CRect rct(startPt.x, startPt.y, curPt.x, curPt.y);
    rct.NormalizeRect();
    CPen pen(PS_SOLID, nSize, m_pTMgr->GetForeColor());
    CBrush* pPrvBrsh;
    if (nSize == 0)
        pPrvBrsh = CBrush::FromHandle(static_cast<HBRUSH>(g_gt.mDC1.SelectObject(m_pTMgr->GetForeBrush())));
    else
        pPrvBrsh = (CBrush*)g_gt.mDC1.SelectStockObject(NULL_BRUSH);
    CPen* pPrvPen = g_gt.mDC1.SelectObject(&pen);

    g_gt.mDC1.Ellipse(&rct);

    g_gt.mDC1.SelectObject(pPrvPen);
    g_gt.mDC1.SelectObject(pPrvBrsh);

    g_gt.SelectSafeObjectsForDC1();

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage(NULL, TRUE);
}

void CBitEditView::DrawImageFill(CPoint pt)
{
    SetViewImageFromMasterImage();          // Get fresh original
    g_gt.mDC1.SelectObject(*m_bmView);

    CBrush* pBrush = CBrush::FromHandle(static_cast<HBRUSH>(g_gt.mDC1.SelectObject(m_pTMgr->GetForeBrush())));

    g_gt.mDC1.ExtFloodFill(pt.x, pt.y, g_gt.mDC1.GetPixel(pt), FLOODFILLSURFACE);

    g_gt.mDC1.SelectObject(pBrush);

    g_gt.SelectSafeObjectsForDC1();
    g_gt.SelectSafeObjectsForDC2();

    SetMasterImageFromViewImage();
    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage(NULL, TRUE);
}

void CBitEditView::DrawImageChangeColor(CPoint pt)
{
    SetViewImageFromMasterImage();          // Get fresh original

    g_gt.mDC1.SelectObject(*m_bmView);

    COLORREF crHit = g_gt.mDC1.GetPixel(pt);
    COLORREF crNew = m_pTMgr->GetForeColor();

    BITMAP bmap;
    memset(&bmap, 0, sizeof(BITMAP));
    VERIFY(m_bmView->GetObject(sizeof(BITMAP), &bmap));

    ASSERT(bmap.bmBits != NULL);
    ASSERT(bmap.bmBitsPixel == 24);
    ASSERT(bmap.bmWidthBytes > 1);
    ASSERT(bmap.bmPlanes == 1);

    std::byte* pxlRowStart = static_cast<std::byte*>(bmap.bmBits);
    ptrdiff_t pxlStride = bmap.bmWidthBytes;
    for (int y = 0 ; y < bmap.bmHeight ; ++y)
    {
        WIN_RGBTRIO* pxl = reinterpret_cast<WIN_RGBTRIO*>(pxlRowStart);
        for (int x = 0 ; x < bmap.bmWidth ; ++x)
        {
            COLORREF cr = *pxl;
            if (cr == crHit)
            {
                *pxl = crNew;
            }
            ++pxl;
        }
        pxlRowStart += pxlStride;
    }
    ::GdiFlush();

    g_gt.SelectSafeObjectsForDC1();
    SetMasterImageFromViewImage();
    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage(NULL, TRUE);
}

// points are in bit image coordinates
void CBitEditView::DrawImageToPixel(CPoint prvPt, CPoint curPt, UINT nSize)
{
    DELTAGEN dg;
    int x = curPt.x;
    int y = curPt.y;
    DeltaGenInit(&dg, prvPt.x, prvPt.y, curPt.x, curPt.y, &x, &y);
    do
    {
        DrawImagePixel(CPoint(x, y), nSize);
    }   while (DeltaGen(&dg, &x, &y));
}

// point is in bit image coordinates
void CBitEditView::DrawImagePixel(CPoint point, UINT nSize)
{
    CPoint bmapPt = point;
    point.x *= m_nZoom;
    point.y *= m_nZoom;

    CDC* pDC = GetDC();

    CBrush *pPrvBrush = CBrush::FromHandle(static_cast<HBRUSH>(pDC->SelectObject(m_pTMgr->GetForeBrush())));
    OnPrepareDC(pDC, NULL);

    if (nSize == 0)
        nSize = 1;          // Zero width is same as one width

    int nStartY;
    int wy = nStartY = point.y - (point.y % m_nZoom) - (nSize / 2) * m_nZoom;
    int wx = point.x - (point.x % m_nZoom) - (nSize / 2) * m_nZoom;
    int wStep = m_nZoom;

    if (m_bGridVisible && m_nZoom > 2) wStep -= 1;

    CSize size(m_size.cx * m_nZoom, m_size.cy * m_nZoom);
    // Draw the bits directly on the screen.
    for (UINT i = 0; i < nSize; i++, wx += m_nZoom)
    {
        wy = nStartY;
        for (UINT j = 0; j < nSize; j++, wy += m_nZoom)
        {
            if (wx >= 0 && wy >= 0 && wx < size.cx && wy < size.cy)
                pDC->PatBlt(xBorder + wx, yBorder + wy, wStep, wStep, PATCOPY);
        }
    }
    pDC->SelectObject(pPrvBrush);
    ReleaseDC(pDC);

    // Now set the point in the view bitmap...
    g_gt.mDC1.SelectObject(*m_bmView);

    wx = point.x / m_nZoom;
    wy = point.y / m_nZoom;
    if (wx < m_size.cx && wy < m_size.cy)
    {
        pPrvBrush = CBrush::FromHandle(static_cast<HBRUSH>(g_gt.mDC1.SelectObject(m_pTMgr->GetForeBrush())));
        g_gt.mDC1.PatBlt(wx - nSize / 2, wy - nSize / 2, nSize, nSize, PATCOPY);
        g_gt.mDC1.SelectObject(pPrvBrush);

        m_pSelView->UpdateViewPixel(bmapPt, nSize, &m_pTMgr->GetForeBrush());
    }
    g_gt.SelectSafeObjectsForDC1();
}

/////////////////////////////////////////////////////////////////////////////

OwnerPtr<CBitmap>& CBitEditView::GetCurrentViewBitmap()
{
    return m_bmView;
}

void CBitEditView::SetCurrentBitmap(TileID tid, const CBitmap* pBMap,
    BOOL bFillOnly /* = FALSE */)
{
    if (m_nCurToolID == ID_ITOOL_TEXT)
        CommitCurrentText();
    SetTextCaretPos(CPoint(-1, -1));        // Turn off the caret

    m_tid = tid;                    // Used for update broadcasts
    m_bFillOnly = bFillOnly;

    ClearAllImages();               // clear undos etc...
    if (pBMap == NULL)
        return;
    m_bmMaster = CloneBitmap(*pBMap);
    SetViewImageFromMasterImage();

    BITMAP bmInfo;
    m_bmMaster->GetObject(sizeof(bmInfo), &bmInfo);
    m_size = CSize(bmInfo.bmWidth, bmInfo.bmHeight);

    RecalcScrollLimits();

    // Generally we leave the current tool alone.
    if (bFillOnly)
    {
        m_nCurToolID = (UINT)ID_ITOOL_FILL;
        m_nLastToolID = m_nCurToolID;
    }

    Invalidate();
}

void CBitEditView::SetViewImageFromMasterImage()
{
    m_bmView = CloneBitmap(*m_bmMaster);
}

void CBitEditView::SetMasterImageFromViewImage()
{
    m_bmMaster = CloneBitmap(*m_bmView);
}

// Clear main image, undo images,  etc...
void CBitEditView::ClearAllImages()
{
    PurgeUndo();
    m_bmMaster = MakeOwner<CBitmap>();
    m_bmView = MakeOwner<CBitmap>();
    ClearPasteImage();
}

void CBitEditView::ClearPasteImage()
{
    if (IsPasteImage())
        m_bmPaste = MakeOwner<CBitmap>();
    m_rctPaste.SetRectEmpty();
}

BOOL CBitEditView::GetImagePixelLoc(CPoint& point) const
{
//  point += (CSize)GetDeviceScrollPosition();
    point -= CSize(xBorder, yBorder);
    point.x = point.x / (int)m_nZoom;
    point.y = point.y / (int)m_nZoom;
    return point.x >= 0 && point.y >= 0 &&
        point.x < m_size.cx && point.y < m_size.cy;
}

// Same as GetImagePixelLoc except the point is clamped to
// always be in the image.
void CBitEditView::GetImagePixelLocClamped(CPoint& point) const
{
//  point += (CSize)GetDeviceScrollPosition();
    point -= CSize(xBorder, yBorder);
    point.x = point.x / (int)m_nZoom;
    point.y = point.y / (int)m_nZoom;
    point.x = CB::max(point.x, 0);
    point.x = CB::min(point.x, m_size.cx - 1);
    point.y = CB::max(point.y, 0);
    point.y = CB::min(point.y, m_size.cy - 1);
}

void CBitEditView::InvalidateFocusBorder(BOOL bUpdate /* = FALSE */)
{
    CRect rct;
    if (m_nCurToolID != ID_ITOOL_SELECT || m_rctPaste.IsRectEmpty())
    {
        CSize size(m_size.cx * m_nZoom, m_size.cy * m_nZoom);
        rct = CRect(0, 0, 2 * xBorder + size.cx, 2 * yBorder + size.cy);
    }
    else
        rct = GetZoomedSelectBorderRect();

    WorkspaceToClient(rct);
    InvalidateRect(&rct, TRUE);
    if (bUpdate)
        UpdateWindow();
}

void CBitEditView::InvalidateViewImage(bool bUpdate)
{
    CRect rct = GetImageRect();
    WorkspaceToClient(rct);
    InvalidateRect(&rct, TRUE);
    if (bUpdate)
        UpdateWindow();
}

BOOL CBitEditView::IsPtInImage(CPoint point) const
{
    return GetImageRect().PtInRect(point);
}

CRect CBitEditView::GetImageRect() const
{
    CRect rct;
    if (m_bmMaster->m_hObject != NULL)
    {
        CSize size = GetZoomedSize();
        rct.SetRect(0, 0, size.cx, size.cy);
        rct.OffsetRect(xBorder, yBorder);
    }
    else
        rct.SetRectEmpty();
    return rct;
}

void CBitEditView::RecalcScrollLimits()
{
    if (m_bmMaster->m_hObject != NULL)
    {
        CSize size = GetZoomedSize();
        SetScrollSizes(MM_TEXT, CSize(size.cx + 2 * xBorder,
            size.cy + 2 * yBorder),
            CSize(CB::max(size.cx/4, 16),
            CB::max(size.cy/4, 16)), CSize(4, 4));
    }
    else
        SetScrollSizes(MM_TEXT, CSize(100, 100));
}

/////////////////////////////////////////////////////////////////////////////

BOOL CBitEditView::IsPtInSelectRect(CPoint point) const
{
    if (m_rctPaste.IsRectEmpty())
        return FALSE;
    return GetZoomedSelectRect().PtInRect(point);
}

CRect CBitEditView::GetZoomedSelectBorderRect() const
{
    CRect rct = GetZoomedSelectRect();
    rct.InflateRect(xBorder, yBorder);
    return rct;
}

CRect CBitEditView::GetZoomedSelectRect() const
{
    CRect rct(m_rctPaste.left * m_nZoom, m_rctPaste.top * m_nZoom,
        m_rctPaste.right * m_nZoom, m_rctPaste.bottom * m_nZoom);
    rct.OffsetRect(xBorder, yBorder);
    if (m_bGridVisible && m_nZoom > 2)
    {
        rct.top--;
        rct.left--;
    }
    return rct;
}

/////////////////////////////////////////////////////////////////////////////

int CBitEditView::CalcCaretHeight(int yLoc) const
{
    if (yLoc >= m_size.cy) return 0;
    int yEnd = yLoc + m_tmHeight;
    yEnd = CB::min(yEnd, m_size.cy);
    return yEnd - yLoc;
}

void CBitEditView::SetTextCaretPos(CPoint ptPos)
{
    int nHt = CalcCaretHeight(ptPos.y);
    if (nHt > 0 && ptPos.x >= 0 && ptPos.x <= m_size.cx)
    {
        CreateSolidCaret(1, nHt * m_nZoom);
        CPoint pnt = GetDeviceScrollPosition();
        SetCaretPos(CPoint(xBorder + ptPos.x * m_nZoom - pnt.x - 1,
            yBorder + ptPos.y * m_nZoom - pnt.y - 1));
        ShowCaret();
    }
    m_ptCaret = ptPos;
}

void CBitEditView::SetTextPosition(CPoint ptPos)
{
    m_ptText = ptPos;
    SetTextCaretPos(ptPos);
}

void CBitEditView::FixupTextCaret()
{
    SetTextCaretPos(CPoint(m_ptText.x + m_nTxtExtent, m_ptText.y));
}

void CBitEditView::CommitCurrentText()
{
    SetMasterImageFromViewImage();
    m_strText.clear();
}

void CBitEditView::UpdateTextView()
{
//  if (m_strText.GetLength() > 0 &&
    if (CRect(CPoint(0, 0), m_size).PtInRect(m_ptText))
    {
        // Draw the text on the view bitmap updating
        // the view with the master bitmap.
        SetViewImageFromMasterImage();          // Get fresh original
        g_gt.mDC1.SelectObject(*m_bmView);
        g_gt.mDC1.SetTextColor(m_pTMgr->GetForeColor());
        UINT nAlign = g_gt.mDC1.SetTextAlign(TA_LEFT | TA_TOP);
        CFontTbl* pFMgr = GetDocument().GetFontManager();
        CFont* pPrvFont = g_gt.mDC1.SelectObject(
            CFont::FromHandle(pFMgr->GetFontHandle(GetBitFont())));
        g_gt.mDC1.SetBkMode(TRANSPARENT);

        g_gt.mDC1.TextOut(m_ptText.x, m_ptText.y, m_strText);
        CSize size = g_gt.mDC1.GetTextExtent(m_strText);
        m_nTxtExtent = size.cx;

        g_gt.mDC1.SelectObject(pPrvFont);
        g_gt.mDC1.SetTextAlign(nAlign);
        g_gt.SelectSafeObjectsForDC1();
        InvalidateViewImage(true);
        m_pSelView->UpdateViewImage(NULL, TRUE);
    }
    FixupTextCaret();
}

void CBitEditView::AddChar(wchar_t nChar)
{
    if (CRect(CPoint(0, 0), m_size).PtInRect(m_ptCaret))
    {
        m_strText += nChar;
        UpdateTextView();
    }
}

void CBitEditView::DelChar()
{
    if (!m_strText.empty())
    {
        m_strText.resize(m_strText.a_size() - size_t(1));
        UpdateTextView();
    }
}

/////////////////////////////////////////////////////////////////////////////

FontID CBitEditView::GetBitFont()
{
    if (m_fontID != m_pTMgr->GetFontID())
    {
        m_fontID = m_pTMgr->GetFontID();
        UpdateFontInfo();
    }
    return m_fontID;
}

void CBitEditView::UpdateFontInfo()
{
    CDC* pDC = GetDC();
    CFontTbl* pFMgr = GetDocument().GetFontManager();
    CFont *pFont = CFont::FromHandle(pFMgr->GetFontHandle(GetBitFont()));
    CFont* pPrvFont = pDC->SelectObject(pFont);
    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    pDC->SelectObject(pPrvFont);
    m_tmHeight = tm.tmHeight;
    ReleaseDC(pDC);
}

/////////////////////////////////////////////////////////////////////////////

void CBitEditView::OnSetFocus(CWnd* pOldWnd)
{
    CScrollView::OnSetFocus(pOldWnd);
    if (m_nCurToolID == ID_ITOOL_TEXT)
        SetTextCaretPos(m_ptCaret);
}

void CBitEditView::OnKillFocus(CWnd* pNewWnd)
{
    CScrollView::OnKillFocus(pNewWnd);
    if (m_nCurToolID == ID_ITOOL_TEXT)
        ::DestroyCaret();
}

void CBitEditView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (m_nCurToolID == ID_ITOOL_TEXT)
    {
        if (nChar != VK_BACK)
        {
            ASSERT(nChar < 0x7f || !"non-ascii needs work");
            AddChar(value_preserving_cast<wchar_t>(nChar));
        }
        else
            DelChar();
    }
    else if (m_nCurToolID == ID_ITOOL_SELECT && m_bmPaste->m_hObject != NULL
        && GetCapture() != this)
    {
        if (nChar == VK_ESCAPE || nChar == VK_RETURN)
        {
            SetMasterImageFromViewImage();
            InvalidateFocusBorder();        // Erase previous focus
            ClearPasteImage();
            InvalidateFocusBorder();        // Draw new focus
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Undo support

void CBitEditView::SetUndoFromView()
{
    m_pSelView->PurgeUndo();        // Resize undos are now not allowed!

    while (m_listUndo.size() >= maxUndoLevels)
    {
        m_listUndo.pop_back();
    }
    OwnerPtr<CBitmap> pBMap = CloneBitmap(*m_bmView);
    m_listUndo.push_front(std::move(pBMap));
}

void CBitEditView::RestoreUndoToView()
{
    if (!IsUndoAvailable())
        return;

    InvalidateFocusBorder();
    ClearPasteImage();

    SetTextCaretPos(CPoint(-1, -1));    // Turn off the caret
    m_strText.clear();
    m_bmView = CloneBitmap(*m_listUndo.front());
    m_listUndo.pop_front();
}

void CBitEditView::PurgeUndo()
{
    m_listUndo.clear();
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers

BOOL CBitEditView::OnEraseBkgnd(CDC* pDC)
{
    CRect rct = GetImageRect();
    if (m_bGridVisible && m_nZoom > 2)
    {
        rct.left--;
        rct.top--;
    }
    WorkspaceToClient(rct);
    pDC->SaveDC();
    pDC->ExcludeClipRect(&rct);
    BOOL bRet = CScrollView::OnEraseBkgnd(pDC);
    pDC->RestoreDC(-1);
    return bRet;
}

///////////////////////////////////////////////////////////////////////

void CBitEditView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    // Make sure window is active.
    GetParentFrame()->ActivateFrame();

    CMenu bar;
    if (bar.LoadMenuW(IDR_MENU_DESIGN_POPUPS))
    {
        CMenu& popup = *bar.GetSubMenu(MENU_IV_BITEDIT);
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

void CBitEditView::OnLButtonDown(UINT nFlags, CPoint point)
{
    IToolType eToolType = MapToolType(m_nCurToolID);
    CImageTool& pTool = CImageTool::GetTool(eToolType);
    ClientToWorkspace(point);
    pTool.OnLButtonDown(this, nFlags, point);
}

void CBitEditView::OnMouseMove(UINT nFlags, CPoint point)
{
    IToolType eToolType = MapToolType(m_nCurToolID);
    CImageTool& pTool = CImageTool::GetTool(eToolType);
    ClientToWorkspace(point);
    pTool.OnMouseMove(this, nFlags, point);
}

void CBitEditView::OnLButtonUp(UINT nFlags, CPoint point)
{
    IToolType eToolType = MapToolType(m_nCurToolID);
    CImageTool& pTool = CImageTool::GetTool(eToolType);
    ClientToWorkspace(point);
    pTool.OnLButtonUp(this, nFlags, point);
}

BOOL CBitEditView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    IToolType eToolType = MapToolType(m_nCurToolID);
    if (pWnd == this && eToolType != itoolUnknown)
    {
        CImageTool& pTool = CImageTool::GetTool(eToolType);
        if(pTool.OnSetCursor(this, nHitTest))
            return TRUE;
    }
    return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

//////////////////////////////////////////////////////////////////////
// The table needs to be in the same order and the ID_ITOOL_* numbers

static IToolType tblTools[] =
{
    itoolPencil,        // ID_ITOOL_PENCIL
    itoolSelect,        // ID_ITOOL_SELECT
    itoolBrush,         // ID_ITOOL_BRUSH
    itoolFill,          // ID_ITOOL_FILL
    itoolText,          // ID_ITOOL_TEXT
    itoolLine,          // ID_ITOOL_LINE
    itoolRect,          // ID_ITOOL_RECT
    itoolOval,          // ID_ITOOL_OVAL
    itoolFillRect,      // ID_ITOOL_FILLRECT
    itoolFillOval,      // ID_ITOOL_FILLOVAL
    itoolEyeDropper,    // ID_ITOOL_DROPPER
    itoolColorChange,   // ID_ITOOL_COLORCHANGE
};

IToolType CBitEditView::MapToolType(UINT nToolResID)
{
    return tblTools[nToolResID - ID_ITOOL_PENCIL];
}

/////////////////////////////////////////////////////////////////////////////
// CBitEditView command message handlers

void CBitEditView::OnImageGridLines()
{
    m_bGridVisible = !m_bGridVisible;
    InvalidateViewImage(false);
}

void CBitEditView::OnUpdateImageGridLines(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_nZoom > 2 && m_bmMaster->m_hObject != NULL);
    pCmdUI->SetCheck(m_bGridVisible);
}

void CBitEditView::OnUpdateToolPalette(CCmdUI* pCmdUI)
{
    BOOL bEnable;
    BOOL bCheck;
    if (m_bFillOnly)
    {
        bEnable = pCmdUI->m_nID == ID_ITOOL_FILL || pCmdUI->m_nID == ID_ITOOL_DROPPER;
        bCheck = pCmdUI->m_nID == m_nCurToolID;
    }
    else
    {
        bEnable = TRUE;
        bCheck = pCmdUI->m_nID == m_nCurToolID;
    }
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // Need to handle menu that the submenu is connected to.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (bEnable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
    pCmdUI->Enable(bEnable);
    pCmdUI->SetCheck(bCheck);
}

BOOL CBitEditView::OnToolPalette(UINT id)
{
    if (id != m_nCurToolID)
    {
        m_nLastToolID = m_nCurToolID;
        m_nCurToolID = id;
        if (m_nLastToolID == ID_ITOOL_TEXT)
        {
            CommitCurrentText();
            SetTextCaretPos(CPoint(-1, -1));    // Turn off the caret
            ::DestroyCaret();
        }
        if (m_nLastToolID == ID_ITOOL_SELECT && m_bmPaste->m_hObject != NULL)
        {
            SetMasterImageFromViewImage();
            InvalidateFocusBorder();        // Erase previous focus
            ClearPasteImage();
            InvalidateFocusBorder();        // Draw new focus
        }
    }
    return TRUE;
}

LRESULT CBitEditView::OnSetColor(WPARAM wParam, LPARAM lParam)
{
    if ((UINT)wParam == ID_COLOR_FOREGROUND)
    {
        m_pTMgr->SetForeColor((COLORREF)lParam);
        if (m_nCurToolID == ID_ITOOL_TEXT)
            UpdateTextView();
    }
    else if ((UINT)wParam == ID_COLOR_BACKGROUND)
        m_pTMgr->SetBackColor((COLORREF)lParam);
    else
        return 0;
    return 1;
}

LRESULT CBitEditView::OnSetCustomColors(WPARAM wParam, LPARAM lParam)
{
    const std::vector<wxColour>& pCustomColors = CheckedDeref(reinterpret_cast<const std::vector<wxColour>*>(wParam));
    GetDocument().SetCustomColors(pCustomColors);
    return (LRESULT)0;
}

LRESULT CBitEditView::OnSetLineWidth(WPARAM wParam, LPARAM lParam)
{
    m_pTMgr->SetLineWidth((UINT)wParam);
    return 1;
}

void CBitEditView::OnUpdateColorForeground(CCmdUI* pCmdUI)
{
    ((CColorCmdUI*)pCmdUI)->SetColor(CB::Convert(m_pTMgr->GetForeColor()));
}

void CBitEditView::OnUpdateColorBackground(CCmdUI* pCmdUI)
{
    ((CColorCmdUI*)pCmdUI)->SetColor(CB::Convert(m_pTMgr->GetBackColor()));
}

void CBitEditView::OnUpdateColorTransparent(CCmdUI* pCmdUI)
{
    ((CColorCmdUI*)pCmdUI)->SetColor(CB::Convert(m_pTMgr->GetTransparentColor()));
}

void CBitEditView::OnUpdateColorCustom(CCmdUI* pCmdUI)
{
    ((CColorCmdUI*)pCmdUI)->SetCustomColors(GetDocument().GetCustomColors());
}

void CBitEditView::OnUpdateLineWidth(CCmdUI* pCmdUI)
{
    ((CColorCmdUI*)pCmdUI)->SetLineWidth(m_pTMgr->GetLineWidth());
}

void CBitEditView::OnImageBoardMask()
{
    CBoardManager* pBMgr = GetDocument().GetBoardManager();

    CBoardMaskDialog dlg(CheckedDeref(pBMgr));
    if (dlg.ShowModal() != wxID_OK || dlg.m_nBrdNum == Invalid_v<size_t>)
        return;

    CBoard& pBoard = pBMgr->GetBoard(value_preserving_cast<size_t>(dlg.m_nBrdNum));

    const CCellForm& pcf = pBoard.GetBoardArray().
        GetCellForm(m_pSelView->GetCurrentScale());

    CSize size = pcf.GetCellSize();
    g_gt.mDC1.SelectObject(*m_bmView);

    const CBitmap* pMask = pcf.GetMask();

    if (pMask != NULL)
    {
        g_gt.mDC2.SelectObject(*pMask);
        g_gt.mDC1.BitBlt(0, 0, size.cx, size.cy, &g_gt.mDC2, 0, 0,
            0x00220326L /* DSna */);
        g_gt.SelectSafeObjectsForDC2();
    }
    else
    {
        // Rectangular Cell...No mask exists. Just PATBLT the boundries.
        // This will only be visible if the target tile is larger than
        // the chosen rectangular. It delineates the interior area
        // of the chosen masking area. I have no idea if this is
        // a useful feature though.
        g_gt.mDC1.PatBlt(size.cx, 0, 1, size.cy, BLACKNESS);
        g_gt.mDC1.PatBlt(0, size.cy, size.cx, 1, BLACKNESS);
    }
    g_gt.SelectSafeObjectsForDC1();

    SetMasterImageFromViewImage();
    InvalidateViewImage(false);
    m_pSelView->UpdateViewImage(NULL, TRUE);
}

void CBitEditView::OnUpdateImageBoardMask(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_pSelView->GetCurrentScale() != smallScale &&
        !GetDocument().GetBoardManager()->IsEmpty());
}

void CBitEditView::OnUpdateViewZoomIn(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_nZoom < 8);
}

void CBitEditView::OnUpdateViewZoomOut(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_nZoom > 1);
}

void CBitEditView::OnViewZoomIn()
{
    switch (m_nZoom)
    {
        case 1: m_nZoom = 2; break;
        case 2: m_nZoom = 6; break;
        case 6: m_nZoom = 8; break;
    }
    RecalcScrollLimits();
    if (m_nCurToolID == ID_ITOOL_TEXT)
        SetTextCaretPos(m_ptCaret);
    Invalidate();
}

void CBitEditView::OnViewZoomOut()
{
    switch (m_nZoom)
    {
        case 8: m_nZoom = 6; break;
        case 6: m_nZoom = 2; break;
        case 2: m_nZoom = 1; break;
    }
    RecalcScrollLimits();
    if (m_nCurToolID == ID_ITOOL_TEXT)
        SetTextCaretPos(m_ptCaret);
    Invalidate();
}

void CBitEditView::OnViewToggleScale()
{
    switch (m_nZoom)
    {
        case 1: m_nZoom = 2; break;
        case 2: m_nZoom = 6; break;
        case 6: m_nZoom = 8; break;
        case 8: m_nZoom = 1; break;
    }
    RecalcScrollLimits();
    if (m_nCurToolID == ID_ITOOL_TEXT)
        SetTextCaretPos(m_ptCaret);
    Invalidate();
}

void CBitEditView::OnDwgFont()
{
    if (m_pTMgr->DoBitFontDialog())
    {
        m_fontID = m_pTMgr->GetFontID();
        UpdateFontInfo();
        if (m_nCurToolID == ID_ITOOL_TEXT)
            UpdateTextView();
    }
}

void CBitEditView::OnEditUndo()
{
    if (!IsUndoAvailable())
        return;
    RestoreUndoToView();
    SetMasterImageFromViewImage();
    InvalidateViewImage(false);
    m_pSelView->UpdateViewImage(NULL, TRUE);
}

void CBitEditView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsUndoAvailable());
}

void CBitEditView::OnEditCopy()
{
    if (m_nCurToolID == ID_ITOOL_SELECT && m_bmPaste->m_hObject != NULL &&
        !m_rctPaste.IsRectEmpty())
    {
        SetClipboardBitmap(this, *m_bmPaste);
    }
    else
        SetClipboardBitmap(this, *m_bmView);
}

void CBitEditView::OnEditPaste()
{
    OwnerPtr<CBitmap> pBMap = GetClipboardBitmap(this);

    SetUndoFromView();

    // First see if bitmaps are the same size. If they are just accept
    // the new bitmap. Otherwise prompt for rescaling.

    int nRescale = 0;

    BITMAP bmInfo;
    pBMap->GetObject(sizeof(bmInfo), &bmInfo);

    if (m_size.cx != bmInfo.bmWidth || m_size.cy != bmInfo.bmHeight)
    {
        CPasteBitmapDialog dlg;
        dlg.m_nPasteAction = nRescale;
        if (dlg.ShowModal() != wxID_OK)
        {
            return;
        }
        nRescale = dlg.m_nPasteAction;
    }

    SetMasterImageFromViewImage();
    ClearPasteImage();

    InvalidateFocusBorder();

    if (nRescale > 0)
    {
        m_bmPaste = CloneScaledBitmap(*pBMap, m_size, STRETCH_DELETESCANS);
        m_rctPaste.SetRect(0, 0, m_size.cx, m_size.cy);
    }
    else
    {
        m_bmPaste = CloneBitmap(*pBMap);
        m_rctPaste.SetRect(0, 0, bmInfo.bmWidth, bmInfo.bmHeight);
    }


    m_nLastToolID = m_nCurToolID;
    m_nCurToolID = ID_ITOOL_SELECT;

    DrawPastedImage();

    InvalidateFocusBorder();
}

void CBitEditView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsClipboardBitmap() && !m_bFillOnly);
}

void CBitEditView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!m_bFillOnly);
}

void CBitEditView::OnUpdateIndicatorCellNum(CCmdUI* pCmdUI)
{
    CPoint point;
    GetCursorPos(&point);
    ScreenToClient(&point);

    CRect rct;
    GetClientRect(&rct);

    if (rct.PtInRect(point))
    {
        ClientToWorkspace(point);
        if (GetImagePixelLoc(point))
        {
            CB::string szCoord = std::format(L"X={}, Y={}", point.x, point.y);
            pCmdUI->Enable();
            pCmdUI->SetText(szCoord);
        }
    }
}
