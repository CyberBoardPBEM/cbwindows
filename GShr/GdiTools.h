// GdiTools.h
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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
    WIN_RGBTRIO(COLORREF cr)
    {
        *this = cr;
    }
    WIN_RGBTRIO& operator=(COLORREF cr)
    {
        blue = GetBValue(cr);
        green = GetGValue(cr);
        red = GetRValue(cr);
        return *this;
    }

    operator COLORREF() const
    {
        return RGB(red, green, blue);
    }
};
static_assert(sizeof(WIN_RGBTRIO) == 3 &&
                    alignof(WIN_RGBTRIO) == 1,
                "WIN_RGBTRIO problem");

////////////////////////////////////////////////////////////////////

const       COLORREF noColor = 0xFF000000;

////////////////////////////////////////////////////////////////////

class CGdiTools
{
public:
    void InitGdiTools();
    // --------- //
    CDC mDC1;
    CDC mDC2;
    CDC mTileDC;
    HBITMAP hbmSafe;            // 1x1 Stock monochrome object
    HPALETTE hpalSafe;          // Default stock palatte
    // --------- //
    void ClearMemDCBitmaps()
    {
        ::SelectObject(mDC1.m_hDC, hbmSafe);
        ::SelectObject(mDC2.m_hDC, hbmSafe);
        ::SelectObject(mTileDC.m_hDC, hbmSafe);
    }
    void ClearMemDCPalettes()
    {
        ::SelectPalette(mDC1.m_hDC, hpalSafe, TRUE);
        ::SelectPalette(mDC2.m_hDC, hpalSafe, TRUE);
        ::SelectPalette(mTileDC.m_hDC, hpalSafe, TRUE);
    }
    void SelectSafeObjectsForDC1()
    {
        ::SelectObject(mDC1.m_hDC, hbmSafe);
        ::SelectPalette(mDC1.m_hDC, hpalSafe, TRUE);
    }
    void SelectSafeObjectsForDC2()
    {
        ::SelectObject(mDC2.m_hDC, hbmSafe);
        ::SelectPalette(mDC2.m_hDC, hpalSafe, TRUE);
    }
    void SelectSafeObjectsForTileDC()
    {
        ::SelectObject(mTileDC.m_hDC, hbmSafe);
        ::SelectPalette(mTileDC.m_hDC, hpalSafe, TRUE);
    }
    void SetupMemDCPalettes(CPalette *pPal1, CPalette *pPal2);
};

extern CGdiTools g_gt;


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

FontID DoFontDialog(FontID fid, CWnd *pParentWnd, BOOL bScreenOK = FALSE);

BOOL DeltaGenInit(PDELTAGEN pdg, int x0, int y0, int xf, int yf, int* px,
    int* py);
BOOL DeltaGen(PDELTAGEN pdg, int* px, int* py);

int GetYPixelsPerLogicalInch();
int ScreenPixelsToTenthPoints(int nPixels);
int TenthPointsToScreenPixels(int nTenthPts);
int GetCurrentVideoResolution();

OwnerPtr<CBitmap> CreateRGBDIBSection(int nWidth, int nHeight);
OwnerPtr<CBitmap> CreateRGBColorBar(int nHueDivisions, int nHeight);
OwnerPtr<CBitmap> CreateRGBSaturationValueWash(int nHue, int nWidth, int nHeight);

void SetRGBDIBSectPixel(CBitmap& hBitmap, int x, int y, COLORREF cr);
void SetRGBDIBSectPixelBlock(CBitmap& hBitmap, int x, int y, int cx, int cy, COLORREF cr);

COLORREF HSVtoRGB(int h, int s, int v);
void HSVtoRGB(double h, double s, double v, double& r, double& g, double& b);

void ClearSystemPalette();
void SetupColorTable(CDWordArray* pTbl, BOOL bInclBlackAndWhite = TRUE);
void SetupPalette(CDC& pDC);
void ResetPalette(CDC& pDC);
OwnerPtr<CBitmap> CloneBitmap(const CBitmap& pbmSrc);
// The copy excludes the right and bottom edges of rctSrc.
// If a crVoided color is supplied, the source bitmap will have its
// copied region filled with that color.
OwnerPtr<CBitmap> CopyBitmapPiece(CBitmap& pbmSrc, CRect rctSrc,
    COLORREF crVoided);
OwnerPtr<CBitmap> CloneScaledBitmap(const CBitmap& pbmSrc, CSize size,
    int nStretchMode = COLORONCOLOR);
void MergeBitmap(CBitmap& pbmDst, const CBitmap& pbmSrc, CPoint pntDst);
void BitmapBlt(CDC& pDC, CPoint pntDst, const CBitmap& pBMap);
void Draw25PctPatBorder(CWnd& pWnd, CDC& pDC, CRect rct, int nThick);
OwnerPtr<CBitmap> CreateColorBitmap(CSize size, COLORREF cr);

CPalette* GetAppPalette();

#ifdef WE_WANT_THIS_STUFF_DLL940113
void ResizeBitmap(CBitmap *pBMap, int iNewX, int iNewY, CPalette* pPalOld,
    CPalette* pPalNew = NULL);
void AppendBitmap(CBitmap *pBMapTo, CBitmap *pBMapFrm, CPalette *pPalTo,
    CPalette *pPalFrm);
void RemoveBitmapSlice(CBitmap *pBMap, int yPos, int iHt, CPalette *pPal = NULL);
CPalette* BuildMasterPalette(CObArray* pPalTbl, BOOL bAppend = TRUE);
#endif
OwnerPtr<CPalette> CreateMergedPalette(const CPalette& palPri, const CPalette& palSec);
void SetupIdentityPalette(uint16_t nNumColors, LPLOGPALETTE pPal);
void AddEntryToPalette(LPPALETTEENTRY pPal, int nSize, const PALETTEENTRY& pe);
void SetPaletteEntryFromColorref(PALETTEENTRY& pe, COLORREF cr);
#ifdef WE_WANT_THIS_STUFF_DLL940113
void BltThruDIB(CDC& pDCDst, int xDst, int yDst, int cx, int cy,
    CDC& pDCSrc, const CPalette *pPalSrc, const CBitmap& pBMap, int xSrc, int ySrc);
#endif

OwnerPtr<CBrush> Clone(const CBrush& brush);

// geomorphic board support
// based on incremental piece rotation, positive rotation is clockwise
enum class Rotation90
{
    rInvalid = -1, r0 = 0, r90 = 1, r180 = 2, r270 = 3
};
OwnerPtr<CBitmap> Rotate(const CBitmap& bmp, Rotation90 rot);

#endif
