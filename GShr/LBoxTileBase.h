// LBoxTileBase.h - class used to handle a variety of tile
//      oriented listbox functions
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

#ifndef _LBOXTILEBASE_H
#define _LBOXTILEBASE_H

#ifndef     _LBOXGRFX_H
#include    "LBoxGrfx.h"
#endif

#ifndef     _TILE_H
#include    "Tile.h"
#endif

///////////////////////////////////////////////////////////////////////

class CTileBaseListBox : public CGrafixListBox
{
public:
    CTileBaseListBox();

    void SetTipMarkVisibility(BOOL bShow = TRUE) { m_bTipMarkItems = bShow; }
    BOOL GetTipMarkVisibility() const { return m_bTipMarkItems; }

// Vars...
protected:
    int         m_bDisplayIDs;              // Set by property [Settings]:DisplayIDs

    BOOL        m_bTipMarkItems;
    CB::string  m_strTipMark;
    CSize       m_sizeTipMark;

// Helpers...
protected:
    void DrawTileImage(CDC& pDC, CRect rctItem, BOOL bDrawIt, int& x, TileID tid) const;
    void DrawItemDebugIDCode(CDC& pDC, size_t nItem, CRect rctItem, BOOL bDrawIt, int& x) const;

    void SetupTipMarkerIfRequired();
    void DrawTipMarker(CDC& pDC, CRect rctItem, BOOL bVisible, int& x) const;

    CSize DoOnItemSize(size_t nItem, const std::vector<TileID>& tids) const;
    void DoOnDrawItem(CDC& pDC, size_t nItem, UINT nAction, UINT nState, CRect rctItem,
        const std::vector<TileID>& tids) const;

    std::vector<CRect> GetTileRectsForItem(size_t nItem, const std::vector<TileID>& tids) const;

// Overrides...
public:
    virtual const CTileManager& GetTileManager() const /* override */ = 0;

// Overrides...
protected:
    virtual BOOL OnDoesItemHaveTipText(size_t nItem) const /* override */ { return FALSE; }

    // see CGrafixListBox::OnGetItemDebugIDCode comment
    virtual int  OnGetItemDebugIDCode(size_t nItem) const override = 0;
    virtual CB::string OnGetItemDebugString(size_t nItem) const /* override */;

    //{{AFX_MSG(CTileBaseListBox)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


#endif
