// DlgPEditMulti.h : header file
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

#ifndef _DLGPEDITMULTI_H_
#define _DLGPEDITMULTI_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPieceEditMultipleDialog dialog

class CPieceEditMultipleDialog : public wxDialog
{
// Construction
public:
    CPieceEditMultipleDialog(size_t sides, wxWindow* parent = &CB::GetMainWndWx());   // standard constructor

// Dialog Data
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxCheckBox> m_chkTopVisibleOwnersToo;
        RefPtr<wxStaticText> m_staticFrontLabel;
        RefPtr<wxCheckBox> m_chkSetBackText;
        RefPtr<wxCheckBox> m_chkSetFrontText;
        RefPtr<wxStaticText> m_staticBackLabel;
        RefPtr<wxCheckBox> m_chkTopVisible;
        RefPtr<wxTextCtrl> m_editBack;
        RefPtr<wxTextCtrl> m_editFront;
        /* sides are 0-based since they are vector indices, but we
            will display them 1-based for human readability */
            // m_currSide should contain 2 - sides
        RefPtr<wxChoice> m_currSide;
    CB_XRC_END_CTRLS_DECL()

public:
    BOOL    m_bSetTopOnlyVisible;               // TRUE if m_bTopOnlyVisible changed
    BOOL    m_bTopOnlyVisible;
    BOOL    m_bTopOnlyOwnersToo;

    std::vector<bool> m_bSetTexts;                      // TRUE if text should change
    std::vector<CB::string> m_strs;

// Overrides
    protected:

// Implementation
protected:
    void UpdateTextControls();

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
    void OnBtnClickChangeBack(wxCommandEvent& event);
    void OnBtnClickChangeFront(wxCommandEvent& event);
    void OnBtnClickTopVisible(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    void OnSelchangeCurrSide(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();

    const size_t m_sides;
    size_t m_prevSide = std::numeric_limits<size_t>::max();
};

#endif
