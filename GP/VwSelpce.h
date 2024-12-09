// VwSelpce.h : header file
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

#ifndef     _LBOXSLCT_H
#include    "LBoxSlct.h"
#endif

class CPlayBoard;

/////////////////////////////////////////////////////////////////////////////
// CSelectedPieceView view

class CSelectedPieceView : public CView
{
    DECLARE_DYNCREATE(CSelectedPieceView)
protected:
    CSelectedPieceView();           // protected constructor used by dynamic creation

// Attributes
public:
    CGamDoc* GetDocument();

// Operations
public:

// Implementation
protected:
    CPlayBoard*     m_pPBoard;      // Board that contains selections

    CSelectListBox  m_listSel;
    std::vector<CB::not_null<CDrawObj*>> m_tblSel;
    CToolTipCtrl    m_toolTip;

// Implementation
protected:
    void ModifySelectionsBasedOnListItems(BOOL bRemoveSelectedItems);

    virtual ~CSelectedPieceView();
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
    virtual void OnInitialUpdate();
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    virtual BOOL PreTranslateMessage(MSG* pMsg);

// Generated message map functions
protected:
    //{{AFX_MSG(CSelectedPieceView)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    //}}AFX_MSG
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in vwselpce.cpp
inline CGamDoc* CSelectedPieceView::GetDocument()
   { return CB::ToCGamDoc(m_pDocument); }
#endif

