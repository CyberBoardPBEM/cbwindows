// Tile.h
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

#ifndef _TILE_H
#define _TILE_H

#ifndef     _GDITOOLS_H
#include    "GdiTools.h"
#endif

#ifndef     _GMISC_H
#include    "GMisc.h"
#endif

////////////////////////////////////////////////////////////////////

class   CTile;
class   CTileUpdatable;

////////////////////////////////////////////////////////////////////

typedef XxxxID16<'T'> TileID16;
typedef XxxxID32<'T'> TileID32;
typedef XxxxID<'T'> TileID;

template<>
struct Invalid<TileID16>
{
    static constexpr TileID16 value = TileID16(0xFFFF);
};

template<>
struct Invalid<TileID32>
{
    static constexpr TileID32 value = TileID32(0xFFFFFFFF);
};

constexpr TileID nullTid = Invalid_v<TileID>;

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
    size_t  m_nSheet;               // (2.91 BYTE->WORD), (4.0 WORD->size_t)
    int     m_nOffset;
    // ------ //
    constexpr static size_t noSheet = Invalid_v<size_t>;
    constexpr static uint16_t noSheet16 = uint16_t(0xFFFF);

    bool IsEmpty() const { return m_nSheet == noSheet; }
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
    void SetEmpty() { m_tileFull.SetEmpty(); m_tileHalf.SetEmpty(); m_tileSmall = 0; }

    void Serialize(CArchive& archive);
};

////////////////////////////////////////////////////////////////////

class CTileSheet
{
    friend class CGamDoc;
    friend class CTile;
public:
    CTileSheet();
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
    void UpdateTile(const CBitmap& pBMap, int yLoc);
    OwnerPtr<CBitmap> CreateBitmapOfTile(int yLoc) const;
    // ---------- //
    void Serialize(CArchive& archive);

// Friendly Access...
protected:
    void TileBlt(CDC& pDC, int xDst, int yDst, int ySrc, DWORD dwRop) const;
    void StretchBlt(CDC& pDC, int xDst, int yDst, int xWid, int yWid,
        int ySrc, DWORD dwRop) const;
    void TransBlt(CDC& pDC, int xDst, int yDst, int ySrc, COLORREF crTrans) const;
    void TransBltThruDIBSectMonoMask(CDC& pDC, int xDst, int yDst, int ySrc,
        COLORREF crTrans, const BITMAP& pMaskBMapInfo) const;

// Implementation - vars...
protected:
    OwnerOrNullPtr<CBitmap> m_pBMap;        // Pointer to DDB

    CSize       m_size;         // Tile sizes for this sheet
    int         m_sheetHt;      // Total height of sheet

// Implementation - methods...
protected:
    class SheetDC
    {
    public:
        SheetDC(const CTileSheet& sheet);
        ~SheetDC();

        operator CDC*() const;
    };

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

    const CB::string& GetName() const { return m_strName; }
    void SetName(CB::string pszName) { m_strName = std::move(pszName); }

// Operations
public:
    void AddTileID(TileID tid, size_t nPos = Invalid_v<size_t>);
    void RemoveTileID(TileID tid);
    size_t FindTileID(TileID tid) const;

    void Serialize(CArchive& archive);

// Implementation
protected:
    CB::string m_strName;
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
    COLORREF GetForeColor() const { return m_crFore; }
    COLORREF GetBackColor() const { return m_crBack; }
    int GetLineWidth() const { return m_nLineWidth; }
    const CBrush& GetForeBrush() const { return m_brFore; }
    const CBrush& GetBackBrush() const { return m_brBack; }
    FontID GetFontID() const { return m_fontID; }

// Operations
public:
    // Tile Ops.
    CTile GetTile(TileID tid, TileScale eScale = fullScale) const;
    CTileUpdatable GetTile(TileID tid, TileScale eScale = fullScale);
    TileID CreateTile(size_t nTSet, CSize sFull, CSize sHalf,
        COLORREF crSmall, size_t nPos = Invalid_v<size_t>);
    void DeleteTile(TileID tid, BOOL bFromSetAlso = TRUE);
    void SetSmallTileColor(TileID tid, COLORREF cr);
    BOOL IsTileIDValid(TileID tid) const;
    size_t FindTileSetFromTileID(TileID tid) const;
    void MoveTileIDsToTileSet(size_t nTSet, const std::vector<TileID>& tidList, size_t nPos = Invalid_v<size_t>);

