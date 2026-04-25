// VwPrjgam.h : header file
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

#ifndef _VWPRJGAM_H
#define _VWPRJGAM_H

/////////////////////////////////////////////////////////////////////////////

#ifndef     _LBOXPROJ_H
#include    "LBoxProj.h"
#endif

#ifndef     _LBOXMARK_H
#include    "LBoxMark.h"
#endif

class CGamProjView;
class wxGamProjView;

/////////////////////////////////////////////////////////////////////////////
// CGamProjView view

/* KLUDGE:  CProjListBox<T> uses Invalid<T>, so Invalid<T>
    must be explicit instantiated before using CProjListBox<T>.
    However, explicit instantiation can't be done in class
    definition.  Therefore, T must be declared in a separate
    class definition.  */
namespace CB { namespace Impl
{
    class CGamProjViewBase
    {
    public:
        // Project list box grouping order.
        enum { grpDoc, grpBrdHdr, grpBrd, grpHistHdr, grpCurHist, grpCurPlay,
            grpHistPlay, grpHist };
    };

    template<>
    struct Invalid<decltype(CGamProjViewBase::grpDoc)>
    {
        static constexpr decltype(CGamProjViewBase::grpDoc) value = static_cast<decltype(CGamProjViewBase::grpDoc)>(std::numeric_limits<std::underlying_type_t<decltype(CGamProjViewBase::grpDoc)>>::max());
    };
}}

class CProjListBoxGam : public CProjListBoxWx<decltype(CB::Impl::CGamProjViewBase::grpDoc)>
{
    wxDECLARE_DYNAMIC_CLASS(CProjListBoxGam);
};

class CGamProjView : public CB::ProcessEventOverride<wxPanel>, private CB::Impl::CGamProjViewBase
{
    friend class CGamProjViewContainer;
public:
    CGamProjView(CGamProjViewContainer& p);

// Attributes
    operator const wxView&() const;
    operator wxView&() { return const_cast<wxView&>(static_cast<const wxView&>(std::as_const(*this))); }
    operator const wxView*() const;
    operator wxView*() { return const_cast<wxView*>(static_cast<const wxView*>(std::as_const(*this))); }
    operator const CGamProjViewContainer&() const { return *parent; }
    operator CGamProjViewContainer&()
    {
        return const_cast<CGamProjViewContainer&>(static_cast<const CGamProjViewContainer&>(std::as_const(*this)));
    }

    CFrameWnd* GetParentFrame();
private:
    const CGamDoc& GetDocument() const { return *document; }
    CGamDoc& GetDocument()
    {
        return const_cast<CGamDoc&>(std::as_const(*this).GetDocument());
    }

    // Various controls...
    CB_XRC_BEGIN_CTRLS_DECL()
public:
        RefPtr<CProjListBoxGam> m_listProj;         // Main project box
private:

        RefPtr<wxTextCtrl> m_editInfo;         // Used for various project info/help

        RefPtr<wxButton> m_btnPrjA;          // Project button group
        RefPtr<wxButton> m_btnPrjB;
    CB_XRC_END_CTRLS_DECL()

// Operations
public:
    size_t Find(BoardID bid) const;

// Implementation
protected:
    int             m_nLastSel;         // Previous listbox selection
    decltype(grpDoc) m_nLastGrp;         // Previous listbox group selection

// Implementation
protected:
#if 0
    int CreateButton(UINT nCtrlID, CButton& btn, CPoint llpos, CSize relsize);
    BOOL CreateListbox(UINT nCtrlID, CListBox& lbox, DWORD dwStyle, CRect& rct);
    BOOL CreateEditbox(UINT nCtrlID, CEdit& ebox, CRect& rct);
#endif

    void SetButtonState(wxButton& btn, UINT nStringID) const;
    void UpdateButtons(decltype(grpDoc) nGrp = Invalid_v<decltype(grpDoc)>);
    void UpdateItemControls(decltype(grpDoc) nGrp = Invalid_v<decltype(grpDoc)>);

#if 0
    void LayoutView();
#endif

    // Main document based support routines...
    void DoUpdateProjectList(BOOL bUpdateItem = TRUE);

    void DoGamProperty() const;
    void DoUpdateGamInfo();

