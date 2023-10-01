// CDib.h
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

#ifndef _CDIB_H
#define _CDIB_H

#ifndef _INC_DIBAPI
#include "DibApi.h"
#endif

class CDib
{
public:
    CDib() { m_nCompressLevel = 0; }
    CDib(const CDib&) = delete;
    CDib& operator=(const CDib&) = delete;
    CDib(CDib&& rhs) noexcept;
    CDib& operator=(CDib&& rhs) noexcept;
    ~CDib() { ClearDib(); }
    void ClearDib();
    explicit operator bool() const { return bool(m_hDib); }
    CDib(DWORD dwWidth, DWORD dwHeight, WORD wBPP = 16);
    // ---------- /
    explicit CDib(const CBitmap& pBM, const CPalette* pPal = NULL, uint16_t nBPP = uint16_t(16));
    OwnerPtr<CBitmap> DIBToBitmap() const;
    int StretchDIBits(CDC& pDC, int xDest, int yDest, int cxDest, int cyDest,
        int xSrc, int ySrc, int cxSrc, int cySrc) const
    {
        return ::StretchDIBits(pDC.m_hDC, xDest, yDest, cxDest, cyDest,
            xSrc, ySrc, cxSrc, cySrc, FindBits(), &GetBmi(), DIB_RGB_COLORS,
            SRCCOPY);
    }
    // ---------- //
    int Height() const { return m_hDib.get().biHeight; }
    int Width() const { return m_hDib.get().biWidth; }
    int NumColorBits() const { return m_hDib.get().biBitCount; }
    const BITMAPINFOHEADER& GetBmiHdr() const { return m_hDib; }
    const BITMAPINFO& GetBmi() const { return m_hDib; }
    const void* FindBits() const { return m_hDib.GetBits(); }
    // ---------- for 16bit/pixel Dibs only -------------- //
    WORD Get16BitColorNumberAtXY(int x, int y) const;
    void Set16BitColorNumberAtXY(int x, int y, WORD nColor);
    // ---------- //
    void SetCompressLevel(int nCompressLevel) { m_nCompressLevel = nCompressLevel; }
    int  GetCompressLevel() const { return m_nCompressLevel; }
    // ---------- //

    static OwnerPtr<CBitmap> CreateDIBSection(int nWidth, int nHeight, uint16_t nBPP = uint16_t(16));

private:
    CBITMAPINFOHEADER m_hDib;
    int   m_nCompressLevel;
    // ---------- //
    friend CArchive& AFXAPI operator<<(CArchive& ar, const CDib& dib);
    friend CArchive& AFXAPI operator>>(CArchive& ar, CDib& dib);
};

wxImage ToImage(const CBitmap& bmp);
OwnerPtr<CBitmap> ToBitmap(const wxImage& img);

#endif

