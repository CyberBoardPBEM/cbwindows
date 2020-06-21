// DlgRot.h : header file
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
// CRotateDialog dialog

class CDib;

class CRotateDialog : public CDialog
{
// Construction
public:
    CRotateDialog(CWnd* pParent = NULL);    // standard constructor
    virtual ~CRotateDialog();

// Dialog Data
    //{{AFX_DATA(CRotateDialog)
    enum { IDD = IDD_ROTATE };
    CStatic m_ctlView;
    UINT    m_nAngle;
    //}}AFX_DATA

    CBitmap*    m_pBMap;        // From caller
    COLORREF    m_crTrans;

    // Generated
    CDib*       m_pDib;         //??
    CBitmap*    m_pRotBMap;     //??

    CBitmap*    m_bmapTbl[12];  //!!testing

    void DeleteBMaps();

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CRotateDialog)
    afx_msg void OnRotApply();
    afx_msg void OnPaint();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
