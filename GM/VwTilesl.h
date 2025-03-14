// VwTilesl.h : header file
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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
class wxBitEditView;

class CTileSelView : public wxScrolledCanvas
{
    friend class CBitEditFrame;
protected:
    // single-part construction
    CTileSelView(wxSplitterWindow& parent,
                    wxBitEditView& view,
                    TileID tid);

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

    const TileID      m_tid;

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
    RefPtr<wxSplitterWindow> parent;
    RefPtr<CGamDoc> document;
    RefPtr<wxBitEditView> view;
};

/////////////////////////////////////////////////////////////////////////////
#endif

