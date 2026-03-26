// WinPoptb.h : header file
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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

#ifndef _WINPOPTB_H_
#define _WINPOPTB_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardPopup window

class CTinyBoardPopup : public wxPopupTransientWindow
{
// Construction
public:
    CTinyBoardPopup(CWnd& pWnd);

// Attributes
private:
    RefPtr<CWnd> m_pWnd;
public:
    wxBitmap    m_bmap;
    wxSize      m_wsize;
    wxSize      m_vsize;
    BOOL        m_bRotate180;       // Board is rotated by 180 degrees

// Operations
public:
    BOOL Create(wxWindow& pParent, wxPoint ptCenter);

// Overrides
    public:
    protected:

// Implementation
public:
    ~CTinyBoardPopup() override;

private:
    void ProcessBoardHit(wxMouseEvent& event);

    void OnRButtonDown(wxMouseEvent& event);
    /* N.B.:  OnLButtonDown() instead of ProcessLeftDown()
    see https://groups.google.com/g/wx-dev/c/506M_PpcsZk/m/FODigxNUAQAJ
    */
#if 1
    void OnLButtonDown(wxMouseEvent& event);
#else
    bool ProcessLeftDown(wxMouseEvent& event) override;
#endif
#if 0
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
#endif
    void OnPaint(wxPaintEvent& event);
    void OnChar(wxKeyEvent& event);
    wxDECLARE_EVENT_TABLE();

    /* default position policy wants no overlap between arg rect
        and this, but we want as close to leftTop as screen
        permits */
    void Position(const wxPoint& leftTop,
        const wxSize&) override;
    void Dismiss() override;
};

/////////////////////////////////////////////////////////////////////////////

#endif

