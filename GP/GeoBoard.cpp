// GeoBoard.cpp
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

#include    "stdafx.h"
#include    "GamDoc.h"
#include    "Board.h"
#include    "GeoBoard.h"
#include    "versions.h"

#define GEOTILESET_NAME "##-GeoTiles-##"

/////////////////////////////////////////////////////////////////////////////

CGeoBoardElement::CGeoBoardElement(size_t row, size_t col,
                                    BoardID nBoardSerialNum, Rotation90 r) :
    m_nBoardSerialNum(nBoardSerialNum),
    m_rotation(r),
    m_row(row),
    m_col(col)
{
}

/////////////////////////////////////////////////////////////////////////////

void CGeoBoardElement::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_nBoardSerialNum;
        if (!CB::GetFeatures(ar).Check(ftrGeoRotateUnit))
        {
            if (m_rotation != Rotation90::r0)
            {
                AfxThrowArchiveException(CArchiveException::badSchema);
            }
        }
        else
        {
            ar << value_preserving_cast<uint8_t>(m_rotation);
        }
    }
    else
    {
        ar >> m_nBoardSerialNum;
        if (!CB::GetFeatures(ar).Check(ftrGeoRotateUnit))
        {
            m_rotation = Rotation90::r0;
        }
        else
        {
            uint8_t temp;
            ar >> temp;
            m_rotation = static_cast<Rotation90>(temp);
        }
    }
}

CB::string CGeoBoardElement::GetCellNumberStr(const CBoardManager& brdMgr,
                            size_t row, size_t col) const
{
    static const auto Odd = [](size_t x)
    {
        return bool(x & size_t(1));
    };

    // get the original board array
    size_t index = brdMgr.FindBoardBySerial(m_nBoardSerialNum);
    const CBoard& board = brdMgr.GetBoard(index);
    const CBoardArray& ba = board.GetBoardArray();
    const CCellForm& cellForm = ba.GetCellForm(fullScale);

    // undo the Clone() GEV-style logic
    if (m_rotation != Rotation90::r0)
    {
        if (cellForm.GetCellType() == cformHexFlat)
        {
            if (cellForm.GetCellStagger() == CellStagger::Out)
            {
                if (board.IsGEVStyle(Edge::Bottom) &&
                    Odd(col))
                {
                    row = (row + size_t(1)) % ba.GetRows();
                }
                else if (board.IsGEVStyle(Edge::Top) &&
                    !Odd(col))
                {
                    // avoid arithmetic overflow trouble
                    row = (row + ba.GetRows() - size_t(1)) % ba.GetRows();
                }
            }
            else
            {
                ASSERT(!"untested code");
                ASSERT(cellForm.GetCellStagger() == CellStagger::In);
                if (board.IsGEVStyle(Edge::Bottom) &&
                    !Odd(col))
                {
                    row = (row + size_t(1)) % ba.GetRows();
                }
                else if (board.IsGEVStyle(Edge::Top) &&
                    Odd(col))
                {
                    // avoid arithmetic overflow trouble
                    row = (row + ba.GetRows() - size_t(1)) % ba.GetRows();
                }
            }
        }
        else if (cellForm.GetCellType() == cformHexPnt)
        {
            if (cellForm.GetCellStagger() == CellStagger::Out)
            {
                if (board.IsGEVStyle(Edge::Right) &&
                    Odd(row))
                {
                    col = (col + size_t(1)) % ba.GetCols();
                }
                else if (board.IsGEVStyle(Edge::Left) &&
                    !Odd(row))
                {
                    // avoid arithmetic overflow trouble
                    col = (col + ba.GetCols() - size_t(1)) % ba.GetCols();
                }
            }
            else
            {
                ASSERT(cellForm.GetCellStagger() == CellStagger::In);
                if (board.IsGEVStyle(Edge::Right) &&
                    !Odd(row))
                {
                    col = (col + size_t(1)) % ba.GetCols();
                }
                else if (board.IsGEVStyle(Edge::Left) &&
                    Odd(row))
                {
                    // avoid arithmetic overflow trouble
                    col = (col + ba.GetCols() - size_t(1)) % ba.GetCols();
                }
            }
        }
    }

    // undo the Clone() rotation logic
    size_t temp;
    switch (m_rotation)
    {
        case Rotation90::r0:
            break;
        case Rotation90::r90:
            temp = row;
            row = ba.GetCols() - size_t(1) - col;
            col = temp;
            break;
        case Rotation90::r180:
            row = ba.GetRows() - size_t(1) - row;
            col = ba.GetCols() - size_t(1) - col;
            break;
        case Rotation90::r270:
            temp = row;
            row = col;
            col = ba.GetRows() - size_t(1) - temp;
            break;
        default:
            AfxThrowInvalidArgException();
    }

    // return original cell's cell number
    return ba.GetCellNumberStr(row, col);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CGeomorphicBoard::CGeomorphicBoard(CGamDoc& pDoc) :
    m_pDoc(&pDoc)
{
    m_nBoardRowCount = size_t(0);
    m_nBoardColCount = size_t(0);
    m_nSerialNum = BoardID(0);
}

/////////////////////////////////////////////////////////////////////////////

size_t CGeomorphicBoard::AddElement(BoardID nBoardSerialNum, Rotation90 rot)
{
    CGeoBoardElement geo(size() / GetBoardColCount(), size() % GetBoardColCount(),
                        nBoardSerialNum, rot);
    push_back(geo);
    return size() - size_t(1);
}

/////////////////////////////////////////////////////////////////////////////

OwnerPtr<CBoard> CGeomorphicBoard::CreateBoard()
{
    // Procedure:
    // 1) Clone the root board to use as template

    // CBoard::Rotate() requires unrotated source
    const CGeoBoardElement& gbe = GetBoardElt(size_t(0), size_t(0));
    CBoardManager& boardMgr = m_pDoc->GetBoardManager();
    size_t index = boardMgr.FindBoardBySerial(gbe.m_nBoardSerialNum);
    const CBoard& pBrd0 = boardMgr.GetBoard(index);
    OwnerPtr<CBoard> pBrdNew = pBrd0.Clone(*m_pDoc, gbe.m_rotation);

    pBrdNew->SetName(m_strName);
    pBrdNew->SetSerialNumber(m_nSerialNum);

    // 2) Reshape to size new master board.

    CBoardArray& pBrdArrayNew = pBrdNew->GetBoardArray();

    size_t nRows;
    size_t nCols;
    ComputeNewBoardDimensions(nRows, nCols);

    pBrdArrayNew.ReshapeBoard(nRows, nCols, -1, -1, CellStagger::Invalid);

    // 3) Fill cells with content of cells from other boards.
    for (size_t nBoardRow = size_t(0) ; nBoardRow < m_nBoardRowCount ; ++nBoardRow)
    {
        for (size_t nBoardCol = size_t(0) ; nBoardCol < m_nBoardColCount ; ++nBoardCol)
        {
            if (nBoardCol == size_t(0) && nBoardRow == size_t(0))
                continue;                       // Skip this one since it is done already

            const CGeoBoardElement& geoBoardElt = GetBoardElt(nBoardRow, nBoardCol);

            CopyCells(pBrdArrayNew, geoBoardElt);
        }
    }

    // 4) Copy in drawing layers from other board. Offset them
    //    as required based on their board positions.

    for (size_t nBoardRow = size_t(0) ; nBoardRow < m_nBoardRowCount ; ++nBoardRow)
    {
        for (size_t nBoardCol = size_t(0); nBoardCol < m_nBoardColCount ; ++nBoardCol)
        {
            if (nBoardCol == size_t(0) && nBoardRow == size_t(0))
                continue;                       // Skip this one since it is done already
            CPoint pntOffset = ComputeGraphicalOffset(nBoardRow, nBoardCol);

            const CBoard& pBrd = GetBoard(nBoardRow, nBoardCol);

            const CDrawList* pDwgList = pBrd.GetBaseDrawing();
            if (pDwgList != NULL)
            {
                CDrawList* pDwgListNewBase = pBrdNew->GetBaseDrawing(TRUE);
                pDwgListNewBase->AppendWithRotOffset(*pDwgList, Rotation90::r0, pntOffset);
            }

            pDwgList = pBrd.GetTopDrawing();
            if (pDwgList != NULL)
            {
                CDrawList* pDwgListNewTop = pBrdNew->GetTopDrawing(TRUE);
                pDwgListNewTop->AppendWithRotOffset(*pDwgList, Rotation90::r0, pntOffset);
            }
        }
    }

    // 5) fill in m_rowToBoardRow/m_colToBoardCol
    size_t row, col;
    ComputeNewBoardDimensions(row, col);
    m_rowToBoardRow.resize(row);
    m_colToBoardCol.resize(col);
    for (size_t boardRow = size_t(0) ; boardRow < GetBoardRowCount() ; ++boardRow)
    {
        ComputeCellOffset(boardRow, size_t(0), row, col);
        const CBoard& unit = GetBoard(boardRow, size_t(0));
        const CBoardArray& ba = unit.GetBoardArray();
        CellFormType cf = ba.GetCellForm(fullScale).GetCellType();
        for (size_t i = size_t(0) ; i < ba.GetRows() ; ++i)
        {
            switch (cf)
            {
                case cformRect:
                    m_rowToBoardRow[row + i][0].boardCoord = boardRow;
                    m_rowToBoardRow[row + i][0].subBoardCoord = i;
                    m_rowToBoardRow[row + i][1].boardCoord = boardRow;
                    m_rowToBoardRow[row + i][1].subBoardCoord = i;
                    break;
                case cformHexFlat:
                case cformHexPnt:
                    if (i == size_t(0))
                    {
                        m_rowToBoardRow[row + i][1].boardCoord = boardRow;
                        m_rowToBoardRow[row + i][1].subBoardCoord = i;
                    }
                    else if (i == ba.GetRows() - size_t(1))
                    {
                        m_rowToBoardRow[row + i][0].boardCoord = boardRow;
                        m_rowToBoardRow[row + i][0].subBoardCoord = i;
                    }
                    else
                    {
                        m_rowToBoardRow[row + i][0].boardCoord = boardRow;
                        m_rowToBoardRow[row + i][0].subBoardCoord = i;
                        m_rowToBoardRow[row + i][1].boardCoord = boardRow;
                        m_rowToBoardRow[row + i][1].subBoardCoord = i;
                    }
                    break;
                default:
                    AfxThrowInvalidArgException();
            }
        }
    }
    // fixup border cells that didn't get merge treatment
    for (size_t i = size_t(0) ; i < m_rowToBoardRow.size() ; ++i)
    {
        if (m_rowToBoardRow[i][0] == BoardToSubBoard())
        {
            m_rowToBoardRow[i][0] = m_rowToBoardRow[i][1];
        }
        else if (m_rowToBoardRow[i][1] == BoardToSubBoard())
        {
            m_rowToBoardRow[i][1] = m_rowToBoardRow[i][0];
        }
    }
    for (size_t boardCol = size_t(0) ; boardCol < GetBoardColCount() ; ++boardCol)
    {
        ComputeCellOffset(size_t(0), boardCol, row, col);
        const CBoard& unit = GetBoard(size_t(0), boardCol);
        const CBoardArray& ba = unit.GetBoardArray();
        CellFormType cf = ba.GetCellForm(fullScale).GetCellType();
        for (size_t i = size_t(0) ; i < ba.GetCols() ; ++i)
        {
            switch (cf)
            {
                case cformRect:
                    m_colToBoardCol[col + i][0].boardCoord = boardCol;
                    m_colToBoardCol[col + i][0].subBoardCoord = i;
                    m_colToBoardCol[col + i][1].boardCoord = boardCol;
                    m_colToBoardCol[col + i][1].subBoardCoord = i;
                    break;
                case cformHexFlat:
                case cformHexPnt:
                    if (i == size_t(0))
                    {
                        m_colToBoardCol[col + i][1].boardCoord = boardCol;
                        m_colToBoardCol[col + i][1].subBoardCoord = i;
                    }
                    else if (i == ba.GetCols() - size_t(1))
                    {
                        m_colToBoardCol[col + i][0].boardCoord = boardCol;
                        m_colToBoardCol[col + i][0].subBoardCoord = i;
                    }
                    else
                    {
                        m_colToBoardCol[col + i][0].boardCoord = boardCol;
                        m_colToBoardCol[col + i][0].subBoardCoord = i;
                        m_colToBoardCol[col + i][1].boardCoord = boardCol;
                        m_colToBoardCol[col + i][1].subBoardCoord = i;
                    }
                    break;
                default:
                    AfxThrowInvalidArgException();
            }
        }
    }
    // fixup border cells that didn't get merge treatment
    for (size_t i = size_t(0) ; i < m_colToBoardCol.size() ; ++i)
    {
        if (m_colToBoardCol[i][0] == BoardToSubBoard())
        {
            m_colToBoardCol[i][0] = m_colToBoardCol[i][1];
        }
        else if (m_colToBoardCol[i][1] == BoardToSubBoard())
        {
            m_colToBoardCol[i][1] = m_colToBoardCol[i][0];
        }
    }

    return pBrdNew;
}

void CGeomorphicBoard::DeleteFromBoardManager()
{
    CBoardManager& pBMgr = m_pDoc->GetBoardManager();
    size_t nBrd = pBMgr.FindBoardBySerial(GetSerialNumber());
    pBMgr.DeleteBoard(nBrd);
}

size_t CTileManager::GetSpecialTileSet()
{
    size_t nTSet = FindNamedTileSet(GEOTILESET_NAME);
    if (nTSet == Invalid_v<size_t>)
        nTSet = CreateTileSet(GEOTILESET_NAME);
    ASSERT(nTSet != Invalid_v<size_t>);
    return nTSet;
}

void CGeomorphicBoard::CopyCells(CBoardArray& pBArryTo,
                                const CGeoBoardElement& gbeFrom)
{
    static const auto Odd = [](size_t x) {
        return bool(x & size_t(1));
    };

    size_t nCellRowOffset;
    size_t nCellColOffset;
    ComputeCellOffset(gbeFrom.GetRow(), gbeFrom.GetCol(), nCellRowOffset, nCellColOffset);

    const CBoard& pBrdFrom = GetBoard(gbeFrom);
    const CBoardArray& pBArryFrom = pBrdFrom.GetBoardArray();

    bool mergeTop = NeedsMerge(gbeFrom, Edge::Top);
    bool mergeLeft = NeedsMerge(gbeFrom, Edge::Left);

    CTileManager& pTMgr = m_pDoc->GetTileManager();
    for (size_t nRow = size_t(0) ; nRow < pBArryFrom.GetRows() ; ++nRow)
    {
        for (size_t nCol = size_t(0) ; nCol < pBArryFrom.GetCols() ; ++nCol)
        {
            size_t nRowTo = nRow + nCellRowOffset;
            size_t nColTo = nCol + nCellColOffset;

            const BoardCell& pCellFrom = pBArryFrom.GetCell(nRow, nCol);
            if (pBrdFrom.IsEmpty(pCellFrom))
            {
                continue;
            }
            BoardCell& pCellTo = pBArryTo.GetCell(nRowTo, nColTo);
            bool mergeTopBottom = false;
            bool mergeLeftRight = false;
            if (mergeTop && nRow == size_t(0))
            {
                switch (pBArryFrom.GetCellForm(fullScale).GetCellType())
                {
                    case CellFormType::cformRect:
                        break;
                    case cformHexFlat:
                        switch (pBrdFrom.GetStagger(Corner::TL))
                        {
                            case CellStagger::Out:
                                mergeTopBottom = !Odd(nCol);
                                break;
                            case CellStagger::In:
                                mergeTopBottom = Odd(nCol);
                                break;
                            default:
                                AfxThrowInvalidArgException();
                        }
                        break;
                    case cformHexPnt:
                        mergeTopBottom = true;
                        break;
                    default:
                        AfxThrowInvalidArgException();
                }
            }
            if (mergeLeft && nCol == size_t(0))
            {
                switch (pBArryFrom.GetCellForm(fullScale).GetCellType())
                {
                    case CellFormType::cformRect:
                        break;
                    case cformHexPnt:
                        switch (pBrdFrom.GetStagger(Corner::TL))
                        {
                            case CellStagger::Out:
                                mergeTopBottom = !Odd(nRow);
                                break;
                            case CellStagger::In:
                                mergeTopBottom = Odd(nRow);
                                break;
                            default:
                                AfxThrowInvalidArgException();
                        }
                        break;
                    case cformHexFlat:
                        mergeLeftRight = true;
                        break;
                    default:
                        AfxThrowInvalidArgException();
                }
            }

            if ((mergeLeftRight || mergeTopBottom) &&
                pCellFrom != pCellTo)
            {
                // We need to create a tile that has the half images of the
                // two hex cells joined as one.
                // We don't bother trying to perform 4-way merge for mergeLeft && mergeTop
                OwnerOrNullPtr<CBitmap> bmapFull;
                OwnerOrNullPtr<CBitmap> bmapHalf;
                if (mergeLeftRight)
                {
                    // Handle left and right halves
                    bmapFull = CombineLeftAndRight(fullScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                    bmapHalf = CombineLeftAndRight(halfScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                }
                else
                {
                    ASSERT(mergeTopBottom);
                    // Handle top and bottom halves
                    bmapFull = CombineTopAndBottom(fullScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                    bmapHalf = CombineTopAndBottom(halfScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                }
                // Determine small cell color. Use the 'To' cell as value;
                COLORREF crSmall;
                if (pCellTo.IsTileID())
                {
                    CTile tile = pTMgr.GetTile(pCellTo.GetTID(), smallScale);
                    crSmall = tile.GetSmallColor();
                }
                else
                    crSmall = pCellTo.GetColor();
                CSize sizeFull = pBArryTo.GetCellSize(fullScale);
                CSize sizeHalf = pBArryTo.GetCellSize(halfScale);
                TileID tidNew = pTMgr.CreateTile(pTMgr.GetSpecialTileSet(),
                    sizeFull, sizeHalf, crSmall);
                pTMgr.UpdateTile(tidNew, *bmapFull, *bmapHalf, crSmall);
                pCellTo.SetTID(tidNew);
            }
            else
                pCellTo = pCellFrom;
        }
    }
}

OwnerPtr<CBitmap> CGeomorphicBoard::CombineLeftAndRight(TileScale eScale,
    const CBoardArray& pBALeft, const CBoardArray& pBARight, size_t nRowLeft, size_t nColLeft,
    size_t nRowRight, size_t nColRight) const
{
    CDC dc;
    dc.CreateCompatibleDC(NULL);

    CSize sizeCell = pBALeft.GetCellSize(eScale);
    OwnerPtr<CBitmap> retval = CreateBitmap(sizeCell);

    // Handle left half...
    CBitmap* prvBMap = dc.SelectObject(&*retval);

    CRect rct = pBALeft.GetCellRect(nRowLeft, nColLeft, eScale);

    dc.SetViewportOrg(-rct.left, -rct.top);
    rct.right -= rct.Width() / 2 - (rct.Width() % 2);
    dc.IntersectClipRect(&rct);

    pBALeft.FillCell(dc, nRowLeft, nColLeft, eScale);
    dc.SelectClipRgn(NULL);

    // Handle right half...
    rct = pBARight.GetCellRect(nRowRight, nColRight, eScale);

    dc.SetViewportOrg(-rct.left, -rct.top);
    rct.left += rct.Width() / 2 + (rct.Width() % 2);
    dc.IntersectClipRect(&rct);

    pBARight.FillCell(dc, nRowRight, nColRight, eScale);

    dc.SelectObject(prvBMap);

    return retval;
}

OwnerPtr<CBitmap> CGeomorphicBoard::CombineTopAndBottom(TileScale eScale,
    const CBoardArray& pBATop, const CBoardArray& pBABottom, size_t nRowTop, size_t nColTop,
    size_t nRowBottom, size_t nColBottom) const
{
    CDC dc;
    dc.CreateCompatibleDC(NULL);

    CSize sizeCell = pBATop.GetCellSize(eScale);
    OwnerPtr<CBitmap> retval = CreateBitmap(sizeCell);

    // Handle top half...
    CBitmap* prvBMap = dc.SelectObject(&*retval);

    CRect rct = pBATop.GetCellRect(nRowTop, nColTop, eScale);

    dc.SetViewportOrg(-rct.left, -rct.top);
    rct.bottom += rct.Height() / 2 - (rct.Height() % 2);
    dc.IntersectClipRect(&rct);

    pBATop.FillCell(dc, nRowTop, nColTop, eScale);
    dc.SelectClipRgn(NULL);

    // Handle bottom half...
    rct = pBABottom.GetCellRect(nRowBottom, nColBottom, eScale);

    dc.SetViewportOrg(-rct.left, -rct.top);
    rct.top += rct.Height() / 2 + (rct.Height() % 2);
    dc.IntersectClipRect(&rct);

    pBABottom.FillCell(dc, nRowBottom, nColBottom, eScale);

    dc.SelectObject(prvBMap);

    return retval;
}

// Create DIB Section bitmap that's filled with the transparent color.
OwnerPtr<CBitmap> CGeomorphicBoard::CreateBitmap(CSize size) const
{
    CDC dc;
    dc.CreateCompatibleDC(NULL);

    OwnerPtr<CBitmap> retval = CreateRGBDIBSection(size.cx, size.cy);
    CBitmap* prvBMap = dc.SelectObject(&*retval);

    CBrush brush(m_pDoc->GetTileManager().GetTransparentColor());
    CBrush* prvBrush = dc.SelectObject(&brush);

    dc.PatBlt(0, 0, size.cx, size.cy, PATCOPY);

    dc.SelectObject(prvBrush);
    dc.SelectObject(prvBMap);

    return retval;
}

CPoint CGeomorphicBoard::ComputeGraphicalOffset(size_t nBoardRow, size_t nBoardCol) const
{
    const CBoard& pBrdRoot = GetBoard(size_t(0), size_t(0));
    CSize sizeCell = pBrdRoot.GetBoardArray().GetCellSize(fullScale);
    CPoint pnt(0, 0);
    for (size_t nCol = size_t(0) ; nCol < nBoardCol ; ++nCol)
    {
        const CGeoBoardElement& gbe = GetBoardElt(size_t(0), nCol);
        const CBoard& pBrd = GetBoard(gbe);
        const CBoardArray& pBArray = pBrd.GetBoardArray();
        pnt.x += pBArray.GetWidth(fullScale);
        if (NeedsMerge(gbe, Edge::Right))
        {
            const CGeoBoardElement& nextGbe = GetBoardElt(size_t(0), nCol + size_t(1));
            const CBoard& nextBrd = GetBoard(nextGbe);
            if (pBrd.IsGEVStyle(Edge::Right) ||
                nextBrd.IsGEVStyle(Edge::Left))
            {
                // remove the width provided by the empty half-cell col
                pnt.x -= sizeCell.cx / 2;
            }
            pnt.x -= sizeCell.cx;               // A column is shared
        }
        else if (pBArray.GetCellForm(fullScale).GetCellType() == cformHexFlat)
        {
            // +2 to round to nearest int
            pnt.x -= (sizeCell.cx + 2) / 4;
        }
        else if (pBArray.GetCellForm(fullScale).GetCellType() == cformHexPnt)
        {
            pnt.x -= sizeCell.cx / 2;           // A column is half shared
        }
    }
    for (size_t nRow = size_t(0) ; nRow < nBoardRow ; ++nRow)
    {
        const CGeoBoardElement& gbe = GetBoardElt(nRow, size_t(0));
        const CBoard& pBrd = GetBoard(gbe);
        const CBoardArray& pBArray = pBrd.GetBoardArray();
        pnt.y += pBArray.GetHeight(fullScale);
        if (NeedsMerge(gbe, Edge::Bottom))
        {
            const CGeoBoardElement& nextGbe = GetBoardElt(nRow + size_t(1), size_t(0));
            const CBoard& nextBrd = GetBoard(nextGbe);
            if (pBrd.IsGEVStyle(Edge::Bottom) ||
                nextBrd.IsGEVStyle(Edge::Top))
            {
                // remove the height provided by the empty half-cell row
                pnt.y -= sizeCell.cy / 2;
            }
            pnt.y -= sizeCell.cy;               // A row is shared
        }
        else if (pBArray.GetCellForm(fullScale).GetCellType() == cformHexPnt)
        {
            // +2 to round to nearest int
            pnt.y -= (sizeCell.cy + 2) / 4;
        }
        else if (pBArray.GetCellForm(fullScale).GetCellType() == cformHexFlat)
        {
            pnt.y -= sizeCell.cy / 2;           // A row is half shared
        }
    }
    return pnt;
}

void CGeomorphicBoard::ComputeNewBoardDimensions(size_t& rnRows, size_t& rnCols) const
{
    // size is offset of bottom right board, plus bottom right's size
    const CGeoBoardElement& bottomRightGBE = GetBoardElt(GetBoardRowCount() - size_t(1), GetBoardColCount() - size_t(1));
    size_t bottomRightRowOffset, bottomRightRowOffsetColOffset;
    ComputeCellOffset(bottomRightGBE.GetRow(), bottomRightGBE.GetCol(),
                        bottomRightRowOffset, bottomRightRowOffsetColOffset);
    const CBoard& bottomRightBrd = GetBoard(bottomRightGBE);
    const CBoardArray& bottomRightBArray = bottomRightBrd.GetBoardArray();
    rnRows = bottomRightRowOffset + bottomRightBArray.GetRows();
    rnCols = bottomRightRowOffsetColOffset + bottomRightBArray.GetCols();
}

void CGeomorphicBoard::ComputeCellOffset(size_t nBoardRow, size_t nBoardCol,
    size_t& rnCellRow, size_t& rnCellCol) const
{
    rnCellRow = size_t(0);
    rnCellCol = size_t(0);
    for (size_t nCol = size_t(0) ; nCol < nBoardCol ; ++nCol)
    {
        const CGeoBoardElement& gbe = GetBoardElt(size_t(0), nCol);
        const CBoard& pBrd = GetBoard(gbe);
        const CBoardArray& pBArray = pBrd.GetBoardArray();
        ASSERT(pBArray.GetCols() >= size_t(1));
        rnCellCol += pBArray.GetCols();
        if (NeedsMerge(gbe, Edge::Right))
        {
            --rnCellCol;
        }
    }
    for (size_t nRow = size_t(0) ; nRow < nBoardRow ; ++nRow)
    {
        const CGeoBoardElement& gbe = GetBoardElt(nRow, size_t(0));
        const CBoard& pBrd = GetBoard(gbe);
        const CBoardArray& pBArray = pBrd.GetBoardArray();
        ASSERT(pBArray.GetRows() >= size_t(1));
        rnCellRow += pBArray.GetRows();
        if (NeedsMerge(gbe, Edge::Bottom))
        {
            --rnCellRow;
        }
    }
}

const CGeoBoardElement& CGeomorphicBoard::GetBoardElt(size_t nBoardRow, size_t nBoardCol) const
{
    ASSERT(nBoardRow < GetBoardRowCount() &&
            nBoardCol < GetBoardColCount());
    size_t nGeoIndex = nBoardRow * m_nBoardColCount + nBoardCol;
    return (*this)[nGeoIndex];
}

const CBoard& CGeomorphicBoard::GetBoard(const CGeoBoardElement& geo) const
{
    return m_pDoc->GetBoardManager().Get(geo);
}

namespace
{
    OwnerOrNullPtr<CDrawList> Rotate(const CBoard& board, const CDrawList* (CBoard::*drawlist)() const, Rotation90 r)
    {
        const CDrawList* src = (board.*drawlist)();
        if (!src)
        {
            return nullptr;
        }

        CPoint offset;
        switch (r)
        {
            case Rotation90::r90:
                offset = CPoint(board.GetHeight(fullScale), 0);
                break;
            case Rotation90::r180:
            {
                offset = CPoint(board.GetWidth(fullScale), board.GetHeight(fullScale));
                const CBoardArray& ba = board.GetBoardArray();
                const CCellForm& cf = ba.GetCellForm(fullScale);
                if (cf.GetCellType() == cformHexFlat &&
                    cf.GetCellStagger() == CellStagger::Out)
                {
                    if (board.IsGEVStyle(Edge::Top))
                    {
                        offset += CSize(0, cf.GetCellSize().cy / 2);
                    }
                    else if (board.IsGEVStyle(Edge::Bottom))
                    {
                        offset += CSize(0, -cf.GetCellSize().cy/2);
                    }
                }
                else if (cf.GetCellType() == cformHexPnt &&
                    cf.GetCellStagger() == CellStagger::Out)
                {
                    if (board.IsGEVStyle(Edge::Right))
                    {
                        offset += CSize(-cf.GetCellSize().cx/2, 0);
                    }
                    if (board.IsGEVStyle(Edge::Left))
                    {
                        offset += CSize(cf.GetCellSize().cx/2, 0);
                    }
                }
                break;
            }
            case Rotation90::r270:
                offset = CPoint(0, board.GetWidth(fullScale));
                break;
            default:
                AfxThrowInvalidArgException();
        }

        std::unique_ptr<CDrawList> dest(new CDrawList);
        dest->AppendWithRotOffset(*src, r, offset);
        return dest.release();
    }
}

// WARNING:  only supported on unrotated boards
OwnerPtr<CBoard> CBoard::Clone(CGamDoc& doc, Rotation90 r) const
{
    // We need to force the current version at this point because
    // we may be loading an earlier version game or scenario. In
    // this case the board data has already been upgraded. Make
    // a copy of the current loading version so we can restore it.
    CGamDoc::SetLoadingVersionGuard setLoadingVersionGuard(NumVersion(fileGsnVerMajor, fileGsnVerMinor));

    CMemFile file;
    CArchive arSave(&file, CArchive::store);
    arSave.m_pDocument = &doc;
    SetFileFeaturesGuard setFileFeaturesGuard(arSave, GetCBFeatures());
    // save should not modify src, so should be safe
    const_cast<CBoard&>(*this).Serialize(arSave);      // Make a copy of the board
    arSave.Close();

    file.SeekToBegin();
    CArchive arRestore(&file, CArchive::load);
    arRestore.m_pDocument = &doc;
    OwnerPtr<CBoard> pNewBoard = MakeOwner<CBoard>();
    pNewBoard->Serialize(arRestore);
    pNewBoard->m_nSerialNum = Invalid_v<BoardID>;

    if (r == Rotation90::r0)
    {
        return pNewBoard;
    }

    static const auto Odd = [](size_t x)
    {
        return bool(x & size_t(1));
    };

    switch (r)
    {
        case Rotation90::r90:
        case Rotation90::r180:
        case Rotation90::r270:
        {
            pNewBoard->SetBaseDrawing(Rotate(*this, &CBoard::GetBaseDrawing, r));

            const CBoardArray& srcBoardArray = GetBoardArray();
            const CCellForm& srcCellForm = srcBoardArray.GetCellForm(fullScale);
            // should have been blocked by DglNewGeoBoard, but just in case...
            switch (srcCellForm.GetCellType())
            {
                case cformRect:
                    if (r != Rotation90::r180 &&
                        srcCellForm.GetCellSize().cx != srcCellForm.GetCellSize().cy)
                    {
                        AfxThrowInvalidArgException();
                    }
                    break;
                default:
                    if (r != Rotation90::r180)
                    {
                        AfxThrowInvalidArgException();
                    }
            }
            CBoardArray& dstBoardArray = pNewBoard->GetBoardArray();
            CellStagger dstStagger = srcCellForm.GetCellStagger();
            int parm1 = -1, parm2 = 0;
            size_t rows = Invalid_v<size_t>, cols = Invalid_v<size_t>;
            switch (srcCellForm.GetCellType())
            {
                case cformRect:
                    switch (r)
                    {
                        case Rotation90::r90:
                        case Rotation90::r270:
                            parm1 = srcCellForm.GetCellSize().cx;
                            parm2 = srcCellForm.GetCellSize().cy;
                            rows = srcBoardArray.GetCols();
                            cols = srcBoardArray.GetRows();
                            break;
                        case Rotation90::r180:
                            parm1 = srcCellForm.GetCellSize().cy;
                            parm2 = srcCellForm.GetCellSize().cx;
                            rows = srcBoardArray.GetRows();
                            cols = srcBoardArray.GetCols();
                            break;
                        default:
                            ASSERT(!"impossible");
                    }
                    break;
                case cformHexFlat:
                    if (Odd(srcBoardArray.GetCols()) &&
                        !(IsGEVStyle(Edge::Top) || IsGEVStyle(Edge::Bottom)))
                    {
                        dstStagger = ~dstStagger;
                    }
                    parm1 = srcCellForm.GetCellSize().cy;
                    rows = srcBoardArray.GetRows();
                    cols = srcBoardArray.GetCols();
                    break;
                case cformHexPnt:
                    if (Odd(srcBoardArray.GetRows()) &&
                        !(IsGEVStyle(Edge::Left) || IsGEVStyle(Edge::Right)))
                    {
                        dstStagger = ~dstStagger;
                    }
                    parm1 = srcCellForm.GetCellSize().cx;
                    rows = srcBoardArray.GetRows();
                    cols = srcBoardArray.GetCols();
                    break;
                default:
                    AfxThrowInvalidArgException();
            }
            dstBoardArray.CreateBoard(srcCellForm.GetCellType(),
                                        rows, cols,
                                        parm1, parm2,
                                        dstStagger);
            CTileManager& tileMgr = *pNewBoard->m_pTMgr;
            for (size_t srcY = size_t(0) ; srcY < srcBoardArray.GetRows() ; ++srcY)
            {
                for (size_t srcX = size_t(0) ; srcX < srcBoardArray.GetCols(); ++srcX)
                {
                    size_t destX = Invalid_v<size_t>;
                    size_t destY = Invalid_v<size_t>;
                    switch (r)
                    {
                        case Rotation90::r90:
                            destY = srcX;
                            destX = srcBoardArray.GetRows() - size_t(1) - srcY;
                            break;
                        case Rotation90::r180:
                            destX = srcBoardArray.GetCols() - size_t(1) - srcX;
                            destY = srcBoardArray.GetRows() - size_t(1) - srcY;
                            break;
                        case Rotation90::r270:
                            destY = srcBoardArray.GetCols() - size_t(1) - srcX;
                            destX = srcY;
                            break;
                        default:
                            ASSERT(!"impossible");
                    }
                    // preserve the GEVStyle edge so the maps are compatible
                    if (srcCellForm.GetCellType() == cformHexFlat)
                    {
                        if (srcCellForm.GetCellStagger() == CellStagger::Out)
                        {
                            if (IsGEVStyle(Edge::Bottom) &&
                                Odd(destX))
                            {
                                // avoid arithmetic overflow trouble
                                destY = (destY + dstBoardArray.GetRows() - size_t(1)) % dstBoardArray.GetRows();
                            }
                            else if (IsGEVStyle(Edge::Top) &&
                                !Odd(destX))
                            {
                                destY = (destY + size_t(1)) % dstBoardArray.GetRows();
                            }
                        }
                        else
                        {
                            ASSERT(!"untested code");
                            ASSERT(srcCellForm.GetCellStagger() == CellStagger::In);
                            if (IsGEVStyle(Edge::Bottom) &&
                                !Odd(destX))
                            {
                                // avoid arithmetic overflow trouble
                                destY = (destY + dstBoardArray.GetRows() - size_t(1)) % dstBoardArray.GetRows();
                            }
                            else if (IsGEVStyle(Edge::Top) &&
                                Odd(destX))
                            {
                                destY = (destY + size_t(1)) % dstBoardArray.GetRows();
                            }
                        }
                    }
                    else if (srcCellForm.GetCellType() == cformHexPnt)
                    {
                        if (srcCellForm.GetCellStagger() == CellStagger::Out)
                        {
                            if (IsGEVStyle(Edge::Right) &&
                                Odd(destY))
                            {
                                // avoid arithmetic overflow trouble
                                destX = (destX + dstBoardArray.GetCols() - size_t(1)) % dstBoardArray.GetCols();
                            }
                            else if (IsGEVStyle(Edge::Left) &&
                                !Odd(destY))
                            {
                                destX = (destX + size_t(1)) % dstBoardArray.GetCols();
                            }
                        }
                        else
                        {
                            ASSERT(srcCellForm.GetCellStagger() == CellStagger::In);
                            if (IsGEVStyle(Edge::Right) &&
                                !Odd(destY))
                            {
                                // avoid arithmetic overflow trouble
                                destX = (destX + dstBoardArray.GetCols() - size_t(1)) % dstBoardArray.GetCols();
                            }
                            else if (IsGEVStyle(Edge::Left) &&
                                Odd(destY))
                            {
                                destX = (destX + size_t(1)) % dstBoardArray.GetCols();
                            }
                        }
                    }

                    const BoardCell& cell = srcBoardArray.GetCell(srcY, srcX);
                    if (!cell.IsTileID())
                    {
                        dstBoardArray.GetCell(destY, destX) = cell;
                    }
                    else
                    {
                        TileID tid = cell.GetTID();
                        ASSERT(tid != nullTid);
                        dstBoardArray.GetCell(destY, destX).SetTID(tileMgr.Get(tid, r));
                    }
                }
            }
            for (Edge e : { Edge::Top, Edge::Bottom, Edge::Left, Edge::Right })
            {
                ASSERT(pNewBoard->IsGEVStyle(e) == IsGEVStyle(e));
            }

            pNewBoard->SetTopDrawing(Rotate(*this, &CBoard::GetTopDrawing, r));

            return pNewBoard;
        }
        default:
            AfxThrowInvalidArgException();
    }
}

/////////////////////////////////////////////////////////////////////////////

bool CGeomorphicBoard::NeedsGeoRotate() const
{
    for (const CGeoBoardElement& elt : *this)
    {
        if (elt.m_rotation != Rotation90::r0)
        {
            return true;
        }
    }
    return false;
}

void CGeomorphicBoard::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_strName;

        ar << m_nSerialNum;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(m_nBoardRowCount);
            ar << value_preserving_cast<WORD>(m_nBoardColCount);
        }
        else
        {
            CB::WriteCount(ar, m_nBoardRowCount);
            CB::WriteCount(ar, m_nBoardColCount);
        }

        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(size());
        }
        else
        {
            CB::WriteCount(ar, size());
        }
        for (size_t i = size_t(0); i < size(); ++i)
        {
            CGeoBoardElement& elt = (*this)[i];
            if (!CB::GetFeatures(ar).Check(ftrGeoSquareCell))
            {
                /* old file format, so enforce old geoboard
                    restrictions (see old DlgNewGeoBoard.cpp for
                    old restrictions)
                    (check here because CGeoBoardElement doesn't
                    have access to CBoard) */
                const CBoard& board = GetBoard(elt);
                const CBoardArray& bArray = board.GetBoardArray();
                CellFormType cellType = bArray.GetCellForm(fullScale).GetCellType();
                if (!(cellType == cformHexFlat && (bArray.GetCols() & size_t(1)) != size_t(0) ||
                        cellType == cformHexPnt && (bArray.GetRows() & size_t(1)) != size_t(0)))
                {
                    if (!GetCBFeatures().Check(ftrGeoSquareCell))
                    {
                        AfxThrowArchiveException(CArchiveException::badSchema);
                    }
                    CB::AddFeature(ar, ftrGeoSquareCell);
                }
            }
            elt.Serialize(ar);
        }
    }
    else
    {
        WORD wTmp;

        ar >> m_strName;

        ar >> m_nSerialNum;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar >> wTmp; m_nBoardRowCount = value_preserving_cast<size_t>(wTmp);
            ar >> wTmp; m_nBoardColCount = value_preserving_cast<size_t>(wTmp);
        }
        else
        {
            m_nBoardRowCount = CB::ReadCount(ar);
            m_nBoardColCount = CB::ReadCount(ar);
        }

        clear();
        size_t wCount;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar >> wTmp;
            wCount = wTmp;
        }
        else
        {
            wCount = CB::ReadCount(ar);
        }
        while (wCount--)
        {
             CGeoBoardElement geo(size() / GetBoardColCount(),
                                    size() % GetBoardColCount(),
                                    BoardID(0), Rotation90::r0);
             geo.Serialize(ar);
             push_back(geo);
        }
    }
}

