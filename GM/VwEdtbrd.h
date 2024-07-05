// VwEdtbrd.h : interface of the CBrdEditView class
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
    DECLARE_DYNCREATE(CBrdEditView)

    void GetCellDrawBounds(CRect &oRct, CRect &oLim);
    void FillMapCells(CDC* pDC, CRect &oLim);
    void FrameMapCells(CDC* pDC, CRect &oLim);

    CBoard* m_pBoard;           // Pointer to document's board
    CBoardManager* m_pBMgr;

    // --------------- //
    BOOL        m_bOffScreen;
    TileScale   m_nZoom;
    // --------------- //
    UINT        m_nCurToolID;   // Current tool ID
    UINT        m_nLastToolID;  // Previous tool ID

// Attributes
public:
    CGamDoc* GetDocument();
    TileScale GetCurrentScale() { return m_nZoom; }
    CBoard* GetBoard() { return m_pBoard; }

    // Current Colors etc...
    COLORREF GetForeColor() { return m_pBMgr->GetForeColor(); }
    COLORREF GetBackColor() { return m_pBMgr->GetBackColor(); }
    UINT GetLineWidth() { return m_pBMgr->GetLineWidth(); }
    void SetForeColor(COLORREF crFore) { m_pBMgr->SetForeColor(crFore); }
    void SetBackColor(COLORREF crBack) { m_pBMgr->SetBackColor(crBack); }

// Operations
public:
    void SetCellTile(TileID tid, CPoint pnt, BOOL bUpdate);
    void SetCellColor(COLORREF crCell, CPoint pnt, BOOL bUpdate);
    void SetBoardBackColor(COLORREF cr, BOOL bUpdate);
    void SetDrawingTile(CDrawList* pDwg, TileID tid, CPoint pnt, BOOL bUpdate);
    void DoCreateTextDrawingObject(CPoint point);
    void DoEditTextDrawingObject(CText* pDObj);
    void RestoreLastTool() { m_nCurToolID = m_nLastToolID; }
    void ResetToDefaultTool();

    virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo);
    void PrepareScaledDC(CDC *pDC);

// Tools and Selection support
public:
    CSelList    m_selList;                      // List of selected objects.

    CSelList* GetSelectList() { return &m_selList; }

    void ClientToWorkspace(CPoint& point) const;
    void ClientToWorkspace(CRect& rect) const;
    void WorkspaceToClient(CPoint& point) const;
    void WorkspaceToClient(CRect& rect) const;
    void InvalidateWorkspaceRect(const CRect* pRect, BOOL bErase = FALSE);
    CPoint GetWorkspaceDim();
    void OnPrepareScaledDC(CDC *pDC);

    void AdjustPoint(CPoint& pnt);              // Limit and grid processing
    void AdjustRect(CRect& rct);

    CDrawObj* ObjectHitTest(CPoint point);
    void AddDrawObject(CDrawObj::OwnerPtr pObj);         // Add to active layer
    void DeleteDrawObject(CDrawObj* pObj);      // Delete from active layer
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
    BOOL IsGridizeActive();
    void GridizeX(long& xPos);
    void GridizeY(long& yPos);
    void LimitPoint(POINT* pPnt);
    void LimitRect(RECT* pRct);
    void PixelToWorkspace(CPoint& point);

    void CreateTextDrawingObject(CPoint pnt, FontID fid, COLORREF crText,
        const CB::string& m_strText, BOOL bInvalidate = TRUE);

    void DoViewScale(TileScale nZoom);
    void CenterViewOnWorkspacePoint(CPoint point);

    BOOL DoMouseWheelFix(UINT fFlags, short zDelta, CPoint point);

public:
    virtual ~CBrdEditView();
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

protected:
    // ------------ //
    ToolType MapToolType(UINT nToolResID);

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
};

#ifndef _DEBUG  // debug version in gmview.cpp
inline CGamDoc* CBrdEditView::GetDocument()
   { return (CGamDoc*) m_pDocument; }
#endif

