// CDib.cpp
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

#include    "stdafx.h"
#include    <wx/mstream.h>
#include    <wx/zstream.h>
#include    "GdiTools.h"
#include    "CDib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////
// Converts 24 bit RGB values to 16 bit 5-6-5 format

namespace {
    inline uint16_t RGB565(COLORREF cref)
    {
        return (uint16_t)(((cref & 0xF8) << 8) | ((cref & 0xFC00) >> 5) | ((cref & 0xF80000) >> 19));
    }

    inline COLORREF RGB565_TO_24(uint16_t clr16)
    {
        uint8_t r = (((clr16 & 0xF800) >> 11) * 0xFF) / 0x1F;
        uint8_t g = static_cast<uint8_t>((((clr16 & 0x7E0) >> 5) * 0xFF) / 0x3F);
        uint8_t b = ((clr16 & 0x1F) * 0xFF) / 0x1F;
        return RGB(r, g, b);
    }
}

////////////////////////////////////////////////////////////////////

namespace {
    size_t BPP(HBITMAP hbmp)
    {
        DIBSECTION dibSect;
        if (!GetObject(hbmp, sizeof(dibSect), &dibSect))
        {
            CbThrowBadCastException();
        }
        switch (dibSect.dsBmih.biBitCount)
        {
            case 16:
                if (!(dibSect.dsBitfields[0] == 0xf800 &&
                        dibSect.dsBitfields[1] == 0x07e0 &&
                        dibSect.dsBitfields[2] == 0x001f))
                {
                    CbThrowBadCastException();
                }
                // fall through
            case 24:
                return value_preserving_cast<size_t>(dibSect.dsBmih.biBitCount);
            default:
                CbThrowBadCastException();
        }
    }

    void* Ptr(HBITMAP hbmp, size_t x, size_t y)
    {
        DIBSECTION dibSect;
        if (!GetObject(hbmp, sizeof(dibSect), &dibSect) ||
            !(dibSect.dsBm.bmBits &&
                x < value_preserving_cast<size_t>(dibSect.dsBm.bmWidth) &&
                y < value_preserving_cast<size_t>(dibSect.dsBm.bmHeight) &&
                dibSect.dsBm.bmWidthBytes &&
                dibSect.dsBmih.biSizeImage == value_preserving_cast<size_t>(dibSect.dsBm.bmHeight*dibSect.dsBm.bmWidthBytes)))
        {
            AfxThrowInvalidArgException();
        }
        return static_cast<std::byte*>(dibSect.dsBm.bmBits) +
                    // data is bottom up
                    (dibSect.dsBm.bmHeight - 1 - value_preserving_cast<ptrdiff_t>(y)) * dibSect.dsBm.bmWidthBytes +
                    value_preserving_cast<ptrdiff_t>(x) * dibSect.dsBm.bmBitsPixel / 8;
    }

    size_t WidthBytes(HBITMAP hbmp)
    {
        DIBSECTION dibSect;
        if (!GetObject(hbmp, sizeof(dibSect), &dibSect))
        {
            AfxThrowInvalidArgException();
        }
        return value_preserving_cast<size_t>(dibSect.dsBm.bmWidthBytes);
    }
}

///////////////////////////////////////////////////////////////

CDib::CBITMAPINFOHEADER::CBITMAPINFOHEADER(int32_t dwWidth, int32_t dwHeight, size_t wBitCount)
{
    switch (wBitCount)
    {
        case 16:
        case 24:
            break;
        default:
            AfxThrowNotSupportedException();
    }

    BITMAPINFOHEADER bi;
    memset(&bi, 0, sizeof(bi));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = dwWidth;
    bi.biHeight = dwHeight;
    bi.biPlanes = 1;
    bi.biBitCount = value_preserving_cast<uint16_t>(wBitCount);
    bi.biCompression = wBitCount == size_t(16) ? BI_BITFIELDS : BI_RGB;

    size_t dataSize = BitsToBytes(bi.biWidth * bi.biBitCount) * value_preserving_cast<size_t>(bi.biHeight);
    reserve(bi.biSize +
            (wBitCount <= size_t(16) ? GetPaletteSize(bi) : size_t(0)) +
            dataSize);
    *reinterpret_cast<BITMAPINFOHEADER*>(buf.data()) = bi;

    if (bi.biBitCount == 16)
    {
        BITMAPINFO& bmi = *this;
        DWORD* pdwMasks = reinterpret_cast<DWORD*>(bmi.bmiColors);
        pdwMasks[0] = 0x0000F800;
        pdwMasks[1] = 0x000007E0;
        pdwMasks[2] = 0x0000001F;
    }
}