CB::string CGeomorphicBoard::GetCellNumberStr(CPoint pnt, TileScale eScale) const
{
    // find sub-board containing pnt
    const CBoardManager& brdMgr = m_pDoc->GetBoardManager();
    size_t index = brdMgr.FindBoardBySerial(m_nSerialNum);
    const CBoard& brd = brdMgr.GetBoard(index);
    const CBoardArray& ba = brd.GetBoardArray();
    size_t overallRow, overallCol;
    if (!ba.FindCell(pnt.x, pnt.y, overallRow, overallCol, eScale))
    {
        return CB::string();
    }
    const CCellForm& cf = ba.GetCellForm(eScale);
    // find which quadrant of cell
    CRect rect = cf.GetRect(value_preserving_cast<CB::ssize_t>(overallRow),
                            value_preserving_cast<CB::ssize_t>(overallCol));
    ASSERT(PtInRect(rect, pnt));
    Edge tb = pnt.y < (rect.top + rect.bottom)/2 ? Edge::Top : Edge::Bottom;
    size_t tbCvt = tb == Edge::Top ? size_t(0) : size_t(1);
    Edge lr = pnt.x < (rect.left + rect.right)/2 ? Edge::Left : Edge::Right;
    size_t lrCvt = lr == Edge::Left ? size_t(0) : size_t(1);
    // get subboard corresponding to quadrant
    const CGeoBoardElement& gbe = GetBoardElt(m_rowToBoardRow[overallRow][tbCvt].boardCoord,
                                                m_colToBoardCol[overallCol][lrCvt].boardCoord);
    const CBoard& unitBoard = GetBoard(gbe);
    const CBoardArray& unitBA = unitBoard.GetBoardArray();
    /* if the cell is a GEV-style non-cell,
                and another board shares the cell,
            use the other board's cell instead */
    size_t row = m_rowToBoardRow[overallRow][tbCvt].subBoardCoord;
    size_t col = m_colToBoardCol[overallCol][lrCvt].subBoardCoord;
    if (unitBoard.IsEmpty(unitBA.GetCell(row, col)))
    {
        if ((row == size_t(0) && unitBoard.IsGEVStyle(Edge::Top) ||
                row == unitBA.GetRows() - size_t(1) && unitBoard.IsGEVStyle(Edge::Bottom)) &&
            m_rowToBoardRow[overallRow][size_t(0)] != m_rowToBoardRow[overallRow][size_t(1)])
        {
            const CGeoBoardElement& otherGbe = GetBoardElt(m_rowToBoardRow[overallRow][size_t(1) - tbCvt].boardCoord,
                                                            m_colToBoardCol[overallCol][lrCvt].boardCoord);
            size_t otherRow = m_rowToBoardRow[overallRow][size_t(1) - tbCvt].subBoardCoord;
            return otherGbe.GetCellNumberStr(brdMgr, otherRow, col);
        }
        else if ((col == size_t(0) && unitBoard.IsGEVStyle(Edge::Left) ||
                    col == unitBA.GetCols() - size_t(1) && unitBoard.IsGEVStyle(Edge::Right)) &&
            m_colToBoardCol[overallCol][size_t(1)] != m_colToBoardCol[overallCol][size_t(1)])
        {
            const CGeoBoardElement& otherGbe = GetBoardElt(m_rowToBoardRow[overallRow][tbCvt].boardCoord,
                                                            m_colToBoardCol[overallCol][size_t(1) - lrCvt].boardCoord);
            size_t otherCol = m_colToBoardCol[overallCol][size_t(1) - lrCvt].subBoardCoord;
            return otherGbe.GetCellNumberStr(brdMgr, row, otherCol);
        }
    }
    return gbe.GetCellNumberStr(brdMgr, row, col);
}

