// FrmProg.h
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

#ifndef _FRMPROJ_H
#define _FRMPROJ_H

/////////////////////////////////////////////////////////////////////////////
// CProjFrame frame

class CProjFrame : public CMDIChildWndEx
{
    DECLARE_DYNCREATE(CProjFrame)
protected:
    CProjFrame();           // protected constructor used by dynamic creation
    BOOL PreCreateWindow(CREATESTRUCT& cs);

// Attributes
public:

// Operations
public:

// Implementation
protected:
    virtual ~CProjFrame();

    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

    // Generated message map functions
    //{{AFX_MSG(CProjFrame)
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnClose();
};

#endif

