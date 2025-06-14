// LBoxSlct.h
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

#ifndef _LBOXSLCT_H
#define _LBOXSLCT_H

#ifndef     _PIECES_H
#include    "Pieces.h"
#endif

#ifndef     _LBOXTILEBASE2_H
#include    "LBoxTileBase2.h"
#endif

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CGamDoc;

/////////////////////////////////////////////////////////////////////////////

class CSelectListBox : public CTileBaseListBox2
{
// Construction
public:
    CSelectListBox()
    {
        m_pDoc = NULL;
    }

// Attributes
public:
    virtual const CTileManager& GetTileManager() const override;

// Operations
public:
    void SetDocument(CGamDoc* pDoc) { CTileBaseListBox2::SetDocument(CheckedDeref(pDoc)); m_pDoc = pDoc; }

// Implementation
protected:
    CGamDoc*    m_pDoc;

    GameElement menuGameElement = Invalid_v<GameElement>;

    // Misc
    // retval[0] is active face, followed by inactives
    std::vector<TileID> GetTileIDs(size_t nIndex) const;

    // Overrides
    virtual CSize OnItemSize(size_t nIndex) const override;
    virtual void OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
        CRect rctItem) const override;
    virtual BOOL OnDragSetup(DragInfo& pDI) const override;

    virtual CB::string OnGetItemDebugString(size_t nItem) const override;

    // Tool tip processing
    virtual BOOL OnIsToolTipsEnabled() const override;
    virtual GameElement OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) const override;
private:
    typedef GameElement (CGamDoc::*GetGameElementCodeForObject_t)(const CDrawObj& pDObj, size_t nSide) const;
    GameElement OnGetHitItemCodeAtPoint(GetGameElementCodeForObject_t func, CPoint point, CRect& rct) const;
public:
    virtual void OnGetTipTextForItemCode(GameElement nItemCode, CB::string& strTip) const override;
    virtual BOOL OnDoesItemHaveTipText(size_t nItem) const override;

    //{{AFX_MSG(CSelectListBox)
    afx_msg LRESULT OnDragItem(WPARAM wParam, LPARAM lParam);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
    afx_msg BOOL OnActTurnOver(UINT id);
    afx_msg void OnUpdateActTurnOver(CCmdUI* pCmdUI);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    const CPlayBoardView& GetBoardView() const;
    CPlayBoardView& GetBoardView()
    {
        return const_cast<CPlayBoardView&>(std::as_const(*this).GetBoardView());
    }
};

#endif

