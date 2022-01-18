// MapFace.cpp
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

#include    <stdafx.h>
#include    "Gp.h"
#include    "GamDoc.h"
#include    "CDib.h"

///////////////////////////////////////////////////////////////////////

void CTileFacingMap::SetTileManager(CTileManager* pTileMgr)
{
    ASSERT(m_pTMgr == NULL);
    ASSERT(pTileMgr != NULL);
    m_pTMgr = pTileMgr;
    // All automatically generated rotated tiles are created in
    // the following hidden tile group.
    m_nTileSet = m_pTMgr->CreateTileSet("##-AutoFacings-##");
}

TileID CTileFacingMap::GetFacingTileID(ElementState state)
{
    ASSERT(m_pTMgr != NULL);
    TileID tid;
    if (Lookup(state, tid))
        return tid;
    else
        return nullTid;
}

TileID CTileFacingMap::CreateFacingTileID(ElementState state, TileID baseTileID)
{
    ASSERT(m_pTMgr != NULL);
    CDib    dibSrc;
    CTile   tile;
    CBitmap bmap;
    uint16_t nAngleDegCW = state.GetFacing();

    // Generate rotated full scale bitmap of tile...

    m_pTMgr->GetTile(baseTileID, &tile, fullScale);
    tile.CreateBitmapOfTile(&bmap);
    dibSrc.BitmapToDIB(&bmap, GetAppPalette());

    OwnerPtr<CDib> pRDib = Rotate16BitDib(&dibSrc, nAngleDegCW, m_pTMgr->GetTransparentColor());
    OwnerPtr<CBitmap> pBMapFull = pRDib->DIBToBitmap(GetAppPalette());
    BITMAP bmapInfo;
    pBMapFull->GetObject(sizeof(BITMAP), &bmapInfo);
    CSize sizeFull(bmapInfo.bmWidth, bmapInfo.bmHeight);

    // Generate rotated halfScale scale bitmap of tile...

    m_pTMgr->GetTile(baseTileID, &tile, halfScale);
    tile.CreateBitmapOfTile(&bmap);
    dibSrc.BitmapToDIB(&bmap, GetAppPalette());

    pRDib = Rotate16BitDib(&dibSrc, nAngleDegCW, m_pTMgr->GetTransparentColor());
    OwnerPtr<CBitmap> pBMapHalf = pRDib->DIBToBitmap(GetAppPalette());

    pBMapHalf->GetObject(sizeof(BITMAP), &bmapInfo);
    CSize sizeHalf(bmapInfo.bmWidth, bmapInfo.bmHeight);

    // Fetch color of small scale tile
    m_pTMgr->GetTile(baseTileID, &tile, smallScale);
    COLORREF crSmall = tile.GetSmallColor();

    // Create the tile in our special tile set...
    TileID tidNew = m_pTMgr->CreateTile(m_nTileSet, sizeFull, sizeHalf, crSmall);
    m_pTMgr->UpdateTile(tidNew, pBMapFull.get(), pBMapHalf.get(), crSmall);

    // Finally, add the piece facing mapping...
    SetAt(state, tidNew);

    return tidNew;
}

