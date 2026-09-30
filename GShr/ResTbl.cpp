// ResTbl.cpp
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

#include    "stdafx.h"
#include    "ResTbl.h"
#include    "Resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ResourceTable g_res;

// ----------------------------------------------------- //

void ResourceTable::InitResourceTable(HINSTANCE hInst)
{
    LoadCursors(hInst);
    LoadFonts();
    LoadBrushes(hInst);
}

// ----------------------------------------------------- //

void ResourceTable::LoadCursors(HINSTANCE hInst)
{
    // TODO:  rewrite w/o wxMSW-specific code
    hcrDragTileWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_DRAGTILE))));
    hcrNoDropWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_NODROP))));
    hcrNoDropTooBigWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_NODROP_TOOBIG))));
    hcrCrossHairWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_CROSSHAIR))));
#ifndef GPLAY
    hcrFillWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_FILL))));
    hcrDropperWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_DROPPER))));
    hcrEraserWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_ERASER))));
    hcrPencilWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_PENCIL))));
    hcrBrushWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_BRUSH))));
    hcrSmall4WayWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_SMALL4WAY))));
    hcrColorChangeWx.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_COLORCHANGE))));
#else
    hcrCompMoveActive.SetHCURSOR(reinterpret_cast<WXHCURSOR>(LoadCursor(hInst, MAKEINTRESOURCE(IDC_CMOVMODE))));
#endif
}

// ----------------------------------------------------- //

void ResourceTable::LoadBrushes(HINSTANCE hInst)
{
#ifndef GPLAY
    // create brush with wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE
    // TODO:  rewrite w/o wxMSW-specific code
    static const auto LoadBrush = [](HINSTANCE hInst, WORD id)
    {
        HBITMAP hbmp = ::LoadBitmap(hInst, MAKEINTRESOURCE(id));
        if (!hbmp)
        {
            AfxThrowResourceException();
        }
        BITMAP bm;
        if (!::GetObject(hbmp, sizeof(BITMAP), &bm))
        {
            DeleteObject(hbmp);
            AfxThrowResourceException();
        }
        wxBitmap mask;
        if (!mask.InitFromHBITMAP(static_cast<WXHBITMAP>(hbmp), bm.bmWidth, bm.bmHeight, bm.bmBitsPixel))
        {
            DeleteObject(hbmp);
            AfxThrowResourceException();
        }
        wxBitmap wxbmp;
        wxbmp.SetMask(new wxMask(mask));
        return wxBrush(wxbmp);
    };
    hbr25PctWx = LoadBrush(hInst, IDB_25PERCENT);
#endif
}

// ----------------------------------------------------- //

void ResourceTable::LoadFonts(void)
{
    // Allow for font override from the resource file
    int     nSizeSmall = 8;
    int     nSizeBig = 10;

    CB::string strFontName = CB::string::LoadString(IDS_BASEFONT_NAME);
    if (strFontName.empty())
        strFontName = "MS Sans Serif";
    CB::string strFontSize = CB::string::LoadString(IDS_BASEFONT_SIZE_SMALL);
    if (!strFontSize.empty())
        nSizeSmall = std::stoi(strFontSize.std_wstr());
    strFontSize = CB::string::LoadString(IDS_BASEFONT_SIZE_BIG);
    if (!strFontSize.empty())
        nSizeBig = std::stoi(strFontSize.std_wstr());

    h8ssWx = wxFont(nSizeSmall,
                    wxFONTFAMILY_SWISS,
                    wxFONTSTYLE_NORMAL,
                    wxFONTWEIGHT_NORMAL,
                    false,
                    strFontName);

    h8ssbWx = wxFont(nSizeSmall,
                    wxFONTFAMILY_SWISS,
                    wxFONTSTYLE_NORMAL,
                    wxFONTWEIGHT_BOLD,
                    false,
                    strFontName);
}

