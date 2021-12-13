// GdiTools.h
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

#ifndef _GDITOOLS_H
#define _GDITOOLS_H

#ifndef     _FONT_H
#include    "font.h"
#endif

////////////////////////////////////////////////////////////////////
// DIB scan lines are always aligned on DWORD boundries. This inline
// computes the number of bytes in a 16bits per pixel scan line.

inline UINT ScanBytesFor16bpp(UINT nWidth)
    { return 2 * nWidth + ((nWidth & 1) ? 2 : 0); }

////////////////////////////////////////////////////////////////////

class CDib;

////////////////////////////////////////////////////////////////////
// Converts 24 bit RGB values to 16 bit 5-6-5 format

inline WORD RGB565(COLORREF cref)
{
    return (WORD)(((cref & 0xF8) << 8) | ((cref & 0xFC00) >> 5) | ((cref & 0xF80000) >> 19));
}

inline COLORREF RGB565_TO_24(WORD clr16)
{
    BYTE r = (((clr16 & 0xF800) >> 11) * 0xFF) / 0x1F;
    BYTE g = static_cast<BYTE>((((clr16 & 0x7E0) >> 5) * 0xFF) / 0x3F);
    BYTE b = (( clr16 & 0x1F) * 0xFF) / 0x1F;
    return RGB(r, g, b);
}

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
// From ROTATE.CPP

CDib Rotate16BitDib(const CDib& pSDib, int angle, COLORREF crTrans);
void  RotatePoints(POINT* pPnts, int nPnts, int nDegrees);
void  OffsetPoints(POINT* pPnts, int nPnts, int xOff, int yOff);

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

void ConvertMonochromeBMapToDIBSection(HDC hDC, BOOL bDelMono = TRUE);

LPVOID GetDIBSectXYLoc(HBITMAP hBitmap, int x, int y);

COLORREF MapWin9xRgbToNtRgb(COLORREF cr);
WORD     MapWin9xRgbToWin9x565(COLORREF cr);
void     FixupTransparentColorsAfter256ColorDibUpgrade(
    HBITMAP hBitmap, COLORREF crTrans);
void    GetDIBSectDimensions(HBITMAP hBitmap, int& rWidth, int& rHeight);

HBITMAP Create16BitDIBSection(HDC hDC, int nWidth, int nHeight);
HBITMAP Create16BitColorBar(int nHueDivisions, int nHeight);
HBITMAP Create16BitSaturationValueWash(int nHue, int nWidth, int nHeight);
HBITMAP Create16BitColorWash(int nHues, int nHueVertSteps, int cxBlock, int cyBlock);
HBITMAP Create16BitColorSweep();

void Set16BitDIBSectPixel(HBITMAP hBitmap, int x, int y, COLORREF cr);
void Set16BitDIBSectPixelBlock(HBITMAP hBitmap, int x, int y, int cx, int cy, COLORREF cr);
COLORREF Get16BitDIBSectPixel(HBITMAP hBitmap, int x, int y);

BOOL Is16BitDIBSectInvisible(HBITMAP hBitmap, COLORREF crTrans, int y = -1, int nHeight = 0);

HBITMAP CreateRGBDIBSection(HDC hDC, int nWidth, int nHeight);
HBITMAP CreateRGBColorBar(int nHueDivisions, int nHeight);
HBITMAP CreateRGBSaturationValueWash(int nHue, int nWidth, int nHeight);

void SetRGBDIBSectPixel(HBITMAP hBitmap, int x, int y, COLORREF cr);
void SetRGBDIBSectPixelBlock(HBITMAP hBitmap, int x, int y, int cx, int cy, COLORREF cr);
COLORREF GetRGBDIBSectPixel(HBITMAP hBitmap, int x, int y);

COLORREF HSVtoRGB(int h, int s, int v);
void HSVtoRGB(double h, double s, double v, double *r, double *g, double *b);

void ClearSystemPalette();
void SetupColorTable(CDWordArray* pTbl, BOOL bInclBlackAndWhite = TRUE);
void SetupPalette(CDC& pDC);
void ResetPalette(CDC& pDC);
OwnerPtr<CBitmap> CloneBitmap(const CBitmap& pbmSrc);
void CopyBitmapPiece(CBitmap *pbmDst, CBitmap *pbmSrc, CRect rctSrc,
    COLORREF crVoided = noColor);
OwnerPtr<CBitmap> CloneScaledBitmap(const CBitmap& pbmSrc, CSize size,
    int nStretchMode = COLORONCOLOR);
void MergeBitmap(CBitmap& pbmDst, const CBitmap& pbmSrc, CPoint pntDst,
    COLORREF crTrans = noColor);
void BitmapBlt(CDC& pDC, CPoint pntDst, const CBitmap& pBMap);
void TransBlt(CDC& pDC, CPoint pntDst, const CBitmap& pBMap, COLORREF crTrans);
void Draw25PctPatBorder(CWnd& pWnd, CDC& pDC, CRect rct, int nThick);
OwnerPtr<CBitmap> CreateColorBitmap(CSize size, COLORREF cr);

CPalette* GetAppPalette();

void ResizeBitmap(CBitmap *pBMap, int iNewX, int iNewY, CPalette* pPalOld,
    CPalette* pPalNew = NULL);
void AppendBitmap(CBitmap *pBMapTo, CBitmap *pBMapFrm, CPalette *pPalTo,
    CPalette *pPalFrm);
void RemoveBitmapSlice(CBitmap *pBMap, int yPos, int iHt, CPalette *pPal = NULL);
#ifdef WE_WANT_THIS_STUFF_DLL940113
CPalette* BuildMasterPalette(CObArray* pPalTbl, BOOL bAppend = TRUE);
#endif
OwnerPtr<CPalette> CreateMergedPalette(const CPalette& palPri, const CPalette& palSec);
void SetupIdentityPalette(uint16_t nNumColors, LPLOGPALETTE pPal);
void AddEntryToPalette(LPPALETTEENTRY pPal, int nSize, const PALETTEENTRY& pe);
void SetPaletteEntryFromColorref(PALETTEENTRY& pe, COLORREF cr);
void BltThruDIB(CDC& pDCDst, int xDst, int yDst, int cx, int cy,
    CDC& pDCSrc, const CPalette *pPalSrc, const CBitmap& pBMap, int xSrc, int ySrc);

OwnerPtr<CBrush> Clone(const CBrush& brush);

// geomorphic board support
// based on incremental piece rotation, positive rotation is clockwise
enum class Rotation90
{
    rInvalid = -1, r0 = 0, r90 = 1, r180 = 2, r270 = 3
};

#endif
