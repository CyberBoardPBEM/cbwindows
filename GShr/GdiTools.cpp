// GdiTools.cpp
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
#include    "FrmMain.h"
#ifdef GPLAY
#include    "GamDoc.h"
#else
#include    "GmDoc.h"
#endif
#include    "ResTbl.h"
#include    "CDib.h"
#include    "GdiTools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGdiTools g_gt;             // Global GDI tools object

/////////////////////////////////////////////////////////////////
// If 0 is returned, Cancel was selected.

FontID DoFontDialog(FontID fid, CWnd *pParentWnd, BOOL bScreenOK)
{
    ASSERT(fid != 0);
    LOGFONT lf;

    CFontTbl* pFontTbl = CGamDoc::GetFontManager();
    pFontTbl->FillLogFontStruct(fid, &lf);

    CFontDialog dlg(&lf, CF_SCREENFONTS |
        (bScreenOK ? 0 : CF_SCALABLEONLY) | CF_NOVECTORFONTS,
        NULL, pParentWnd);

    if (dlg.DoModal() != IDOK)
        return (FontID)0;
    return pFontTbl->AddFont(TenthPointsToScreenPixels(dlg.GetSize()),
        (dlg.IsBold()?taBold:0)|(dlg.IsItalic()?taItalic:0),
        static_cast<uint8_t>(lf.lfPitchAndFamily & 0xF0), dlg.GetFaceName());
}

/////////////////////////////////////////////////////////////////

int GetYPixelsPerLogicalInch()
{
    static int yPxlPerInch = 0;
    if (yPxlPerInch == 0)
    {
        HDC hDC = GetDC(NULL);
        yPxlPerInch = GetDeviceCaps(hDC, LOGPIXELSY);
        ReleaseDC(NULL, hDC);
    }
    return yPxlPerInch;
}

int TenthPointsToScreenPixels(int nTenthPts)
{
    return MulDiv(nTenthPts, GetYPixelsPerLogicalInch(), 720);
}

int ScreenPixelsToTenthPoints(int nPixels)
{
    return MulDiv(720, nPixels, GetYPixelsPerLogicalInch());
}

/////////////////////////////////////////////////////////////////
// Returns the number bits per pixel (clamps to 16)

int GetCurrentVideoResolution()
{
    CWindowDC dc(NULL);
    int nbpp = dc.GetDeviceCaps(BITSPIXEL) * dc.GetDeviceCaps(PLANES);
    return nbpp < 16 ? nbpp : 16;
}

/////////////////////////////////////////////////////////////////

void SetupPalette(CDC& pDC)
{
    CPalette* pPal = GetAppPalette();

    if (pPal == NULL || pPal->m_hObject == NULL)
        pPal = CPalette::FromHandle(
            (HPALETTE)::GetStockObject(DEFAULT_PALETTE));
    else
        pDC.SelectPalette(pPal, FALSE);
    pDC.RealizePalette();
}

void ResetPalette(CDC& pDC)
{
    pDC.SelectPalette(CPalette::FromHandle(
        (HPALETTE)::GetStockObject(DEFAULT_PALETTE)), TRUE);
}

/////////////////////////////////////////////////////////////////

void Draw25PctPatBorder(CWnd& pWnd, CDC& pDC, CRect rct, int nThick)
{
    pDC.SetBkColor(RGB(255, 255, 255));
    pDC.SetTextColor(RGB(0, 255, 0));

    CPoint pt(0, 0);
    pWnd.ClientToScreen(&pt);

    UnrealizeObject(g_res.hbr25Pct);
    pDC.SetBrushOrg(pt.x, pt.y);
    CBrush* pPrvBrush = pDC.SelectObject(CBrush::FromHandle(g_res.hbr25Pct));

    // Top border
    pDC.PatBlt(rct.left, rct.top, rct.Width(), nThick, PATCOPY);
    // Left border
    pDC.PatBlt(rct.left, rct.top + nThick, nThick, rct.Height() - nThick,
        PATCOPY);
    // Bottom border
    pDC.PatBlt(rct.left + nThick, rct.bottom - nThick, rct.Width() - nThick,
        nThick, PATCOPY);
    // Right border
    pDC.PatBlt(rct.right - nThick, rct.top + nThick, nThick,
        rct.Height() - 2 * nThick, PATCOPY);

    pDC.SelectObject(pPrvBrush);
}

