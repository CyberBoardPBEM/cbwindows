// TileMgr.cpp
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

#include    <stdafx.h>
#include    "WinExt.h"
#include    "CDib.h"
#include    "GdiTools.h"

#ifdef      GPLAY
    #include    "Gp.h"
    #include    "GamDoc.h"
    #include    "MapFace.h"
#else
    #include    "Gm.h"
    #include    "GmDoc.h"
#endif
#include    "Tile.h"
#include    "GMisc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////

CTileManager::CTileManager()
{
    m_crTrans = RGB(0, 255, 255);
    SetForeColor(RGB(0, 0, 0));
    SetBackColor(RGB(255, 255, 255));
    SetLineWidth(1);
    m_fontID = CGamDoc::GetFontManager().AddFont(TenthPointsToScreenPixels(80),
        0, uint8_t(FF_SWISS), "Arial");
    // --------- //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

void CTileManager::Clear()
{
    m_pTileTbl.Clear();
    m_TSetTbl.clear();
    m_TShtTbl.clear();
}

///////////////////////////////////////////////////////////////////////

CTile CTileManager::GetTile(TileID tid,
    TileScale eScale /* = fullScale */) const
{
    ASSERT(m_pTileTbl != NULL);
    ASSERT(m_pTileTbl.Valid(tid));
    ASSERT(!m_pTileTbl[tid].IsEmpty());

    CTile pTile;
    const TileDef *pDef = &m_pTileTbl[tid];
    if (eScale == fullScale)
    {
        const TileLoc *pLoc = &m_pTileTbl[tid].m_tileFull;
        pTile.m_pTS  = &GetTileSheet(pLoc->m_nSheet);
        pTile.m_yLoc = pLoc->m_nOffset;
    }
    else if (eScale == halfScale)
    {
        const TileLoc *pLoc = &m_pTileTbl[tid].m_tileHalf;
        pTile.m_pTS  = &GetTileSheet(pLoc->m_nSheet);
        pTile.m_yLoc = pLoc->m_nOffset;
    }
    else
    {
        pTile.m_pTS = NULL;
        pTile.m_crSmall = pDef->m_tileSmall;
        const TileLoc *pLoc = &m_pTileTbl[tid].m_tileFull;
        pTile.m_size = GetTileSheet(pLoc->m_nSheet).GetSize();
    }
    pTile.m_crTrans = m_crTrans;

    return pTile;
}

///////////////////////////////////////////////////////////////////////

TileID CTileManager::CreateTile(size_t nTSet, CSize sFull, CSize sHalf,
    COLORREF crSmall, size_t nPos /* = Invalid_v<size_t> */)
{
    ASSERT(nTSet < m_TSetTbl.size());
    TileID tid = m_pTileTbl.CreateIDEntry(&TileDef::SetEmpty);

    TileDef* pDef = &m_pTileTbl[tid];
    pDef->m_tileSmall = crSmall;

    CreateTileOnSheet(sFull, pDef->m_tileFull);
    CreateTileOnSheet(sHalf, pDef->m_tileHalf);

    GetTileSet(nTSet).AddTileID(tid, nPos);
    return tid;
}

void CTileManager::DeleteTile(TileID tid, BOOL bFromSetAlso /* = TRUE */)
{
    ASSERT(m_pTileTbl != NULL);
    ASSERT(m_pTileTbl.Valid(tid));
    ASSERT(!m_pTileTbl[tid].IsEmpty());

    TileDef& pDef = m_pTileTbl[tid];
    DeleteTileFromSheet(pDef.m_tileFull);
    DeleteTileFromSheet(pDef.m_tileHalf);
    if (bFromSetAlso)
        RemoveTileIDFromTileSets(tid);

    pDef.SetEmpty();
}

void CTileManager::UpdateTile(TileID tid, const CBitmap& pbmFull, const CBitmap& pbmHalf,
    COLORREF crSmall)
{
    ASSERT(m_pTileTbl != NULL);
    ASSERT(m_pTileTbl.Valid(tid));
    ASSERT(!m_pTileTbl[tid].IsEmpty());
    TileDef& pDef = m_pTileTbl[tid];

    // Update full and half scale tile bitmaps
    BITMAP bmInfo;

    pbmFull.GetObject(sizeof(bmInfo), &bmInfo);
    CSize bmSize(bmInfo.bmWidth, bmInfo.bmHeight);
    CTileUpdatable tile = GetTile(tid, fullScale);

    if (tile.GetSize() != bmSize)
    {
        DeleteTileFromSheet(pDef.m_tileFull);
        CreateTileOnSheet(bmSize, pDef.m_tileFull);
        tile = GetTile(tid, fullScale);
    }
    tile.Update(pbmFull);

    // Half scale tile.
    pbmHalf.GetObject(sizeof(bmInfo), &bmInfo);
    bmSize = CSize(bmInfo.bmWidth, bmInfo.bmHeight);
    tile = GetTile(tid, halfScale);

    if (tile.GetSize() != bmSize)
    {
        DeleteTileFromSheet(pDef.m_tileHalf);
        CreateTileOnSheet(bmSize, pDef.m_tileHalf);
        tile = GetTile(tid, halfScale);
    }
    tile.Update(pbmHalf);

    // Update small scale color...
    SetSmallTileColor(tid, crSmall);
}

BOOL CTileManager::IsTileIDValid(TileID tid) const
{
    for (size_t i = 0; i < GetNumTileSets(); i++)
    {
        const CTileSet& pTSet = GetTileSet(i);
        if (pTSet.HasTileID(tid))
            return TRUE;
    }
    return FALSE;
}

size_t CTileManager::FindTileSetFromTileID(TileID tid) const
{
    for (size_t i = 0; i < GetNumTileSets(); i++)
    {
        const CTileSet& pTSet = GetTileSet(i);
        if (pTSet.HasTileID(tid))
            return i;
    }
    return Invalid_v<size_t>;
}

size_t CTileManager::CreateTileSet(CB::string pszName)
{
    m_TSetTbl.resize(m_TSetTbl.size() + size_t(1));
    m_TSetTbl.back().SetName(std::move(pszName));
    return m_TSetTbl.size() - size_t(1);
}

size_t CTileManager::FindNamedTileSet(const CB::string& pszName) const
{
    for (size_t i = 0; i < GetNumTileSets(); i++)
    {
        const CTileSet& pTSet = GetTileSet(i);
        CB::string strName = pTSet.GetName();
        if (strName.CompareNoCase(pszName) == 0)
            return i;
    }
    return Invalid_v<size_t>;
}

void CTileManager::DeleteTileSet(size_t nTSet)
{
    const CTileSet& pTSet = GetTileSet(nTSet);
    const std::vector<TileID>& pTids = pTSet.GetTileIDTable();
    for (size_t i = 0; i < pTids.size(); i++)
        DeleteTile(pTids.at(i), FALSE);
    m_TSetTbl.erase(m_TSetTbl.begin() + value_preserving_cast<ptrdiff_t>(nTSet));
}

void CTileManager::SetSmallTileColor(TileID tid, COLORREF cr)
{
    ASSERT(m_pTileTbl != NULL);
    ASSERT(m_pTileTbl.Valid(tid));
    ASSERT(!m_pTileTbl[tid].IsEmpty());
    TileDef* pDef = &m_pTileTbl[tid];
    pDef->m_tileSmall = cr;
}

///////////////////////////////////////////////////////////////////////

size_t CTileManager::GetSheetForTile(CSize size)
{
    for (size_t i = 0; i < m_TShtTbl.size(); i++)
    {
        CTileSheet& pSht = *m_TShtTbl.at(i);
        if (pSht.IsSameDimensions(size) &&
            (pSht.GetSheetHeight() + size.cy < int(maxSheetHeight)))
            return i;
    }
    // If an empty one exists, reuse a tile sheet having a zero height...
    for (size_t i = 0; i < m_TShtTbl.size(); i++)
    {
        CTileSheet& pSht = *m_TShtTbl.at(i);
        if (pSht.GetSheetHeight() == 0)
        {
            pSht.SetSize(size);
            return i;
        }
    }
    TRACE2("Creating new tile sheet for cx=%d by cy=%d tile\n", size.cx, size.cy);
    m_TShtTbl.emplace_back(MakeOwner<CTileSheet>());
    m_TShtTbl.back()->SetSize(size);
    return m_TShtTbl.size() - 1;
}

void CTileManager::RemoveTileIDFromTileSets(TileID tid)
{
    for (size_t i = 0; i < GetNumTileSets(); i++)
    {
        CTileSet& pTSet = GetTileSet(i);
        if (pTSet.HasTileID(tid))
        {
            pTSet.RemoveTileID(tid);
            return;
        }
    }
    ASSERT(FALSE);
}

void CTileManager::MoveTileIDsToTileSet(size_t nTSet, const std::vector<TileID>& tidList,
    size_t nPos /* = Invalid_v<size_t> */)
{
    for (size_t i = 0; i < tidList.size(); i++)
    {
        TileID tid = tidList[i];
        size_t nCurSet = FindTileSetFromTileID(tid);
        ASSERT(nCurSet != Invalid_v<size_t>);
        if (nCurSet == nTSet && nPos != Invalid_v<size_t>)
        {
            if (nPos > GetTileSet(nTSet).FindTileID(tid))
                nPos--;
            ASSERT(nPos != Invalid_v<size_t>);
        }
        GetTileSet(nCurSet).RemoveTileID(tid);
        GetTileSet(nTSet).AddTileID(tid, nPos);
        if (nPos != Invalid_v<size_t>)
            nPos++;
    }
}

void CTileManager::CreateTileOnSheet(CSize size, TileLoc& pLoc)
{
    size_t nSht = GetSheetForTile(size);
    CTileSheet& pSht = *m_TShtTbl.at(nSht);
    pLoc.m_nSheet = nSht;
    pLoc.m_nOffset = pSht.GetSheetHeight();
    ASSERT(pLoc.m_nOffset < (int)65535U);
    pSht.CreateTile();         // Always creates tile at end filled white
}

void CTileManager::DeleteTileFromSheet(const TileLoc& pLoc)
{
    CTileSheet& pSht = *m_TShtTbl.at(pLoc.m_nSheet);
    pSht.DeleteTile(pLoc.m_nOffset);
    for (size_t i = 0; i < m_pTileTbl.GetSize(); i++)
    {
        AdjustTileLoc(m_pTileTbl[static_cast<TileID>(i)].m_tileFull, pLoc.m_nSheet,
            pLoc.m_nOffset, pSht.GetHeight());
        AdjustTileLoc(m_pTileTbl[static_cast<TileID>(i)].m_tileHalf, pLoc.m_nSheet,
            pLoc.m_nOffset, pSht.GetHeight());
    }
}

void CTileManager::AdjustTileLoc(TileLoc& pLoc, size_t nSht, int yLoc, int cy)
{
    if (pLoc.m_nSheet == nSht && pLoc.m_nOffset > yLoc)
        pLoc.m_nOffset -= cy;
}

///////////////////////////////////////////////////////////////////////

void CTileManager::CopyTileImagesToArchive(CArchive& ar,
    const std::vector<TileID>& tidsList)
{
#ifndef GPLAY
    ASSERT(ar.IsStoring());
    ar << value_preserving_cast<DWORD>(tidsList.size());
    for (size_t i = 0; i < tidsList.size(); i++)
    {
        CTile   tileFull;
        CTile   tileHalf;
        CTile   tileSmall;

        TileID tid = tidsList[i];

        tileFull = GetTile(tid, fullScale);
        tileHalf = GetTile(tid, halfScale);
        tileSmall = GetTile(tid, smallScale);

        ar << (DWORD)tileSmall.GetSmallColor();

        OwnerPtr<CBitmap> bitmap = tileFull.CreateBitmapOfTile();
        CDib dib(*bitmap);
        ASSERT(dib);
        ar << dib;

        bitmap = tileHalf.CreateBitmapOfTile();
        dib = CDib(*bitmap);
        ASSERT(dib);
        ar << dib;
    }
#endif
}

// Optionally fills the list with the TileID's of the created tiles.
void CTileManager::CreateTilesFromTileImageArchive(CArchive& ar,
    size_t nTSet, std::vector<TileID>* pTidTbl /* = NULL */, size_t nPos /* = Invalid_v<size_t> */)
{
#ifndef GPLAY
    ASSERT(ar.IsLoading());
    ASSERT(nTSet < GetNumTileSets());
    if (pTidTbl)
        pTidTbl->clear();

    DWORD nTileCount;
    ar >> nTileCount;
    if (pTidTbl)
    {
        pTidTbl->reserve(value_preserving_cast<size_t>(nTileCount));
    }
    for (DWORD i = 0; i < nTileCount; i++)
    {
        COLORREF    crSmall;
        BITMAP      bmInfoFull;
        BITMAP      bmInfoHalf;
        CDib        dib;
        DWORD       dwTmp;

        ar >> dwTmp; crSmall = (COLORREF)dwTmp;

        ar >> dib;
        OwnerPtr<CBitmap> pBMapFull = std::move(dib.DIBToBitmap());

        // >> expects empty dib
        dib.ClearDib();
        ar >> dib;
        OwnerPtr<CBitmap> pBMapHalf = std::move(dib.DIBToBitmap());

        VERIFY(pBMapFull->GetObject(sizeof(BITMAP), &bmInfoFull) > 0);
        VERIFY(pBMapHalf->GetObject(sizeof(BITMAP), &bmInfoHalf) > 0);

        TileID tid = CreateTile(nTSet,
            CSize(bmInfoFull.bmWidth, bmInfoFull.bmHeight),
            CSize(bmInfoHalf.bmWidth, bmInfoHalf.bmHeight),
            crSmall, nPos);
        if (nPos != Invalid_v<size_t>)
            nPos++;             // Position to next insertion point
        UpdateTile(tid, *pBMapFull, *pBMapHalf, crSmall);
        if (pTidTbl)
            pTidTbl->push_back(tid);
    }
#endif
}

///////////////////////////////////////////////////////////////////////

void CTileManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
#ifndef GPLAY
        ar << (DWORD)m_crTrans;

        ar << (DWORD)m_crFore;
        ar << (DWORD)m_crBack;
        ar << (WORD)m_nLineWidth;
        CFontTbl& pFontMgr = CGamDoc::GetFontManager();
        pFontMgr.Archive(ar, m_fontID);

        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;
        ar << m_pTileTbl;
#endif
    }
    else
    {
        Clear();
        WORD wTmp;
        DWORD dwTmp;
        ar >> dwTmp; m_crTrans = (COLORREF)dwTmp;

        ar >> dwTmp;
        SetForeColor((COLORREF)dwTmp);      // So brushes are created
        ar >> dwTmp;
        SetBackColor((COLORREF)dwTmp);      // So brushes are created

        ar >> wTmp; m_nLineWidth = (UINT)wTmp;
        /* delay font deletion in case archive read adds a
            reference */
        FontID tempRef = std::move(m_fontID);

        CFontTbl& pFontMgr = CGamDoc::GetFontManager();
        pFontMgr.Archive(ar, m_fontID);

        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;

        ar >> m_pTileTbl;
    }
    SerializeTileSets(ar);
    SerializeTileSheets(ar);
}

void CTileManager::SerializeTileSets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(GetNumTileSets());
        }
        else
        {
            CB::WriteCount(ar, GetNumTileSets());
        }
        for (size_t i = size_t(0); i < GetNumTileSets(); i++)
            GetTileSet(i).Serialize(ar);
    }
    else
    {
        size_t wSize;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            WORD temp;
            ar >> temp;
            wSize = temp;
        }
        else
        {
            wSize = CB::ReadCount(ar);
        }
        m_TSetTbl.resize(wSize);
        for (size_t i = size_t(0); i < m_TSetTbl.size(); i++)
        {
            m_TSetTbl[i].Serialize(ar);
        }
    }
}

void CTileManager::SerializeTileSheets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(m_TShtTbl.size());
        }
        else
        {
            CB::WriteCount(ar, m_TShtTbl.size());
        }
        for (size_t i = size_t(0); i < m_TShtTbl.size(); i++)
            m_TShtTbl.at(i)->Serialize(ar);
    }
    else
    {
        size_t wSize;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            WORD temp;
            ar >> temp;
            wSize = temp;
        }
        else
        {
            wSize = CB::ReadCount(ar);
        }
        m_TShtTbl.reserve(wSize);
        for (size_t i = size_t(0); i < wSize; i++)
        {
            m_TShtTbl.emplace_back(MakeOwner<CTileSheet>());
            ASSERT(i + size_t(1) == m_TShtTbl.size());
            CTileSheet& pTSht = *m_TShtTbl[i];
            pTSht.Serialize(ar);
#ifdef _DEBUG
            if (pTSht.GetSheetHeight() == 0)
                TRACE3(
                    "CTileManager::SerializeTileSheets - Zero length tile sheet"
                    "(index %zu, cx=%d, cy=%d) encountered.\n",
                    i, pTSht.GetWidth(), pTSht.GetHeight());
#endif
        }
    }
}

