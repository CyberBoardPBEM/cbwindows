// CDib.cpp
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

#include    "stdafx.h"
#include    "GdiTools.h"
#include    "CDib.h"
#include    "zlib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////

static UINT CalcSize(DWORD cbTotal, const void FAR* lpStart);

///////////////////////////////////////////////////////////////

BYTE CDib::Get256ColorNumberAtXY(int x, int y)
{
    ASSERT(m_hDib != NULL);
    ASSERT(x >= 0 && x < Width());
    ASSERT(y >= 0 && y < Height());
    return *::DibXY(m_lpDib, x, y);
}

void CDib::Set256ColorNumberAtXY(int x, int y, BYTE nColor)
{
    ASSERT(m_hDib != NULL);
    ASSERT(x >= 0 && x < Width());
    ASSERT(y >= 0 && y < Height());
    *::DibXY(m_lpDib, x, y) = nColor;
}

WORD CDib::Get16BitColorNumberAtXY(int x, int y)
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

void CDib::CreateDIB(DWORD dwWidth, DWORD dwHeight, WORD wBPP /* = 16 */)
{
    ClearDib();
    m_hDib = ::CreateDIB(dwWidth, dwHeight, wBPP);
    if (!m_hDib)
    {
        AfxThrowMemoryException();
    }
    m_lpDib = (LPSTR)::GlobalLock((HGLOBAL)m_hDib);
    if (!m_lpDib)
    {
        GlobalFree((HGLOBAL)m_hDib);
        m_hDib = NULL;
        AfxThrowMemoryException();
    }
}

void CDib::ReadDIBFile(CFile& file)
{
    ClearDib();
    m_hDib = ::ReadDIBFile(file);
    if (!m_hDib)
    {
        AfxThrowMemoryException();
    }
    m_lpDib = (LPSTR)::GlobalLock((HGLOBAL)m_hDib);
    if (!m_lpDib)
    {
        GlobalFree((HGLOBAL)m_hDib);
        m_hDib = NULL;
        AfxThrowMemoryException();
    }
}

BOOL CDib::WriteDIBFile(CFile& file)
{
    if (m_hDib)
        return ::SaveDIB(m_hDib, file);
    else
        return FALSE;
}

void CDib::CloneDIB(CDib *pDib)
{
    ClearDib();
    m_hDib = (HDIB)CopyHandle(pDib->m_hDib);
    if (!m_hDib)
    {
        AfxThrowMemoryException();
    }
    m_lpDib = (LPSTR)::GlobalLock((HGLOBAL)m_hDib);
    if (!m_lpDib)
    {
        GlobalFree((HGLOBAL)m_hDib);
        m_hDib = NULL;
        AfxThrowMemoryException();
    }
}

void CDib::SetDibHandle(HANDLE hDib)
{
    ClearDib();
    m_hDib = (HDIB)hDib;
    if (m_hDib == NULL)
        return;
    m_lpDib = (LPSTR)::GlobalLock((HGLOBAL)m_hDib);
    if (!m_lpDib)
    {
        GlobalFree((HGLOBAL)m_hDib);
        m_hDib = NULL;
        AfxThrowMemoryException();
    }
}

BOOL CDib::BitmapToDIB(const CBitmap* pBM, CPalette* pPal, int nBPP/* = 16*/)
{
    ClearDib();
    if (pBM->m_hObject != NULL)
    {
        m_hDib = (HDIB)::BitmapToDIB((HBITMAP)(pBM->m_hObject),
            (HPALETTE)(pPal ? pPal->m_hObject : NULL), nBPP);
        if (!m_hDib)
        {
            AfxThrowMemoryException();
        }
        m_lpDib = (LPSTR)::GlobalLock((HGLOBAL)m_hDib);
        if (!m_lpDib)
        {
            GlobalFree((HGLOBAL)m_hDib);
            m_hDib = NULL;
            AfxThrowMemoryException();
        }
    }
    return m_hDib != NULL;
}

OwnerPtr<CBitmap> CDib::DIBToBitmap(CPalette *pPal, BOOL bDibSect /* = TRUE */)
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
            Create16BitDIBSection(memDC.m_hDC, pbmiDib->bmiHeader.biWidth,
            pbmiDib->bmiHeader.biHeight);

        SetDIBits(memDC.m_hDC, hDibSect, 0,
            pbmiDib->bmiHeader.biHeight, FindDIBBits((char*)pbmiDib), pbmiDib,
            DIB_RGB_COLORS);

        memDC.SelectPalette(prvPal, FALSE);

        OwnerPtr<CBitmap> pBMap(MakeOwner<CBitmap>());
        pBMap->Attach((HGDIOBJ)hDibSect);
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
            return pBMap;
        }
        else
            AfxThrowInvalidArgException();
    }
}