/////////////////////////////////////////////////////////////////
// Creates a 16 bit DIB section that is 16 bits per pixel
// in 5-6-5 format.

OwnerPtr<CBitmap> Create16BitDIBSection(int nWidth, int nHeight)
{
    return CDib::CreateDIBSection(nWidth, nHeight, uint16_t(16));
}

/////////////////////////////////////////////////////////////////
// Creates a 24 bit DIB section.

OwnerPtr<CBitmap> CreateRGBDIBSection(int nWidth, int nHeight)
{
    return CDib::CreateDIBSection(nWidth, nHeight, uint16_t(24));
}

/////////////////////////////////////////////////////////////////

namespace {
LPVOID GetDIBSectXYLoc(HBITMAP hBitmap, int x, int y)
{
    DIBSECTION dibSect;
    VERIFY(GetObject(hBitmap, sizeof(DIBSECTION), &dibSect));

    ASSERT(dibSect.dsBmih.biBitCount == 24);

    size_t dwWidthBytes = CDib::WidthBytes(dibSect.dsBmih);
    LPBYTE pBits = (LPBYTE)dibSect.dsBm.bmBits;
    pBits += value_preserving_cast<ptrdiff_t>((dwWidthBytes * value_preserving_cast<size_t>(dibSect.dsBmih.biHeight - y - 1)) +
        value_preserving_cast<size_t>(x * dibSect.dsBmih.biBitCount / 8));

    ASSERT(pBits < (LPBYTE)dibSect.dsBm.bmBits + dibSect.dsBmih.biSizeImage);

    return pBits;
}
}

/////////////////////////////////////////////////////////////////

void SetRGBDIBSectPixel(CBitmap& hBitmap, int x, int y, COLORREF cr)
{
    BYTE* pPxl = (BYTE*)GetDIBSectXYLoc(hBitmap, x, y);
    *pPxl++ = GetBValue(cr);
    *pPxl++ = GetGValue(cr);
    *pPxl   = GetRValue(cr);
}

/////////////////////////////////////////////////////////////////

void SetRGBDIBSectPixelBlock(CBitmap& hBitmap, int x, int y, int cx, int cy, COLORREF cr)
{
    int cxSave = cx;
    while (cy--)
    {
        cx = cxSave;
        BYTE* pPxl = (BYTE*)GetDIBSectXYLoc(hBitmap, x, y);
        while (cx--)
        {
            *pPxl++ = GetBValue(cr);
            *pPxl++ = GetGValue(cr);
            *pPxl++ = GetRValue(cr);
        }
        y++;
    }
}

/////////////////////////////////////////////////////////////////

OwnerPtr<CBitmap> CreateRGBColorBar(int nHueDivisions, int nHeight)
{
    OwnerPtr<CBitmap> hBitmap = CreateRGBDIBSection(nHueDivisions, nHeight);

    COLORREF cref;

    for (int h = 0; h < nHueDivisions; h++)
    {
        cref = HSVtoRGB((h * 359) / (nHueDivisions - 1), 255, 255);
        SetRGBDIBSectPixelBlock(*hBitmap, h, 0, 1, nHeight, cref);
    }
    return hBitmap;
}

OwnerPtr<CBitmap> CreateRGBSaturationValueWash(int nHue, int nWidth, int nHeight)
{
    OwnerPtr<CBitmap> hBitmap = CreateRGBDIBSection(nWidth, nHeight);

    COLORREF cref;

    for (int x = 0; x < nWidth; x++)
    {
        for (int y = 0; y < nHeight; y++)
        {
            cref = HSVtoRGB(nHue, 255 - (255 * y) / (nHeight - 1), (255 * x) / (nWidth - 1));
            SetRGBDIBSectPixel(*hBitmap, x, y, cref);
        }
    }
    return hBitmap;
}

/////////////////////////////////////////////////////////////////
// h: 0..359
// s, v: 0..255