///////////////////////////////////////////////////////////////////////
// TOOL FOR FIXING CORRUPTED GAMEBOXES
// NOTE: This repair routine may not apply are gameboxes are upgraded
// to 2.91.
BOOL CTileManager::PruneTilesOnSheet255()
{
    BOOL bTileFound = FALSE;
    if (m_TShtTbl.size() < 256)
        return FALSE;

    for (size_t tid = 0; tid < m_pTileTbl.GetSize(); tid++)
    {
        TileDef& tp = m_pTileTbl[static_cast<TileID>(tid)];
        if (tp.m_tileFull.m_nSheet == size_t(0xFF) || tp.m_tileHalf.m_nSheet == size_t(0xFF))
        {
            size_t nSet = FindTileSetFromTileID(static_cast<TileID>(tid));
            if (nSet != Invalid_v<size_t>)
            {
                RemoveTileIDFromTileSets(static_cast<TileID>(tid));
                CB::string szBfr = std::format(L"Removed TileID {} from tile group:\n\n\"{}\".",
                    tid, GetTileSet(nSet).GetName());
                AfxMessageBox(szBfr);
                tp.m_tileFull.SetEmpty();
                tp.m_tileHalf.SetEmpty();
                tp.m_tileSmall = 0;
                bTileFound = TRUE;
            }
        }
    }
    // delete improper sheets...
    while (m_TShtTbl.size() > size_t(255))
    {
        m_TShtTbl.pop_back();
    }
    return bTileFound;
}

