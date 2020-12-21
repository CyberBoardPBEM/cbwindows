// BrdCell.cpp
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

CBoardArray::CBoardArray()
{
    m_nRows = m_nCols = 0;
    m_pMap = NULL;
    m_pTsa = NULL;
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

void CBoardArray::CreateBoard(CellFormType eType, int nRows, int nCols,
    int nParm1, int nParm2, int nStagger)
{
    BoardCell* pMap = (BoardCell*)GlobalAllocPtr(GHND,
        (DWORD)sizeof(BoardCell) * nRows * nCols);
    if (pMap == NULL)
        AfxThrowMemoryException();

    for (int i = 0; i < nRows * nCols; i++)
        pMap[i].Clear();

    GenerateBoard(eType, nRows, nCols, nParm1, nParm2, nStagger, pMap);
}

// ----------------------------------------------------- //

void CBoardArray::ReshapeBoard(int nRows, int nCols, int nParm1, int nParm2,
        int nStagger)
{
    ASSERT(m_pMap != NULL);

    // First allocate a new table and copy info from old map array to
    // new map array.
    BoardCell* pMap = (BoardCell*)GlobalAllocPtr(GHND,
        (DWORD)sizeof(BoardCell) * nRows * nCols);
    if (pMap == NULL)
        AfxThrowMemoryException();
    for (int i = 0; i < nRows * nCols; i++)
        pMap[i].Clear();

    int maxRows = CB::min(m_nRows, nRows);
    int maxCols = CB::min(m_nCols, nCols);

    for (int r = 0; r < maxRows; r++)
    {
        for (int c = 0; c < maxCols; c++)
            pMap[r*nCols + c] = m_pMap[r*m_nCols + c];
    }

    // OK we've copied over the old data destroy the old and
    // use the new.

    if (nStagger == -1)
        nStagger = m_cfFull.GetCellStagger();

    CellFormType eType = m_cfFull.GetCellType();    // Get cell type

    if (nParm1 == -1)
    {
        CSize size = GetCellSize(fullScale);
        // Only the first paramter is used for hex definitions.
        nParm1 = (eType == cformHexFlat) ? size.cy : size.cx;
        nParm2 = 0;
    }

    GenerateBoard(eType, nRows, nCols, nParm1, nParm2, nStagger, pMap);
}

// ----------------------------------------------------- //

void CBoardArray::GenerateBoard(CellFormType eType, int nRows, int nCols,
    int nParm1, int nParm2, int nStagger, BoardCell* pMap)
{
    DestroyBoard();             // Wipe out current board info.

    m_pMap = pMap;
    m_nRows = nRows;
    m_nCols = nCols;

    // Now set up cell geometry. There are 3 CCellForm objects. One
    // for each scaling. When scaling the view port, GetBoardScaling()
    // to get info for viewport and window extents.
    GenerateCellDefs(eType, nParm1, nParm2, nStagger, m_cfFull, m_cfHalf, m_cfSmall);
}

/* static */
void CBoardArray::GenerateCellDefs(CellFormType eType, int nParm1, int nParm2,
    int nStagger, CCellForm& cfFull, CCellForm& cfHalf, CCellForm& cfSmall)
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
    if (m_pMap)
        GlobalFreePtr(m_pMap);
    m_pMap = NULL;
    m_cfFull.Clear();
    m_cfHalf.Clear();
    m_cfSmall.Clear();
}

// ----------------------------------------------------- //

void CBoardArray::GetBoardScaling(TileScale eScale, CSize& worldSize,
    CSize& viewSize)
{
    worldSize = m_cfFull.CalcBoardSize(m_nRows, m_nCols);
    viewSize = GetCellForm(eScale)->CalcBoardSize(m_nRows, m_nCols);
}

// ----------------------------------------------------- //
// Cell drawing assumes MM_TEXT mode. The selection of
// the appropriate CCellForm object will produce the
// appropriate scaling.

