// DlbMkbrd.h
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

class CGridType : public wxDialog
{
// Construction
public:
    CGridType(wxWindow* parent = &CB::GetMainWndWx());    // standard constructor

// Dialog Data
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxStaticText> m_staticPixelSize;
        RefPtr<wxCheckBox> m_chkStagger;
        RefPtr<wxTextCtrl> m_editRows;
        RefPtr<wxTextCtrl> m_editCols;
        RefPtr<wxTextCtrl> m_editCellWd;
        RefPtr<wxTextCtrl> m_editCellHt;
        RefPtr<wxRadioButton> m_radioRect;
        RefPtr<wxTextCtrl> m_editBoardName;
    CB_XRC_END_CTRLS_DECL()
    int m_nBoardTypeHelper;
public:
    int     m_iCellWd;
    int     m_iCellHt;
    size_t  m_iCols;
    size_t  m_iRows;
    wxString m_strBoardName;
    CellStagger m_bStagger;
    CellFormType m_nBoardType;

// Implementation
protected:
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override
    {
        return TransferDataFromWindowRangeCheck(true);
    }

    void UpdateBoardDimensions();

    void OnHBrick(wxCommandEvent& event);
    void OnVBrick(wxCommandEvent& event);
    void OnRectCell(wxCommandEvent& event);
    void OnHexFlat(wxCommandEvent& event);
    void OnHexPnt(wxCommandEvent& event);
    void OnChangeDNewbrdCellheight(wxCommandEvent& event);
    void OnChangeDNewbrdCellwidth(wxCommandEvent& event);
    void OnChangeDNewbrdGridcols(wxCommandEvent& event);
    void OnChangeDNewbrdGridrows(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();

private:
    class DisableRangeCheck;
    bool TransferDataFromWindowRangeCheck(bool check);
};
