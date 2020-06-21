// VwTilesl.h : header file
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

#ifndef _VWTILESL_H
#define _VWTILESL_H

#ifndef     _TILE_H
#include    "Tile.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CTileSelView view

class CBitEditView;

class CTileSelView : public CScrollView
{
    friend class CBitEditFrame;
    DECLARE_DYNCREATE(CTileSelView)
protected:
    CTileSelView();         // protected constructor used by dynamic creation

// Attributes
public:
    CGamDoc* GetDocument() { return (CGamDoc*) m_pDocument; }

    void SetBitEditor(CBitEditView *pEditView);
    TileScale GetCurrentScale() { return m_eCurTile; }
    void SetNoUpdate(BOOL bNoUpdate = TRUE) { m_bNoUpdate = bNoUpdate; }
    TileID GetTileID() { return m_tid; }

// Operations
public:
    void UpdateViewPixel(CPoint pt, UINT nBrushSize, CBrush *pBrush);
    void UpdateViewImage(CRect* pRct, BOOL bImmed = FALSE);
    // ------------- //
    void DoTileResizeDialog();
    void DoTileRotation(int nAngle);
    void UpdateDocumentTiles();
    // ------------- //
    BOOL IsUndoAvailable();
    void PurgeUndo();
    void RestoreFromUndo();

// Implementation
protected:
    BOOL        m_bNoUpdate;        // TRUE if should discard changes

    TileID      m_tid;

    CBitmap*    m_pBmFullUndo;      // Save from previous resize
    CBitmap*    m_pBmHalfUndo;      // Save from previous resize

    CBitmap     m_bmFull;
    CBitmap     m_bmHalf;
    CBitmap     m_bmSmall;

    COLORREF    m_crSmall;

    CSize       m_sizeFull;
    CSize       m_sizeHalf;
    CSize       m_sizeSmall;

    CRect       m_rctFull;
    CRect       m_rctHalf;
    CRect       m_rctSmall;

    CSize       m_sizeSelArea;          // Size of selection area.

    TileScale   m_eCurTile;

    CBitEditView* m_pEditView;
    CTileManager* m_pTileMgr;

    // ------- //
    CRect GetActiveTileRect();
    CPoint GetActiveTileLoc();
    CBitmap* GetActiveBitmap();
    void DrawTile(CDC* pDC, CBitmap* pBMap, CRect rct);
    void SelectCurrentBitmap(TileScale eScale);
    void CalcViewLayout();
    // ------- //
    virtual ~CTileSelView();
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();
    virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

    // Generated message map functions
protected:
    //{{AFX_MSG(CTileSelView)
    afx_msg LRESULT OnSetColor(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetCustomColors(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetLineWidth(WPARAM wParam, LPARAM lParam);
    afx_msg void OnUpdateColorForeground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorBackground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorTransparent(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorCustom(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLineWidth(CCmdUI* pCmdUI);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnDestroy();
    afx_msg void OnUpdateToolPalette(CCmdUI* pCmdUI);
    afx_msg BOOL OnToolPalette(UINT id);
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
    afx_msg void OnEditUndo();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnViewToggleScale();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif

