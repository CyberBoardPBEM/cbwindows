// VwTilesl.h : header file
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

#ifndef _VWTILESL_H
#define _VWTILESL_H

#ifndef     _TILE_H
#include    "Tile.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CTileSelView view

class CBitEditView;
class CTileSelViewContainer;

class CTileSelView : public wxScrolledCanvas
{
    friend class CBitEditFrame;
    friend class CTileSelViewContainer;
protected:
    // single-part construction
    CTileSelView(CTileSelViewContainer& parent);

// Attributes
public:
    CGamDoc& GetDocument() { return *document; }

    void SetBitEditor(CBitEditView& pEditView);
    CBitEditView& GetBitEditor() { return CheckedDeref(m_pEditView); }
    TileScale GetCurrentScale() const { return m_eCurTile; }
    void SetNoUpdate(BOOL bNoUpdate = TRUE) { m_bNoUpdate = bNoUpdate; }
    TileID GetTileID() const { return m_tid; }

// Operations
public:
    void UpdateViewPixel(wxPoint pt, UINT nBrushSize, const wxBrush& pBrush);
    void UpdateViewImage();
    // ------------- //
    void DoTileResizeDialog();
    void DoTileRotation(int nAngle);
    void UpdateDocumentTiles();
    // ------------- //
    BOOL IsUndoAvailable() const;
    void PurgeUndo();
    void RestoreFromUndo();

// Implementation
protected:
    BOOL        m_bNoUpdate;        // TRUE if should discard changes

    TileID      m_tid;

    wxBitmap m_pBmFullUndo;      // Save from previous resize
    wxBitmap m_pBmHalfUndo;      // Save from previous resize

    wxBitmap m_bmFull;
    wxBitmap m_bmHalf;
    wxBitmap m_bmSmall;

    wxColour    m_crSmall;

    wxSize      m_sizeFull;
    wxSize      m_sizeHalf;
    wxSize      m_sizeSmall;

    wxRect      m_rctFull;
    wxRect      m_rctHalf;
    wxRect      m_rctSmall;

    wxSize      m_sizeSelArea;          // Size of selection area.

    TileScale   m_eCurTile;

    CBitEditView* m_pEditView;
    CTileManager* m_pTileMgr;

    // ------- //
    wxRect GetActiveTileRect() const;
    wxPoint GetActiveTileLoc() const;
    wxBitmap& GetActiveBitmap();
    static void DrawTile(wxDC& pDC, const wxBitmap& pBMap, wxRect rct);
    void SelectCurrentBitmap(TileScale eScale);
    void CalcViewLayout();
    // ------- //
    ~CTileSelView() override;
    void OnDraw(wxDC& dc) override;

public:
    void OnInitialUpdate();
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

protected:
    void OnLButtonDown(wxMouseEvent& event);
    wxDECLARE_EVENT_TABLE();

private:
    RefPtr<CTileSelViewContainer> parent;
    RefPtr<CGamDoc> document;
};

class CTileSelViewContainer : public CView,
                                public CB::wxNativeContainerWindowMixin
{
public:
    const CTileSelView& GetChild() const { return CheckedDeref(child); }
    CTileSelView& GetChild()
    {
        return const_cast<CTileSelView&>(std::as_const(*this).GetChild());
    }
    void OnDraw(CDC* pDC) override;
    void OnInitialUpdate() override;

protected:
    void OnActivateView(BOOL bActivate,
                        CView* pActivateView,
                        CView* pDeactiveView) override;
    void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

private:
    CTileSelViewContainer();         // used by dynamic creation
    DECLARE_DYNCREATE(CTileSelViewContainer)

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnSetColor(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetCustomColors(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSetLineWidth(WPARAM wParam, LPARAM lParam);
    afx_msg void OnUpdateColorForeground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorBackground(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorTransparent(CCmdUI* pCmdUI);
    afx_msg void OnUpdateColorCustom(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLineWidth(CCmdUI* pCmdUI);
    afx_msg void OnUpdateToolPalette(CCmdUI* pCmdUI);
    afx_msg BOOL OnToolPalette(UINT id);
    afx_msg void OnEditPaste();
    afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
    afx_msg void OnEditUndo();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
    afx_msg void OnViewToggleScale();
    DECLARE_MESSAGE_MAP()

    // owned by wx
    CB::propagate_const<CTileSelView*> child = nullptr;

    friend CTileSelView;
};

/////////////////////////////////////////////////////////////////////////////
#endif

