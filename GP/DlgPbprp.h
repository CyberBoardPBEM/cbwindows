// DlgPbprp.h - Playing board properties
//
// Copyright (c) 1994-2020 By Dale L. Larson, All Rights Reserved.
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

class CPBrdPropDialog : public CDialog
{
// Construction
public:
    CPBrdPropDialog(CWnd* pParent = NULL);  // standard constructor

// Dialog Data
    //{{AFX_DATA(CPBrdPropDialog)
    enum { IDD = IDD_PBRDPROP };
    CButton m_chkAllowAccess;
    CComboBox   m_comboOwners;
    CStatic m_staticOwnerLabel;
    CMFCColorButton  m_cpPlotColor;
    CComboBox   m_comboPlotWd;
    BOOL        m_bGridSnap;
    BOOL        m_bSmallCellBorders;
    BOOL        m_bCellBorders;
    int         m_xStackStagger;
    int         m_yStackStagger;
    CString     m_strBoardName;
    BOOL        m_bGridRectCenters;
    BOOL        m_bSnapMovePlot;
    float       m_fXGridSnapOff;
    float       m_fYGridSnapOff;
    float       m_fXGridSnap;
    float       m_fYGridSnap;
    BOOL        m_bOpenBoardOnLoad;
    BOOL        m_bShowSelListAndTinyMap;
    BOOL        m_bDrawLockedBeneath;
    //}}AFX_DATA

    COLORREF    m_crPlotColor;
    UINT        m_nPlotWd;

    UINT        m_xGridSnapOff;
    UINT        m_yGridSnapOff;
    UINT        m_xGridSnap;
    UINT        m_yGridSnap;

    BOOL            m_bOwnerInfoIsReadOnly;
    BOOL            m_bNonOwnerAccess;
    int             m_nOwnerSel;    // -1 = no owner, 0 = first player, ....
    CPlayerManager* m_pPlayerMgr;

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CPBrdPropDialog)
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedDPbprpShowSelAndTiny2();
};


