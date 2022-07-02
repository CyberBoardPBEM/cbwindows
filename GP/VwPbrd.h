// VwPbrd.h : interface of the CPlayBoardView class
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

class CPlayBoardView : public CScrollView
{
    friend class CPlayBoardFrame;

protected: // create from serialization only
    CPlayBoardView();
    DECLARE_DYNCREATE(CPlayBoardView)

// Attributes
public:
    const CGamDoc* GetDocument() const;
    CGamDoc* GetDocument() { return const_cast<CGamDoc*>(std::as_const(*this).GetDocument()); }
    const CPlayBoard* GetPlayBoard() const { return m_pPBoard.get(); }
    CPlayBoard* GetPlayBoard() { return const_cast<CPlayBoard*>(std::as_const(*this).GetPlayBoard()); }

// Operations
public:

// Implementation
public:
    virtual ~CPlayBoardView();
    virtual void OnDraw(CDC* pDC) override;      // Overridden to draw this view

// Tools and selection support
public:
    CSelList    m_selList;              // List of selected objects.

    void NotifySelectListChange();
    const CSelList* GetSelectList() const { return &m_selList; }
    CSelList* GetSelectList() { return const_cast<CSelList*>(std::as_const(*this).GetSelectList()); }
    CPoint GetWorkspaceDim() const;

    void AddDrawObject(CDrawObj::OwnerPtr pObj);
    void MoveObjsInSelectList(BOOL bToFront, BOOL bInvalidate = TRUE);

    void PrepareScaledDC(CDC& pDC, CRect* pRct = NULL, BOOL bHonor180Flip = FALSE) const;
    void OnPrepareScaledDC(CDC& pDC, BOOL bHonor180Flip = FALSE);

    void AdjustPoint(CPoint& pnt) const;      // Limit and grid processing
    void AdjustRect(CRect& rct) const;

    void SelectWithinRect(CRect rctNet, BOOL bInclIntersects = FALSE);
    void SelectAllUnderPoint(CPoint point);
    CDrawObj* ObjectHitTest(CPoint point);
    void SelectAllObjectsInList(const std::vector<CB::not_null<CDrawObj*>>& pLst);
    void SelectAllObjectsInTable(const std::vector<CB::not_null<CDrawObj*>>& pTbl);
    void SelectMarkersInGroup(size_t nGroup);
    void SelectAllMarkers();

    // TEMP FOR NOW!
    COLORREF GetTextColor() const { return RGB(255, 0, 0); }
    COLORREF GetLineColor() const { return RGB(0, 255, 0); }
    UINT GetLineWidth() const { return 3; }

// Coordinate scaling...
public:
    void WorkspaceToClient(CPoint& point) const;
    void WorkspaceToClient(CRect& rect) const;
    void ClientToWorkspace(CPoint& point) const;
    void ClientToWorkspace(CRect& rect) const;
    void InvalidateWorkspaceRect(const CRect& pRect, BOOL bErase = FALSE);

// View support
public:
    void ScrollWorkspacePointIntoView(CPoint point);
    void CenterViewOnWorkspacePoint(CPoint point);
    BOOL CheckAutoScroll(CPoint point);
    BOOL ProcessAutoScroll(CPoint point);
    void SetOurScrollSizes(TileScale nZoom);
    void DoViewScaleBrd(TileScale nZoom);

// Tooltip Support
public:
    void SetNotificationTip(CPoint pointClient, UINT nResID);
    void SetNotificationTip(CPoint pointClient, LPCTSTR pszTip);
    void ClearNotificationTip();
    void ClearToolTip();
    static void CALLBACK NotificationTipTimeoutHandler(HWND hwnd, UINT uMsg,
        UINT_PTR idEvent, DWORD dwTime);
    void DoToolTipHitProcessing(CPoint pointClient);

// Grid and limiting support
protected:
    BOOL IsGridizeActive() const;
#ifdef WIN32
    void GridizeX(long& xPos) const;
    void GridizeY(long& yPos) const;
#else
    void GridizeX(int& xPos) const;
    void GridizeY(int& yPos) const;
#endif
    void LimitPoint(POINT& pPnt) const;
    void LimitRect(RECT& pRct) const;
    BOOL IsRectFullyOnBoard(const RECT& pRct, BOOL* pbXOK = NULL, BOOL* pbYOK = NULL) const;

// Implementation
protected:
    CB::propagate_const<CPlayBoard*> m_pPBoard;          // Board that contains selections etc...
    TileScale   m_nZoom;            // Current zoom level of view
    // -------- //
    BOOL        m_bInDrag;          // Currently being dragged over
    CB::propagate_const<CSelList*> m_pDragSelList;     // Pointer the select list being dragged
    uintptr_t    m_nTimerID;         // Used to control autoscrolls
    // -------- //
    UINT        m_nCurToolID;       // Current tool ID
    // -------- //
    CToolTipCtrl m_toolMsgTip;      // Tooltip for notifications
    CToolTipCtrl m_toolHitTip;      // Tooltip hit support for view
    CB::propagate_const<CDrawObj*> m_pCurTipObj;      // Currently hit tip object

    // Tables used to process relative piece rotations. DON'T Serialize!
    BOOL        m_bWheelRotation;   // Indicates the type of rotation being done
    CPoint      m_pntWheelMid;      // The wheel rotation point
    std::vector<uint16_t> m_tblCurAngles;     // Original angles of pieces
    std::vector<CB::not_null<CDrawObj*>> m_tblCurPieces;     // Pieces being rotated
    CUIntArray  m_tblXMidPnt;       // X coord of piece midpoint
    CUIntArray  m_tblYMidPnt;       // Y coord of piece midpoint

// Implementation
protected:
    BOOL IsBoardContentsAvailableToCurrentPlayer() const;

    void AddPiece(CPoint pnt, PieceID pid);

    PToolType MapToolType(UINT nToolResID) const;

    void SetupDrawListDC(CDC& pDC, CRect& pRct) const;
    void RestoreDrawListDC(CDC& pDC) const;

    LRESULT DoDragPiece(WPARAM wParam, DragInfo& pdi);
    LRESULT DoDragMarker(WPARAM wParam, DragInfo& pdi);
    LRESULT DoDragPieceList(WPARAM wParam, DragInfo& pdi);
    LRESULT DoDragSelectList(WPARAM wParam, DragInfo& pdi);

    void DragDoAutoScroll();
    void DragCheckAutoScroll();
    void DragKillAutoScroll();

    void DoAutostackOfSelectedObjects(int xStagger, int yStagger);
    void DoRotateRelative(BOOL bWheelRotation);

    BOOL DoMouseWheelFix(UINT fFlags, short zDelta, CPoint point);

protected:
    virtual void OnInitialUpdate();
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
                    CView* pDeactiveView);

    // Printing support
    virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
    virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Generated message map functions
protected:
    //{{AFX_MSG(CPlayBoardView)
    afx_msg void OnViewFullScaleBrd();
    afx_msg void OnUpdateViewFullScaleBrd(CCmdUI* pCmdUI);
    afx_msg void OnViewHalfScaleBrd();
    afx_msg void OnUpdateViewHalfScaleBrd(CCmdUI* pCmdUI);
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
    afx_msg void OnActTurnOver();
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
    afx_msg void OnViewToggleScale();
    afx_msg void OnUpdateViewToggleScale(CCmdUI* pCmdUI);
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
    afx_msg void OnViewSmallScaleBoard();
    afx_msg void OnUpdateViewSmallScaleBoard(CCmdUI* pCmdUI);
    afx_msg void OnViewBoardRotate180();
    afx_msg void OnUpdateViewBoardRotate180(CCmdUI* pCmdUI);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    //}}AFX_MSG
    afx_msg void OnSelectGroupMarkers(UINT nID);
    afx_msg void OnUpdateSelectGroupMarkers(CCmdUI* pCmdUI, UINT nID);
    afx_msg void OnRotatePiece(UINT nID);
    afx_msg void OnUpdateRotatePiece(CCmdUI* pCmdUI, UINT nID);
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMessageSelectBoardObjectList(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnActRotateGroupRelative();
    afx_msg void OnUpdateActRotateGroupRelative(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in vwmbrd.cpp
inline const CGamDoc* CPlayBoardView::GetDocument() const
   { return (const CGamDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

