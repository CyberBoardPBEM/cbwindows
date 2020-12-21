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

const size_t maxTiles = 32000;
typedef XxxxID<'T'> TileID;

const       TileID nullTid = TileID(0xFFFF);

const       UINT maxSheetHeight = 8192;     // Max y pixels allowed in a tile sheet


// Note that the values from this enum are used as qualifying
// visiblilty masks. They are also stored in files so messing with them
// will break existing files. BE CAREFULL!

enum TileScale { fullScale = 1, halfScale = 2, smallScale = 4 };

const int AllTileScales = fullScale | halfScale | smallScale;

///////////////////////////////////////////////////////////////////////

const size_t tileTblBaseSize = 32;            // TileDef table allocation strategy
const size_t tileTblIncrSize = 8;

////////////////////////////////////////////////////////////////////

struct TileLoc
{
    /* TODO:  m_nSheet should be size_t, but that breaks file
        format compatibility, so change this on next new format */
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
    CTileSheet(const CTileSheet&) = delete;
    CTileSheet& operator=(const CTileSheet&) = delete;
    CTileSheet(CTileSheet&&) noexcept = default;
    CTileSheet& operator=(CTileSheet&&) noexcept = default;
    ~CTileSheet() = default;
// Attributes
public:
    int GetWidth() const { return m_size.cx; }
    int GetHeight() const { return m_size.cy; }
    CSize GetSize() const { return CSize(m_size.cx, m_size.cy); }
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
    std::unique_ptr<CBitmap> m_pBMap;        // Pointer to DDB
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
    CTileSet() = default;
    CTileSet(const CTileSet&) = delete;
    CTileSet& operator=(const CTileSet&) = delete;
    CTileSet(CTileSet&&) noexcept = default;
    CTileSet& operator=(CTileSet&&) noexcept = default;
    ~CTileSet() = default;

    const std::vector<TileID>& GetTileIDTable() const { return m_tidTbl; }
    BOOL HasTileID(TileID tid) const;

    const char* GetName() const { return m_strName.c_str(); }
    void SetName(const char *pszName) { m_strName = pszName; }

// Operations
public:
    void AddTileID(TileID tid, size_t nPos = Invalid_v<size_t>);
    void RemoveTileID(TileID tid);
    size_t FindTileID(TileID tid) const;

    void Serialize(CArchive& archive);

// Implementation
protected:
    std::string m_strName;
    std::vector<TileID> m_tidTbl;
};

//////////////////////////////////////////////////////////////////////

class CTileManager
{
    friend class CGamDoc;
public:
    CTileManager();
    CTileManager(const CTileManager&) = delete;
    CTileManager& operator=(const CTileManager&) = delete;
    ~CTileManager() = default;

// Attributes
public:
    // Tile Mangager attributes
    void SetTransparentColor(COLORREF crTrans) { m_crTrans = crTrans; }
    COLORREF GetTransparentColor() const { return m_crTrans; }

    // Tile Set attributes
    size_t GetNumTileSets() const { return m_TSetTbl.size(); }
    bool IsEmpty() const { return m_TSetTbl.empty(); }
    const CTileSet& GetTileSet(size_t nTSet) const
    {
        ASSERT(nTSet < m_TSetTbl.size());
        return m_TSetTbl.at(nTSet);
    }
    CTileSet& GetTileSet(size_t nTSet)
    {
        return const_cast<CTileSet&>(std::as_const(*this).GetTileSet(nTSet));
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
    TileID CreateTile(size_t nTSet, CSize sFull, CSize sHalf,
        COLORREF crSmall, size_t nPos = Invalid_v<size_t>);
    void DeleteTile(TileID tid, BOOL bFromSetAlso = TRUE);
    void SetSmallTileColor(TileID tid, COLORREF cr);
    BOOL IsTileIDValid(TileID tid);
    size_t FindTileSetFromTileID(TileID tid) const;
    void MoveTileIDsToTileSet(size_t nTSet, const std::vector<TileID>& tidList, size_t nPos = Invalid_v<size_t>);

    // Nulls aren't updated...
    void UpdateTile(TileID tid, CBitmap* bmFull, CBitmap* bmHalf,
        COLORREF crSmall);

    // Tile Set Ops.
    size_t CreateTileSet(const char* pszName);
    size_t FindNamedTileSet(const char* pszName) const;
    void DeleteTileSet(size_t nTSet);

    // ---------- //
    void CopyTileImagesToArchive(CArchive& ar, const std::vector<TileID>& tidsList);
    void CreateTilesFromTileImageArchive(CArchive& ar, size_t nTSet,
            std::vector<TileID>* pTidTbl  = NULL, size_t nPos = Invalid_v<size_t>);
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
    XxxxIDTable<TileID, TileDef,
                maxTiles, tileTblBaseSize, tileTblIncrSize,
                false> m_pTileTbl;
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
    std::vector<CTileSet> m_TSetTbl;
    std::vector<CTileSheet> m_TShtTbl;
    // ------- //
    void Clear();
    void CreateTileOnSheet(CSize size, TileLoc& pLoc);
    void DeleteTileFromSheet(TileLoc& pLoc);
    void AdjustTileLoc(TileLoc& pLoc, size_t nSht, int yLoc, int cy);
    size_t GetSheetForTile(CSize size);
    void RemoveTileIDFromTileSets(TileID tid);
    CTileSheet& GetTileSheet(size_t nSheet)
        { return m_TShtTbl.at(nSheet); }
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

