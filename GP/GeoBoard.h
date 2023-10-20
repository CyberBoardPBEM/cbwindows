// GeoBoard.h
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

#ifndef _GEOBOARD_H
#define _GEOBOARD_H

#include <array>

/////////////////////////////////////////////////////////////////////////////

class CBoard;
class CGamDoc;

/////////////////////////////////////////////////////////////////////////////

struct CGeoBoardElement
{
    BoardID m_nBoardSerialNum;
    Rotation90 m_rotation;

public:
    CGeoBoardElement(size_t row, size_t col,
                    BoardID nBoardSerialNum, Rotation90 r);
    CGeoBoardElement(const CGeoBoardElement&) = default;
    CGeoBoardElement& operator=(const CGeoBoardElement&) = delete;
    ~CGeoBoardElement() = default;

    // position within CGeoBoard
    /* we could find out the row,col by searching for this in
        the containing CGeomorphicBoard, but caching it isn't
        too difficult, and will be faster */
    size_t GetRow() const { return m_row; }
    size_t GetCol() const { return m_col; }

public:
    void Serialize(CArchive& ar);
    CB::string GetCellNumberStr(const CBoardManager& brdMgr,
                                size_t row, size_t col) const;

private:
    const size_t m_row;
    const size_t m_col;
};

/////////////////////////////////////////////////////////////////////////////

class CGeomorphicBoard : private std::vector< CGeoBoardElement >
{
public:
    CGeomorphicBoard(CGamDoc& pDoc);
    CGeomorphicBoard(const CGeomorphicBoard&) = delete;
    CGeomorphicBoard& operator=(const CGeomorphicBoard&) = delete;
    ~CGeomorphicBoard() = default;

// Atributes...
public:
    void SetName(CB::string pszName) { m_strName = std::move(pszName); }

    void SetSerialNumber(BoardID nBoardSerialNumber) { m_nSerialNum = nBoardSerialNumber; }
    BoardID GetSerialNumber() const { return m_nSerialNum; }

    void SetBoardRowCount(size_t nBoardRowCount) { m_nBoardRowCount = nBoardRowCount; }
    void SetBoardColCount(size_t nBoardColCount) { m_nBoardColCount = nBoardColCount; }

    size_t GetBoardRowCount() const { return m_nBoardRowCount; }
    size_t GetBoardColCount() const { return m_nBoardColCount; }

// Methods...
public:
    OwnerPtr<CBoard> CreateBoard();
    void DeleteFromBoardManager();

    size_t  AddElement(BoardID nBoardSerialNum, Rotation90 rot);

    bool NeedsGeoRotate() const;
    void Serialize(CArchive& ar);

    CB::string GetCellNumberStr(CPoint pnt, TileScale eScale) const;

// Implementation - methods
protected:
    const CBoard& GetBoard(size_t nBoardRow, size_t nBoardCol) const
    {
        return GetBoard(GetBoardElt(nBoardRow, nBoardCol));
    }
    const CBoard& GetBoard(const CGeoBoardElement& geo) const;
    const CGeoBoardElement& GetBoardElt(size_t nBoardRow, size_t nBoardCol) const;
    void    ComputeNewBoardDimensions(size_t& rnRows, size_t& rnCols) const;
    CPoint  ComputeGraphicalOffset(size_t nBoardRow, size_t nBoardCol) const;
    void    ComputeCellOffset(size_t nBoardRow, size_t nBoardCol, size_t& rnCellRow, size_t& rnCellCol) const;
    // non-const because new tiles get added to CTileManager
    void    CopyCells(CBoardArray& pBArryTo, const CGeoBoardElement& gbeFrom);
    OwnerPtr<CBitmap> CreateBitmap(CSize size) const;
    OwnerPtr<CBitmap> CombineLeftAndRight(TileScale eScale, const CBoardArray& pBALeft,
                const CBoardArray& pBARight, size_t nRowLeft, size_t nColLeft, size_t nRowRight, size_t nColRight) const;
    OwnerPtr<CBitmap> CombineTopAndBottom(TileScale eScale, const CBoardArray& pBATop,
                const CBoardArray& pBABottom, size_t nRowTop, size_t nColTop, size_t nRowBottom, size_t nColBottom) const;
    // non-const because new tiles get added to CTileManager
    size_t  GetSpecialTileSet();

    bool NeedsMerge(const CGeoBoardElement& gbe, Edge e) const;

// Implementation - vars
protected:
    CB::string m_strName;                 // The board's name
    BoardID  m_nSerialNum;              // The issued serial number

    size_t m_nBoardRowCount;          // Number of boards vertically
    size_t m_nBoardColCount;          // Number of boards horizontally

    RefPtr<CGamDoc> m_pDoc;                    // Used internally

private:
    // convert from overall row,col to GeoBoardElement row,col
    // remember some cells are merged, so have two identities
    struct BoardToSubBoard
    {
        size_t boardCoord = Invalid_v<size_t>;
        size_t subBoardCoord = Invalid_v<size_t>;

        BoardToSubBoard() = default;
        ~BoardToSubBoard() = default;

        bool operator==(const BoardToSubBoard& rhs) const;
        bool operator!=(const BoardToSubBoard& rhs) const
        {
            return !(*this == rhs);
        }
    };
    /* arr[x][0] is top or left half of hex,
        arr[x][1] is right or bottom half of hex */
    std::vector<std::array<BoardToSubBoard, 2>> m_rowToBoardRow;
    std::vector<std::array<BoardToSubBoard, 2>> m_colToBoardCol;
};

#endif
