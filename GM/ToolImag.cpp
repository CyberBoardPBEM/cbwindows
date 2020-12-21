// ToolImag.cpp
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

CPtrList CImageTool::c_toolLib;     // Bit Image Tool library

CPoint CImageTool::c_ptDown;        // Mouse down location
CPoint CImageTool::c_ptLast;        // Last mouse location

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

CImageTool::CImageTool(IToolType eType)
{
    m_eToolType = eType;
    c_toolLib.AddTail(this);
}

CImageTool* CImageTool::GetTool(IToolType eToolType)
{
    if (eToolType == itoolPencil)
        return &s_bitPencilTool;            // Optimization
    POSITION pos = c_toolLib.GetHeadPosition();
    while (pos != NULL)
    {
        CImageTool* pTool = (CImageTool*)c_toolLib.GetNext(pos);
        if (pTool->m_eToolType == eToolType)
            return pTool;
    }
    return NULL;
}

void CImageTool::OnLButtonDown(CBitEditView* pView, UINT nFlags, CPoint point)
{
    pView->SetCapture();

    c_ptDown = point;
    c_ptLast = point;
}

void CImageTool::OnMouseMove(CBitEditView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() == pView)
        c_ptLast = point;
//  SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void CImageTool::OnLButtonUp(CBitEditView* pView, UINT, CPoint point)
{
    if (CWnd::GetCapture() != pView)
        return;
    ReleaseCapture();
}

////////////////////////////////////////////////////////////////////////
// CBitSelectTool

void CBitSelectTool::OnLButtonDown(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    m_bMoveMode = FALSE;

    // If the mouse clicked in the rect region of the current paste
    // rect this will be a image move operation. Turn off the focus rect
    // and set image move mode.
    if (pView->IsPtInSelectRect(point))
    {
        m_bMoveMode = TRUE;
        pView->GetImagePixelLoc(point); // to image coords
        // Setup allowable bounds of image point.
        // This is done to make sure drag doesn't leave image.
        CRect rct = pView->GetSelectRect();
        CSize sizeSel = rct.Size();
        CSize sizeBMap = pView->GetBitmapSize();
        CPoint ptRel = (CPoint)(point - rct.TopLeft());
        m_rctBound.SetRect(-( sizeSel.cx - ptRel.x - 1),
            -(sizeSel.cy - ptRel.y - 1),
            sizeBMap.cx + ptRel.x - 1, sizeBMap.cy + ptRel.y - 1);
        // Lock down starting point
        pView->SetSelectToolControl(TRUE);
        CImageTool::OnLButtonDown(pView, nFlags, point);
        pView->InvalidateFocusBorder();
        return;
    }
    if (!pView->GetImagePixelLoc(point))
        return;                         // Not in image
    // If the mouse click is not in the rect region of the current
    // paste image, or a paste image simply doesn't exist, start tracking
    // a select rectangle. The previous paste image should be merged
    // into the master image prior to select tracking.
    if (pView->IsPasteImage())
    {
        pView->SetUndoFromView();
        pView->SetMasterImageFromViewImage();
        pView->ClearPasteImage();
    }
    pView->InvalidateFocusBorder();
    pView->SetSelectRect(CRect(point, CSize(0, 0)));
    pView->SetSelectToolControl(TRUE);
    CImageTool::OnLButtonDown(pView, nFlags, point);
}

void CBitSelectTool::OnMouseMove(CBitEditView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() != pView)
    {
        m_bMoveMode = FALSE;
        pView->SetSelectToolControl(FALSE);
        return;
    }
    // If we are in move mode, offset the current paste rect.
    if (m_bMoveMode)
    {
        pView->GetImagePixelLoc(point);
        // Make sure drag doesn't leave image.
        point.x = CB::max(point.x, m_rctBound.left);
        point.y = CB::max(point.y, m_rctBound.top);
        point.x = CB::min(point.x, m_rctBound.right);
        point.y = CB::min(point.y, m_rctBound.bottom);

        if (point == c_ptLast)
            return;
        CSize size = point - c_ptLast;
        pView->OffsetSelectRect(size);      // Makes sure part always in image.
        pView->DrawPastedImage();
    }
    else
    {
        // If we are in select mode resize the select rect. The select
        // rect isn't allowed to leave the image.
        pView->GetImagePixelLocClamped(point);
        if (point == c_ptLast)
            return;
        pView->DrawImageSelectRect(c_ptDown, point);
    }
    CImageTool::OnMouseMove(pView, nFlags, point);
}

