// ResTbl.h
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

#ifndef _RESTBL_H
#define _RESTBL_H

struct ResourceTable
{
    BOOL    m_bInited;
    // ======== Screen Fonts ========= //
    HFONT   h8ss;               // 8pt Sans Serif
    wxFont  h8ssWx;
    HFONT   h8ssb;              // 8pt Sans Serif Bold
    HFONT   h10ss;              // 10pt Sans Serif
    // Text metrics for associated screen fonts
    TEXTMETRIC  tm8ss;
    TEXTMETRIC  tm8ssb;
    TEXTMETRIC  tm10ss;
    // ======== Cursors ========= //
    HCURSOR     hcrDragTile;
    wxCursor    hcrDragTileWx;
    HCURSOR     hcrNoDrop;
    wxCursor    hcrNoDropWx;
    HCURSOR     hcrNoDropTooBig;
    wxCursor    hcrNoDropTooBigWx;
    HCURSOR     hcrCrossHair;
    wxCursor    hcrCrossHairWx;
#ifndef GPLAY
    HCURSOR     hcrFill;
    wxCursor    hcrFillWx;
    HCURSOR     hcrDropper;
    wxCursor    hcrDropperWx;
    HCURSOR     hcrEraser;
    wxCursor    hcrEraserWx;
    HCURSOR     hcrPencil;
    wxCursor    hcrPencilWx;
    HCURSOR     hcrBrush;
    wxCursor    hcrBrushWx;
    HCURSOR     hcrSmall4Way;
    wxCursor    hcrSmall4WayWx;
    HCURSOR     hcrColorChange;
    wxCursor    hcrColorChangeWx;
#else
    HCURSOR     hcrCompMoveActive;
#endif
#ifndef GPLAY
    // ======== Brushes ========= //
    wxBrush     hbr25PctWx;     // 25% grey pattern
#endif
    // ------ //
    ResourceTable() { m_bInited = FALSE; }
    ~ResourceTable();
    // ------ //
    void InitResourceTable(HINSTANCE hInst);
private:
    void LoadCursors(HINSTANCE hInst);
    void FreeCursors(void);
    void LoadBrushes(HINSTANCE hInst);  // Load new brushes and colors
    void FreeBrushes(void);         // free the brushes
    void LoadFonts(void);           // Load screen fonts
    void FreeFonts(void);           // Free screen fonts
};

// ------------------------------------------------------- //
// MOVE THIS INTO A GLOBAL ACCESS HEADER???

extern ResourceTable g_res;

// ------------------------------------------------------- //
//  HBRUSH      hbrBtnShd;      // Button shade colored brush
//  HBRUSH      hbrBtnFace;     // Button face colored brush
//  HBRUSH      hbrBtnHlt;      // Button hightlight colored brush
//  HCURSOR     hcrSmlArr;
//  HCURSOR     hcrHArrows;
//  HCURSOR     hcrVArrows;
//  HCURSOR     hcrNESWArr;
//  HCURSOR     hcrNWSEArr;
//  HCURSOR     hcrHSizeBar;
//  HCURSOR     hcrVSizeBar;
// ======== Colors ========= //
//  COLORREF    crBtnShd;       // Button shade color
//  COLORREF    crBtnFace;      // Button face color
//  COLORREF    crBtnHlt;       // Button hightlight color
//  COLORREF    crBtnTxt;       // Button text color


#endif

