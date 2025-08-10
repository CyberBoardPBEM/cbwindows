// VwPrjgsn.h : header file
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

#ifndef _VWPRJGSN_H
#define _VWPRJGSN_H

/////////////////////////////////////////////////////////////////////////////

#ifndef     _LBOXPROJ_H
#include    "LBoxProj.h"
#endif

#ifndef     _LBOXMARK_H
#include    "LBoxMark.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CGsnProjView view

/* KLUDGE:  CProjListBox<T> uses Invalid<T>, so Invalid<T>
    must be explicit instantiated before using CProjListBox<T>.
    However, explicit instantiation can't be done in class
    definition.  Therefore, T must be declared in a separate
    class definition.  */
namespace CB { namespace Impl
{
    class CGsnProjViewBase
    {
    public:
        // Project list box grouping order.
        enum { grpDoc, grpBrdHdr, grpBrd, grpTrayHdr, grpTray };
    };

    template<>
    struct Invalid<decltype(CGsnProjViewBase::grpDoc)>
    {
        static constexpr decltype(CGsnProjViewBase::grpDoc) value = static_cast<decltype(CGsnProjViewBase::grpDoc)>(std::numeric_limits<std::underlying_type_t<decltype(CGsnProjViewBase::grpDoc)>>::max());
    };
}}

class CGsnProjViewContainer;

class CProjListBoxGsn : public CProjListBoxWx<decltype(CB::Impl::CGsnProjViewBase::grpDoc)>
{
    wxDECLARE_DYNAMIC_CLASS(CProjListBoxGsn);
};

class CGsnProjView : public CB::ProcessEventOverride<wxPanel>, private CB::Impl::CGsnProjViewBase
{
public:
    CGsnProjView(CGsnProjViewContainer& p);

// Attributes
private:
    CGamDoc& GetDocument() { return *document; }

    // Various controls...
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<CProjListBoxGsn> m_listProj;         // Main project box

        RefPtr<wxTextCtrl> m_editInfo;         // Used for various project info/help
        RefPtr<CTrayListBoxWx> m_listTrays;        // For viewing tray contents

        RefPtr<wxButton> m_btnPrjA;          // Project button group
        RefPtr<wxButton> m_btnPrjB;
        RefPtr<wxButton> m_btnPrjC;
    CB_XRC_END_CTRLS_DECL()

// Operations
public:

// Implementation
protected:
    int             m_nLastSel;         // Previous listbox selection
    int             m_nLastGrp;         // Previous listbox group selection

// Implementation
protected:
#if 0
    int CreateButton(UINT nCtrlID, CButton& btn, CPoint llpos, CSize relsize);
    BOOL CreateListbox(UINT nCtrlID, CListBox& lbox, DWORD dwStyle, CRect& rct);
    BOOL CreateEditbox(UINT nCtrlID, CEdit& ebox, CRect& rct);
#endif

    void SetButtonState(wxButton& btn, UINT nStringID);
    void UpdateButtons(int nGrp = -1);
    void UpdateItemControls(int nGrp = -1);

    void LayoutView();

    // Main document based support routines...
    void DoUpdateProjectList(BOOL bUpdateItem = TRUE);

    void DoGsnProperty();
    void DoUpdateGsnInfo();

    void DoBoardSelection();
    void DoBoardProperty();
    void DoBoardView();
    void DoBoardRemove();

    void DoUpdateBoardHelpInfo();
    void DoUpdateBoardInfo();

    void DoTrayCreate();
    void DoTrayProperty();
    void DoTrayEdit();
    void DoTrayDelete();

    void DoUpdateTrayHelpInfo();
    void DoUpdateTrayList();

// Implementation
protected:
    virtual ~CGsnProjView();
    virtual void OnInitialUpdate();
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

    virtual void OnDraw(CDC* pDC);      // overridden to draw this view

    // Generated message map functions
protected:
    //{{AFX_MSG(CGsnProjView)
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
    void OnClickedProjBtnC(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
#endif
    void OnEditBoardProperties(wxCommandEvent& event);
    void OnUpdateEditBoardProperties(wxUpdateUIEvent& pCmdUI);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnProjItemEdit(wxCommandEvent& event);
    void OnUpdateProjItemEdit(wxUpdateUIEvent& pCmdUI);
    void OnProjItemDelete(wxCommandEvent& event);
    void OnUpdateProjItemDelete(wxUpdateUIEvent& pCmdUI);
    void OnProjItemProperties(wxCommandEvent& event);
    void OnUpdateProjItemProperties(wxUpdateUIEvent& pCmdUI);
    void OnProjItemView(wxCommandEvent& event);
    void OnUpdateProjItemView(wxUpdateUIEvent& pCmdUI);
    void OnMessageShowPlayingBoard(ShowPlayingBoardEvent& event);
    void OnMessageRestoreWinState(WinStateRestoreEvent& event);

    wxDECLARE_EVENT_TABLE();

private:
    // IGetCmdTarget
    CCmdTarget& Get() override;

    RefPtr<CGsnProjViewContainer> parent;
    RefPtr<CGamDoc> document;

    friend class CGsnProjViewContainer;
};

class CGsnProjViewContainer :  public CB::OnCmdMsgOverride<CView>,
                                public CB::wxNativeContainerWindowMixin
{
public:
    void OnDraw(CDC* pDC) override;

    void OnInitialUpdate() override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

private:
    CGsnProjViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CGsnProjViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()

    // IGetEvtHandler
    wxEvtHandler& Get() override
    {
        return CheckedDeref(CheckedDeref(child).GetEventHandler());
    }

    // owned by wx
    CB::propagate_const<CGsnProjView*> child = nullptr;

    typedef CB::OnCmdMsgOverride<CView> BASE;
};

inline CCmdTarget& CGsnProjView::Get()
{
    return *parent;
}

/////////////////////////////////////////////////////////////////////////////

#endif