////////////////////////////////////////////////////////////////////////

static CB::string GetLastErrorAsText()
{
    wchar_t* lpMessageBuffer;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<wchar_t*>(&lpMessageBuffer), 0, NULL);
    CB::string msg = lpMessageBuffer;
    LocalFree(lpMessageBuffer);
    return msg;
}

static BOOL WriteFileString(HANDLE hFile, const CB::string& psz)
{
    DWORD dwBytesWritten;
    return WriteFile(hFile, psz.a_str(), value_preserving_cast<DWORD>(psz.a_size()), &dwBytesWritten, NULL);
}

static int compsheets(const void *elem1, const void *elem2)
{
    UINT* pElem1 = (UINT*)elem1;
    UINT* pElem2 = (UINT*)elem2;
    UINT nElem1 = (pElem1[1] << 16) + pElem1[2];
    UINT nElem2 = (pElem2[1] << 16) + pElem2[2];

    if (nElem1 < nElem2)
        return -1;
    else if (nElem1 > nElem2)
        return 1;
    else
        return 0;
}

void CTileManager::DumpTileDatabaseInfoToFile(const CB::string& pszFileName, BOOL bNewFile) const
{
    HANDLE hFile = CreateFile(pszFileName, GENERIC_WRITE, 0,
        NULL, bNewFile ? CREATE_ALWAYS : OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        CB::string strError = GetLastErrorAsText();
        AfxMessageBox(strError);
        return;
    }

    if (!bNewFile)
        SetFilePointer(hFile, 0, NULL, FILE_END);

    // Dump the tile set info...

    WriteFileString(hFile,
        "            Tile Sets\r\n"
        "+-----+----------------------------+\r\n"
        "| Set | Name                       | Tile's in set (in order)...\r\n"
        "+-----+----------------------------+----------------------------\r\n");

    for (size_t nSet = 0; nSet < m_TSetTbl.size(); nSet++)
    {
        const CTileSet& pTSet = m_TSetTbl.at(nSet);

        CB::string szBfr = std::format(L"| {:2}  | {:<25}  |", nSet, pTSet.GetName());
        WriteFileString(hFile, szBfr);
        for (size_t i = 0; i < pTSet.GetTileIDTable().size(); i++)
        {
            szBfr = std::format(L" {}", pTSet.GetTileIDTable().at(i));
            WriteFileString(hFile, szBfr);
        }
        WriteFileString(hFile, "\r\n");
    }

    WriteFileString(hFile,
        "+-----+----------------------------+\r\n");
    WriteFileString(hFile, "\r\n\r\n\r\n");


    // Dump the tile sheet info...

    WriteFileString(hFile,
        "     Tile Sheet Table\r\n"
        "+-------+-----+-----+-------+\r\n"
        "|  Sht  |  CX |  CY | ShtHt |\r\n"
        "+-------+-----+-----+-------+\r\n");

    for (size_t nSheet = 0; nSheet < m_TShtTbl.size(); nSheet++)
    {
        const CTileSheet& pTSheet = *m_TShtTbl.at(nSheet);

        CB::string szBfr = std::format(L"| {:04X}  | {:3} | {:3} | {:5} |\r\n", nSheet,
            pTSheet.GetWidth(), pTSheet.GetHeight(),
            pTSheet.GetSheetHeight());
        WriteFileString(hFile, szBfr);
    }
    WriteFileString(hFile,
        "+-------+-----+-----+-------+\r\n");

    WriteFileString(hFile, "\r\n\r\n\r\n");

    // Sort the tile sheet table by CX and CY and dump it again...

    uintmax_t *pShtTbl = new uintmax_t[size_t(4) * m_TShtTbl.size()];
    uintmax_t* pTbl = pShtTbl;
    for (size_t nSheet = size_t(0) ; nSheet < m_TShtTbl.size() ; nSheet++)
    {
        const CTileSheet& pTSheet = *m_TShtTbl.at(nSheet);
        *pTbl++ = nSheet;
        *pTbl++ = value_preserving_cast<uintmax_t>(pTSheet.GetWidth());
        *pTbl++ = value_preserving_cast<uintmax_t>(pTSheet.GetHeight());
        *pTbl++ = value_preserving_cast<uintmax_t>(pTSheet.GetSheetHeight());
    }
    qsort(pShtTbl, m_TShtTbl.size(), sizeof(uintmax_t) * 4, compsheets);

    WriteFileString(hFile,
        "     Tile Sheet Table\r\n"
        "   (sorted by dimension)\r\n"
        "+-------+-----+-----+-------+\r\n"
        "|  Sht  |  CX |  CY | ShtHt |\r\n"
        "+-------+-----+-----+-------+\r\n");

    pTbl = pShtTbl;
    for (size_t nSheet = 0; nSheet < m_TShtTbl.size(); nSheet++)
    {
        CB::string szBfr = std::format(L"| {:04X}  | {:3} | {:3} | {:5} |\r\n", pTbl[0], pTbl[1], pTbl[2], pTbl[3]);
        pTbl += 4;
        WriteFileString(hFile, szBfr);
    }
    WriteFileString(hFile,
        "+-------+-----+-----+-------+\r\n");

    WriteFileString(hFile, "\r\n\r\n\r\n");

    // Dump the tile table...

    WriteFileString(hFile,
        "                  TileID Table\r\n"
        "+------------+----------------+----------------+-------------+------+\r\n"
        "|     TID    |   FULL    SIZE |   HALF    SIZE | SMALL COLOR |  IN  |\r\n"
        "|            |   Sht      Off |   Sht      Off |     RGB     | TSET |\r\n"
        "+------------+----------------+----------------+-------------+------+\r\n");

    for (size_t tid = 0; tid < m_pTileTbl.GetSize(); tid++)
    {
        const TileDef& tp = m_pTileTbl[static_cast<TileID>(tid)];
        CB::string szBfr = std::format(L"| {:10} | {:08X} {:5} | {:08X} {:5} | 0x{:08X}  |", tid,
            tp.m_tileFull.m_nSheet, tp.m_tileFull.m_nOffset,
            tp.m_tileHalf.m_nSheet, tp.m_tileHalf.m_nOffset,
            tp.m_tileSmall);
        WriteFileString(hFile, szBfr);

        size_t nTSet = FindTileSetFromTileID(static_cast<TileID>(tid));
        if (nTSet != Invalid_v<size_t>)
            szBfr = std::format(L" {:3}  |\r\n", nTSet);
        else
            szBfr = " ***  |\r\n";
        WriteFileString(hFile, szBfr);
    }
    WriteFileString(hFile,
        "+------------+----------------+----------------+-------------+------+\r\n");

    CloseHandle(hFile);
}

