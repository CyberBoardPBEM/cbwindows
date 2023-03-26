// BrdCell.h
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

#ifndef _BRDCELL_H
#define _BRDCELL_H

#ifndef     _TILE_H
#include    "tile.h"
#endif

#ifndef     _CELLFORM_H
#include    "cellform.h"
#endif

struct BoardCell
{
    bool m_tile;
    union
    {
        COLORREF    m_crCell;   // 0xFF000000 is noColor
        TileID      m_tidCell;  // Typically terrain bitmap
    };
    // -------- //
    BoardCell();
    BoardCell& operator=(const BoardCell&) = default;
    bool operator==(const BoardCell& rhs) const
    {
        return m_tile == rhs.m_tile &&
                (m_tile ?
                    m_tidCell == rhs.m_tidCell
                :
                    m_crCell == rhs.m_crCell);
    }
    bool operator!=(const BoardCell& rhs) const { return !(*this == rhs); }
    // -------- //
    BOOL IsTileID() const;
    BOOL IsEmpty() const;
    // -------- //
    TileID GetTID() const;
    COLORREF GetColor() const;
    // -------- //
    void Clear();
    void SetTID(TileID id);
    void SetColor(COLORREF cr);
    // ------- //
    void Serialize(CArchive& ar);
};

////////////////////////////////////////////////////////////////////

class CBoardArray
{
    friend class CGamDoc;
public:
    CBoardArray(CTileManager& tileMgr);
    CBoardArray(const CBoardArray&) = delete;
    CBoardArray& operator=(const CBoardArray&) = delete;
    virtual ~CBoardArray() = default;

// Attributes
public:
    COLORREF GetCellColor(size_t row, size_t col) const
    {
        return GetCell(row, col).m_crCell;
    }
    TileID GetCellTile(size_t row, size_t col) const
        { return GetCell(row, col).GetTID(); }
    BOOL IsTransparentCellTilesEnabled() const
        { return m_bTransparentCells; }
    void SetTransparentCellTilesEnabled(BOOL bEnabled = TRUE)
        { m_bTransparentCells = bEnabled; }
    // ------ //
    void SetCellNumTracking(BOOL bTrack = TRUE) { m_bTrackCellNum = bTrack; }
    void SetRowCellTrackingOffset(int nTrkOff) { m_nRowTrkOffset = nTrkOff; }
    void SetColCellTrackingOffset(int nTrkOff) { m_nColTrkOffset = nTrkOff; }
    void SetRowCellTrackingInvert(BOOL bTrkInv) { m_bRowTrkInvert = bTrkInv; }
    void SetColCellTrackingInvert(BOOL bTrkInv) { m_bColTrkInvert = bTrkInv; }
    void SetCellNumStyle(CellNumStyle eStyle) { m_eNumStyle = eStyle; }
    BOOL GetCellNumTracking() const { return m_bTrackCellNum; }
    int GetRowCellTrackingOffset() const { return m_nRowTrkOffset; }
    int GetColCellTrackingOffset() const { return m_nColTrkOffset; }
    BOOL GetRowCellTrackingInvert() const { return m_bRowTrkInvert; }
    BOOL GetColCellTrackingInvert() const { return m_bColTrkInvert; }
    CellNumStyle GetCellNumStyle() const { return m_eNumStyle; }
    // ------ //
    CB::string GetCellNumberStr(CPoint pnt, TileScale eScale) const;
    CB::string GetCellNumberStr(size_t row, size_t col) const;
    // ------ //
    COLORREF GetCellFrameColor() const { return m_crCellFrame; }
    void SetCellFrameColor(COLORREF cr);
    // ------ //
    const BoardCell& GetCell(size_t row, size_t col) const;
    BoardCell& GetCell(size_t row, size_t col)
    {
        return const_cast<BoardCell&>(std::as_const(*this).GetCell(row, col));
    }
    const CCellForm& GetCellForm(TileScale eScale) const;
    const CTileManager& GetTileManager() const { return *m_pTsa; }
    // ------ //
    size_t GetRows() const { return m_nRows; }
    size_t GetCols() const { return m_nCols; }
    int GetWidth(TileScale eScale) const;     // Board's pixel width
    int GetHeight(TileScale eScale) const;    // Board's pixel height
    CSize GetSize(TileScale eScale) const;    // Board's pixel size
    CSize GetCellSize(TileScale eScale) const;// Cell's pixel size

// Operations
public:
    void CreateBoard(CellFormType eType, size_t nRows, size_t nCols, int nParm1,
        int nParm2, CellStagger nStagger);
    void ReshapeBoard(size_t nRows, size_t nCols, int nParm1, int nParm2,
        CellStagger nStagger);
    void DestroyBoard();
    void GenerateBoard(CellFormType eType, size_t nRows, size_t nCols,
        int nParm1, int nParm2, CellStagger nStagger, std::vector<BoardCell>&& pMap);
    static void GenerateCellDefs(CellFormType eType, int nParm1, int nParm2,
        CellStagger nStagger, CCellForm& cfFull, CCellForm& cfHalf, CCellForm& cfSmall);
    // ------- //
    void DrawCells(CDC& pDC, const CRect& pCellRct, TileScale eScale) const;
    void DrawCellLines(CDC& pDC, const CRect& pCellRct, TileScale eScale) const;
    void FillCell(CDC& pDC, size_t row, size_t col, TileScale eScale) const;
    void FrameCell(CDC& pDC, size_t row, size_t col, TileScale eScale) const;
    // ------- //
    void SetCellTile(size_t row, size_t col, TileID tid);
    void SetCellColor(size_t row, size_t col, COLORREF cr);
    void SetCellColorInRange(size_t rowBeg, size_t colBeg, size_t rowEnd,
        size_t colEnd, COLORREF cr);
    // ------- //
    void GetBoardScaling(TileScale eScale, CSize& worldsize, CSize& viewSize) const;
    BOOL FindCell(long x, long y, size_t& rRow, size_t& rCol, TileScale eScale) const;
    CRect GetCellRect(size_t row, size_t col, TileScale eScale) const;
    BOOL MapPixelsToCellBounds(const CRect& pPxlRct, CRect& pCellRct,
        TileScale eScale) const;
    // ------- //
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid) const;
    // ------- //
    void Serialize(CArchive& ar);
// Implementation
protected:
    // Saved in file...
    WORD    m_wReserved1;       // For future need (set to 0)
    WORD    m_wReserved2;       // For future need (set to 0)
    WORD    m_wReserved3;       // For future need (set to 0)
    WORD    m_wReserved4;       // For future need (set to 0)
    int     m_nRowTrkOffset;    // Row track num offset
    int     m_nColTrkOffset;    // Col track num offset
    int     m_bRowTrkInvert;    // Row track num invert direction
    int     m_bColTrkInvert;    // Col track num invert direction
    size_t  m_nRows;            // Number of cell rows in board
    size_t  m_nCols;            // Number of cell columns in board
    BOOL    m_bTransparentCells;// (Not currently used)
    BOOL    m_bTrackCellNum;    // Mouse traking of cell number
    CellNumStyle m_eNumStyle;   // Cell numbering style
    COLORREF m_crCellFrame;     // Cell frame color

    CCellForm   m_cfFull;       // Defines cell geometry of map (full scale)
    CCellForm   m_cfHalf;       // Defines cell geometry of map (half scale)
    CCellForm   m_cfSmall;      // Defines cell geometry of map (small scale)

    std::vector<BoardCell> m_pMap;
    // -------- //
    CPen    m_pnCellFrame;      // Cell frame color pen.
    // -------- //
    RefPtr<CTileManager> m_pTsa;
    // -------- //
    size_t CellIndex(size_t row, size_t col) const { return row*m_nCols + col; }
};

////////////////////////////////////////////////////////////////////

#ifndef _DEBUG
inline  BoardCell::BoardCell() { Clear(); }
inline  void BoardCell::Clear() { m_tile = false; m_crCell = noColor; }
inline  BOOL BoardCell::IsTileID() const
    { return m_tile && m_tidCell != nullTid; }
inline  BOOL BoardCell::IsEmpty() const { return !m_tile && m_crCell == noColor; }
inline  TileID BoardCell::GetTID() const
    { return m_tile ? m_tidCell : nullTid; }
inline  void BoardCell::SetTID(TileID id)
    { m_tile = true; m_tidCell = id; }
inline  COLORREF BoardCell::GetColor() const { ASSERT(!m_tile); return !m_tile ? m_crCell : noColor; }
inline  void BoardCell::SetColor(COLORREF cr) { m_tile = false; m_crCell = cr; }
#endif

#endif

