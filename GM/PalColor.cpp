// ColorPal.cpp : implementation file
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
#include    "afxpriv.h"         // for WM_IDLEUPDATECMDUI
#include    <limits.h>
#include    "LibMfc.h"

#include    "Gm.h"
#include    "GmDoc.h"
#include    "ResTbl.h"

#include    "FrmMain.h"
#include    "PalColor.h"
#include    "CDib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

static const CB::string szColorPalDefPos = "140 350";
static const CB::string szSectSettings = "Settings";
static const CB::string szEntryColorPalPos = "ColorPalPos";

/////////////////////////////////////////////////////////////////////////////
// Color picker area layout:
//
// S S G  C C C  +---+ +-------------------+
// S S G  C C C  | C | | Saturation        |
// S S G  C C C  | O | | Value             |
// S S G  C C C  | L | | Wash              |
// S S G  C C C  | O | |                   |
// S S G  C C C  | R | |                   |
// S S G  C C C  |   | |                   |
// S S G  C C C  +---+ +-------------------+
// +---------------------------------------+
// |               Color Bar               |
// +---------------------------------------+
//
// S = Standard colors
// G = Grey scale
// C = Custom color picks

const int sizeVertMargin = 4;
const int sizeHorzMargin = 4;

const int sizeLeftInset = sizeHorzMargin + 45;

// The constants related to the color picker area

const int cellArrayRows = 8;
const int cellStdArrayCols = 3;
const int cellCustArrayCols = 6;

const int cellStdArraySize = cellStdArrayCols * cellArrayRows;
const int cellCustArraySize = cellCustArrayCols * cellArrayRows;

const int sizeColorCell = 9;
const int sizeCellGap = 1;
const int sizeGroupGap = 4;

const int sizeColorMixWidth = 20;
const int sizeColorBarHeight = 16;

// These constants relate to items left of
// the color picker area

const int numLineWidths = 25;       // Combo line width entries

const int sizeSelectCellStagger = 5;
const int sizeLeftMarg = 45;
const int sizeXSelectCell = 35;
const int sizeYSelectCell = 20;
const int sizeXTransCell = 41;
const int sizeYTransCell = 15;

const int posXNoColor = 2;
const int posYNoColor = 60;
const int sizeXNoColor = 42;        // Y is computed from font.

/////////////////////////////////////////////////////////////////////////////
// Standard colors...

COLORREF acrStdColors[cellStdArraySize] =
{
    RGB(255, 255, 255), RGB(  0,   0,   0), RGB( 28,  28,  28),
    RGB(192, 192, 192), RGB(128, 128, 128), RGB( 57,  57,  57),
    RGB(255,   0,   0), RGB(128,   0,   0), RGB( 85,  85,  85),
    RGB(255, 255,   0), RGB(128, 128,   0), RGB(114, 114, 114),
    RGB(  0, 255,   0), RGB(  0, 128,   0), RGB(142, 142, 142),
    RGB(  0, 255, 255), RGB(  0, 128, 128), RGB(171, 171, 171),
    RGB(255,   0, 255), RGB(128,   0, 128), RGB(199, 199, 199),
    RGB(  0,   0, 255), RGB(  0,   0, 128), RGB(228, 228, 228)
};

inline COLORREF& CellColor(COLORREF* pCref, int nCol, int nRow)
{
    return *(pCref + nRow *
        (acrStdColors == pCref ? cellStdArrayCols : cellCustArrayCols) + nCol);
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CColorPalette, CDockablePane)
    ON_WM_PAINT()
    ON_WM_DESTROY()
    ON_WM_CREATE()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
    ON_CBN_SELCHANGE(IDC_W_COLORPAL_LINEWIDTH, OnLineWidthCbnSelchange)
//@@@   ON_WM_SYSCOMMAND()
    ON_WM_HELPINFO()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONUP()
    ON_WM_ERASEBKGND()
    ON_WM_NCCALCSIZE()
    ON_WM_NCPAINT()
    ON_WM_NCHITTEST()
    ON_WM_LBUTTONDBLCLK()
    ON_MESSAGE(WM_PALETTE_HIDE, OnPaletteHide)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CColorPalette, CDockablePane);

/////////////////////////////////////////////////////////////////////////////
// CColorPalette