bool CGeomorphicBoard::NeedsMerge(const CGeoBoardElement& gbe, Edge e) const
{
    switch (e)
    {
        case Edge::Top:
            if (gbe.GetRow() == size_t(0))
            {
                return false;
            }
            return NeedsMerge(GetBoardElt(gbe.GetRow() - size_t(1), gbe.GetCol()), Edge::Bottom);
        case Edge::Bottom:
        {
            if (gbe.GetRow() + size_t(1) == GetBoardRowCount())
            {
                return false;
            }
            const CBoard& brd = GetBoard(gbe);
            const CBoardArray& bArray = brd.GetBoardArray();
            const CGeoBoardElement& nextGBE = GetBoardElt(gbe.GetRow() + size_t(1), gbe.GetCol());
            const CBoard& nextBrd = GetBoard(nextGBE);
            const CBoardArray& nextBArray = nextBrd.GetBoardArray();
            switch (bArray.GetCellForm(fullScale).GetCellType())
            {
                case cformRect:
                    return false;
                case cformHexFlat:
                case cformHexPnt:
                    return brd.GetStagger(Corner::BL) == nextBrd.GetStagger(Corner::TL);
                default:
                    AfxThrowInvalidArgException();
            }
        }
        case Edge::Left:
            if (gbe.GetCol() == size_t(0))
            {
                return false;
            }
            return NeedsMerge(GetBoardElt(gbe.GetRow(), gbe.GetCol() - size_t(1)), Edge::Right);
        case Edge::Right:
        {
            if (gbe.GetCol() + size_t(1) == GetBoardColCount())
            {
                return false;
            }
            const CBoard& brd = GetBoard(gbe);
            const CBoardArray& bArray = brd.GetBoardArray();
            const CGeoBoardElement& nextGBE = GetBoardElt(gbe.GetRow(), gbe.GetCol() + size_t(1));
            const CBoard& nextBrd = GetBoard(nextGBE);
            const CBoardArray& nextBArray = nextBrd.GetBoardArray();
            switch (bArray.GetCellForm(fullScale).GetCellType())
            {
                case cformRect:
                    return false;
                case cformHexFlat:
                case cformHexPnt:
                    return brd.GetStagger(Corner::TR) == nextBrd.GetStagger(Corner::TL);
                default:
                    AfxThrowInvalidArgException();
            }
            break;
        }
        default:
            AfxThrowInvalidArgException();
    }
}

bool CGeomorphicBoard::BoardToSubBoard::operator==(const BoardToSubBoard& rhs) const
{
    return boardCoord == rhs.boardCoord &&
            subBoardCoord == rhs.subBoardCoord;
}
