// VwSelpce.h : header file
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

#ifndef     _LBOXSLCT_H
#include    "LBoxSlct.h"
#endif

class CPlayBoard;

/////////////////////////////////////////////////////////////////////////////
// CSelectedPieceView view

class CSelectedPieceView : public CB::ProcessEventOverride<wxPanel>
{
private:
    friend class CSelectedPieceViewContainer;
    typedef CB::ProcessEventOverride<wxPanel> BASE;
    CSelectedPieceView(CSelectedPieceViewContainer& parent);

// Attributes
private:
    CGamDoc& GetDocument();

// Operations
public:

// Implementation
private:
    RefPtr<CSelectedPieceViewContainer> parent;
    RefPtr<CGamDoc> document;
protected:
    RefPtr<CPlayBoard> m_pPBoard;      // Board that contains selections

    // owned by wx
    RefPtr<CSelectListBox> m_listSel;
    std::vector<RefPtr<CDrawObj>> m_tblSel;
    CB::ToolTip    m_toolTip;

// Implementation
protected:
    void ModifySelectionsBasedOnListItems(BOOL bRemoveSelectedItems);

    ~CSelectedPieceView() override;
    void OnInitialUpdate();     // first time after construct
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

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
    // IGetCmdTarget
    CCmdTarget& Get() override;
};

#ifndef _DEBUG  // debug version in vwselpce.cpp
inline CGamDoc& CSelectedPieceView::GetDocument()
   { return *document; }
#endif

class CSelectedPieceViewContainer : public CB::OnCmdMsgOverride<CView>,
                                    public CB::wxNativeContainerWindowMixin
{
public:
    void OnDraw(CDC* pDC) override;

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

inline CCmdTarget& CSelectedPieceView::Get()
{
    return *parent;
}