void CBitSelectTool::OnLButtonUp(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (CWnd::GetCapture() != pView)
    {
        m_bMoveMode = FALSE;
        return;
    }
    if (!m_bMoveMode)
    {
        pView->InvalidateFocusBorder();

        // If we are selecting a part of the image rect we need to
        // extract that part of the image into the paste bitmap.

        pView->GetImagePixelLocClamped(point);
        pView->SetViewImageFromMasterImage();   // Restore original

        CRect rct(c_ptDown.x, c_ptDown.y, point.x, point.y);
        rct.NormalizeRect();
        if (!rct.IsRectEmpty())
        {
            rct.right++;                // Include right and bottom sides
            rct.bottom++;
            // Copy selected area into the paste bitmap.
            pView->SetUndoFromView();
            CopyBitmapPiece(pView->GetPasteBitmap(),
                pView->GetCurrentMasterBitmap(), rct,
                pView->GetBackColor());
            pView->InvalidateFocusBorder();
        }
        pView->SetSelectRect(rct);
        pView->GetTileSelectView()->UpdateViewImage(NULL, TRUE);
        pView->InvalidateViewImage(NULL);
    }
    else
        pView->InvalidateFocusBorder();

    pView->SetSelectToolControl(FALSE);
    CImageTool::OnLButtonUp(pView, nFlags, point);
}

BOOL CBitSelectTool::OnSetCursor(CBitEditView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;

    CPoint point;
    GetCursorPos(&point);

    // If the point is in the selected rect, show the move image cursor.
    // If the point is in the bit image, show the crosshair

    pView->ScreenToClient(&point);
    pView->ClientToWorkspace(point);

    if (pView->IsPtInSelectRect(point))
        SetCursor(g_res.hcrSmall4Way);
    else if (pView->IsPtInImage(point))
        SetCursor(g_res.hcrCrossHair);
    else
        return FALSE;

    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CBitPencilTool

void CBitPencilTool::OnLButtonDown(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (!pView->GetImagePixelLoc(point))
        return;                         // Not in image
    pView->SetUndoFromView();
    pView->DrawImagePixel(point,
        m_eToolType == itoolPencil ? 1 : pView->GetLineWidth());
    CImageTool::OnLButtonDown(pView, nFlags, point);
}

void CBitPencilTool::OnMouseMove(CBitEditView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() != pView) return;
    if (!pView->GetImagePixelLoc(point)) return;    // Not in image
    if (point == c_ptLast) return;
    pView->DrawImageToPixel(c_ptLast, point,
        m_eToolType == itoolPencil ? 1 : pView->GetLineWidth());
    CImageTool::OnMouseMove(pView, nFlags, point);
}

void CBitPencilTool::OnLButtonUp(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (CWnd::GetCapture() != pView)
        return;
    pView->SetMasterImageFromViewImage();
    CImageTool::OnLButtonUp(pView, nFlags, point);
}

BOOL CBitPencilTool::OnSetCursor(CBitEditView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    CPoint point;
    GetCursorPos(&point);
    pView->ScreenToClient(&point);
    pView->ClientToWorkspace(point);
    if (!pView->IsPtInImage(point))
        return FALSE;

    SetCursor(m_eToolType == itoolPencil ? g_res.hcrPencil : g_res.hcrBrush);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CBitLineTool

void CBitLineTool::OnLButtonDown(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (!pView->GetImagePixelLoc(point))
        return;                         // Not in image
    pView->SetUndoFromView();
    pView->DrawImagePixel(point, pView->GetLineWidth());
    CImageTool::OnLButtonDown(pView, nFlags, point);
}

void CBitLineTool::OnMouseMove(CBitEditView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() != pView) return;
    pView->GetImagePixelLoc(point);
    if (point == c_ptLast) return;
    pView->DrawImageLine(c_ptDown, point, pView->GetLineWidth());
    CImageTool::OnMouseMove(pView, nFlags, point);
}

void CBitLineTool::OnLButtonUp(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (CWnd::GetCapture() != pView)
        return;
    pView->SetMasterImageFromViewImage();
    CImageTool::OnLButtonUp(pView, nFlags, point);
}

