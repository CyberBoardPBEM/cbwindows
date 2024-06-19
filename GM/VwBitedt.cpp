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

wxBEGIN_EVENT_TABLE(CBitEditView, wxScrolledCanvas)
#if 0
    ON_WM_ERASEBKGND()
#endif
    EVT_MENU(XRCID("ID_IMAGE_GRIDLINES"), OnImageGridLines)
    EVT_MENU(XRCID("ID_ITOOL_PENCIL"), OnToolPalette)
    EVT_SETCOLOR(OnSetColor)
    EVT_SETCUSTOMCOLOR(OnSetCustomColors)
    EVT_SETLINEWIDTH(OnSetLineWidth)
    EVT_UPDATE_UI(XRCID("ID_IMAGE_GRIDLINES"), OnUpdateImageGridLines)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_PENCIL"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_COLOR_FOREGROUND"), OnUpdateColorForeground)
    EVT_UPDATE_UI(XRCID("ID_COLOR_BACKGROUND"), OnUpdateColorBackground)
    EVT_UPDATE_UI(XRCID("ID_COLOR_TRANSPARENT"), OnUpdateColorTransparent)
    EVT_UPDATE_UI(XRCID("ID_COLOR_CUSTOM"), OnUpdateColorCustom)
    EVT_UPDATE_UI(XRCID("ID_LINE_WIDTH"), OnUpdateLineWidth)
    EVT_LEFT_DOWN(OnLButtonDown)
    EVT_LEFT_UP(OnLButtonUp)
    EVT_MOTION(OnMouseMove)
    EVT_SET_CURSOR(OnSetCursor)
#if 0
    ON_COMMAND(ID_IMAGE_BOARDMASK, OnImageBoardMask)
#endif
    EVT_MENU(wxID_ZOOM_IN, OnViewZoomIn)
    EVT_UPDATE_UI(wxID_ZOOM_IN, OnUpdateViewZoomIn)
    EVT_MENU(wxID_ZOOM_OUT, OnViewZoomOut)
    EVT_UPDATE_UI(wxID_ZOOM_OUT, OnUpdateViewZoomOut)
#if 0
    ON_UPDATE_COMMAND_UI(ID_IMAGE_BOARDMASK, OnUpdateImageBoardMask)
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
#endif
    EVT_MENU(XRCID("ID_DWG_FONT"), OnDwgFont)
    EVT_UPDATE_UI(XRCID("ID_DWG_FONT"), OnUpdateEnable)
    EVT_CHAR(OnChar)
    EVT_MENU(wxID_UNDO, OnEditUndo)
    EVT_UPDATE_UI(wxID_UNDO, OnUpdateEditUndo)
#if 0
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
#endif
    EVT_UPDATE_UI(XRCID("ID_INDICATOR_CELLNUM"), OnUpdateIndicatorCellNum)
#if 0
    ON_WM_CONTEXTMENU()
#endif
    EVT_MENU(XRCID("ID_ITOOL_SELECT"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_BRUSH"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_FILL"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_TEXT"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_LINE"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_RECT"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_OVAL"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_FILLRECT"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_FILLOVAL"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_DROPPER"), OnToolPalette)
    EVT_MENU(XRCID("ID_ITOOL_COLORCHANGE"), OnToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_SELECT"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_BRUSH"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_FILL"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_TEXT"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_LINE"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_RECT"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_OVAL"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_FILLRECT"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_FILLOVAL"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_DROPPER"), OnUpdateToolPalette)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_COLORCHANGE"), OnUpdateToolPalette)
    EVT_MENU(XRCID("ID_VIEW_TOGGLE_SCALE"), OnViewToggleScale)
    EVT_UPDATE_UI(XRCID("ID_VIEW_TOGGLE_SCALE"), OnUpdateEnable)
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CBitEditViewContainer, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_SETFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBitEditView

IMPLEMENT_DYNCREATE(CBitEditViewContainer, CView)

CBitEditView::CBitEditView(CBitEditViewContainer& p) :
    parent(&p),
    document(dynamic_cast<CGamDoc*>(parent->GetDocument()))
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
    m_nCurToolID = XRCID("ID_ITOOL_PENCIL");
    m_nLastToolID = XRCID("ID_ITOOL_PENCIL");
    m_bSelectCapture = FALSE;
    wxScrolledCanvas::Create(*parent, 0);
}

CBitEditView::~CBitEditView()
{
    PurgeUndo();
}

/////////////////////////////////////////////////////////////////////////////

#if 0
BOOL CBitEditView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CScrollView::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)(COLOR_BTNFACE + 1));

    return TRUE;
}
#endif

void CBitEditView::OnInitialUpdate()
{
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
    sizer->Add(100, 100);
    SetScrollRate(4, 4);
    m_pTMgr = GetDocument().GetTileManager();
    UpdateFontInfo();
    RecalcScrollLimits();
}

void CBitEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    WORD wHint = LOWORD(lHint);
    // Update for nonspecific notification only
    if (wHint == HINT_ALWAYSUPDATE)
        parent->CView::OnUpdate(pSender, lHint, pHint);
}

/////////////////////////////////////////////////////////////////////////////
// CBitEditView drawing

