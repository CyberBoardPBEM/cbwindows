// DlgRotpc.h : header file
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

#ifndef _DLGROTPC_H_
#define _DLGROTPC_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CRotatePieceDialog dialog

class CRotatePieceDialog : public wxDialog
{
// Construction
public:
    /* TEMPORARY:  this needs to send msgs to the view, but that
        hasn't been converted to wx yet, so can't be passed as
        parent */
    CRotatePieceDialog(CWnd& v, wxWindow* pParent = &CB::GetMainWndWx());   // standard constructor

// Dialog Data
    int m_nRelativeRotation;
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxTextCtrl> m_editCurVal;
    CB_XRC_END_CTRLS_DECL()

// Overrides
protected:

// Implementation
protected:
    void ApplyOffset(int nOffset);

    void OnRotatePieceCCW1(wxCommandEvent& event);
    void OnRotatePieceCCW5(wxCommandEvent& event);
    void OnRotatePieceCCW10(wxCommandEvent& event);
    void OnRotatePieceCCW50(wxCommandEvent& event);
    void OnRotatePieceCW1(wxCommandEvent& event);
    void OnRotatePieceCW5(wxCommandEvent& event);
    void OnRotatePieceCW10(wxCommandEvent& event);
    void OnRotatePieceCW50(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();

private:
    RefPtr<CWnd> view;
};

#endif