    // Nulls aren't updated...
    void UpdateTile(TileID tid, const CBitmap& bmFull, const CBitmap& bmHalf,
        COLORREF crSmall);

    // Tile Set Ops.
    size_t CreateTileSet(CB::string pszName);
    size_t FindNamedTileSet(const CB::string& pszName) const;
    void DeleteTileSet(size_t nTSet);

    // ---------- //
    void CopyTileImagesToArchive(CArchive& ar, const std::vector<TileID>& tidsList);
    void CreateTilesFromTileImageArchive(CArchive& ar, size_t nTSet,
            std::vector<TileID>* pTidTbl  = NULL, size_t nPos = Invalid_v<size_t>);
    // ---------- //
    bool Needs32BitIDs() const
    {
        return m_pTileTbl.GetSize() >= size_t(0xFFFF);
    }
    void Serialize(CArchive& archive);
    void SerializeTileSets(CArchive& ar);
    void SerializeTileSheets(CArchive& ar);

    // TOOL CODE //
    BOOL PruneTilesOnSheet255();
    void DumpTileDatabaseInfoToFile(const CB::string& pszFileName, BOOL bNewFile = TRUE) const;
    // TOOL CODE //

// Implementation
protected:
    XxxxIDTable<TileID, TileDef,
                tileTblBaseSize, tileTblIncrSize,
                false,
                CalcAllocSize> m_pTileTbl;
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
    /* need to store pointers, not objects, here because
        CTile has pointer to CTileSheet, and if m_TShtTbl
        holds objects, those pointers become invalid when
        m_TShtTbl increases capacity */
    std::vector<OwnerPtr<CTileSheet>> m_TShtTbl;
    // ------- //
    void Clear();
    void CreateTileOnSheet(CSize size, TileLoc& pLoc);
    void DeleteTileFromSheet(const TileLoc& pLoc);
    void AdjustTileLoc(TileLoc& pLoc, size_t nSht, int yLoc, int cy);
    size_t GetSheetForTile(CSize size);
    void RemoveTileIDFromTileSets(TileID tid);
    const CTileSheet& GetTileSheet(size_t nSheet) const
        { return *m_TShtTbl.at(nSheet); }
    CTileSheet& GetTileSheet(size_t nSheet)
        { return const_cast<CTileSheet&>(std::as_const(*this).GetTileSheet(nSheet)); }

#ifdef GPLAY
    // geomorphic board support
    /* rotated geomorphic boards get new tiles,
        but reuse replacements when possible */
public:
    size_t GetSpecialTileSet();
    TileID Get(TileID tid, Rotation90 rot);

private:
    typedef std::map<std::pair<TileID, Rotation90>, TileID> GeoTiles;
    GeoTiles geoTiles;
#endif
};

////////////////////////////////////////////////////////////////////
// CTiles are the proxy objects used to manipulate single tiles.
// CTile is like const_iterator and CTileUpdatable like iterator

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
    void BitBlt(CDC& pDC, int x, int y, DWORD dwRop = SRCCOPY) const;
    void StretchBlt(CDC& pDC, int x, int y, int cx, int cy, DWORD dwRop = SRCCOPY) const;
    void TransBlt(CDC& pDC, int x, int y, const BITMAP* pMaskBMapInfo = NULL) const;
    OwnerPtr<CBitmap> CreateBitmapOfTile() const;

// Implementation
protected:
    const CTileSheet* m_pTS;      // For Reference Only (DON'T DELETE!!!)
    int         m_yLoc;     // Y location in sheet
    CSize       m_size;     // size of bitmap of color patch
    COLORREF    m_crTrans;  // Transparency color in bitmaps
    COLORREF    m_crSmall;  // For smallScale (m_pTS == NULL)

private:
    friend CTile CTileManager::GetTile(TileID tid, TileScale eScale /*= fullScale*/) const;
    CTile() = default;
};

class CTileUpdatable : public CTile
{
public:
    void Update(const CBitmap& pBMap);
};

inline CTileUpdatable CTileManager::GetTile(TileID tid, TileScale eScale /*= fullScale*/)
{
    CTile temp = std::as_const(*this).GetTile(tid, eScale);
    return reinterpret_cast<CTileUpdatable&>(temp);
}

#endif