    void DoBoardProperty();
    void DoBoardView();

    void DoUpdateBoardHelpInfo();
    void DoUpdateBoardInfo();

    void DoHistorySave();
    void DoHistoryLoad();
    void DoHistoryDone();
    void DoHistoryDiscard();
    void DoHistoryReplay();
    void DoHistoryReplayDone();
    void DoHistoryExport();

    void DoUpdateHistoryHelpInfo();
    void DoUpdateCurPlayInfo();
    void DoUpdateHistoryInfo();

// Implementation
protected:
    ~CGamProjView() override;
    void OnInitialUpdate();
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

#if 0
    void OnDraw(CDC* pDC) override;      // overridden to draw this view
#endif

protected:
#if 0
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
#endif
    void OnSelChangeProjList(wxCommandEvent& event);
    void OnSelChangeProjList()
    {
        wxCommandEvent dummy;
        OnSelChangeProjList(dummy);
    }
    void OnDblClkProjList(wxCommandEvent& event);
    void OnClickedProjBtnA(wxCommandEvent& event);
    void OnClickedProjBtnB(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
#endif
    void OnEditBoardProperties(wxCommandEvent& event);
    void OnUpdateEditBoardProperties(wxUpdateUIEvent& pCmdUI);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnProjItemView(wxCommandEvent& event);
    void OnUpdateProjItemView(wxUpdateUIEvent& pCmdUI);
    void OnProjItemReplay(wxCommandEvent& event);
    void OnUpdateProjItemReplay(wxUpdateUIEvent& pCmdUI);
    void OnProjItemExport(wxCommandEvent& event);
    void OnUpdateProjItemExport(wxUpdateUIEvent& pCmdUI);
    void OnProjItemProperties(wxCommandEvent& event);
    void OnUpdateProjItemProperties(wxUpdateUIEvent& pCmdUI);
    void OnMessageShowPlayingBoard(ShowPlayingBoardEvent& event);
    void OnMessageRestoreWinState(WinStateRestoreEvent& event);

    wxDECLARE_EVENT_TABLE();

private:
    // IGetCmdTarget
    CCmdTarget& Get() override;

    RefPtr<CGamProjViewContainer> parent;
    RefPtr<CGamDoc> document;

    OwnerPtr<wxGamProjView> wxview;
};

class CGamProjViewContainer :  public CB::OnCmdMsgOverride<CView>,
                                public CB::NativeContainerWindowMixin
{
public:
    operator const CGamProjView&() const { return *child; }
    operator CGamProjView&()
    {
        return const_cast<CGamProjView&>(static_cast<const CGamProjView&>(std::as_const(*this)));
    }

    void OnDraw(CDC* pDC) override;

    void OnInitialUpdate() override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

private:
    CGamProjViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CGamProjViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
#if 0
    afx_msg void OnSize(UINT nType, int cx, int cy);
#endif
    DECLARE_MESSAGE_MAP()

    // IGetEvtHandler
    wxEvtHandler& Get() override
    {
        return CheckedDeref(CheckedDeref(child).GetEventHandler());
    }

    // owned by wx
    CB::propagate_const<CGamProjView*> child = nullptr;

    typedef CB::OnCmdMsgOverride<CView> BASE;
};

class wxGamProjView : public CB::View
{
public:
    const CGamProjView& GetWindow() const { return DoGetWindow(); }
    CGamProjView& GetWindow()
    {
        return const_cast<CGamProjView&>(std::as_const(*this).GetWindow());
    }
    operator const CGamProjView&() const { return GetWindow(); }
    operator CGamProjView&()
    {
        return const_cast<CGamProjView&>(static_cast<const CGamProjView&>(std::as_const(*this)));
    }

protected:
    const CGamProjView& DoGetWindow() const override;

private:
    wxGamProjView(CGamProjView& v) : window(&v) {}

    RefPtr<CGamProjView> window;

    friend CGamProjView;
};

inline CGamProjView::operator const wxView&() const
{
    return *wxview;
}

inline CGamProjView::operator const wxView*() const
{
    return &*wxview;
}

inline CCmdTarget& CGamProjView::Get()
{
    return *parent;
}


/////////////////////////////////////////////////////////////////////////////

#endif