CDib::CBITMAPINFOHEADER::CBITMAPINFOHEADER(HBITMAP hBitmap, HPALETTE hPal)
{
    if (!hBitmap)
    {
        CbThrowBadCastException();
    }

    DIBSECTION dibSect;
    if (!GetObject(hBitmap, sizeof(dibSect), &dibSect))
    {
        CbThrowBadCastException();
    }
    ASSERT(dibSect.dsBmih.biHeight > 0);
    *this = CBITMAPINFOHEADER(dibSect.dsBmih.biWidth, dibSect.dsBmih.biHeight, size_t(24));

    if (BPP(hBitmap) == size_t(16))
    {
        // DIBs are bottom up
        ASSERT(!"untested code");
        const uint16_t* srcRowStart = static_cast<const uint16_t*>(Ptr(hBitmap, size_t(0), size_t(0)));
        ptrdiff_t srcStride = -value_preserving_cast<ptrdiff_t>(dibSect.dsBm.bmWidthBytes / sizeof(*srcRowStart));
        std::byte* destRowStart = static_cast<std::byte*>(DibXY(0, 0));
        ptrdiff_t destStride = -value_preserving_cast<ptrdiff_t>(WidthBytes(*this));
        for (int y = 0 ; y < dibSect.dsBmih.biHeight ; ++y)
        {
            const uint16_t* src = srcRowStart;
            WIN_RGBTRIO* dest = reinterpret_cast<WIN_RGBTRIO*>(destRowStart);
            for (int x = 0 ; x < dibSect.dsBmih.biWidth ; ++x)
            {
                COLORREF cr = RGB565_TO_24(*src++);
                *dest++ = cr;
            }
            srcRowStart += srcStride;
            destRowStart += destStride;
        }
    }
    else if (BPP(hBitmap) == size_t(24))
    {
        // DIBs are bottom up
        const std::byte* src = static_cast<const std::byte*>(Ptr(hBitmap, size_t(0), value_preserving_cast<size_t>(dibSect.dsBmih.biHeight - 1)));
        std::byte* dest = static_cast<std::byte*>(DibXY(0, dibSect.dsBmih.biHeight - 1));
        size_t size = value_preserving_cast<size_t>(get().biHeight) * WidthBytes(*this);
        memcpy(dest, src, size);
    }
    else
    {
        AfxThrowNotSupportedException();
    }
}

