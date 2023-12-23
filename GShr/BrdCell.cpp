// BrdCell.cpp
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

#include    "stdafx.h"
#include    "WinExt.h"
#ifdef      GPLAY
    #include    "Gp.h"
    #include    "GamDoc.h"
#else
    #include    "Gm.h"
    #include    "GmDoc.h"
#endif
#include    "CellForm.h"
#include    "BrdCell.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef  _DEBUG
#define new DEBUG_NEW
#endif

CBoardArray::CBoardArray(CTileManager& tileMgr) :
    m_pTsa(&tileMgr)
{
    m_nRows = m_nCols = size_t(0);
    m_bTransparentCells = FALSE;
    m_bTrackCellNum = TRUE;
    m_eNumStyle = cnsRowCol;
    m_nRowTrkOffset = 0;
    m_nColTrkOffset = 0;
    m_bRowTrkInvert = 0;
    m_bColTrkInvert = 0;
    SetCellFrameColor(RGB(0, 0, 0));
    // ------ //
    m_wReserved1 = 0;
    m_wReserved2 = 0;
    m_wReserved3 = 0;
    m_wReserved4 = 0;
}

///////////////////////////////////////////////////////////////////////

void CBoardArray::CreateBoard(CellFormType eType, size_t nRows, size_t nCols,
    int nParm1, int nParm2, CellStagger nStagger)
{
    std::vector<BoardCell> pMap(nRows * nCols);

    GenerateBoard(eType, nRows, nCols, nParm1, nParm2, nStagger, std::move(pMap));
}

// ----------------------------------------------------- //

void CBoardArray::ReshapeBoard(size_t nRows, size_t nCols, int nParm1, int nParm2,
        CellStagger nStagger)
{
    ASSERT(!m_pMap.empty());

    // First allocate a new table and copy info from old map array to
    // new map array.
    std::vector<BoardCell> pMap(nRows * nCols);

    size_t maxRows = CB::min(m_nRows, nRows);
    size_t maxCols = CB::min(m_nCols, nCols);

    for (size_t r = size_t(0) ; r < maxRows ; ++r)
    {
        for (size_t c = size_t(0) ; c < maxCols ; ++c)
            pMap[r*nCols + c] = m_pMap[r*m_nCols + c];
    }

    // OK we've copied over the old data destroy the old and
    // use the new.

    if (nStagger == CellStagger::Invalid)
        nStagger = m_cfFull.GetCellStagger();

    CellFormType eType = m_cfFull.GetCellType();    // Get cell type

    if (nParm1 == -1)
    {
        CSize size = GetCellSize(fullScale);
        switch (eType)
        {
            case cformRect:
                nParm1 = size.cy;
                nParm2 = size.cx;
                break;
            case cformHexFlat:
            case cformHexPnt:
                // Only the first paramter is used for hex definitions.
                nParm1 = (eType == cformHexFlat) ? size.cy : size.cx;
                nParm2 = 0;
                break;
            default:
                AfxThrowInvalidArgException();
        }
    }

    GenerateBoard(eType, nRows, nCols, nParm1, nParm2, nStagger, std::move(pMap));
}

// ----------------------------------------------------- //

void CBoardArray::GenerateBoard(CellFormType eType, size_t nRows, size_t nCols,
    int nParm1, int nParm2, CellStagger nStagger, std::vector<BoardCell>&& pMap)
{
    DestroyBoard();             // Wipe out current board info.

    m_pMap = std::move(pMap);
    m_nRows = nRows;
    m_nCols = nCols;

    // Now set up cell geometry. There are 3 CCellForm objects. One
    // for each scaling. When scaling the view port, GetBoardScaling()
    // to get info for viewport and window extents.
    GenerateCellDefs(eType, nParm1, nParm2, nStagger, m_cfFull, m_cfHalf, m_cfSmall);
}

