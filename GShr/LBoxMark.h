// LBoxMark.h
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

#ifndef _LBOXMARK_H
#define _LBOXMARK_H

#ifndef     _LBOXTILEBASE_H
#include    "LBoxTileBase.h"
#endif

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

#ifndef     _MARKS_H
#include    "Marks.h"
#endif

#include    "LBoxVHScrl.h"

/////////////////////////////////////////////////////////////////////////////

class CGamDoc;
enum  MarkerTrayViz;

/////////////////////////////////////////////////////////////////////////////

class CMarkListBox : public CGrafixListBoxData<CTileBaseListBox, MarkID>
{
// Construction
public:
    CMarkListBox();

// Attributes
public:
    void SetTrayContentVisibility(MarkerTrayViz eTrayViz, const CB::string& pszHiddenString = CB::string())
    {
        m_eTrayViz = eTrayViz;
        m_strHiddenString = pszHiddenString;
    }

    virtual const CTileManager& GetTileManager() const override;

// Operations
public:
    void SetDocument(CGamDoc* pDoc)
    {
        CGrafixListBoxData::SetDocument(CheckedDeref(pDoc));
        if (m_hWnd != NULL)
            ResetContent();
        m_pDoc = pDoc;
    }

    void SelectMarker(MarkID mid);
    void ShowListIndex(int nPos);

// Implementation
protected:
    CGamDoc*        m_pDoc;
    MarkerTrayViz   m_eTrayViz;
    CB::string         m_strHiddenString;

    // Overrides
    virtual CSize OnItemSize(size_t nIndex) const override;
    virtual void OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
        CRect rctItem) const override;
    virtual BOOL OnDragSetup(DragInfo& pDI) const override;

    // Tool tip processing
    virtual BOOL OnIsToolTipsEnabled() const override;
    virtual GameElement OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) const override;
    virtual void OnGetTipTextForItemCode(GameElement nItemCode, CB::string& strTip) const override;
    virtual BOOL OnDoesItemHaveTipText(size_t nItem) const override;

    //{{AFX_MSG(CMarkListBox)
//  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

class CMarkListBoxWx : public CB::VListBoxHScroll//CGrafixListBoxData<CTileBaseListBox, MarkID>
{
// Construction
public:
    CMarkListBoxWx();

#if 1   // temp stubs
    void SetDocument(CGamDoc* pDoc) { SetItemCount(20); }
    void SetItemMap(const std::vector<MarkID>* pMap, BOOL bKeepPosition = TRUE) {}
#endif

#if 0
// Attributes
public:
    void SetTrayContentVisibility(MarkerTrayViz eTrayViz, const CB::string& pszHiddenString = CB::string())
    {
        m_eTrayViz = eTrayViz;
        m_strHiddenString = pszHiddenString;
    }

    virtual const CTileManager& GetTileManager() const override;

// Operations
public:
    void SetDocument(CGamDoc* pDoc)
    {
        CGrafixListBoxData::SetDocument(CheckedDeref(pDoc));
        if (m_hWnd != NULL)
            ResetContent();
        m_pDoc = pDoc;
    }

    void SelectMarker(MarkID mid);
    void ShowListIndex(int nPos);
#endif

// Implementation
protected:
#if 0
    CGamDoc*        m_pDoc;
    MarkerTrayViz   m_eTrayViz;
    CB::string         m_strHiddenString;
#endif

    // Overrides
    wxSize GetItemSize(size_t nIndex) const override;
    void OnDrawItem(wxDC& pDC, const wxRect& rctItem, size_t nIndex) const override;
#if 0
    virtual BOOL OnDragSetup(DragInfo& pDI) const override;

    // Tool tip processing
    virtual BOOL OnIsToolTipsEnabled() const override;
    virtual GameElement OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) const override;
    virtual void OnGetTipTextForItemCode(GameElement nItemCode, CB::string& strTip) const override;
    virtual BOOL OnDoesItemHaveTipText(size_t nItem) const override;

    //{{AFX_MSG(CMarkListBox)
//  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
#endif
private:
    wxDECLARE_DYNAMIC_CLASS(CMarkListBoxWx);
};

#endif

