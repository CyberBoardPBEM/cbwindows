// FrmPbrd.h : header file
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

#if !defined(_FRMPBRD_H)
#define _FRMPBRD_H

#ifndef     _WINMYSPL_H
#include    "WinMyspl.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayBoardPanel frame

class CenterBoardOnPointEvent;
class CPlayBoard;
class CPlayBoardView;

/* wxDocChildFrame implementation doesn't support multiple views,
    but CBPlay's board frames should.  We will use this class as
    the CBPlay board wxDocChildFrame, CPlayBoardPanelView as
    its corresponding wxView, and CPlayBoardPanel as its
    wxWindow.  CPlayBoardPanel will host the CPlayBoardView
    windows that the user typically interacts with,
    CPlayBoardView will have wxPlayBoardView as its corresponding
    wxView, and the frame is still the wxDocChildFrame. */
/* KLUDGE:  wxDocManager passes events to
            wxView::ProcessEventLocally(), which means
            TryAfter() doesn't get checked, so use this
            class to also give CPlayBoardPanel a chance at
            event */
class DocChildBoardFrame : public CB::DocChildFrame
{
public:
    using CB::DocChildFrame::DocChildFrame;
    bool ProcessEvent(wxEvent& event) override;
};

class CPlayBoardPanel : public wxPanel
{
    friend class CPlayBoardPanelView;
protected:

// Attributes
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxSplitterWindow> m_wndSplitter1;  // The overall view container
        RefPtr<wxSplitterWindow> m_wndSplitter2;  // Embedded in the first
        RefPtr<wxSplitterWindow> m_wndSplitBoards;// Holds playing board views
        RefPtr<CPlayBoardView> m_vwBoard1;
        RefPtr<CPlayBoardView> m_vwBoard2;
    CB_XRC_END_CTRLS_DECL()
public:

    CB::propagate_const<CPlayBoard*> m_pPBoard;       // The playing board associated with this frame

// Operations
public:
    bool SendMessageToActiveBoardPane(wxEvent& event);

// Implementation
protected:
public:     // for parent's OwnerPtr
    ~CPlayBoardPanel() override;
protected:
    CPlayBoardPanel(wxWindow& parent,
                        CGamDoc& doc);
#if 0
    BOOL PreCreateWindow(CREATESTRUCT& cs) override;
#endif

public:
    void SetActiveBoardView(CPlayBoardView& view) { activeView = &view; }
    const CPlayBoardView& GetActiveBoardView() const { return *activeView; }
    CPlayBoardView& GetActiveBoardView() { return const_cast<CPlayBoardView&>(std::as_const(*this).GetActiveBoardView()); }
protected:
    wxSplitterWindow& GetBoardSplitter() { return *m_wndSplitBoards; }

#if 0
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
#endif
    void OnViewSplitBoardRows(wxCommandEvent& event);
    void OnUpdateViewSplitBoardRows(wxUpdateUIEvent& pCmdUI);
    void OnViewSplitBoardCols(wxCommandEvent& event);
    void OnUpdateViewSplitBoardCols(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg void OnSelectGroupMarkers(UINT nID);
    afx_msg void OnUpdateSelectGroupMarkers(CCmdUI* pCmdUI, UINT nID);
#endif
    void OnMessageCenterBoardOnPoint(CenterBoardOnPointEvent& event);
#if 0
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSize(UINT nType, int cx, int cy);
#endif
    wxDECLARE_EVENT_TABLE();

    CB::not_null<CB::propagate_const<CPlayBoardView*>> activeView = &*m_vwBoard1;
};

#if 0
class CPlayBoardPanelContainer : public CB::OnCmdMsgOverride<CMDIChildWndEx>
{
    DECLARE_DYNCREATE(CPlayBoardPanelContainer)
protected:
    CPlayBoardPanelContainer() = default;  // Protected constructor used by dynamic creation

// Attributes
public:
    const CPlayBoardPanel& GetChild() const { return CheckedDeref(child); }
    CPlayBoardPanel& GetChild()
    {
        return const_cast<CPlayBoardPanel&>(std::as_const(*this).GetChild());
    }
    CDocument* GetActiveDocument() override;

// Operations
public:

// Implementation
protected:
    ~CPlayBoardPanelContainer() override = default;
    BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) override;

    void OnUpdateFrameTitle(BOOL bAddToTitle) override;

    afx_msg LRESULT OnMessageCenterBoardOnPoint(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    // IGetEvtHandler
    wxEvtHandler& Get() override
    {
        return CheckedDeref(CheckedDeref(child).GetEventHandler());
    }

    OwnerOrNullPtr<CB::NativeContainerWindowMixin> wxWindow;
    // owned by wx
    CB::propagate_const<CPlayBoardPanel*> child = nullptr;

    typedef CB::OnCmdMsgOverride<CMDIChildWndEx> BASE;
};
#endif

class CPlayBoardPanelView : public CB::View
{
public:
    static void New(CGamDoc& doc);

    const CPlayBoardPanel& GetPanel() const;
    CPlayBoardPanel& GetPanel()
    {
        return const_cast<CPlayBoardPanel&>(std::as_const(*this).GetPanel());
    }
    operator const CPlayBoardPanel&() const { return GetPanel(); }
    operator CPlayBoardPanel&()
    {
        return const_cast<CPlayBoardPanel&>(static_cast<const CPlayBoardPanel&>(std::as_const(*this)));
    }

    void Activate(bool activate) override;
    bool OnClose(bool deleteWindow) override;
    bool OnCreate(wxDocument* doc, long flags) override;

protected:
    const wxWindow& DoGetWindow() const override { return GetPanel(); }

private:
    wxDECLARE_DYNAMIC_CLASS(CPlayBoardPanelView);

    typedef CB::View BASE;
};

/////////////////////////////////////////////////////////////////////////////

#endif