void CBitEditView::OnDraw(wxDC& pDC)
{
    if (!m_bmView.IsOk())
        return;                 // Nothing to draw.

    wxSize size(m_size.x * m_nZoom, m_size.y * m_nZoom);

    if (m_nCurToolID != XRCID("ID_ITOOL_SELECT") || m_rctPaste.IsEmpty())
    {
        // Handle fancy focus rect
        if (parent->GetParentFrame()->GetActiveView() == parent)
        {
            wxRect rct(0, 0, 2 * xBorder + size.x, 2 * yBorder + size.y);
            Draw25PctPatBorder(*this, pDC, rct, xBorder);
        }
    }

    // Now handle the actual bitmap

    wxMemoryDC sourceDC;
    sourceDC.SelectObjectAsSource(m_bmView);

    if (m_bGridVisible && m_nZoom > 2)
    {
        wxBitmap bmTmp(
            size.x + 1, size.y + 1);
        wxMemoryDC tempDC(bmTmp);
        tempDC.StretchBlit(0, 0, size.x, size.y, &sourceDC, 0, 0,
            m_size.x, m_size.y);
        tempDC.SetPen(*wxBLACK_PEN);
        for (int i = 0; i <= size.x; i += m_nZoom)
            tempDC.DrawLine(wxPoint(i, 0), wxPoint(i, size.y + 1));
        for (int i = 0; i <= size.y; i += m_nZoom)
            tempDC.DrawLine(wxPoint(0, i), wxPoint(size.x + 1, i));
        pDC.Blit(xBorder - 1, yBorder - 1, size.x + 1, size.y + 1, &tempDC,
            0, 0);
    }
    else
    {
        pDC.StretchBlit(xBorder, yBorder, size.x, size.y, &sourceDC, 0, 0,
            m_size.x, m_size.y);
        pDC.SetPen(*wxBLACK_PEN);
        pDC.DrawLine(xBorder-1, yBorder-1, xBorder-1, yBorder-1 + size.y + 2);
        pDC.DrawLine(xBorder-1, yBorder-1, xBorder-1 + size.x + 2, yBorder-1);
        pDC.DrawLine(xBorder + size.x, yBorder-1, xBorder + size.x, yBorder-1 + size.y + 2);
        pDC.DrawLine(xBorder-1, yBorder + size.y, xBorder-1 + size.x + 2, yBorder + size.y);
    }
    if (m_nCurToolID == XRCID("ID_ITOOL_SELECT") && !m_bSelectCapture &&
        m_bmPaste.IsOk() && !m_rctPaste.IsEmpty())
    {
        // Handle fancy focus rect
        if (parent->GetParentFrame()->GetActiveView())
        {
            // Convert paste rect to view coordinates.
            wxRect rct = GetZoomedSelectBorderRect();
            Draw25PctPatBorder(*this, pDC, rct, xBorder);
        }
    }
}

void CBitEditViewContainer::OnActivateView(BOOL bActivate, CView *pActivateView,
    CView* pDeactivateView)
{
    CView::OnActivateView(bActivate, pActivateView, pDeactivateView);
    if (pActivateView == pDeactivateView)
        return;
//  if (m_nCurToolID == ID_ITOOL_TEXT)
//  {
//      child->SetFocus();
//      child->SetTextCaretPos(m_ptCaret);
//  }
    child->Refresh();       // Later only mess with focus rect.
}

/////////////////////////////////////////////////////////////////////////////

void CBitEditView::ClientToWorkspace(wxPoint& pnt) const
{
    pnt = CalcUnscrolledPosition(pnt);
}

void CBitEditView::WorkspaceToClient(wxPoint& pnt) const
{
    pnt = CalcScrolledPosition(pnt);
}

void CBitEditView::WorkspaceToClient(wxRect& rct) const
{
    rct.SetLeftTop(CalcScrolledPosition(rct.GetTopLeft()));
}

/////////////////////////////////////////////////////////////////////////////
// Draw tool support.
// points are in bit image coordinates

void CBitEditView::DrawImageLine(wxPoint startPt, wxPoint curPt, UINT nSize)
{
    SetViewImageFromMasterImage();          // Get fresh original

    {
        wxMemoryDC dc(m_bmView);
        dc.SetBrush(m_pTMgr->GetForeBrushWx());
        dc.SetPen(*wxTRANSPARENT_PEN);

        if (nSize == 0)
            nSize = 1;          // Zero width is same as one width

        int x = startPt.x;
        int y = startPt.y;
        DELTAGEN dg;

        DeltaGenInit(&dg, startPt.x, startPt.y, curPt.x, curPt.y, &x, &y);
        do
        {
            if (x < m_size.x && y < m_size.y)
            {
                dc.DrawRectangle(wxPoint(x - nSize / 2, y - nSize / 2),
                    wxSize(nSize, nSize));
            }
        } while (DeltaGen(&dg, &x, &y));
    }

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage();
}

