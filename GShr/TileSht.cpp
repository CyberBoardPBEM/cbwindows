// TileSht.cpp
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
#ifdef      GPLAY
    #include    "Gp.h"
    #include    "GameBox.h"
    #include    "GamDoc.h"
#else
    #include    "Gm.h"
    #include    "GmDoc.h"
#endif
#include    "CDib.h"
#include    "Tile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////

CTileSheet::CTileSheet()
{
    m_size = CSize(0, 0);
    m_sheetHt = 0;
}

//////////////////////////////////////////////////////////////////

void CTileSheet::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (short)m_size.cx;
        ar << (short)m_size.cy;

        if (m_pBMap)
        {
            CDib dib(*m_pBMap);
            ASSERT(dib);
            if (dib)
            {
                ar << (WORD)1;      // Store "HasBitmap" flag
#ifndef GPLAY
                dib.SetCompressLevel(((CGamDoc*)ar.m_pDocument)->
                    GetCompressLevel());
#endif
                ar << dib;
            }
            else
                ar << (WORD)0;      // Store "HasBitmap" flag
        }
        else
            ar << (WORD)0;          // Store "HasBitmap" flag
    }
    else
    {
        ClearSheet();
        short sTmp;
        ar >> sTmp; m_size.cx = sTmp;
        ar >> sTmp; m_size.cy = sTmp;

        WORD wHasBitmap = 1;            // Force for old versions of file
#ifdef GPLAY
        if (CGameBox::GetLoadingVersion() > NumVersion(0, 52))
#else
        if (CGamDoc::GetLoadingVersion() > NumVersion(0, 52))
#endif
            ar >> wHasBitmap;

        if (wHasBitmap)
        {
            CDib dib;
            ar >> dib;
            if (dib)
            {
                m_pBMap = dib.DIBToBitmap();
                BITMAP bmInfo;
                m_pBMap->GetObject(sizeof(bmInfo), &bmInfo);
                TRACE3("-- Loaded Tile Sheet for cx=%d, cy=%d tiles. Tile sheet height = %d\n",
                    m_size.cx, m_size.cy, bmInfo.bmHeight);
                //  ASSERT(bmInfo.bmHeight < maxSheetHeight);
                m_sheetHt = bmInfo.bmHeight;
            }
            else
            {
                TRACE0("CTileSheet::Serialize - Forced truncation of tile sheet. "
                    "No DIB found even though should have one.\n");
                m_sheetHt = 0;
            }
        }
        else
            m_sheetHt = 0;
    }
}

//////////////////////////////////////////////////////////////////

void CTileSheet::SetSize(CSize size)
{
    ASSERT(m_sheetHt == 0 && m_pBMap == NULL);  // ONLY LEGAL IF NO DATA IN SHEET!!!!!!
    m_size = size;
}

//////////////////////////////////////////////////////////////////

void CTileSheet::CreateTile()
{
    if (m_pBMap != NULL)
    {
        BITMAP bmInfo;
        VERIFY(m_pBMap->GetObject(sizeof(bmInfo), &bmInfo) > 0);
        bmInfo.bmBits = NULL;
        bmInfo.bmHeight += m_size.cy;       // Increase size.

        OwnerPtr<CBitmap> pBMap = CDib::CreateDIBSection(
            bmInfo.bmWidth, bmInfo.bmHeight);
        ASSERT(pBMap->m_hObject != NULL);
        g_gt.mDC1.SelectObject(pBMap.get());          // Dest bitmap
        g_gt.mDC2.SelectObject(m_pBMap.get());        // Source bitmap

        g_gt.mDC1.BitBlt(0, 0, m_size.cx, m_sheetHt, &g_gt.mDC2, 0, 0, SRCCOPY);
        g_gt.mDC1.PatBlt(0, m_sheetHt, m_size.cx, m_size.cy, WHITENESS);

        g_gt.SelectSafeObjectsForDC1();
        g_gt.SelectSafeObjectsForDC2();

        m_sheetHt = bmInfo.bmHeight;

        m_pBMap = std::move(pBMap);
    }
    else
    {
        ASSERT(m_size != CSize(0,0));
        TRACE("CTileSheet::CreateTile - Creating new TileSheet bitmap\n");
        m_pBMap = CDib::CreateDIBSection(
            m_size.cx, m_size.cy);
        g_gt.mDC1.SelectObject(m_pBMap.get());
        g_gt.mDC1.PatBlt(0, 0, m_size.cx, m_size.cy, WHITENESS);
        g_gt.SelectSafeObjectsForDC1();

        m_sheetHt = m_size.cy;
    }
}