BOOL CDib::RemoveDIBSlice(int y, int ht)
{
    //WE REALLY COULD DO THIS IN PLACE BUT I'M LAZY TODAY...
    CPalette pal;
    CreatePalette(&pal);

    CWindowDC scrnDC(NULL);
    CDC memDC;
    memDC.CreateCompatibleDC(&scrnDC);

    CPalette* prvPal = memDC.SelectPalette(&pal, TRUE);

    CBitmap bmap;
    bmap.Attach(Create16BitDIBSection(memDC.m_hDC,
        Width(), Height() - ht));

    CBitmap* prvBMap = memDC.SelectObject(&bmap);
    memDC.RealizePalette();

    if (y + ht < Height())
        StretchDIBits(&memDC, 0, 0, Width(), Height() - (y + ht),
            0, y + ht, Width(), Height() - (y + ht));
    if (y > 0)
        StretchDIBits(&memDC, 0, Height() - (y + ht), Width(), y,
            0, 0, Width(), y);

    memDC.SelectPalette(prvPal, TRUE);
    memDC.SelectObject(prvBMap);

    ClearDib();

    BitmapToDIB(&bmap, &pal);
    return TRUE;
}

BOOL CDib::AppendDIB(CDib *pDib)
{
    ASSERT(Width() == pDib->Width());
    CPalette pal1;
    CPalette pal2;
    CreatePalette(&pal1);
    pDib->CreatePalette(&pal2);
    CPalette* pMPal = CreateMergedPalette(&pal1, &pal2);

    CWindowDC scrnDC(NULL);
    CDC memDC;
    memDC.CreateCompatibleDC(&scrnDC);

    CPalette* prvPal = memDC.SelectPalette(pMPal, TRUE);
    memDC.RealizePalette();

    CBitmap bmap;
    bmap.Attach(Create16BitDIBSection(memDC.m_hDC,
        Width(), Height() + pDib->Height()));

    ASSERT(bmap.m_hObject != NULL);

    CBitmap* prvBMap = memDC.SelectObject(&bmap);

    StretchDIBits(&memDC, 0, 0, Width(), Height(), 0, 0, Width(), Height());
    pDib->StretchDIBits(&memDC, 0, Height(), Width(), pDib->Height(), 0, 0,
        Width(), pDib->Height());
    memDC.SelectPalette(prvPal, TRUE);
    memDC.SelectObject(prvBMap);

    ClearDib();

    BitmapToDIB(&bmap, pMPal);
    delete pMPal;

    return TRUE;
}

///////////////////////////////////////////////////////////////

CArchive& AFXAPI operator<<(CArchive& ar, const CDib& dib)
{
    if (dib.m_hDib)
    {
        DWORD dwSize = GlobalSize(dib.m_hDib);
        ASSERT(dwSize > 0);
        ASSERT(dib.m_nCompressLevel >= Z_NO_COMPRESSION
            && dib.m_nCompressLevel <= Z_BEST_COMPRESSION);
        if (dib.m_nCompressLevel > Z_NO_COMPRESSION)
        {
            // Store size of the uncompressed dib bfr with upper bit set.
            // The set upper bit allows us to detect loading of uncompressed
            // bitmaps.
            ar << (dwSize | 0x80000000);

            // Use zlib to compress the dib before writing it out the the archive.
            DWORD dwDestLen = MulDiv(dwSize, 1001, 1000) + 12;
            LPVOID pDestBfr = GlobalAlloc(GPTR, dwDestLen);
            if (pDestBfr == NULL)
                AfxThrowMemoryException();
            int err = compress2((LPBYTE)pDestBfr, &dwDestLen, (LPBYTE)dib.m_lpDib,
                dwSize, dib.m_nCompressLevel);
            if (err != Z_OK)
            {
                GlobalFree(GlobalHandle(pDestBfr));
                AfxThrowMemoryException();
            }
            ar << dwDestLen;                    // Store the compressed size of the bitmap
            ar.Write(pDestBfr, dwDestLen);      // Store the compressed bitmap
            GlobalFree(GlobalHandle(pDestBfr));
        }
        else
        {
            // Store size of the raw dib
            ar << dwSize;
            ar.Write(dib.m_lpDib, dwSize);      // Store the uncompressed bitmap
        }
    }
    else
        ar << (DWORD)0;
    return ar;
}

CArchive& AFXAPI operator>>(CArchive& ar, CDib& dib)
{
    dib.ClearDib();
    DWORD dwSize;
    ar >> dwSize;
    if ((dwSize & 0x80000000) != 0)
    {
        // Load compressed bitmap...
        dwSize &= 0x7FFFFFFF;       // Remove flag bit

        DWORD dwCompSize;
        ar >> dwCompSize;           // Get compressed data size

        LPVOID pCompBfr = GlobalAlloc(GPTR, dwCompSize);
        if (pCompBfr == NULL)
            AfxThrowMemoryException();

        if ((dib.m_hDib = (HDIB)GlobalAlloc(GHND, dwSize)) == NULL)
            AfxThrowMemoryException();
        dib.m_lpDib = (LPSTR)GlobalLock((HGLOBAL)dib.m_hDib);

        ar.Read(pCompBfr, dwCompSize);

        int err = uncompress((LPBYTE)dib.m_lpDib, &dwSize,
            (LPBYTE)pCompBfr, dwCompSize);

        GlobalFree(GlobalHandle(pCompBfr));
        if (err != Z_OK)
            AfxThrowMemoryException();
    }
    else if (dwSize > 0)
    {
        // Load uncompressed bitmap...
        if ((dib.m_hDib = (HDIB)GlobalAlloc(GHND, dwSize)) == NULL)
            AfxThrowMemoryException();
        dib.m_lpDib = (LPSTR)GlobalLock((HGLOBAL)dib.m_hDib);

        ar.Read(dib.m_lpDib, dwSize);
    }
    return ar;
}

