// DlgPbprp.h - Playing board properties
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

/////////////////////////////////////////////////////////////////////////////
// CPBrdPropDialog dialog

class CPlayerManager;

class CPBrdPropDialog : public wxDialog
{
// Construction
public:
    CPBrdPropDialog(wxWindow* pParent = &CB::GetMainWndWx());  // standard constructor

// Dialog Data
    bool        m_bGridSnap;
    bool        m_bSmallCellBorders;
    bool        m_bCellBorders;
    int         m_xStackStagger;
    int         m_yStackStagger;
    wxString    m_strBoardName;
    bool        m_bGridRectCenters;
    bool        m_bSnapMovePlot;
    bool        m_bOpenBoardOnLoad;
    bool        m_bShowSelListAndTinyMap;
    bool        m_bDrawLockedBeneath;

    wxColour    m_crPlotColor;
    UINT        m_nPlotWd;

    uint32_t    m_xGridSnapOff;
    uint32_t    m_yGridSnapOff;
    uint32_t    m_xGridSnap;
    uint32_t    m_yGridSnap;

    BOOL            m_bOwnerInfoIsReadOnly;
    BOOL            m_bNonOwnerAccess;
    BOOL            m_bPrivate;
    PlayerId        m_nOwnerSel;    // -1 = no owner, 0 = first player, ....
    CPlayerManager* m_pPlayerMgr;
private:
    float       m_fXGridSnapOff;
    float       m_fYGridSnapOff;
    float       m_fXGridSnap;
    float       m_fYGridSnap;
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxCheckBox> m_chkAllowAccess;
        RefPtr<wxCheckBox> m_chkPrivate;
        RefPtr<wxChoice> m_comboOwners;
        RefPtr<wxStaticText> m_staticOwnerLabel;
        RefPtr<wxColourPickerCtrl> m_cpPlotColor;
        RefPtr<wxChoice> m_comboPlotWd;
        RefPtr<wxCheckBox> m_chkGridSnap;
        RefPtr<wxCheckBox> m_chkSmallCellBorders;
        RefPtr<wxCheckBox> m_chkCellBorders;
        RefPtr<wxTextCtrl> m_editXStackStagger;
        RefPtr<wxTextCtrl> m_editYStackStagger;
        RefPtr<wxTextCtrl> m_editBoardName;
        RefPtr<wxCheckBox> m_chkGridRectCenters;
        RefPtr<wxCheckBox> m_chkSnapMovePlot;
        RefPtr<wxTextCtrl> m_editXGridSnapOff;
        RefPtr<wxTextCtrl> m_editYGridSnapOff;
        RefPtr<wxTextCtrl> m_editXGridSnap;
        RefPtr<wxTextCtrl> m_editYGridSnap;
        RefPtr<wxCheckBox> m_chkOpenBoardOnLoad;
        RefPtr<wxCheckBox> m_chkShowSelListAndTinyMap;
        RefPtr<wxCheckBox> m_chkDrawLockedBeneath;
    CB_XRC_END_CTRLS_DECL()

// Implementation
protected:
    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();
};