void CTileSheet::DeleteTile(int yLoc)
{
    ASSERT(m_pBMap != NULL);
    ASSERT(yLoc < m_sheetHt - 1);
    if (m_size.cy == m_sheetHt)
    {
        TRACE("CTileSheet::DeleteTile - Deleting TileSheet bitmap\n");
        // Single tile is all that's left...
        ASSERT(yLoc == 0);
        m_pBMap = nullptr;
        m_sheetHt = 0;
    }
    else
    {
        BITMAP bmInfo;
        m_pBMap->GetObject(sizeof(bmInfo), &bmInfo);
        bmInfo.bmBits = NULL;
        bmInfo.bmHeight -= m_size.cy;           // Decrease size.

        g_gt.mDC2.SelectObject(m_pBMap.get());        // Source bitmap

        OwnerPtr<CBitmap> pBMap = CDib::CreateDIBSection(
            bmInfo.bmWidth, bmInfo.bmHeight);

        g_gt.mDC1.SelectObject(pBMap.get());          // Dest bitmap

        if (yLoc > 0)
        {
            // There is a piece above the deleted piece.
            g_gt.mDC1.BitBlt(0, 0, m_size.cx, yLoc, &g_gt.mDC2, 0, 0, SRCCOPY);
        }
        if (yLoc + m_size.cy < m_sheetHt)           // See if not last tile
        {
            // There is a piece after the deleted piece.
            g_gt.mDC1.BitBlt(0, yLoc, m_size.cx, m_sheetHt - (yLoc + m_size.cy),
                &g_gt.mDC2, 0, yLoc + m_size.cy, SRCCOPY);
        }
        g_gt.SelectSafeObjectsForDC1();
        g_gt.SelectSafeObjectsForDC2();

        m_sheetHt = bmInfo.bmHeight;

        m_pBMap = std::move(pBMap);
    }
}

void CTileSheet::UpdateTile(const CBitmap& pBMap, int yLoc)
{
    ASSERT(m_pBMap != NULL);
    ASSERT(yLoc < m_sheetHt - 1);
    g_gt.mDC1.SelectObject(m_pBMap.get());        // Dest bitmap
    g_gt.mDC2.SelectObject(pBMap);          // Source bitmap

    g_gt.mDC1.BitBlt(0, yLoc, m_size.cx, m_size.cy, &g_gt.mDC2, 0, 0, SRCCOPY);

    g_gt.SelectSafeObjectsForDC1();
    g_gt.SelectSafeObjectsForDC2();
}

OwnerPtr<CBitmap> CTileSheet::CreateBitmapOfTile(int yLoc) const
{
    ASSERT(m_pBMap != NULL);
    ASSERT(yLoc < m_sheetHt - 1);
    g_gt.mDC1.SelectObject(*m_pBMap);        // Source bitmap

    OwnerPtr<CBitmap> pBMap(MakeOwner<CBitmap>());

    BITMAP bmap;
    memset(&bmap, 0, sizeof(BITMAP));
    m_pBMap->GetObject(sizeof(BITMAP), &bmap);

    if (bmap.bmBits != NULL)                // DIB Section check
    {
        pBMap = CDib::CreateDIBSection(
            m_size.cx, m_size.cy);
    }
    else
    {
        pBMap->CreateCompatibleBitmap(&g_gt.mDC1, m_size.cx, m_size.cy);
    }

    g_gt.mDC2.SelectObject(&*pBMap);          // Activate dest bitmap

    g_gt.mDC2.BitBlt(0, 0, m_size.cx, m_size.cy, &g_gt.mDC1, 0, yLoc, SRCCOPY);

    g_gt.SelectSafeObjectsForDC1();
    g_gt.SelectSafeObjectsForDC2();

    return pBMap;
}

////////////////////////////////////////////////////////////////////////

void CTileSheet::TileBlt(CDC& pDC, int xDst, int yDst, int ySrc, DWORD dwRop) const
{
    SheetDC sheetDC(*this);
    pDC.BitBlt(xDst, yDst, m_size.cx, m_size.cy, sheetDC, 0, ySrc, dwRop);
}

