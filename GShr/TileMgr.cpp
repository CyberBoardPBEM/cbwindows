// TileMgr.cpp
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

const UINT tblBaseSize = 32;            // TileDef table allocation strategy
const UINT tblIncrSize = 8;

///////////////////////////////////////////////////////////////////////

CTileManager::CTileManager()
{
    m_pTileTbl = NULL;
    m_nTblSize = 0;
    m_crTrans = RGB(0, 255, 255);
    SetForeColor(RGB(0, 0, 0));
    SetBackColor(RGB(255, 255, 255));
    SetLineWidth(1);
    m_fontID = CGamDoc::GetFontManager()->AddFont(TenthPointsToScreenPixels(80),
        0, FF_SWISS, "Arial");
    // --------- //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

CTileManager::~CTileManager()
{
    Clear();
}

void CTileManager::Clear()
{
    if (m_pTileTbl != NULL) GlobalFreePtr(m_pTileTbl);
    m_pTileTbl = NULL;
    m_nTblSize = 0;
    for (int i = 0; i < m_TSetTbl.GetSize(); i++)
        delete (CTileSet*)m_TSetTbl.GetAt(i);
    m_TSetTbl.RemoveAll();
    for (int i = 0; i < m_TShtTbl.GetSize(); i++)
        delete (CTileSheet*)m_TShtTbl.GetAt(i);
    m_TShtTbl.RemoveAll();
}

///////////////////////////////////////////////////////////////////////

void CTileManager::GetTile(TileID tid, CTile* pTile,
    TileScale eScale /* = fullScale */)
{
    ASSERT(m_pTileTbl != NULL);
    ASSERT(tid < m_nTblSize);
    ASSERT(!m_pTileTbl[tid].IsEmpty());

    TileDef *pDef = &m_pTileTbl[tid];
    if (eScale == fullScale)
    {
        TileLoc *pLoc = &m_pTileTbl[tid].m_tileFull;
        pTile->m_pTS  = GetTileSheet(pLoc->m_nSheet);
        pTile->m_yLoc = pLoc->m_nOffset;
    }
    else if (eScale == halfScale)
    {
        TileLoc *pLoc = &m_pTileTbl[tid].m_tileHalf;
        pTile->m_pTS  = GetTileSheet(pLoc->m_nSheet);
        pTile->m_yLoc = pLoc->m_nOffset;
    }
    else
    {
        pTile->m_pTS = NULL;
        pTile->m_crSmall = pDef->m_tileSmall;
        TileLoc *pLoc = &m_pTileTbl[tid].m_tileFull;
        pTile->m_size = GetTileSheet(pLoc->m_nSheet)->GetSize();
    }
    pTile->m_crTrans = m_crTrans;
}

///////////////////////////////////////////////////////////////////////

TileID CTileManager::CreateTile(int nTSet, CSize sFull, CSize sHalf,
    COLORREF crSmall, int nPos /* = -1 */)
{
    ASSERT(nTSet < m_TSetTbl.GetSize());
    TileID tid = CreateTileIDEntry();

    TileDef* pDef = &m_pTileTbl[tid];
    pDef->m_tileSmall = crSmall;

    CreateTileOnSheet(sFull, &pDef->m_tileFull);
    CreateTileOnSheet(sHalf, &pDef->m_tileHalf);

    GetTileSet(nTSet)->AddTileID(tid, nPos);
    return tid;
}

void CTileManager::DeleteTile(TileID tid, BOOL bFromSetAlso /* = TRUE */)
{
    ASSERT(m_pTileTbl != NULL);
    ASSERT(tid < m_nTblSize);
    ASSERT(!m_pTileTbl[tid].IsEmpty());

    TileDef* pDef = &m_pTileTbl[tid];
    DeleteTileFromSheet(&pDef->m_tileFull);
    DeleteTileFromSheet(&pDef->m_tileHalf);
    if (bFromSetAlso)
        RemoveTileIDFromTileSets(tid);

    pDef->SetEmpty();
}

void CTileManager::UpdateTile(TileID tid, CBitmap* pbmFull, CBitmap* pbmHalf,
    COLORREF crSmall)
{
    ASSERT(m_pTileTbl != NULL);
    ASSERT(tid < m_nTblSize);
    ASSERT(!m_pTileTbl[tid].IsEmpty());
    TileDef* pDef = &m_pTileTbl[tid];

    // Update full and half scale tile bitmaps
    CTile tile;
    BITMAP bmInfo;

    pbmFull->GetObject(sizeof(bmInfo), &bmInfo);
    CSize bmSize(bmInfo.bmWidth, bmInfo.bmHeight);
    GetTile(tid, &tile, fullScale);

    if (tile.GetSize() != bmSize)
    {
        DeleteTileFromSheet(&pDef->m_tileFull);
        CreateTileOnSheet(bmSize, &pDef->m_tileFull);
        GetTile(tid, &tile, fullScale);
    }
    tile.Update(pbmFull);

    // Half scale tile.
    pbmHalf->GetObject(sizeof(bmInfo), &bmInfo);
    bmSize = CSize(bmInfo.bmWidth, bmInfo.bmHeight);
    GetTile(tid, &tile, halfScale);

    if (tile.GetSize() != bmSize)
    {
        DeleteTileFromSheet(&pDef->m_tileHalf);
        CreateTileOnSheet(bmSize, &pDef->m_tileHalf);
        GetTile(tid, &tile, halfScale);
    }
    tile.Update(pbmHalf);

    // Update small scale color...
    SetSmallTileColor(tid, crSmall);
}

BOOL CTileManager::IsTileIDValid(TileID tid)
{
    for (int i = 0; i < GetNumTileSets(); i++)
    {
        CTileSet* pTSet = GetTileSet(i);
        if (pTSet->HasTileID(tid))
            return TRUE;
    }
    return FALSE;
}

int CTileManager::FindTileSetFromTileID(TileID tid)
{
    for (int i = 0; i < GetNumTileSets(); i++)
    {
        CTileSet* pTSet = GetTileSet(i);
        if (pTSet->HasTileID(tid))
            return i;
    }
    return -1;
}

int CTileManager::CreateTileSet(const char* pszName)
{
    CTileSet* pTSet = new CTileSet;
    pTSet->SetName(pszName);
    m_TSetTbl.Add(pTSet);
    return m_TSetTbl.GetSize() - 1;
}

int CTileManager::FindNamedTileSet(const char* pszName)
{
    for (int i = 0; i < GetNumTileSets(); i++)
    {
        CTileSet* pTSet = GetTileSet(i);
        CString strName = pTSet->GetName();
        if (strName.CompareNoCase(pszName))
            return i;
    }
    return -1;
}

void CTileManager::DeleteTileSet(int nTSet)
{
    CTileSet* pTSet = GetTileSet(nTSet);
    CWordArray* pTids = pTSet->GetTileIDTable();
    for (int i = 0; i < pTids->GetSize(); i++)
        DeleteTile((TileID)pTids->GetAt(i), FALSE);
    m_TSetTbl.RemoveAt(nTSet);
    delete pTSet;
}

void CTileManager::SetSmallTileColor(TileID tid, COLORREF cr)
{
    ASSERT(m_pTileTbl != NULL);
    ASSERT(tid < m_nTblSize);
    ASSERT(!m_pTileTbl[tid].IsEmpty());
    TileDef* pDef = &m_pTileTbl[tid];
    pDef->m_tileSmall = cr;
}

///////////////////////////////////////////////////////////////////////

void CTileManager::ResizeTileTable(UINT nEntsNeeded)
{
    UINT nNewSize = CalcAllocSize(nEntsNeeded, tblBaseSize, tblIncrSize);
    if (m_pTileTbl != NULL)
    {
        TileDef * pNewTbl = (TileDef *)GlobalReAllocPtr(
            m_pTileTbl, (DWORD)nNewSize * sizeof(TileDef), GHND);
        if (pNewTbl == NULL)
            AfxThrowMemoryException();
        m_pTileTbl = pNewTbl;
    }
    else
    {
        m_pTileTbl = (TileDef *)GlobalAllocPtr(GHND,
            (DWORD)nNewSize * sizeof(TileDef));
        if (m_pTileTbl == NULL)
            AfxThrowMemoryException();
    }
    for (UINT i = m_nTblSize; i < nNewSize; i++)
        m_pTileTbl[i].SetEmpty();
    m_nTblSize = nNewSize;
}

///////////////////////////////////////////////////////////////////////

int CTileManager::GetSheetForTile(CSize size)
{
    CTileSheet* pSht;
    for (int i = 0; i < m_TShtTbl.GetSize(); i++)
    {
        pSht = (CTileSheet*)m_TShtTbl.GetAt(i);
        ASSERT(pSht != NULL);
        if (pSht->IsSameDimensions(size) &&
            (pSht->GetSheetHeight() + size.cy < maxSheetHeight))
            return i;
    }
    // If an empty one exists, reuse a tile sheet having a zero height...
    for (int i = 0; i < m_TShtTbl.GetSize(); i++)
    {
        pSht = (CTileSheet*)m_TShtTbl.GetAt(i);
        ASSERT(pSht != NULL);
        if (pSht->GetSheetHeight() == 0)
        {
            pSht->SetSize(size);
            return i;
        }
    }
    TRACE2("Creating new tile sheet for cx=%d by cy=%d tile\n", size.cx, size.cy);
    pSht = new CTileSheet(size);
    m_TShtTbl.Add(pSht);
    return m_TShtTbl.GetSize() - 1;
}

TileID CTileManager::CreateTileIDEntry()
{
    // Allocate from empty entry if possible
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        if (m_pTileTbl[i].IsEmpty())
            return (TileID)i;
    }
    // Get TileID from end of table.
    TileID newTid = m_nTblSize;
    ResizeTileTable(m_nTblSize + 1);
    return newTid;
}