// If nSize is Zero. The rect is filled. Otherwise it is a frame.
void CBitEditView::DrawImageSelectRect(wxPoint startPt, wxPoint curPt)
{
    SetViewImageFromMasterImage();          // Get fresh original

    {
        wxMemoryDC dc(m_bmView);

        wxRect rct(wxPoint(std::min(startPt.x, curPt.x), std::min(startPt.y, curPt.y)),
            wxSize(std::abs(curPt.x - startPt.x), std::abs(curPt.y - startPt.y)));
        if (rct.GetWidth() != 0 && rct.GetHeight() != 0)
        {
            dc.SetPen(*wxWHITE_PEN);
            dc.SetLogicalFunction(wxINVERT);
            dc.DrawLine(rct.GetLeft(), rct.GetTop(), rct.GetLeft() + rct.GetWidth(), rct.GetTop());
            dc.DrawLine(rct.GetLeft() + rct.GetWidth(), rct.GetTop(), rct.GetLeft() + rct.GetWidth(), rct.GetTop() + rct.GetHeight());
            dc.DrawLine(rct.GetLeft() + 1, rct.GetTop() + rct.GetHeight(), rct.GetLeft() + 1 + rct.GetWidth(), rct.GetTop() + rct.GetHeight());
            dc.DrawLine(rct.GetLeft(), rct.GetTop() + 1, rct.GetLeft(), rct.GetTop() + 1 + rct.GetHeight());
        }
    }

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage();
}

void CBitEditView::DrawPastedImage()
{
    ASSERT(m_bmPaste.IsOk());
    SetViewImageFromMasterImage();          // Get fresh original
    MergeBitmap(m_bmView, m_bmPaste, m_rctPaste.GetTopLeft());

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage();
}

// If nSize is Zero. The rect is filled. Otherwise it is a frame.
void CBitEditView::DrawImageRect(wxPoint startPt, wxPoint curPt, UINT nSize)
{
    SetViewImageFromMasterImage();          // Get fresh original

    {
        wxMemoryDC dc(m_bmView);

        wxRect rct(wxPoint(std::min(startPt.x, curPt.x), std::min(startPt.y, curPt.y)),
                    wxSize(std::abs(curPt.x - startPt.x) + 1, std::abs(curPt.y - startPt.y) + 1));
        if (nSize == 0)
        {
            dc.SetBrush(m_pTMgr->GetForeBrushWx());
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(rct);
        }
        else
        {
            wxPen pen(CB::Convert(m_pTMgr->GetForeColor()), nSize);
            dc.SetPen(pen);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);

            dc.DrawRectangle(rct);
        }
    }

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage();
}

// If nSize is Zero. The ellipse is filled. Otherwise it is a frame.
void CBitEditView::DrawImageEllipse(wxPoint startPt, wxPoint curPt, UINT nSize)
{
    SetViewImageFromMasterImage();          // Get fresh original

    {
        wxMemoryDC dc(m_bmView);

        /* KLUDGE:  size - 1 due to
                    https://groups.google.com/g/wx-dev/c/0HZLlbTB6do/m/JMQy61LqAQAJ */
        wxRect rct(wxPoint(std::min(startPt.x, curPt.x), std::min(startPt.y, curPt.y)),
                    wxSize(std::abs(curPt.x - startPt.x) + 1 - 1, std::abs(curPt.y - startPt.y) + 1 - 1));
        wxPen pen(CB::Convert(m_pTMgr->GetForeColor()), nSize);
        if (nSize == 0)
            dc.SetBrush(m_pTMgr->GetForeBrushWx());
        else
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(pen);

        dc.DrawEllipse(rct);
    }

    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage();
}

void CBitEditView::DrawImageFill(wxPoint pt)
{
    SetViewImageFromMasterImage();          // Get fresh original
    {
        wxColour oldColor = GetPixel(m_bmView, pt.x, pt.y);
        wxMemoryDC dc(m_bmView);

        dc.SetBrush(m_pTMgr->GetForeBrushWx());

        /* TODO:  not portable:
                    https://docs.wxwidgets.org/latest/classwx_d_c.html#af510e22ffc274d3d3b29659941f2b5a9 */
        CB_VERIFY(dc.FloodFill(pt.x, pt.y, oldColor, wxFLOOD_SURFACE));
    }

    SetMasterImageFromViewImage();
    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage();
}

void CBitEditView::DrawImageChangeColor(wxPoint pt)
{
    SetViewImageFromMasterImage();          // Get fresh original

    wxASSERT(m_bmView.GetDepth() == 24 && m_bmView.IsDIB());
    wxNativePixelData imgData(m_bmView);
    wxNativePixelData::Iterator it(imgData);
    it.MoveTo(imgData, pt.x, pt.y);

    wxColour crHit(it.Red(), it.Green(), it.Blue());
    wxColour crNew = CB::Convert(m_pTMgr->GetForeColor());

    it.Reset(imgData);
    for (int y = 0 ; y < m_bmView.GetHeight() ; ++y)
    {
        wxNativePixelData::Iterator curr = it;
        for (int x = 0 ; x < m_bmView.GetWidth(); ++x)
        {
            wxColour cr = wxColour(curr.Red(), curr.Green(), curr.Blue());
            if (cr == crHit)
            {
                curr.Red() = crNew.Red();
                curr.Green() = crNew.Green();
                curr.Blue() = crNew.Blue();
            }
            ++curr;
        }
        it.OffsetY(imgData, 1);
    }

    SetMasterImageFromViewImage();
    InvalidateViewImage(true);
    m_pSelView->UpdateViewImage();
}

