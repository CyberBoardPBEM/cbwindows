// dibapi.h

// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#ifndef _INC_DIBAPI
#define _INC_DIBAPI

// TODO:  move to CDib.h when DibApi.h removed
// this includes the palette and pixel data
class CBITMAPINFOHEADER
{
public:
    CBITMAPINFOHEADER() = default;
    CBITMAPINFOHEADER(std::nullptr_t) : CBITMAPINFOHEADER() {}
    CBITMAPINFOHEADER(int32_t dwWidth, int32_t dwHeight, uint16_t wBitCount);
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
    operator const BITMAPINFOHEADER*() const { return &get(); }
    operator BITMAPINFOHEADER*() { return &get(); }

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
    const void* DibXY(ptrdiff_t x, ptrdiff_t y) const
    {
        return DibXY(*this, x, y);
    }

    // for reading in CDib
    void reserve(size_t s);
    // don't use void* except for serialize-in
    operator void*();

private:
    static uint16_t GetPaletteSize(const BITMAPINFOHEADER& lpbi);
    static uint16_t GetNumColors(const BITMAPINFOHEADER& lpbi);
    static const void* DibXY(const BITMAPINFOHEADER& lpbi, ptrdiff_t x, ptrdiff_t y);

    std::vector<std::byte> buf;
};

/* Handle to a DIB */
DECLARE_HANDLE(HDIB);

/* DIB constants */
#define PALVERSION   0x300

/* DIB Macros*/

#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

#define WIDTHBYTES(i)     ((unsigned)((i + 31) & (~31)) / 8)  /* ULONG aligned ! */
#define DIBWIDTHBYTES(bi) (int)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.

/* Function prototypes */
CBITMAPINFOHEADER BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal, uint16_t nBPP = uint16_t(0));

CBITMAPINFOHEADER ConvertDIBSectionToDIB(HBITMAP hDibSect);

#endif //!_INC_DIBAPI
