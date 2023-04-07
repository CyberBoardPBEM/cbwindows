// ResTbl.cpp
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
    m_bInited = TRUE;
    LoadCursors(hInst);
    LoadFonts();
    LoadBrushes();
}

// ----------------------------------------------------- //

ResourceTable::~ResourceTable(void)
{
    FreeCursors();
    FreeFonts();
    FreeBrushes();
}

// ----------------------------------------------------- //

void ResourceTable::LoadCursors(HINSTANCE hInst)
{
    hcrDragTile = LoadCursor(hInst, MAKEINTRESOURCE(IDC_DRAGTILE));
    hcrNoDrop = LoadCursor(hInst, MAKEINTRESOURCE(IDC_NODROP));
    hcrNoDropTooBig = LoadCursor(hInst, MAKEINTRESOURCE(IDC_NODROP_TOOBIG));
    hcrCrossHair = LoadCursor(hInst, MAKEINTRESOURCE(IDC_CROSSHAIR));
#ifndef GPLAY
    hcrFill = LoadCursor(hInst, MAKEINTRESOURCE(IDC_FILL));
    hcrDropper = LoadCursor(hInst, MAKEINTRESOURCE(IDC_DROPPER));
    hcrEraser = LoadCursor(hInst, MAKEINTRESOURCE(IDC_ERASER));
    hcrPencil = LoadCursor(hInst, MAKEINTRESOURCE(IDC_PENCIL));
    hcrBrush = LoadCursor(hInst, MAKEINTRESOURCE(IDC_BRUSH));
    hcrSmall4Way = LoadCursor(hInst, MAKEINTRESOURCE(IDC_SMALL4WAY));
    hcrColorChange = LoadCursor(hInst, MAKEINTRESOURCE(IDC_COLORCHANGE));
#else
    hcrCompMoveActive = LoadCursor(hInst, MAKEINTRESOURCE(IDC_CMOVMODE));
#endif
}

// ----------------------------------------------------- //

void ResourceTable::FreeCursors(void)
{
    if (!m_bInited)     // Handles are bad
        return;
    DestroyCursor(hcrDragTile);
    DestroyCursor(hcrNoDrop);
    DestroyCursor(hcrNoDropTooBig);
    DestroyCursor(hcrCrossHair);
#ifndef GPLAY
    DestroyCursor(hcrFill);
    DestroyCursor(hcrDropper);
    DestroyCursor(hcrEraser);
    DestroyCursor(hcrPencil);
    DestroyCursor(hcrBrush);
    DestroyCursor(hcrColorChange);
#else
    DestroyCursor(hcrCompMoveActive);
#endif
}

// ----------------------------------------------------- //

void ResourceTable::FreeBrushes(void)
{
    if (!m_bInited)     // Handles are bad
        return;
    if (hbr50Pct)   DeleteObject(hbr50Pct);
    if (hbm50Pct)   DeleteObject(hbm50Pct);
    if (hbr25Pct)   DeleteObject(hbr25Pct);
    if (hbm25Pct)   DeleteObject(hbm25Pct);

    hbm50Pct = NULL;
    hbr50Pct = NULL;
    hbm25Pct = NULL;
    hbr25Pct = NULL;
}

// ----------------------------------------------------- //

void ResourceTable::LoadBrushes(void)
{
    hbm50Pct = ::LoadBitmap(AfxGetResourceHandle(),
        MAKEINTRESOURCE(IDB_50PERCENT));
    hbr50Pct = ::CreatePatternBrush(hbm50Pct);
    hbm25Pct = ::LoadBitmap(AfxGetResourceHandle(),
        MAKEINTRESOURCE(IDB_25PERCENT));
    hbr25Pct = ::CreatePatternBrush(hbm25Pct);
}

// ----------------------------------------------------- //

void ResourceTable::LoadFonts(void)
{
    LOGFONT lf;
    HDC     hDC = GetDC(NULL);      // get the screen DC
    int     yLogPix = GetDeviceCaps(hDC, LOGPIXELSY);

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

    memset(&lf, 0, sizeof(LOGFONT));

    lf.lfHeight = -(nSizeSmall * yLogPix) / 72;
    lf.lfWeight = FW_NORMAL;
    lf.lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
    lstrcpy(lf.lfFaceName, strFontName);

    h8ss = CreateFontIndirect(&lf);
    HFONT hFntSave = (HFONT) SelectObject(hDC, h8ss);
    GetTextMetrics(hDC, &tm8ss);

    lf.lfWeight = FW_BOLD;
    h8ssb = CreateFontIndirect(&lf);
    SelectObject(hDC, h8ssb);
    GetTextMetrics(hDC, &tm8ssb);

    lf.lfHeight = -(nSizeBig * yLogPix) / 72;
    lf.lfWeight = FW_NORMAL;
    h10ss = CreateFontIndirect(&lf);
    SelectObject(hDC, h10ss);
    GetTextMetrics(hDC, &tm10ss);

    SelectObject(hDC, hFntSave);
    ReleaseDC(NULL, hDC);
}

// ----------------------------------------------------- //

void ResourceTable::FreeFonts(void)
{
    if (!m_bInited)     // Handles are bad
        return;
    if (h8ss)  DeleteObject(h8ss);
    if (h8ssb) DeleteObject(h8ssb);
    if (h10ss) DeleteObject(h10ss);
}

