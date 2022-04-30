// FrmPbrd.h : header file
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

#ifndef     _WINMYSPL_H
#include    "WinMyspl.h"
#endif

#ifndef     _PBOARD_H
#include    "PBoard.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardFrame frame

class CPlayBoardView;

class CPlayBoardFrame : public CMDIChildWndEx
{
    DECLARE_DYNCREATE(CPlayBoardFrame)
protected:
    CPlayBoardFrame();  // Protected constructor used by dynamic creation

// Attributes
public:
    CMySplitWnd    m_wndSplitter1;  // The overall view container
    CMySplitWnd    m_wndSplitter2;  // Embedded in the first
    CCbSplitterWnd m_wndSplitBoards;// Holds playing board views

    CB::propagate_const<CPlayBoard*> m_pPBoard;       // The playing board associated with this frame

// Operations
public:
    LRESULT SendMessageToActiveBoardPane(UINT nMsg, WPARAM wParam, LPARAM lParam);

// Implementation
protected:
    virtual ~CPlayBoardFrame();
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

public:
    const CPlayBoardView& GetActiveBoardView() const;
protected:
    CPlayBoardView& GetActiveBoardView() { return const_cast<CPlayBoardView&>(std::as_const(*this).GetActiveBoardView()); }
    CCbSplitterWnd& GetBoardSplitter();

    // Generated message map functions
    //{{AFX_MSG(CPlayBoardFrame)
    afx_msg void OnViewHalfScaleBrd();
    afx_msg void OnUpdateViewHalfScaleBrd(CCmdUI* pCmdUI);
    afx_msg void OnViewFullScaleBrd();
    afx_msg void OnUpdateViewFullScaleBrd(CCmdUI* pCmdUI);
    afx_msg void OnClose();
    afx_msg void OnViewSnapGrid();
    afx_msg void OnUpdateViewSnapGrid(CCmdUI* pCmdUI);
    afx_msg void OnEditSelAllMarkers();
    afx_msg void OnEditBoardProperties();
    afx_msg void OnViewPieces();
    afx_msg void OnUpdateViewPieces(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditSelAllMarkers(CCmdUI* pCmdUI);
    afx_msg void OnViewToggleScale();
    afx_msg void OnUpdateViewToggleScale(CCmdUI* pCmdUI);
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
    afx_msg void OnViewSplitBoardRows();
    afx_msg void OnUpdateViewSplitBoardRows(CCmdUI* pCmdUI);
    afx_msg void OnViewSplitBoardCols();
    afx_msg void OnUpdateViewSplitBoardCols(CCmdUI* pCmdUI);
    //}}AFX_MSG
    afx_msg void OnSelectGroupMarkers(UINT nID);
    afx_msg void OnUpdateSelectGroupMarkers(CCmdUI* pCmdUI, UINT nID);
    afx_msg LRESULT OnMessageCenterBoardOnPoint(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