void CTileManager::RemoveTileIDFromTileSets(TileID tid)
{
    for (int i = 0; i < GetNumTileSets(); i++)
    {
        CTileSet* pTSet = GetTileSet(i);
        if (pTSet->HasTileID(tid))
        {
            pTSet->RemoveTileID(tid);
            return;
        }
    }
    ASSERT(FALSE);
}

void CTileManager::MoveTileIDsToTileSet(int nTSet, CWordArray& tidList,
    int nPos /* = -1 */)
{
    for (int i = 0; i < tidList.GetSize(); i++)
    {
        TileID tid = (TileID)tidList[i];
        int nCurSet = FindTileSetFromTileID(tid);
        ASSERT(nCurSet >= 0);
        if (nCurSet == nTSet && nPos >= 0)
        {
            if (nPos > GetTileSet(nTSet)->FindTileID(tid))
                nPos--;
            ASSERT(nPos >= 0);
        }
        GetTileSet(nCurSet)->RemoveTileID(tid);
        GetTileSet(nTSet)->AddTileID(tid, nPos);
        if (nPos >= 0)
            nPos++;
    }
}

void CTileManager::CreateTileOnSheet(CSize size, TileLoc* pLoc)
{
    int nSht = GetSheetForTile(size);
    CTileSheet* pSht = (CTileSheet*)m_TShtTbl.GetAt(nSht);
    pLoc->m_nSheet = nSht;
    pLoc->m_nOffset = pSht->GetSheetHeight();
    ASSERT(pLoc->m_nOffset < (int)65535U);
    pSht->CreateTile();         // Always creates tile at end filled white
}

