// FrmDock.h - container window for the marker/tray palettes.
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

#ifndef _FRMDOCK_H
#define _FRMDOCK_H

/////////////////////////////////////////////////////////////////////////////
// CDockPalette window

class CDockPalette : public wxWindow
{
#if 0
    DECLARE_DYNAMIC(CDockPalette);
#endif
// Construction
public:
    CDockPalette();

// Attributes
private:
    wxWindow* GetChild();

// Operations
public:
    void SetChild(wxWindow* pChildWnd);

// Implementation
public:
    ~CDockPalette() override;

    // Generated message map functions
protected:
    // see https ://docs.wxwidgets.org/latest/classwx_window_destroy_event.html
#if 0
    afx_msg void OnDestroy();
#endif
    void OnSize(wxSizeEvent& event);
    wxDECLARE_EVENT_TABLE();
};

/////////////////////////////////////////////////////////////////////////////

#endif