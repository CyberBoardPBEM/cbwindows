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
    union
    {
        COLORREF    m_crCell;   // 0xFFFFxxxx is TileID, 0xFF000000 is noColor
        struct TID
        {
            TileID  m_tidCell;  // Typically terrain bitmap
            WORD    m_key;
        } tid;
    };
    // -------- //
    BoardCell();
    // -------- //
    BOOL IsTileID();
    BOOL IsEmpty();
    // -------- //
    TileID GetTID();
    COLORREF GetColor();
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
    CBoardArray();
    virtual ~CBoardArray() { DestroyBoard(); }

// Attributes
public:
    COLORREF GetCellColor(int row, int col)
        { return GetCell(row, col)->m_crCell; }
    TileID GetCellTile(int row, int col)
        { return GetCell(row, col)->GetTID(); }
    BOOL IsTransparentCellTilesEnabled()
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
    BOOL GetCellNumTracking() { return m_bTrackCellNum; }
    int GetRowCellTrackingOffset() { return m_nRowTrkOffset; }
    int GetColCellTrackingOffset() { return m_nColTrkOffset; }
    BOOL GetRowCellTrackingInvert() { return m_bRowTrkInvert; }
    BOOL GetColCellTrackingInvert() { return m_bColTrkInvert; }
    CellNumStyle GetCellNumStyle() { return m_eNumStyle; }
    // ------ //
    void GetCellNumberStr(CPoint pnt, CString& str, TileScale eScale);
    void GetCellNumberStr(int row, int col, CString& str);
    // ------ //
    COLORREF GetCellFrameColor() { return m_crCellFrame; }
    void SetCellFrameColor(COLORREF cr);
    // ------ //
    BoardCell* GetCell(int row, int col);
    CCellForm* GetCellForm(TileScale eScale);
    void SetTileManager(CTileManager *pTsa) { m_pTsa = pTsa; }
    CTileManager *GetTileManager() { return m_pTsa; }
    // ------ //
    int GetRows() { return m_nRows; }
    int GetCols() { return m_nCols; }
    int GetWidth(TileScale eScale);     // Board's pixel width
    int GetHeight(TileScale eScale);    // Board's pixel height
    CSize GetSize(TileScale eScale);    // Board's pixel size
    CSize GetCellSize(TileScale eScale);// Cell's pixel size

// Operations
public:
    void CreateBoard(CellFormType eType, int nRows, int nCols, int nParm1,
        int nParm2, int nStagger);
    void ReshapeBoard(int nRows, int nCols, int nParm1, int nParm2,
        int nStagger);
    void DestroyBoard();
    void GenerateBoard(CellFormType eType, int nRows, int nCols,
        int nParm1, int nParm2, int nStagger, BoardCell* pMap);
    static void GenerateCellDefs(CellFormType eType, int nParm1, int nParm2,
        int nStagger, CCellForm& cfFull, CCellForm& cfHalf, CCellForm& cfSmall);
    // ------- //
    void DrawCells(CDC* pDC, CRect* pCellRct, TileScale eScale);
    void DrawCellLines(CDC* pDC, CRect* pCellRct, TileScale eScale);
    void FillCell(CDC *pDC, int row, int col, TileScale eScale);
    void FrameCell(CDC *pDC, int row, int col, TileScale eScale);
    // ------- //
    void SetCellTile(int row, int col, TileID tid);
    void SetCellColor(int row, int col, COLORREF cr);
    void SetCellColorInRange(int rowBeg, int colBeg, int rowEnd,
        int colEnd, COLORREF cr);
    // ------- //
    void GetBoardScaling(TileScale eScale, CSize& worldsize, CSize& viewSize);
    BOOL FindCell(int x, int y, int& rRow, int& rCol, TileScale eScale);
    void GetCellRect(int row, int col, CRect* pRct, TileScale eScale);
    BOOL MapPixelsToCellBounds(CRect* pPxlRct, CRect* pCellRct,
        TileScale eScale);
    // ------- //
    BOOL PurgeMissingTileIDs();
    BOOL IsTileInUse(TileID tid);
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
    int     m_nRows;            // Number of cell rows in board
    int     m_nCols;            // Number of cell columns in board
    BOOL    m_bTransparentCells;// (Not currently used)
    BOOL    m_bTrackCellNum;    // Mouse traking of cell number
    CellNumStyle m_eNumStyle;   // Cell numbering style
    COLORREF m_crCellFrame;     // Cell frame color

    CCellForm   m_cfFull;       // Defines cell geometry of map (full scale)
    CCellForm   m_cfHalf;       // Defines cell geometry of map (half scale)
    CCellForm   m_cfSmall;      // Defines cell geometry of map (small scale)

    BoardCell* m_pMap;          // ** GlobalAlloc'ed
    // -------- //
    CPen    m_pnCellFrame;      // Cell frame color pen.
    // -------- //
    CTileManager* m_pTsa;       // For reference only! Don't destroy!
    // -------- //
    int CellIndex(int row, int col) { return row*m_nCols + col; }
};

////////////////////////////////////////////////////////////////////

#ifndef _DEBUG
inline  BoardCell::BoardCell() { Clear(); }
inline  void BoardCell::Clear() { m_crCell = noColor; }
inline  BOOL BoardCell::IsTileID()
    { return tid.m_key == 0xFFFF && tid.m_tidCell != nullTid; }
inline  BOOL BoardCell::IsEmpty()  { return m_crCell == noColor; }
inline  TileID BoardCell::GetTID()
    { return tid.m_key == 0xFFFF ? tid.m_tidCell : nullTid; }
inline  void BoardCell::SetTID(TileID id)
    { tid.m_tidCell = id; tid.m_key = 0xFFFF; }
inline  COLORREF BoardCell::GetColor() { return m_crCell; }
inline  void BoardCell::SetColor(COLORREF cr) { m_crCell = cr; }
#endif

#endif