/* static */
void CBoardArray::GenerateCellDefs(CellFormType eType, int32_t nParm1, int32_t nParm2,
    CellStagger nStagger, CCellForm& cfFull, CCellForm& cfHalf, CCellForm& cfSmall)
{
    // Set up cell geometry. There are 3 CCellForm objects. One
    // for each scaling. When scaling the view port, GetBoardScaling()
    // to get info for viewport and window extents.

    cfFull.CreateCell(eType, nParm1, nParm2, nStagger);
    int nHalfP1 = (nParm1 / 2) + ((nParm1 & 1) != 0 ? 1 : 0);
    int nHalfP2 = (nParm2 / 2) + ((nParm2 & 1) != 0 ? 1 : 0);
    cfHalf.CreateCell(eType, CB::max(2, nHalfP1), CB::max(2, nHalfP2 ), nStagger);

    // Attempt to compute 1/8 scale. Rounds up to get dimensions.
    // Minimum cell size is two pixels.
    int nSmallP1 = (nParm1 / 8) + ((nParm1 % 8) != 0 ? 1 : 0);
    int nSmallP2 = (nParm2 / 8) + ((nParm2 % 8) != 0 ? 1 : 0);
    nSmallP1 = CB::max(nSmallP1, 2);
    nSmallP2 = CB::max(nSmallP2, 2);

    // Hexagonal grids are shown as brick grids in small scale.
    if (eType == cformHexFlat)
        cfSmall.CreateCell(cformBrickVert, nSmallP1, nSmallP1, nStagger);
    else if (eType == cformHexPnt)
        cfSmall.CreateCell(cformBrickHorz, nSmallP1, nSmallP1, nStagger);
    else
        cfSmall.CreateCell(eType, nSmallP1, nSmallP1, nStagger);
}

// ----------------------------------------------------- //

void CBoardArray::DestroyBoard()
{
    m_pMap.clear();
    m_cfFull.Clear();
    m_cfHalf.Clear();
    m_cfSmall.Clear();
}

// ----------------------------------------------------- //

void CBoardArray::GetBoardScaling(TileScale eScale, CSize& worldSize,
    CSize& viewSize) const
{
    worldSize = m_cfFull.CalcBoardSize(m_nRows, m_nCols);
    viewSize = GetCellForm(eScale).CalcBoardSize(m_nRows, m_nCols);
}

// ----------------------------------------------------- //
// Cell drawing assumes MM_TEXT mode. The selection of
// the appropriate CCellForm object will produce the
// appropriate scaling.

void CBoardArray::DrawCells(CDC& pDC, const CRect& pCellRct, TileScale eScale) const
{
    // Fill the cells with tile bitmaps or solid color or nothing
    for (size_t row = value_preserving_cast<size_t>(pCellRct.top) ; row <= value_preserving_cast<size_t>(pCellRct.bottom) ; ++row)
        for (size_t col = value_preserving_cast<size_t>(pCellRct.left) ; col <= value_preserving_cast<size_t>(pCellRct.right) ; ++col)
            FillCell(pDC, row, col, eScale);
}

// ----------------------------------------------------- //

void CBoardArray::DrawCellLines(CDC& pDC, const CRect& pCellRct, TileScale eScale) const
{
//  CPen* pPrvPen = (CPen*)pDC->SelectStockObject(BLACK_PEN);
    // drawing with a pen shouldn't modify it, so should be safe
    CPen* pPrvPen = pDC.SelectObject(const_cast<CPen*>(&m_pnCellFrame));
    for (size_t row = value_preserving_cast<size_t>(pCellRct.top) ; row <= value_preserving_cast<size_t>(pCellRct.bottom) ; ++row)
        for (size_t col = value_preserving_cast<size_t>(pCellRct.left) ; col <= value_preserving_cast<size_t>(pCellRct.right) ; ++col)
            FrameCell(pDC, row, col, eScale);
    pDC.SelectObject(pPrvPen);
}

// ----------------------------------------------------- //

BOOL CBoardArray::MapPixelsToCellBounds(const CRect& pPxlRct, CRect& pCellRct,
    TileScale eScale) const
{
    BOOL bOutOfRange;
    size_t row, col;
    bOutOfRange = !FindCell(pPxlRct.left, pPxlRct.top, row, col, eScale);
    pCellRct.top = value_preserving_cast<long>(row);
    pCellRct.left = value_preserving_cast<long>(col);
    bOutOfRange |= !FindCell(pPxlRct.right, pPxlRct.bottom, row, col, eScale);
    pCellRct.bottom = value_preserving_cast<long>(row);
    pCellRct.right = value_preserving_cast<long>(col);

    // Expand the rectangle for good measure.
    pCellRct.left--;
    if (pCellRct.left < 0) pCellRct.left = 0;
    pCellRct.right++;
    if (pCellRct.right >= value_preserving_cast<long>(m_nCols)) pCellRct.right = value_preserving_cast<long>(m_nCols - size_t(1));
    pCellRct.top--;
    if (pCellRct.top < 0) pCellRct.top = 0;
    pCellRct.bottom++;
    if (pCellRct.bottom >= value_preserving_cast<long>(m_nRows)) pCellRct.bottom = value_preserving_cast<long>(m_nRows - size_t(1));
    return !bOutOfRange;
}

// ----------------------------------------------------- //

void CBoardArray::FillCell(CDC& pDC, size_t row, size_t col, TileScale eScale) const
{
    const BoardCell& pCell = GetCell(row, col);
    const CCellForm& pCF = GetCellForm(eScale);

    CRect rct = pCF.GetRect(value_preserving_cast<CB::ssize_t>(row), value_preserving_cast<CB::ssize_t>(col));

    if (!pDC.RectVisible(&rct))
        return;

    if (pCell.IsTileID())
    {
        CTile tile = m_pTsa->GetTile(pCell.GetTID(), eScale);

        if (eScale != smallScale)
        {
            const CBitmap *pBMap = pCF.GetMask();
            if (pBMap != NULL && m_bTransparentCells)
                tile.TransBlt(pDC, rct.left, rct.top, pCF.GetMaskMemoryInfo());
            else if (pBMap != NULL)
            {
                g_gt.mDC1.SelectObject(*pBMap);

                pDC.SetBkColor(RGB(255, 255, 255));
                pDC.SetTextColor(RGB(0, 0, 0));

                tile.BitBlt(pDC, rct.left, rct.top, SRCINVERT);
                pDC.BitBlt(rct.left, rct.top, rct.right, rct.bottom,
                    &g_gt.mDC1, 0, 0, SRCAND);
                tile.BitBlt(pDC, rct.left, rct.top, SRCINVERT);
                g_gt.SelectSafeObjectsForDC1();
            }
            else
                tile.BitBlt(pDC, rct.left, rct.top, SRCCOPY);
        }
        else    // Handle small scale tiles here.
        {
            if (tile.GetSmallColor() == m_pTsa->GetTransparentColor())
                return;                 // Dont draw the cell
            CBrush oBrsh;
            oBrsh.CreateSolidBrush(tile.GetSmallColor());
            CBrush* pPrvBrsh = pDC.SelectObject(&oBrsh);
            pDC.PatBlt(rct.left, rct.top, rct.Width(), rct.Height(), PATCOPY);
            pDC.SelectObject(pPrvBrsh);
        }
    }
    else
    {
        CBrush oBrsh;
        if (pCell.GetColor() == noColor)
            return;                 // Let base color show through

        oBrsh.CreateSolidBrush(pCell.GetColor());
        CBrush* pPrvBrsh = pDC.SelectObject(&oBrsh);
        pCF.FillCell(pDC, rct.left, rct.top);
        pDC.SelectObject(pPrvBrsh);
    }
}

// ----------------------------------------------------- //

CRect CBoardArray::GetCellRect(size_t row, size_t col, TileScale eScale) const
{
    return GetCellForm(eScale).GetRect(value_preserving_cast<CB::ssize_t>(row), value_preserving_cast<CB::ssize_t>(col));
}

// ----------------------------------------------------- //

CSize CBoardArray::GetCellSize(TileScale eScale) const
{
    return GetCellForm(eScale).GetCellSize();
}

// ----------------------------------------------------- //

void CBoardArray::FrameCell(CDC& pDC, size_t row, size_t col, TileScale eScale) const
{
    const CCellForm& pCF = GetCellForm(eScale);
    CRect rct = pCF.GetRect(value_preserving_cast<CB::ssize_t>(row), value_preserving_cast<CB::ssize_t>(col));
    if (!pDC.RectVisible(&rct))
        return;
    pCF.FrameCell(pDC, rct.left, rct.top);
}

// ----------------------------------------------------- //

CSize CBoardArray::GetSize(TileScale eScale) const
{
    return GetCellForm(eScale).CalcBoardSize(m_nRows, m_nCols);
}

// ----------------------------------------------------- //

int CBoardArray::GetWidth(TileScale eScale) const
{
    CSize size = GetCellForm(eScale).CalcBoardSize(m_nRows, m_nCols);
    return size.cx;
}

// ----------------------------------------------------- //

int CBoardArray::GetHeight(TileScale eScale) const
{
    CSize size = GetCellForm(eScale).CalcBoardSize(m_nRows, m_nCols);
    return size.cy;
}

// ----------------------------------------------------- //

void CBoardArray::SetCellColor(size_t row, size_t col, COLORREF cr)
{
    size_t nCellIdx = CellIndex(row, col);
    m_pMap[nCellIdx].SetColor(cr);
}

// ----------------------------------------------------- //

void CBoardArray::SetCellTile(size_t row, size_t col, TileID tid)
{
    size_t nCellIdx = CellIndex(row, col);
    m_pMap[nCellIdx].SetTID(tid);
}

// ----------------------------------------------------- //

void CBoardArray::SetCellColorInRange(size_t rowBeg, size_t colBeg, size_t rowEnd,
    size_t colEnd, COLORREF cr)
{
    for (size_t r = rowBeg ; r <= rowEnd ; ++r)
        for (size_t c = colBeg ; c <= colEnd ; ++c)
            SetCellColor(r, c, cr);
}

// ----------------------------------------------------- //

const BoardCell& CBoardArray::GetCell(size_t row, size_t col) const
{
    ASSERT(!m_pMap.empty());
    return m_pMap[CellIndex(row, col)];
}

// ----------------------------------------------------- //
// X and Y is in pixels

BOOL CBoardArray::FindCell(long x, long y, size_t& rRow, size_t& rCol, TileScale eScale) const
{
    CB::ssize_t sRow, sCol;
    BOOL bRet = TRUE;
    GetCellForm(eScale).FindCell(x, y, sRow, sCol);
    // Make sure result is in bounds. Return FALSE if row and
    // column is forced to be in range.
    if (sRow < 0)
    {
        sRow = 0;
        bRet = FALSE;
    }
    else if (sRow >= value_preserving_cast<CB::ssize_t>(m_nRows))
    {
        sRow = value_preserving_cast<CB::ssize_t>(m_nRows) - 1;
        bRet = FALSE;
    }
    if (sCol < 0)
    {
        sCol = 0;
        bRet = FALSE;
    }
    else if (sCol >= value_preserving_cast<CB::ssize_t>(m_nCols))
    {
        sCol = value_preserving_cast<CB::ssize_t>(m_nCols) - 1;
        bRet = FALSE;
    }
    rRow = value_preserving_cast<size_t>(sRow);
    rCol = value_preserving_cast<size_t>(sCol);
    return bRet;
}

// ----------------------------------------------------- //

BOOL CBoardArray::PurgeMissingTileIDs()
{
    BOOL bPurged = FALSE;
    for (size_t r = size_t(0) ; r < m_nRows ; ++r)
    {
        for (size_t c = size_t(0) ; c < m_nCols ; ++c)
        {
            BoardCell& pCell = GetCell(r, c);
            if (pCell.IsTileID())
            {
                if (!m_pTsa->IsTileIDValid(pCell.GetTID()))
                {
                    pCell.Clear();
                    bPurged = TRUE;
                }
            }
        }
    }
    return bPurged;
}

// ----------------------------------------------------- //

BOOL CBoardArray::IsTileInUse(TileID tid) const
{
    for (size_t r = size_t(0) ; r < m_nRows ; ++r)
    {
        for (size_t c = size_t(0) ; c < m_nCols ; ++c)
        {
            const BoardCell& pCell = GetCell(r, c);
            if (pCell.IsTileID() && pCell.GetTID() == tid)
                return TRUE;
        }
    }
    return FALSE;
}

// ----------------------------------------------------- //

const CCellForm& CBoardArray::GetCellForm(TileScale eScale) const
{
    switch (eScale)
    {
        case fullScale: return m_cfFull;
        case halfScale: return m_cfHalf;
        default:        return m_cfSmall;
    }
}

///////////////////////////////////////////////////////////////////////////

void CBoardArray::SetCellFrameColor(COLORREF cr)
{
    m_pnCellFrame.DeleteObject();
    m_pnCellFrame.CreatePen(PS_SOLID, 0, cr);
    m_crCellFrame = cr;
}

///////////////////////////////////////////////////////////////////////////

CB::string CBoardArray::GetCellNumberStr(CPoint pnt, TileScale eScale) const
{
    size_t row, col;
    if (!FindCell(pnt.x, pnt.y, row, col, eScale))
        return "";
    return GetCellNumberStr(row, col);
}

CB::string CBoardArray::GetCellNumberStr(size_t row, size_t col) const
{
    if (row >= m_nRows || col >= m_nCols)
        return "";

    if (m_bRowTrkInvert)
        row = m_nRows - row - size_t(1);
    if (m_bColTrkInvert)
        col = m_nCols - col - size_t(1);

    // Only computer wonks start counting at zero...
    // max prevents negative values
    return CCellForm::GetCellNumberStr(m_eNumStyle, value_preserving_cast<size_t>(CB::max(value_preserving_cast<int>(row) + m_nRowTrkOffset + 1, 0)),
        value_preserving_cast<size_t>(CB::max(value_preserving_cast<int>(col) + m_nColTrkOffset + 1, 0)));
}

///////////////////////////////////////////////////////////////////////////