void CTileManager::DeleteTileFromSheet(TileLoc *pLoc)
{
    CTileSheet* pSht = (CTileSheet*)m_TShtTbl.GetAt(pLoc->m_nSheet);
    pSht->DeleteTile(pLoc->m_nOffset);
    for (UINT i = 0; i < m_nTblSize; i++)
    {
        AdjustTileLoc(&m_pTileTbl[i].m_tileFull, pLoc->m_nSheet,
            pLoc->m_nOffset, pSht->GetHeight());
        AdjustTileLoc(&m_pTileTbl[i].m_tileHalf, pLoc->m_nSheet,
            pLoc->m_nOffset, pSht->GetHeight());
    }
}

void CTileManager::AdjustTileLoc(TileLoc *pLoc, int nSht, int yLoc, int cy)
{
    if (pLoc->m_nSheet == nSht && pLoc->m_nOffset > yLoc)
        pLoc->m_nOffset -= cy;
}

///////////////////////////////////////////////////////////////////////

void CTileManager::CopyTileImagesToArchive(CArchive& ar,
    CWordArray& tidsList)
{
#ifndef GPLAY
    ASSERT(ar.IsStoring());
    ar << (DWORD)tidsList.GetSize();
    for (int i = 0; i < tidsList.GetSize(); i++)
    {
        CTile   tileFull;
        CTile   tileHalf;
        CTile   tileSmall;
        CBitmap bitmap;
        CDib    dib;

        TileID tid = (TileID)tidsList[i];

        GetTile(tid, &tileFull, fullScale);
        GetTile(tid, &tileHalf, halfScale);
        GetTile(tid, &tileSmall, smallScale);

        ar << (DWORD)tileSmall.GetSmallColor();

        tileFull.CreateBitmapOfTile(&bitmap);
        VERIFY(dib.BitmapToDIB(&bitmap, GetAppPalette()));
        ar << dib;

        tileHalf.CreateBitmapOfTile(&bitmap);
        VERIFY(dib.BitmapToDIB(&bitmap, GetAppPalette()));
        ar << dib;
    }
#endif
}