// points are in bit image coordinates
void CBitEditView::DrawImageToPixel(wxPoint prvPt, wxPoint curPt, UINT nSize)
{
    DELTAGEN dg;
    int x = curPt.x;
    int y = curPt.y;
    DeltaGenInit(&dg, prvPt.x, prvPt.y, curPt.x, curPt.y, &x, &y);
    do
    {
        DrawImagePixel(wxPoint(x, y), nSize);
    }   while (DeltaGen(&dg, &x, &y));
}

// point is in bit image coordinates
void CBitEditView::DrawImagePixel(wxPoint point, UINT nSize)
{
    wxPoint bmapPt = point;
    point.x *= m_nZoom;
    point.y *= m_nZoom;

    if (nSize == 0)
        nSize = 1;          // Zero width is same as one width

    int nStartY;
    int wy = nStartY = point.y - (point.y % m_nZoom) - (nSize / 2) * m_nZoom;
    int wx = point.x - (point.x % m_nZoom) - (nSize / 2) * m_nZoom;
    int wStep = m_nZoom;

    if (m_bGridVisible && m_nZoom > 2) wStep -= 1;

    wxPoint ptScroll = CalcUnscrolledPosition(wxPoint(0, 0));

    wxSize size(m_size.x * m_nZoom, m_size.y * m_nZoom);
    // Can't draw the bits directly on the screen, so mark rect for refresh
    for (UINT i = 0; i < nSize; i++, wx += m_nZoom)
    {
        wy = nStartY;
        for (UINT j = 0; j < nSize; j++, wy += m_nZoom)
        {
            if (wx >= 0 && wy >= 0 && wx < size.x && wy < size.y)
            {
                RefreshRect(wxRect(wxPoint(xBorder + wx - ptScroll.x, yBorder + wy - ptScroll.y), wxSize(wStep, wStep)));
            }
        }
    }

    // Now set the point in the view bitmap...
    wxMemoryDC dc(m_bmView);

    wx = point.x / m_nZoom;
    wy = point.y / m_nZoom;
    if (wx < m_size.x && wy < m_size.y)
    {
        dc.SetBrush(m_pTMgr->GetForeBrushWx());
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(wxPoint(wx - nSize / 2, wy - nSize / 2), wxSize(nSize, nSize));

        m_pSelView->UpdateViewPixel(bmapPt, nSize, m_pTMgr->GetForeBrushWx());
    }
}

/////////////////////////////////////////////////////////////////////////////

const wxBitmap& CBitEditView::GetCurrentViewBitmap() const
{
    return m_bmView;
}

void CBitEditView::SetCurrentBitmap(TileID tid, const wxBitmap& pBMap,
    BOOL bFillOnly /* = FALSE */)
{
    if (m_nCurToolID == XRCID("ID_ITOOL_TEXT"))
        CommitCurrentText();
    SetTextCaretPos(wxPoint(-1, -1));        // Turn off the caret

    m_tid = tid;                    // Used for update broadcasts
    m_bFillOnly = bFillOnly;

    ClearAllImages();               // clear undos etc...
    if (!pBMap.IsOk())
        return;
    m_bmMaster = CloneBitmap(pBMap);
    SetViewImageFromMasterImage();

    m_size = m_bmMaster.GetSize();

    RecalcScrollLimits();

    // Generally we leave the current tool alone.
    if (bFillOnly)
    {
        m_nCurToolID = XRCID("ID_ITOOL_FILL");
        m_nLastToolID = m_nCurToolID;
    }

    Refresh();
}

void CBitEditView::SetViewImageFromMasterImage()
{
    m_bmView = CloneBitmap(m_bmMaster);
}

void CBitEditView::SetMasterImageFromViewImage()
{
    m_bmMaster = CloneBitmap(m_bmView);
}

// Clear main image, undo images,  etc...
void CBitEditView::ClearAllImages()
{
    PurgeUndo();
    m_bmMaster = wxBitmap();
    m_bmView = wxBitmap();
    ClearPasteImage();
}

void CBitEditView::ClearPasteImage()
{
    if (IsPasteImage())
        m_bmPaste = wxBitmap();
    m_rctPaste = wxRect();
}

BOOL CBitEditView::GetImagePixelLoc(wxPoint& point) const
{
//  point += (CSize)GetDeviceScrollPosition();
    point -= wxSize(xBorder, yBorder);
    point.x = point.x / (int)m_nZoom;
    point.y = point.y / (int)m_nZoom;
    return point.x >= 0 && point.y >= 0 &&
        point.x < m_size.x && point.y < m_size.y;
}

// Same as GetImagePixelLoc except the point is clamped to
// always be in the image.
void CBitEditView::GetImagePixelLocClamped(wxPoint& point) const
{
//  point += (CSize)GetDeviceScrollPosition();
    point -= wxSize(xBorder, yBorder);
    point.x = point.x / (int)m_nZoom;
    point.y = point.y / (int)m_nZoom;
    point.x = CB::max(point.x, 0);
    point.x = CB::min(point.x, m_size.x - 1);
    point.y = CB::max(point.y, 0);
    point.y = CB::min(point.y, m_size.y - 1);
}

