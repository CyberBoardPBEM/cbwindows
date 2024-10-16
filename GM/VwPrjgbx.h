// VwPrjgbx.h : header file
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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

#ifndef _VWPRJGBX_H
#define _VWPRJGBX_H

/////////////////////////////////////////////////////////////////////////////

#ifndef     _LBOXPROJ_H
#include    "LBoxProj.h"
#endif

#ifndef     _LBOXTILE_H
#include    "LBoxTile.h"
#endif

#ifndef     _LBOXPIEC_H
#include    "LBoxPiec.h"
#endif

#ifndef     _LBOXMARK_H
#include    "LBoxMark.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CGbxProjView view

/* KLUDGE:  CProjListBox<T> uses Invalid<T>, so Invalid<T>
    must be explicit instantiated before using CProjListBox<T>.
    However, explicit instantiation can't be done in class
    definition.  Therefore, T must be declared in a separate
    class definition.  */
namespace CB { namespace Impl
{
    class CGbxProjViewBase
    {
    public:
        // Project list box grouping order.
        enum { grpDoc, grpBHdr, grpBrd, grpTHdr, grpTile, grpPHdr,
            grpPce, grpMHdr, grpMark };
    };

    template<>
    struct Invalid<decltype(CGbxProjViewBase::grpDoc)>
    {
        static constexpr decltype(CGbxProjViewBase::grpDoc) value = static_cast<decltype(CGbxProjViewBase::grpDoc)>(std::numeric_limits<std::underlying_type_t<decltype(CGbxProjViewBase::grpDoc)>>::max());
    };
}}

class CProjListBoxGm : public CProjListBoxWx<decltype(CB::Impl::CGbxProjViewBase::grpDoc)>
{
    wxDECLARE_DYNAMIC_CLASS(CProjListBoxGm);
};

class CGbxProjViewContainer;

class CGbxProjView : public wxPanel, private CB::Impl::CGbxProjViewBase
{
protected:
    CGbxProjView(CGbxProjViewContainer& p);

// Attributes
public:
    const CGamDoc& GetDocument() const { return *document; }
    CGamDoc& GetDocument() { return const_cast<CGamDoc&>(std::as_const(*this).GetDocument()); }

private:
    CB_XRC_BEGIN_CTRLS_DECL()
        // Various controls...
        RefPtr<CProjListBoxGm>  m_listProj;         // Main project box

        RefPtr<wxTextCtrl>      m_editInfo;         // Used for various project info/help
        RefPtr<CTileListBoxWx>  m_listTiles;        // For viewing tiles
        RefPtr<CPieceListBoxWx> m_listPieces;       // For viewing pieces
        RefPtr<CMarkListBoxWx>  m_listMarks;        // For viewing marker graphics

        RefPtr<wxButton>        m_btnPrjA;          // Project button group
        RefPtr<wxButton>        m_btnPrjB;

        RefPtr<wxButton>        m_btnItmA;          // Item button group
        RefPtr<wxButton>        m_btnItmB;
        RefPtr<wxButton>        m_btnItmC;
        RefPtr<wxButton>        m_btnItmD;
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

#if 0
    void LayoutView();
#endif

    // Main document based support routines...

    void DoUpdateProjectList(BOOL bUpdateItem = TRUE);

    void DoUpdateGbxInfo();
    void DoGbxProperty();

    void DoBoardCreate();
    void DoBoardProperty();
    void DoBoardDelete();
    void DoBoardEdit();
    void DoBoardClone();
    void DoUpdateBoardHelpInfo();
    void DoUpdateBoardInfo();

    void DoTileManagerProperty();
    void DoTileGroupCreate();
    void DoTileGroupProperty();
    void DoTileGroupDelete();
    void DoTileNew();
    void DoTileEdit();
    void DoTileClone();
    void DoTileDelete();
    void DoUpdateTileHelpInfo();
    void DoUpdateTileList();

    void DoPieceGroupCreate();
    void DoPieceGroupProperty();
    void DoPieceGroupDelete();
    void DoPieceNew();
    void DoPieceEdit();
    void DoPieceDelete();
    void DoUpdatePieceHelpInfo();
    void DoUpdatePieceList();

    void DoMarkGroupCreate();
    void DoMarkGroupProperty();
    void DoMarkGroupDelete();
    void DoMarkNew();
    void DoMarkEdit();
    void DoMarkDelete();
    void DoUpdateMarkHelpInfo();
    void DoUpdateMarkList();

// Implementation
protected:
    ~CGbxProjView() override;
    void OnInitialUpdate();
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

#if 0
    void OnDraw(CDC* pDC) override;      // overridden to draw this view
#endif

    // Generated message map functions
protected:
#if 0
    //{{AFX_MSG(CGbxProjView)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
#endif
    void OnSelChangeProjList(wxCommandEvent& /*event*/);
    void OnSelChangeProjList()
    {
        wxCommandEvent dummy;
        OnSelChangeProjList(dummy);
    }
    void OnDblClkProjList(wxCommandEvent& event);
    void OnDblClkTileList(wxCommandEvent& event);
    void OnDblClkPieceList(wxCommandEvent& event);
    void OnDblClkMarkList(wxCommandEvent& event);
    void OnClickedProjBtnA(wxCommandEvent& event);
    void OnClickedProjBtnB(wxCommandEvent& event);
    void OnClickedItemBtnA(wxCommandEvent& event);
    void OnClickedItemBtnB(wxCommandEvent& event);
    void OnClickedItemBtnC(wxCommandEvent& event);
    void OnClickedItemBtnD(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
#endif
    void OnEditCopy(wxCommandEvent& event);
    void OnUpdateEditCopy(wxUpdateUIEvent& pCmdUI);
    void OnEditPaste(wxCommandEvent& event);
    void OnUpdateEditPaste(wxUpdateUIEvent& pCmdUI);
    void OnEditMove(wxCommandEvent& event);
    void OnUpdateEditMove(wxUpdateUIEvent& pCmdUI);
    void OnProjectSaveTileFile(wxCommandEvent& event);
    void OnUpdateProjectSaveTileFile(wxUpdateUIEvent& pCmdUI);
    void OnProjectLoadTileFile(wxCommandEvent& event);
    void OnUpdateProjectLoadTileFile(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnProjItemProperties();
    afx_msg void OnUpdateProjItemProperties(CCmdUI* pCmdUI);
    afx_msg void OnTileClone();
    afx_msg void OnUpdateTileClone(CCmdUI* pCmdUI);
    afx_msg void OnTileDelete();
    afx_msg void OnUpdateTileDelete(CCmdUI* pCmdUI);
    afx_msg void OnTileEdit();
    afx_msg void OnUpdateTileEdit(CCmdUI* pCmdUI);
    afx_msg void OnTileNew();
    afx_msg void OnProjItemDelete();
    afx_msg void OnUpdateProjItemDelete(CCmdUI* pCmdUI);
    afx_msg void OnPieceNew();
    afx_msg void OnPieceEdit();
    afx_msg void OnUpdatePieceEdit(CCmdUI* pCmdUI);
    afx_msg void OnPieceDelete();
    afx_msg void OnUpdatePieceDelete(CCmdUI* pCmdUI);
    afx_msg void OnMarkerNew();
    afx_msg void OnMarkerEdit();
    afx_msg void OnUpdateMarkerEdit(CCmdUI* pCmdUI);
    afx_msg void OnMarkerDelete();
    afx_msg void OnUpdateMarkerDelete(CCmdUI* pCmdUI);
    afx_msg void OnProjectCloneBoard();
    afx_msg void OnUpdateProjectCloneBoard(CCmdUI* pCmdUI);
    //}}AFX_MSG
    afx_msg LRESULT OnDragItem(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetDragSize(WPARAM wParam, LPARAM lParam);
#endif
    wxDECLARE_EVENT_TABLE();

private:
    RefPtr<CGbxProjViewContainer> parent;
    RefPtr<CGamDoc> document;

    friend class CGbxProjViewContainer;
};

class CGbxProjViewContainer : public CB::OnCmdMsgOverride<CView>,
                                public CB::wxNativeContainerWindowMixin
{
public:
    const CGbxProjView& GetChild() const { return CheckedDeref(child); }
    CGbxProjView& GetChild()
    {
        return const_cast<CGbxProjView&>(std::as_const(*this).GetChild());
    }
    void OnDraw(CDC* pDC) override;
    void OnInitialUpdate() override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

private:
    CGbxProjViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CGbxProjViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    DECLARE_MESSAGE_MAP()

    // IGetEvtHandler
    wxEvtHandler& Get() override
    {
        return CheckedDeref(CheckedDeref(child).GetEventHandler());
    }

    // owned by wx
    CB::propagate_const<CGbxProjView*> child = nullptr;

    friend CGbxProjView;
};

/////////////////////////////////////////////////////////////////////////////

#endif

