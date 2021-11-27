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

CGeoBoardElement::CGeoBoardElement()
{
    m_nBoardSerialNum = BoardID(0);
}

CGeoBoardElement::CGeoBoardElement(CGeoBoardElement& geo)
{
    m_nBoardSerialNum = geo.m_nBoardSerialNum;
}

CGeoBoardElement::CGeoBoardElement(BoardID nBoardSerialNum)
{
    m_nBoardSerialNum = nBoardSerialNum;
}

/////////////////////////////////////////////////////////////////////////////

void CGeoBoardElement::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_nBoardSerialNum;
    }
    else
    {
        ar >> m_nBoardSerialNum;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CGeomorphicBoard::CGeomorphicBoard()
{
    m_nBoardRowCount = size_t(0);
    m_nBoardColCount = size_t(0);
    m_nSerialNum = BoardID(0);
    m_pDoc = NULL;
}

CGeomorphicBoard::CGeomorphicBoard(const CGeomorphicBoard& pGeoBoard)
{
    m_strName = pGeoBoard.m_strName;
    m_nSerialNum = pGeoBoard.m_nSerialNum;
    m_nBoardRowCount = pGeoBoard.m_nBoardRowCount;
    m_nBoardColCount = pGeoBoard.m_nBoardColCount;
    m_pDoc = pGeoBoard.m_pDoc;

    RemoveAll();
    for (int i = 0; i < pGeoBoard.GetSize(); i++)
         Add(pGeoBoard.GetAt(i));
}

/////////////////////////////////////////////////////////////////////////////

intptr_t CGeomorphicBoard::AddElement(BoardID nBoardSerialNum)
{
    CGeoBoardElement geo(nBoardSerialNum);
    return Add(geo);
}

/////////////////////////////////////////////////////////////////////////////

CBoard* CGeomorphicBoard::CreateBoard(CGamDoc* pDoc)
{
    m_pDoc = pDoc;
    ASSERT(pDoc != NULL);

    // Procedure:
    // 1) Clone the root board to use as template

    CBoard& pBrd = GetBoard(size_t(0), size_t(0));
    CBoard* pBrdNew = CloneBoard(pBrd);

    pBrdNew->SetName(m_strName);
    pBrdNew->SetSerialNumber(m_nSerialNum);

    // 2) Reshape to size new master board.

    CBoardArray* pBrdArrayNew = pBrdNew->GetBoardArray();

    size_t nRows;
    size_t nCols;
    ComputeNewBoardDimensions(nRows, nCols);

    pBrdArrayNew->ReshapeBoard(nRows, nCols, -1, -1, -1);

    // 3) Fill cells with content of cells from other boards.
    for (size_t nBoardRow = size_t(0) ; nBoardRow < m_nBoardRowCount ; ++nBoardRow)
    {
        for (size_t nBoardCol = size_t(0) ; nBoardCol < m_nBoardColCount ; ++nBoardCol)
        {
            if (nBoardCol == size_t(0) && nBoardRow == size_t(0))
                continue;                       // Skip this one since it is done already
            size_t nCellRowOffset;
            size_t nCellColOffset;
            ComputeCellOffset(nBoardRow, nBoardCol, nCellRowOffset, nCellColOffset);

            CBoard& pBrd = GetBoard(nBoardRow, nBoardCol);
            CBoardArray* pBArray = pBrd.GetBoardArray();

            CopyCells(pBrdArrayNew, pBArray, nCellRowOffset, nCellColOffset);
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

            CBoard& pBrd = GetBoard(nBoardRow, nBoardCol);

            CDrawList* pDwgList = pBrd.GetBaseDrawing();
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

size_t CGeomorphicBoard::GetSpecialTileSet()
{
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    size_t nTSet = pTMgr->FindNamedTileSet(GEOTILESET_NAME);
    if (nTSet == Invalid_v<size_t>)
        nTSet = pTMgr->CreateTileSet(GEOTILESET_NAME);
    ASSERT(nTSet != Invalid_v<size_t>);
    return nTSet;
}

void CGeomorphicBoard::CopyCells(CBoardArray* pBArryTo, CBoardArray* pBArryFrom,
    size_t nCellRowOffset, size_t nCellColOffset)
{
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    for (size_t nRow = size_t(0) ; nRow < pBArryFrom->GetRows() ; ++nRow)
    {
        for (size_t nCol = size_t(0) ; nCol < pBArryFrom->GetCols() ; ++nCol)
        {
            size_t nRowTo = nRow + nCellRowOffset;
            size_t nColTo = nCol + nCellColOffset;

            BoardCell* pCellFrom = pBArryFrom->GetCell(nRow, nCol);
            BoardCell* pCellTo = pBArryTo->GetCell(nRowTo, nColTo);
            BOOL bMergeTheCell = FALSE;
            if (pBArryTo->GetCellForm(fullScale)->GetCellType() == cformHexFlat)
                bMergeTheCell = nCol == size_t(0) && nColTo != size_t(0);
            else
                bMergeTheCell = nRow == size_t(0) && nRowTo != size_t(0);

            if (bMergeTheCell && memcmp(pCellFrom, pCellTo, sizeof(BoardCell)) != 0)
            {
                // We need to create a tile that has the half images of the
                // two hex cells joined as one.
                CBitmap bmapFull;
                CBitmap bmapHalf;
                if (pBArryTo->GetCellForm(fullScale)->GetCellType() == cformHexFlat)
                {
                    // Handle left and right halves
                    CombineLeftAndRight(bmapFull, fullScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                    CombineLeftAndRight(bmapHalf, halfScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                }
                else
                {
                    // Handle top and bottom halves
                    CombineTopAndBottom(bmapFull, fullScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                    CombineTopAndBottom(bmapHalf, halfScale, pBArryTo, pBArryFrom,
                        nRowTo, nColTo, nRow, nCol);
                }
                // Determine small cell color. Use the 'To' cell as value;
                COLORREF crSmall;
                if (pCellTo->IsTileID())
                {
                    CTile tile;
                    pTMgr->GetTile(pCellTo->GetTID(), &tile, smallScale);
                    crSmall = tile.GetSmallColor();
                }
                else
                    crSmall = pCellTo->GetColor();
                CSize sizeFull = pBArryTo->GetCellSize(fullScale);
                CSize sizeHalf = pBArryTo->GetCellSize(halfScale);
                TileID tidNew = pTMgr->CreateTile(GetSpecialTileSet(),
                    sizeFull, sizeHalf, crSmall);
                pTMgr->UpdateTile(tidNew, &bmapFull, &bmapHalf, crSmall);
                pCellTo->SetTID(tidNew);
            }
            else
                memcpy(pCellTo, pCellFrom, sizeof(BoardCell));
        }
    }
}

void CGeomorphicBoard::CombineLeftAndRight(CBitmap& bmap, TileScale eScale,
    CBoardArray* pBALeft, CBoardArray* pBARight, size_t nRowLeft, size_t nColLeft,
    size_t nRowRight, size_t nColRight)
{
    CDC dc;
    dc.CreateCompatibleDC(NULL);
    CPalette* prvPal = dc.SelectPalette(GetAppPalette(), FALSE);
    dc.RealizePalette();

    CSize sizeCell = pBALeft->GetCellSize(eScale);
    CreateBitmap(bmap, sizeCell);

    // Handle left half...
    CBitmap* prvBMap = dc.SelectObject(&bmap);

    CRect rct;
    pBALeft->GetCellRect(nRowLeft, nColLeft, &rct, eScale);

    dc.SetViewportOrg(-rct.left, -rct.top);
    rct.right -= rct.Width() / 2 - (rct.Width() % 2);
    dc.IntersectClipRect(&rct);

    pBALeft->FillCell(&dc, nRowLeft, nColLeft, eScale);
    dc.SelectClipRgn(NULL);

    // Handle right half...
    pBARight->GetCellRect(nRowRight, nColRight, &rct, eScale);

    dc.SetViewportOrg(-rct.left, -rct.top);
    rct.left += rct.Width() / 2 + (rct.Width() % 2);
    dc.IntersectClipRect(&rct);

    pBARight->FillCell(&dc, nRowRight, nColRight, eScale);

    dc.SelectPalette(prvPal, FALSE);
    dc.SelectObject(prvBMap);
}

void CGeomorphicBoard::CombineTopAndBottom(CBitmap& bmap, TileScale eScale,
    CBoardArray* pBATop, CBoardArray* pBABottom, size_t nRowTop, size_t nColTop,
    size_t nRowBottom, size_t nColBottom)
{
    CDC dc;
    dc.CreateCompatibleDC(NULL);
    CPalette* prvPal = dc.SelectPalette(GetAppPalette(), FALSE);
    dc.RealizePalette();

    CSize sizeCell = pBATop->GetCellSize(eScale);
    CreateBitmap(bmap, sizeCell);

    // Handle top half...
    CBitmap* prvBMap = dc.SelectObject(&bmap);

    CRect rct;
    pBATop->GetCellRect(nRowTop, nColTop, &rct, eScale);

    dc.SetViewportOrg(-rct.left, -rct.top);
    rct.bottom += rct.Height() / 2 - (rct.Height() % 2);
    dc.IntersectClipRect(&rct);

    pBATop->FillCell(&dc, nRowTop, nColTop, eScale);
    dc.SelectClipRgn(NULL);

    // Handle bottom half...
    pBABottom->GetCellRect(nRowBottom, nColBottom, &rct, eScale);

    dc.SetViewportOrg(-rct.left, -rct.top);
    rct.top += rct.Height() / 2 + (rct.Height() % 2);
    dc.IntersectClipRect(&rct);

    pBABottom->FillCell(&dc, nRowBottom, nColBottom, eScale);

    dc.SelectPalette(prvPal, FALSE);
    dc.SelectObject(prvBMap);
}

// Create DIB Section bitmap that's filled with the transparent color.
void CGeomorphicBoard::CreateBitmap(CBitmap& m_bmap, CSize size)
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

CPoint CGeomorphicBoard::ComputeGraphicalOffset(size_t nBoardRow, size_t nBoardCol)
{
    CBoard& pBrdRoot = GetBoard(size_t(0), size_t(0));
    CSize sizeCell = pBrdRoot.GetBoardArray()->GetCellSize(fullScale);
    CPoint pnt(0, 0);
    for (size_t nCol = size_t(0) ; nCol < nBoardCol ; ++nCol)
    {
        CBoard& pBrd = GetBoard(size_t(0), nCol);
        CBoardArray* pBArray = pBrd.GetBoardArray();
        pnt.x += pBArray->GetWidth(fullScale);
        if (nBoardCol != size_t(0) && pBArray->GetCellForm(fullScale)->GetCellType() == cformHexFlat)
            pnt.x -= sizeCell.cx;               // A column is shared
        else
            pnt.x -= sizeCell.cx / 2;           // A column is half shared
    }
    for (size_t nRow = size_t(0) ; nRow < nBoardRow ; ++nRow)
    {
        CBoard& pBrd = GetBoard(nRow, size_t(0));
        CBoardArray* pBArray = pBrd.GetBoardArray();
        pnt.y += pBArray->GetHeight(fullScale);
        if (nBoardRow != size_t(0) && pBArray->GetCellForm(fullScale)->GetCellType() == cformHexPnt)
            pnt.y -= sizeCell.cy;               // A row is shared
        else
            pnt.y -= sizeCell.cy / 2;           // A row is half shared

    }
    return pnt;
}

void CGeomorphicBoard::ComputeNewBoardDimensions(size_t& rnRows, size_t& rnCols)
{
    rnRows = size_t(0);
    rnCols = size_t(0);
    for (size_t nBoardCol = size_t(0) ; nBoardCol < m_nBoardColCount ; ++nBoardCol)
    {
        CBoard& pBrd = GetBoard(size_t(0), nBoardCol);
        CBoardArray* pBArray = pBrd.GetBoardArray();
        rnCols += pBArray->GetCols();
        if (nBoardCol != size_t(0) && pBArray->GetCellForm(fullScale)->GetCellType() == cformHexFlat)
            rnCols--;                   // A column is shared
    }
    for (size_t nBoardRow = size_t(0) ; nBoardRow < m_nBoardRowCount ; ++nBoardRow)
    {
        CBoard& pBrd = GetBoard(nBoardRow, size_t(0));
        CBoardArray* pBArray = pBrd.GetBoardArray();
        rnRows += pBArray->GetRows();
        if (nBoardRow != size_t(0) && pBArray->GetCellForm(fullScale)->GetCellType() == cformHexPnt)
            rnRows--;                   // A row is shared
    }
}

void CGeomorphicBoard::ComputeCellOffset(size_t nBoardRow, size_t nBoardCol,
    size_t& rnCellRow, size_t& rnCellCol)
{
    rnCellRow = size_t(0);
    rnCellCol = size_t(0);
    for (size_t nCol = size_t(0) ; nCol < nBoardCol ; ++nCol)
    {
        CBoard& pBrd = GetBoard(size_t(0), nCol);
        CBoardArray* pBArray = pBrd.GetBoardArray();
        ASSERT(pBArray->GetCols() >= size_t(1));
        rnCellCol += pBArray->GetCols();
        if (pBArray->GetCellForm(fullScale)->GetCellType() == cformHexFlat)
            rnCellCol--;
    }
    for (size_t nRow = size_t(0) ; nRow < nBoardRow ; ++nRow)
    {
        CBoard& pBrd = GetBoard(nRow, size_t(0));
        CBoardArray* pBArray = pBrd.GetBoardArray();
        ASSERT(pBArray->GetRows() >= size_t(1));
        rnCellRow += pBArray->GetRows();
        if (pBArray->GetCellForm(fullScale)->GetCellType() == cformHexPnt)
            rnCellRow--;
    }
}

CBoard& CGeomorphicBoard::GetBoard(size_t nBoardRow, size_t nBoardCol)
{
    size_t nGeoIndex = nBoardRow * m_nBoardColCount + nBoardCol;
    ASSERT(nGeoIndex < m_nBoardRowCount * m_nBoardColCount);
    CGeoBoardElement geo = GetAt(value_preserving_cast<intptr_t>(nGeoIndex));
    size_t nBrdIndex = m_pDoc->GetBoardManager()->FindBoardBySerial(geo.m_nBoardSerialNum);
    CBoard& pBrd = m_pDoc->GetBoardManager()->GetBoard(nBrdIndex);
    return pBrd;
}

CBoard* CGeomorphicBoard::CloneBoard(CBoard& pOrigBoard)
{
    // We need to force the current version at this point because
    // we may be loading an earlier version game or scenario. In
    // this case the board data has already been upgraded. Make
    // a copy of the current loading version so we can restore it.
    CGamDoc::SetLoadingVersionGuard setLoadingVersionGuard(NumVersion(fileGsnVerMajor, fileGsnVerMinor));

    CBoard* pNewBoard = NULL;
    TRY
    {
        CMemFile file;
        CArchive arSave(&file, CArchive::store);
        arSave.m_pDocument = m_pDoc;
        pOrigBoard.Serialize(arSave);      // Make a copy of the board
        arSave.Close();

        file.SeekToBegin();
        CArchive arRestore(&file, CArchive::load);
        arRestore.m_pDocument = m_pDoc;
        pNewBoard = new CBoard();
        pNewBoard->Serialize(arRestore);
    }
    CATCH_ALL(e)
    {
        if (pNewBoard != NULL)
            delete pNewBoard;
        pNewBoard = NULL;
    }
    END_CATCH_ALL

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
            ar << value_preserving_cast<WORD>(GetSize());
        }
        else
        {
            CB::WriteCount(ar, value_preserving_cast<size_t>(GetSize()));
        }
        for (int i = 0; i < GetSize(); i++)
            ElementAt(i).Serialize(ar);
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

        RemoveAll();
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
             CGeoBoardElement geo;
             geo.Serialize(ar);
             Add(geo);
        }
    }
}
