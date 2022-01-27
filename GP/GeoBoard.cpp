// GeoBoard.cpp
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

#include    "stdafx.h"
#include    "GamDoc.h"
#include    "Board.h"
#include    "GeoBoard.h"
#include    "versions.h"

#define GEOTILESET_NAME "##-GeoTiles-##"

/////////////////////////////////////////////////////////////////////////////

CGeoBoardElement::CGeoBoardElement(size_t row, size_t col,
                                    BoardID nBoardSerialNum /*= BoardID(0)*/) :
    m_nBoardSerialNum(nBoardSerialNum),
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
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            if (m_nReserved != uint8_t(0))
            {
                AfxThrowArchiveException(CArchiveException::badSchema);
            }
        }
        else
        {
            ar << m_nReserved;
        }
    }
    else
    {
        ar >> m_nBoardSerialNum;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            m_nReserved = uint8_t(0);
        }
        else
        {
            ar >> m_nReserved;
        }
    }
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

size_t CGeomorphicBoard::AddElement(BoardID nBoardSerialNum)
{
    CGeoBoardElement geo(size() / GetBoardColCount(), size() % GetBoardColCount(),
                        nBoardSerialNum);
    push_back(geo);
    return size() - size_t(1);
}

/////////////////////////////////////////////////////////////////////////////

OwnerPtr<CBoard> CGeomorphicBoard::CreateBoard()
{
    // Procedure:
    // 1) Clone the root board to use as template

    const CBoard& pBrd = GetBoard(size_t(0), size_t(0));
    OwnerPtr<CBoard> pBrdNew = CloneBoard(pBrd);

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
                pDwgListNewBase->AppendWithOffset(*pDwgList, pntOffset);
            }

            pDwgList = pBrd.GetTopDrawing();
            if (pDwgList != NULL)
            {
                CDrawList* pDwgListNewTop = pBrdNew->GetTopDrawing(TRUE);
                pDwgListNewTop->AppendWithOffset(*pDwgList, pntOffset);
            }
        }
    }

    return pBrdNew;
}

void CGeomorphicBoard::DeleteFromBoardManager()
{
    CBoardManager* pBMgr = m_pDoc->GetBoardManager();
    if (pBMgr != NULL)
    {
        size_t nBrd = pBMgr->FindBoardBySerial(GetSerialNumber());
        pBMgr->DeleteBoard(nBrd);
    }
}