// Optionally fills the list with the TileID's of the created tiles.
void CTileManager::CreateTilesFromTileImageArchive(CArchive& ar,
    int nTSet, CWordArray* pTidTbl /* = NULL */, int nPos /* = -1 */)
{
#ifndef GPLAY
    ASSERT(ar.IsLoading());
    ASSERT(nTSet >= 0 && nTSet < GetNumTileSets());
    if (pTidTbl)
        pTidTbl->RemoveAll();

    DWORD nTileCount;
    ar >> nTileCount;
    for (int i = 0; i < (int)nTileCount; i++)
    {
        CBitmap*    pBMapFull;
        CBitmap*    pBMapHalf;
        COLORREF    crSmall;
        BITMAP      bmInfoFull;
        BITMAP      bmInfoHalf;
        CDib        dib;
        DWORD       dwTmp;

        ar >> dwTmp; crSmall = (COLORREF)dwTmp;

        ar >> dib;
        pBMapFull = dib.DIBToBitmap(GetAppPalette());
        ASSERT(pBMapFull != NULL);

        ar >> dib;
        pBMapHalf = dib.DIBToBitmap(GetAppPalette());
        ASSERT(pBMapHalf != NULL);

        VERIFY(pBMapFull->GetObject(sizeof(BITMAP), &bmInfoFull) > 0);
        VERIFY(pBMapHalf->GetObject(sizeof(BITMAP), &bmInfoHalf) > 0);

        TileID tid = CreateTile(nTSet,
            CSize(bmInfoFull.bmWidth, bmInfoFull.bmHeight),
            CSize(bmInfoHalf.bmWidth, bmInfoHalf.bmHeight),
            crSmall, nPos);
        if (nPos >= 0)
            nPos++;             // Position to next insertion point
        UpdateTile(tid, pBMapFull, pBMapHalf, crSmall);
        if (pTidTbl)
            pTidTbl->Add((WORD)tid);

        delete pBMapFull;
        delete pBMapHalf;
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
        CFontTbl* pFontMgr = CGamDoc::GetFontManager();
        pFontMgr->Archive(ar, m_fontID);

        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;
        ar << (WORD)m_nTblSize;
        for (UINT i = 0; i < m_nTblSize; i++)
            m_pTileTbl[i].Serialize(ar);
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
        m_fontID = 0;

        CFontTbl* pFontMgr = CGamDoc::GetFontManager();
        pFontMgr->Archive(ar, m_fontID);

        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;

        ar >> wTmp;
        if (wTmp > 0)
        {
            ResizeTileTable((UINT)wTmp);
            for (UINT i = 0; i < (UINT)wTmp; i++)
                m_pTileTbl[i].Serialize(ar);
        }
    }
    SerializeTileSets(ar);
    SerializeTileSheets(ar);
}

void CTileManager::SerializeTileSets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (WORD)GetNumTileSets();
        for (int i = 0; i < GetNumTileSets(); i++)
            GetTileSet(i)->Serialize(ar);
    }
    else
    {
        WORD wSize;
        ar >> wSize;
        for (int i = 0; i < (int)wSize; i++)
        {
            CTileSet* pTSet = new CTileSet;
            pTSet->Serialize(ar);
            m_TSetTbl.Add(pTSet);
        }
    }
}