////////////////////////////////////////////////////////////////////////

BOOL CTileManager::DoBitFontDialog()
{
    FontID newFontID = DoFontDialog(m_fontID, GetApp()->m_pMainWnd, TRUE);
    if (newFontID != (FontID)0)
    {
        m_fontID = newFontID;
        return TRUE;
    }
    return FALSE;
}

void CTileManager::SetForeColor(COLORREF cr)
{
    m_crFore = cr;
    m_brForeMFC.DeleteObject();
    m_brForeMFC.CreateSolidBrush(m_crFore);
    m_brForeWx = wxBrush(CB::Convert(m_crFore));
}

void CTileManager::SetBackColor(COLORREF cr)
{
    m_crBack = cr;
    m_brBack.DeleteObject();
    m_brBack.CreateSolidBrush(m_crBack);
}

#ifdef GPLAY
bool operator<(const std::pair<TileID, Rotation90>& lhs,
                const std::pair<TileID, Rotation90>& rhs)
{
    return static_cast<TileID::UNDERLYING_TYPE>(lhs.first) < static_cast<TileID::UNDERLYING_TYPE>(rhs.first) ||
        lhs.first == rhs.first && lhs.second < rhs.second;
}

TileID CTileManager::Get(TileID tid, Rotation90 rot)
{
    if (rot == Rotation90::r0)
    {
        ASSERT(!"no-op call");
        return tid;
    }
    GeoTiles::key_type key = std::make_pair(tid, rot);
    GeoTiles::iterator it = geoTiles.find(key);
    if (it == geoTiles.end())
    {
        CTile tile = GetTile(tid, fullScale);
        OwnerPtr<CBitmap> bmp = tile.CreateBitmapOfTile();
        OwnerPtr<CBitmap> rotBmpFull = Rotate(*bmp, rot);
        CSize sizeFull(rotBmpFull->GetBitmapDimension());

        tile = GetTile(tid, halfScale);
        bmp = tile.CreateBitmapOfTile();
        OwnerPtr<CBitmap> rotBmpHalf = Rotate(*bmp, rot);
        CSize sizeHalf(rotBmpHalf->GetBitmapDimension());

        tile = GetTile(tid, smallScale);
        COLORREF crSmall = tile.GetSmallColor();

        TileID tidNew = CreateTile(GetSpecialTileSet(),
            sizeFull, sizeHalf, crSmall);
        UpdateTile(tidNew, *rotBmpFull, *rotBmpHalf, crSmall);
        it = geoTiles.insert(std::make_pair(key, tidNew)).first;
    }
    return it->second;
}
#endif