void CBitEditView::InvalidateFocusBorder()
{
    wxRect rct;
    if (m_nCurToolID != XRCID("ID_ITOOL_SELECT") || m_rctPaste.IsEmpty())
    {
        wxSize size(m_size.x * m_nZoom, m_size.y * m_nZoom);
        rct = wxRect(wxPoint(0, 0), wxSize(2 * xBorder + size.x, 2 * yBorder + size.y));
    }
    else
        rct = GetZoomedSelectBorderRect();

    WorkspaceToClient(rct);
    RefreshRect(rct, TRUE);
}

void CBitEditView::InvalidateViewImage(bool bUpdate)
{
    wxRect rct = GetImageRect();
    WorkspaceToClient(rct);
    RefreshRect(rct, TRUE);
    if (bUpdate)
        Update();
}

BOOL CBitEditView::IsPtInImage(wxPoint point) const
{
    return GetImageRect().Contains(point);
}

wxRect CBitEditView::GetImageRect() const
{
    wxRect rct;
    if (m_bmMaster.IsOk())
    {
        wxSize size = GetZoomedSize();
        rct = wxRect(wxPoint(xBorder, yBorder), wxSize(size.x, size.y));
    }
    else
        rct = wxRect();
    return rct;
}

void CBitEditView::RecalcScrollLimits()
{
    wxSizer* sizer = GetSizer();
    // this can be called before OnInitialUpdate
    if (!sizer)
    {
        return;
    }
    wxSizerItemList& items = sizer->GetChildren();
    wxASSERT(items.size() == 1);
    wxSizerItem* item = items[0];
    wxASSERT(item->IsSpacer());
    if (m_bmMaster.IsOk())
    {
        wxSize size = GetZoomedSize();
        item->AssignSpacer(size.x + 2 * xBorder,
                            size.y + 2 * yBorder);
    }
    else
    {
        item->AssignSpacer(100, 100);
    }
    sizer->FitInside(this);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CBitEditView::IsPtInSelectRect(wxPoint point) const
{
    if (m_rctPaste.IsEmpty())
        return FALSE;
    return GetZoomedSelectRect().Contains(point);
}

wxRect CBitEditView::GetZoomedSelectBorderRect() const
{
    wxRect rct = GetZoomedSelectRect();
    rct.Inflate(xBorder, yBorder);
    return rct;
}

wxRect CBitEditView::GetZoomedSelectRect() const
{
    wxRect rct(wxPoint(m_rctPaste.GetLeft() * m_nZoom, m_rctPaste.GetTop() * m_nZoom),
        wxSize(m_rctPaste.GetWidth() * m_nZoom, m_rctPaste.GetHeight() * m_nZoom));
    rct.Offset(xBorder, yBorder);
    if (m_bGridVisible && m_nZoom > 2)
    {
        rct.Offset(-1, -1);
    }
    return rct;
}

/////////////////////////////////////////////////////////////////////////////

int CBitEditView::CalcCaretHeight(int yLoc) const
{
    if (yLoc >= m_size.y) return 0;
    int yEnd = yLoc + m_tmHeight;
    yEnd = CB::min(yEnd, m_size.y);
    return yEnd - yLoc;
}

void CBitEditView::SetTextCaretPos(wxPoint ptPos)
{
    int nHt = CalcCaretHeight(ptPos.y);
    if (nHt > 0 && ptPos.x >= 0 && ptPos.x <= m_size.x)
    {
        wxCaret* caret = new wxCaret(this, 1, nHt * m_nZoom);
        SetCaret(caret);
        wxPoint pnt = ClientToWorkspace(wxPoint(0, 0));
        caret->Move(wxPoint(xBorder + ptPos.x * m_nZoom - pnt.x - 1,
            yBorder + ptPos.y * m_nZoom - pnt.y - 1));
        caret->Show(true);
    }
    m_ptCaret = ptPos;
}

void CBitEditView::SetTextPosition(wxPoint ptPos)
{
    m_ptText = ptPos;
    SetTextCaretPos(ptPos);
}

void CBitEditView::FixupTextCaret()
{
    SetTextCaretPos(wxPoint(m_ptText.x + m_nTxtExtent, m_ptText.y));
}

void CBitEditView::CommitCurrentText()
{
    SetMasterImageFromViewImage();
    m_strText.clear();
}

void CBitEditView::UpdateTextView()
{
//  if (m_strText.GetLength() > 0 &&
    if (wxRect(wxPoint(0, 0), m_size).Contains(m_ptText))
    {
        // Draw the text on the view bitmap updating
        // the view with the master bitmap.
        SetViewImageFromMasterImage();          // Get fresh original
        {
            wxMemoryDC dc(m_bmView);
            dc.SetTextForeground(CB::Convert(m_pTMgr->GetForeColor()));
            dc.SetFont(ToWxFont(UpdateBitFont()));

            dc.DrawText(m_strText, m_ptText.x, m_ptText.y);
            // GetTextExtent is only for single line
            wxASSERT(m_strText.find('\n') == CB::string::npos);
            wxSize size = dc.GetTextExtent(m_strText);
            m_nTxtExtent = size.x;
        }

        InvalidateViewImage(true);
        m_pSelView->UpdateViewImage();
    }
    FixupTextCaret();
}

void CBitEditView::AddChar(wchar_t nChar)
{
    if (wxRect(wxPoint(0, 0), m_size).Contains(m_ptCaret))
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

FontID CBitEditView::UpdateBitFont()
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
    wxFont font = ToWxFont(UpdateBitFont());
    wxClientDC pDC(this);
    pDC.SetFont(font);
    m_tmHeight = pDC.GetCharHeight();
}

/////////////////////////////////////////////////////////////////////////////

#if 0
void CBitEditView::OnSetFocus(CWnd* pOldWnd)
{
    CScrollView::OnSetFocus(pOldWnd);
    if (m_nCurToolID == XRCID("ID_ITOOL_TEXT"))
        SetTextCaretPos(m_ptCaret);
}

void CBitEditView::OnKillFocus(CWnd* pNewWnd)
{
    CScrollView::OnKillFocus(pNewWnd);
    if (m_nCurToolID == XRCID("ID_ITOOL_TEXT"))
        ::DestroyCaret();
}
#endif

void CBitEditView::OnChar(wxKeyEvent& event)
{
    wxASSERT(event.GetKeyCode() != WXK_NONE ||
            !"TODO:  unicode");
    if (m_nCurToolID == XRCID("ID_ITOOL_TEXT"))
    {
        if (event.GetKeyCode() == WXK_RETURN)
        {
            // ignore return; it doesn't get printed correctly
        }
        else if (event.GetKeyCode() != WXK_BACK)
        {
            wxASSERT(event.GetKeyCode() < 0x7f || !"non-ascii needs work");
            AddChar(value_preserving_cast<wchar_t>(event.GetKeyCode()));
        }
        else
            DelChar();
    }
    else if (m_nCurToolID == XRCID("ID_ITOOL_SELECT") && m_bmPaste.IsOk()
        && HasCapture())
    {
        if (event.GetKeyCode() == WXK_ESCAPE || event.GetKeyCode() == WXK_RETURN)
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
    wxBitmap pBMap = CloneBitmap(m_bmView);
    m_listUndo.push_front(std::move(pBMap));
}

void CBitEditView::RestoreUndoToView()
{
    if (!IsUndoAvailable())
        return;

    InvalidateFocusBorder();
    ClearPasteImage();

    SetTextCaretPos(wxPoint(-1, -1));    // Turn off the caret
    m_strText.clear();
    m_bmView = CloneBitmap(m_listUndo.front());
    m_listUndo.pop_front();
}

void CBitEditView::PurgeUndo()
{
    m_listUndo.clear();
}

/////////////////////////////////////////////////////////////////////////////
// Message handlers

#if 0
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
#endif

///////////////////////////////////////////////////////////////////////

#if 0
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
#endif

void CBitEditView::OnLButtonDown(wxMouseEvent& event)
{
    IToolType eToolType = MapToolType(m_nCurToolID);
    CImageTool& pTool = CImageTool::GetTool(eToolType);
    wxPoint point = ClientToWorkspace(event.GetPosition());
    pTool.OnLButtonDown(*this, event.GetModifiers(), point);
}

void CBitEditView::OnMouseMove(wxMouseEvent& event)
{
    IToolType eToolType = MapToolType(m_nCurToolID);
    CImageTool& pTool = CImageTool::GetTool(eToolType);
    wxPoint point = ClientToWorkspace(event.GetPosition());
    pTool.OnMouseMove(*this, event.GetModifiers(), point);
}

void CBitEditView::OnLButtonUp(wxMouseEvent& event)
{
    IToolType eToolType = MapToolType(m_nCurToolID);
    CImageTool& pTool = CImageTool::GetTool(eToolType);
    wxPoint point = ClientToWorkspace(event.GetPosition());
    pTool.OnLButtonUp(*this, event.GetModifiers(), point);
}

void CBitEditView::OnSetCursor(wxSetCursorEvent& event)
{
    IToolType eToolType = MapToolType(m_nCurToolID);
    if (eToolType != itoolUnknown)
    {
        CImageTool& pTool = CImageTool::GetTool(eToolType);
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

IToolType CBitEditView::MapToolType(int nToolResID)
{
    // wx doesn't guarantee XRCID() value order
    static const std::unordered_map<int, IToolType> map {
        { XRCID("ID_ITOOL_PENCIL"), itoolPencil },
        { XRCID("ID_ITOOL_SELECT"), itoolSelect },
        { XRCID("ID_ITOOL_BRUSH"), itoolBrush },
        { XRCID("ID_ITOOL_FILL"), itoolFill },
        { XRCID("ID_ITOOL_TEXT"), itoolText },
        { XRCID("ID_ITOOL_LINE"), itoolLine },
        { XRCID("ID_ITOOL_RECT"), itoolRect },
        { XRCID("ID_ITOOL_OVAL"), itoolOval },
        { XRCID("ID_ITOOL_FILLRECT"), itoolFillRect },
        { XRCID("ID_ITOOL_FILLOVAL"), itoolFillOval },
        { XRCID("ID_ITOOL_DROPPER"), itoolEyeDropper },
        { XRCID("ID_ITOOL_COLORCHANGE"), itoolColorChange },
    };
    return map.at(nToolResID);
}

/////////////////////////////////////////////////////////////////////////////
// CBitEditView command message handlers

void CBitEditView::OnImageGridLines(wxCommandEvent& /*event*/)
{
    m_bGridVisible = !m_bGridVisible;
    InvalidateViewImage(false);
}

void CBitEditView::OnUpdateImageGridLines(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_nZoom > 2 && m_bmMaster.IsOk());
    pCmdUI.Check(m_bGridVisible);
}

void CBitEditView::OnUpdateToolPalette(wxUpdateUIEvent& pCmdUI)
{
    BOOL bEnable;
    BOOL bCheck;
    if (m_bFillOnly)
    {
        bEnable = pCmdUI.GetId() == XRCID("ID_ITOOL_FILL") || pCmdUI.GetId() == XRCID("ID_ITOOL_DROPPER");
        bCheck = pCmdUI.GetId() == m_nCurToolID;
    }
    else
    {
        bEnable = TRUE;
        bCheck = pCmdUI.GetId() == m_nCurToolID;
    }
    pCmdUI.Enable(bEnable);
    pCmdUI.Check(bCheck);
}

void CBitEditView::OnToolPalette(wxCommandEvent& event)
{
    if (event.GetId() != m_nCurToolID)
    {
        m_nLastToolID = m_nCurToolID;
        m_nCurToolID = event.GetId();
        if (m_nLastToolID == XRCID("ID_ITOOL_TEXT"))
        {
            CommitCurrentText();
            SetTextCaretPos(wxPoint(-1, -1));    // Turn off the caret
            // wx doesn't have precise DestroyCaret() equivalent
            GetCaret()->Hide();
        }
        if (m_nLastToolID == XRCID("ID_ITOOL_SELECT") && m_bmPaste.IsOk())
        {
            SetMasterImageFromViewImage();
            InvalidateFocusBorder();        // Erase previous focus
            ClearPasteImage();
            InvalidateFocusBorder();        // Draw new focus
        }
    }
}

void CBitEditView::OnSetColor(SetColorEvent& event)
{
    if (event.GetSubId() == XRCID("ID_COLOR_FOREGROUND"))
    {
        m_pTMgr->SetForeColor(CB::Convert(event.GetColor()));
        if (m_nCurToolID == XRCID("ID_ITOOL_TEXT"))
            UpdateTextView();
    }
    else if (event.GetSubId() == XRCID("ID_COLOR_BACKGROUND"))
    {
        m_pTMgr->SetBackColor(CB::Convert(event.GetColor()));
    }
    else
    {
        event.Skip();
    }
}

void CBitEditView::OnSetCustomColors(SetCustomColorEvent& event)
{
    const std::vector<wxColour>& pCustomColors = event.GetColors();
    GetDocument().SetCustomColors(pCustomColors);
}

void CBitEditView::OnSetLineWidth(SetLineWidthEvent& event)
{
    m_pTMgr->SetLineWidth(event.GetWidth());
}

void CBitEditView::OnUpdateColorForeground(wxUpdateUIEvent& pCmdUI)
{
    CCmdUI* cmdui = static_cast<CCmdUI*>(pCmdUI.GetClientData());
    CColorCmdUI& colorCmdUI = CheckedDeref(dynamic_cast<CColorCmdUI*>(cmdui));
    colorCmdUI.SetColor(CB::Convert(m_pTMgr->GetForeColor()));
}

void CBitEditView::OnUpdateColorBackground(wxUpdateUIEvent& pCmdUI)
{
    CCmdUI* cmdui = static_cast<CCmdUI*>(pCmdUI.GetClientData());
    CColorCmdUI& colorCmdUI = CheckedDeref(dynamic_cast<CColorCmdUI*>(cmdui));
    colorCmdUI.SetColor(CB::Convert(m_pTMgr->GetBackColor()));
}

void CBitEditView::OnUpdateColorTransparent(wxUpdateUIEvent& pCmdUI)
{
    CCmdUI* cmdui = static_cast<CCmdUI*>(pCmdUI.GetClientData());
    CColorCmdUI& colorCmdUI = CheckedDeref(dynamic_cast<CColorCmdUI*>(cmdui));
    colorCmdUI.SetColor(CB::Convert(m_pTMgr->GetTransparentColor()));
}

void CBitEditView::OnUpdateColorCustom(wxUpdateUIEvent& pCmdUI)
{
    CCmdUI* cmdui = static_cast<CCmdUI*>(pCmdUI.GetClientData());
    CColorCmdUI& colorCmdUI = CheckedDeref(dynamic_cast<CColorCmdUI*>(cmdui));
    colorCmdUI.SetCustomColors(GetDocument().GetCustomColors());
}

void CBitEditView::OnUpdateLineWidth(wxUpdateUIEvent& pCmdUI)
{
    CCmdUI* cmdui = static_cast<CCmdUI*>(pCmdUI.GetClientData());
    CColorCmdUI& colorCmdUI = CheckedDeref(dynamic_cast<CColorCmdUI*>(cmdui));
    colorCmdUI.SetLineWidth(m_pTMgr->GetLineWidth());
}

#if 0
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
    m_pSelView->UpdateViewImage();
}

void CBitEditView::OnUpdateImageBoardMask(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_pSelView->GetCurrentScale() != smallScale &&
        !GetDocument().GetBoardManager()->IsEmpty());
}
#endif

void CBitEditView::OnUpdateViewZoomIn(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_nZoom < 8);
}

