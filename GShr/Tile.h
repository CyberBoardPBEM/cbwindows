// Tile.h
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

#ifndef _TILE_H
#define _TILE_H

#ifndef     _GDITOOLS_H
#include    "GdiTools.h"
#endif

////////////////////////////////////////////////////////////////////

class   CTile;

////////////////////////////////////////////////////////////////////

typedef     unsigned short TileID;

const       TileID nullTid = 0xFFFF;

const       UINT maxSheetHeight = 8192;     // Max y pixels allowed in a tile sheet


// Note that the values from this enum are used as qualifying
// visiblilty masks. They are also stored in files so messing with them
// will break existing files. BE CAREFULL!

enum TileScale { fullScale = 1, halfScale = 2, smallScale = 4 };

const int AllTileScales = fullScale | halfScale | smallScale;

////////////////////////////////////////////////////////////////////

struct TileLoc
{
    WORD    m_nSheet;               // (2.91 BYTE->WORD)
    int     m_nOffset;
    // ------ //
    enum { noSheet = 0xFFFF };

    BOOL IsEmpty() const { return m_nSheet == noSheet; }
    void SetEmpty() { m_nSheet = noSheet; m_nOffset = 0;  } // (clear offset too)

    void Serialize(CArchive& archive);
};

struct TileDef
{
    TileLoc     m_tileFull;         // Full scale tile graphic
    TileLoc     m_tileHalf;         // Half scale tile graphic
    COLORREF    m_tileSmall;        // Color for reduced scale maps
    // ------ //
    BOOL IsEmpty() const { return m_tileFull.IsEmpty(); }
    void SetEmpty() { m_tileFull.SetEmpty(); }

    void Serialize(CArchive& archive);
};

////////////////////////////////////////////////////////////////////

class CTileSheet
{
    friend class CGamDoc;
    friend class CTile;
public:
    CTileSheet();
    CTileSheet(CSize size);
    ~CTileSheet();
// Attributes
public:
    int GetWidth() { return m_size.cx; }
    int GetHeight() { return m_size.cy; }
    CSize GetSize() { return CSize(m_size.cx, m_size.cy); }
    void SetSize(CSize size);

    BOOL IsSameDimensions(CSize size) const { return size == m_size; }
    int GetSheetHeight() const { return m_sheetHt; }

// Operations
public:
    void CreateTile();
    void DeleteTile(int yOffset);
    void UpdateTile(CBitmap *pBMap, int yLoc);
    void CreateBitmapOfTile(CBitmap *pBMap, int yLoc);
    // ---------- //
    void Serialize(CArchive& archive);

// Friendly Access...
protected:
    void TileBlt(CDC *pDC, int xDst, int yDst, int ySrc, DWORD dwRop);
    void StretchBlt(CDC *pDC, int xDst, int yDst, int xWid, int yWid,
        int ySrc, DWORD dwRop);
    void TransBlt(CDC *pDC, int xDst, int yDst, int ySrc, COLORREF crTrans);
    void TransBltThruDIBSectMonoMask(CDC *pDC, int xDst, int yDst, int ySrc,
        COLORREF crTrans, BITMAP* pMaskBMapInfo);

// Implementation - vars...
protected:
    CBitmap*    m_pBMap;        // Pointer to DDB
    LPBYTE      m_pMem;         // Ptr to DIB Memory (DIBSection)

    CSize       m_size;         // Tile sizes for this sheet
    int         m_sheetHt;      // Total height of sheet

    CDC*        m_pDC;

// Implementation - methods...
protected:
    void CreateSheetDC();
    void DeleteSheetDC();
    void ClearSheet();
};

////////////////////////////////////////////////////////////////////

class CTileSet
{
    friend class CGamDoc;
// Attributes
public:
    CWordArray* GetTileIDTable() { return &m_tidTbl; }
    BOOL HasTileID(TileID tid);

    const char* GetName() const { return m_strName; }
    void SetName(const char *pszName) { m_strName = pszName; }

// Operations
public:
    void AddTileID(TileID tid, int nPos = -1);
    void RemoveTileID(TileID tid);
    int  FindTileID(TileID tid);

    void Serialize(CArchive& archive);

// Implementation
protected:
    CString     m_strName;
    CWordArray  m_tidTbl;       // TileIDs in this set.
};

//////////////////////////////////////////////////////////////////////

class CTileManager
{
    friend class CGamDoc;
public:
    CTileManager();
    ~CTileManager();

// Attributes
public:
    // Tile Mangager attributes
    void SetTransparentColor(COLORREF crTrans) { m_crTrans = crTrans; }
    COLORREF GetTransparentColor() const { return m_crTrans; }

    // Tile Set attributes
    int GetNumTileSets() const { return m_TSetTbl.GetSize(); }
    CTileSet* GetTileSet(UINT nTSet)
    {
        if (m_TSetTbl.GetSize() == 0)
            return NULL;
        else
            return (CTileSet*)m_TSetTbl.GetAt(nTSet);
    }