////////////////////////////////////////////////////////////////////

void TileDef::Serialize(CArchive& ar)
{
    m_tileFull.Serialize(ar);
    m_tileHalf.Serialize(ar);
    if (ar.IsStoring())
        ar << (DWORD)m_tileSmall;
    else
    {
        DWORD dwTmp;
        ar >> dwTmp;
        m_tileSmall = (COLORREF)dwTmp;
    }
}

void TileLoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << (m_nSheet == noSheet ? noSheet16 : value_preserving_cast<uint16_t>(m_nSheet));
        }
        else
        {
            CB::WriteCount(ar, m_nSheet);
        }
        ar << value_preserving_cast<uint16_t>(m_nOffset);
    }
    else
    {
        if (CB::GetVersion(ar) < NumVersion(2, 91))   // V2.91
        {
            BYTE chTmp;
            ar >> chTmp;
            m_nSheet = (chTmp == 0xFF) ? noSheet : value_preserving_cast<size_t>(chTmp);
        }
        else if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))   // V3.90
        {
            uint16_t wTmp;
            ar >> wTmp;
            m_nSheet = (wTmp == noSheet16) ? noSheet : value_preserving_cast<size_t>(wTmp);
        }
        else
            m_nSheet = CB::ReadCount(ar);
        uint16_t wTmp;
        ar >> wTmp; m_nOffset = value_preserving_cast<int>(wTmp);
    }
}

