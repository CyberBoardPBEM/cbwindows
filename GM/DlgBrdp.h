// dlgbrdp.h : header file
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
// CBoardPropDialog dialog

enum CellFormType;

class CBoardPropDialog : public wxDialog
{
// Construction
public:
    CBoardPropDialog(wxWindow* parent = &CB::GetMainWndWx()); // standard constructor

// Dialog Data
    bool    m_bCellLines;
    bool    m_bGridSnap;
    bool    m_bTrackCellNum;
    int32_t m_nRowTrkOffset;
    int32_t m_nColTrkOffset;
private:
    float   m_fXGridSnapOff;
    float   m_fYGridSnapOff;
    float   m_fXGridSnap;
    float   m_fYGridSnap;
public:
    wxString m_strName;
    int     m_nStyleNum;
    bool    m_bColTrkInvert;
    bool    m_bRowTrkInvert;
    bool    m_bCellBorderOnTop;
    bool    m_bEnableXParentCells;

    wxColour m_crCellFrame;

    bool            m_bShapeChanged;// Set to true if reshape desired

    uint32_t        m_xGridSnap;
    uint32_t        m_yGridSnap;
    uint32_t        m_xGridSnapOff;
    uint32_t        m_yGridSnapOff;

    CellFormType    m_eCellStyle;   // Used to disable cell a cell dimension
    size_t          m_nRows;
    size_t          m_nCols;
    uint32_t        m_nCellHt;
    uint32_t        m_nCellWd;
    CellStagger     m_bStagger;
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxStaticText> m_staticHalfPixelWidth;
        RefPtr<wxStaticText> m_staticSmallPixelWidth;
        RefPtr<wxStaticText> m_staticSmallPixelHeight;
        RefPtr<wxStaticText> m_staticHalfPixelHeight;
        RefPtr<wxStaticText> m_staticPixelWidth;
        RefPtr<wxStaticText> m_staticPixelHeight;
        RefPtr<wxTextCtrl> m_editBrdName;
        RefPtr<wxColourPickerCtrl> m_cpCellFrame;
        RefPtr<wxStaticText> m_staticWidth;
        RefPtr<wxStaticText> m_staticHeight;
        RefPtr<wxStaticText> m_staticRows;
        RefPtr<wxStaticText> m_staticCols;
        RefPtr<wxChoice> m_comboStyle;
        RefPtr<wxCheckBox> m_chkCellLines;
        RefPtr<wxCheckBox> m_chkGridSnap;
        RefPtr<wxCheckBox> m_chkTrackCellNum;
        RefPtr<wxTextCtrl> m_editRowTrkOffset;
        RefPtr<wxTextCtrl> m_editColTrkOffset;
        RefPtr<wxTextCtrl> m_editXGridSnapOff;
        RefPtr<wxTextCtrl> m_editYGridSnapOff;
        RefPtr<wxTextCtrl> m_editXGridSnap;
        RefPtr<wxTextCtrl> m_editYGridSnap;
        RefPtr<wxCheckBox> m_chkColTrkInvert;
        RefPtr<wxCheckBox> m_chkRowTrkInvert;
        RefPtr<wxCheckBox> m_chkCellBorderOnTop;
        RefPtr<wxCheckBox> m_chkEnableXParentCells;
    CB_XRC_END_CTRLS_DECL()
public:

// Implementation
protected:
    void UpdateInfoArea();

// Implementation
protected:
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    void OnReshape(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();
};

