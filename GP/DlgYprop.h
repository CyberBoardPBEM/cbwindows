// DlgYprop.h : header file
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

class CTrayManager;
class CPlayerManager;

/////////////////////////////////////////////////////////////////////////////
// CTrayPropDialog dialog

class CTrayPropDialog : public wxDialog
{
// Construction
public:
    CTrayPropDialog(const CTrayManager& yMgr,
                    const CPlayerManager* playerMgr,
                    wxWindow* parent = &CB::GetMainWndWx());  // standard constructor

    wxString m_strName;
private:
    int     m_nVizOpts;
public:
    bool    m_bRandomSel;
    bool    m_bRandomSide;

    size_t          m_nYSel;
    PlayerId        m_nOwnerSel;    // -1 = no owner, 0 = first player, ....
    bool            m_bNonOwnerAccess;
    bool            m_bEnforceVizForOwnerToo;
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxCheckBox> m_chkVizOwnerToo;
        RefPtr<wxCheckBox> m_chkAllowAccess;
        RefPtr<wxStaticText> m_staticOwnerLabel;
        RefPtr<wxChoice> m_comboOwners;
        RefPtr<wxTextCtrl> m_editName;
        RefPtr<wxRadioButton> m_radioVizOpts;
        RefPtr<wxCheckBox> m_checkRandomSel;
        RefPtr<wxCheckBox> m_checkRandomSide;
    CB_XRC_END_CTRLS_DECL()

    const CTrayManager&   m_pYMgr;
    const CPlayerManager* const m_pPlayerMgr;

public:
    void SetTrayViz(TrayViz eTrayViz) { m_nVizOpts = (int)eTrayViz; }
    TrayViz GetTrayViz() { return (TrayViz)m_nVizOpts; }

// Implementation
protected:
    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;
    void OnSelChangeOwnerList(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();
};
