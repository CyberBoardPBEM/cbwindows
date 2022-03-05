// TileSht.cpp
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
            CDib dib(*m_pBMap, GetAppPalette());
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
                m_pBMap = dib.DIBToBitmap(GetAppPalette());
                BITMAP bmInfo;
                m_pBMap->GetObject(sizeof(bmInfo), &bmInfo);
                TRACE3("-- Loaded Tile Sheet for cx=%d, cy=%d tiles. Tile sheet height = %d\n",
                    m_size.cx, m_size.cy, bmInfo.bmHeight);
                //  ASSERT(bmInfo.bmHeight < maxSheetHeight);
                m_sheetHt = bmInfo.bmHeight;
                // If the DIB was in 256 color format and we are running
                // on Win9x/ME we need to remap all transparent colors
                // to the WinNT/2000/XP 16 bit equivalent.
                if (dib.NumColors() == 8)       // (should be changed to NumColorBits!)
                {
                    COLORREF crTrans = ((CGamDoc*)ar.m_pDocument)->
                        GetTileManager()->GetTransparentColor();
                    FixupTransparentColorsAfter256ColorDibUpgrade(
                        (HBITMAP)m_pBMap->m_hObject, crTrans);
                }
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

        SetupPalette(g_gt.mDC1);
        SetupPalette(g_gt.mDC2);

        OwnerPtr<CBitmap> pBMap = MakeOwner<CBitmap>();
        pBMap->Attach(Create16BitDIBSection(g_gt.mDC1.m_hDC,
            bmInfo.bmWidth, bmInfo.bmHeight));
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
        m_pBMap = MakeOwner<CBitmap>();
        SetupPalette(g_gt.mDC1);
        m_pBMap->Attach(Create16BitDIBSection(g_gt.mDC1.m_hDC,
            m_size.cx, m_size.cy));
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

        OwnerPtr<CBitmap> pBMap = MakeOwner<CBitmap>();
        g_gt.mDC2.SelectObject(m_pBMap.get());        // Source bitmap
        SetupPalette(g_gt.mDC2);

        pBMap->Attach(Create16BitDIBSection(g_gt.mDC2.m_hDC,
            bmInfo.bmWidth, bmInfo.bmHeight));

        g_gt.mDC1.SelectObject(pBMap.get());          // Dest bitmap
        SetupPalette(g_gt.mDC1);

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
    SetupPalette(g_gt.mDC1);
    g_gt.mDC2.SelectObject(pBMap);          // Source bitmap
    SetupPalette(g_gt.mDC2);

    g_gt.mDC1.BitBlt(0, yLoc, m_size.cx, m_size.cy, &g_gt.mDC2, 0, 0, SRCCOPY);

    g_gt.SelectSafeObjectsForDC1();
    g_gt.SelectSafeObjectsForDC2();
}

