// VwPbrd.h : interface of the CPlayBoardView class
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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

#ifndef     _TOOLPLAY_H
#include    "ToolPlay.h"
#endif

/////////////////////////////////////////////////////////////////////////////

#define     ID_TIP_PLAYBOARD_HIT        1       // ID used for hit tested tips
#define     ID_TIP_PLAYBOARD_MSG        2       // ID used for notify tips

#define     ID_TIP_MSG_TIMER            42      // ID used in tip timer
#define     MAX_TIP_MSG_TIME            4000    // Max time for tip in ms.

#define     MAX_PLAYBOARD_TIP_WIDTH     200     // Max pixel width of tips

/////////////////////////////////////////////////////////////////////////////

class CPlayBoard;
enum  TileScale;

class CPlayBoardView : public CB::ProcessEventOverride<wxScrolledCanvas>
{
private:
    friend class CPlayBoardFrame;
    friend class CPlayBoardViewContainer;
    typedef CB::ProcessEventOverride<wxScrolledCanvas> BASE;
    CPlayBoardView(CPlayBoardViewContainer& parent);

// Attributes
public:
    const CGamDoc& GetDocument() const;
    CGamDoc& GetDocument() { return const_cast<CGamDoc&>(std::as_const(*this).GetDocument()); }
    const CPlayBoard& GetPlayBoard() const { return *m_pPBoard; }
    CPlayBoard& GetPlayBoard() { return const_cast<CPlayBoard&>(std::as_const(*this).GetPlayBoard()); }
    wxOverlay& GetOverlay() { return *overlay; }
    CFrameWnd* GetParentFrame();

// Operations
public:

// Implementation
public:
    ~CPlayBoardView() override;
    void OnDraw(wxDC& pDC) override;      // Overridden to draw this view

// Tools and selection support
public:
    CSelList    m_selList;              // List of selected objects.

    void NotifySelectListChange();
    const CSelList& GetSelectList() const { return m_selList; }
    CSelList& GetSelectList() { return const_cast<CSelList&>(std::as_const(*this).GetSelectList()); }
    wxPoint GetWorkspaceDim() const;

    void AddDrawObject(CDrawObj::OwnerPtr pObj);
    void MoveObjsInSelectList(BOOL bToFront, BOOL bInvalidate = TRUE);

    void PrepareScaledDC(wxDC& pDC, wxRect* pRct = NULL, BOOL bHonor180Flip = FALSE) const;
    void OnPrepareScaledDC(wxDC& pDC, BOOL bHonor180Flip = FALSE);

    [[nodiscard]] wxPoint AdjustPoint(wxPoint pnt) const;      // Limit and grid processing
    [[nodiscard]] wxRect AdjustRect(wxRect rct) const;

    void SelectWithinRect(wxRect rctNet, BOOL bInclIntersects = FALSE);
    void SelectAllUnderPoint(wxPoint point);
    CDrawObj* ObjectHitTest(wxPoint point);
    void SelectAllObjectsInList(const std::vector<CB::not_null<CDrawObj*>>& pLst);
    void SelectAllObjectsInTable(const std::vector<CB::not_null<CDrawObj*>>& pTbl);
    void SelectMarkersInGroup(size_t nGroup);
    void SelectAllMarkers();

