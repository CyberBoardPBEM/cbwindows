// DlgGboxp.h : header file
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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

/////////////////////////////////////////////////////////////////////////////
// CGmBoxPropsDialog dialog

class CGmBoxPropsDialog : public wxDialog
{
// Construction
public:
    CGmBoxPropsDialog(wxWindow* parent = &CB::GetMainWndWx());    // standard constructor

// Dialog Data
    wxString m_strAuthor;
    wxString m_strDescr;
    wxString m_strTitle;

    bool m_bPropEdit = true;        // Set to true is editing existing props
    int m_nCompressLevel = wxZ_BEST_SPEED;

    bool m_bPassSet = false;
    wxString m_strPassword;

// Implementation
private:
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    void OnSetPassword(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif

    wxDECLARE_EVENT_TABLE();

    RefPtr<wxObject> m_dummy;
    RefPtr<wxChoice> m_comboCompress;
    RefPtr<wxTextCtrl> m_editAuthor;
    RefPtr<wxTextCtrl> m_editTitle;
    RefPtr<wxTextCtrl> m_editDescr;

    // adapt between m_nCompressLevel and m_comboCompress
    int m_nCompressLevelIndex;
};
