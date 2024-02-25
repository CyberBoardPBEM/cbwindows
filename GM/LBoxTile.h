// LBoxTile.h
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

#ifndef _LBOXTILE_H
#define _LBOXTILE_H

#ifndef     _LBOXGRFX_H
#include    "LBoxGrfx.h"
#endif

#ifndef     _TILE_H
#include    "Tile.h"
#endif

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CGamDoc;

/////////////////////////////////////////////////////////////////////////////

class CTileListBox : public CGrafixListBoxData<CGrafixListBox, TileID>
{
// Construction
public:
    CTileListBox();

// Attributes
public:
    void SetDrawAllScales(BOOL bDrawAll) { m_bDrawAllScales = bDrawAll; }
    BOOL GetDrawAllScaled() { return m_bDrawAllScales; }

// Operations
public:
    void SetDocument(const CGamDoc* pDoc) { m_pDoc = pDoc; }

// Implementation
protected:
    const CGamDoc* m_pDoc;
    BOOL        m_bDrawAllScales;
    int         m_bDisplayIDs;     // Set to prop [Settings]:DisplayIDs

    // Overrides
    virtual CSize OnItemSize(size_t nIndex) const override;
    virtual void OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
        CRect rctItem) const override;
    virtual BOOL OnDragSetup(DragInfo& pDI) const override;

    //{{AFX_MSG(CTileListBox)
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

class CTileListBoxWx : public CGrafixListBoxDataWx<CGrafixListBoxWx, TileID>
{
// Construction
public:
    CTileListBoxWx();

// Attributes
public:
    void SetDrawAllScales(BOOL bDrawAll) { m_bDrawAllScales = bDrawAll; }
    BOOL GetDrawAllScaled() { return m_bDrawAllScales; }

// Operations
public:
    void SetDocument(const CGamDoc* pDoc) { m_pDoc = pDoc; }

// Implementation
protected:
    const CGamDoc* m_pDoc;
    BOOL        m_bDrawAllScales;
    int         m_bDisplayIDs;     // Set to prop [Settings]:DisplayIDs

    // Overrides
    wxSize GetItemSize(size_t nIndex) const override;
    void OnDrawItem(wxDC& pDC, const wxRect& rctItem, size_t nIndex) const override;
#if 0
    virtual BOOL OnDragSetup(DragInfo& pDI) const override;
#endif

private:
    wxDECLARE_DYNAMIC_CLASS(CTileListBoxWx);
};

#endif