CDib::CBITMAPINFOHEADER::CBITMAPINFOHEADER(const CDib::CBITMAPINFOHEADER& other, size_t wBitCount)
{
    const BITMAPINFOHEADER& bmih = other;
    if (bmih.biBitCount == wBitCount)
    {
        // caller shouldn't ask for unnecessary conversion
        AfxThrowInvalidArgException();
    }
    if (bmih.biBitCount == 16 && wBitCount == uint16_t(24))
    {
        *this = CBITMAPINFOHEADER(bmih.biWidth, bmih.biHeight, wBitCount);
        // DIBs are bottom up
        const uint16_t* srcRowStart = static_cast<const uint16_t*>(other.DibXY(0, 0));
        ptrdiff_t srcStride = -value_preserving_cast<ptrdiff_t>(WidthBytes(other) / sizeof(*srcRowStart));
        std::byte* destRowStart = static_cast<std::byte*>(DibXY(0, 0));
        ptrdiff_t destStride = -value_preserving_cast<ptrdiff_t>(WidthBytes(*this));
        for (int y = 0 ; y < bmih.biHeight ; ++y)
        {
            const uint16_t* src = srcRowStart;
            WIN_RGBTRIO* dest = reinterpret_cast<WIN_RGBTRIO*>(destRowStart);
            for (int x = 0 ; x < bmih.biWidth ; ++x)
            {
                COLORREF cr = RGB565_TO_24(*src++);
                *dest++ = cr;
            }
            srcRowStart += srcStride;
            destRowStart += destStride;
        }
    }
    else if (bmih.biBitCount == 24 && wBitCount == uint16_t(16))
    {
        *this = CBITMAPINFOHEADER(bmih.biWidth, bmih.biHeight, wBitCount);
        // DIBs are bottom up
        const std::byte* srcRowStart = static_cast<const std::byte*>(other.DibXY(0, 0));
        ptrdiff_t srcStride = -value_preserving_cast<ptrdiff_t>(WidthBytes(other));
        uint16_t* destRowStart = static_cast<uint16_t*>(DibXY(0, 0));
        ptrdiff_t destStride = -value_preserving_cast<ptrdiff_t>(WidthBytes(*this) / sizeof(*destRowStart));
        for (int y = 0; y < bmih.biHeight; ++y)
        {
            const WIN_RGBTRIO* src = reinterpret_cast<const WIN_RGBTRIO*>(srcRowStart);
            uint16_t* dest = destRowStart;
            for (int x = 0; x < bmih.biWidth; ++x)
            {
                COLORREF cr = *src++;
                *dest++ = RGB565(cr);
            }
            srcRowStart += srcStride;
            destRowStart += destStride;
        }
    }
    else if (bmih.biBitCount == 8 && wBitCount == uint16_t(24))
    {
        const BITMAPINFO& bm = reinterpret_cast<const BITMAPINFO&>(bmih);
        *this = CBITMAPINFOHEADER(bmih.biWidth, bmih.biHeight, wBitCount);
        // DIBs are bottom up
        const uint8_t* srcRowStart = static_cast<const uint8_t*>(other.DibXY(0, 0));
        ptrdiff_t srcStride = -value_preserving_cast<ptrdiff_t>(WidthBytes(other) / sizeof(*srcRowStart));
        std::byte* destRowStart = static_cast<std::byte*>(DibXY(0, 0));
        ptrdiff_t destStride = -value_preserving_cast<ptrdiff_t>(WidthBytes(*this));
        for (int y = 0 ; y < bmih.biHeight ; ++y)
        {
            const uint8_t* src = srcRowStart;
            WIN_RGBTRIO* dest = reinterpret_cast<WIN_RGBTRIO*>(destRowStart);
            for (int x = 0 ; x < bmih.biWidth ; ++x)
            {
                const RGBQUAD& rgbq = bm.bmiColors[*src++];
                COLORREF cr = RGB(rgbq.rgbRed,
                                    rgbq.rgbGreen,
                                    rgbq.rgbBlue);
                *dest++ = cr;
            }
            srcRowStart += srcStride;
            destRowStart += destStride;
        }
    }
    else
    {
        AfxThrowNotSupportedException();
    }
}

CDib::CBITMAPINFOHEADER::operator bool() const
{
    if (buf.size() < sizeof(BITMAPINFOHEADER))
    {
        return false;
    }
    const BITMAPINFOHEADER* retval = reinterpret_cast<const BITMAPINFOHEADER*>(buf.data());
    ASSERT(!retval->biSize ||
            retval->biSize == sizeof(BITMAPINFOHEADER));
    return retval->biSize == sizeof(BITMAPINFOHEADER);
}

const BITMAPINFOHEADER& CDib::CBITMAPINFOHEADER::get() const
{
    if (!*this)
    {
        CbThrowBadCastException();
    }
    return *reinterpret_cast<const BITMAPINFOHEADER*>(buf.data());
}

