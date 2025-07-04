// GdiTools.cpp
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

    CFontTbl& pFontTbl = CGamDoc::GetFontManager();
    pFontTbl.FillLogFontStruct(fid, &lf);

    CFontDialog dlg(&lf, CF_SCREENFONTS |
        (bScreenOK ? 0 : CF_SCALABLEONLY) | CF_NOVECTORFONTS,
        NULL, pParentWnd);

    if (dlg.DoModal() != IDOK)
        return (FontID)0;
    return pFontTbl.AddFont(TenthPointsToScreenPixels(dlg.GetSize()),
        (dlg.IsBold()?taBold:0)|(dlg.IsItalic()?taItalic:0),
        static_cast<uint8_t>(lf.lfPitchAndFamily & 0xF0), dlg.GetFaceName());
}

FontID DoFontDialog(FontID fid, wxWindow* pParentWnd, BOOL bScreenOK)
{
    ASSERT(fid != 0);

    wxFontData fd;
    fd.EnableEffects(false);
    fd.SetAllowSymbols(false);
    if (!bScreenOK)
    {
        fd.RestrictSelection(wxFONTRESTRICT_SCALABLE);
    }
    fd.SetInitialFont(ToWxFont(fid));
    wxFontDialog dlg(pParentWnd, fd);

    if (dlg.ShowModal() != wxID_OK)
    {
        return static_cast<FontID>(0);
    }
    return ToFontID(dlg.GetFontData().GetChosenFont());
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

#ifndef GPLAY
void Draw25PctPatBorder(wxWindow& pWnd, wxDC& pDC, wxRect rct, int nThick)
{
    // see wxWidgets\src\msw\dc.cpp wxBrushAttrsSetter
    wxDCTextBgColourChanger setBg(pDC, *wxGREEN);
    wxDCTextColourChanger setFg(pDC, *wxWHITE);

    wxDCBrushChanger setBrush(pDC, g_res.hbr25PctWx);
    wxDCPenChanger setPen(pDC, *wxTRANSPARENT_PEN);

    // Top border
    pDC.DrawRectangle(rct.GetTopLeft(), wxSize(rct.GetWidth(), nThick));
    // Left border
    pDC.DrawRectangle(wxPoint(rct.GetLeft(), rct.GetTop() + nThick), wxSize(nThick, rct.GetHeight() - nThick));
    // Bottom border
    pDC.DrawRectangle(wxPoint(rct.GetLeft() + nThick, rct.GetBottom() + 1 - nThick), wxSize(rct.GetWidth() - nThick,
        nThick));
    // Right border
    pDC.DrawRectangle(wxPoint(rct.GetRight() + 1 - nThick, rct.GetTop() + nThick), wxSize(nThick,
        rct.GetHeight() - 2 * nThick));
}
#endif

/////////////////////////////////////////////////////////////////
// Creates a 24 bit DIB section.

OwnerPtr<CBitmap> CreateRGBDIBSection(int nWidth, int nHeight)
{
    return CDib::CreateDIBSection(nWidth, nHeight);
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

void SetPixel(wxBitmap& hBitmap, int x, int y, wxColour cr)
{
    wxASSERT(hBitmap.GetDepth() == 24 && hBitmap.IsDIB());
    wxNativePixelData imgData(hBitmap);
    wxNativePixelData::Iterator it(imgData);
    it.MoveTo(imgData, x, y);
    it.Red() = cr.Red();
    it.Green() = cr.Green();
    it.Blue() = cr.Blue();
}

wxColour GetPixel(const wxBitmap& hBitmap, int x, int y)
{
    // const_cast safe since we only read
    /* TODO:  create and submit to upstream const version of
        wxNativePixelData */
    wxASSERT(hBitmap.GetDepth() == 24 && hBitmap.IsDIB());
    wxNativePixelData imgData(const_cast<wxBitmap&>(hBitmap));
    wxNativePixelData::Iterator it(imgData);
    it.MoveTo(imgData, x, y);
    return wxColour(it.Red(), it.Green(), it.Blue());
}

/////////////////////////////////////////////////////////////////

void SetPixelBlock(wxBitmap& hBitmap, int x, int y, int cx, int cy, wxColour cr)
{
    wxASSERT(cx == 1 || !"untested code");
    wxASSERT(hBitmap.GetDepth() == 24 && hBitmap.IsDIB());
    wxNativePixelData imgData(hBitmap, wxPoint(x, y), wxSize(cx, cy));
    wxNativePixelData::Iterator rowStart(imgData);
    for (int currY = 0 ; currY < cy ; ++currY)
    {
        wxNativePixelData::Iterator it = rowStart;
        for (int currX = 0 ; currX < cx ; ++currX)
        {
            it.Red() = cr.Red();
            it.Green() = cr.Green();
            it.Blue() = cr.Blue();
            ++it;
        }
        rowStart.OffsetY(imgData, 1);
    }
}

/////////////////////////////////////////////////////////////////

OwnerPtr<wxBitmap> CreateRGBColorBar(int nHueDivisions, int nHeight)
{
    OwnerPtr<wxBitmap> hBitmap = MakeOwner<wxBitmap>(nHueDivisions, nHeight, 24);

    wxColour cref;

    for (int h = 0; h < nHueDivisions; h++)
    {
        cref = CB::Convert(HSVtoRGB((h * 359) / (nHueDivisions - 1), 255, 255));
        SetPixelBlock(*hBitmap, h, 0, 1, nHeight, cref);
    }
    return hBitmap;
}

OwnerPtr<wxBitmap> CreateRGBSaturationValueWash(int nHue, int nWidth, int nHeight)
{
    OwnerPtr<wxBitmap> hBitmap = MakeOwner<wxBitmap>(nWidth, nHeight, 24);

    wxColour cref;

    for (int x = 0; x < nWidth; x++)
    {
        for (int y = 0; y < nHeight; y++)
        {
            cref = CB::Convert(HSVtoRGB(nHue, 255 - (255 * y) / (nHeight - 1), (255 * x) / (nWidth - 1)));
            SetPixel(*hBitmap, x, y, cref);
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
        if (bmInfo.bmBitsPixel != 24)
        {
            AfxThrowNotSupportedException();
        }
        pbmDst = CDib::CreateDIBSection(
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
        ASSERT(!"untested code");
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

wxBitmap CloneBitmap(const wxBitmap& pbmSrc)
{
    // see https://docs.wxwidgets.org/latest/classwx_bitmap.html#abfaa21ec563a64ea913af918150db900
    return pbmSrc.GetSubBitmap(wxRect(wxPoint(0, 0), pbmSrc.GetSize()));
}

// The source bitmap will have its
// cut region filled with the crVoided color.
wxBitmap CutBitmapPiece(wxBitmap& pbmSrc, wxRect rctSrc,
    wxColour crVoided)
{
    wxASSERT(pbmSrc.IsOk());
    wxRect rct(wxPoint(0, 0), pbmSrc.GetSize());
    rct.Intersect(rctSrc);
    wxASSERT(!rct.IsEmpty());

    wxBitmap pbmDst = pbmSrc.GetSubBitmap(rct);
    if (crVoided != noColorWx)
    {
        wxMemoryDC dc(pbmSrc);
        dc.SetPen(crVoided);
        dc.SetBrush(crVoided);
        dc.DrawRectangle(rct);
    }

    return pbmDst;
}

// Draws on the source bitmap over the destination bitmap at the
// specified position.

void MergeBitmap(wxBitmap& pbmDst, const wxBitmap& pbmSrc, wxPoint pntDst)
{
    wxASSERT(pbmSrc.IsOk());
    wxASSERT(pbmDst.IsOk());

    wxMemoryDC dstDC(pbmDst);
    wxSize srcSize = pbmSrc.GetSize();
    wxMemoryDC srcDC;
    srcDC.SelectObjectAsSource(pbmSrc);
    dstDC.Blit(pntDst, srcSize,
                &srcDC, wxPoint(0, 0));
}

OwnerPtr<CBitmap> CloneScaledBitmap(const CBitmap& pbmSrc, CSize size)
{
    ASSERT(pbmSrc.m_hObject != NULL);
    OwnerPtr<CBitmap> pbmDst(MakeOwner<CBitmap>());
    BITMAP bmInfo;
    memset(&bmInfo, 0, sizeof(BITMAP));
    pbmSrc.GetObject(sizeof(bmInfo), &bmInfo);

    g_gt.mDC1.SelectObject(pbmSrc);

    if (bmInfo.bmBits != NULL)      // Check for DIB Section
    {
        pbmDst = CDib::CreateDIBSection(
            size.cx, size.cy);
    }
    else
        pbmDst->CreateCompatibleBitmap(&g_gt.mDC1, size.cx, size.cy);

    g_gt.mDC2.SelectObject(&*pbmDst);

    int nPrvMode = g_gt.mDC2.SetStretchBltMode(COLORONCOLOR);
    g_gt.mDC2.StretchBlt(0, 0, size.cx, size.cy, &g_gt.mDC1,
        0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);
    g_gt.mDC2.SetStretchBltMode(nPrvMode);

    g_gt.ClearMemDCBitmaps();

    return pbmDst;
}

wxBitmap CloneScaledBitmap(const wxBitmap& pbmSrc, wxSize size)
{
    wxMemoryDC srcDC;
    wxSize srcSize = pbmSrc.GetSize();
    srcDC.SelectObjectAsSource(pbmSrc);

    wxBitmap pbmDst(size);

    {
        wxMemoryDC dstDC(pbmDst);

        dstDC.StretchBlit(0, 0, size.x, size.y,
                            &srcDC, 0, 0, srcSize.x, srcSize.y);
    }

    return pbmDst;
}

wxBitmap CreateColorBitmap(wxSize size, wxColour cr)
{
    wxBrush brush(cr);
    wxPen pen(cr);

    wxBitmap bmp(size, 24);
    wxMemoryDC dc(bmp);

    dc.SetBrush(brush);
    dc.SetPen(pen);
    dc.DrawRectangle(wxPoint(0, 0), size);

    return bmp;
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

/////////////////////////////////////////////////////////////////

void CGdiTools::InitGdiTools()
{
    mDC1.CreateCompatibleDC(NULL);
    mDC2.CreateCompatibleDC(NULL);
    mTileDC.CreateCompatibleDC(NULL);
    // Make a copy of the stock bitmap that is automatically
    // attached to compatible memory DC's. This stock bitmap
    // is used for EZ deselection of 'real' bitmaps.
    CBitmap bmTmp;
    bmTmp.CreateCompatibleBitmap(&mDC1, 1, 1);
    CBitmap* pbmStock = mDC1.SelectObject(&bmTmp);
    hbmSafe = (HBITMAP)pbmStock->m_hObject;
    mDC1.SelectObject(pbmStock);
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
    CDib sourceDib(bmp);
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