BOOL CBitLineTool::OnSetCursor(CBitEditView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;

    CPoint point;
    GetCursorPos(&point);
    pView->ScreenToClient(&point);
    pView->ClientToWorkspace(point);
    if (!pView->IsPtInImage(point))
        return FALSE;

    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CBitDropperTool

void CBitDropperTool::OnLButtonDown(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    CDC *pDC = pView->GetDC();
    pView->OnPrepareDC(pDC, NULL);
    SetupPalette(pDC);
    COLORREF crPxl = pDC->GetPixel(point);
    ASSERT((long)crPxl != -1);
    ResetPalette(pDC);
    pView->ReleaseDC(pDC);
    if ((nFlags & (MK_CONTROL | MK_SHIFT)) == 0)
        pView->SetForeColor(crPxl);
    else if (nFlags & MK_SHIFT)
        pView->SetBackColor(crPxl);
    pView->SetCapture();
}

void CBitDropperTool::OnLButtonUp(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (CWnd::GetCapture() != pView)
        return;
    pView->WorkspaceToClient(point);
    pView->ClientToScreen(&point);

    CWindowDC dc(CWnd::GetDesktopWindow());
    SetupPalette(&dc);
    COLORREF crPxl = dc.GetPixel(point);
    ASSERT((long)crPxl != -1);
    ResetPalette(&dc);

    if ((nFlags & (MK_CONTROL | MK_SHIFT)) == 0)
        pView->SetForeColor(crPxl);
    else if (nFlags & MK_SHIFT)
        pView->SetBackColor(crPxl);

    ReleaseCapture();
    pView->RestoreLastTool();           // For convenience.
}

BOOL CBitDropperTool::OnSetCursor(CBitEditView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;
    SetCursor(g_res.hcrDropper);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CBitRectTool

void CBitRectTool::OnLButtonDown(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (!pView->GetImagePixelLoc(point))
        return;                         // Not in image
    pView->SetUndoFromView();
    CImageTool::OnLButtonDown(pView, nFlags, point);
}

void CBitRectTool::OnMouseMove(CBitEditView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() != pView) return;
    pView->GetImagePixelLoc(point);
    if (point == c_ptLast) return;

    int nWidth = 0;
    if (m_eToolType == itoolRect)
    {
        nWidth = pView->GetLineWidth();
        nWidth = nWidth ? nWidth : 1;           // Make sure never zero
    }
    pView->DrawImageRect(c_ptDown, point, nWidth);
    CImageTool::OnMouseMove(pView, nFlags, point);
}

void CBitRectTool::OnLButtonUp(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (CWnd::GetCapture() != pView)
        return;
    pView->SetMasterImageFromViewImage();
    CImageTool::OnLButtonUp(pView, nFlags, point);
}

BOOL CBitRectTool::OnSetCursor(CBitEditView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;

    CPoint point;
    GetCursorPos(&point);
    pView->ScreenToClient(&point);
    pView->ClientToWorkspace(point);
    if (!pView->IsPtInImage(point))
        return FALSE;

    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CBitOvalTool

void CBitOvalTool::OnLButtonDown(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (!pView->GetImagePixelLoc(point))
        return;                                 // Not in image
    pView->SetUndoFromView();
    CImageTool::OnLButtonDown(pView, nFlags, point);
}

void CBitOvalTool::OnMouseMove(CBitEditView* pView, UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() != pView) return;
    pView->GetImagePixelLoc(point);
    if (point == c_ptLast) return;

    int nWidth = 0;
    if (m_eToolType == itoolOval)
    {
        nWidth = pView->GetLineWidth();
        nWidth = nWidth ? nWidth : 1;           // Make sure never zero
    }
    pView->DrawImageEllipse(c_ptDown, point, nWidth);
    CImageTool::OnMouseMove(pView, nFlags, point);
}

void CBitOvalTool::OnLButtonUp(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (CWnd::GetCapture() != pView)
        return;
    pView->SetMasterImageFromViewImage();
    CImageTool::OnLButtonUp(pView, nFlags, point);
}

BOOL CBitOvalTool::OnSetCursor(CBitEditView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;

    CPoint point;
    GetCursorPos(&point);
    pView->ScreenToClient(&point);
    pView->ClientToWorkspace(point);
    if (!pView->IsPtInImage(point))
        return FALSE;

    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CBitFillTool

void CBitFillTool::OnLButtonDown(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (!pView->GetImagePixelLoc(point))
        return;                         // Not in image
    pView->SetUndoFromView();
    pView->DrawImageFill(point);
}

BOOL CBitFillTool::OnSetCursor(CBitEditView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;

    CPoint point;
    GetCursorPos(&point);
    pView->ScreenToClient(&point);
    pView->ClientToWorkspace(point);
    if (!pView->IsPtInImage(point))
        return FALSE;

    SetCursor(g_res.hcrFill);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CBitColorChangeTool

void CBitColorChangeTool::OnLButtonDown(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (!pView->GetImagePixelLoc(point))
        return;                         // Not in image
    pView->SetUndoFromView();
    pView->DrawImageChangeColor(point);
}

BOOL CBitColorChangeTool::OnSetCursor(CBitEditView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;

    CPoint point;
    GetCursorPos(&point);
    pView->ScreenToClient(&point);
    pView->ClientToWorkspace(point);
    if (!pView->IsPtInImage(point))
        return FALSE;

    SetCursor(g_res.hcrColorChange);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CBitTextTool

void CBitTextTool::OnLButtonDown(CBitEditView* pView, UINT nFlags,
    CPoint point)
{
    if (!pView->GetImagePixelLoc(point))
        return;                         // Not in image
    pView->SetUndoFromView();
    pView->CommitCurrentText();
    pView->SetTextPosition(point);
}

BOOL CBitTextTool::OnSetCursor(CBitEditView* pView, UINT nHitTest)
{
    if (nHitTest != HTCLIENT)
        return FALSE;

    CPoint point;
    GetCursorPos(&point);
    pView->ScreenToClient(&point);
    pView->ClientToWorkspace(point);
    if (!pView->IsPtInImage(point))
        return FALSE;

    SetCursor(g_res.hcrCrossHair);
    return TRUE;
}