CDib::CBITMAPINFOHEADER::operator const BITMAPINFO&() const
{
    const BITMAPINFOHEADER& retval = *this;
    if (retval.biBitCount > 16)
    {
        CbThrowBadCastException();
    }
    return reinterpret_cast<const BITMAPINFO&>(retval);
}

const void* CDib::CBITMAPINFOHEADER::GetBits() const
{
    const BITMAPINFOHEADER& lpbi = *this;
    ASSERT(lpbi.biSize == sizeof(lpbi));
    return(reinterpret_cast<const std::byte*>(&lpbi) + lpbi.biSize + value_preserving_cast<ptrdiff_t>(GetPaletteSize(lpbi)));
}

const void* CDib::CBITMAPINFOHEADER::DibXY(ptrdiff_t x, ptrdiff_t y) const
{
    const BITMAPINFOHEADER& lpbi = *this;
    const std::byte* pBits = reinterpret_cast<const std::byte*>(&lpbi) + lpbi.biSize + value_preserving_cast<ptrdiff_t>(GetPaletteSize(lpbi));
    pBits += value_preserving_cast<ptrdiff_t>((WidthBytes(lpbi) * value_preserving_cast<size_t>(lpbi.biHeight - y - 1)) +
        value_preserving_cast<size_t>(x * lpbi.biBitCount / 8));
    return pBits;
}

void CDib::CBITMAPINFOHEADER::reserve(size_t s)
{
    ASSERT(buf.empty() && s >= sizeof(BITMAPINFO));
    buf.resize(s);
    ASSERT(!*this);
}

// don't use void* except for serialize-in
CDib::CBITMAPINFOHEADER::operator void*()
{
    if (*this ||
        buf.size() < sizeof(BITMAPINFOHEADER))
    {
        CbThrowBadCastException();
    }
    return reinterpret_cast<BITMAPINFOHEADER*>(buf.data());
}

///////////////////////////////////////////////////////////////////////

size_t CDib::CBITMAPINFOHEADER::GetPaletteSize(const BITMAPINFOHEADER& lpbi)
{
    if (lpbi.biBitCount == 24)
    {
        return size_t(0);
    }
    else
    {
        return value_preserving_cast<size_t>(GetNumColors(lpbi) * sizeof(RGBQUAD));
    }
}

///////////////////////////////////////////////////////////////////////

size_t CDib::CBITMAPINFOHEADER::GetNumColors(const BITMAPINFOHEADER& lpbi)
{
    // check for explicit count
    if (lpbi.biClrUsed != 0)
    {
        return value_preserving_cast<size_t>(lpbi.biClrUsed);
    }

    switch (lpbi.biBitCount)
    {
        case 1:
        case 4:
        case 8:
            return size_t(1) << lpbi.biBitCount;
        case 16:
            return size_t(3);       // Special: Used only for the pixel bit masks
        default:
            AfxThrowNotSupportedException();
    }
}

///////////////////////////////////////////////////////////////

void CDib::Fill(COLORREF color)
{
    ASSERT(m_hDib);
    switch (m_hDib.get().biBitCount)
    {
        case 16:
        {
            ASSERT(!"dead code");
            // Number of pixels (words) to fill
            size_t nBfrLen = (value_preserving_cast<size_t>(Height()) * WidthBytes(m_hDib)) / sizeof(uint16_t);
            uint16_t* pwBfr = static_cast<uint16_t*>(m_hDib.GetBits());
            while (nBfrLen--)
                *pwBfr++ = RGB565(color);
            break;
        }
        case 24:
        {
            int width = Width();
            int height = Height();
            // DIBs are bottom up
            std::byte* destRowStart = static_cast<std::byte*>(m_hDib.DibXY(0, 0));
            ptrdiff_t destStride = -value_preserving_cast<ptrdiff_t>(WidthBytes(m_hDib));
            for (int y = 0 ; y < height ; ++y)
            {
                WIN_RGBTRIO* dest = reinterpret_cast<WIN_RGBTRIO*>(destRowStart);
                for (int x = 0 ; x < width ; ++x)
                {
                    *dest++ = color;
                }
                destRowStart += destStride;
            }
            break;
        }
        default:
            CbThrowBadCastException();
    }
}