COLORREF HSVtoRGB(int h, int s, int v)
{
    double r, g, b;
    HSVtoRGB((double)h, (double)s/255, (double)v/255, r, g, b);
    return RGB((int)(r * 255 + 0.5), (int)(g * 255 + 0.5), (int)(b * 255 + 0.5));
}

/////////////////////////////////////////////////////////////////

void HSVtoRGB(double h, double s, double v, double& r, double& g, double& b)
{
    double f, p, q, t;
    int i;

    if (s == 0.0)
        r = g = b = v;
    else
    {
        if (h >= 360.0)
            h = 0.0;
        h /= 60.0;
        i = (int)floor(h);
        f = h - i;
        p = v * (1.0 - s);
        q = v * (1.0 - s * f);
        t = v * (1.0 - s * (1.0 - f));
        switch (i)
        {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
        }
    }
}

/////////////////////////////////////////////////////////////////

OwnerPtr<CBitmap> CloneBitmap(const CBitmap& pbmSrc)
{
    ASSERT(pbmSrc.m_hObject != NULL);
    OwnerPtr<CBitmap> pbmDst(MakeOwner<CBitmap>());
    BITMAP bmInfo;
    memset(&bmInfo, 0, sizeof(BITMAP));
    pbmSrc.GetObject(sizeof(bmInfo), &bmInfo);

    if (bmInfo.bmBits != NULL)
    {
        pbmDst = Create16BitDIBSection(
            bmInfo.bmWidth, bmInfo.bmHeight);
        BITMAP bmap;
        memset(&bmap, 0, sizeof(BITMAP));
        GetObject(*pbmDst, sizeof(BITMAP), &bmap);
        // How many bytes in the image data?
        size_t dwBitsSize = value_preserving_cast<size_t>(bmap.bmHeight) * CDib::BitsToBytes(bmap.bmWidth * bmap.bmPlanes *
            bmap.bmBitsPixel);

        // Copy the bits
        memcpy(bmap.bmBits, bmInfo.bmBits, dwBitsSize);

        g_gt.ClearMemDCBitmaps();
    }
    else
    {
        g_gt.mDC1.SelectObject(pbmSrc);
        pbmDst->CreateCompatibleBitmap(&g_gt.mDC1, bmInfo.bmWidth, bmInfo.bmHeight);
        g_gt.mDC2.SelectObject(&*pbmDst);
        // I don't select and realize the palettes since they aren't used
        // in memory to memory Bitblt operations.
        g_gt.mDC2.BitBlt(0, 0, bmInfo.bmWidth, bmInfo.bmHeight, &g_gt.mDC1, 0, 0,
            SRCCOPY);
        g_gt.ClearMemDCBitmaps();
    }

    return pbmDst;
}

// The copy excludes the right and bottom edges of rctSrc.
// If a crVoided color is supplied, the source bitmap will have its
// copied region filled with that color.
OwnerPtr<CBitmap> CopyBitmapPiece(CBitmap& pbmSrc, CRect rctSrc,
    COLORREF crVoided)
{
    ASSERT(pbmSrc.m_hObject != NULL);
    BITMAP bmInfo;
    memset(&bmInfo, 0, sizeof(BITMAP));
    pbmSrc.GetObject(sizeof(bmInfo), &bmInfo);
    CRect rct(0, 0, bmInfo.bmWidth, bmInfo.bmHeight);
    rct &= rctSrc;
    ASSERT(!rct.IsRectEmpty());

    g_gt.mDC1.SelectObject(&pbmSrc);

    OwnerPtr<CBitmap> pbmDst = MakeOwner<CBitmap>();
    if (bmInfo.bmBits != NULL)      // Check for DIB Section
    {
        pbmDst->Attach(Create16BitDIBSection(
            rct.Width(), rct.Height())->Detach());
    }
    else
    {
        pbmDst->CreateCompatibleBitmap(&g_gt.mDC1, rct.Width(), rct.Height());
    }

    g_gt.mDC2.SelectObject(&*pbmDst);

    // I don't select and realize the palettes since they aren't used
    // in memory to memory Bitblt operations.
    g_gt.mDC2.BitBlt(0, 0, rct.Width(), rct.Height(), &g_gt.mDC1, rct.left,
        rct.top, SRCCOPY);
    if (crVoided != noColor)
    {
        CBrush brush(crVoided);
        SetupPalette(g_gt.mDC1);
        g_gt.mDC1.FillRect(&rct, &brush);
        ResetPalette(g_gt.mDC1);
    }
    g_gt.ClearMemDCBitmaps();

    return pbmDst;
}