    // Access routines for all Tile Editor info...
    void SetForeColor(COLORREF cr);
    void SetBackColor(COLORREF cr);
    void SetLineWidth(int nWidth) { m_nLineWidth = nWidth; }
    BOOL DoBitFontDialog();
    COLORREF GetForeColor() { return m_crFore; }
    COLORREF GetBackColor() { return m_crBack; }
    int GetLineWidth() { return m_nLineWidth; }
    CBrush* GetForeBrush() { return &m_brFore; }
    CBrush* GetBackBrush() { return &m_brBack; }
    FontID GetFontID() { return m_fontID; }

// Operations
public:
    // Tile Ops.
    void GetTile(TileID tid, CTile* pTile, TileScale eScale = fullScale);
    TileID CreateTile(int nTSet, CSize sFull, CSize sHalf,
        COLORREF crSmall, int nPos = -1);
    void DeleteTile(TileID tid, BOOL bFromSetAlso = TRUE);
    void SetSmallTileColor(TileID tid, COLORREF cr);
    BOOL IsTileIDValid(TileID tid);
    int FindTileSetFromTileID(TileID tid);
    void MoveTileIDsToTileSet(int nTSet, CWordArray& tidList, int nPos = -1);

    // Nulls aren't updated...
    void UpdateTile(TileID tid, CBitmap* bmFull, CBitmap* bmHalf,
        COLORREF crSmall);

    // Tile Set Ops.
    int CreateTileSet(const char* pszName);
    int FindNamedTileSet(const char* pszName);
    void DeleteTileSet(int nSet);

    // ---------- //
    void CopyTileImagesToArchive(CArchive& ar, CWordArray& tidsList);
    void CreateTilesFromTileImageArchive(CArchive& ar, int nTSet,
            CWordArray* pTidTbl  = NULL, int nPos = -1);
    // ---------- //
    void Serialize(CArchive& archive);
    void SerializeTileSets(CArchive& ar);
    void SerializeTileSheets(CArchive& ar);

    // TOOL CODE //
    BOOL PruneTilesOnSheet255();
    void DumpTileDatabaseInfoToFile(LPCTSTR pszFileName, BOOL bNewFile = TRUE);
    // TOOL CODE //

// Implementation
protected:
    TileDef *   m_pTileTbl;         // Global def'ed
    UINT        m_nTblSize;         // Number of alloc'ed ents in tile table
    COLORREF    m_crTrans;          // Transparency color for all tiles
    WORD        m_wReserved1;       // For future need (set to 0)
    WORD        m_wReserved2;       // For future need (set to 0)
    WORD        m_wReserved3;       // For future need (set to 0)
    WORD        m_wReserved4;       // For future need (set to 0)
    // These are shared for all Tile Editors...
    COLORREF    m_crFore;           // Current foreground color
    CBrush      m_brFore;           // Foreground solid brush
    COLORREF    m_crBack;           // Current background color
    CBrush      m_brBack;           // Background solid brush
    UINT        m_nLineWidth;       // Current line width
    FontID      m_fontID;           // Current font
    // ------- //
    CPtrArray   m_TSetTbl;          // Table of tile set pointers
    CPtrArray   m_TShtTbl;          // Table of tile sheet pointers
    // ------- //
    void Clear();
    TileID CreateTileIDEntry();
    void ResizeTileTable(UINT nEntsNeeded);
    void CreateTileOnSheet(CSize size, TileLoc* pLoc);
    void DeleteTileFromSheet(TileLoc *pLoc);
    void AdjustTileLoc(TileLoc *pLoc, int nSht, int yLoc, int cy);
    int GetSheetForTile(CSize size);
    void RemoveTileIDFromTileSets(TileID tid);
    CTileSheet* GetTileSheet(int nSheet)
        { return (CTileSheet*)m_TShtTbl.GetAt(nSheet); }
};

////////////////////////////////////////////////////////////////////
// CTiles are the proxy objects used to manipulate single tiles.

class CTile
{
    friend class CTileManager;
// Attributes
public:
    int GetWidth() const;
    int GetHeight() const;
    CSize GetSize() const;
    void SetNoTransparent() { m_crTrans = noColor; }
    COLORREF GetTransparent() const { return m_crTrans; }
    COLORREF GetSmallColor() const { return m_crSmall; }

// Operations
public:
    void BitBlt(CDC *pDC, int x, int y, DWORD dwRop = SRCCOPY);
    void StretchBlt(CDC *pDC, int x, int y, int cx, int cy, DWORD dwRop = SRCCOPY);
    void TransBlt(CDC *pDC, int x, int y, BITMAP* pMaskBMapInfo = NULL);
    void Update(CBitmap *pBMap);
    void CreateBitmapOfTile(CBitmap *pBMap);

// Implementation
protected:
    CTileSheet* m_pTS;      // For Reference Only (DON'T DELETE!!!)
    int         m_yLoc;     // Y location in sheet
    CSize       m_size;     // size of bitmap of color patch
    COLORREF    m_crTrans;  // Transparency color in bitmaps
    COLORREF    m_crSmall;  // For smallScale (m_pTS == NULL)
};

#endif

