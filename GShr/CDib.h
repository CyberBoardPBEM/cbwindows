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
    CDib(const CDib&) = delete;
    CDib& operator=(const CDib&) = delete;
    CDib(CDib&& rhs) noexcept;
    CDib& operator=(CDib&& rhs) noexcept;
    ~CDib() { ClearDib(); }
    void ClearDib();
    operator bool() const { return m_hDib; }
    CDib(DWORD dwWidth, DWORD dwHeight, WORD wBPP = 16);
    // ---------- /
    explicit CDib(CFile& file);
    BOOL WriteDIBFile(CFile& file) const;
    explicit CDib(const CBitmap& pBM, const CPalette* pPal = NULL, uint16_t nBPP = uint16_t(16));
    OwnerPtr<CBitmap> DIBToBitmap(const CPalette *pPal, BOOL bDibSect = TRUE) const;
    BOOL AppendDIB(const CDib& pDib);
    BOOL RemoveDIBSlice(int y, int ht);
    void AddColorsToPaletteEntryTable(PALETTEENTRY& pLP, int nSize,
            BOOL bReducedPalette = TRUE)
        { AddDIBColorsToPaletteEntryTable(m_hDib, &pLP, nSize, bReducedPalette);}
    OwnerPtr<CPalette> CreatePalette(BOOL bReducePal = FALSE) const
    {
        OwnerPtr<CPalette> pPal(MakeOwner<CPalette>());
        ::CreateDIBPalette(m_hDib, &*pPal, bReducePal);
        return pPal;
    }
    int StretchDIBits(CDC& pDC, int xDest, int yDest, int cxDest, int cyDest,
        int xSrc, int ySrc, int cxSrc, int cySrc) const
    {
        return ::StretchDIBits(pDC.m_hDC, xDest, yDest, cxDest, cyDest,
            xSrc, ySrc, cxSrc, cySrc, FindBits(), GetBmi(), DIB_RGB_COLORS,
            SRCCOPY);
    }
    explicit CDib(HANDLE hDib);
    HDIB CopyHandle() const { return static_cast<HDIB>(::CopyHandle(m_hDib)); }
    // ---------- //
    int Height() const { return (int)DIBHeight(m_lpDib); }
    int Width() const { return (int)DIBWidth(m_lpDib); }
    int NumColors() const { return ((LPBITMAPINFOHEADER)m_lpDib)->biBitCount; }
    int NumColorsInColorTable() const { return DIBNumColors(m_lpDib); }
    const LPBITMAPINFOHEADER GetBmiHdr() const { return (LPBITMAPINFOHEADER)m_lpDib; }
    const LPBITMAPINFO GetBmi() const { return (LPBITMAPINFO)m_lpDib; }
    const LPSTR FindBits() const { return FindDIBBits(m_lpDib); }
    // ---------- for 256 color Dibs only -------------- //
    BYTE Get256ColorNumberAtXY(int x, int y) const;
    void Set256ColorNumberAtXY(int x, int y, BYTE nColor);
    // ---------- for 16bit/pixel Dibs only -------------- //
    WORD Get16BitColorNumberAtXY(int x, int y) const;
    void Set16BitColorNumberAtXY(int x, int y, WORD nColor);
    // ---------- //
    void SetCompressLevel(int nCompressLevel) { m_nCompressLevel = nCompressLevel; }
    int  GetCompressLevel() const { return m_nCompressLevel; }
    // ---------- //
private:
    HDIB  m_hDib;
    LPSTR m_lpDib;
    int   m_nCompressLevel;
    // ---------- //
    friend CArchive& AFXAPI operator<<(CArchive& ar, const CDib& dib);
    friend CArchive& AFXAPI operator>>(CArchive& ar, CDib& dib);
};

#endif

