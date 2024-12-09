// VwPrjgam.h : header file
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

#ifndef _VWPRJGAM_H
#define _VWPRJGAM_H

/////////////////////////////////////////////////////////////////////////////

#ifndef     _LBOXPROJ_H
#include    "LBoxProj.h"
#endif

#ifndef     _LBOXMARK_H
#include    "LBoxMark.h"
#endif

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

class CGamProjView : public CView, private CB::Impl::CGamProjViewBase
{
    DECLARE_DYNCREATE(CGamProjView)
protected:
    CGamProjView();         // protected constructor used by dynamic creation

// Attributes
public:
    const CGamDoc* GetDocument() const { return CB::ToCGamDoc(m_pDocument); }
    CGamDoc* GetDocument()
    {
        return const_cast<CGamDoc*>(std::as_const(*this).GetDocument());
    }

    // Various controls...
    CProjListBox<decltype(grpDoc)>    m_listProj;         // Main project box

    CEdit           m_editInfo;         // Used for various project info/help

    CButton         m_btnPrjA;          // Project button group
    CButton         m_btnPrjB;

// Operations
public:
    int Find(BoardID bid) const;

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

    void DoGamProperty();
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
    virtual ~CGamProjView();
    virtual void OnInitialUpdate();
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

    virtual void OnDraw(CDC* pDC);      // overridden to draw this view

    // Generated message map functions
protected:
    //{{AFX_MSG(CGamProjView)
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSelChangeProjList();
    afx_msg void OnDblClkProjList();
    afx_msg void OnClickedProjBtnA();
    afx_msg void OnClickedProjBtnB();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnEditBoardProperties();
    afx_msg void OnUpdateEditBoardProperties(CCmdUI* pCmdUI);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnProjItemView();
    afx_msg void OnUpdateProjItemView(CCmdUI* pCmdUI);
    afx_msg void OnProjItemReplay();
    afx_msg void OnUpdateProjItemReplay(CCmdUI* pCmdUI);
    afx_msg void OnProjItemExport();
    afx_msg void OnUpdateProjItemExport(CCmdUI* pCmdUI);
    afx_msg void OnProjItemProperties();
    afx_msg void OnUpdateProjItemProperties(CCmdUI* pCmdUI);
    //}}AFX_MSG
    afx_msg LRESULT OnMessageShowPlayingBoard(WPARAM wParam, LPARAM);
    afx_msg LRESULT OnMessageRestoreWinState(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

#endif