void CBoardArray::DrawCells(CDC* pDC, CRect* pCellRct, TileScale eScale)
{
    // Fill the cells with tile bitmaps or solid color or nothing
    for (int row = pCellRct->top; row <= pCellRct->bottom; row++)
        for (int col = pCellRct->left; col <= pCellRct->right; col++)
            FillCell(pDC, row, col, eScale);
}

// ----------------------------------------------------- //

void CBoardArray::DrawCellLines(CDC* pDC, CRect* pCellRct, TileScale eScale)
{
//  CPen* pPrvPen = (CPen*)pDC->SelectStockObject(BLACK_PEN);
    CPen* pPrvPen = pDC->SelectObject(&m_pnCellFrame);
    for (int row = pCellRct->top; row <= pCellRct->bottom; row++)
        for (int col = pCellRct->left; col <= pCellRct->right; col++)
            FrameCell(pDC, row, col, eScale);
    pDC->SelectObject(pPrvPen);
}

// ----------------------------------------------------- //

BOOL CBoardArray::MapPixelsToCellBounds(CRect* pPxlRct, CRect* pCellRct,
    TileScale eScale)
{
    BOOL bOutOfRange;
    int row, col;
    bOutOfRange = !FindCell(pPxlRct->left, pPxlRct->top, row, col, eScale);
    pCellRct->top = row;
    pCellRct->left = col;
    bOutOfRange |= !FindCell(pPxlRct->right, pPxlRct->bottom, row, col, eScale);
    pCellRct->bottom = row;
    pCellRct->right = col;

    // Expand the rectangle for good measure.
    pCellRct->left--;
    if (pCellRct->left < 0) pCellRct->left = 0;
    pCellRct->right++;
    if (pCellRct->right >= m_nCols) pCellRct->right = m_nCols - 1;
    pCellRct->top--;
    if (pCellRct->top < 0) pCellRct->top = 0;
    pCellRct->bottom++;
    if (pCellRct->bottom >= m_nRows) pCellRct->bottom = m_nRows - 1;
    return !bOutOfRange;
}

// ----------------------------------------------------- //

void CBoardArray::FillCell(CDC *pDC, int row, int col, TileScale eScale)
{
    BoardCell* pCell = GetCell(row, col);
    CCellForm* pCF = GetCellForm(eScale);

    CRect rct;
    pCF->GetRect(row, col, &rct);

    if (!pDC->RectVisible(&rct))
        return;

    if (pCell->IsTileID())
    {
        CTile tile;
        m_pTsa->GetTile(pCell->GetTID(), &tile, eScale);

        if (eScale != smallScale)
        {
            CBitmap *pBMap = pCF->GetMask();
            if (pBMap != NULL && m_bTransparentCells)
                tile.TransBlt(pDC, rct.left, rct.top, pCF->GetMaskMemoryInfo());
            else if (pBMap != NULL)
            {
                g_gt.mDC1.SelectObject(pBMap);

                pDC->SetBkColor(RGB(255, 255, 255));
                pDC->SetTextColor(RGB(0, 0, 0));

                tile.BitBlt(pDC, rct.left, rct.top, SRCINVERT);
                pDC->BitBlt(rct.left, rct.top, rct.right, rct.bottom,
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
            CBrush* pPrvBrsh = pDC->SelectObject(&oBrsh);
            pDC->PatBlt(rct.left, rct.top, rct.Width(), rct.Height(), PATCOPY);
            pDC->SelectObject(pPrvBrsh);
        }
    }
    else
    {
        CBrush oBrsh;
        if (pCell->GetColor() == noColor)
            return;                 // Let base color show through

        oBrsh.CreateSolidBrush(pCell->GetColor());
        CBrush* pPrvBrsh = pDC->SelectObject(&oBrsh);
        pCF->FillCell(pDC, rct.left, rct.top);
        pDC->SelectObject(pPrvBrsh);
    }
}

// ----------------------------------------------------- //

void CBoardArray::GetCellRect(int row, int col, CRect* pRct, TileScale eScale)
{
    GetCellForm(eScale)->GetRect(row, col, pRct);
}

// ----------------------------------------------------- //

CSize CBoardArray::GetCellSize(TileScale eScale)
{
    return GetCellForm(eScale)->GetCellSize();
}

// ----------------------------------------------------- //

void CBoardArray::FrameCell(CDC *pDC, int row, int col, TileScale eScale)
{
    BoardCell* pCell = GetCell(row, col);
    CRect rct;
    CCellForm *pCF = GetCellForm(eScale);
    pCF->GetRect(row, col, &rct);
    if (!pDC->RectVisible(&rct))
        return;
    pCF->FrameCell(pDC, rct.left, rct.top);
}

// ----------------------------------------------------- //

CSize CBoardArray::GetSize(TileScale eScale)
{
    return GetCellForm(eScale)->CalcBoardSize(m_nRows, m_nCols);
}

// ----------------------------------------------------- //

int CBoardArray::GetWidth(TileScale eScale)
{
    CSize size = GetCellForm(eScale)->CalcBoardSize(m_nRows, m_nCols);
    return size.cx;
}

// ----------------------------------------------------- //

int CBoardArray::GetHeight(TileScale eScale)
{
    CSize size = GetCellForm(eScale)->CalcBoardSize(m_nRows, m_nCols);
    return size.cy;
}

// ----------------------------------------------------- //

void CBoardArray::SetCellColor(int row, int col, COLORREF cr)
{
    int nCellIdx = CellIndex(row, col);
    m_pMap[nCellIdx].SetColor(cr);
}

// ----------------------------------------------------- //

void CBoardArray::SetCellTile(int row, int col, TileID tid)
{
    int nCellIdx = CellIndex(row, col);
    m_pMap[nCellIdx].SetTID(tid);
}

// ----------------------------------------------------- //

void CBoardArray::SetCellColorInRange(int rowBeg, int colBeg, int rowEnd,
    int colEnd, COLORREF cr)
{
    for (int r = rowBeg; r <= rowEnd; r++)
        for (int c = colBeg; c <= colEnd; c++)
            SetCellColor(r, c, cr);
}

// ----------------------------------------------------- //

BoardCell* CBoardArray::GetCell(int row, int col)
{
    ASSERT(m_pMap != NULL);
    return &m_pMap[CellIndex(row, col)];
}

// ----------------------------------------------------- //
// X and Y is in pixels

BOOL CBoardArray::FindCell(int x, int y, int& rRow, int& rCol, TileScale eScale)
{
    BOOL bRet = TRUE;
    GetCellForm(eScale)->FindCell(x, y, rRow, rCol);
    // Make sure result is in bounds. Return FALSE if row and
    // column is forced to be in range.
    if (rRow < 0)
    {
        rRow = 0;
        bRet = FALSE;
    }
    else if (rRow >= m_nRows)
    {
        rRow = m_nRows - 1;
        bRet = FALSE;
    }
    if (rCol < 0)
    {
        rCol = 0;
        bRet = FALSE;
    }
    else if (rCol >= m_nCols)
    {
        rCol = m_nCols - 1;
        bRet = FALSE;
    }
    return bRet;
}

// ----------------------------------------------------- //

BOOL CBoardArray::PurgeMissingTileIDs()
{
    BOOL bPurged = FALSE;
    for (int r = 0; r < m_nRows; r++)
    {
        for (int c = 0; c < m_nCols; c++)
        {
            BoardCell *pCell = GetCell(r, c);
            if (pCell->IsTileID())
            {
                if (!m_pTsa->IsTileIDValid(pCell->GetTID()))
                {
                    pCell->Clear();
                    bPurged = TRUE;
                }
            }
        }
    }
    return bPurged;
}

// ----------------------------------------------------- //

BOOL CBoardArray::IsTileInUse(TileID tid)
{
    for (int r = 0; r < m_nRows; r++)
    {
        for (int c = 0; c < m_nCols; c++)
        {
            BoardCell *pCell = GetCell(r, c);
            if (pCell->IsTileID() && pCell->GetTID() == tid)
                return TRUE;
        }
    }
    return FALSE;
}

// ----------------------------------------------------- //

CCellForm* CBoardArray::GetCellForm(TileScale eScale)
{
    switch (eScale)
    {
        case fullScale: return &m_cfFull;
        case halfScale: return &m_cfHalf;
        default:        return &m_cfSmall;
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

void CBoardArray::GetCellNumberStr(CPoint pnt, CString& str, TileScale eScale)
{
    int row, col;
    str.Empty();
    if (!FindCell(pnt.x, pnt.y, row, col, eScale))
        return;
    GetCellNumberStr(row, col, str);
}

void CBoardArray::GetCellNumberStr(int row, int col, CString& str)
{
    str.Empty();
    if (row < 0 || row >= m_nRows || col < 0 || col >= m_nCols)
        return;

    if (m_bRowTrkInvert)
        row = m_nRows - row - 1;
    if (m_bColTrkInvert)
        col = m_nCols - col - 1;

    // Only computer wonks start counting at zero...
    CCellForm::GetCellNumberStr(m_eNumStyle, row + m_nRowTrkOffset + 1,
        col + m_nColTrkOffset + 1, str);
}

///////////////////////////////////////////////////////////////////////////

void BoardCell::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
        ar << (DWORD)m_crCell;
    else
    {
        DWORD dwTmp;
        ar >> dwTmp; m_crCell = (COLORREF)dwTmp;
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
    m_crCell = noColor;
}

BOOL BoardCell::IsTileID()
{
    return tid.m_key == 0xFFFF && tid.m_tidCell != nullTid;
}

BOOL BoardCell::IsEmpty()
{
    return m_crCell == noColor;
}

TileID BoardCell::GetTID()
{
    return tid.m_key == 0xFFFF ? tid.m_tidCell : nullTid;
}

void BoardCell::SetTID(TileID id)
{
    tid.m_tidCell = id;
    tid.m_key = 0xFFFF;
}

COLORREF BoardCell::GetColor()
{
    return m_crCell;
}

void BoardCell::SetColor(COLORREF cr)
{
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
        ar << (WORD)m_nRowTrkOffset;
        ar << (WORD)m_nColTrkOffset;
        ar << (WORD)m_bRowTrkInvert;
        ar << (WORD)m_bColTrkInvert;
        ar << (WORD)m_nRows;
        ar << (WORD)m_nCols;
        ar << (WORD)m_bTransparentCells;
        ar << (WORD)m_eNumStyle;
        ar << (WORD)m_bTrackCellNum;
        ar << (DWORD)m_crCellFrame;
    }
    else
    {
        WORD wVal;
        DestroyBoard();

        m_pTsa = ((CGamDoc*)ar.m_pDocument)->GetTileManager();

        ar >> m_wReserved1;
        ar >> m_wReserved2;
        ar >> m_wReserved3;
        ar >> m_wReserved4;
        ar >> wVal; m_nRowTrkOffset = (short)wVal;  // (was int cast)
        ar >> wVal; m_nColTrkOffset = (short)wVal;  // (was int cast)
        ar >> wVal; m_bRowTrkInvert = (BOOL)wVal;
        ar >> wVal; m_bColTrkInvert = (BOOL)wVal;
        ar >> wVal; m_nRows = (int)wVal;
        ar >> wVal; m_nCols = (int)wVal;
        ar >> wVal; m_bTransparentCells = (BOOL)wVal;
        ar >> wVal; m_eNumStyle = (CellNumStyle)wVal;
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

        m_pMap = (BoardCell*)GlobalAllocPtr(GHND,
            (DWORD)sizeof(BoardCell) * m_nRows * m_nCols);

        if (m_pMap == NULL)
            AfxThrowMemoryException();
    }
    m_cfFull.Serialize(ar);
    m_cfHalf.Serialize(ar);
    m_cfSmall.Serialize(ar);

    for (int r = 0; r < m_nRows; r++)
    {
        for (int c = 0; c < m_nCols; c++)
            GetCell(r, c)->Serialize(ar);
    }
}


