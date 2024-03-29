// PalTile.h - Tile palette window
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

#ifndef _PALTILE_H
#define _PALTILE_H

#ifndef     _LBOXTILE_H
#include    "LBoxTile.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CGamDoc;

/////////////////////////////////////////////////////////////////////////////
// CTilePalette window - Tile palette's are part of the document object.

class CTilePalette : public CWnd
{
    DECLARE_DYNCREATE(CTilePalette)
// Construction
public:
    CTilePalette();
    BOOL Create(CWnd* pOwnerWnd, DWORD dwStyle = 0, UINT nID = 0);

// Attributes
public:
    void SetDocument(CGamDoc *pDoc);
    TileID GetCurrentTileID();

// Operations
public:
    void UpdatePaletteContents();

    void Serialize(CArchive &ar);

// Implementation
protected:
    CGamDoc*    m_pDoc;
    CDockablePane* m_pDockingFrame;

    // Enclosed controls....
    CComboBox   m_comboTGrp;
    CTileListBox m_listTile;
    int         m_nComboHeight;

    void LoadTileNameList();
    void UpdateTileList();

// Implementation
public:
    virtual void PostNcDestroy();

    CDockablePane* GetDockingFrame() { return m_pDockingFrame; }
    void SetDockingFrame(CDockablePane* pDockingFrame)
    {
        m_pDockingFrame = pDockingFrame;
        SetParent(pDockingFrame);
    }

    // Generated message map functions
protected:
    //{{AFX_MSG(CTilePalette)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTileNameCbnSelchange();
    afx_msg LRESULT OnGetDragSize(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    afx_msg LRESULT OnPaletteHide(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif

