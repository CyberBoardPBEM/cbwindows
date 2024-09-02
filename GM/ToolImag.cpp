// ToolImag.cpp
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
#include    "FrmMain.h"
#include    "ResTbl.h"
#include    "GdiTools.h"
#include    "ToolImag.h"
#include    "VwBitedt.h"
#include    "VwTilesl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////
// Class variables

std::vector<CImageTool*> CImageTool::c_toolLib;     // Bit Image Tool library

wxPoint CImageTool::c_ptDown;        // Mouse down location
wxPoint CImageTool::c_ptLast;        // Last mouse location

// The tool objects...

static CBitPencilTool       s_bitPencilTool(itoolPencil);
static CBitPencilTool       s_bitBrushTool(itoolBrush);
static CBitSelectTool       s_bitSelectTool;
static CBitFillTool         s_bitFillTool;
static CBitColorChangeTool  s_bitColorChageTool;
static CBitTextTool         s_bitTextTool;
static CBitLineTool         s_bitLineTool;
static CBitRectTool         s_bitRectTool(itoolRect);
static CBitRectTool         s_bitRectFillTool(itoolFillRect);
static CBitOvalTool         s_bitOvalTool(itoolOval);
static CBitOvalTool         s_bitOvalFillTool(itoolFillOval);
static CBitDropperTool      s_bitDropperTool;

////////////////////////////////////////////////////////////////////////
// CImageTool - Basic tool support (abstract class)

CImageTool::CImageTool(IToolType eType) :
    m_eToolType(eType)
{
    size_t i = static_cast<size_t>(eType);
    c_toolLib.resize(std::max(i + size_t(1), c_toolLib.size()));
    wxASSERT(!c_toolLib[i]);
    c_toolLib[i] = this;
}

CImageTool& CImageTool::GetTool(IToolType eToolType)
{
    if (eToolType == itoolPencil)
        return s_bitPencilTool;            // Optimization
    size_t i = static_cast<size_t>(eToolType);
    wxASSERT(i < c_toolLib.size());
    CImageTool& retval = CheckedDeref(c_toolLib[i]);
    wxASSERT(retval.m_eToolType == eToolType);
    return retval;
}

void CImageTool::OnLButtonDown(CBitEditView& pView, int /*nMods*/, wxPoint point)
{
    pView.CaptureMouse();

    c_ptDown = point;
    c_ptLast = point;
}

