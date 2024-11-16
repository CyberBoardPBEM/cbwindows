// VwTilesl.cpp : implementation file
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

IMPLEMENT_DYNCREATE(CTileSelViewContainer, CView)

CTileSelView::CTileSelView(CTileSelViewContainer& p) :
    parent(&p),
    document(dynamic_cast<CGamDoc*>(parent->GetDocument()))
{
    m_pTileMgr = NULL;
    m_pEditView = NULL;
    m_tid = nullTid;
    m_bNoUpdate = FALSE;
    wxScrolledCanvas::Create(*parent, 0);
}

wxBEGIN_EVENT_TABLE(CTileSelView, wxScrolledCanvas)
    EVT_LEFT_DOWN(OnLButtonDown)
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CTileSelViewContainer, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_MESSAGE(WM_SETCOLOR, OnSetColor)
    ON_MESSAGE(WM_SETCUSTOMCOLOR, OnSetCustomColors)
    ON_MESSAGE(WM_SETLINEWIDTH, OnSetLineWidth)
    ON_UPDATE_COMMAND_UI(ID_COLOR_FOREGROUND, OnUpdateColorForeground)
    ON_UPDATE_COMMAND_UI(ID_COLOR_BACKGROUND, OnUpdateColorBackground)
    ON_UPDATE_COMMAND_UI(ID_COLOR_TRANSPARENT, OnUpdateColorTransparent)
    ON_UPDATE_COMMAND_UI(ID_COLOR_CUSTOM, OnUpdateColorCustom)
    ON_UPDATE_COMMAND_UI(ID_LINE_WIDTH, OnUpdateLineWidth)
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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::OnInitialUpdate()
{
    m_pTileMgr = &GetDocument().GetTileManager();
    m_tid = static_cast<TileID>(reinterpret_cast<uintptr_t>(GetDocument().GetCreateParameter()));
    ASSERT(m_tid != nullTid);

    // Fetch full scale tile
    CTile tile = m_pTileMgr->GetTile(m_tid, fullScale);
    m_bmFull = CB::Convert(*tile.CreateBitmapOfTile());
    m_sizeFull = CB::Convert(tile.GetSize());

    // Fetch half scale tile
    tile = m_pTileMgr->GetTile(m_tid, halfScale);
    m_bmHalf = CB::Convert(*tile.CreateBitmapOfTile());
    m_sizeHalf = CB::Convert(tile.GetSize());

    // Fetch small scale color and create bogus tile for color editing
    tile = m_pTileMgr->GetTile(m_tid, smallScale);
    m_crSmall = tile.GetSmallColor();
    m_sizeSmall = wxSize(8, 8);
    m_bmSmall = CreateColorBitmap(m_sizeSmall, m_crSmall);

    // Setup rectangles
    CalcViewLayout();

    // Finally hand the full size tile to the bit editor.
    ASSERT(m_pEditView != NULL);
    SelectCurrentBitmap(fullScale);
    m_pEditView->SetCurrentBitmap(m_tid, m_bmFull);

    SetSizer(new wxBoxSizer(wxVERTICAL));
    GetSizer()->Add(m_sizeSelArea.x, m_sizeSelArea.y);
    SetScrollRate(8, 8);
}

void CTileSelView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    WORD wHint = LOWORD(lHint);

    if (wHint == HINT_TILEDELETED)
    {
        if (static_cast<CGmBoxHint*>(pHint)->GetArgs<HINT_TILEDELETED>().m_tid == m_tid)
        {
            m_bNoUpdate = TRUE;
            CFrameWnd* pFrm = parent->GetParentFrame();
            ASSERT(pFrm != NULL);
            pFrm->PostMessage(WM_CLOSE, 0, 0L);
        }
    }
    else if (wHint == HINT_TILESETDELETED && !m_pTileMgr->IsTileIDValid(m_tid))
    {
        m_bNoUpdate = TRUE;
        CFrameWnd* pFrm = parent->GetParentFrame();
        ASSERT(pFrm != NULL);
        pFrm->PostMessage(WM_CLOSE, 0, 0L);
    }
    else if (wHint == HINT_FORCETILEUPDATE)
    {
        UpdateDocumentTiles();
    }
    else if (wHint == HINT_ALWAYSUPDATE)
        parent->CView::OnUpdate(pSender, lHint, pHint);
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::SetBitEditor(CBitEditView& pEditView)
{
    m_pEditView = &pEditView;
}

