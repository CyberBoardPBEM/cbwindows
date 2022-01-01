// DlbMkbrd.h
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

class CGridType : public CDialog
{
// Construction
public:
    CGridType(CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    //{{AFX_DATA(CGridType)
    enum { IDD = IDD_BRDNEW };
    CStatic m_staticPixelSize;
    CButton m_chkStagger;
    CEdit   m_editRows;
    CEdit   m_editCols;
    CEdit   m_editCellWd;
    CEdit   m_editCellHt;
    int     m_iCellWd;
    int     m_iCellHt;
    size_t  m_iCols;
    size_t  m_iRows;
    CString m_strBoardName;
    CellStagger m_bStagger;
    int     m_nBoardType;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    void UpdateBoardDimensions();

    // Generated message map functions
    //{{AFX_MSG(CGridType)
    virtual void OnOK();
    afx_msg void OnHBrick();
    afx_msg void OnVBrick();
    afx_msg void OnRectCell();
    afx_msg void OnHexFlat();
    afx_msg void OnHexPnt();
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeDNewbrdCellheight();
    afx_msg void OnChangeDNewbrdCellwidth();
    afx_msg void OnChangeDNewbrdGridcols();
    afx_msg void OnChangeDNewbrdGridrows();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