CColorPalette::CColorPalette()
{
    m_xCurPos = INT_MIN;
    m_yCurPos = INT_MIN;

    m_crFore = RGB(0, 0, 0);
    m_crBack = RGB(255, 255, 255);
    m_crTrans = nullColorRef;

    m_nHue = 120;
    m_nSat = 200;
    m_nVal = 200;

    UpdateCurrentColorMix(FALSE);

    m_bTrackHue = FALSE;
    m_bTrackSV = FALSE;
    m_bIgnoreRButtonUp = FALSE;

    m_pCustColors = (COLORREF*)CustomColorsAllocate();
}

CColorPalette::~CColorPalette()
{
    CustomColorsFree(m_pCustColors);
}

int CColorPalette::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    ComputeLayout();

    lpCreateStruct->cx = m_sizeClient.cx;
    lpCreateStruct->cy = m_sizeClient.cy;

    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!SetupLineControl())
        return FALSE;

    m_bmapBar = CreateRGBColorBar(m_rctColorBar.Width() - 2, m_rctColorBar.Height() - 2);
    GenerateSVWash(FALSE);

    SetupToolTips(m_sizeClient.cx);
    return 0;
}

CSize CColorPalette::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
    CRect rctInside(0, 0, 0, 0);
    CalcInsideRect(rctInside, TRUE);
    CSize sizeInside(rctInside.Size());
    return CSize(m_sizeClient - sizeInside);
    // return CDockablePane::CalcFixedLayout(bStretch, bHorz);
}

BOOL CColorPalette::OnEraseBkgnd(CDC* pDC)
{
    CRect rectClient;
    GetClientRect(rectClient);

    pDC->FillRect(rectClient, &afxGlobalData.brBtnFace);
    return TRUE;
}

void CColorPalette::OnNcPaint()
{
    CDockablePane::OnNcPaint();
}

void CColorPalette::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
    CDockablePane::OnNcCalcSize(bCalcValidRects, lpncsp);
}

LRESULT CColorPalette::OnNcHitTest(CPoint point)
{
    return CDockablePane::OnNcHitTest(point);
}

void CColorPalette::OnDestroy()
{
    CDockablePane::OnDestroy();
//  // Save the current position of the palette
//  CRect rct;
//  GetWindowRect(&rct);
//  m_xCurPos = rct.left;
//  m_yCurPos = rct.top;
}

void CColorPalette::PostNcDestroy()
{
    CDockablePane::PostNcDestroy();
    /* DO NOTHING - FRAME CLASS WOULD DELETE ITSELF! */
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing...

void CColorPalette::SetupToolTips(int nMaxWidth)
{
    m_toolTip.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX);
    m_toolTip.SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    m_toolTip.SetMaxTipWidth(nMaxWidth);

    SetupToolTip(&m_comboLine, IDS_TT_LINEWIDTH_COMBO, TTF_CENTERTIP);

    SetupToolTip(m_rctNoColor, IDS_TT_NULLCOLOR);
    SetupToolTip(m_rctStdColors, IDS_TT_COLORCELLS, TTF_CENTERTIP);
    SetupToolTip(m_rctColorMix, IDS_TT_COLORCELLS, TTF_CENTERTIP);

    CB::string strRes = CB::string::LoadString(IDS_TT_CUST_COLOR_CELLS);
    SetupToolTip(m_rctCustColors, IDS_TT_CUST_COLOR_CELLS, TTF_CENTERTIP, &strRes);

    m_toolTip.Activate(TRUE);
}

