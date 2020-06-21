// DlgNtile.h : header file
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
// CNewTileDialog dialog

class CBoardManager;

class CNewTileDialog : public CDialog
{
// Construction
public:
    CNewTileDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CNewTileDialog)
    enum { IDD = IDD_NEWTILE };
    CEdit   m_editWidth;
    CEdit   m_editHeight;
    CComboBox   m_comboBoard;
    UINT    m_nHeight;
    UINT    m_nWidth;
    //}}AFX_DATA

    UINT    m_nHalfHeight;
    UINT    m_nHalfWidth;

    CBoardManager* m_pBMgr;

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CNewTileDialog)
    virtual BOOL OnInitDialog();
    afx_msg void OnSelChangeBoardName();
    afx_msg void OnUpdateHeight();
    afx_msg void OnUpdateWidth();
    virtual void OnOK();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
