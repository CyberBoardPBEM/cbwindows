// VwTbrd.h : header file
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
// CTinyBoardView view

class CTinyBoardView : public CScrollView
{
    DECLARE_DYNCREATE(CTinyBoardView)
protected:
    CTinyBoardView();           // protected constructor used by dynamic creation

// Attributes
public:
    CGamDoc* GetDocument();

// Operations
public:

// Implementation
protected:
    CPlayBoard* m_pPBoard;          // The playing board we are viewing
    OwnerOrNullPtr<CBitmap> m_pBMap;            // Cached predrawn board bitmap

    TileScale   m_nZoom;

    void RegenCachedMap(CDC* pDC);
    void DrawFullMap(CDC* pDC, CBitmap& bmap);

// Implementation
protected:
    void SetupDrawListDC(CDC* pDC, CRect& rct);
    void RestoreDrawListDC(CDC *pDC);
    // -------- //
    void WorkspaceToClient(CRect& rect);
    void ClientToWorkspace(CPoint& pnt);
    void InvalidateWorkspaceRect(const CRect* pRect, BOOL bErase = FALSE);

// Implementation
protected:
    virtual ~CTinyBoardView() = default;
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();     // first time after construct
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view

    // Generated message map functions
    //{{AFX_MSG(CTinyBoardView)
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    //}}AFX_MSG
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in vwmbrd.cpp
inline CGamDoc* CTinyBoardView::GetDocument()
   { return CB::ToCGamDoc(m_pDocument); }
#endif


