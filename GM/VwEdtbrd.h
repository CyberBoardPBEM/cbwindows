// VwEdtbrd.h : interface of the CBrdEditView class
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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

#ifndef     _TILE_H
#include    "Tile.h"
#endif

#ifndef     _TOOLOBJS_H
#include    "ToolObjs.h"
#endif

class CBrdEditView : public CScrollView,
                        public CB::wxNativeContainerWindowMixin
{
protected: // create from serialization only
    CBrdEditView();
    DECLARE_DYNAMIC(CBrdEditView)


    CB::propagate_const<CBoard*> m_pBoard;           // Pointer to document's board
    CB::propagate_const<CBoardManager*> m_pBMgr;

    // --------------- //
    BOOL        m_bOffScreen;
    TileScale   m_nZoom;
    // --------------- //
    UINT        m_nCurToolID;   // Current tool ID
    UINT        m_nLastToolID;  // Previous tool ID

// Attributes
public:
    const CGamDoc& GetDocument() const;
    CGamDoc& GetDocument()
    {
        return const_cast<CGamDoc&>(std::as_const(*this).GetDocument());
    }
    TileScale GetCurrentScale() const { return m_nZoom; }
    const CBoard& GetBoard() const { return CheckedDeref(m_pBoard); }

    // Current Colors etc...
    COLORREF GetForeColor() const { return m_pBMgr->GetForeColor(); }
    COLORREF GetBackColor() const { return m_pBMgr->GetBackColor(); }
    UINT GetLineWidth() const { return value_preserving_cast<UINT>(m_pBMgr->GetLineWidth()); }
    void SetForeColor(COLORREF crFore) { m_pBMgr->SetForeColor(crFore); }
    void SetBackColor(COLORREF crBack) { m_pBMgr->SetBackColor(crBack); }

// Operations
public:
    void SetCellTile(TileID tid, CPoint pnt, BOOL bUpdate);
    void SetCellColor(COLORREF crCell, CPoint pnt, BOOL bUpdate);
    void SetBoardBackColor(COLORREF cr, BOOL bUpdate);
    void SetDrawingTile(CDrawList& pDwg, TileID tid, CPoint pnt, BOOL bUpdate);
    void DoCreateTextDrawingObject(CPoint point);
    void DoEditTextDrawingObject(CText& pDObj);
    void RestoreLastTool() { m_nCurToolID = m_nLastToolID; }
    void ResetToDefaultTool();

    void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) override;
    void PrepareScaledDC(CDC& pDC) const;

// Tools and Selection support
private:
    CSelList    m_selList;                      // List of selected objects.
public:

    const CSelList& GetSelectList() const { return m_selList; }
    CSelList& GetSelectList()
    {
        return const_cast<CSelList&>(std::as_const(*this).GetSelectList());
    }

    void ClientToWorkspace(CPoint& point) const;
    void ClientToWorkspace(CRect& rect) const;
    void WorkspaceToClient(CPoint& point) const;
    void WorkspaceToClient(CRect& rect) const;
    void InvalidateWorkspaceRect(const CRect& pRect, BOOL bErase = FALSE);
    CPoint GetWorkspaceDim() const;
    void OnPrepareScaledDC(CDC& pDC);

    void AdjustPoint(CPoint& pnt) const;              // Limit and grid processing
    void AdjustRect(CRect& rct) const;

    CDrawObj* ObjectHitTest(CPoint point);
    void AddDrawObject(CDrawObj::OwnerPtr pObj);         // Add to active layer
    void DeleteDrawObject(CDrawObj::OwnerPtr pObj);      // Delete from active layer
    void SelectWithinRect(CRect rctNet, BOOL bInclIntersects = FALSE);
    void SelectAllUnderPoint(CPoint point);

    // --- Misc Draw object manipulations
    void DeleteObjsInSelectList(BOOL bInvalidate = TRUE);
    void MoveObjsInSelectList(BOOL bToFront, BOOL bInvalidate = TRUE);
    void NudgeObjsInSelectList(int dX, int dY, BOOL forceScroll = FALSE); //DFM19991014
    CDrawList* GetDrawList(BOOL bCanCreateList = TRUE);

// Implementation
protected:
    // Grid and limiting support
    BOOL IsGridizeActive() const;
    void GridizeX(long& xPos) const;
    void GridizeY(long& yPos) const;
    void LimitPoint(POINT& pPnt) const;
    void LimitRect(RECT& pRct) const;
    void PixelToWorkspace(CPoint& point) const;

    void CreateTextDrawingObject(CPoint pnt, FontID fid, COLORREF crText,
        const CB::string& m_strText, BOOL bInvalidate = TRUE);

    void DoViewScale(TileScale nZoom);
    void CenterViewOnWorkspacePoint(CPoint point);

    BOOL DoMouseWheelFix(UINT fFlags, short zDelta, CPoint point);