void BoardCell::Serialize(CArchive& ar)
{
    // file format <= 3.90
    union
    {
        COLORREF    m_crCell;   // 0xFFFFxxxx is TileID, 0xFF000000 is noColor
        struct TID
        {
            TileID16  m_tidCell;  // Typically terrain bitmap
            WORD    m_key;
        } tid;
    } temp;
    static_assert(sizeof(temp) == sizeof(DWORD), "file v3.90 mixup");
    static_assert(sizeof(temp.m_crCell) == sizeof(DWORD), "file v3.90 mixup");
    if (ar.IsStoring())
    {
        if (!CB::GetFeatures(ar).Check(ftrId32Bit))
        {
            if (IsTileID())
            {
                SerializeBackdoor sb;
                temp.tid.m_tidCell = SerializeBackdoor::Convert(m_tidCell);
                temp.tid.m_key = 0xFFFF;
            }
            else
            {
                temp.m_crCell = m_crCell;
            }
            ar << (DWORD)temp.m_crCell;
        }
        else
        {
            if (IsTileID())
            {
                ar << (uint8_t)1;
                ar << m_tidCell;
            }
            else
            {
                ar << (uint8_t)0;
                ar << m_crCell;
            }
        }
    }
    else
    {
        if (!CB::GetFeatures(ar).Check(ftrId32Bit))
        {
            DWORD dwTmp;
            ar >> dwTmp; temp.m_crCell = (COLORREF)dwTmp;
            if (temp.tid.m_key == 0xFFFF && temp.tid.m_tidCell != Invalid_v<TileID16>)
            {
                SerializeBackdoor sb;
                m_tidCell = SerializeBackdoor::Convert(temp.tid.m_tidCell);
                m_tile = true;
            }
            else
            {
                m_crCell = temp.m_crCell;
                m_tile = false;
            }
        }
        else
        {
            uint8_t isTileID;
            ar >> isTileID;
            if (isTileID)
            {
                ar >> m_tidCell;
                m_tile = true;
            }
            else
            {
                ar >> m_crCell;
                m_tile = false;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

BoardCell::BoardCell()
{
    Clear();
}

void BoardCell::Clear()
{
    m_tile = false;
    m_crCell = noColor;
}

BOOL BoardCell::IsTileID() const
{
    return m_tile && m_tidCell != nullTid;
}

BOOL BoardCell::IsEmpty() const
{
    return !m_tile && m_crCell == noColor;
}

TileID BoardCell::GetTID() const
{
    return m_tile ? m_tidCell : nullTid;
}

void BoardCell::SetTID(TileID id)
{
    m_tile = true;
    m_tidCell = id;
}

COLORREF BoardCell::GetColor() const
{
    ASSERT(!m_tile);
    return !m_tile ? m_crCell : noColor;
}

void BoardCell::SetColor(COLORREF cr)
{
    m_tile = false;
    m_crCell = cr;
}
#endif

///////////////////////////////////////////////////////////////////////////

void CBoardArray::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_wReserved1;
        ar << m_wReserved2;
        ar << m_wReserved3;
        ar << m_wReserved4;
        ar << value_preserving_cast<int16_t>(m_nRowTrkOffset);
        ar << value_preserving_cast<int16_t>(m_nColTrkOffset);
        ar << (WORD)m_bRowTrkInvert;
        ar << (WORD)m_bColTrkInvert;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<WORD>(m_nRows);
            ar << value_preserving_cast<WORD>(m_nCols);
        }
        else
        {
            CB::WriteCount(ar, m_nRows);
            CB::WriteCount(ar, m_nCols);
        }
        ar << (WORD)m_bTransparentCells;
        ar << (WORD)m_eNumStyle;
        ar << (WORD)m_bTrackCellNum;
        ar << (DWORD)m_crCellFrame;
    }
    else
    {
        WORD wVal;
        int16_t iVal;
        DestroyBoard();

        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;
        ar >> iVal; m_nRowTrkOffset = iVal;  // (was int cast)
        ar >> iVal; m_nColTrkOffset = iVal;  // (was int cast)
        ar >> wVal; m_bRowTrkInvert = (BOOL)wVal;
        ar >> wVal; m_bColTrkInvert = (BOOL)wVal;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar >> wVal; m_nRows = wVal;
            ar >> wVal; m_nCols = wVal;
        }
        else
        {
            m_nRows = CB::ReadCount(ar);
            m_nCols = CB::ReadCount(ar);
        }
        ar >> wVal; m_bTransparentCells = (BOOL)wVal;
        ar >> wVal; m_eNumStyle = static_cast<CellNumStyle>(wVal);
        ar >> wVal; m_bTrackCellNum = (BOOL)wVal;

#ifdef GPLAY
        // Version handling unique for game box loading
        if (CGameBox::GetLoadingVersion() > NumVersion(0, 51))
#else
        if (CGamDoc::GetLoadingVersion() > NumVersion(0, 51))
#endif
        {
            DWORD dwTmp;
            ar >> dwTmp;
            SetCellFrameColor((COLORREF)dwTmp);
        }

        m_pMap.resize(m_nRows * m_nCols);
    }
    m_cfFull.Serialize(ar);
    m_cfHalf.Serialize(ar);
    m_cfSmall.Serialize(ar);

    for (size_t r = size_t(0) ; r < m_nRows ; ++r)
    {
        for (size_t c = size_t(0) ; c < m_nCols ; ++c)
            GetCell(r, c).Serialize(ar);
    }
}