size_t CGeomorphicBoard::GetSpecialTileSet()
{
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    size_t nTSet = pTMgr->FindNamedTileSet(GEOTILESET_NAME);
    if (nTSet == Invalid_v<size_t>)
        nTSet = pTMgr->CreateTileSet(GEOTILESET_NAME);
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

    CTileManager* pTMgr = m_pDoc->GetTileManager();
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
                CBitmap bmapFull;
                CBitmap bmapHalf;
                if (mergeLeftRight)
                {
                    // Handle left and right halves
                    CombineLeftAndRight(bmapFull, fullScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                    CombineLeftAndRight(bmapHalf, halfScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                }
                else
                {
                    ASSERT(mergeTopBottom);
                    // Handle top and bottom halves
                    CombineTopAndBottom(bmapFull, fullScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                    CombineTopAndBottom(bmapHalf, halfScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                }
                // Determine small cell color. Use the 'To' cell as value;
                COLORREF crSmall;
                if (pCellTo.IsTileID())
                {
                    CTile tile = pTMgr->GetTile(pCellTo.GetTID(), smallScale);
                    crSmall = tile.GetSmallColor();
                }
                else
                    crSmall = pCellTo.GetColor();
                CSize sizeFull = pBArryTo.GetCellSize(fullScale);
                CSize sizeHalf = pBArryTo.GetCellSize(halfScale);
                TileID tidNew = pTMgr->CreateTile(GetSpecialTileSet(),
                    sizeFull, sizeHalf, crSmall);
                pTMgr->UpdateTile(tidNew, bmapFull, bmapHalf, crSmall);
                pCellTo.SetTID(tidNew);
            }
            else
                pCellTo = pCellFrom;
        }
    }
}

void CGeomorphicBoard::CombineLeftAndRight(CBitmap& bmap, TileScale eScale,
    const CBoardArray& pBALeft, const CBoardArray& pBARight, size_t nRowLeft, size_t nColLeft,
    size_t nRowRight, size_t nColRight) const
{
    CDC dc;
    dc.CreateCompatibleDC(NULL);
    CPalette* prvPal = dc.SelectPalette(GetAppPalette(), FALSE);
    dc.RealizePalette();

    CSize sizeCell = pBALeft.GetCellSize(eScale);
    CreateBitmap(bmap, sizeCell);

    // Handle left half...
    CBitmap* prvBMap = dc.SelectObject(&bmap);

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

    dc.SelectPalette(prvPal, FALSE);
    dc.SelectObject(prvBMap);
}

void CGeomorphicBoard::CombineTopAndBottom(CBitmap& bmap, TileScale eScale,
    const CBoardArray& pBATop, const CBoardArray& pBABottom, size_t nRowTop, size_t nColTop,
    size_t nRowBottom, size_t nColBottom) const
{
    CDC dc;
    dc.CreateCompatibleDC(NULL);
    CPalette* prvPal = dc.SelectPalette(GetAppPalette(), FALSE);
    dc.RealizePalette();

    CSize sizeCell = pBATop.GetCellSize(eScale);
    CreateBitmap(bmap, sizeCell);

    // Handle top half...
    CBitmap* prvBMap = dc.SelectObject(&bmap);

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

    dc.SelectPalette(prvPal, FALSE);
    dc.SelectObject(prvBMap);
}

// Create DIB Section bitmap that's filled with the transparent color.
void CGeomorphicBoard::CreateBitmap(CBitmap& m_bmap, CSize size) const
{
    CDC dc;
    dc.CreateCompatibleDC(NULL);
    CPalette* prvPal = dc.SelectPalette(GetAppPalette(), FALSE);
    dc.RealizePalette();

    m_bmap.Attach(Create16BitDIBSection(dc.m_hDC, size.cx, size.cy));
    CBitmap* prvBMap = dc.SelectObject(&m_bmap);

    CBrush brush(m_pDoc->GetTileManager()->GetTransparentColor());
    CBrush* prvBrush = dc.SelectObject(&brush);

    dc.PatBlt(0, 0, size.cx, size.cy, PATCOPY);

    dc.SelectObject(prvBrush);
    dc.SelectObject(prvBMap);
    dc.SelectPalette(prvPal, FALSE);
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
        const CGeoBoardElement& pBrd = GetBoardElt(size_t(0), nCol);
        const CBoardArray& pBArray = GetBoard(pBrd).GetBoardArray();
        ASSERT(pBArray.GetCols() >= size_t(1));
        rnCellCol += pBArray.GetCols();
        if (NeedsMerge(pBrd, Edge::Right))
        {
            --rnCellCol;
        }
    }
    for (size_t nRow = size_t(0) ; nRow < nBoardRow ; ++nRow)
    {
        const CGeoBoardElement& pBrd = GetBoardElt(nRow, size_t(0));
        const CBoardArray& pBArray = GetBoard(pBrd).GetBoardArray();
        ASSERT(pBArray.GetRows() >= size_t(1));
        rnCellRow += pBArray.GetRows();
        if (NeedsMerge(pBrd, Edge::Bottom))
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
    size_t nBrdIndex = m_pDoc->GetBoardManager()->FindBoardBySerial(geo.m_nBoardSerialNum);
    const CBoard& pBrd = m_pDoc->GetBoardManager()->GetBoard(nBrdIndex);
    return pBrd;
}

OwnerPtr<CBoard> CGeomorphicBoard::CloneBoard(const CBoard& pOrigBoard) const
{
    // We need to force the current version at this point because
    // we may be loading an earlier version game or scenario. In
    // this case the board data has already been upgraded. Make
    // a copy of the current loading version so we can restore it.
    CGamDoc::SetLoadingVersionGuard setLoadingVersionGuard(NumVersion(fileGsnVerMajor, fileGsnVerMinor));

    CMemFile file;
    CArchive arSave(&file, CArchive::store);
    /* save should not modify src, and restore is modifying new
        object, so const_cast should be safe */
    CGamDoc& doc = const_cast<CGamDoc&>(*m_pDoc);
    arSave.m_pDocument = &doc;
    // save should not modify src, so should be safe
    const_cast<CBoard&>(pOrigBoard).Serialize(arSave);      // Make a copy of the board
    arSave.Close();

    file.SeekToBegin();
    CArchive arRestore(&file, CArchive::load);
    arRestore.m_pDocument = &doc;
    OwnerPtr<CBoard> pNewBoard = new CBoard();
    pNewBoard->Serialize(arRestore);

    return pNewBoard;
}

/////////////////////////////////////////////////////////////////////////////

void CGeomorphicBoard::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_strName;

        ar << m_nSerialNum;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<WORD>(m_nBoardRowCount);
            ar << value_preserving_cast<WORD>(m_nBoardColCount);
        }
        else
        {
            CB::WriteCount(ar, m_nBoardRowCount);
            CB::WriteCount(ar, m_nBoardColCount);
        }

        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<WORD>(size());
        }
        else
        {
            CB::WriteCount(ar, value_preserving_cast<size_t>(size()));
        }
        for (size_t i = size_t(0); i < size(); ++i)
        {
            CGeoBoardElement& elt = (*this)[i];
            if (CB::GetVersion(ar) <= NumVersion(3, 90))
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
                    AfxThrowArchiveException(CArchiveException::badSchema);
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
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
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
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
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
                                    size() % GetBoardColCount());
             geo.Serialize(ar);
             push_back(geo);
        }
    }
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