void CTileSheet::TileBlt(wxDC& pDC, wxCoord xDst, wxCoord yDst, wxCoord ySrc, wxRasterOperationMode dwRop) const
{
    wxMemoryDC sheetDC;
    wxBitmap wxbmp(ToImage(CheckedDeref(m_pBMap)));
    sheetDC.SelectObjectAsSource(wxbmp);
    pDC.Blit(xDst, yDst, m_size.cx, m_size.cy, &sheetDC, 0, ySrc, dwRop);
}

void CTileSheet::StretchBlt(CDC& pDC, int xDst, int yDst,
    int xWid, int yWid, int ySrc, DWORD dwRop) const
{
    SheetDC sheetDC(*this);
    pDC.StretchBlt(xDst, yDst, xWid, yWid, sheetDC, 0, ySrc,
        m_size.cx, m_size.cy, dwRop);
}

void CTileSheet::TransBlt(CDC& pDC, int xDst, int yDst, int ySrc,
    COLORREF crTrans) const
{
    GdiFlush();
    HBITMAP hBMapDest = (HBITMAP)GetCurrentObject(pDC.m_hDC, OBJ_BITMAP);
    ASSERT(hBMapDest != NULL);

    BITMAP  bmapTile;
    BITMAP  bmapDest;

    memset(&bmapTile, 0, sizeof(BITMAP));
    memset(&bmapDest, 0, sizeof(BITMAP));

    m_pBMap->GetObject(sizeof(BITMAP), &bmapTile);
    ::GetObject(hBMapDest, sizeof(BITMAP), &bmapDest);
    ASSERT(bmapTile.bmBits != NULL && bmapDest.bmBits != NULL);

    CPoint pntOrg = pDC.GetViewportOrg();
    xDst += pntOrg.x;
    yDst += pntOrg.y;

    ASSERT(bmapTile.bmBitsPixel == 24);
    // DIBs are bottom-up
    ptrdiff_t srcStride = -bmapTile.bmWidthBytes;
    const std::byte* srcRowStart = static_cast<const std::byte*>(bmapTile.bmBits) +
                (bmapTile.bmHeight - 1 - ySrc) * -srcStride;
    ASSERT(bmapDest.bmBitsPixel == 24);
    ptrdiff_t destStride = -bmapDest.bmWidthBytes;
    std::byte* destRowStart = static_cast<std::byte*>(bmapDest.bmBits) +
                (bmapDest.bmHeight - 1 - yDst) * -destStride +
                xDst * (bmapDest.bmBitsPixel / 8);

    for (int y = 0 ;
        y < m_size.cy && yDst + y < bmapDest.bmHeight ;
        ++y)
    {
        if (yDst + y >= 0)
        {
            const WIN_RGBTRIO* src = reinterpret_cast<const WIN_RGBTRIO*>(srcRowStart);
            WIN_RGBTRIO* dest = reinterpret_cast<WIN_RGBTRIO*>(destRowStart);
            for (int x = 0 ;
                x < m_size.cx && xDst + x < bmapDest.bmWidth ;
                ++x)
            {
                COLORREF cr = *src++;
                if (xDst + x >= 0)
                {
                    if (cr != crTrans)
                    {
                        *dest = cr;
                    }
                }
                ++dest;
            }
        }
        srcRowStart += srcStride;
        destRowStart += destStride;
    }
}

void CTileSheet::TransBlt(wxDC& pDC, int xDst, int yDst, int ySrc,
    wxColour crTrans) const
{
    wxBitmap wxBMap = CB::Convert(*m_pBMap);
    wxBMap.SetMask(new wxMask(wxBMap, crTrans));
    wxMemoryDC srcDC;
    srcDC.SelectObjectAsSource(wxBMap);
    pDC.Blit(xDst, yDst, m_size.cx, m_size.cy, &srcDC, 0, ySrc, wxCOPY, true);
}

////////////////////////////////////////////////////////////////////////

void CTileSheet::TransBltThruDIBSectMonoMask(CDC& pDC, int xDst, int yDst, int ySrc,
    COLORREF crTrans, const BITMAP& pMaskBMapInfo) const
{
    GdiFlush();

    HBITMAP hBMapDest = (HBITMAP)GetCurrentObject(pDC.m_hDC, OBJ_BITMAP);
    ASSERT(hBMapDest != NULL);

    BITMAP  bmapTile;
    BITMAP  bmapDest;

    memset(&bmapTile, 0, sizeof(BITMAP));
    memset(&bmapDest, 0, sizeof(BITMAP));

    m_pBMap->GetObject(sizeof(BITMAP), &bmapTile);
    ::GetObject(hBMapDest, sizeof(BITMAP), &bmapDest);
    ASSERT(bmapTile.bmBits != NULL && bmapDest.bmBits != NULL);

    CPoint pntOrg = pDC.GetViewportOrg();
    xDst += pntOrg.x;
    yDst += pntOrg.y;

    ASSERT(bmapTile.bmBitsPixel == 24);
    ptrdiff_t srcStride = -bmapTile.bmWidthBytes;
    const std::byte* srcRowStart = static_cast<const std::byte*>(bmapTile.bmBits) +
                                        (bmapTile.bmHeight - 1 - ySrc) * -srcStride;
    ASSERT(bmapDest.bmBitsPixel == 24);
    ptrdiff_t destStride = -bmapDest.bmWidthBytes;
    std::byte* destRowStart = static_cast<std::byte*>(bmapDest.bmBits) +
                                        (bmapDest.bmHeight - 1 - yDst) * -destStride +
                                        xDst * (bmapDest.bmBitsPixel / 8);
    ASSERT(pMaskBMapInfo.bmBitsPixel == 24);
    ptrdiff_t maskStride = -pMaskBMapInfo.bmWidthBytes;
    const std::byte* maskRowStart = static_cast<const std::byte*>(pMaskBMapInfo.bmBits) +
                                        (pMaskBMapInfo.bmHeight - 1) * -maskStride;

    for (int y = 0 ;
        y < std::min(m_size.cy, pMaskBMapInfo.bmHeight) &&
                yDst + y < bmapDest.bmHeight ;
        ++y)
    {
        if (yDst + y >= 0)
        {
            const WIN_RGBTRIO* src = reinterpret_cast<const WIN_RGBTRIO*>(srcRowStart);
            WIN_RGBTRIO* dest = reinterpret_cast<WIN_RGBTRIO*>(destRowStart);
            const WIN_RGBTRIO* mask = reinterpret_cast<const WIN_RGBTRIO*>(maskRowStart);
            for (int x = 0 ;
                x < std::min(m_size.cx, pMaskBMapInfo.bmWidth) &&
                        xDst + x < bmapDest.bmWidth ;
                ++x)
            {
                COLORREF crMask = *mask++;
                COLORREF crSrc = *src++;
                if (xDst + x >= 0)
                {
                    if (crMask == RGB(0, 0, 0) &&   // Only black mask bits get through
                        crSrc != crTrans)
                    {
                        *dest = crSrc;
                    }
                }
                ++dest;
            }
        }
        srcRowStart += srcStride;
        destRowStart += destStride;
        maskRowStart += maskStride;
    }
}

