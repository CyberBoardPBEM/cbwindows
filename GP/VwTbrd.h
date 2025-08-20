// VwTbrd.h : header file
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

/////////////////////////////////////////////////////////////////////////////
// CTinyBoardView view

class CTinyBoardView : public CB::ProcessEventOverride<wxScrolledCanvas>
{
private:
    friend class CTinyBoardViewContainer;
    typedef CB::ProcessEventOverride<wxScrolledCanvas> BASE;
    CTinyBoardView(CTinyBoardViewContainer& parent);

// Attributes

// Operations
public:

// Implementation
private:
    // member declaration order determines construction order
    RefPtr<CTinyBoardViewContainer> parent;
    RefPtr<CGamDoc> document;
protected:
    RefPtr<CPlayBoard> m_pPBoard; // The playing board we are viewing
    wxBitmap m_pBMap;            // Cached predrawn board bitmap

    TileScale   m_nZoom;

    void RegenCachedMap(wxDC& pDC);
    wxBitmap DrawFullMap();

// Implementation
protected:
    class DCSetupDrawListDC
    {
    public:
        DCSetupDrawListDC(const CTinyBoardView& rThis, wxDC& pDC, wxRect& rct);
    private:
        CB::DCUserScaleChanger scaleChanger;
    };
    // -------- //
    void WorkspaceToClient(wxRect& rect) const;
    void ClientToWorkspace(wxPoint& pnt) const;
    void InvalidateWorkspaceRect(const wxRect& pRect, BOOL bErase = FALSE);

// Implementation
protected:
    ~CTinyBoardView() override = default;
#if 0
    BOOL PreCreateWindow(CREATESTRUCT& cs) override;
#endif
    void OnInitialUpdate();     // first time after construct
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
    void OnDraw(wxDC& pDC) override;      // overridden to draw this view

    void OnLButtonDown(wxMouseEvent& event);
    void OnRButtonDown(wxMouseEvent& event);
#if 0
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
#endif
    void OnMessageWindowState(WinStateEvent& event);
    wxDECLARE_EVENT_TABLE();

private:
    // IGetCmdTarget
    CCmdTarget& Get() override;

    void RecalcScrollLimits();
};

class CTinyBoardViewContainer : public CB::OnCmdMsgOverride<CView>,
                                public CB::wxNativeContainerWindowMixin
{
public:
    void OnDraw(CDC* pDC) override;

    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

private:
    CTinyBoardViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CTinyBoardViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
#if 0
    afx_msg void OnSize(UINT nType, int cx, int cy);
#endif
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg LRESULT OnMessageWindowState(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    // IGetEvtHandler
    wxEvtHandler& Get() override
    {
        return CheckedDeref(CheckedDeref(child).GetEventHandler());
    }

    // owned by wx
    CB::propagate_const<CTinyBoardView*> child = nullptr;

    typedef CB::OnCmdMsgOverride<CView> BASE;
    friend CTinyBoardView;
};

inline CCmdTarget& CTinyBoardView::Get()
{
    return *parent;
}


