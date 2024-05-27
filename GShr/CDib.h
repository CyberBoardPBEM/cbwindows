// CDib.h
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

#ifndef _CDIB_H
#define _CDIB_H

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
    explicit operator bool() const { return m_wximg.IsOk(); }
    CDib(DWORD dwWidth, DWORD dwHeight);
    // ---------- /
    explicit CDib(const CBitmap& pBM);
    OwnerPtr<CBitmap> DIBToBitmap() const;
#ifdef WE_WANT_THIS_STUFF_DLL940113
    int StretchDIBits(CDC& pDC, int xDest, int yDest, int cxDest, int cyDest,
        int xSrc, int ySrc, int cxSrc, int cySrc) const
    {
        return ::StretchDIBits(pDC.m_hDC, xDest, yDest, cxDest, cyDest,
            xSrc, ySrc, cxSrc, cySrc, FindBits(), &GetBmi(), DIB_RGB_COLORS,
            SRCCOPY);
    }
#endif
    // ---------- //
    int Height() const { return m_wximg.GetHeight(); }
    int Width() const { return m_wximg.GetWidth(); }
#ifdef WE_WANT_THIS_STUFF_DLL940113
    const BITMAPINFO& GetBmi() const { return m_hDib; }
#endif
private:
    void Fill(COLORREF color);
public:
    // angle is clockwise
    CDib Rotate(int angle, COLORREF crTrans) const;
    // ---------- //
    void SetCompressLevel(int nCompressLevel) { m_nCompressLevel = nCompressLevel; }
    int  GetCompressLevel() const { return m_nCompressLevel; }
    // ---------- //

    // convert bits to uint32_t aligned bytes
    static size_t BitsToBytes(size_t i)
    {
        return ((i + size_t(31)) & size_t(~31)) / size_t(8);
    }
    static size_t BitsToBytes(LONG i)
    {
        return BitsToBytes(value_preserving_cast<size_t>(i));
    }

    static size_t WidthBytes(const BITMAPINFOHEADER& bmi)
    {
        return BitsToBytes(bmi.biWidth * bmi.biBitCount);
    }

    static OwnerPtr<CBitmap> CreateDIBSection(int nWidth, int nHeight);

private:
    CDib RotateFast(int angle) const;
    struct ImgEdge;
    void DrawScanLine(ImgEdge& lftEdge, ImgEdge& rgtEdge, int dstY,
        CDib& pDDib) const;
    static CDib CreateTransparentColorDIB(CSize size, COLORREF crTrans);

    wxImage m_wximg;
    int   m_nCompressLevel;
    // ---------- //
    friend CArchive& AFXAPI operator<<(CArchive& ar, const CDib& dib);
    friend CArchive& AFXAPI operator>>(CArchive& ar, CDib& dib);
    friend CArchive& WriteRGB565Zlib(CArchive& ar, const CDib& dib);
    friend CArchive& WriteImgBMPZlib(CArchive& ar, const CDib& dib);
    friend CArchive& ReadRGB565Zlib(CArchive& ar, CDib& dib);
    friend CArchive& ReadImgBMPZlib(CArchive& ar, CDib& dib);
};

wxImage ToImage(const CBitmap& bmp);
OwnerPtr<CBitmap> ToBitmap(const wxImage& img);
namespace CB
{
    inline wxBitmap Convert(const CBitmap& bmp)
    {
        return wxBitmap(ToImage(bmp), 24);
    }

    inline OwnerPtr<CBitmap> Convert(const wxBitmap& bmp)
    {
        return ToBitmap(bmp.ConvertToImage());
    }
}

#endif

