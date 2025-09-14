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

class CTrayPropDialog : public CDialog
{
// Construction
public:
    CTrayPropDialog(const CTrayManager& yMgr,
                    const CPlayerManager* playerMgr,
                    CWnd* pParent = NULL);  // standard constructor

// Dialog Data
    //{{AFX_DATA(CTrayPropDialog)
private:
    enum { IDD = IDD_TRAYPRP };
    CButton m_chkVizOwnerToo;
    CButton m_chkAllowAccess;
    CStatic m_staticOwnerLabel;
    CComboBox   m_comboOwners;
    CEdit   m_editName;
public:
    CB::string m_strName;
private:
    int     m_nVizOpts;
public:
    BOOL    m_bRandomSel;
    BOOL    m_bRandomSide;
    //}}AFX_DATA

    size_t          m_nYSel;
    PlayerId        m_nOwnerSel;    // -1 = no owner, 0 = first player, ....
    BOOL            m_bNonOwnerAccess;
    BOOL            m_bEnforceVizForOwnerToo;

private:
    const CTrayManager&   m_pYMgr;
    const CPlayerManager* const m_pPlayerMgr;

public:
    void SetTrayViz(TrayViz eTrayViz) { m_nVizOpts = (int)eTrayViz; }
    TrayViz GetTrayViz() { return (TrayViz)m_nVizOpts; }

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CTrayPropDialog)
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnSelChangeOwnerList();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