void CBitEditView::OnUpdateViewZoomOut(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_nZoom > 1);
}

void CBitEditView::OnViewZoomIn(wxCommandEvent& /*event*/)
{
    switch (m_nZoom)
    {
        case 1: m_nZoom = 2; break;
        case 2: m_nZoom = 6; break;
        case 6: m_nZoom = 8; break;
    }
    RecalcScrollLimits();
    if (m_nCurToolID == XRCID("ID_ITOOL_TEXT"))
        SetTextCaretPos(m_ptCaret);
    Refresh();
}

void CBitEditView::OnViewZoomOut(wxCommandEvent& /*event*/)
{
    switch (m_nZoom)
    {
        case 8: m_nZoom = 6; break;
        case 6: m_nZoom = 2; break;
        case 2: m_nZoom = 1; break;
    }
    RecalcScrollLimits();
    if (m_nCurToolID == XRCID("ID_ITOOL_TEXT"))
        SetTextCaretPos(m_ptCaret);
    Refresh();
}

void CBitEditView::OnViewToggleScale(wxCommandEvent& /*event*/)
{
    switch (m_nZoom)
    {
        case 1: m_nZoom = 2; break;
        case 2: m_nZoom = 6; break;
        case 6: m_nZoom = 8; break;
        case 8: m_nZoom = 1; break;
    }
    RecalcScrollLimits();
    if (m_nCurToolID == XRCID("ID_ITOOL_TEXT"))
        SetTextCaretPos(m_ptCaret);
    Refresh();
}

