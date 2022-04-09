// VwPrjgsn.h : header file
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

class CGsnProjView : public CView, private CB::Impl::CGsnProjViewBase
{
    DECLARE_DYNCREATE(CGsnProjView)
protected:
    CGsnProjView();         // protected constructor used by dynamic creation

// Attributes
public:
    CGamDoc* GetDocument() { return (CGamDoc*)m_pDocument; }

    // Various controls...
    CProjListBox<decltype(grpDoc)>    m_listProj;         // Main project box

    CEdit           m_editInfo;         // Used for various project info/help
    OwnerOrNullPtr<CTrayListBox> m_listTrays;        // For viewing tray contents

    CButton         m_btnPrjA;          // Project button group
    CButton         m_btnPrjB;
    CButton         m_btnPrjC;

// Operations
public:

// Implementation
protected:
    int             m_nLastSel;         // Previous listbox selection
    int             m_nLastGrp;         // Previous listbox group selection

// Implementation
protected:
    int CreateButton(UINT nCtrlID, CButton& btn, CPoint llpos, CSize relsize);
    BOOL CreateListbox(UINT nCtrlID, CListBox& lbox, DWORD dwStyle, CRect& rct);
    BOOL CreateEditbox(UINT nCtrlID, CEdit& ebox, CRect& rct);

    void SetButtonState(CButton& btn, UINT nStringID);
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
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSelChangeProjList();
    afx_msg void OnDblClkProjList();
    afx_msg void OnClickedProjBtnA();
    afx_msg void OnClickedProjBtnB();
    afx_msg void OnClickedProjBtnC();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnEditBoardProperties();
    afx_msg void OnUpdateEditBoardProperties(CCmdUI* pCmdUI);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnProjItemEdit();
    afx_msg void OnUpdateProjItemEdit(CCmdUI* pCmdUI);
    afx_msg void OnProjItemDelete();
    afx_msg void OnUpdateProjItemDelete(CCmdUI* pCmdUI);
    afx_msg void OnProjItemProperties();
    afx_msg void OnUpdateProjItemProperties(CCmdUI* pCmdUI);
    afx_msg void OnProjItemView();
    afx_msg void OnUpdateProjItemView(CCmdUI* pCmdUI);
    //}}AFX_MSG
    afx_msg LRESULT OnMessageShowPlayingBoard(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMessageRestoreWinState(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif

