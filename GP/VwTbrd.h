// VwTbrd.h : header file
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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
public:
    CTinyBoardView();

// Attributes
private:
    CGamDoc& GetDocument();

// Operations
public:

// Implementation
protected:
    CB::propagate_const<CPlayBoard*> m_pPBoard; // The playing board we are viewing
    OwnerOrNullPtr<CBitmap> m_pBMap;            // Cached predrawn board bitmap

    TileScale   m_nZoom;

    void RegenCachedMap(CDC& pDC);
    OwnerPtr<CBitmap> DrawFullMap(CDC& pDC);

// Implementation
protected:
    void SetupDrawListDC(CDC& pDC, CRect& rct) const;
    void RestoreDrawListDC(CDC &pDC) const;
    // -------- //
    void WorkspaceToClient(CRect& rect) const;
    void ClientToWorkspace(CPoint& pnt) const;
    void InvalidateWorkspaceRect(const CRect& pRect, BOOL bErase = FALSE);

// Implementation
protected:
    ~CTinyBoardView() override = default;
    BOOL PreCreateWindow(CREATESTRUCT& cs) override;
    void OnInitialUpdate() override;     // first time after construct
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
    void OnDraw(CDC* pDC) override;      // overridden to draw this view

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
inline CGamDoc& CTinyBoardView::GetDocument()
   { return CheckedDeref(CB::ToCGamDoc(m_pDocument)); }
#endif

class CTinyBoardViewContainer : public CView
{
public:
    void OnDraw(CDC* pDC) override;

private:
    CTinyBoardViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CTinyBoardViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    // owned by MFC
    RefPtr<CTinyBoardView> child;
};