void CImageTool::OnMouseMove(CBitEditView& pView, int /*nMods*/, wxPoint point)
{
    if (pView.HasCapture())
        c_ptLast = point;
//  SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void CImageTool::OnLButtonUp(CBitEditView& pView, int /*nMods*/, wxPoint /*point*/)
{
    if (!pView.HasCapture())
        return;
    pView.ReleaseMouse();
}

void CImageTool::OnMouseCaptureLost(CBitEditView& pView)
{
}

////////////////////////////////////////////////////////////////////////
// CBitSelectTool

void CBitSelectTool::OnLButtonDown(CBitEditView& pView, int nMods,
    wxPoint point)
{
    m_bMoveMode = FALSE;

    // If the mouse clicked in the rect region of the current paste
    // rect this will be a image move operation. Turn off the focus rect
    // and set image move mode.
    if (pView.IsPtInSelectRect(point))
    {
        m_bMoveMode = TRUE;
        pView.GetImagePixelLoc(point); // to image coords
        // Setup allowable bounds of image point.
        // This is done to make sure drag doesn't leave image.
        wxRect rct = pView.GetSelectRect();
        wxSize sizeSel = rct.GetSize();
        wxSize sizeBMap = pView.GetBitmapSize();
        wxPoint ptRel = point - rct.GetTopLeft();
        m_rctBound = wxRect(wxPoint(- (sizeSel.x - ptRel.x - 1),
            -(sizeSel.y - ptRel.y - 1)),
            wxSize(sizeBMap.x + sizeSel.x - 1, sizeBMap.y + sizeSel.y - 1));
        // Lock down starting point
        pView.SetSelectToolControl(TRUE);
        CImageTool::OnLButtonDown(pView, nMods, point);
        pView.InvalidateFocusBorder();
        return;
    }
    if (!pView.GetImagePixelLoc(point))
        return;                         // Not in image
    // If the mouse click is not in the rect region of the current
    // paste image, or a paste image simply doesn't exist, start tracking
    // a select rectangle. The previous paste image should be merged
    // into the master image prior to select tracking.
    if (pView.IsPasteImage())
    {
        pView.SetUndoFromView();
        pView.SetMasterImageFromViewImage();
        pView.ClearPasteImage();
    }
    pView.InvalidateFocusBorder();
    pView.SetSelectRect(wxRect(point, wxSize(0, 0)));
    pView.SetSelectToolControl(TRUE);
    CImageTool::OnLButtonDown(pView, nMods, point);
}

void CBitSelectTool::OnMouseMove(CBitEditView& pView, int nMods, wxPoint point)
{
    if (!pView.HasCapture())
    {
        m_bMoveMode = FALSE;
        pView.SetSelectToolControl(FALSE);
        return;
    }
    // If we are in move mode, offset the current paste rect.
    if (m_bMoveMode)
    {
        pView.GetImagePixelLoc(point);
        // Make sure drag doesn't leave image.
        point.x = CB::max(point.x, m_rctBound.GetLeft());
        point.y = CB::max(point.y, m_rctBound.GetTop());
        point.x = CB::min(point.x, m_rctBound.GetRight());
        point.y = CB::min(point.y, m_rctBound.GetBottom());

        if (point == c_ptLast)
            return;
        wxPoint size = point - c_ptLast;
        pView.OffsetSelectRect(wxSize(size.x, size.y));      // Makes sure part always in image.
        pView.DrawPastedImage();
    }
    else
    {
        // If we are in select mode resize the select rect. The select
        // rect isn't allowed to leave the image.
        pView.GetImagePixelLocClamped(point);
        if (point == c_ptLast)
            return;
        pView.DrawImageSelectRect(c_ptDown, point);
    }
    CImageTool::OnMouseMove(pView, nMods, point);
}

void CBitSelectTool::OnLButtonUp(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.HasCapture())
    {
        m_bMoveMode = FALSE;
        return;
    }
    if (!m_bMoveMode)
    {
        pView.InvalidateFocusBorder();

        // If we are selecting a part of the image rect we need to
        // extract that part of the image into the paste bitmap.

        pView.GetImagePixelLocClamped(point);
        pView.SetViewImageFromMasterImage();   // Restore original

        wxRect rct(wxPoint(std::min(c_ptDown.x, point.x), std::min(c_ptDown.y, point.y)),
                    wxSize(std::abs(point.x - c_ptDown.x), std::abs(point.y - c_ptDown.y)));
        if (!rct.IsEmpty())
        {
            rct.SetWidth(rct.GetWidth() + 1);
            rct.SetHeight(rct.GetHeight() + 1);
            // Copy selected area into the paste bitmap.
            pView.SetUndoFromView();
            pView.GetPasteBitmap() = CutBitmapPiece(
                pView.GetCurrentMasterBitmap(), rct,
                pView.GetBackColor());
            pView.InvalidateFocusBorder();
        }
        pView.SetSelectRect(rct);
        pView.GetTileSelectView().UpdateViewImage();
        pView.InvalidateViewImage(false);
    }
    else
        pView.InvalidateFocusBorder();

    pView.SetSelectToolControl(FALSE);
    CImageTool::OnLButtonUp(pView, nMods, point);
}

void CBitSelectTool::OnMouseCaptureLost(CBitEditView& pView)
{
    pView.InvalidateFocusBorder();
    if (!m_bMoveMode)
    {
        pView.SetViewImageFromMasterImage();   // Restore original
        pView.InvalidateViewImage(false);
    }

    pView.SetSelectToolControl(FALSE);
    CImageTool::OnMouseCaptureLost(pView);
}

wxCursor CBitSelectTool::OnSetCursor(CBitEditView& pView, wxPoint point)
{
    // If the point is in the selected rect, show the move image cursor.
    // If the point is in the bit image, show the crosshair

    if (pView.IsPtInSelectRect(point))
        return g_res.hcrSmall4WayWx;
    else if (pView.IsPtInImage(point))
        return g_res.hcrCrossHairWx;
    else
        return wxNullCursor;
}

////////////////////////////////////////////////////////////////////////
// CBitPencilTool

void CBitPencilTool::OnLButtonDown(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.GetImagePixelLoc(point))
        return;                         // Not in image
    pView.SetUndoFromView();
    pView.DrawImagePixel(point,
        m_eToolType == itoolPencil ? 1 : pView.GetLineWidth());
    CImageTool::OnLButtonDown(pView, nMods, point);
}