    // TEMP FOR NOW!
    wxColour GetTextColor() const { return wxColour(255, 0, 0); }
    wxColour GetLineColor() const { return wxColour(0, 255, 0); }
    UINT GetLineWidth() const { return 3; }

// Coordinate scaling...
public:
    [[nodiscard]] wxPoint WorkspaceToClient(wxPoint point) const;
    [[nodiscard]] wxRect WorkspaceToClient(wxRect rect) const;
    [[nodiscard]] wxPoint ClientToWorkspace(wxPoint point) const;
    [[nodiscard]] wxRect ClientToWorkspace(wxRect rect) const;
    void InvalidateWorkspaceRect(const wxRect& pRect, BOOL bErase = FALSE);

// View support
public:
    void ScrollWorkspacePointIntoView(wxPoint point);
    void CenterViewOnWorkspacePoint(wxPoint point);
    BOOL CheckAutoScroll(wxPoint point);
    BOOL ProcessAutoScroll(wxPoint point);
    // this method can be overridden in a derived class to forbid sending the
    // auto scroll events - note that unlike StopAutoScrolling() it doesn't
    // stop the timer, so it will be called repeatedly and will typically
    // return different values depending on the current mouse position
    bool SendAutoScrollEvents(wxScrollWinEvent& event) const override;
    void SetOurScrollSizes(TileScale nZoom);
    void DoViewScaleBrd(TileScale nZoom);

// Tooltip Support
public:
#if 0
    void SetNotificationTip(wxPoint pointClient, UINT nResID);
#endif
    void SetNotificationTip(wxPoint pointClient, const CB::string& pszTip);
    void ClearNotificationTip();
    void ClearToolTip();
    void NotificationTipTimeoutHandler(wxTimerEvent& event);
    void DoToolTipHitProcessing(wxPoint pointClient);

// Grid and limiting support
protected:
    BOOL IsGridizeActive() const;
#ifdef WIN32
    [[nodiscard]] long GridizeX(long xPos) const;
    [[nodiscard]] long GridizeY(long yPos) const;
#else
    void GridizeX(int& xPos) const;
    void GridizeY(int& yPos) const;
#endif
    [[nodiscard]] wxPoint LimitPoint(wxPoint pPnt) const;
    [[nodiscard]] wxRect LimitRect(wxRect pRct) const;
    BOOL IsRectFullyOnBoard(const wxRect& pRct, BOOL* pbXOK = NULL, BOOL* pbYOK = NULL) const;

// Implementation
private:
    // member declaration order determines construction order
    RefPtr<CPlayBoardViewContainer> parent;
    RefPtr<CGamDoc> document;
protected:
    RefPtr<CPlayBoard> m_pPBoard;          // Board that contains selections etc...
    TileScale   m_nZoom;            // Current zoom level of view
    // -------- //
    BOOL        m_bInDrag;          // Currently being dragged over
    CB::propagate_const<CSelList*> m_pDragSelList;     // Pointer the select list being dragged
#if 0
    uintptr_t    m_nTimerID;         // Used to control autoscrolls
#else
public:
    void SetTimer(int id, unsigned milliseconds);
    void KillTimer(int id);
private:
    OwnerOrNullPtr<wxTimer> timer;
protected:
#endif
    // -------- //
    int         m_nCurToolID;       // Current tool ID
    // -------- //
    CB::ToolTip m_toolMsgTip;      // Tooltip for notifications
    wxTimer m_toolMsgTipTimer;
    CB::ToolTip m_toolHitTip;      // Tooltip hit support for view
    wxRect m_toolHitTipRect = wxRect();
    CB::propagate_const<CDrawObj*> m_pCurTipObj;      // Currently hit tip object

    // Tables used to process relative piece rotations. DON'T Serialize!
    BOOL        m_bWheelRotation;   // Indicates the type of rotation being done
    wxPoint     m_pntWheelMid;      // The wheel rotation point
    std::vector<uint16_t> m_tblCurAngles;     // Original angles of pieces
    std::vector<RefPtr<CDrawObj>> m_tblCurPieces;     // Pieces being rotated
    std::vector<wxPoint> m_tblMidPnt;       // X coord of piece midpoint

// Implementation
protected:
    BOOL IsBoardContentsAvailableToCurrentPlayer() const;

    void AddPiece(wxPoint pnt, PieceID pid);

    PToolType MapToolType(int nToolResID) const;

    class DCSetupDrawListDC
    {
    public:
        DCSetupDrawListDC(const CPlayBoardView& rThis, wxDC& dc, wxRect& pRct);
    private:
        CB::DCUserScaleChanger scaleChanger;
        CB::DCLogicalOriginChanger logOrgChanger;
    };

    void DoDragPiece(const DragInfoWx& pdi);
    void DoDragMarker(DragDropEvent& event);
    void DoDragPieceList(const DragInfoWx& pdi);
    void DoDragSelectList(DragDropEvent& event);

#if 0
    void DragDoAutoScroll();
    void DragCheckAutoScroll();
    void DragKillAutoScroll();
#endif

