// Editnocv.h : header file
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

#ifndef __EDITNOCV_H__
#define __EDITNOCV_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CEditNoChevron window

class CEditNoChevron : public CEdit
{
// Construction
public:
    CEditNoChevron();

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CEditNoChevron)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CEditNoChevron();

    // Generated message map functions
protected:
    //{{AFX_MSG(CEditNoChevron)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg LRESULT OnPasteMessage(WPARAM, LPARAM);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

class CEditNoChevronWx : public wxTextCtrl
{
// Construction
public:
    using wxTextCtrl::wxTextCtrl;

// Attributes
public:

// Operations
public:

// Overrides

// Implementation
public:

    // Generated message map functions
protected:
    void OnChar(wxKeyEvent& event);
    void OnPasteMessage(wxClipboardTextEvent& event);

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(CEditNoChevronWx);
    wxDECLARE_DYNAMIC_CLASS(CEditNoChevronWx);
};

/////////////////////////////////////////////////////////////////////////////

#endif
