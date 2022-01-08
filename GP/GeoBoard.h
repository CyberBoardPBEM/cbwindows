// GeoBoard.h
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

#ifndef _GEOBOARD_H
#define _GEOBOARD_H

/////////////////////////////////////////////////////////////////////////////

class CBoard;
class CGamDoc;

/////////////////////////////////////////////////////////////////////////////

struct CGeoBoardElement
{
    BoardID m_nBoardSerialNum;
    uint8_t m_nReserved = uint8_t(0);    // for board rotation

public:
    CGeoBoardElement();
    CGeoBoardElement(CGeoBoardElement& geo);
    CGeoBoardElement(BoardID nBoardSerialNum);

public:
    void Serialize(CArchive& ar);
};

/////////////////////////////////////////////////////////////////////////////

class CGeomorphicBoard : public CArray< CGeoBoardElement >
{
public:
    CGeomorphicBoard();
    CGeomorphicBoard(const CGeomorphicBoard& pGeoBoard);

// Atributes...
public:
    void SetName(LPCTSTR pszName) { m_strName = pszName; }

    void SetSerialNumber(BoardID nBoardSerialNumber) { m_nSerialNum = nBoardSerialNumber; }
    BoardID GetSerialNumber() const { return m_nSerialNum; }

    void SetBoardRowCount(size_t nBoardRowCount) { m_nBoardRowCount = nBoardRowCount; }
    void SetBoardColCount(size_t nBoardColCount) { m_nBoardColCount = nBoardColCount; }

    size_t GetBoardRowCount() { return m_nBoardRowCount; }
    size_t GetBoardColCount() { return m_nBoardColCount; }

// Methods...
public:
    CBoard* CreateBoard(CGamDoc* pDoc);
    void DeleteFromBoardManager();

    intptr_t  AddElement(BoardID nBoardSerialNum);

    void Serialize(CArchive& ar);

// Implementation - methods
protected:
    CBoard& GetBoard(size_t nBoardRow, size_t nBoardCol);
    CBoard* CloneBoard(CBoard& pOrigBoard);
    void    ComputeNewBoardDimensions(size_t& rnRows, size_t& rnCols);
    CPoint  ComputeGraphicalOffset(size_t nBoardRow, size_t nBoardCol);
    void    ComputeCellOffset(size_t nBoardRow, size_t nBoardCol, size_t& rnCellRow, size_t& rnCellCol);
    void    CopyCells(CBoardArray* pBArryTo, CBoardArray* pBArryFrom,
                size_t nCellRowOffset, size_t nCellColOffset);
    void    CreateBitmap(CBitmap& m_bmap, CSize size);
    void    CombineLeftAndRight(CBitmap& bmap, TileScale eScale, CBoardArray* pBALeft,
                CBoardArray* pBARight, size_t nRowLeft, size_t nColLeft, size_t nRowRight, size_t nColRight);
    void    CombineTopAndBottom(CBitmap& bmap, TileScale eScale, CBoardArray* pBATop,
                CBoardArray* pBABottom, size_t nRowTop, size_t nColTop, size_t nRowBottom, size_t nColBottom);
    size_t  GetSpecialTileSet();

// Implementation - vars
protected:
    CString  m_strName;                 // The board's name
    BoardID  m_nSerialNum;              // The issued serial number

    size_t m_nBoardRowCount;          // Number of boards vertically
    size_t m_nBoardColCount;          // Number of boards horizontally

    CGamDoc* m_pDoc;                    // Used internally
};

#endif