void CBitPencilTool::OnMouseMove(CBitEditView& pView, int nMods, wxPoint point)
{
    if (!pView.HasCapture()) return;
    if (!pView.GetImagePixelLoc(point)) return;    // Not in image
    if (point == c_ptLast) return;
    pView.DrawImageToPixel(c_ptLast, point,
        m_eToolType == itoolPencil ? 1 : pView.GetLineWidth());
    CImageTool::OnMouseMove(pView, nMods, point);
}

void CBitPencilTool::OnLButtonUp(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.HasCapture())
        return;
    pView.SetMasterImageFromViewImage();
    CImageTool::OnLButtonUp(pView, nMods, point);
}

void CBitPencilTool::OnMouseCaptureLost(CBitEditView& pView)
{
    pView.InvalidateFocusBorder();
    pView.SetViewImageFromMasterImage();   // Restore original
    pView.InvalidateViewImage(false);
    CImageTool::OnMouseCaptureLost(pView);
}

wxCursor CBitPencilTool::OnSetCursor(CBitEditView& pView, wxPoint point)
{
    if (!pView.IsPtInImage(point))
        return wxNullCursor;

    return m_eToolType == itoolPencil ? g_res.hcrPencilWx : g_res.hcrBrushWx;
}

////////////////////////////////////////////////////////////////////////
// CBitLineTool

void CBitLineTool::OnLButtonDown(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.GetImagePixelLoc(point))
        return;                         // Not in image
    pView.SetUndoFromView();
    pView.DrawImagePixel(point, pView.GetLineWidth());
    CImageTool::OnLButtonDown(pView, nMods, point);
}

void CBitLineTool::OnMouseMove(CBitEditView& pView, int nMods, wxPoint point)
{
    if (!pView.HasCapture()) return;
    pView.GetImagePixelLoc(point);
    if (point == c_ptLast) return;
    pView.DrawImageLine(c_ptDown, point, pView.GetLineWidth());
    CImageTool::OnMouseMove(pView, nMods, point);
}

void CBitLineTool::OnLButtonUp(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.HasCapture())
        return;
    pView.SetMasterImageFromViewImage();
    CImageTool::OnLButtonUp(pView, nMods, point);
}

void CBitLineTool::OnMouseCaptureLost(CBitEditView& pView)
{
    pView.SetViewImageFromMasterImage();   // Restore original
    pView.InvalidateViewImage(false);
    CImageTool::OnMouseCaptureLost(pView);
}

wxCursor CBitLineTool::OnSetCursor(CBitEditView& pView, wxPoint point)
{
    if (!pView.IsPtInImage(point))
        return wxNullCursor;

    return g_res.hcrCrossHairWx;
}

////////////////////////////////////////////////////////////////////////
// CBitDropperTool

void CBitDropperTool::OnLButtonDown(CBitEditView& pView, int nMods,
    wxPoint point)
{
    // wxDC::GetPixel isn't portable
    pView.GetImagePixelLoc(point);
    wxColour crPxl = GetPixel(pView.GetCurrentViewBitmap(),
                                point.x, point.y);
    if ((nMods & (wxMOD_CMD | wxMOD_SHIFT)) == 0)
        pView.SetForeColor(crPxl);
    else if (nMods & wxMOD_SHIFT)
        pView.SetBackColor(crPxl);
    pView.CaptureMouse();
}

void CBitDropperTool::OnLButtonUp(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.HasCapture())
        return;
    // wxDC::GetPixel isn't portable
    pView.GetImagePixelLoc(point);
    wxColour crPxl = GetPixel(pView.GetCurrentViewBitmap(),
                                point.x, point.y);

    if ((nMods & (wxMOD_CMD | wxMOD_SHIFT)) == 0)
        pView.SetForeColor(crPxl);
    else if (nMods & wxMOD_SHIFT)
        pView.SetBackColor(crPxl);

    pView.ReleaseMouse();
    pView.RestoreLastTool();           // For convenience.
}

void CBitDropperTool::OnMouseCaptureLost(CBitEditView& pView)
{
    pView.RestoreLastTool();           // For convenience.
    CImageTool::OnMouseCaptureLost(pView);
}

wxCursor CBitDropperTool::OnSetCursor(CBitEditView& pView, wxPoint point)
{
    return g_res.hcrDropperWx;
}

////////////////////////////////////////////////////////////////////////
// CBitRectTool

void CBitRectTool::OnLButtonDown(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.GetImagePixelLoc(point))
        return;                         // Not in image
    pView.SetUndoFromView();
    CImageTool::OnLButtonDown(pView, nMods, point);
}

