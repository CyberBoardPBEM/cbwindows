// PalTile.h - Tile palette window
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

#ifndef _PALTILE_H
#define _PALTILE_H

#include    "FrmDockTile.h"
#ifndef     _LBOXTILE_H
#include    "LBoxTile.h"
#endif

/////////////////////////////////////////////////////////////////////////////

class CGamDoc;

/////////////////////////////////////////////////////////////////////////////
// CTilePalette window - Tile palette's are part of the document object.

class CTilePalette : public wxPanel
{
// Construction
public:
    CTilePalette(const CGamDoc& pDoc, wxWindow& pOwnerWnd);

// Attributes
public:
    TileID GetCurrentTileID() const;

// Operations
public:
    void UpdatePaletteContents();

    void Serialize(CArchive &ar);

// Implementation
protected:
    const CGamDoc& m_pDoc;
    CDockTilePalette* m_pDockingFrame;

    // Enclosed controls....
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxChoice> m_comboTGrp;
        RefPtr<CTileListBoxWx> m_listTile;
    CB_XRC_END_CTRLS_DECL()

    void LoadTileNameList();
    void UpdateTileList();

// Implementation
public:
    CDockTilePalette* GetDockingFrame() const { return m_pDockingFrame; }
    void SetDockingFrame(CDockTilePalette* pDockingFrame)
    {
        m_pDockingFrame = pDockingFrame;
        Reparent(pDockingFrame ? static_cast<wxWindow*>(*pDockingFrame) : nullptr);
    }

protected:
    void OnTileNameCbnSelchange(wxCommandEvent& event);
    void OnGetDragSize(GetDragSizeEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
#endif
    void OnPaletteHide(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};

/////////////////////////////////////////////////////////////////////////////

#endif

