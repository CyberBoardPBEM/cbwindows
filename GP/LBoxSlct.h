// LBoxSlct.h
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
    void SetDocument(CGamDoc& pDoc) { CTileBaseListBox2::SetDocument(pDoc); m_pDoc = &pDoc; }

// Implementation
protected:
    CB::propagate_const<CGamDoc*> m_pDoc;

    GameElement menuGameElement = Invalid_v<GameElement>;

    // Misc
    // retval[0] is active face, followed by inactives
    std::vector<TileID> GetTileIDs(size_t nIndex) const;

    // Overrides
    wxSize GetItemSize(size_t nIndex) const override;
    void OnDrawItem(wxDC& pDC, const wxRect& rctItem, size_t nIndex) const override;
    virtual BOOL OnDragSetup(DragInfoWx& pDI) const override;

    virtual CB::string OnGetItemDebugString(size_t nItem) const override;

    // Tool tip processing
    virtual BOOL OnIsToolTipsEnabled() const override;
    virtual GameElement OnGetHitItemCodeAtPoint(wxPoint point, wxRect& rct) const override;
private:
    typedef GameElement (CGamDoc::*GetGameElementCodeForObject_t)(const CDrawObj& pDObj, size_t nSide) const;
    GameElement OnGetHitItemCodeAtPoint(GetGameElementCodeForObject_t func, wxPoint point, wxRect& rct) const;
public:
    virtual CB::string OnGetTipTextForItemCode(GameElement nItemCode) const override;
    virtual BOOL OnDoesItemHaveTipText(size_t nItem) const override;

    void OnDragItem(DragDropEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnInitMenuPopup(wxMenuEvent& event);
    void OnActTurnOver(wxCommandEvent& event);
    void OnUpdateActTurnOver(wxUpdateUIEvent& pCmdUI);
    wxDECLARE_EVENT_TABLE();

private:
    const CPlayBoardView& GetBoardView() const;
    CPlayBoardView& GetBoardView()
    {
        return const_cast<CPlayBoardView&>(std::as_const(*this).GetBoardView());
    }

    wxDECLARE_DYNAMIC_CLASS(CSelectListBox);
};

#endif
