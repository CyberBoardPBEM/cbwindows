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

// The starting serial number for geomorpically created boards.
const int GEO_BOARD_SERNUM_BASE = 1000;

/////////////////////////////////////////////////////////////////////////////

struct CGeoBoardElement
{
    int m_nBoardSerialNum;

public:
    CGeoBoardElement();
    CGeoBoardElement(CGeoBoardElement& geo);
    CGeoBoardElement(int nBoardSerialNum);

public:
    void Serialize(CArchive& ar);
};

/////////////////////////////////////////////////////////////////////////////

class CGeomorphicBoard : public CArray< CGeoBoardElement, CGeoBoardElement& >
{
public:
    CGeomorphicBoard();
    CGeomorphicBoard(CGeomorphicBoard* pGeoBoard);

// Atributes...
public:
    void SetName(LPCTSTR pszName) { m_strName = pszName; }

    void SetSerialNumber(int nBoardSerialNumber) { m_nSerialNum = nBoardSerialNumber; }
    int  GetSerialNumber() { return m_nSerialNum; }

    void SetBoardRowCount(int nBoardRowCount) { m_nBoardRowCount = nBoardRowCount; }
    void SetBoardColCount(int nBoardColCount) { m_nBoardColCount = nBoardColCount; }

    int  GetBoardRowCount() { return m_nBoardRowCount; }
    int  GetBoardColCount() { return m_nBoardColCount; }

// Methods...
public:
    CBoard* CreateBoard(CGamDoc* pDoc);

    int  AddElement(int nBoardSerialNum);

    void Serialize(CArchive& ar);

// Implementation - methods
protected:
    CBoard* GetBoard(int nBoardRow, int nBoardCol);
    CBoard* CloneBoard(CBoard* pOrigBoard);
    void    ComputeNewBoardDimensions(int& rnRows, int& rnCols);
    CPoint  ComputeGraphicalOffset(int nBoardRow, int nBoardCol);
    void    ComputeCellOffset(int nBoardRow, int nBoardCol, int& rnCellRow, int& rnCellCol);
    void    CopyCells(CBoardArray* pBArryTo, CBoardArray* pBArryFrom,
                int nCellRowOffset, int nCellColOffset);
    void    CreateBitmap(CBitmap& m_bmap, CSize size);
    void    CombineLeftAndRight(CBitmap& bmap, TileScale eScale, CBoardArray* pBALeft,
                CBoardArray* pBARight, int nRowLeft, int nColLeft, int nRowRight, int nColRight);
    void    CombineTopAndBottom(CBitmap& bmap, TileScale eScale, CBoardArray* pBATop,
                CBoardArray* pBABottom, int nRowTop, int nColTop, int nRowBottom, int nColBottom);
    int     GetSpecialTileSet();

// Implementation - vars
protected:
    CString m_strName;                  // The board's name
    int     m_nSerialNum;               // The issued serial number

    int     m_nBoardRowCount;           // Number of boards vertically
    int     m_nBoardColCount;           // Number of boards horizontally

    CGamDoc* m_pDoc;                    // Used internally
};

#endif