// Draws on the source bitmap over the destination bitmap at the
// specified position. If crTrans is specified, a masked merge is
// performed.

void MergeBitmap(CBitmap& pbmDst, const CBitmap& pbmSrc, CPoint pntDst)
{
    ASSERT(pbmSrc.m_hObject != NULL);
    ASSERT(pbmDst.m_hObject != NULL);

    g_gt.mDC1.SelectObject(pbmDst);
    SetupPalette(g_gt.mDC1);
    BITMAP bmi;
    pbmSrc.GetObject(sizeof(bmi), &bmi);
    g_gt.mDC2.SelectObject(pbmSrc);
    SetupPalette(g_gt.mDC2);
    g_gt.mDC1.BitBlt(pntDst.x, pntDst.y, bmi.bmWidth, bmi.bmHeight,
        &g_gt.mDC2, 0, 0, SRCCOPY);
    g_gt.SelectSafeObjectsForDC2();

    g_gt.SelectSafeObjectsForDC1();
}

OwnerPtr<CBitmap> CloneScaledBitmap(const CBitmap& pbmSrc, CSize size,
    int nStretchMode /* = STRETCH_DELETESCANS */)
{
    ASSERT(pbmSrc.m_hObject != NULL);
    OwnerPtr<CBitmap> pbmDst(MakeOwner<CBitmap>());
    BITMAP bmInfo;
    memset(&bmInfo, 0, sizeof(BITMAP));
    pbmSrc.GetObject(sizeof(bmInfo), &bmInfo);

    g_gt.mDC1.SelectObject(pbmSrc);

    if (bmInfo.bmBits != NULL)      // Check for DIB Section
    {
        pbmDst = Create16BitDIBSection(
            size.cx, size.cy);
    }
    else
        pbmDst->CreateCompatibleBitmap(&g_gt.mDC1, size.cx, size.cy);

    g_gt.mDC2.SelectObject(&*pbmDst);
    SetupPalette(g_gt.mDC1);
    SetupPalette(g_gt.mDC2);

    int nPrvMode = g_gt.mDC2.SetStretchBltMode(nStretchMode);
    g_gt.mDC2.StretchBlt(0, 0, size.cx, size.cy, &g_gt.mDC1,
        0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);
    g_gt.mDC2.SetStretchBltMode(nPrvMode);

    g_gt.ClearMemDCBitmaps();
    g_gt.ClearMemDCPalettes();

    return pbmDst;
}

OwnerPtr<CBitmap> CreateColorBitmap(CSize size, COLORREF cr)
{
    CBrush brush(cr);

    OwnerPtr<CBitmap> pBMap = Create16BitDIBSection(size.cx, size.cy);
    g_gt.mDC1.SelectObject(&*pBMap);
    SetupPalette(g_gt.mDC1);

    CBrush* prvBrush = g_gt.mDC1.SelectObject(&brush);
    g_gt.mDC1.PatBlt(0, 0, size.cx, size.cy, PATCOPY);
    g_gt.mDC1.SelectObject(prvBrush);

    g_gt.SelectSafeObjectsForDC2();

    return pBMap;
}

/////////////////////////////////////////////////////////////////
// Bitmap type bitblt. Used global DC mTileDC.
// Caller may use mDC1 or mDC2.

void BitmapBlt(CDC& pDC, CPoint pntDst, const CBitmap& pBMap)
{
    BITMAP bmInfo;
    pBMap.GetObject(sizeof(bmInfo), &bmInfo);
    CSize size(bmInfo.bmWidth, bmInfo.bmHeight);

    g_gt.mTileDC.SelectObject(pBMap);
    g_gt.mTileDC.SetMapMode(MM_TEXT);
    g_gt.mTileDC.SetViewportOrg(0, 0);

    // SetupPalette(&g_gt.mTileDC);

    // Now do the bitblt
    pDC.BitBlt(pntDst.x, pntDst.y, size.cx, size.cy,
        &g_gt.mTileDC, 0, 0, SRCCOPY);

    g_gt.SelectSafeObjectsForTileDC();
}