COLORREF CDib::GetColorAtXY(int x, int y) const
{
    ASSERT(m_hDib);
    ASSERT(x >= 0 && x < Width());
    ASSERT(y >= 0 && y < Height());
    switch (m_hDib.get().biBitCount)
    {
        case 16:
            ASSERT(!"dead code");
            return RGB565_TO_24(*static_cast<const uint16_t*>(m_hDib.DibXY(x, y)));
        case 24:
        {
            const WIN_RGBTRIO* p = static_cast<const WIN_RGBTRIO*>(m_hDib.DibXY(x, y));
            return *p;
        }
        default:
            CbThrowBadCastException();
    }
}

void CDib::SetColorAtXY(int x, int y, COLORREF nColor)
{
    ASSERT(m_hDib);
    ASSERT(x >= 0 && x < Width());
    ASSERT(y >= 0 && y < Height());
    switch (m_hDib.get().biBitCount)
    {
        case 16:
            ASSERT(!"dead code");
            *static_cast<uint16_t*>(m_hDib.DibXY(x, y)) = RGB565(nColor);
            break;
        case 24:
        {
            WIN_RGBTRIO* p = static_cast<WIN_RGBTRIO*>(m_hDib.DibXY(x, y));
            *p = nColor;
            break;
        }
        default:
            CbThrowBadCastException();
    }
}

///////////////////////////////////////////////////////////////

CDib::CDib(CDib&& rhs) noexcept :
    CDib()
{
    *this = std::move(rhs);
}

CDib& CDib::operator=(CDib&& rhs) noexcept
{
    m_hDib = std::move(rhs.m_hDib);
    std::swap(m_nCompressLevel, rhs.m_nCompressLevel);
    return *this;
}

void CDib::ClearDib()
{
    m_hDib = nullptr;
}

CDib::CDib(DWORD dwWidth, DWORD dwHeight)
{
    m_hDib = CBITMAPINFOHEADER(value_preserving_cast<int32_t>(dwWidth), value_preserving_cast<int32_t>(dwHeight), size_t(24));
    m_nCompressLevel = 0;
}

CDib::CDib(const CBitmap& pBM, const CPalette* pPal)
{
    if (pBM.m_hObject != NULL)
    {
        m_hDib = CBITMAPINFOHEADER((HBITMAP)(pBM.m_hObject),
            (HPALETTE)(pPal ? pPal->m_hObject : NULL));
    }
    else
    {
        m_hDib = nullptr;
    }
    m_nCompressLevel = 0;
}

OwnerPtr<CBitmap> CDib::DIBToBitmap() const
{
    const BITMAPINFOHEADER& pbmiDib = m_hDib;

    OwnerPtr<CBitmap> pBMap = CreateDIBSection(pbmiDib.biWidth,
        pbmiDib.biHeight);

    ASSERT(pbmiDib.biBitCount == 24);
    // DIBs are bottom up
    const std::byte* src = static_cast<const std::byte*>(m_hDib.DibXY(0, pbmiDib.biHeight - 1));
    std::byte* dest = static_cast<std::byte*>(Ptr(*pBMap, size_t(0), value_preserving_cast<size_t>(pbmiDib.biHeight - 1)));
    memcpy(dest, src, value_preserving_cast<size_t>(pbmiDib.biHeight) * WidthBytes(pbmiDib));
    pBMap->SetBitmapDimension(Width(), Height());
    return pBMap;
}

OwnerPtr<CBitmap> CDib::CreateDIBSection(int nWidth, int nHeight)
{
    OwnerPtr<CBitmap> retval = MakeOwner<CBitmap>();

    CBITMAPINFOHEADER bmi(nWidth, nHeight, size_t(24));

    VOID* pBits;
    HBITMAP hBmap = ::CreateDIBSection(NULL, reinterpret_cast<BITMAPINFO*>(static_cast<BITMAPINFOHEADER*>(bmi)), DIB_RGB_COLORS,
        &pBits, NULL, 0);
    if (!hBmap)
    {
        AfxThrowMemoryException();
    }

    retval->Attach(hBmap);
    return retval;
}

///////////////////////////////////////////////////////////////

CArchive& AFXAPI operator<<(CArchive& ar, const CDib& dib)
{
    /* TODO:  Are bmps >= 2g possible?  If so, and we support
                them, then file format must change */
    if (dib.m_hDib)
    {
        // mem format is 24bit, but file format is still 16bit
        ASSERT(dib.m_hDib.get().biBitCount == 24);
        OwnerOrNullPtr<CDib::CBITMAPINFOHEADER> converted;
        if (dib.m_hDib.get().biBitCount != 16)
        {
            converted = MakeOwner<CDib::CBITMAPINFOHEADER>(dib.m_hDib, uint16_t(16));
        }
        const CDib::CBITMAPINFOHEADER& serialize = converted ? *converted : dib.m_hDib;
        uint32_t dwSize = value_preserving_cast<uint32_t>(serialize.size());
        ASSERT(dwSize > uint32_t(0));
        ASSERT(dib.m_nCompressLevel >= wxZ_NO_COMPRESSION
            && dib.m_nCompressLevel <= wxZ_BEST_COMPRESSION);
        if (dib.m_nCompressLevel > wxZ_NO_COMPRESSION)
        {
            // Store size of the uncompressed dib bfr with upper bit set.
            // The set upper bit allows us to detect loading of uncompressed
            // bitmaps.
            if (dwSize & uint32_t(0x80000000))
            {
                AfxThrowMemoryException();
            }
            ar << (dwSize | uint32_t(0x80000000));

            // Use zlib to compress the dib before writing it out the the archive.
            uint32_t dwDestLen = value_preserving_cast<uint32_t>(MulDiv(value_preserving_cast<int>(dwSize), 1001, 1000) + 12);
            std::vector<std::byte> pDestBfr(dwDestLen);
            wxMemoryOutputStream mstream(pDestBfr.data(), pDestBfr.size());
            {
                wxZlibOutputStream zstream(mstream, dib.m_nCompressLevel, wxZLIB_NO_HEADER | wxZLIB_ZLIB);
                bool rc = zstream.WriteAll(&serialize.get(), dwSize);
                if (!rc)
                {
                    AfxThrowMemoryException();
                }
            }
            dwDestLen = value_preserving_cast<uint32_t>(mstream.TellO());
            ar << dwDestLen;                    // Store the compressed size of the bitmap
            ar.Write(pDestBfr.data(), dwDestLen);      // Store the compressed bitmap
        }
        else
        {
            // Store size of the raw dib
            ar << dwSize;
            ar.Write(&serialize.get(), value_preserving_cast<UINT>(dwSize));      // Store the uncompressed bitmap
        }
    }
    else
        ar << (DWORD)0;
    return ar;
}

CArchive& AFXAPI operator>>(CArchive& ar, CDib& dib)
{
    dib.ClearDib();
    uint32_t dwSize;
    ar >> dwSize;
    if ((dwSize & uint32_t(0x80000000)) != uint32_t(0))
    {
        // Load compressed bitmap...
        dwSize &= uint32_t(0x7FFFFFFF);       // Remove flag bit

        uint32_t dwCompSize;
        ar >> dwCompSize;           // Get compressed data size

        std::vector<std::byte> pCompBfr(dwCompSize);

        dib.m_hDib.reserve(dwSize);

        ar.Read(pCompBfr.data(), dwCompSize);

        wxMemoryInputStream mstream(pCompBfr.data(), pCompBfr.size());
        wxZlibInputStream zstream(mstream, wxZLIB_NO_HEADER | wxZLIB_ZLIB);
        bool rc = zstream.ReadAll(dib.m_hDib, dwSize);
        ASSERT(zstream.LastRead() == dwSize);

        if (!rc)
            AfxThrowMemoryException();
    }
    else if (dwSize > uint32_t(0))
    {
        // Load uncompressed bitmap...
        dib.m_hDib.reserve(dwSize);

        ar.Read(dib.m_hDib, dwSize);
    }
    if (dib.m_hDib)
    {
        // mem format is 24bit, but old files aren't
        if (dib.m_hDib.get().biBitCount != 24)
        {
            dib.m_hDib = CDib::CBITMAPINFOHEADER(dib.m_hDib, size_t(24));
        }
    }
    return ar;
}