    void DoAutostackOfSelectedObjects(int xStagger, int yStagger);
    void DoRotateRelative(BOOL bWheelRotation);

#if 0
    BOOL DoMouseWheelFix(UINT fFlags, short zDelta, CPoint point);
#endif

protected:
    void OnInitialUpdate();
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
#if 0
    BOOL PreCreateWindow(CREATESTRUCT& cs) override;
    BOOL PreTranslateMessage(MSG* pMsg) override;
#endif
    void OnActivateView(BOOL bActivate, CView* pActivateView,
                    CView* pDeactiveView);

protected:
    void OnViewFullScaleBrd(wxCommandEvent& event);
    void OnUpdateViewFullScaleBrd(wxUpdateUIEvent& pCmdUI);
    void OnViewHalfScaleBrd(wxCommandEvent& event);
    void OnUpdateViewHalfScaleBrd(wxUpdateUIEvent& pCmdUI);
    void OnDragItem(DragDropEvent& event);
    void OnMessageRotateRelative(RotatePieceDeltaEvent& event);
    void OnMessageCenterBoardOnPoint(CenterBoardOnPointEvent& event);
    void OnLButtonDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnLButtonUp(wxMouseEvent& event);
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnLButtonDblClk(wxMouseEvent& event);
    void OnSetCursor(wxSetCursorEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnPlayTool(wxCommandEvent& event);
    void OnUpdatePlayTool(wxUpdateUIEvent& pCmdUI);
    void OnActStack(wxCommandEvent& event);
    void OnUpdateActStack(wxUpdateUIEvent& pCmdUI);
    void OnActToBack(wxCommandEvent& event);
    void OnUpdateActToBack(wxUpdateUIEvent& pCmdUI);
    void OnActToFront(wxCommandEvent& event);
    void OnUpdateActToFront(wxUpdateUIEvent& pCmdUI);
    void OnActTurnOver(wxCommandEvent& event);
    void OnUpdateActTurnOver(wxUpdateUIEvent& pCmdUI);
    void OnActPlotMove(wxCommandEvent& event);
    void OnUpdateActPlotMove(wxUpdateUIEvent& pCmdUI);
    void OnActPlotDone();
    void OnActPlotDone(wxCommandEvent& /*event*/) { OnActPlotDone(); }
    void OnUpdateActPlotDone(wxUpdateUIEvent& pCmdUI);
    void OnActPlotDiscard();
    void OnActPlotDiscard(wxCommandEvent& /*event*/) { OnActPlotDiscard(); }
    void OnUpdateActPlotDiscard(wxUpdateUIEvent& pCmdUI);
    void OnUpdateIndicatorCellNum(wxUpdateUIEvent& pCmdUI);
    void OnViewSnapGrid(wxCommandEvent& event);
    void OnUpdateViewSnapGrid(wxUpdateUIEvent& pCmdUI);
    void OnEditSelAllMarkers(wxCommandEvent& event);
    void OnUpdateEditSelAllMarkers(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg void OnActRotate();
    afx_msg void OnUpdateActRotate(CCmdUI* pCmdUI);
#endif
    void OnViewToggleScale(wxCommandEvent& event);
    void OnUpdateViewToggleScale(wxUpdateUIEvent& pCmdUI);
    void OnViewPieces(wxCommandEvent& event);
    void OnUpdateViewPieces(wxUpdateUIEvent& pCmdUI);
    void OnEditCopy(wxCommandEvent& event);
    void OnEditBoardToFile(wxCommandEvent& event);
    void OnEditBoardProperties(wxCommandEvent& event);
    void OnActRotateRelative(wxCommandEvent& event);
    void OnUpdateActRotateRelative(wxUpdateUIEvent& pCmdUI);
    void OnEditClear();
    void OnEditClear(wxCommandEvent& /*event*/) { OnEditClear(); }
    void OnUpdateEditClear(wxUpdateUIEvent& pCmdUI);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnViewDrawIndOnTop(wxCommandEvent& event);
    void OnUpdateViewDrawIndOnTop(wxUpdateUIEvent& pCmdUI);
    void OnEditElementText(wxCommandEvent& event);
    void OnUpdateEditElementText(wxUpdateUIEvent& pCmdUI);
    void OnActLockObject(wxCommandEvent& event);
    void OnUpdateActLockObject(wxUpdateUIEvent& pCmdUI);
    void OnActLockSuspend(wxCommandEvent& event);
    void OnUpdateActLockSuspend(wxUpdateUIEvent& pCmdUI);
    void OnActShuffleSelectedObjects(wxCommandEvent& event);
    void OnUpdateActShuffleSelectedObjects(wxUpdateUIEvent& pCmdUI);
    void OnActAutostackDeck(wxCommandEvent& event);
    void OnUpdateActAutostackDeck(wxUpdateUIEvent& pCmdUI);
    void OnActTakeOwnership(wxCommandEvent& event);
    void OnUpdateActTakeOwnership(wxUpdateUIEvent& pCmdUI);
    void OnActReleaseOwnership(wxCommandEvent& event);
    void OnUpdateActReleaseOwnership(wxUpdateUIEvent& pCmdUI);
    void OnActSetOwner(wxCommandEvent& event);
    void OnUpdateActSetOwner(wxUpdateUIEvent& pCmdUI);
    void OnViewSmallScaleBoard(wxCommandEvent& event);
    void OnUpdateViewSmallScaleBoard(wxUpdateUIEvent& pCmdUI);
    void OnViewBoardRotate180(wxCommandEvent& event);
    void OnUpdateViewBoardRotate180(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
#endif
    void OnSelectGroupMarkers(wxCommandEvent& event);
    void OnMenuOpen(wxMenuEvent& event);
    void OnUpdateSelectGroupMarkers(wxMenu& menu);
    void OnRotatePiece(wxCommandEvent& event);
    void OnUpdateRotatePiece(wxUpdateUIEvent& pCmdUI);
    void OnMessageWindowState(WinStateEvent& event);
    void OnMessageSelectBoardObjectList(SelectBoardObjListEvent& event);
    void OnScrollWinLine(wxScrollWinEvent& event);
    wxDECLARE_EVENT_TABLE();
public:
    void OnActRotateGroupRelative(wxCommandEvent& event);
    void OnUpdateActRotateGroupRelative(wxUpdateUIEvent& pCmdUI);
private:
    void OnUpdateEnable(wxUpdateUIEvent& pCmdUI);

private:
    // IGetCmdTarget
    CCmdTarget& Get() override;

    /* This view should support scrolling by individual pixels,
        but don't make the line-up and line-down scrolling that
        slow.  */
    int m_xScrollPixelsPerLine;
    int m_yScrollPixelsPerLine;

    OwnerPtr<wxOverlay> overlay = MakeOwner<wxOverlay>();
    int m_bindEnd = 0;
};

#ifndef _DEBUG  // debug version in vwmbrd.cpp
inline const CGamDoc& CPlayBoardView::GetDocument() const
   { return *document; }
#endif

class CPlayBoardViewContainer : public CB::OnCmdMsgOverride<CView>,
                                public CB::wxNativeContainerWindowMixin
{
public:
    void OnDraw(CDC* pDC) override;
    operator const CPlayBoardView&() const { return *child; }
    operator CPlayBoardView&()
    {
        return const_cast<CPlayBoardView&>(static_cast<const CPlayBoardView&>(std::as_const(*this)));
    }

    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
    void OnActivateView(BOOL bActivate, CView* pActivateView,
                    CView* pDeactiveView) override;

private:
    CPlayBoardViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CPlayBoardViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    // IGetEvtHandler
    wxEvtHandler& Get() override
    {
        return CheckedDeref(CheckedDeref(child).GetEventHandler());
    }

    // owned by wx
    CB::propagate_const<CPlayBoardView*> child = nullptr;

    typedef CB::OnCmdMsgOverride<CView> BASE;
};

inline CCmdTarget& CPlayBoardView::Get()
{
    return *parent;
}

/////////////////////////////////////////////////////////////////////////////