void CTileManager::SerializeTileSheets(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (WORD)m_TShtTbl.GetSize();
        for (int i = 0; i < m_TShtTbl.GetSize(); i++)
            ((CTileSheet*)m_TShtTbl.GetAt(i))->Serialize(ar);
    }
    else
    {
        WORD wSize;
        ar >> wSize;
        for (int i = 0; i < (int)wSize; i++)
        {
            CTileSheet* pTSht = new CTileSheet;
            pTSht->Serialize(ar);
            m_TShtTbl.Add(pTSht);
#ifdef _DEBUG
            if (pTSht->GetSheetHeight() == 0)
                TRACE3(
                    "CTileManager::SerializeTileSheets - Zero length tile sheet"
                    "(index %d, cx=%d, cy=%d) encountered.\n",
                    i, pTSht->GetWidth(), pTSht->GetHeight());
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
    if (m_TShtTbl.GetSize() < 256)
        return FALSE;

    for (UINT tid = 0; tid < m_nTblSize; tid++)
    {
        TileDef* tp = &m_pTileTbl[tid];
        if (tp->m_tileFull.m_nSheet == 0xFF || tp->m_tileHalf.m_nSheet == 0xFF)
        {
            int nSet = FindTileSetFromTileID((TileID)tid);
            if (nSet >= 0)
            {
                RemoveTileIDFromTileSets((TileID)tid);
                char szBfr[256];
                wsprintf(szBfr, "Removed TileID %u from tile group:\n\n\"%s\".",
                    tid, GetTileSet(nSet)->GetName());
                AfxMessageBox(szBfr);
                tp->m_tileFull.SetEmpty();
                tp->m_tileHalf.SetEmpty();
                tp->m_tileSmall = 0;
                bTileFound = TRUE;
            }
        }
    }
    // delete improper sheets...
    for (int nSheet = 255; nSheet < m_TShtTbl.GetSize(); nSheet++)
    {
        CTileSheet* pTSheet = (CTileSheet*)m_TShtTbl.GetAt(nSheet);
        delete pTSheet;
    }
    m_TShtTbl.SetSize(255);
    return bTileFound;
}

////////////////////////////////////////////////////////////////////////

static CString GetLastErrorAsText()
{
    LPVOID lpMessageBuffer;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR)&lpMessageBuffer,  0,  NULL);
    CString msg;
    msg = (char*)lpMessageBuffer;
    LocalFree(lpMessageBuffer);
    return msg;
}

