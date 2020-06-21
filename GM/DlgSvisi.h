// DlgSvisi.h : header file
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
// CSetScaleVisibilityDialog dialog

class CSetScaleVisibilityDialog : public CDialog
{
// Construction
public:
    CSetScaleVisibilityDialog(CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    //{{AFX_DATA(CSetScaleVisibilityDialog)
    enum { IDD = IDD_SETSCALEVISIBILITY };
    CButton m_chkNaturalScale;
    CButton m_chkSmallScale;
    CButton m_chkHalfScale;
    CButton m_chkFullScale;
    BOOL    m_bFullScale;
    BOOL    m_bHalfScale;
    BOOL    m_bSmallScale;
    BOOL    m_bNaturalScale;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CSetScaleVisibilityDialog)
    virtual void OnOK();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    virtual BOOL OnInitDialog();
    afx_msg void OnClickHalf();
    afx_msg void OnClickFull();
    afx_msg void OnClickSmall();
    afx_msg void OnClickNatural();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