void CBitEditView::OnUpdateEnable(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
}

void CBitEditView::OnDwgFont(wxCommandEvent& /*event*/)
{
    if (m_pTMgr->DoBitFontDialog())
    {
        m_fontID = m_pTMgr->GetFontID();
        UpdateFontInfo();
        if (m_nCurToolID == XRCID("ID_ITOOL_TEXT"))
            UpdateTextView();
    }
}

void CBitEditView::OnEditUndo(wxCommandEvent& /*event*/)
{
    if (!IsUndoAvailable())
        return;
    RestoreUndoToView();
    SetMasterImageFromViewImage();
    InvalidateViewImage(false);
    m_pSelView->UpdateViewImage();
}

void CBitEditView::OnUpdateEditUndo(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(IsUndoAvailable());
}

#if 0
void CBitEditView::OnEditCopy()
{
    if (m_nCurToolID == XRCID("ID_ITOOL_SELECT") && m_bmPaste->m_hObject != NULL &&
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
        m_bmPaste = CloneScaledBitmap(*pBMap, m_size);
        m_rctPaste.SetRect(0, 0, m_size.cx, m_size.cy);
    }
    else
    {
        m_bmPaste = CloneBitmap(*pBMap);
        m_rctPaste.SetRect(0, 0, bmInfo.bmWidth, bmInfo.bmHeight);
    }


    m_nLastToolID = m_nCurToolID;
    m_nCurToolID = XRCID("ID_ITOOL_SELECT");

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
#endif

void CBitEditView::OnUpdateIndicatorCellNum(wxUpdateUIEvent& pCmdUI)
{
    wxPoint point = wxGetMouseState().GetPosition();
    point = ScreenToClient(point);

    wxRect rct = GetClientRect();

    if (rct.Contains(point))
    {
        ClientToWorkspace(point);
        if (GetImagePixelLoc(point))
        {
            CB::string szCoord = std::format(L"X={}, Y={}", point.x, point.y);
            pCmdUI.Enable(true);
            pCmdUI.SetText(szCoord);
        }
    }
}

void CBitEditViewContainer::OnDraw(CDC* pDC)
{
    // do nothing because child covers entire client rect
}

void CBitEditViewContainer::OnInitialUpdate()
{
    child->OnInitialUpdate();
}

void CBitEditViewContainer::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    child->OnUpdate(pSender, lHint, pHint);
}

CBitEditViewContainer::CBitEditViewContainer() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

int CBitEditViewContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    child = new CBitEditView(*this);

    return 0;
}

void CBitEditViewContainer::OnSize(UINT nType, int cx, int cy)
{
    child->SetSize(0, 0, cx, cy);
    return CView::OnSize(nType, cx, cy);
}

// MFC puts the focus here, so move it to the useful window
void CBitEditViewContainer::OnSetFocus(CWnd* pOldWnd)
{
    CB::OnCmdMsgOverride<CView>::OnSetFocus(pOldWnd);
    child->SetFocus();
}