static BOOL WriteFileString(HANDLE hFile, LPCSTR psz)
{
    DWORD dwBytesWritten;
    return WriteFile(hFile, psz, lstrlen(psz), &dwBytesWritten, NULL);
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

void CTileManager::DumpTileDatabaseInfoToFile(LPCTSTR pszFileName, BOOL bNewFile)
{
    char  szBfr[256];

    HANDLE hFile = CreateFile(pszFileName, GENERIC_WRITE, 0,
        NULL, bNewFile ? CREATE_ALWAYS : OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        CString strError = GetLastErrorAsText();
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

    for (UINT nSet = 0; nSet < (UINT)m_TSetTbl.GetSize(); nSet++)
    {
        CTileSet* pTSet = (CTileSet*)m_TSetTbl.GetAt(nSet);

        wsprintf(szBfr, "| %2u  | %-25s  |", nSet, pTSet->GetName());
        WriteFileString(hFile, szBfr);
        for (int i = 0; i < pTSet->GetTileIDTable()->GetSize(); i++)
        {
            wsprintf(szBfr, " %u", pTSet->GetTileIDTable()->GetAt(i));
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

    for (UINT nSheet = 0; nSheet < (UINT)m_TShtTbl.GetSize(); nSheet++)
    {
        CTileSheet* pTSheet = (CTileSheet*)m_TShtTbl.GetAt(nSheet);

        wsprintf(szBfr, "| %04X  | %3u | %3u | %5u |\r\n", nSheet,
            pTSheet->GetWidth(), pTSheet->GetHeight(),
            pTSheet->GetSheetHeight());
        WriteFileString(hFile, szBfr);
    }
    WriteFileString(hFile,
        "+-------+-----+-----+-------+\r\n");

    WriteFileString(hFile, "\r\n\r\n\r\n");

    // Sort the tile sheet table by CX and CY and dump it again...

    UINT *pShtTbl = new UINT[4 * m_TShtTbl.GetSize()];
    UINT* pTbl = pShtTbl;
    for (UINT nSheet = 0; nSheet < (UINT)m_TShtTbl.GetSize(); nSheet++)
    {
        CTileSheet* pTSheet = (CTileSheet*)m_TShtTbl.GetAt(nSheet);
        *pTbl++ = nSheet;
        *pTbl++ = pTSheet->GetWidth();
        *pTbl++ = pTSheet->GetHeight();
        *pTbl++ = pTSheet->GetSheetHeight();
    }
    qsort(pShtTbl, m_TShtTbl.GetSize(), sizeof(UINT) * 4, compsheets);

    WriteFileString(hFile,
        "     Tile Sheet Table\r\n"
        "   (sorted by dimension)\r\n"
        "+-------+-----+-----+-------+\r\n"
        "|  Sht  |  CX |  CY | ShtHt |\r\n"
        "+-------+-----+-----+-------+\r\n");

    pTbl = pShtTbl;
    for (UINT nSheet = 0; nSheet < (UINT)m_TShtTbl.GetSize(); nSheet++)
    {
        wsprintf(szBfr, "| %04X  | %3u | %3u | %5u |\r\n", pTbl[0], pTbl[1], pTbl[2], pTbl[3]);
        pTbl += 4;
        WriteFileString(hFile, szBfr);
    }
    WriteFileString(hFile,
        "+-------+-----+-----+-------+\r\n");

    WriteFileString(hFile, "\r\n\r\n\r\n");

    // Dump the tile table...

    WriteFileString(hFile,
        "                  TileID Table\r\n"
        "+-------+------------+------------+-------------+------+\r\n"
        "|  TID  | FULL  SIZE | HALF  SIZE | SMALL COLOR |  IN  |\r\n"
        "|       | Sht    Off | Sht    Off |     RGB     | TSET |\r\n"
        "+-------+------------+------------+-------------+------+\r\n");

    for (UINT tid = 0; tid < m_nTblSize; tid++)
    {
        TileDef* tp = &m_pTileTbl[tid];
        wsprintf(szBfr, "| %5u | %04X %5u | %04X %5u | 0x%08X  |", tid,
            (UINT)tp->m_tileFull.m_nSheet, tp->m_tileFull.m_nOffset,
            (UINT)tp->m_tileHalf.m_nSheet, tp->m_tileHalf.m_nOffset,
            tp->m_tileSmall);
        WriteFileString(hFile, szBfr);

        int nTSet = FindTileSetFromTileID((TileID)tid);
        if (nTSet >= 0)
            wsprintf(szBfr, " %3u  |\r\n", nTSet);
        else
            lstrcpy(szBfr, " ***  |\r\n");
        WriteFileString(hFile, szBfr);
    }
    WriteFileString(hFile,
        "+-------+------------+------------+-------------+------+\r\n");

    CloseHandle(hFile);
}

////////////////////////////////////////////////////////////////////////

BOOL CTileManager::DoBitFontDialog()
{
    FontID newFontID = DoFontDialog(m_fontID, GetApp()->m_pMainWnd, TRUE);
    if (newFontID != (FontID)0)
    {
        CGamDoc::GetFontManager()->DeleteFont(m_fontID);
        m_fontID = newFontID;
        return TRUE;
    }
    return FALSE;
}

void CTileManager::SetForeColor(COLORREF cr)
{
    m_crFore = cr;
    m_brFore.DeleteObject();
    COLORREF crTemp = MapWin9xRgbToNtRgb(m_crFore);
    m_brFore.CreateSolidBrush(crTemp);
}

void CTileManager::SetBackColor(COLORREF cr)
{
    m_crBack = cr;
    m_brBack.DeleteObject();
    COLORREF crTemp = MapWin9xRgbToNtRgb(m_crBack);
    m_brBack.CreateSolidBrush(crTemp);
}

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
        ar << m_nSheet;
        ASSERT(m_nOffset < (int)65535U);
        ar << (WORD)m_nOffset;
    }
    else
    {
        if (CGamDoc::GetLoadingVersion() < NumVersion(2, 91))   // V2.91
        {
            BYTE chTmp;
            ar >> chTmp;
            m_nSheet = (chTmp == 0xFF) ? TileLoc::noSheet : (WORD)chTmp;
        }
        else
            ar >> m_nSheet;
        WORD wTmp;
        ar >> wTmp; m_nOffset = (int)wTmp;
    }
}

