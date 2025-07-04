// VwTilesl.cpp : implementation file
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
#include    "Gm.h"
#include    "GmDoc.h"
#include    "CDib.h"
#include    "VwTilesl.h"
#include    "FrmBited.h"
#include    "VwBitedt.h"
#include    "DlgTilsz.h"
#include    "FrmMain.h"     // TODO:  remove?

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int xGap = 10;
const int yGap = 10;
const int nBorderWidth = 5;

/////////////////////////////////////////////////////////////////////////////
// CTileSelView

CTileSelView::CTileSelView(wxSplitterWindow& p,
                            wxBitEditView& v,
                            TileID tid) :
    m_tid(tid),
    parent(&p),
    document(&v.GetDocument()),
    view(&v)
{
    m_pTileMgr = NULL;
    m_pEditView = NULL;
    m_bNoUpdate = FALSE;
    wxScrolledCanvas::Create(&*parent, 0);
}

wxBEGIN_EVENT_TABLE(CTileSelView, wxScrolledCanvas)
    EVT_LEFT_DOWN(OnLButtonDown)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////

void CTileSelView::OnInitialUpdate()
{
    m_pTileMgr = &GetDocument().GetTileManager();
    wxASSERT(m_tid != nullTid);

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
    m_crSmall = CB::Convert(tile.GetSmallColor());
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
            // close this view
            /* can't delete now
                because iterator in caller would be invalidated */
            wxTheApp->ScheduleForDestruction(&*view);
        }
    }
    else if (wHint == HINT_TILESETDELETED && !m_pTileMgr->IsTileIDValid(m_tid))
    {
        m_bNoUpdate = TRUE;
        // close this view
        /* can't delete now
            because iterator in caller would be invalidated */
        wxTheApp->ScheduleForDestruction(&*view);
    }
    else if (wHint == HINT_FORCETILEUPDATE)
    {
        UpdateDocumentTiles();
    }
    else if (wHint == HINT_ALWAYSUPDATE)
    {
        wxASSERT(!"untested code");
#if 0
        parent->CView::OnUpdate(pSender, lHint, pHint);
#endif
    }
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
    pDoc.UpdateAllViews(&*view, CGmBoxHintWx(HINT_TILEMODIFIED, &hint));
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
