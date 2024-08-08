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

class CBrdEditViewContainer;

class CBrdEditView : public wxScrolledCanvas
{
protected: // create from serialization only
    CBrdEditView(CBrdEditViewContainer& p);

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
    wxOverlay& GetOverlay() { return overlay; }
    const CGamDoc& GetDocument() const { return *document; }
    CGamDoc& GetDocument()
    {
        return const_cast<CGamDoc&>(std::as_const(*this).GetDocument());
    }
    TileScale GetCurrentScale() const { return m_nZoom; }
    const CBoard& GetBoard() const { return CheckedDeref(m_pBoard); }

    // Current Colors etc...
    wxColour GetForeColor() const { return CB::Convert(m_pBMgr->GetForeColor()); }
    wxColour GetBackColor() const { return CB::Convert(m_pBMgr->GetBackColor()); }
    UINT GetLineWidth() const { return value_preserving_cast<UINT>(m_pBMgr->GetLineWidth()); }
    void SetForeColor(wxColour crFore) { m_pBMgr->SetForeColor(CB::Convert(crFore)); }
    void SetBackColor(wxColour crBack) { m_pBMgr->SetBackColor(CB::Convert(crBack)); }

// Operations
public:
    void SetCellTile(TileID tid, wxPoint pnt, BOOL bUpdate);
    void SetCellColor(wxColour crCell, wxPoint pnt, BOOL bUpdate);
    void SetBoardBackColor(wxColour cr, BOOL bUpdate);
    void SetDrawingTile(CDrawList& pDwg, TileID tid, wxPoint pnt, BOOL bUpdate);
    void DoCreateTextDrawingObject(wxPoint point);
    void DoEditTextDrawingObject(CText& pDObj);
    void RestoreLastTool() { m_nCurToolID = m_nLastToolID; }
    void ResetToDefaultTool();

#if 0
    void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) override;
#endif
    void PrepareScaledDC(wxDC& pDC) const;

// Tools and Selection support
private:
    CSelList    m_selList;                      // List of selected objects.
public:

    const CSelList& GetSelectList() const { return m_selList; }
    CSelList& GetSelectList()
    {
        return const_cast<CSelList&>(std::as_const(*this).GetSelectList());
    }

    void ClientToWorkspace(wxPoint& point) const;
    void ClientToWorkspace(wxRect& rect) const;
    void WorkspaceToClient(wxPoint& point) const;
    [[nodiscard]] wxPoint WorkspaceToClient(const wxPoint& pnt) const
    {
        wxPoint retval = pnt;
        WorkspaceToClient(retval);
        return retval;
    }
    void WorkspaceToClient(wxRect& rect) const;
    void InvalidateWorkspaceRect(const wxRect& pRect, BOOL bErase = FALSE);
    wxPoint GetWorkspaceDim() const;
    void OnPrepareScaledDC(wxDC& pDC);

    void AdjustPoint(wxPoint& pnt) const;              // Limit and grid processing
    void AdjustRect(wxRect& rct) const;

    CDrawObj* ObjectHitTest(wxPoint point);
    void AddDrawObject(CDrawObj::OwnerPtr pObj);         // Add to active layer
    void DeleteDrawObject(CDrawObj::OwnerPtr pObj);      // Delete from active layer
    void SelectWithinRect(wxRect rctNet, BOOL bInclIntersects = FALSE);
    void SelectAllUnderPoint(wxPoint point);

#if 0
    // --- Misc Draw object manipulations
    void DeleteObjsInSelectList(BOOL bInvalidate = TRUE);
    void MoveObjsInSelectList(BOOL bToFront, BOOL bInvalidate = TRUE);
    void NudgeObjsInSelectList(int dX, int dY, BOOL forceScroll = FALSE); //DFM19991014
#endif
    CDrawList* GetDrawList(BOOL bCanCreateList = TRUE);

