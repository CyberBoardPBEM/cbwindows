// GdiTools.h
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

#ifndef _GDITOOLS_H
#define _GDITOOLS_H

#ifndef     _FONT_H
#include    "font.h"
#endif

////////////////////////////////////////////////////////////////////
// DIB scan lines are always aligned on DWORD boundries. This inline
// computes the number of bytes in a 16bits per pixel scan line.

inline size_t ScanBytesFor24bpp(size_t nWidth)
    { return (size_t(3)*nWidth + size_t(3)) & ~size_t(3); }

////////////////////////////////////////////////////////////////////

// for 24bpp DIBs
struct WIN_RGBTRIO
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;

    WIN_RGBTRIO() = default;
    WIN_RGBTRIO(WIN_COLORREF cr)
    {
        *this = cr;
    }
    WIN_RGBTRIO& operator=(WIN_COLORREF cr)
    {
        blue = cr.blue;
        green = cr.green;
        red = cr.red;
        return *this;
    }

    operator WIN_COLORREF() const
    {
        return WIN_COLORREF(red, green, blue);
    }
};
static_assert(sizeof(WIN_RGBTRIO) == 3 &&
                    alignof(WIN_RGBTRIO) == 1,
                "WIN_RGBTRIO problem");

////////////////////////////////////////////////////////////////////

const       COLORREF noColor = 0xFF000000;
const       wxColour noColorWx = CB::Convert(noColor);

////////////////////////////////////////////////////////////////////

// This structure is used in conjunction with DeltaGenInit() and DeltaGen().

typedef struct _DELTAGEN {
    BOOL fSwap;
    int xf;
    int yf;
    int dx;
    int dy;
    int d;
    int incx;
    int incy;
    int inc1;
    int inc2;
}   DELTAGEN;
typedef DELTAGEN *PDELTAGEN;

////////////////////////////////////////////////////////////////////
// Some non-class GDI tools...

FontID DoFontDialog(FontID fid, wxWindow* pParentWnd, BOOL bScreenOK = FALSE);

BOOL DeltaGenInit(PDELTAGEN pdg, int x0, int y0, int xf, int yf, int* px,
    int* py);
BOOL DeltaGen(PDELTAGEN pdg, int* px, int* py);

int GetYPixelsPerLogicalInch();
int ScreenPixelsToTenthPoints(int nPixels);
int TenthPointsToScreenPixels(int nTenthPts);
int GetCurrentVideoResolution();

OwnerPtr<wxBitmap> CreateRGBColorBar(int nHueDivisions, int nHeight);
OwnerPtr<wxBitmap> CreateRGBSaturationValueWash(int nHue, int nWidth, int nHeight);

void SetPixel(wxBitmap& hBitmap, int x, int y, wxColour cr);
wxColour GetPixel(const wxBitmap& hBitmap, int x, int y);
void SetPixelBlock(wxBitmap& hBitmap, int x, int y, int cx, int cy, wxColour cr);

COLORREF HSVtoRGB(int h, int s, int v);
void HSVtoRGB(double h, double s, double v, double& r, double& g, double& b);

wxBitmap CloneBitmap(const wxBitmap& pbmSrc);
// The source bitmap will have its
// cut region filled with the crVoided color.
wxBitmap CutBitmapPiece(wxBitmap& pbmSrc, wxRect rctSrc,
    wxColour crVoided);
wxBitmap CloneScaledBitmap(const wxBitmap& pbmSrc, wxSize size);
void MergeBitmap(wxBitmap& pbmDst, const wxBitmap& pbmSrc, wxPoint pntDst);
#ifndef GPLAY
void Draw25PctPatBorder(wxWindow& pWnd, wxDC& pDC, wxRect rct, int nThick);
#endif
wxBitmap CreateColorBitmap(wxSize size, wxColour cr);

#ifdef WE_WANT_THIS_STUFF_DLL940113
void ResizeBitmap(CBitmap *pBMap, int iNewX, int iNewY, CPalette* pPalOld,
    CPalette* pPalNew = NULL);
void AppendBitmap(CBitmap *pBMapTo, CBitmap *pBMapFrm, CPalette *pPalTo,
    CPalette *pPalFrm);
void RemoveBitmapSlice(CBitmap *pBMap, int yPos, int iHt, CPalette *pPal = NULL);
CPalette* BuildMasterPalette(CObArray* pPalTbl, BOOL bAppend = TRUE);
#endif
#ifdef WE_WANT_THIS_STUFF_DLL940113
void BltThruDIB(CDC& pDCDst, int xDst, int yDst, int cx, int cy,
    CDC& pDCSrc, const CPalette *pPalSrc, const CBitmap& pBMap, int xSrc, int ySrc);
#endif

// geomorphic board support
// based on incremental piece rotation, positive rotation is clockwise
enum class Rotation90
{
    rInvalid = -1, r0 = 0, r90 = 1, r180 = 2, r270 = 3
};
wxBitmap Rotate(const wxBitmap& bmp, Rotation90 rot);

#endif
