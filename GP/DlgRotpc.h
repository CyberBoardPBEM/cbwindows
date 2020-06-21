// DlgRotpc.h : header file
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

#ifndef _DLGROTPC_H_
#define _DLGROTPC_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CRotatePieceDialog dialog

class CRotatePieceDialog : public CDialog
{
// Construction
public:
    CRotatePieceDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CRotatePieceDialog)
    enum { IDD = IDD_ROTATEPIECE };
    CEdit   m_editCurVal;
    //}}AFX_DATA

    int m_nRelativeRotation;

// Overrides
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

// Implementation
protected:
    void ApplyOffset(int nOffset);
    void UpdateRotationValue();

    // Generated message map functions
    //{{AFX_MSG(CRotatePieceDialog)
    afx_msg void OnRotatePieceCCW1();
    afx_msg void OnRotatePieceCCW5();
    afx_msg void OnRotatePieceCCW10();
    afx_msg void OnRotatePieceCCW50();
    afx_msg void OnRotatePieceCW1();
    afx_msg void OnRotatePieceCW5();
    afx_msg void OnRotatePieceCW10();
    afx_msg void OnRotatePieceCW50();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif
