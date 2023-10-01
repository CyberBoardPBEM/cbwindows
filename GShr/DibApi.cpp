//  dibapi.cpp
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include    "stdafx.h"
#include    <io.h>
#include    <errno.h>
#include    "GdiTools.h"
#include    "DibApi.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

/*
 * Dib Header Marker - used in writing DIBs to files
 */

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

///////////////////////////////////////////////////////////////////////

CBITMAPINFOHEADER BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal, uint16_t nBPP)
{
    CBITMAPINFOHEADER hDIB = NULL;
    if (!hBitmap)
        return NULL;
    // If the target format is 16 bits per pixel we avoid the use
    // of GetDIBits() since it forces 555 format and we desire 565
    // format.
    if (nBPP == 16)
    {
        HDC hMemDCScreen = NULL;
        HDC hMemDCSrc = NULL;
        HDC hMemDCSect = NULL;
        HBITMAP hBmapSect = NULL;
        HBITMAP hPrvBmapSrc = NULL;
        HBITMAP hPrvBmapSect = NULL;
        HPALETTE hPrvPal = NULL;
        LPVOID pBits = NULL;

        // If the bitmap is already a DIB section of the proper format
        // we can just directly make a DIB.
        DIBSECTION dibSect;
        if (GetObject(hBitmap, sizeof(dibSect), (LPVOID)&dibSect))
        {
            if (dibSect.dsBmih.biBitCount == 16 && dibSect.dsBitfields[0] == 0xF800)
                return ConvertDIBSectionToDIB(hBitmap);
        }

        ASSERT(!"untested code");
        BITMAP bmapSrc;                 // Source bitmap
        if (!GetObject(hBitmap, sizeof(bmapSrc), (LPVOID)&bmapSrc))
            return NULL;

        // To force the proper 16bit bitmap format we'll use
        // DIB sections. A DIB section of the desired format is
        // created and the bitmap is Blited into it. Then the
        // section is converted to a DIB.

        CBITMAPINFOHEADER bmi(bmapSrc.bmWidth, bmapSrc.bmHeight, uint16_t(16));

        // We need a reference DC for palette bitmaps
        hMemDCScreen = GetDC(NULL);
        hMemDCSrc = CreateCompatibleDC(hMemDCScreen);
        ReleaseDC(NULL, hMemDCSrc);

        if (hPal)
        {
            ASSERT(!"untested code");
            hPrvPal = SelectPalette(hMemDCSrc, hPal, FALSE);
            RealizePalette(hMemDCSrc);
        }

        hBmapSect = CreateDIBSection(hMemDCSrc, bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        if (hBmapSect != NULL)
        {
            hMemDCSect = CreateCompatibleDC(hMemDCScreen);
            hPrvBmapSect = (HBITMAP)SelectObject(hMemDCSect, hBmapSect);

            hPrvBmapSrc = (HBITMAP)SelectObject(hMemDCSrc, hBitmap);

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
        hDIB = ConvertDIBSectionToDIB(hBmapSect);
        DeleteObject(hBmapSect);
    }
    else
    {
        ASSERT(!"untested code");
        BITMAP             Bitmap;
        void*              lpBits;
        HDC                hMemDC;
        HPALETTE           hOldPal = NULL;

        // Do some setup -- make sure the Bitmap passed in is valid,
        // get info on the bitmap (like its height, width, etc.),
        // then setup a BITMAPINFOHEADER.

        if (!hBitmap)
            return NULL;

        if (!GetObject(hBitmap, sizeof(Bitmap), &Bitmap))
            return NULL;

        // Now allocate memory for the DIB.  Then, set the BITMAPINFOHEADER
        // into this memory, and find out where the bitmap bits go.

        hDIB = CBITMAPINFOHEADER(Bitmap.bmWidth, Bitmap.bmHeight,
            nBPP == 0 ? value_preserving_cast<uint16_t>(Bitmap.bmPlanes * Bitmap.bmBitsPixel) : nBPP);

        lpBits = hDIB.GetBits();

        // Now, we need a DC to hold our bitmap.  If the app passed us
        //  a palette, it should be selected into the DC.

        hMemDC = GetDC(NULL);

        if (hPal)
        {
            ASSERT(!"untested code");
            hOldPal = SelectPalette(hMemDC, hPal, FALSE);
            RealizePalette(hMemDC);
        }
        // We're finally ready to get the DIB.  Call the driver and let
        // it party on our bitmap.  It will fill in the color table,
        // and bitmap bits of our global memory block.
        if (!GetDIBits(hMemDC, hBitmap, 0, Bitmap.bmHeight, lpBits,
            hDIB, DIB_RGB_COLORS))
        {
            hDIB = NULL;
        }

        // Finally, clean up and return.
        if (hOldPal)
            SelectPalette(hMemDC, hOldPal, FALSE);

        ReleaseDC(NULL, hMemDC);
    }
    return hDIB;
}

///////////////////////////////////////////////////////////////////////

CBITMAPINFOHEADER ConvertDIBSectionToDIB(HBITMAP hDibSect)
{
    DIBSECTION dibSect;
    if (!GetObject(hDibSect, sizeof(dibSect), (LPVOID)&dibSect))
        return nullptr;

    ASSERT(dibSect.dsBmih.biBitCount == 16);            // Only support this
    CBITMAPINFOHEADER hDIB(dibSect.dsBmih.biWidth, dibSect.dsBmih.biHeight, dibSect.dsBmih.biBitCount);

    BITMAPINFO* pbmInfoHdr = hDIB;
    DWORD* pdwMasks = reinterpret_cast<DWORD*>(pbmInfoHdr->bmiColors);

    pbmInfoHdr->bmiHeader = dibSect.dsBmih;

    pdwMasks[0] = dibSect.dsBitfields[0];
    pdwMasks[1] = dibSect.dsBitfields[1];
    pdwMasks[2] = dibSect.dsBitfields[2];

    void* pBits = hDIB.GetBits();

    memcpy(pBits, dibSect.dsBm.bmBits, dibSect.dsBmih.biSizeImage);

    return hDIB;
}
