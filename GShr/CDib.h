// CDib.h
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

#ifndef _CDIB_H
#define _CDIB_H

#ifndef _INC_DIBAPI
#include "DibApi.h"
#endif

class CDib
{
public:
    CDib() { m_hDib = NULL; m_lpDib = NULL; m_nCompressLevel = 0; }
    ~CDib() { ClearDib(); }
    void ClearDib();
    // ---------- /
    void CreateDIB(DWORD dwWidth, DWORD dwHeight, WORD wBPP = 16);
    // ---------- /
    void ReadDIBFile(CFile& file);
    BOOL WriteDIBFile(CFile& file);
    void CloneDIB(CDib *pDib);
    BOOL BitmapToDIB(const CBitmap* pBM, const CPalette* pPal = NULL, uint16_t nBPP = uint16_t(16));
    OwnerPtr<CBitmap> DIBToBitmap(const CPalette *pPal, BOOL bDibSect = TRUE);
    BOOL AppendDIB(CDib *pDib);
    BOOL RemoveDIBSlice(int y, int ht);
    void AddColorsToPaletteEntryTable(LPPALETTEENTRY pLP, int nSize,
            BOOL bReducedPalette = TRUE)
        { AddDIBColorsToPaletteEntryTable(m_hDib, pLP, nSize, bReducedPalette);}
    BOOL CreatePalette(CPalette* pPal, BOOL bReducePal = FALSE)
        { return ::CreateDIBPalette(m_hDib, pPal, bReducePal); }
    int StretchDIBits(CDC* pDC, int xDest, int yDest, int cxDest, int cyDest,
        int xSrc, int ySrc, int cxSrc, int cySrc)
    {
        return ::StretchDIBits(pDC->m_hDC, xDest, yDest, cxDest, cyDest,
            xSrc, ySrc, cxSrc, cySrc, FindBits(), GetBmi(), DIB_RGB_COLORS,
            SRCCOPY);
    }
    void SetDibHandle(HANDLE hDib);
    // ---------- //
    int Height() { return (int)DIBHeight(m_lpDib); }
    int Width()  { return (int)DIBWidth(m_lpDib); }
    int NumColors() { return ((LPBITMAPINFOHEADER)m_lpDib)->biBitCount; }
    int NumColorsInColorTable() { return DIBNumColors(m_lpDib); }
    LPBITMAPINFOHEADER GetBmiHdr() { return (LPBITMAPINFOHEADER)m_lpDib; }
    LPBITMAPINFO GetBmi() { return (LPBITMAPINFO)m_lpDib; }
    LPSTR FindBits() { return FindDIBBits(m_lpDib); }
    // ---------- for 256 color Dibs only -------------- //
    BYTE Get256ColorNumberAtXY(int x, int y);
    void Set256ColorNumberAtXY(int x, int y, BYTE nColor);
    // ---------- for 16bit/pixel Dibs only -------------- //
    WORD Get16BitColorNumberAtXY(int x, int y);
    void Set16BitColorNumberAtXY(int x, int y, WORD nColor);
    // ---------- //
    void SetCompressLevel(int nCompressLevel) { m_nCompressLevel = nCompressLevel; }
    int  GetCompressLevel() { return m_nCompressLevel; }
    // ---------- //
    HDIB  m_hDib;
    LPSTR m_lpDib;
    int   m_nCompressLevel;
    // ---------- //
    friend CArchive& AFXAPI operator<<(CArchive& ar, const CDib& dib);
    friend CArchive& AFXAPI operator>>(CArchive& ar, CDib& dib);
};

#endif