public:
    ~CBrdEditView() override;
    void OnDraw(CDC* pDC) override;      // overridden to draw this view

#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif
    BOOL PreCreateWindow(CREATESTRUCT& cs) override;
    void OnInitialUpdate() override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

protected:
    // ------------ //
    ToolType MapToolType(UINT nToolResID) const;

    // Nudge and scroll functions
    void HandleKeyDown ();
    void HandleKeyUp ();
    void HandleKeyLeft ();
    void HandleKeyRight ();
    void HandleKeyPageUp ();
    void HandleKeyPageDown ();
    void HandleKeyTop ();
    void HandleKeyBottom ();

    void ScrollDown();
    void ScrollUp();
    void ScrollLeft();
    void ScrollRight();
    void NudgeScrollDown();
    void NudgeScrollUp();
    void NudgeScrollLeft();
    void NudgeScrollRight();
    void NudgeDown();
    void NudgeUp();
    void NudgeLeft();
    void NudgeRight();
    void NudgescrollLeft();
    void FastScrollDown();
    void FastScrollUp();
    void FastScrollLeft();
    void FastScrollRight();
    void FastNudgeScrollDown();
    void FastNudgeScrollUp();
    void FastNudgeScrollLeft();
    void FastNudgeScrollRight();
    void FastNudgeDown();
    void FastNudgeUp();
    void FastNudgeLeft();
    void FastNudgeRight();
    void FastNudgescrollLeft();
    void PageDown();
    void PageUp();
    void PageLeft();
    void PageRight();
    void PageTop();
    void PageBottom();
    void PageFarLeft();
    void PageFarRight();

    // Generated message map functions
protected:
    //{{AFX_MSG(CBrdEditView)
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnTimer(uintptr_t nIDEvent);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    void OnDragTileItem(DragDropEvent& event);
    afx_msg LRESULT OnSetColor(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetCustomColors(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetLineWidth(WPARAM wParam, LPARAM lParam);
    afx_msg void OnOffscreen();
    afx_msg void OnViewGridLines();
    afx_msg void OnDwgToBack();
    afx_msg void OnDwgToFront();
    afx_msg BOOL OnToolPalette(UINT id);
    afx_msg BOOL OnEditLayer(UINT id);
    afx_msg void OnUpdateViewGridLines(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOffscreen(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditLayer(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorForeground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorBackground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorCustom(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLineWidth(CCmdUI* pCmdUI);
    afx_msg void OnUpdateToolPalette(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDwgToFrontOrBack(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewFullScale(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewHalfScale(CCmdUI* pCmdUI);
    afx_msg void OnDwgFont();
    afx_msg void OnViewFullScale();
    afx_msg void OnViewHalfScale();
    afx_msg void OnViewSmallScale();
    afx_msg void OnUpdateViewSmallScale(CCmdUI* pCmdUI);
    afx_msg void OnUpdateIndicatorCellNum(CCmdUI* pCmdUI);
    afx_msg void OnToolsBrdSnapGrid();
    afx_msg void OnUpdateToolsBrdSnapGrid(CCmdUI* pCmdUI);
    afx_msg void OnToolsBrdProps();
    afx_msg void OnToolSetVisibleScale();
    afx_msg void OnUpdateToolSetVisibleScale(CCmdUI* pCmdUI);
    afx_msg void OnToolSuspendScaleVisibility();
    afx_msg void OnUpdateToolSuspendScaleVsibility(CCmdUI* pCmdUI);
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
    afx_msg void OnEditPasteBitmapFromFile();
    afx_msg void OnUpdateEditPasteBitmapFromFile(CCmdUI* pCmdUI);
    afx_msg void OnEditClear();
    afx_msg void OnUpdateEditClear(CCmdUI* pCmdUI);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnDwgDrawAboveGrid();
    afx_msg void OnUpdateDwgDrawAboveGrid(CCmdUI* pCmdUI);
    afx_msg void OnViewToggleScale();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    friend class CBrdEditViewContainer;
};

#ifndef _DEBUG  // debug version in gmview.cpp
inline CGamDoc& CBrdEditView::GetDocument() const
   { return *static_cast<CGamDoc*>(m_pDocument); }
#endif

class CBrdEditViewContainer : public CView
{
public:
    const CBrdEditView& GetChild() const { return *child; }
    CBrdEditView& GetChild()
    {
        return const_cast<CBrdEditView&>(std::as_const(*this).GetChild());
    }
    void OnDraw(CDC* pDC) override;

protected:
    void OnActivateView(BOOL bActivate,
                        CView *pActivateView,
                        CView* pDeactivateView) override;

private:
    CBrdEditViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CBrdEditViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

    // owned by MFC
    RefPtr<CBrdEditView> child;
};
