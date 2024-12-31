// FrmBited.h
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

#include "VwBitedt.h"

class CTileSelView;
class wxBitEditView;

/////////////////////////////////////////////////////////////////////////////
// CBitEditFrame frame

class CBitEditFrame : public wxDocChildFrameAny<wxAuiMDIChildFrame, wxAuiMDIParentFrame>
{
protected:
    CBitEditFrame(wxDocument& doc,
                    wxBitEditView& view,
                    wxAuiMDIParentFrame& parent);

// Attributes
protected:
    RefPtr<wxSplitterWindow> m_wndSplitter;

#if 0
// Operations
public:

// Implementation
protected:
    virtual ~CBitEditFrame();

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnCreateClient(LPCREATESTRUCT, CCreateContext* pContext);

    // Generated message map functions
    //{{AFX_MSG(CBitEditFrame)
    afx_msg void OnClose();
#endif
    void OnImageDiscard(wxCommandEvent& event);
    void OnImageUpdate(wxCommandEvent& event);
    void OnToolsResizeTile(wxCommandEvent& event);
#if 0
    afx_msg void OnEditUndo();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
#endif
    void OnRotateTile(wxCommandEvent& event);
    void OnUpdateRotateTile(wxUpdateUIEvent& pCmdUI);
    void OnUpdateEnable(wxUpdateUIEvent& pCmdUI);
    wxDECLARE_EVENT_TABLE();

private:
    using BASE = wxDocChildFrameAny<wxAuiMDIChildFrame, wxAuiMDIParentFrame>;

    CBitEditView& GetBitEditView();
    CTileSelView& GetTileSelView();

    friend class wxBitEditView;
};

class wxBitEditView : public CB::wxView
{
public:
    static wxBitEditView* New(CGamDoc& doc, TileID tid);

    CGamDoc& GetDocument();
    CBitEditFrame& GetFrame();
    CBitEditView& GetWindow() override { return GetBitEditView(); }
    CBitEditView& GetBitEditView() { return GetFrame().GetBitEditView(); }
    CTileSelView& GetTileSelView() { return GetFrame().GetTileSelView(); }

    void OnActivateView(bool activate, ::wxView* activeView, ::wxView* deactiveView) override;
    bool OnClose(bool deleteWindow) override;
    bool OnCreate(wxDocument* doc, long flags) override;
    void OnUpdate(::wxView* sender, wxObject* hint = nullptr) override;

private:
    wxBitEditView() = default;
    wxDECLARE_DYNAMIC_CLASS(wxBitEditView);

    // wx tries to activate this before it is ready
    bool ready = false;
};

/////////////////////////////////////////////////////////////////////////////
