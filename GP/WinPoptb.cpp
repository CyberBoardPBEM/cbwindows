// WinPoptb.cpp : implementation file
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


#include "stdafx.h"
#include "Gp.h"
#include "Gmisc.h"
#include "GdiTools.h"
#include "WinPoptb.h"
#include "CDib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

wxBEGIN_EVENT_TABLE(CTinyBoardPopup, wxPopupTransientWindow)
    EVT_RIGHT_DOWN(OnRButtonDown)
#if 1
    EVT_LEFT_DOWN(OnLButtonDown)
#endif
#if 0
    ON_WM_CREATE()
#endif
    EVT_PAINT(OnPaint)
    EVT_CHAR(OnChar)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardPopup

CTinyBoardPopup::CTinyBoardPopup(CWnd& pWnd) :
    m_pWnd(&pWnd)
{
    m_bRotate180 = FALSE;
}

CTinyBoardPopup::~CTinyBoardPopup()
{
}

/////////////////////////////////////////////////////////////////////////////

BOOL CTinyBoardPopup::Create(wxWindow& pParent, wxPoint ptCenter)
{
    // wxPU_CONTAINS_CONTROLS required to enable having focus
    BOOL retval = wxPopupTransientWindow::Create(&pParent,
                                                wxPU_CONTAINS_CONTROLS | wxBORDER_SUNKEN);
    if (retval)
    {
        SetClientSize(m_vsize);
        wxRect rct(wxPoint(0, 0), GetSize());
        rct.Offset(ptCenter.x - rct.GetWidth()/2, ptCenter.y - rct.GetHeight()/2);
        Position(rct.GetLeftTop(), wxDefaultSize);
        Popup();
    }
    return retval;
}

#if 0
int CTinyBoardPopup::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    SetCapture();

    CRect rct;
    GetWindowRect(&rct);
    CPoint pnt = rct.CenterPoint();
    SetCursorPos(pnt.x, pnt.y);

    return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardPopup message handlers

void CTinyBoardPopup::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);          // device context for painting

    wxMemoryDC dcMem;
    wxRect oRct;

    dcMem.SelectObject(m_bmap);

    wxRect rctClient = GetClientRect();

    if (m_bRotate180)
    {
        dc.StretchBlit(0, 0, rctClient.GetRight(), rctClient.GetBottom(), &dcMem,
            m_vsize.x - 1,m_vsize.y - 1, -m_vsize.x, -m_vsize.y);
    }
    else
    {
        dc.StretchBlit(0, 0, rctClient.GetRight(), rctClient.GetBottom(), &dcMem, 0, 0,
            m_vsize.x, m_vsize.y);
    }
}

void CTinyBoardPopup::ProcessBoardHit(wxMouseEvent& event)
{
    wxPoint point = event.GetPosition();

    wxRect rctClient = GetClientRect();
    if (rctClient.Contains(point))
    {
        ScalePoint(point, m_wsize, rctClient.GetSize());
        if (m_bRotate180)
            point = wxPoint(m_wsize.x - point.x, m_wsize.y - point.y);
        CPoint cpoint = CB::Convert(point);
        m_pWnd->SendMessage(WM_CENTERBOARDONPOINT, (WPARAM)&cpoint);
    }
    Dismiss();
}

void CTinyBoardPopup::OnChar(wxKeyEvent& event)
{
    if (event.GetUnicodeKey() == WXK_ESCAPE)
    {
        Dismiss();
    }
}

void CTinyBoardPopup::OnRButtonDown(wxMouseEvent& event)
{
    ProcessBoardHit(event);
}

#if 1
void CTinyBoardPopup::OnLButtonDown(wxMouseEvent& event)
#else
bool CTinyBoardPopup::ProcessLeftDown(wxMouseEvent& event)
#endif
{
    ProcessBoardHit(event);
#if 1
#else
    return false;
#endif
}

/* default position policy wants no overlap between arg rect
    and this, but we want as close to leftTop as screen
    permits */
void CTinyBoardPopup::Position(const wxPoint& ptOrigin,
                                const wxSize& sizePopup)
{
    // based on wxPopupWindowBase::Position()
    // determine the position and size of the screen we clamp the popup to
    wxPoint posScreen;
    wxSize sizeScreen;

    const int displayNum = wxDisplay::GetFromPoint(ptOrigin);
    if ( displayNum != wxNOT_FOUND )
    {
        const wxRect rectScreen = wxDisplay(displayNum).GetGeometry();
        posScreen = rectScreen.GetPosition();
        sizeScreen = rectScreen.GetSize();
    }
    else // outside of any display?
    {
        // just use the primary one then
        posScreen = wxPoint(0, 0);
        sizeScreen = wxGetDisplaySize();
    }


    const wxSize sizeSelf = GetSize();

    wxCoord y = ptOrigin.y;
    if ( y + sizeSelf.y > posScreen.y + sizeScreen.y )
    {
        y = posScreen.y + sizeScreen.y - sizeSelf.y;
    }

    // now check left/right too
    wxCoord x = ptOrigin.x;

    if ( wxTheApp->GetLayoutDirection() == wxLayout_RightToLeft )
    {
        wxASSERT(!"untested code");
        // shift the window to the left instead of the right.
        x -= sizeSelf.x;        // also shift it by window width.
    }


    if ( x + sizeSelf.x > posScreen.x + sizeScreen.x )
    {
        x = posScreen.x + sizeScreen.x - sizeSelf.x;
    }

    Move(x, y, wxSIZE_NO_ADJUSTMENTS);
}

void CTinyBoardPopup::Dismiss()
{
    /* KLUDGE:  sometimes getting multiple dismisses, but should
                not destroy multiple times */
    if (IsShown())
    {
        wxPopupTransientWindow::Dismiss();
        Destroy();
    }
}