/////////////////////////////////////////////////////////////////////////////
// CTileSelView drawing

void CTileSelView::DrawTile(wxDC& pDC, const wxBitmap& pBMap, wxRect rct)
{
    wxASSERT(pBMap.GetSize() == rct.GetSize());
    pDC.DrawBitmap(pBMap, rct.GetTopLeft());
    rct.Inflate(1, 1);
    wxDCBrushChanger setBrush(pDC, *wxTRANSPARENT_BRUSH);
    wxDCPenChanger setPen(pDC, wxPen(*wxBLACK));
    pDC.DrawRectangle(rct);
}

void CTileSelView::OnDraw(wxDC& pDC)
{
    wxRect rctActive;

    if (m_eCurTile != fullScale)
        DrawTile(pDC, m_bmFull, m_rctFull);
    else
        rctActive = m_rctFull;

    if (m_eCurTile != halfScale)
        DrawTile(pDC, m_bmHalf, m_rctHalf);
    else
        rctActive = m_rctHalf;

    if (m_eCurTile != smallScale)
        DrawTile(pDC, m_bmSmall, m_rctSmall);
    else
        rctActive = m_rctSmall;

    const wxBitmap& pbm = m_pEditView->GetCurrentViewBitmap();
    wxASSERT(pbm.IsOk());

    DrawTile(pDC, pbm, rctActive);

    rctActive.Inflate(nBorderWidth + 1, nBorderWidth + 1);
    Draw25PctPatBorder(*this, pDC, rctActive, nBorderWidth);
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::UpdateViewPixel(wxPoint pt, UINT nBrushSize, const wxBrush& pBrush)
{
    int nSize;
    int nSizeX;
    int nSizeY;

    wxPoint pnt = GetActiveTileLoc();
    wxSize size = m_pEditView->GetBitmapSize();

    nSizeX = pt.x - nBrushSize / 2;
    nSizeY = pt.y - nBrushSize / 2;

    wxPoint ptScroll = CalcUnscrolledPosition(wxPoint(0, 0));

    RefreshRect(wxRect(wxPoint(pnt.x + (nSizeX >= 0 ? nSizeX : 0) - ptScroll.x,
        pnt.y + (nSizeY >= 0 ? nSizeY : 0) - ptScroll.y),
        wxSize(((nSize = size.x - nSizeX) >= (int)nBrushSize ? nBrushSize : nSize),
        ((nSize = size.y - nSizeY) >= (int)nBrushSize ? nBrushSize : nSize))));
}

void CTileSelView::UpdateViewImage()
{
    wxRect rct = GetActiveTileRect();
    rct.SetLeftTop(CalcScrolledPosition(rct.GetTopLeft()));
    GetDocument().SetModifiedFlag();
    RefreshRect(rct, FALSE);
    Update();
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::UpdateDocumentTiles()
{
    CGamDoc& pDoc = GetDocument();
    // Make sure our bitmaps are up to date.
    GetActiveBitmap() = CloneBitmap(m_pEditView->GetCurrentViewBitmap());

    // Get small scale color from tiny bitmap and update tile
    /* https://docs.wxwidgets.org/stable/classwx_d_c.html#abe0f8a22ec58783bd728f01e493040d1
        says wxDC::GetPixel() doesn't work in wxOSX */
    wxColour crSmall = GetPixel(m_bmSmall, 0, 0);

    // The update happens here...
    m_pTileMgr->UpdateTile(m_tid, *CB::Convert(m_bmFull), *CB::Convert(m_bmHalf), CB::Convert(crSmall));

    // Finally handle various notifications
    CGmBoxHint hint;
    hint.GetArgs<HINT_TILEMODIFIED>().m_tid = m_tid;
    pDoc.UpdateAllViews(&*parent, HINT_TILEMODIFIED, &hint);
    pDoc.SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::DoTileResizeDialog()
{
    CResizeTileDialog dlg;
    GetActiveBitmap() = CloneBitmap(m_pEditView->GetCurrentViewBitmap());
    dlg.m_pBMgr = &GetDocument().GetBoardManager();
    dlg.m_bRescaleBMaps = true;
    dlg.m_nWidth = m_sizeFull.x;
    dlg.m_nHeight = m_sizeFull.y;
    if (dlg.ShowModal() == wxID_OK)
    {
        // Copy current bitmaps for undo
        PurgeUndo();            // Make sure no mem leaks.
        m_pBmFullUndo = CloneBitmap(m_bmFull);
        m_pBmHalfUndo = CloneBitmap(m_bmHalf);

        wxBitmap bmFull;
        wxBitmap bmHalf;
        m_sizeFull = wxSize(dlg.m_nWidth, dlg.m_nHeight);
        m_sizeHalf = wxSize(dlg.m_nHalfWidth, dlg.m_nHalfHeight);

        if (dlg.m_bRescaleBMaps)
        {
            bmFull = CloneScaledBitmap(m_bmFull, m_sizeFull);
            bmHalf = CloneScaledBitmap(m_bmHalf, m_sizeHalf);
        }
        else
        {
            bmFull = CreateColorBitmap(m_sizeFull, *wxWHITE);
            bmHalf = CreateColorBitmap(m_sizeHalf, *wxWHITE);
            MergeBitmap(bmFull, m_bmFull, wxPoint(0,0));
            MergeBitmap(bmHalf, m_bmHalf, wxPoint(0,0));
        }
        m_bmFull = std::move(bmFull);
        m_bmHalf = std::move(bmHalf);
        // Recompute selection window layout.
        CalcViewLayout();

        // Finally hand the full size tile to the bit editor.
        SelectCurrentBitmap(fullScale);
        m_pEditView->SetCurrentBitmap(m_tid, m_bmFull);
        Refresh();
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::DoTileRotation(int nAngle)
{
    GetActiveBitmap() = CloneBitmap(m_pEditView->GetCurrentViewBitmap());
    wxBitmap dibFull = CloneBitmap(m_bmFull);
    wxBitmap dibHalf = CloneBitmap(m_bmHalf);

    wxBitmap pbmFull;
    wxBitmap pbmHalf;

    switch (nAngle)
    {
        case 90:
            pbmFull = wxBitmap(dibFull.ConvertToImage().Rotate90(true));
            pbmHalf = wxBitmap(dibHalf.ConvertToImage().Rotate90(true));
            break;
        case 180:
            pbmFull = wxBitmap(dibFull.ConvertToImage().Rotate180());
            pbmHalf = wxBitmap(dibHalf.ConvertToImage().Rotate180());
            break;
        case 270:
            pbmFull = wxBitmap(dibFull.ConvertToImage().Rotate90(false));
            pbmHalf = wxBitmap(dibHalf.ConvertToImage().Rotate90(false));
            break;
        default:
            wxASSERT("invalid argument");
    }

    m_bmFull = std::move(pbmFull);
    m_bmHalf = std::move(pbmHalf);
    // Recompute selection window layout.
    CalcViewLayout();
    // Finally hand the full size tile to the bit editor.
    SelectCurrentBitmap(fullScale);
    m_pEditView->SetCurrentBitmap(m_tid, m_bmFull);
    Refresh();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CTileSelView::IsUndoAvailable() const
{
    wxASSERT(!"unreachable code?");
    return m_pBmFullUndo.IsOk();
}

void CTileSelView::PurgeUndo()
{
    m_pBmFullUndo = wxBitmap();
    m_pBmHalfUndo = wxBitmap();
}

void CTileSelView::RestoreFromUndo()
{
    wxASSERT(!"unreachable code?");
    wxASSERT(m_pBmFullUndo.IsOk());
    wxASSERT(m_pBmHalfUndo.IsOk());
    if (!m_pBmFullUndo.IsOk())
        return;

    m_bmFull = std::move(m_pBmFullUndo);
    m_bmHalf = std::move(m_pBmHalfUndo);

    PurgeUndo();            // Dump empty objects.

    // Recompute selection window layout.
    CalcViewLayout();

    // Finally hand the full size tile to the bit editor.
    SelectCurrentBitmap(fullScale);
    m_pEditView->SetCurrentBitmap(m_tid, m_bmFull);
    Refresh();
}

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::CalcViewLayout()
{
    // Redo sizes since some callers count on it.

    m_sizeFull = m_bmFull.GetSize();
    m_sizeHalf = m_bmHalf.GetSize();

    m_rctFull = wxRect(wxPoint(0, 0), m_sizeFull);
    m_rctFull.Offset(xGap, yGap);
    m_rctHalf = wxRect(wxPoint(0, 0), m_sizeHalf);
    m_rctHalf.Offset(xGap + (m_sizeFull.x - m_sizeHalf.x) / 2,
        yGap + m_rctFull.GetBottom() + 1 + 2);
    m_rctSmall = wxRect(wxPoint(0, 0), m_sizeSmall);
    m_rctSmall.Offset(xGap + (m_sizeFull.x - m_sizeSmall.x) / 2,
        yGap + m_rctHalf.GetBottom() + 1 + 2);

    m_sizeSelArea.x = m_rctFull.GetRight() + 1 + nBorderWidth;
    m_sizeSelArea.y = m_rctSmall.GetBottom() + 1 + nBorderWidth;

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

wxRect CTileSelView::GetActiveTileRect() const
{
    if (m_eCurTile == fullScale)
        return m_rctFull;
    else if (m_eCurTile == halfScale)
        return m_rctHalf;
    else
        return m_rctSmall;
}

wxPoint CTileSelView::GetActiveTileLoc() const
{
    if (m_eCurTile == fullScale)
        return m_rctFull.GetTopLeft();
    else if (m_eCurTile == halfScale)
        return m_rctHalf.GetTopLeft();
    else
        return m_rctSmall.GetTopLeft();
}

wxBitmap& CTileSelView::GetActiveBitmap()
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
        m_pEditView->SetCurrentBitmap(m_tid, m_bmFull);
    else if (eScale == halfScale)
        m_pEditView->SetCurrentBitmap(m_tid, m_bmHalf);
    else
        m_pEditView->SetCurrentBitmap(m_tid, m_bmSmall, TRUE);
    m_eCurTile = eScale;
}

/////////////////////////////////////////////////////////////////////////////
// CTileSelView message handlers

void CTileSelView::OnLButtonDown(wxMouseEvent& event)
{
    wxPoint point = CalcUnscrolledPosition(event.GetPosition());
    TileScale eScale;
    if (m_rctFull.Contains(point))
        eScale = fullScale;
    else if (m_rctHalf.Contains(point))
        eScale = halfScale;
    else if (m_rctSmall.Contains(point))
        eScale = smallScale;
    else
    {
        event.Skip();
        return;
    }
    if (GetCurrentScale() == eScale)
    {
        return;
    }
    GetActiveBitmap() = CloneBitmap(m_pEditView->GetCurrentViewBitmap());

    SelectCurrentBitmap(eScale);
    Refresh();
}

CTileSelView::~CTileSelView()
{
    if (!m_bNoUpdate)
    {
        // Update master copies...
        UpdateDocumentTiles();
    }
}

/////////////////////////////////////////////////////////////////////////////
// CTileSelView command handlers
// Hot potato most of these to the editor view.

LRESULT CTileSelViewContainer::OnSetColor(WPARAM wParam, LPARAM lParam)
{
    wxASSERT(!"unreachable code?");
#if 0
    return child->GetBitEditor().OnSetColor(wParam, lParam);
#else
    return 0;
#endif
}

LRESULT CTileSelViewContainer::OnSetCustomColors(WPARAM wParam, LPARAM lParam)
{
    wxASSERT(!"unreachable code?");
#if 0
    return child->GetBitEditor().OnSetCustomColors(wParam, lParam);
#else
    return 0;
#endif
}

LRESULT CTileSelViewContainer::OnSetLineWidth(WPARAM wParam, LPARAM lParam)
{
    wxASSERT(!"unreachable code?");
#if 0
    return child->GetBitEditor().OnSetLineWidth(wParam, lParam);
#else
    return 0;
#endif
}

void CTileSelViewContainer::OnUpdateColorForeground(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnUpdateColorForeground(pCmdUI);
#endif
}

void CTileSelViewContainer::OnUpdateColorBackground(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnUpdateColorBackground(pCmdUI);
#endif
}

void CTileSelViewContainer::OnUpdateColorTransparent(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnUpdateColorTransparent(pCmdUI);
#endif
}

void CTileSelViewContainer::OnUpdateColorCustom(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnUpdateColorCustom(pCmdUI);
#endif
}

void CTileSelViewContainer::OnUpdateLineWidth(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnUpdateLineWidth(pCmdUI);
#endif
}

void CTileSelViewContainer::OnUpdateToolPalette(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnUpdateToolPalette(pCmdUI);
#endif
}

BOOL CTileSelViewContainer::OnToolPalette(UINT id)
{
    wxASSERT(!"unreachable code?");
#if 0
    return child->GetBitEditor().OnToolPalette(id);
#else
    return 0;
#endif
}

void CTileSelViewContainer::OnEditPaste()
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnEditPaste();
#endif
}

void CTileSelViewContainer::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnUpdateEditPaste(pCmdUI);
#endif
}

void CTileSelViewContainer::OnEditCopy()
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnEditCopy();
#endif
}

void CTileSelViewContainer::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnUpdateEditCopy(pCmdUI);
#endif
}

void CTileSelViewContainer::OnEditUndo()
{
    wxASSERT(!"unreachable code?");
#if 0
    if (child->IsUndoAvailable())      // Us first.
        child->RestoreFromUndo();
    else
        child->GetBitEditor().OnEditUndo();
#endif
}

void CTileSelViewContainer::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    if (child->IsUndoAvailable())      // Us first.
        pCmdUI->Enable(TRUE);
    else
        child->GetBitEditor().OnUpdateEditUndo(pCmdUI);
#endif
}

void CTileSelViewContainer::OnViewToggleScale()
{
    wxASSERT(!"unreachable code?");
#if 0
    child->GetBitEditor().OnViewToggleScale();
#endif
}

void CTileSelViewContainer::OnDraw(CDC* pDC)
{
    // do nothing because child covers entire client rect
}

void CTileSelViewContainer::OnInitialUpdate()
{
    child->OnInitialUpdate();
}

void CTileSelViewContainer::OnActivateView(BOOL bActivate,
    CView* pActivateView,
    CView* /*pDeactiveView*/)
{
    WXUNUSED_UNLESS_DEBUG(pActivateView);
    if (bActivate)
    {
        wxASSERT(pActivateView == this);
        CBitEditView& bitEdit = child->GetBitEditor();
        wxWindow& bitEditParent = CheckedDeref(bitEdit.GetParent());
        CView& view = CheckedDeref(dynamic_cast<CView*>(CB::ToCWnd(bitEditParent)));
        GetParentFrame()->SetActiveView(&view);
    }
}

void CTileSelViewContainer::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    child->OnUpdate(pSender, lHint, pHint);
}

CTileSelViewContainer::CTileSelViewContainer() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

int CTileSelViewContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    child = new CTileSelView(*this);

    return 0;
}

void CTileSelViewContainer::OnSize(UINT nType, int cx, int cy)
{
    child->SetSize(0, 0, cx, cy);
    return CView::OnSize(nType, cx, cy);
}