OwnerPtr<CBitmap> CTileSheet::CreateBitmapOfTile(int yLoc) const
{
    ASSERT(m_pBMap != NULL);
    ASSERT(yLoc < m_sheetHt - 1);
    g_gt.mDC1.SelectObject(*m_pBMap);        // Source bitmap
    SetupPalette(g_gt.mDC1);

    OwnerPtr<CBitmap> pBMap(MakeOwner<CBitmap>());

    BITMAP bmap;
    memset(&bmap, 0, sizeof(BITMAP));
    m_pBMap->GetObject(sizeof(BITMAP), &bmap);

    if (bmap.bmBits != NULL)                // DIB Section check
    {
        pBMap->Attach(Create16BitDIBSection(g_gt.mDC1.m_hDC,
            m_size.cx, m_size.cy));
    }
    else
    {
        pBMap->CreateCompatibleBitmap(&g_gt.mDC1, m_size.cx, m_size.cy);
    }

    g_gt.mDC2.SelectObject(&*pBMap);          // Activate dest bitmap
    SetupPalette(g_gt.mDC2);

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

    LPBYTE pTile = (LPBYTE)bmapTile.bmBits;
    LPBYTE pDest = (LPBYTE)bmapDest.bmBits;

    WORD cr16Trans = RGB565(crTrans);

    CPoint pntOrg = pDC.GetViewportOrg();
    xDst += pntOrg.x;
    yDst += pntOrg.y;
    int xDstBase = xDst;

    int nBytesPerScanLineTile = WIDTHBYTES(bmapTile.bmWidth * 16);
    int nBytesPerScanLineDest = WIDTHBYTES(bmapDest.bmWidth * 16);
    for (int nScanLine = 0; nScanLine < m_size.cy; nScanLine++)
    {
        xDst = xDstBase;
        WORD* pPxlTile = (WORD*)(pTile + (bmapTile.bmHeight - ySrc - 1) *
            nBytesPerScanLineTile);
        WORD* pPxlDest = (WORD*)(pDest + (bmapDest.bmHeight - yDst - 1) *
            nBytesPerScanLineDest + 2 * xDst); // Two bytes per pixel
        for (int nPxl = 0; nPxl < m_size.cx; nPxl++)
        {
            if (*pPxlTile != cr16Trans &&
                xDst >= 0 && xDst < bmapDest.bmWidth &&
                yDst >= 0 && yDst < bmapDest.bmHeight)
            {
                *pPxlDest = *pPxlTile;
            }
            pPxlDest++;
            pPxlTile++;
            xDst++;
        }
        yDst++;
        ySrc++;
    }
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

    LPBYTE pTile = (LPBYTE)bmapTile.bmBits;
    LPBYTE pDest = (LPBYTE)bmapDest.bmBits;
    LPBYTE pMask = (LPBYTE)pMaskBMapInfo.bmBits;

    WORD cr16Trans = RGB565(crTrans);
    CPoint pntOrg = pDC.GetViewportOrg();
    xDst += pntOrg.x;
    yDst += pntOrg.y;
    int xDstBase = xDst;

    int nBytesPerScanLineTile = WIDTHBYTES(bmapTile.bmWidth * 16);
    int nBytesPerScanLineDest = WIDTHBYTES(bmapDest.bmWidth * 16);
    int nBytesPerScanLineMask = WIDTHBYTES(pMaskBMapInfo.bmWidth * 16);
    for (int nScanLine = 0; nScanLine < m_size.cy; nScanLine++)
    {
        xDst = xDstBase;

        WORD* pPxlTile = (WORD*)(pTile + (bmapTile.bmHeight - ySrc - 1) *
            nBytesPerScanLineTile);
        WORD* pPxlDest = (WORD*)(pDest + (bmapDest.bmHeight - yDst - 1) *
            nBytesPerScanLineDest + 2 * xDst); // Two bytes per pixel
        WORD* pPxlMask = (WORD*)(pMask + (pMaskBMapInfo.bmHeight - nScanLine - 1) *
            nBytesPerScanLineMask);

        for (int nPxl = 0; nPxl < m_size.cx; nPxl++)
        {
            if (nScanLine < pMaskBMapInfo.bmHeight &&  // Still in mask?
                nPxl < pMaskBMapInfo.bmWidth &&        // Still in mask?
                *pPxlMask == 0 &&                       // Only black mask bits get through
                *pPxlTile != cr16Trans &&
                xDst >= 0 && xDst < bmapDest.bmWidth &&
                yDst >= 0 && yDst < bmapDest.bmHeight)
            {
                *pPxlDest = *pPxlTile;
            }
            pPxlDest++;
            pPxlTile++;
            pPxlMask++;

            xDst++;
        }
        yDst++;
        ySrc++;
    }
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
    g_gt.mTileDC.SelectPalette(GetAppPalette(), TRUE);
}

CTileSheet::SheetDC::~SheetDC()
{
    g_gt.SelectSafeObjectsForTileDC();
}

CTileSheet::SheetDC::operator CDC*() const
{
    return &g_gt.mTileDC;
}
