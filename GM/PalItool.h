// PalItool.h
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

#ifndef _PALITOOL_H
#define _PALITOOL_H

#ifndef __AFXEXT_H__
#include <afxext.h>         // for access to CToolBar
#endif

class CImageToolPalette : public CToolBar
{
// Constructor
public:
    CImageToolPalette();
    BOOL Create(CWnd* pOwnerWnd);

// Attributes
public:
    int GetNonClientPartsHeight() const { return m_bmapHeight + 2; }
    int GetNonClientPartsWidth() const { return 2; }

// Operations
public:
    void SetSizes(SIZE sizeButton, SIZE sizeImage, UINT nColumns);
    BOOL SetButtons(const UINT FAR* lpIDArray, int nIDCount, UINT nColumns);
    void SaveProfileSettings();

// Implementation
public:
    virtual ~CImageToolPalette();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    // Overridden from CToolBar implementation
    virtual void GetItemRect(int nIndex, LPRECT lpRect) const;
    virtual int HitTest(CPoint point);
    virtual void DoPaint(CDC* pDC);

    void RecalcLayout(UINT nButtonCount);

protected:
    CWnd*   m_pOwnerWnd;
    UINT    m_nColumns;
    UINT    m_cxRightBorder;

    int     m_xCurPos;
    int     m_yCurPos;

// Non Client support stuff
    UINT    m_bmapWidth;
    UINT    m_bmapHeight;
    CBitmap m_bmapSysButton;

// Window Overrides
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Generated message map functions
protected:
    //{{AFX_MSG(CImageToolPalette)
    afx_msg void OnDestroy();
    afx_msg UINT OnNcHitTest(CPoint point);
    afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnNcDestroy();
    afx_msg void OnNcPaint();
    afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif

