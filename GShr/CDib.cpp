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

WORD CDib::Get16BitColorNumberAtXY(int x, int y) const
{
    ASSERT(m_hDib != NULL);
    ASSERT(x >= 0 && x < Width());
    ASSERT(y >= 0 && y < Height());
    return *((WORD*)::DibXY(m_lpDib, x, y));
}

void CDib::Set16BitColorNumberAtXY(int x, int y, WORD nColor)
{
    ASSERT(m_hDib != NULL);
    ASSERT(x >= 0 && x < Width());
    ASSERT(y >= 0 && y < Height());
    *((WORD*)::DibXY(m_lpDib, x, y)) = nColor;
}

///////////////////////////////////////////////////////////////

CDib::CDib(CDib&& rhs) noexcept :
    CDib()
{
    *this = std::move(rhs);
}

CDib& CDib::operator=(CDib&& rhs) noexcept
{
    std::swap(m_hDib, rhs.m_hDib);
    std::swap(m_lpDib, rhs.m_lpDib);
    std::swap(m_nCompressLevel, rhs.m_nCompressLevel);
    return *this;
}

void CDib::ClearDib()
{
    if (m_hDib)
    {
        GlobalUnlock((HGLOBAL)m_hDib);
        GlobalFree((HGLOBAL)m_hDib);
        m_hDib = NULL;
        m_lpDib = NULL;
    }
}

CDib::CDib(DWORD dwWidth, DWORD dwHeight, WORD wBPP /* = 16 */)
{
    m_hDib = ::CreateDIB(dwWidth, dwHeight, wBPP);
    if (!m_hDib)
    {
        AfxThrowMemoryException();
    }
    m_lpDib = ::GlobalLock((HGLOBAL)m_hDib);
    if (!m_lpDib)
    {
        GlobalFree((HGLOBAL)m_hDib);
        m_hDib = NULL;
        AfxThrowMemoryException();
    }
    m_nCompressLevel = 0;
}

CDib::CDib(HANDLE hDib)
{
    m_hDib = (HDIB)hDib;
    if (m_hDib == NULL)
    {
        m_lpDib = nullptr;
        return;
    }
    m_lpDib = ::GlobalLock((HGLOBAL)m_hDib);
    if (!m_lpDib)
    {
        GlobalFree((HGLOBAL)m_hDib);
        m_hDib = NULL;
        AfxThrowMemoryException();
    }
    m_nCompressLevel = 0;
}

CDib::CDib(const CBitmap& pBM, const CPalette* pPal, uint16_t nBPP/* = uint16_t(16)*/)
{
    if (pBM.m_hObject != NULL)
    {
        m_hDib = (HDIB)::BitmapToDIB((HBITMAP)(pBM.m_hObject),
            (HPALETTE)(pPal ? pPal->m_hObject : NULL), nBPP);
        if (!m_hDib)
        {
            AfxThrowMemoryException();
        }
        m_lpDib = ::GlobalLock((HGLOBAL)m_hDib);
        if (!m_lpDib)
        {
            GlobalFree((HGLOBAL)m_hDib);
            m_hDib = NULL;
            AfxThrowMemoryException();
        }
    }
    else
    {
        m_hDib = nullptr;
        m_lpDib = nullptr;
    }
    m_nCompressLevel = 0;
}

OwnerPtr<CBitmap> CDib::DIBToBitmap(const CPalette *pPal, BOOL bDibSect /* = TRUE */) const
{
    if (bDibSect)
    {
        // This weird code is used to xfer the dib into a dib section
        // having a standard color table.
        BITMAPINFO* pbmiDib = (BITMAPINFO*)m_lpDib;
        CWindowDC scrnDC(NULL);
        CDC memDC;
        memDC.CreateCompatibleDC(&scrnDC);
        CPalette* prvPal = memDC.SelectPalette(GetAppPalette(), FALSE);
        memDC.RealizePalette();

        HBITMAP hDibSect =
            Create16BitDIBSection(pbmiDib->bmiHeader.biWidth,
            pbmiDib->bmiHeader.biHeight);

        SetDIBits(NULL, hDibSect, 0,
            pbmiDib->bmiHeader.biHeight, FindDIBBits(pbmiDib), pbmiDib,
            DIB_RGB_COLORS);

        memDC.SelectPalette(prvPal, FALSE);

        OwnerPtr<CBitmap> pBMap(MakeOwner<CBitmap>());
        pBMap->Attach((HGDIOBJ)hDibSect);
        ASSERT(Width() == pbmiDib->bmiHeader.biWidth &&
                Height() == pbmiDib->bmiHeader.biHeight);
        pBMap->SetBitmapDimension(Width(), Height());
        return pBMap;

    }
    else
    {
        HBITMAP hBMap = ::DIBToBitmap(m_hDib,
            (HPALETTE)(pPal ? pPal->m_hObject : NULL));
        ASSERT(hBMap != NULL);
        if (hBMap != NULL)
        {
            OwnerPtr<CBitmap> pBMap(MakeOwner<CBitmap>());
            pBMap->Attach((HGDIOBJ)hBMap);
            pBMap->SetBitmapDimension(Width(), Height());
            return pBMap;
        }
        else
            AfxThrowInvalidArgException();
    }
}

///////////////////////////////////////////////////////////////

CArchive& AFXAPI operator<<(CArchive& ar, const CDib& dib)
{
    /* TODO:  Are bmps >= 2g possible?  If so, and we support
                them, then file format must change */
    if (dib.m_hDib)
    {
        uint32_t dwSize = value_preserving_cast<uint32_t>(GlobalSize(dib.m_hDib));
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
                bool rc = zstream.WriteAll(dib.m_lpDib, dwSize);
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
            ar.Write(dib.m_lpDib, value_preserving_cast<UINT>(dwSize));      // Store the uncompressed bitmap
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

        if ((dib.m_hDib = (HDIB)GlobalAlloc(GHND, dwSize)) == NULL)
            AfxThrowMemoryException();
        dib.m_lpDib = GlobalLock((HGLOBAL)dib.m_hDib);

        ar.Read(pCompBfr.data(), dwCompSize);

        wxMemoryInputStream mstream(pCompBfr.data(), pCompBfr.size());
        wxZlibInputStream zstream(mstream, wxZLIB_NO_HEADER | wxZLIB_ZLIB);
        bool rc = zstream.ReadAll(dib.m_lpDib, dwSize);
        ASSERT(zstream.LastRead() == dwSize);

        if (!rc)
            AfxThrowMemoryException();
    }
    else if (dwSize > uint32_t(0))
    {
        // Load uncompressed bitmap...
        if ((dib.m_hDib = (HDIB)GlobalAlloc(GHND, dwSize)) == NULL)
            AfxThrowMemoryException();
        dib.m_lpDib = GlobalLock((HGLOBAL)dib.m_hDib);

        ar.Read(dib.m_lpDib, dwSize);
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

    OwnerPtr<CBitmap> retval(MakeOwner<CBitmap>());
    HBITMAP hbmp = Create16BitDIBSection(img.GetWidth(), img.GetHeight());
    if (!hbmp)
    {
        AfxThrowMemoryException();
    }
    retval->Attach(hbmp);

    DIBSECTION dibSect;
    VERIFY(GetObject(hbmp, sizeof(DIBSECTION), &dibSect));
    ASSERT(dibSect.dsBm.bmWidth == img.GetWidth() &&
            dibSect.dsBm.bmHeight == img.GetHeight() &&
            dibSect.dsBmih.biBitCount == 16 &&
            dibSect.dsBitfields[0] == 0xf800 &&
            dibSect.dsBitfields[1] == 0x07e0 &&
            dibSect.dsBitfields[2] == 0x001f);
    size_t lineBytes = DIBWIDTHBYTES(dibSect.dsBmih);

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
