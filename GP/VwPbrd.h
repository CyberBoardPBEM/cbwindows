// VwPbrd.h : interface of the CPlayBoardView class
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
    void SetOurScrollSizes(TileScale nZoom);
    void DoViewScaleBrd(TileScale nZoom);

// Tooltip Support
public:
    void SetNotificationTip(wxPoint pointClient, UINT nResID);
    void SetNotificationTip(wxPoint pointClient, const CB::string* pszTip);
    void ClearNotificationTip();
    void ClearToolTip();
#if 0
    static void CALLBACK NotificationTipTimeoutHandler(HWND hwnd, UINT uMsg,
        UINT_PTR idEvent, DWORD dwTime);
#endif
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
#endif
    // -------- //
    UINT        m_nCurToolID;       // Current tool ID
    // -------- //
    CB::ToolTip m_toolMsgTip;      // Tooltip for notifications
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

    PToolType MapToolType(UINT nToolResID) const;

    class DCSetupDrawListDC
    {
    public:
        DCSetupDrawListDC(const CPlayBoardView& rThis, wxDC& dc, wxRect& pRct);
    private:
        CB::DCUserScaleChanger scaleChanger;
        CB::DCLogicalOriginChanger logOrgChanger;
    };

    LRESULT DoDragPiece(const DragInfo& pdi);
    LRESULT DoDragMarker(const DragInfo& pdi);
    LRESULT DoDragPieceList(const DragInfo& pdi);
    LRESULT DoDragSelectList(const DragInfo& pdi);

    void DragDoAutoScroll();
    void DragCheckAutoScroll();
    void DragKillAutoScroll();

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
#if 0
    afx_msg LRESULT OnDragItem(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMessageRotateRelative(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMessageCenterBoardOnPoint(WPARAM wParam, LPARAM lParam);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(uintptr_t nIDEvent);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnPlayTool(UINT id);
    afx_msg void OnUpdatePlayTool(CCmdUI* pCmdUI);
    afx_msg void OnActStack();
    afx_msg void OnUpdateActStack(CCmdUI* pCmdUI);
    afx_msg void OnActToBack();
    afx_msg void OnUpdateActToBack(CCmdUI* pCmdUI);
    afx_msg void OnActToFront();
    afx_msg void OnUpdateActToFront(CCmdUI* pCmdUI);
    afx_msg BOOL OnActTurnOver(UINT id);
    afx_msg void OnUpdateActTurnOver(CCmdUI* pCmdUI);
    afx_msg void OnActPlotMove();
    afx_msg void OnUpdateActPlotMove(CCmdUI* pCmdUI);
    afx_msg void OnActPlotDone();
    afx_msg void OnUpdateActPlotDone(CCmdUI* pCmdUI);
    afx_msg void OnActPlotDiscard();
    afx_msg void OnUpdateActPlotDiscard(CCmdUI* pCmdUI);
    afx_msg void OnUpdateIndicatorCellNum(CCmdUI* pCmdUI);
    afx_msg void OnViewSnapGrid();
    afx_msg void OnUpdateViewSnapGrid(CCmdUI* pCmdUI);
    afx_msg void OnEditSelAllMarkers();
    afx_msg void OnUpdateEditSelAllMarkers(CCmdUI* pCmdUI);
    afx_msg void OnActRotate();
    afx_msg void OnUpdateActRotate(CCmdUI* pCmdUI);
#endif
    void OnViewToggleScale(wxCommandEvent& event);
    void OnUpdateViewToggleScale(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg void OnViewPieces();
    afx_msg void OnUpdateViewPieces(CCmdUI* pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnEditBoardToFile();
    afx_msg void OnEditBoardProperties();
    afx_msg void OnActRotateRelative();
    afx_msg void OnUpdateActRotateRelative(CCmdUI* pCmdUI);
    afx_msg void OnEditClear();
    afx_msg void OnUpdateEditClear(CCmdUI* pCmdUI);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnViewDrawIndOnTop();
    afx_msg void OnUpdateViewDrawIndOnTop(CCmdUI* pCmdUI);
    afx_msg void OnEditElementText();
    afx_msg void OnUpdateEditElementText(CCmdUI* pCmdUI);
    afx_msg void OnActLockObject();
    afx_msg void OnUpdateActLockObject(CCmdUI* pCmdUI);
    afx_msg void OnActLockSuspend();
    afx_msg void OnUpdateActLockSuspend(CCmdUI* pCmdUI);
    afx_msg void OnActShuffleSelectedObjects();
    afx_msg void OnUpdateActShuffleSelectedObjects(CCmdUI* pCmdUI);
    afx_msg void OnActAutostackDeck();
    afx_msg void OnUpdateActAutostackDeck(CCmdUI* pCmdUI);
    afx_msg void OnActTakeOwnership();
    afx_msg void OnUpdateActTakeOwnership(CCmdUI* pCmdUI);
    afx_msg void OnActReleaseOwnership();
    afx_msg void OnUpdateActReleaseOwnership(CCmdUI* pCmdUI);
    afx_msg void OnActSetOwner();
    afx_msg void OnUpdateActSetOwner(CCmdUI* pCmdUI);
#endif
    void OnViewSmallScaleBoard(wxCommandEvent& event);
    void OnUpdateViewSmallScaleBoard(wxUpdateUIEvent& pCmdUI);
    void OnViewBoardRotate180(wxCommandEvent& event);
    void OnUpdateViewBoardRotate180(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnSelectGroupMarkers(UINT nID);
    afx_msg void OnUpdateSelectGroupMarkers(CCmdUI* pCmdUI, UINT nID);
    afx_msg void OnRotatePiece(UINT nID);
    afx_msg void OnUpdateRotatePiece(CCmdUI* pCmdUI, UINT nID);
#endif
    void OnMessageWindowState(WinStateEvent& event);
#if 0
    afx_msg LRESULT OnMessageSelectBoardObjectList(WPARAM wParam, LPARAM lParam);
#endif
    void OnScrollWinLine(wxScrollWinEvent& event);
    wxDECLARE_EVENT_TABLE();
#if 0
public:
    afx_msg void OnActRotateGroupRelative();
    afx_msg void OnUpdateActRotateGroupRelative(CCmdUI *pCmdUI);
#endif

private:
    // IGetCmdTarget
    CCmdTarget& Get() override;

    /* This view should support scrolling by individual pixels,
        but don't make the line-up and line-down scrolling that
        slow.  */
    int m_xScrollPixelsPerLine;
    int m_yScrollPixelsPerLine;
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

