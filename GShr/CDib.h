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

class CDib
{
// this includes the palette and pixel data
class CBITMAPINFOHEADER
{
public:
    CBITMAPINFOHEADER() = default;
    CBITMAPINFOHEADER(std::nullptr_t) : CBITMAPINFOHEADER() {}
    CBITMAPINFOHEADER(int32_t dwWidth, int32_t dwHeight, uint16_t wBitCount);
    CBITMAPINFOHEADER(HBITMAP hBitmap, HPALETTE hPal, uint16_t nBPP = uint16_t(0));
    CBITMAPINFOHEADER(HBITMAP hDibSect);
    CBITMAPINFOHEADER(const CBITMAPINFOHEADER&) = delete;
    CBITMAPINFOHEADER(CBITMAPINFOHEADER&&) = default;
    CBITMAPINFOHEADER& operator=(const CBITMAPINFOHEADER&) = delete;
    CBITMAPINFOHEADER& operator=(CBITMAPINFOHEADER&&) = default;
    ~CBITMAPINFOHEADER() = default;

    /* KLUDGE:  need const and non-const to avoid ambiguity with
                operator BITMAPINFOHEADER* */
    explicit operator bool() const;
    explicit operator bool() { return bool(std::as_const(*this)); }
    size_t size() const { return *this ? buf.size() : size_t(0); }

    const BITMAPINFOHEADER& get() const;
    BITMAPINFOHEADER& get()
    {
        return const_cast<BITMAPINFOHEADER&>(std::as_const(*this).get());
    }
    operator const BITMAPINFOHEADER&() const { return get(); }
    operator BITMAPINFOHEADER&() { return get(); }
    operator const BITMAPINFOHEADER*() const { return &static_cast<const BITMAPINFOHEADER&>(*this); }
    operator BITMAPINFOHEADER*() { return &static_cast<BITMAPINFOHEADER&>(*this); }

    operator const BITMAPINFO&() const;
    operator BITMAPINFO&()
    {
        return const_cast<BITMAPINFO&>(static_cast<const BITMAPINFO&>(std::as_const(*this)));
    }
    operator const BITMAPINFO*() const { return &static_cast<const BITMAPINFO&>(*this); }
    operator BITMAPINFO*() { return &static_cast<BITMAPINFO&>(*this); }

    const void* GetBits() const;
    void* GetBits()
    {
        return const_cast<void*>(std::as_const(*this).GetBits());
    }
    const void* DibXY(ptrdiff_t x, ptrdiff_t y) const;

    // for reading in CDib
    void reserve(size_t s);
    // don't use void* except for serialize-in
    operator void*();

private:
    static WORD GetPaletteSize(const BITMAPINFOHEADER& lpbi);
    static WORD GetNumColors(const BITMAPINFOHEADER& lpbi);

    std::vector<std::byte> buf;
};

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
    const BITMAPINFO& GetBmi() const { return m_hDib; }
    const void* FindBits() const { return m_hDib.GetBits(); }
    void* FindBits() { return m_hDib.GetBits(); }
    // ---------- for 16bit/pixel Dibs only -------------- //
    WORD Get16BitColorNumberAtXY(int x, int y) const;
    void Set16BitColorNumberAtXY(int x, int y, WORD nColor);
    // ---------- //
    void SetCompressLevel(int nCompressLevel) { m_nCompressLevel = nCompressLevel; }
    int  GetCompressLevel() const { return m_nCompressLevel; }
    // ---------- //

    size_t WidthBytes() const
    {
        const BITMAPINFOHEADER& bmi = m_hDib;
        return BitsToBytes(bmi.biWidth * bmi.biBitCount);
    }

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