wxImage ToImage(const CBitmap& bmp)
{
    BITMAP info;
    if (!const_cast<CBitmap&>(bmp).GetBitmap(&info))
    {
        AfxThrowMemoryException();
    }
    ASSERT(info.bmWidth > 0 && info.bmHeight > 0);
    wxImage retval(info.bmWidth, info.bmHeight, false);
    switch (info.bmBitsPixel)
    {
        case 16:
        {
            ASSERT(!"untested code");
            // DIBS are bottom up;
            const uint16_t* srcRowStart = static_cast<const uint16_t*>(Ptr(bmp, size_t(0), size_t(0)));
            ptrdiff_t srcStride = -info.bmWidthBytes / ptrdiff_t(sizeof(*srcRowStart));
            for (int y = 0 ; y < info.bmHeight ; ++y)
            {
                const uint16_t* src = srcRowStart;
                for (int x = 0 ; x < info.bmWidth ; ++x)
                {
                    COLORREF cr = RGB565_TO_24(*src++);
                    retval.SetRGB(x, y,
                                    GetRValue(cr),
                                    GetGValue(cr),
                                    GetBValue(cr));
                }
                srcRowStart += srcStride;
            }
            break;
        }
        case 24:
        {
            // DIBS are bottom up;
            const std::byte* srcRowStart = static_cast<const std::byte*>(Ptr(bmp, size_t(0), size_t(0)));
            ptrdiff_t srcStride = -info.bmWidthBytes;
            for (int y = 0 ; y < info.bmHeight ; ++y)
            {
                const WIN_RGBTRIO* src = reinterpret_cast<const WIN_RGBTRIO*>(srcRowStart);
                for (int x = 0 ; x < info.bmWidth ; ++x)
                {
                    retval.SetRGB(x, y,
                                    src->red,
                                    src->green,
                                    src->blue);
                    ++src;
                }
                srcRowStart += srcStride;
            }
            break;
        }
        default:
            AfxThrowNotSupportedException();
    }
    return retval;
}

OwnerPtr<CBitmap> ToBitmap(const wxImage& img)
{
    if (img.HasAlpha() || img.HasMask())
    {
        AfxThrowNotSupportedException();
    }

    OwnerPtr<CBitmap> retval = CDib::CreateDIBSection(img.GetWidth(), img.GetHeight());

    DIBSECTION dibSect;
    VERIFY(GetObject(*retval, sizeof(DIBSECTION), &dibSect));
    ASSERT(dibSect.dsBm.bmWidth == img.GetWidth() &&
            dibSect.dsBm.bmHeight == img.GetHeight() &&
            dibSect.dsBmih.biBitCount == 24);

    // DIBs are bottom up
    std::byte* destRowStart = static_cast<std::byte*>(Ptr(*retval, size_t(0), size_t(0)));
    ptrdiff_t destStride = -value_preserving_cast<ptrdiff_t>(CDib::WidthBytes(dibSect.dsBmih));

    for (int y = 0 ; y < dibSect.dsBm.bmHeight ; ++y)
    {
        WIN_RGBTRIO* dest = reinterpret_cast<WIN_RGBTRIO*>(destRowStart);
        for (int x = 0 ; x < dibSect.dsBm.bmWidth ; ++x)
        {
            dest->red = img.GetRed(x, y);
            dest->green = img.GetGreen(x, y);
            dest->blue = img.GetBlue(x, y);
            ++dest;
        }
        destRowStart += destStride;
    }

    return retval;
}
