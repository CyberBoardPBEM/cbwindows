// VwSelpce.h : header file
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

#ifndef     _LBOXSLCT_H
#include    "LBoxSlct.h"
#endif

class CPlayBoard;
class CSelectedPieceView;
class WinStateEvent;
class wxSelectedPieceView;

/////////////////////////////////////////////////////////////////////////////
// CSelectedPieceView view

class CSelectedPieceView : public wxPanel
{
    wxDECLARE_DYNAMIC_CLASS(CSelectedPieceView);
private:
#if 0
    friend class CSelectedPieceViewContainer;
#endif
    friend wxSelectedPieceView;
    typedef CB::ProcessEventOverride<wxPanel> BASE;
    CSelectedPieceView();
    void Initialize();

// Attributes
public:
    operator const wxSelectedPieceView&() const { return *wxview; }
    operator wxSelectedPieceView&() { return const_cast<wxSelectedPieceView&>(static_cast<const wxSelectedPieceView&>(std::as_const(*this))); }
    operator const wxView&() const;
    operator wxView&() { return const_cast<wxView&>(static_cast<const wxView&>(std::as_const(*this))); }
    operator const wxView*() const { return &static_cast<const wxView&>(*this); }
    operator wxView*() { return const_cast<wxView*>(static_cast<const wxView*>(std::as_const(*this))); }

private:
    CGamDoc& GetDocument();

// Operations
public:

// Implementation
private:
    CB::propagate_const<wxSplitterWindow*> parent = nullptr;
    CB::propagate_const<CGamDoc*> document = nullptr;
protected:
    CB::propagate_const<CPlayBoard*> m_pPBoard = nullptr;      // Board that contains selections

    // owned by wx
    RefPtr<CSelectListBox> m_listSel = new CSelectListBox;
    std::vector<RefPtr<CDrawObj>> m_tblSel;
    CB::ToolTip    m_toolTip;

// Implementation
protected:
    void ModifySelectionsBasedOnListItems(BOOL bRemoveSelectedItems);

    ~CSelectedPieceView() override;
public:
    void OnInitialUpdate(CGamDoc& doc);     // first time after construct
protected:
    void OnUpdate(wxView* sender, const CGamDocHint& hint);

protected:
#if 0
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
#endif
    void OnSize(wxSizeEvent& event);
#if 0
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
#endif
    void OnVKeyToItem(wxKeyEvent& event);
    void OnMessageWindowState(WinStateEvent& event);
    wxDECLARE_EVENT_TABLE();

private:
    OwnerPtr<wxSelectedPieceView> wxview;
};

#ifndef _DEBUG  // debug version in vwselpce.cpp
inline CGamDoc& CSelectedPieceView::GetDocument()
   { return *document; }
#endif

#if 0
class CSelectedPieceViewContainer : public CB::OnCmdMsgOverride<CView>,
                                    public CB::NativeContainerWindowMixin
{
public:
    void OnDraw(CDC* pDC) override;

    void OnInitialUpdate() override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

private:
    CSelectedPieceViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CSelectedPieceViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    // IGetEvtHandler
    wxEvtHandler& Get() override
    {
        return CheckedDeref(CheckedDeref(child).GetEventHandler());
    }

    // owned by wx
    CB::propagate_const<CSelectedPieceView*> child = nullptr;

    typedef CB::OnCmdMsgOverride<CView> BASE;
};
#endif

class wxSelectedPieceView : public CB::View
{
    wxDECLARE_DYNAMIC_CLASS(wxSelectedPieceView);
public:
    void OnActivateView(bool activate,
                        wxView *activeView,
                        wxView *deactiveView) override;
    bool OnClose(bool deleteWindow) override;
    bool OnCreate(wxDocument* doc, long flags) override;
    void OnUpdate(wxView* sender, wxObject* hint = nullptr) override;

protected:
    const CSelectedPieceView& DoGetWindow() const override { return *window; }

private:
    wxSelectedPieceView();

    RefPtr<CSelectedPieceView> window;

    friend CSelectedPieceView;
};

inline CSelectedPieceView::operator const wxView&() const
{
    return *wxview;
}