// Implementation
protected:
    // Grid and limiting support
    BOOL IsGridizeActive() const;
    void GridizeX(decltype(wxPoint::x)& xPos) const;
    void GridizeY(decltype(wxPoint::y)& yPos) const;
    void LimitPoint(wxPoint& pPnt) const;
    void LimitRect(wxRect& pRct) const;
#if 0
    void PixelToWorkspace(CPoint& point) const;
#endif

    void CreateTextDrawingObject(wxPoint pnt, FontID fid, wxColour crText,
        const CB::string& m_strText, BOOL bInvalidate = TRUE);

    void DoViewScale(TileScale nZoom);
    void CenterViewOnWorkspacePoint(wxPoint point);

#if 0
    BOOL DoMouseWheelFix(UINT fFlags, short zDelta, CPoint point);
#endif

public:
    ~CBrdEditView() override;
    void OnDraw(wxDC& pDC) override;      // overridden to draw this view

#if 0
#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif
    BOOL PreCreateWindow(CREATESTRUCT& cs) override;
#endif
    void OnInitialUpdate();
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

protected:
#if 0
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
#endif

    // Generated message map functions
protected:
#if 0
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
#endif
    void OnDragTileItem(DragDropEvent& event);
#if 0
    afx_msg LRESULT OnSetColor(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetCustomColors(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetLineWidth(WPARAM wParam, LPARAM lParam);
    afx_msg void OnOffscreen();
    afx_msg void OnViewGridLines();
    afx_msg void OnDwgToBack();
    afx_msg void OnDwgToFront();
    afx_msg BOOL OnToolPalette(UINT id);
#endif
    void OnEditLayer(wxCommandEvent& event);
#if 0
    afx_msg void OnUpdateViewGridLines(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOffscreen(CCmdUI* pCmdUI);
#endif
    void OnUpdateEditLayer(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg void OnUpdateColorForeground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorBackground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorCustom(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLineWidth(CCmdUI* pCmdUI);
    afx_msg void OnUpdateToolPalette(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDwgToFrontOrBack(CCmdUI* pCmdUI);
#endif
    void OnUpdateViewFullScale(wxUpdateUIEvent& pCmdUI);
    void OnUpdateViewHalfScale(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg void OnDwgFont();
#endif
    void OnViewFullScale(wxCommandEvent& event);
    void OnViewHalfScale(wxCommandEvent& event);
    void OnViewSmallScale(wxCommandEvent& event);
    void OnUpdateViewSmallScale(wxUpdateUIEvent& pCmdUI);
#if 0
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
#endif
    void OnViewToggleScale(wxCommandEvent& event);
    void OnUpdateEnable(wxUpdateUIEvent& pCmdUI);
    void OnScrollWinLine(wxScrollWinEvent& event);
    wxDECLARE_EVENT_TABLE();

private:
    void RecalcScrollLimits();

    RefPtr<CBrdEditViewContainer> parent;
    RefPtr<CGamDoc> document;
    wxOverlay overlay;

    /* This view should support scrolling by individual pixels,
        but don't make the line-up and line-down scrolling that
        slow.  */
    int m_xScrollPixelsPerLine;
    int m_yScrollPixelsPerLine;

    friend class CBrdEditViewContainer;
};

class CBrdEditViewContainer : public CB::OnCmdMsgOverride<CView>,
                                public CB::wxNativeContainerWindowMixin
{
public:
    const CBrdEditView& GetChild() const { return CheckedDeref(child); }
    CBrdEditView& GetChild()
    {
        return const_cast<CBrdEditView&>(std::as_const(*this).GetChild());
    }
    void OnDraw(CDC* pDC) override;
    void OnInitialUpdate() override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

private:
    CBrdEditViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CBrdEditViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()

    // IGetEvtHandler
    wxEvtHandler& Get() override
    {
        return CheckedDeref(CheckedDeref(child).GetEventHandler());
    }

    // owned by wx
    CB::propagate_const<CBrdEditView*> child = nullptr;

    friend CBrdEditView;
};