void CBitRectTool::OnMouseMove(CBitEditView& pView, int nMods, wxPoint point)
{
    if (!pView.HasCapture()) return;
    pView.GetImagePixelLoc(point);
    if (point == c_ptLast) return;

    int nWidth = 0;
    if (m_eToolType == itoolRect)
    {
        nWidth = pView.GetLineWidth();
        nWidth = nWidth ? nWidth : 1;           // Make sure never zero
    }
    pView.DrawImageRect(c_ptDown, point, nWidth);
    CImageTool::OnMouseMove(pView, nMods, point);
}

void CBitRectTool::OnLButtonUp(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.HasCapture())
        return;
    pView.SetMasterImageFromViewImage();
    CImageTool::OnLButtonUp(pView, nMods, point);
}

void CBitRectTool::OnMouseCaptureLost(CBitEditView& pView)
{
    pView.SetViewImageFromMasterImage();   // Restore original
    pView.InvalidateViewImage(false);
    CImageTool::OnMouseCaptureLost(pView);
}

wxCursor CBitRectTool::OnSetCursor(CBitEditView& pView, wxPoint point)
{
    if (!pView.IsPtInImage(point))
        return wxNullCursor;

    return g_res.hcrCrossHairWx;
}

////////////////////////////////////////////////////////////////////////
// CBitOvalTool

void CBitOvalTool::OnLButtonDown(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.GetImagePixelLoc(point))
        return;                                 // Not in image
    pView.SetUndoFromView();
    CImageTool::OnLButtonDown(pView, nMods, point);
}

void CBitOvalTool::OnMouseMove(CBitEditView& pView, int nMods, wxPoint point)
{
    if (!pView.HasCapture()) return;
    pView.GetImagePixelLoc(point);
    if (point == c_ptLast) return;

    int nWidth = 0;
    if (m_eToolType == itoolOval)
    {
        nWidth = pView.GetLineWidth();
        nWidth = nWidth ? nWidth : 1;           // Make sure never zero
    }
    pView.DrawImageEllipse(c_ptDown, point, nWidth);
    CImageTool::OnMouseMove(pView, nMods, point);
}

void CBitOvalTool::OnLButtonUp(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.HasCapture())
        return;
    pView.SetMasterImageFromViewImage();
    CImageTool::OnLButtonUp(pView, nMods, point);
}

void CBitOvalTool::OnMouseCaptureLost(CBitEditView& pView)
{
    pView.SetViewImageFromMasterImage();   // Restore original
    pView.InvalidateViewImage(false);
    CImageTool::OnMouseCaptureLost(pView);
}

wxCursor CBitOvalTool::OnSetCursor(CBitEditView& pView, wxPoint point)
{
    if (!pView.IsPtInImage(point))
        return wxNullCursor;

    return g_res.hcrCrossHairWx;
}

////////////////////////////////////////////////////////////////////////
// CBitFillTool

void CBitFillTool::OnLButtonDown(CBitEditView& pView, int /*nMods*/,
    wxPoint point)
{
    if (!pView.GetImagePixelLoc(point))
        return;                         // Not in image
    pView.SetUndoFromView();
    pView.DrawImageFill(point);
}

wxCursor CBitFillTool::OnSetCursor(CBitEditView& pView, wxPoint point)
{
    if (!pView.IsPtInImage(point))
        return wxNullCursor;

    return g_res.hcrFillWx;
}

////////////////////////////////////////////////////////////////////////
// CBitColorChangeTool

void CBitColorChangeTool::OnLButtonDown(CBitEditView& pView, int /*nMods*/,
    wxPoint point)
{
    if (!pView.GetImagePixelLoc(point))
        return;                         // Not in image
    pView.SetUndoFromView();
    pView.DrawImageChangeColor(point);
}

wxCursor CBitColorChangeTool::OnSetCursor(CBitEditView& pView, wxPoint point)
{
    if (!pView.IsPtInImage(point))
        return wxNullCursor;

    return g_res.hcrColorChangeWx;
}

////////////////////////////////////////////////////////////////////////
// CBitTextTool

void CBitTextTool::OnLButtonDown(CBitEditView& pView, int nMods,
    wxPoint point)
{
    if (!pView.GetImagePixelLoc(point))
        return;                         // Not in image
    pView.SetUndoFromView();
    pView.CommitCurrentText();
    pView.SetTextPosition(point);
}

wxCursor CBitTextTool::OnSetCursor(CBitEditView& pView, wxPoint point)
{
    if (!pView.IsPtInImage(point))
        return wxNullCursor;

    return g_res.hcrCrossHairWx;
}