/////////////////////////////////////////////////////////////////

CPalette* GetAppPalette()
{
    return ((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetMasterPalette();
}

// ----------------------------------------------------------- //
// Combines secondary palette and primary palette into new merged
// Palette. Also makes sure the resulting palette is an 'Identity'
// palette for Realization speed.

OwnerPtr<CPalette> CreateMergedPalette(const CPalette& palPri, const CPalette& palSec)
{
    short nSizePri, nSizeSec;
    uint16_t nPalSize = uint16_t(256);

    palSec.GetObject(sizeof(short), &nSizeSec);
    palPri.GetObject(sizeof(short), &nSizePri);

    std::vector<std::byte> v(size_t(sizeof(LOGPALETTE) +
        nPalSize * sizeof(PALETTEENTRY)));
    LPLOGPALETTE pLP = reinterpret_cast<LPLOGPALETTE>(v.data());

    SetupIdentityPalette(nPalSize, pLP);

    // First merge primary palette (which has priority)
    for (int i = 0; i < nSizePri; i++)
    {
        PALETTEENTRY pe;
        palPri.GetPaletteEntries(i, 1, &pe);
        AddEntryToPalette(pLP->palPalEntry, nPalSize, pe);
    }
    // Then merge secondary palette
    for (int i = 0; i < nSizeSec; i++)
    {
        PALETTEENTRY pe;
        palSec.GetPaletteEntries(i, 1, &pe);
        AddEntryToPalette(pLP->palPalEntry, nPalSize, pe);
    }
    OwnerPtr<CPalette> palNew(MakeOwner<CPalette>());
    palNew->CreatePalette(pLP);
    return palNew;
}

#ifdef WE_WANT_THIS_STUFF_DLL940113
// ----------------------------------------------------------- //
// Assumes bitmap and palette of dest DC has been selected
// and realized.
void BltThruDIB(CDC* pDCDst, int xDst, int yDst, int cx, int cy,
    CDC* pDCSrc, CPalette *pPalSrc, CBitmap* pBMap, int xSrc, int ySrc)
{
    CDib dib(*pBMap, pPalSrc);
    dib.StretchDIBits(*pDCDst, xDst, yDst, cx, cy, xSrc, ySrc, cx, cy);
}

// ----------------------------------------------------------- //

void RemoveBitmapSlice(CBitmap *pBMap, int yPos, int iHt, CPalette *pPal)
{
    int yPos2 = yPos + iHt;
    BITMAP bmi;
    pBMap->GetObject(sizeof(BITMAP), &bmi);

    if (yPos2 < bmi.bmHeight)
    {
        g_gt.mDC1.SelectObject(pBMap);
        g_gt.mDC1.BitBlt(0, yPos, bmi.bmWidth,
            bmi.bmHeight - yPos2, &g_gt.mDC1, 0, yPos2, SRCCOPY);
        g_gt.SelectSafeObjectsForDC1();
    }
    ResizeBitmap(pBMap, bmi.bmWidth, bmi.bmHeight - iHt, pPal);
}

// ----------------------------------------------------------- //

void ResizeBitmap(CBitmap *pBMap, int iNewX, int iNewY, CPalette* pPalOld,
    CPalette* pPalNew)
{
    BITMAP bminfo;
    pBMap->GetObject(sizeof(BITMAP), &bminfo);
    int iBltWd = CB::min(iNewX, bminfo.bmWidth);
    int iBltHt = CB::min(iNewY, bminfo.bmHeight);

    CBitmap bmap;
    bminfo.bmHeight = iNewY;
    bmap.CreateBitmapIndirect(&bminfo);

    g_gt.mDC1.SelectObject(&bmap);
    g_gt.mDC1.SelectPalette(pPalNew, TRUE);
    g_gt.mDC1.RealizePalette();

    BltThruDIB(&g_gt.mDC1, 0, 0, iBltWd, iBltHt, &g_gt.mDC2, pPalOld,
        pBMap, 0, 0);

    g_gt.ClearMemDCPalettes();
    g_gt.ClearMemDCBitmaps();

    pBMap->DeleteObject();
    pBMap->Attach(bmap.Detach());
}
#endif

// ----------------------------------------------------------- //

static BYTE sysColors[20*3] =
{
      0,   0,   0,
    128,   0,   0,
      0, 128,   0,
    128, 128,   0,
      0,   0, 128,
    128,   0, 128,
      0, 128, 128,
    192, 192, 192,
    192, 220, 192,
    166, 202, 240,

    255, 251, 240,
    160, 160, 164,
    128, 128, 128,
    255,   0,   0,
      0, 255,   0,
    255, 255,   0,
      0,   0, 255,
    255,   0, 255,
      0, 255, 255,
    255, 255, 255,
};

void SetupIdentityPalette(uint16_t nNumColors, LPLOGPALETTE pPal)
{
    pPal->palVersion = 0x300;
    pPal->palNumEntries = nNumColors;
    memset(pPal->palPalEntry, 0, sizeof(PALETTEENTRY) * nNumColors);
    BYTE* tPtr = sysColors;
    for (int i = 0; i < 256; i++)
    {
        PALETTEENTRY *pPe = &pPal->palPalEntry[i];
        pPe->peRed   = *tPtr++;
        pPe->peGreen = *tPtr++;
        pPe->peBlue  = *tPtr++;
        if (i == 9) i = 245;        // kick it to palette top
    }
}

// ----------------------------------------------------------- //

void SetupColorTable(CDWordArray* pTbl, BOOL bInclBlackAndWhite /* = TRUE */)
{
    pTbl->RemoveAll();
    for (int i = bInclBlackAndWhite ? 0 : 1; i < 8; i++)
    {
        int nIdx = i * 3;
        pTbl->Add((DWORD)RGB(sysColors[nIdx], sysColors[nIdx+1],
            sysColors[nIdx+2]));
    }
    for (int i = 0; i < (bInclBlackAndWhite ? 8 : 7); i++)
    {
        int nIdx = (i + 12) * 3;
        pTbl->Add((DWORD)RGB(sysColors[nIdx], sysColors[nIdx+1],
            sysColors[nIdx+2]));
    }
}

// ----------------------------------------------------------- //

void SetPaletteEntryFromColorref(PALETTEENTRY& pe, COLORREF cr)
{
    pe.peRed   = GetRValue(cr);
    pe.peGreen = GetGValue(cr);
    pe.peBlue  = GetBValue(cr);
    pe.peFlags = 0;
}

// ----------------------------------------------------------- //

void AddEntryToPalette(LPPALETTEENTRY pPal, int nSize, const PALETTEENTRY& pe)
{
    LPPALETTEENTRY pPe;
    int iAvail = -1;
    for (int i = 0; i < nSize; i++)
    {
        pPe = &pPal[i];
        if (pe.peRed == pPe->peRed && pe.peGreen == pPe->peGreen &&
                pe.peBlue == pPe->peBlue)
            return;     // No need to add
        if ((iAvail == -1) && (i > 0) &&
                ((pPe->peRed | pPe->peGreen | pPe->peBlue) == 0))
            iAvail = i;
    }
    if (iAvail != -1)
        pPal[iAvail] = pe;      // Store the value
}

/////////////////////////////////////////////////////////////////
//*** Resetting the system palette code here (From WinG)

void ClearSystemPalette()
{
    //*** A dummy palette setup
    struct
    {
        WORD Version;
        WORD NumberOfEntries;
        PALETTEENTRY aEntries[256];
    } Palette =
    {
        0x300,
        256
    };

    HPALETTE ScreenPalette = 0;
    HDC ScreenDC;
    int Counter;

    //*** Reset everything in the system palette to black
    for(Counter = 0; Counter < 256; Counter++)
    {
        Palette.aEntries[Counter].peRed = 0;
        Palette.aEntries[Counter].peGreen = 0;
        Palette.aEntries[Counter].peBlue = 0;


        Palette.aEntries[Counter].peFlags = PC_NOCOLLAPSE;
    }

    //*** Create, select, realize, deselect, and delete the palette
    ScreenDC = GetDC(NULL);
    ScreenPalette = CreatePalette((LOGPALETTE *)&Palette);
    ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
    RealizePalette(ScreenDC);
    ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
    DeleteObject(ScreenPalette);
    ReleaseDC(NULL, ScreenDC);
}

/////////////////////////////////////////////////////////////////

void CGdiTools::InitGdiTools()
{
    mDC1.CreateCompatibleDC(NULL);
    mDC2.CreateCompatibleDC(NULL);
    mTileDC.CreateCompatibleDC(NULL);
    hpalSafe = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
    // Make a copy of the stock bitmap that is automatically
    // attached to compatible memory DC's. This stock bitmap
    // is used for EZ deselection of 'real' bitmaps.
    CBitmap bmTmp;
    bmTmp.CreateCompatibleBitmap(&mDC1, 1, 1);
    CBitmap* pbmStock = mDC1.SelectObject(&bmTmp);
    hbmSafe = (HBITMAP)pbmStock->m_hObject;
    mDC1.SelectObject(pbmStock);
}

void CGdiTools::SetupMemDCPalettes(CPalette *pPal1, CPalette *pPal2)
{
    if (pPal1)
    {
        mDC1.SelectPalette(pPal1, TRUE);
        mDC1.RealizePalette();
        mDC2.SelectPalette(pPal2 ? pPal2 : pPal1, TRUE);
        mDC2.RealizePalette();
    }
}

/////////////////////////////////////////////////////////////////
// DeltaGenInit
//
// This routine initializes the pdg, px and py in preparation for using
// DeltaGen().  Returns fContinue.
//
// Algorithm derived from BRESENHAM line algorighm on p. 435 of Fund. of
// interactive computer graphics, Foley/VanDam, addison-wesley 1983.

BOOL DeltaGenInit(PDELTAGEN pdg, int x0, int y0, int xf, int yf,
    int* px, int* py)
{
    int nT;

    pdg->xf = xf;
    pdg->yf = yf;

    if (x0 == xf && y0 == yf)
        return FALSE;

    if (xf >= x0)
        pdg->incx = 1;
    else
        pdg->incx = -1;

    if (yf >= y0)        pdg->incy = 1;
    else
        pdg->incy = -1;

    pdg->dx = (xf - x0) * pdg->incx;
    pdg->dy = (yf - y0) * pdg->incy;

    if (pdg->dy > pdg->dx)
    {
        nT = pdg->dy;
        pdg->dy = pdg->dx;
        pdg->dx = nT;
        nT = pdg->incx;
        pdg->incx = pdg->incy;
        pdg->incy = nT;
        pdg->fSwap = TRUE;
    }
    else
    {
        pdg->fSwap = FALSE;
    }

    pdg->inc1 = pdg->dy * 2;
    pdg->inc2 = (pdg->dy - pdg->dx) * 2;
    pdg->d = pdg->inc1 - pdg->dx;

    pdg->xf = xf;
    pdg->yf = yf;

      *px = x0;
      *py = y0;

    return TRUE;
}

/////////////////////////////////////////////////////////////////
// DeltaGen
//
// This routine generates the next coordinates for px,py assuming that this
// point is proceeding linearly from x0,y0 to xf, yf.  It returns FALSE only
// if *px == xf and *py == yf on entry.  (ie returns fContinue)  pdg should
// have been previously set by DeltaGenInit().
//
// Algorithm derived from BRESENHAM line algorighm on p. 435 of Fund. of
// interactive computer graphics, Foley/VanDam, addison-wesley 1983.

BOOL DeltaGen(PDELTAGEN pdg, int* px, int* py)
{
    int* pnT;

    if ((*px == pdg->xf) && (*py == pdg->yf))
        return FALSE;

    if (pdg->fSwap)
    {
        pnT = px;
        px = py;
        py = pnT;
    }

    *px += pdg->incx;
    if (pdg->d < 0)
    {
        pdg->d += pdg->inc1;
    }
    else
    {
        *py += pdg->incy;
        pdg->d += pdg->inc2;
    }
    return TRUE;
}

OwnerPtr<CBrush> Clone(const CBrush& brush)
{
    LOGBRUSH lb;
/*  surely this is safe, but we'll avoid the const_cast
    int rc = const_cast<CBrush&>(brush).GetLogBrush(&lb);
*/
    int rc = ::GetObject(brush, sizeof(lb), &lb);
    ASSERT(rc == sizeof(lb));
    OwnerPtr<CBrush> retval(MakeOwner<CBrush>());
    VERIFY(retval->CreateBrushIndirect(&lb));
    return retval;
}

OwnerPtr<CBitmap> Rotate(const CBitmap& bmp, Rotation90 rot)
{
    CDib sourceDib(bmp, GetAppPalette());
    if (!sourceDib)
    {
        AfxThrowMemoryException();
    }
    CDib pDib = sourceDib.Rotate(static_cast<int>(rot)*90, RGB(255, 255, 255));
    return pDib.DIBToBitmap();
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
#ifdef WE_WANT_THIS_STUFF_DLL940113

// This routine takes a list of palettes and sweeps through
// them an entry at a time to create a 'fair' master palette
// for the application. The palette pointers are stored in
// an array.

CPalette* BuildMasterPalette(CObArray* pPalTbl, BOOL bAppend)
{
    int nPalSize = 256;
#error needs work
    LPLOGPALETTE pLP = (LPLOGPALETTE) new char[sizeof(LOGPALETTE) +
        nPalSize * sizeof(PALETTEENTRY)];
    SetupIdentityPalette(nPalSize, pLP);    // Start with speedy identity pal
    if (bAppend)
    {
        // Append approach
        for (int i = 0; i < pPalTbl->GetSize(); i++)
        {
            short iPalSize;
            CPalette *pPal = (CPalette*)(*pPalTbl)[i];
            pPal->GetObject(sizeof(short), &iPalSize);

            for (int j = 0; j < iPalSize; j++)
            {
                PALETTEENTRY pe;
                pPal->GetPaletteEntries(j, 1, &pe);
                AddEntryToPalette(pLP->palPalEntry, nPalSize, pe);
            }
        }
    }
    else
    {
        // Interleaved approach
        for (int i = 0; i < nPalSize; i++)
        {
            for (int j = 0; j < pPalTbl->GetSize(); j++)
            {
                short iPalSize;
                CPalette *pPal = (CPalette*)(*pPalTbl)[j];
                pPal->GetObject(sizeof(short), &iPalSize);
                if (iPalSize >= i)
                {
                    PALETTEENTRY pe;
                    pPal->GetPaletteEntries(i, 1, &pe);
                    AddEntryToPalette(pLP->palPalEntry, nPalSize, pe);
                }
            }
        }
    }
    CPalette* pPal = new CPalette;
    pPal->CreatePalette(pLP);
#error needs to be checked
    delete (char*)pLP;
    return pPal;
}
#endif

//#ifdef WE_WANT_THIS_STUFF_DLL940113
// ----------------------------------------------------------- //
//
//void AppendBitmap(CBitmap *pBMapTo, CBitmap *pBMapFrm, CPalette *pPalTo,
//  CPalette *pPalFrm)
//{
//  BITMAP bmiTo;
//  BITMAP bmiFrm;
//  pBMapTo->GetObject(sizeof(BITMAP), &bmiTo);
//  pBMapFrm->GetObject(sizeof(BITMAP), &bmiFrm);
//
//  CPalette* pPalNew = CreateMergedPalette(pPalTo, pPalFrm);
//  ResizeBitmap(pBMapTo, bmiTo.bmWidth, bmiTo.bmHeight + bmiFrm.bmHeight,
//      pPalTo, pPalNew);
//
//  g_gt.mDC1.SelectObject(pBMapTo);
//  g_gt.mDC1.SelectPalette(pPalNew, TRUE);
//  g_gt.mDC1.RealizePalette();
//
//  BltThruDIB(&g_gt.mDC1, 0, bmiTo.bmHeight, bmiTo.bmWidth, bmiFrm.bmHeight,
//      &g_gt.mDC2, pPalFrm, pBMapFrm, 0, 0);
//
//  g_gt.ClearMemDCPalettes();
//  g_gt.ClearMemDCBitmaps();
//  pPalTo->DeleteObject();
//  pPalTo->Attach(pPalNew->Detach());
//  delete pPalNew;
//}
//#endif