void CTileSheet::TransBltThruDIBSectMonoMask(wxDC& pDC, int xDst, int yDst, int ySrc,
    wxColour crTrans, const wxBitmap& pMaskBMapInfo) const
{
    /* TODO:  when CB::Convert() no longer required, compare
        performance of these different implementations */
#if 0
    // can't modify bitmap while it's selected
    class DCBitmapChanger
    {
    public:
        DCBitmapChanger(wxMemoryDC& d, wxBitmap select) :
            dc(d),
            oldBitmap(dc.GetSelectedBitmap())
        {
            wxASSERT(!oldBitmap.HasAlpha());
            dc.SelectObject(select);
        }
        ~DCBitmapChanger()
        {
            // modifying oldBitmap causes it to gain alpha channel
            oldBitmap.ResetAlpha();
            dc.SelectObject(oldBitmap);
        }
        wxBitmap& GetOldBitmap() { return oldBitmap; }
    private:
        wxMemoryDC& dc;
        wxBitmap oldBitmap;
    } setBitmap(dynamic_cast<wxMemoryDC&>(pDC), wxNullBitmap);
    wxBitmap& hBMapDest = setBitmap.GetOldBitmap();

    BITMAP  bmapTile;

    memset(&bmapTile, 0, sizeof(BITMAP));

    m_pBMap->GetObject(sizeof(BITMAP), &bmapTile);
    wxASSERT(bmapTile.bmBits != NULL && hBMapDest.IsOk());

    wxPoint pntOrg = pDC.GetLogicalOrigin();
    wxASSERT(pntOrg == wxPoint(0, 0) || !"untested code");
    xDst += pntOrg.x;
    yDst += pntOrg.y;

    wxASSERT(bmapTile.bmBitsPixel == 24);
    ptrdiff_t srcStride = -bmapTile.bmWidthBytes;
    const std::byte* srcRowStart = static_cast<const std::byte*>(bmapTile.bmBits) +
                                        (bmapTile.bmHeight - 1 - ySrc) * -srcStride;
    wxASSERT(hBMapDest.GetDepth() == 32);
    wxAlphaPixelData hBMapDestData(hBMapDest);
    wxAlphaPixelData::Iterator destRowStart(hBMapDestData);
    destRowStart.MoveTo(hBMapDestData, xDst, yDst);
    wxASSERT(pMaskBMapInfo.GetDepth() == 24);
    // KLUDGE:  only reading mask, so const_cast safe
    wxNativePixelData pMaskBMapInfoData(const_cast<wxBitmap&>(pMaskBMapInfo));
    wxNativePixelData::Iterator maskRowStart(pMaskBMapInfoData);

    for (int y = 0 ;
        y < std::min(value_preserving_cast<int>(m_size.cy).get_value(), pMaskBMapInfo.GetHeight()) &&
                yDst + y < hBMapDest.GetHeight() ;
        ++y)
    {
        if (yDst + y >= 0)
        {
            const WIN_RGBTRIO* src = reinterpret_cast<const WIN_RGBTRIO*>(srcRowStart);
            wxAlphaPixelData::Iterator dest = destRowStart;
            wxNativePixelData::Iterator mask = maskRowStart;
            for (int x = 0 ;
                x < std::min(value_preserving_cast<int>(m_size.cx).get_value(), pMaskBMapInfo.GetWidth()) &&
                        xDst + x < hBMapDest.GetWidth() ;
                ++x)
            {
                wxColour crMask = wxColour(mask.Red(), mask.Green(), mask.Blue());
                ++mask;
                wxColour crSrc = CB::Convert(COLORREF(*src++));
                if (xDst + x >= 0)
                {
                    if (crMask == RGB(0, 0, 0) &&   // Only black mask bits get through
                        crSrc != crTrans)
                    {
                        dest.Red() = crSrc.Red();
                        dest.Green() = crSrc.Green();
                        dest.Blue() = crSrc.Blue();
                    }
                }
                ++dest;
            }
        }
        srcRowStart += srcStride.get_value();
        destRowStart.OffsetY(hBMapDestData, 1);
        maskRowStart.OffsetY(pMaskBMapInfoData, 1);
    }
#else
    wxPoint pntOrg = pDC.GetLogicalOrigin();
    wxASSERT(pntOrg == wxPoint(0, 0) || !"untested code");
    xDst += pntOrg.x;
    yDst += pntOrg.y;

    wxBitmap accum(CB::Convert(m_size));
    {
        wxMemoryDC accumDC(accum);
        {
            wxMemoryDC tileDC;
            tileDC.SelectObjectAsSource(CB::Convert(*m_pBMap));
            accumDC.Blit(wxPoint(0, 0), accum.GetSize(), &tileDC, wxPoint(0, ySrc), wxCOPY);
        }
        {
            wxBitmap trans(accum.GetSize());
            {
                wxMemoryDC transDC(trans);
                transDC.SetPen(wxPen(crTrans));
                transDC.SetBrush(wxBrush(crTrans));
                transDC.DrawRectangle(wxPoint(0, 0), trans.GetSize());
            }
            trans.SetMask(new wxMask(pMaskBMapInfo, *wxBLACK));
            accumDC.DrawBitmap(trans, 0, 0, true);
        }
    }
    accum.SetMask(new wxMask(accum, crTrans));
    pDC.DrawBitmap(accum, xDst, yDst, true);
#endif
}

////////////////////////////////////////////////////////////////////////

void CTileSheet::ClearSheet()
{
    m_size = CSize(0, 0);
    m_pBMap = nullptr;
}

CTileSheet::SheetDC::SheetDC(const CTileSheet& sheet)
{
    ASSERT(sheet.m_pBMap);
    g_gt.mTileDC.SelectObject(*sheet.m_pBMap);
}

CTileSheet::SheetDC::~SheetDC()
{
    g_gt.SelectSafeObjectsForTileDC();
}

CTileSheet::SheetDC::operator CDC*() const
{
    return &g_gt.mTileDC;
}
