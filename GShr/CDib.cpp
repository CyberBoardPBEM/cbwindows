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

///////////////////////////////////////////////////////////////

CDib::CBITMAPINFOHEADER::CBITMAPINFOHEADER(int32_t dwWidth, int32_t dwHeight, uint16_t wBitCount)
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
    bi.biBitCount = wBitCount;
    bi.biCompression = wBitCount == 16 ? BI_BITFIELDS : BI_RGB;

    size_t dataSize = BitsToBytes(bi.biWidth * bi.biBitCount) * value_preserving_cast<size_t>(bi.biHeight);
    reserve(bi.biSize +
            (wBitCount <= size_t(16) ? GetPaletteSize(bi) : uint16_t(0)) +
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

    CBITMAPINFOHEADER hDIB;
    // The target format is 16 bits per pixel, so we avoid the use
    // of GetDIBits() since it forces 555 format and we desire 565
    // format.

    // If the bitmap is already a DIB section of the proper format
    // we can just directly make a DIB.
    DIBSECTION dibSect;
    if (GetObject(hBitmap, sizeof(dibSect), &dibSect))
    {
        if (dibSect.dsBmih.biBitCount == 16 && dibSect.dsBitfields[0] == 0xF800)
        {
            *this = CBITMAPINFOHEADER(hBitmap);
            return;
        }
    }

    ASSERT(!"untested code");
    BITMAP bmapSrc;                 // Source bitmap
    if (!GetObject(hBitmap, sizeof(bmapSrc), &bmapSrc))
    {
        CbThrowBadCastException();
    }

    // To force the proper 16bit bitmap format we'll use
    // DIB sections. A DIB section of the desired format is
    // created and the bitmap is Blited into it. Then the
    // section is converted to a DIB.

    CBITMAPINFOHEADER bmi(bmapSrc.bmWidth, bmapSrc.bmHeight, uint16_t(16));

    // We need a reference DC for palette bitmaps
    HDC hMemDCScreen = GetDC(NULL);
    // WARNING:  this looks strange, but is copied from the original code
    HDC hMemDCSrc = CreateCompatibleDC(hMemDCScreen);
    ReleaseDC(NULL, hMemDCSrc);

    HPALETTE hPrvPal = NULL;
    if (hPal)
    {
        ASSERT(!"untested code");
        hPrvPal = SelectPalette(hMemDCSrc, hPal, FALSE);
        RealizePalette(hMemDCSrc);
    }

    LPVOID pBits = NULL;
    HBITMAP hBmapSect = ::CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    if (hBmapSect != NULL)
    {
        HDC hMemDCSect = CreateCompatibleDC(hMemDCScreen);
        HBITMAP hPrvBmapSect = (HBITMAP)SelectObject(hMemDCSect, hBmapSect);

        HBITMAP hPrvBmapSrc = (HBITMAP)SelectObject(hMemDCSrc, hBitmap);

        BitBlt(hMemDCSect, 0, 0, bmapSrc.bmWidth, bmapSrc.bmHeight,
            hMemDCSrc, 0, 0, SRCCOPY);

        SelectObject(hMemDCSect, hPrvBmapSect);
        SelectObject(hMemDCSrc, hPrvBmapSrc);

        DeleteDC(hMemDCSect);
    }

    if (hPrvPal)
        SelectPalette(hMemDCSrc, hPrvPal, FALSE);
    DeleteDC(hMemDCSrc);

    // Now that we have the converted bitmap create the
    // actual DIB.
    hDIB = CBITMAPINFOHEADER(hBmapSect);
    DeleteObject(hBmapSect);

    if (!hDIB)
    {
        CbThrowBadCastException();
    }
    *this = std::move(hDIB);
}

CDib::CBITMAPINFOHEADER::CBITMAPINFOHEADER(HBITMAP hDibSect)
{
    DIBSECTION dibSect;
    if (!GetObject(hDibSect, sizeof(dibSect), &dibSect))
    {
        CbThrowBadCastException();
    }

    ASSERT(dibSect.dsBmih.biBitCount == 16);            // Only support this
    *this = CBITMAPINFOHEADER(dibSect.dsBmih.biWidth, dibSect.dsBmih.biHeight, dibSect.dsBmih.biBitCount);

    BITMAPINFO& pbmInfoHdr = *this;
    DWORD* pdwMasks = reinterpret_cast<DWORD*>(pbmInfoHdr.bmiColors);

    pbmInfoHdr.bmiHeader = dibSect.dsBmih;

    ASSERT(pdwMasks[0] == dibSect.dsBitfields[0]);
    ASSERT(pdwMasks[1] == dibSect.dsBitfields[1]);
    ASSERT(pdwMasks[2] == dibSect.dsBitfields[2]);

    memcpy(GetBits(), dibSect.dsBm.bmBits, dibSect.dsBmih.biSizeImage);
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
    return(reinterpret_cast<const std::byte*>(&lpbi) + lpbi.biSize + GetPaletteSize(lpbi));
}

const void* CDib::CBITMAPINFOHEADER::DibXY(ptrdiff_t x, ptrdiff_t y) const
{
    const BITMAPINFOHEADER& lpbi = *this;
    const std::byte* pBits = reinterpret_cast<const std::byte*>(&lpbi) + lpbi.biSize + GetPaletteSize(lpbi);
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

uint16_t CDib::CBITMAPINFOHEADER::GetPaletteSize(const BITMAPINFOHEADER& lpbi)
{
    if (lpbi.biBitCount == 24)
    {
        return uint16_t(0);
    }
    else
    {
        return value_preserving_cast<uint16_t>(GetNumColors(lpbi) * sizeof(RGBQUAD));
    }
}

///////////////////////////////////////////////////////////////////////

uint16_t CDib::CBITMAPINFOHEADER::GetNumColors(const BITMAPINFOHEADER& lpbi)
{
    // check for explicit count
    if (lpbi.biClrUsed != 0)
    {
        return value_preserving_cast<uint16_t>(lpbi.biClrUsed);
    }

    switch (lpbi.biBitCount)
    {
        case 1:
        case 4:
        case 8:
            return static_cast<uint16_t>(1 << lpbi.biBitCount);
        case 16:
            return uint16_t(3);       // Special: Used only for the pixel bit masks
        default:
            AfxThrowNotSupportedException();
    }
}

///////////////////////////////////////////////////////////////

WORD CDib::Get16BitColorNumberAtXY(int x, int y) const
{
    ASSERT(m_hDib && NumColorBits() == 16);
    ASSERT(x >= 0 && x < Width());
    ASSERT(y >= 0 && y < Height());
    return *((WORD*)m_hDib.DibXY(x, y));
}

void CDib::Set16BitColorNumberAtXY(int x, int y, WORD nColor)
{
    ASSERT(m_hDib && NumColorBits() == 16);
    ASSERT(x >= 0 && x < Width());
    ASSERT(y >= 0 && y < Height());
    *((WORD*)m_hDib.DibXY(x, y)) = nColor;
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
    m_hDib = CBITMAPINFOHEADER(value_preserving_cast<int32_t>(dwWidth), value_preserving_cast<int32_t>(dwHeight), uint16_t(16));
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
    // This weird code is used to xfer the dib into a dib section
    // having a standard color table.
    const BITMAPINFO& pbmiDib = m_hDib;
    CWindowDC scrnDC(NULL);
    CDC memDC;
    memDC.CreateCompatibleDC(&scrnDC);
    CPalette* prvPal = memDC.SelectPalette(GetAppPalette(), FALSE);
    memDC.RealizePalette();

    OwnerPtr<CBitmap> pBMap = Create16BitDIBSection(pbmiDib.bmiHeader.biWidth,
        pbmiDib.bmiHeader.biHeight);

    SetDIBits(NULL, *pBMap, 0,
        pbmiDib.bmiHeader.biHeight, m_hDib.GetBits(), &pbmiDib,
        DIB_RGB_COLORS);

    memDC.SelectPalette(prvPal, FALSE);

    ASSERT(Width() == pbmiDib.bmiHeader.biWidth &&
            Height() == pbmiDib.bmiHeader.biHeight);
    pBMap->SetBitmapDimension(Width(), Height());
    return pBMap;
}

OwnerPtr<CBitmap> CDib::CreateDIBSection(int nWidth, int nHeight, uint16_t nBPP)
{
    OwnerPtr<CBitmap> retval = MakeOwner<CBitmap>();

    CBITMAPINFOHEADER bmi(nWidth, nHeight, nBPP);

    VOID* pBits;
    HBITMAP hBmap = ::CreateDIBSection(NULL, bmi, DIB_RGB_COLORS,
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
        uint32_t dwSize = value_preserving_cast<uint32_t>(dib.m_hDib.size());
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
                bool rc = zstream.WriteAll(&dib.m_hDib.get(), dwSize);
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
            ar.Write(&dib.m_hDib.get(), value_preserving_cast<UINT>(dwSize));      // Store the uncompressed bitmap
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
            const uint16_t* srcLine = static_cast<const uint16_t*>(info.bmBits);
            for (int y = 0 ; y < info.bmHeight ; ++y)
            {
                const uint16_t* src = srcLine;
                for (int x = 0 ; x < info.bmWidth ; ++x)
                {
                    COLORREF cr = RGB565_TO_24(*src);
                    // KLUDGE:  data seems to be bottom up
                    retval.SetRGB(x, info.bmHeight - 1 - y,
                                    GetRValue(cr),
                                    GetGValue(cr),
                                    GetBValue(cr));
                    ++src;
                }
                srcLine += info.bmWidthBytes/sizeof(*srcLine);
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

    OwnerPtr<CBitmap> retval = Create16BitDIBSection(img.GetWidth(), img.GetHeight());

    DIBSECTION dibSect;
    VERIFY(GetObject(*retval, sizeof(DIBSECTION), &dibSect));
    ASSERT(dibSect.dsBm.bmWidth == img.GetWidth() &&
            dibSect.dsBm.bmHeight == img.GetHeight() &&
            dibSect.dsBmih.biBitCount == 16 &&
            dibSect.dsBitfields[0] == 0xf800 &&
            dibSect.dsBitfields[1] == 0x07e0 &&
            dibSect.dsBitfields[2] == 0x001f);
    size_t lineBytes = CDib::WidthBytes(dibSect.dsBmih);

    uint16_t* lineStart = static_cast<uint16_t*>(dibSect.dsBm.bmBits);
    for (int y = 0; y < dibSect.dsBm.bmHeight ; ++y)
    {
        // KLUDGE:  dibsection data is bottom up
        int imgY = dibSect.dsBm.bmHeight - y - 1;
        uint16_t* rgb565 = lineStart;
        for (int x = 0; x < dibSect.dsBm.bmWidth ; ++x)
        {
            uint8_t red = img.GetRed(x, imgY);
            uint8_t green = img.GetGreen(x, imgY);
            uint8_t blue = img.GetBlue(x, imgY);
            *rgb565++ = RGB565(RGB(red, green, blue));
        }
        reinterpret_cast<std::byte*&>(lineStart) += lineBytes;
    }

    return retval;
}
