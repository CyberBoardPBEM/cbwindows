// dlgbrdsz.h : header file
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
// CBoardReshapeDialog dialog

enum CellFormType;

class CBoardReshapeDialog : public wxDialog
{
// Construction
public:
    CBoardReshapeDialog(wxWindow* parent = &CB::GetMainWndWx());  // standard constructor

// Dialog Data
    uint32_t     m_nCellHt;
    uint32_t     m_nCellWd;
    size_t  m_nCols;
    size_t  m_nRows;
    CellStagger m_bStagger;

    CellFormType m_eCellStyle;  // Used to disable cell a cell dimension
private:
    bool m_bStaggerAdapter;
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxCheckBox> m_checkStaggerIn;
        RefPtr<wxTextCtrl> m_editCellWd;
        RefPtr<wxTextCtrl> m_editCellHt;
        RefPtr<wxTextCtrl> m_editCols;
        RefPtr<wxTextCtrl> m_editRows;
    CB_XRC_END_CTRLS_DECL()
public:

// Implementation
protected:
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();
};