void CColorPalette::SetupToolTip(RECT* rct, UINT nID, UINT nFlags, const CB::string* pszText)
{
    TOOLINFO ti;
    memset(&ti, 0, sizeof(TOOLINFO));
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags |= nFlags;
    ti.uFlags |= TTF_SUBCLASS | nFlags;
    ti.hwnd = m_hWnd;
    ti.uId = nID;
    if (pszText == NULL)
        ti.lpszText = const_cast<CB::string::value_type*>(MAKEINTRESOURCE(nID));
    else
        ti.lpszText = const_cast<CB::string::value_type*>(pszText->v_str());
    ti.hinst = AfxGetResourceHandle();
    ti.rect = *rct;
    m_toolTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void CColorPalette::SetupToolTip(CWnd* pWnd, UINT nID, UINT nFlags, const CB::string* pszText)
{
    TOOLINFO ti;
    memset(&ti, 0, sizeof(TOOLINFO));
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags |= TTF_IDISHWND | TTF_SUBCLASS | nFlags;
    ti.hwnd = m_hWnd;
    ti.uId = reinterpret_cast<uintptr_t>(pWnd->GetSafeHwnd());
    if (pszText == NULL)
        ti.lpszText = const_cast<CB::string::value_type*>(MAKEINTRESOURCE(nID));
    else
        ti.lpszText = const_cast<CB::string::value_type*>(pszText->v_str());
    ti.hinst = AfxGetResourceHandle();
    m_toolTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::ComputeLayout()
{
    // Standard color cell group...
    int x = sizeLeftInset;
    int y = sizeVertMargin;
    int nWd = sizeColorCell * cellStdArrayCols + sizeCellGap * (cellStdArrayCols - 1);
    int nHt = sizeColorCell * cellArrayRows + sizeCellGap * (cellArrayRows - 1);
    m_rctStdColors = CRect(CPoint(x, y), CSize(nWd, nHt));

    // Custom color cell group...
    x = m_rctStdColors.right + sizeGroupGap;
    nWd = sizeColorCell * cellCustArrayCols + sizeCellGap * (cellCustArrayCols - 1);
    m_rctCustColors = CRect(CPoint(x, y), CSize(nWd, nHt));

    // Color mix preview area...
    x = m_rctCustColors.right + sizeGroupGap;
    nWd = sizeColorMixWidth;
    m_rctColorMix = CRect(CPoint(x, y), CSize(nWd, nHt));

    // Color saturation, value picker area...
    x = m_rctColorMix.right + sizeGroupGap;
    nWd = nHt;          // Make it square
    m_rctSatValWash = CRect(CPoint(x, y), CSize(nWd, nHt));

    // Hue picker area...
    x = sizeLeftInset;
    y = m_rctStdColors.bottom + sizeGroupGap;
    nWd = m_rctSatValWash.right - m_rctStdColors.left;
    nHt = sizeColorBarHeight;
    m_rctColorBar = CRect(CPoint(x, y), CSize(nWd, nHt));

    // Compute client size...
    m_sizeClient.cx = m_rctColorBar.right + sizeHorzMargin;
    m_sizeClient.cy = m_rctColorBar.bottom + sizeVertMargin;

    // Compute various status and selector rectangles...
    m_rctNoColor.SetRect(posXNoColor, posYNoColor, posXNoColor + sizeXNoColor,
        posYNoColor + g_res.tm8ss.tmHeight + 2);

    int xOffset = 2*sizeCellGap + 1;
    int yOffset = 2*sizeCellGap + 1;
    m_rctForeColor.SetRect(xOffset, yOffset, xOffset + sizeXSelectCell,
        yOffset + sizeYSelectCell);

    m_rctBackColor = m_rctForeColor;
    m_rctBackColor.OffsetRect(2 * xOffset, 2 * yOffset);

    yOffset += m_rctBackColor.bottom + yOffset;
    m_rctTrans.SetRect(xOffset, yOffset, xOffset + sizeXTransCell,
        yOffset + sizeYTransCell);
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CColorPalette::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
    if (IsVisible())             // Ignore if child is invisible
    {
        CFrameWnd* pTarget = GetMainFrame();
        if (pTarget != NULL)
            OnUpdateCmdUI(pTarget, (BOOL)wParam);
    }
    return 0L;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CColorPalette::SetupLineControl()
{
    // Rect needed so we can create the control
    CRect rctCombo(0, 0, g_res.tm8ss.tmMaxCharWidth, g_res.tm8ss.tmHeight);

    if (!m_comboLine.Create(
        WS_CHILD | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST,
        rctCombo, this, IDC_W_COLORPAL_LINEWIDTH))
    {
        return FALSE;
    }
    m_comboLine.SetFont(CFont::FromHandle(g_res.h8ss));
    m_comboLine.GetWindowRect(&rctCombo);   // Fetch result of create
    int y = m_sizeClient.cy - rctCombo.Height() - sizeCellGap * 2 + 1;
    m_comboLine.MoveWindow(sizeCellGap * 2 + 1, y, sizeXTransCell,
        5 * rctCombo.Height());
    for (int i = 0; i <= numLineWidths; i++)
    {
        CB::string strNum = std::format("{}", i);
        m_comboLine.AddString(strNum);
    }
    m_comboLine.SetCurSel(1);
    m_comboLine.ShowWindow(SW_SHOW);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Command UI Stuff

// Only used for auto disable
void CColorCmdUI::Enable(BOOL bOn)
{
    m_bEnableChanged = TRUE;
    if (!bOn)
        ((CColorPalette*)m_pOther)->SetIDColor(m_nID, nullColorRef);
}

void CColorCmdUI::SetColor(COLORREF cr)
{
    m_bEnableChanged = TRUE;
    ((CColorPalette*)m_pOther)->SetIDColor(m_nID, cr);
}

void CColorCmdUI::SetLineWidth(UINT uiLineWidth)
{
    m_bEnableChanged = TRUE;
    ((CColorPalette*)m_pOther)->SetLineWidth(uiLineWidth);
}

void CColorCmdUI::SetCustomColors(LPVOID pCustColors)
{
    m_bEnableChanged = TRUE;
    ((CColorPalette*)m_pOther)->SetCustomColors(pCustColors);
}

void CColorPalette::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
    CColorCmdUI state;
    state.m_pOther = this;
    state.m_nID = ID_COLOR_FOREGROUND;
    state.DoUpdate(pTarget, bDisableIfNoHndler);
    state.m_nID = ID_COLOR_BACKGROUND;
    state.DoUpdate(pTarget, bDisableIfNoHndler);
    state.m_nID = ID_COLOR_TRANSPARENT;
    state.DoUpdate(pTarget, bDisableIfNoHndler);
    state.m_nID = ID_COLOR_CUSTOM;
    state.DoUpdate(pTarget, bDisableIfNoHndler);
    state.m_nID = ID_LINE_WIDTH;
    state.DoUpdate(pTarget, bDisableIfNoHndler);
}

void CColorPalette::OnPressCloseButton()
{
    CDockablePane::OnPressCloseButton();
    SendMessage(WM_PALETTE_HIDE);
}

/////////////////////////////////////////////////////////////////////////////
// Operations

/////////////////////////////////////////////////////////////////////////////
// CColorPalette message handlers

void CColorPalette::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    CPalette* pPal = GetMainFrame()->GetMasterPalette();

    if (pPal == NULL || pPal->m_hObject == NULL)
        pPal = CPalette::FromHandle(
            (HPALETTE)::GetStockObject(DEFAULT_PALETTE));
    else
        dc.SelectPalette(pPal, FALSE);
    dc.RealizePalette();

    DoPaint(&dc);

    dc.SelectPalette(CPalette::FromHandle(
        (HPALETTE)::GetStockObject(DEFAULT_PALETTE)), TRUE);
}

void CColorPalette::DoPaint(CDC* pDC)
{
    CBrush brBlack;
    brBlack.CreateStockObject(BLACK_BRUSH);
    CBrush brWhite;
    brWhite.CreateStockObject(WHITE_BRUSH);

    // Draw the no color selection area.
    if (pDC->RectVisible(&m_rctNoColor))
    {
        pDC->FillRect(&m_rctNoColor, &brWhite);

        DrawEdge(pDC->m_hDC, m_rctNoColor, EDGE_BUMP, BF_RECT);

        CFont* pPrvFont = pDC->SelectObject(CFont::FromHandle(g_res.h8ss));
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(RGB(0,0,0));
        pDC->SetTextAlign(TA_CENTER | TA_TOP);
        pDC->ExtTextOut((m_rctNoColor.left + m_rctNoColor.right) / 2 ,
            m_rctNoColor.top + 1, 0, NULL, "no color"_cbstring, 8, NULL);
        pDC->SelectObject(pPrvFont);
    }

    // Paint the selected colors...
    PaintSelections(pDC);

    // Paint the standard colors...
    PaintCellGroup(pDC, acrStdColors, m_rctStdColors.left, m_rctStdColors.top);

    // Paint the custom color selections...
    PaintCellGroup(pDC, m_pCustColors, m_rctCustColors.left, m_rctCustColors.top);

    // Paint the color mix preview...
    CBrush brColor;
    brColor.CreateSolidBrush(m_crCurMix);
    pDC->FillRect(&m_rctColorMix, &brColor);
    pDC->FrameRect(&m_rctColorMix, &brBlack);

    // Paint the color bar...
    CDC dcMem;
    dcMem.CreateCompatibleDC(pDC);
    CBitmap* pPrvBMap = dcMem.SelectObject(&*m_bmapBar);
    pDC->BitBlt(m_rctColorBar.left + 1, m_rctColorBar.top + 1,
        m_rctColorBar.Width(), m_rctColorBar.Height(), &dcMem, 0, 0, SRCCOPY);
    pDC->FrameRect(m_rctColorBar, &brBlack);

    // Paint the current hue saturation/value wash...
    dcMem.SelectObject(&*m_bmapWash);
    pDC->BitBlt(m_rctSatValWash.left + 1, m_rctSatValWash.top + 1,
        m_rctSatValWash.Width(), m_rctSatValWash.Height(), &dcMem, 0, 0, SRCCOPY);
    dcMem.SelectObject(pPrvBMap);
    pDC->FrameRect(m_rctSatValWash, &brBlack);

    // Mark current selections...
    int nHueMark, nSatMark, nValMark;

    MapHSVtoPixelLoc(&nHueMark, &nSatMark, &nValMark);

    pDC->PatBlt(m_rctColorBar.left + nHueMark + 1, m_rctColorBar.top + 1,
        1, m_rctColorBar.Height() - 2, DSTINVERT);
    pDC->PatBlt(m_rctSatValWash.left + nValMark + 1, m_rctSatValWash.top + 1,
        1, m_rctSatValWash.Height() - 2, DSTINVERT);
    pDC->PatBlt(m_rctSatValWash.left + 1, m_rctSatValWash.top + nSatMark + 1,
        m_rctSatValWash.Width() - 2, 1, DSTINVERT);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::PaintSelections(CDC* pDC)
{
    PaintCell(pDC, m_rctForeColor, m_crFore);
    DrawEdge(pDC->m_hDC, m_rctForeColor, EDGE_RAISED, BF_RECT);

    pDC->SaveDC();
    pDC->ExcludeClipRect(m_rctForeColor);
    PaintCell(pDC, m_rctBackColor, m_crBack);
    DrawEdge(pDC->m_hDC, m_rctBackColor, EDGE_SUNKEN, BF_RECT);
    pDC->RestoreDC(-1);

    if (m_crTrans != nullColorRef)
    {
        PaintCell(pDC, m_rctTrans, m_crTrans);
        DrawEdge(pDC->m_hDC, m_rctTrans, EDGE_BUMP, BF_RECT);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::PaintCellGroup(CDC* pDC, COLORREF* pArray, int xLoc, int yLoc)
{
    CBrush brBlack;
    brBlack.CreateStockObject(BLACK_BRUSH);
    CBrush brGray;
    brGray.CreateStockObject(GRAY_BRUSH);

    int x = xLoc;
    int y = yLoc;
    int nCols = pArray == acrStdColors ? cellStdArrayCols : cellCustArrayCols;
    for (int nCol = 0; nCol < nCols; nCol++)
    {
        for (int nRow = 0; nRow < cellArrayRows; nRow++)
        {
            CBrush brColor;
            CRect rct(x, y, x + sizeColorCell, y + sizeColorCell);
            COLORREF cref = CellColor(pArray, nCol, nRow);
            if (cref != nullColorRef)
            {
                brColor.CreateSolidBrush(cref);
                pDC->FillRect(&rct, &brColor);
                pDC->FrameRect(&rct, &brBlack);
            }
            else
                pDC->FrameRect(&rct, &brGray);
            y += sizeColorCell + sizeCellGap;
        }
        x += sizeColorCell + sizeCellGap;
        y = yLoc;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::PaintCell(CDC* pDC, CRect& rct, COLORREF cref, BOOL bSelBorder)
{
    if (bSelBorder)
        rct.InflateRect(1, 1);
    if (!pDC->RectVisible(&rct))
        return;

    CBrush brBlack;
    brBlack.CreateStockObject(BLACK_BRUSH);

    CBrush brColor;
    if (cref != nullColorRef)
        brColor.CreateSolidBrush(cref);
    else
        brColor.CreateHatchBrush(HS_DIAGCROSS, RGB(0,0,0));

    if (bSelBorder)
    {
        pDC->FrameRect(&rct, &brBlack);
        rct.InflateRect(-1, -1);
    }
    pDC->FillRect(&rct, &brColor);
    pDC->FrameRect(&rct, &brBlack);
}

/////////////////////////////////////////////////////////////////////////////

COLORREF* CColorPalette::MapMouseToColorCell(COLORREF* pArray, CPoint pntClient, CRect& rctArray)
{
    if (!rctArray.PtInRect(pntClient))
        return NULL;
    int x = rctArray.left;
    int y = rctArray.top;
    int nCols = pArray == acrStdColors ? cellStdArrayCols : cellCustArrayCols;
    for (int nCol = 0; nCol < nCols; nCol++)
    {
        for (int nRow = 0; nRow < cellArrayRows; nRow++)
        {
            CRect rct(x, y, x + sizeColorCell, y + sizeColorCell);
            if (rct.PtInRect(pntClient))
                return &CellColor(pArray, nCol, nRow);
            y += sizeColorCell + sizeCellGap;
        }
        x += sizeColorCell + sizeCellGap;
        y = rctArray.top;
    }
    return NULL;
}

void CColorPalette::MapHSVtoPixelLoc(int* pnHLoc, int* pnSLoc, int* pnVLoc)
{
    if (pnHLoc != NULL)
        *pnHLoc = (m_nHue * (m_rctColorBar.Width() - 2)) / 359;
    if (pnSLoc != NULL)
        *pnSLoc = ((255 - m_nSat) * (m_rctSatValWash.Width() - 2)) / 255;
    if (pnVLoc != NULL)
        *pnVLoc = (m_nVal * (m_rctSatValWash.Height() - 2)) / 255;
}

BOOL CColorPalette::MapMouseLocToH(CPoint pntClient, int& nH, BOOL bCheckValidPoint)
{
    if (bCheckValidPoint && !m_rctColorBar.PtInRect(pntClient))
        return FALSE;
    nH = (359 * (pntClient.x - m_rctColorBar.left + 1)) / (m_rctColorBar.Width() -  2);
    if (nH < 0) nH = 0;
    if (nH > 359) nH = 359;
    return m_rctColorBar.PtInRect(pntClient);
}

BOOL CColorPalette::MapMouseLocToSV(CPoint pntClient, int& nS, int& nV, BOOL bCheckValidPoint)
{
    if (bCheckValidPoint && !m_rctSatValWash.PtInRect(pntClient))
        return FALSE;
    nS = 255 - (255 * (pntClient.y - m_rctSatValWash.top + 1)) / (m_rctSatValWash.Height() - 2);
    if (nS < 0) nS = 0;
    if (nS > 255) nS = 255;
    nV = (255 * (pntClient.x - m_rctSatValWash.left + 1)) / (m_rctSatValWash.Width() - 2);
    if (nV < 0) nV = 0;
    if (nV > 255) nV = 255;
    return m_rctSatValWash.PtInRect(pntClient);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::GenerateSVWash(BOOL bInvalidate /* = TRUE */)
{
    m_bmapWash = CreateRGBSaturationValueWash(m_nHue, m_rctSatValWash.Width() - 2,
        m_rctSatValWash.Height() - 2);
    if (bInvalidate)
        InvalidateRect(m_rctSatValWash, FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::SetLineWidth(UINT nLineWidth)
{
    if (m_comboLine.GetDroppedState())
        return;

    if (nLineWidth > numLineWidths)
        nLineWidth = (unsigned)-1;
    if (m_comboLine.GetCurSel() != (int)nLineWidth)
        m_comboLine.SetCurSel(nLineWidth);
}

void CColorPalette::SetIDColor(UINT nID, COLORREF cr)
{
    if (nID == ID_COLOR_FOREGROUND)
    {
        if (cr == m_crFore) return;
        m_crFore = cr;
    }
    else if (nID == ID_COLOR_BACKGROUND)
    {
        if (cr == m_crBack) return;
        m_crBack = cr;
    }
    else if (nID == ID_COLOR_TRANSPARENT)
    {
        if (cr == m_crTrans) return;
        m_crTrans = cr;
        if (m_crTrans == nullColorRef)
            m_toolTip.DelTool(this, IDS_TT_TRANSCOLOR);
        else
            SetupToolTip(m_rctTrans, IDS_TT_TRANSCOLOR);
        InvalidateRect(m_rctNoColor);
    }
    else
        return;
    UpdateCurrentColors(FALSE);
}

void CColorPalette::SetCustomColors(LPVOID pCustColors)
{
    if (CustomColorsCompareEqual(m_pCustColors, pCustColors))
        return;
    CustomColorsSet(m_pCustColors, pCustColors);
    InvalidateRect(m_rctCustColors, FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::UpdateCurrentColors(BOOL bImmediate)
{
    CRect rct;
    GetClientRect(&rct);
    rct.right = sizeCellGap * 2 + sizeLeftMarg - 1;
    rct.bottom = (rct.bottom * 5) / 8;              // Miss combo box.
    InvalidateRect(&rct, FALSE);

    if (bImmediate)
        UpdateWindow();
}

void CColorPalette::UpdateCurrentColorMix(BOOL bUpdate /* = TRUE*/)
{
    COLORREF cref = HSVtoRGB(m_nHue, m_nSat, m_nVal);
    // Make sure we only show possible colors.
    m_crCurMix = RGB565_TO_24(RGB565(cref));
    if (bUpdate)
        InvalidateRect(m_rctColorMix, FALSE);
}

/////////////////////////////////////////////////////////////////////////////

/* static */
LPVOID CColorPalette::CustomColorsAllocate()
{
    COLORREF* pCustColors = new COLORREF[cellCustArraySize];
    return pCustColors;
}

/* static */
void CColorPalette::CustomColorsFree(LPVOID pCustColors)
{
    COLORREF* crTbl = (COLORREF*)pCustColors;
    if (crTbl != NULL)
        delete [] crTbl;
}

/* static */
void CColorPalette::CustomColorsSet(LPVOID pCustColorsTo, LPVOID pCustColorsFrom)
{
    memcpy(pCustColorsTo, pCustColorsFrom, cellCustArraySize * sizeof(COLORREF));
}

/* static */
BOOL CColorPalette::CustomColorsCompareEqual(LPVOID pCustColors1, LPVOID pCustColors2)
{
    return memcmp(pCustColors1, pCustColors2, cellCustArraySize * sizeof(COLORREF)) == 0;
}

/* static */
void CColorPalette::CustomColorsSerialize(CArchive& ar, LPVOID pCustColors)
{
    ASSERT(pCustColors != NULL);
    COLORREF* crTbl = (COLORREF*)pCustColors;
    if (ar.IsStoring())
    {
        ar << (WORD)cellCustArraySize;
        for (int i = 0; i < cellCustArraySize; i++)
            ar << (DWORD)crTbl[i];
    }
    else
    {
        WORD wCount;
        DWORD dwColor;

        ar >> wCount;
        for (int i = 0; i < (int)wCount; i++)
        {
            ar >> dwColor;
            crTbl[i] = (COLORREF)dwColor;
        }
    }
}

/* static */
void CColorPalette::CustomColorsClear(LPVOID pCustColors)
{
    COLORREF* crTbl = (COLORREF*)pCustColors;
    for (int i = 0; i < cellCustArraySize; i++)
        crTbl[i] = nullColorRef;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CColorPalette::HandleButtonDown(UINT nFlags, CPoint point)
{
    COLORREF* pCRef = NULL;
    if (MapMouseLocToH(point, m_nHue))
    {
        GenerateSVWash();
        UpdateCurrentColorMix(TRUE);
        InvalidateRect(m_rctColorBar, FALSE);   // So select line is updated
        if (nFlags & MK_LBUTTON)
        {
            SetCapture();
            m_bTrackHue = TRUE;
        }
    }
    else if (MapMouseLocToSV(point, m_nSat, m_nVal))
    {
        UpdateCurrentColorMix(TRUE);
        InvalidateRect(m_rctSatValWash, FALSE); // So select lines are updated
        if (nFlags & MK_LBUTTON)
        {
            SetCapture();
            m_bTrackSV = TRUE;
        }
    }
    else if (m_rctColorMix.PtInRect(point))
    {
        NotifyColorChange(nFlags, m_crCurMix);
    }
    else if (m_rctNoColor.PtInRect(point))
    {
        NotifyColorChange(nFlags, nullColorRef);
    }
    else if (m_crTrans != nullColorRef && m_rctTrans.PtInRect(point))
    {
        NotifyColorChange(nFlags, m_crTrans);
    }
    else if ((pCRef = MapMouseToColorCell(acrStdColors, point, m_rctStdColors)) != NULL)
    {
        NotifyColorChange(nFlags, *pCRef);
    }
    else if ((pCRef = MapMouseToColorCell(m_pCustColors, point, m_rctCustColors)) != NULL)
    {
        if ((nFlags & mouseMask) == mouseSetCustomFromFore)
        {
            if (m_crFore != nullColorRef)
            {
                *pCRef = m_crFore;
                InvalidateRect(m_rctCustColors, FALSE);
                NotifyCustomColorChange(m_pCustColors);
            }
        }
        else if ((nFlags & mouseMask) == mouseSetCustomFromBack)
        {
            if (m_crBack != nullColorRef)
            {
                *pCRef = m_crBack;
                InvalidateRect(m_rctCustColors, FALSE);
                NotifyCustomColorChange(m_pCustColors);
            }
        }
        else if ((nFlags & mouseMask) == mouseSetCustomFromMix)
        {
            *pCRef = m_crCurMix;
            InvalidateRect(m_rctCustColors, FALSE);
            NotifyCustomColorChange(m_pCustColors);
        }
        else if ((nFlags & mouseMask) == mouseSetCustomClear)
        {
            *pCRef = nullColorRef;
            InvalidateRect(m_rctCustColors, TRUE);
            NotifyCustomColorChange(m_pCustColors);
        }
        else if (*pCRef != nullColorRef)
            NotifyColorChange(nFlags, *pCRef);
    }
    else
        return FALSE;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::NotifyColorChange(UINT nFlags, COLORREF cref)
{
    CView* pView = GetMainFrame()->GetActiveView();

    if (pView == NULL)
        return;

    if ((nFlags & mouseMask) == mouseFore)
    {
        pView->SendMessage(WM_SETCOLOR, (WPARAM)ID_COLOR_FOREGROUND, (LPARAM)cref);
    }
    else if ((nFlags & mouseMask) == mouseBack1 ||
        (nFlags & mouseMask) == mouseBack2)
    {
        pView->SendMessage(WM_SETCOLOR, (WPARAM)ID_COLOR_BACKGROUND, (LPARAM)cref);
    }
}

void CColorPalette::NotifyCustomColorChange(COLORREF* pcrCustomColors)
{
    CView* pView = GetMainFrame()->GetActiveView();

    if (pView == NULL)
        return;
    pView->SendMessage(WM_SETCUSTOMCOLOR, (WPARAM)pcrCustomColors);
}

/////////////////////////////////////////////////////////////////////////////


void CColorPalette::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if (m_rctCustColors.PtInRect(point))
    {
        // Double clicking in a color cell allows the user to
        // explicitly set the color using a color dialog.
        COLORREF* pCRef = NULL;
        if ((pCRef = MapMouseToColorCell(m_pCustColors, point, m_rctCustColors)) == NULL)
            return;         // Just the mouse hit
        COLORREF cr = *pCRef == nullColorRef ? 0 : *pCRef;

        CColorDialog dlg(cr, cr);
        if (dlg.DoModal())
        {
            *pCRef = dlg.GetColor();
            InvalidateRect(m_rctCustColors, FALSE);
            NotifyCustomColorChange(m_pCustColors);
        }
    }
    else    // Let control bar processing handle it
        CDockablePane::OnLButtonDblClk(nFlags, point);
}

void CColorPalette::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (!HandleButtonDown(nFlags, point))
        CDockablePane::OnLButtonDown(nFlags, point);
}

void CColorPalette::OnRButtonDown(UINT nFlags, CPoint point)
{
    if (!HandleButtonDown(nFlags, point))
    {
        CDockablePane::OnRButtonDown(nFlags, point);
    }
    else
    {
        // in case user drags mouse to another window before RButtonUp
        SetCapture();
        m_bIgnoreRButtonUp = TRUE;
    }
}

void CColorPalette::OnMouseMove(UINT nFlags, CPoint point)
{
    if (GetCapture() == this)
    {
        if (m_bTrackHue)
        {
            MapMouseLocToH(point, m_nHue, FALSE);
            UpdateCurrentColorMix(TRUE);
            GenerateSVWash();
            InvalidateRect(m_rctColorBar, FALSE);   // So select line is updated
        }
        else if (m_bTrackSV)
        {
            MapMouseLocToSV(point, m_nSat, m_nVal, FALSE);
            UpdateCurrentColorMix(TRUE);
            InvalidateRect(m_rctSatValWash, FALSE); // So select lines are updated
        }
    }
    CDockablePane::OnMouseMove(nFlags, point);
}

void CColorPalette::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_bTrackHue = FALSE;
    m_bTrackSV = FALSE;
    ReleaseCapture();

    CDockablePane::OnLButtonUp(nFlags, point);
}

void CColorPalette::OnRButtonUp(UINT nFlags, CPoint point)
{
    if (m_bIgnoreRButtonUp)
    {
        m_bIgnoreRButtonUp = FALSE;
        ReleaseCapture();
        return;
    }

    CDockablePane::OnRButtonUp(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::OnLineWidthCbnSelchange()
{
    if (!m_comboLine.GetDroppedState())
    {
        CView* pView = GetMainFrame()->GetActiveView();
        pView->SendMessage(WM_SETLINEWIDTH,
            (WPARAM)m_comboLine.GetCurSel(), (LPARAM)0);
    }
}

//void CColorPalette::OnSysCommand(UINT nID, LPARAM lParam)
//{
//  if ((nID & 0xFFF0) == SC_CLOSE)
//      GetMainFrame()->SendMessage(WM_COMMAND, ID_WINDOW_COLORPAL);
//  else
//      CDockablePane::OnSysCommand(nID, lParam);
//}

BOOL CColorPalette::OnHelpInfo(HELPINFO* pHelpInfo)
{
    GetApp()->DoHelpTopic("gm-ref-pal-color.htm");
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CColorPalette::OnPaletteHide(WPARAM, LPARAM)
{
    GetMainFrame()->SendMessage(WM_COMMAND, ID_WINDOW_COLORPAL);
    return (LRESULT)0;
}

