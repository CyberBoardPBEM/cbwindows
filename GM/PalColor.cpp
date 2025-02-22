// ColorPal.cpp : implementation file
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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
// TODO:  remove when CTileSelViewContainer removed
#include    "VwTilesl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

wxDEFINE_EVENT(cbEVT_UPDATE_UI_COLOR, CColorCmdUI);

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

wxColour acrStdColors[cellStdArraySize] =
{
    wxColour(255, 255, 255), wxColour(  0,   0,   0), wxColour( 28,  28,  28),
    wxColour(192, 192, 192), wxColour(128, 128, 128), wxColour( 57,  57,  57),
    wxColour(255,   0,   0), wxColour(128,   0,   0), wxColour( 85,  85,  85),
    wxColour(255, 255,   0), wxColour(128, 128,   0), wxColour(114, 114, 114),
    wxColour(  0, 255,   0), wxColour(  0, 128,   0), wxColour(142, 142, 142),
    wxColour(  0, 255, 255), wxColour(  0, 128, 128), wxColour(171, 171, 171),
    wxColour(255,   0, 255), wxColour(128,   0, 128), wxColour(199, 199, 199),
    wxColour(  0,   0, 255), wxColour(  0,   0, 128), wxColour(228, 228, 228)
};

inline const wxColour& CellColor(const wxColour* pCref, int nCol, int nRow)
{
    return *(pCref + nRow *
        (acrStdColors == pCref ? cellStdArrayCols : cellCustArrayCols) + nCol);
}

inline wxColour& CellColor(wxColour* pCref, int nCol, int nRow)
{
    return const_cast<wxColour&>(CellColor(&std::as_const(*pCref), nCol, nRow));
}

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CColorPalette, wxPanel)
    EVT_PAINT(OnPaint)
    EVT_WINDOW_CREATE(OnCreate)
    EVT_LEFT_DOWN(OnLButtonDown)
    EVT_RIGHT_DOWN(OnRButtonDown)
    EVT_UPDATE_UI(wxID_ANY, OnUpdateCmdUI)
    EVT_CHOICE(XRCID("IDC_W_COLORPAL_LINEWIDTH"), OnLineWidthCbnSelchange)
#if 0
    ON_WM_HELPINFO()
#endif
    EVT_MOTION(OnMouseMove)
    EVT_LEFT_UP(OnLButtonUp)
    EVT_RIGHT_UP(OnRButtonUp)
    EVT_LEFT_DCLICK(OnLButtonDblClk)
    EVT_MOUSE_CAPTURE_LOST(OnMouseCaptureLost)
    EVT_COMMAND(wxID_ANY, WM_PALETTE_HIDE_WX, OnPaletteHide)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////
// CColorPalette

CColorPalette::CColorPalette() :
    m_comboLine(new wxChoice)
{
    m_crFore = *wxBLACK;
    m_crBack = *wxWHITE;
    m_crTrans = wxNullColour;

    m_nHue = 120;
    m_nSat = 200;
    m_nVal = 200;

    UpdateCurrentColorMix(FALSE);

    m_bTrackHue = FALSE;
    m_bTrackSV = FALSE;
    m_bIgnoreRButtonUp = FALSE;

    m_pCustColors = CustomColorsAllocate();
}

void CColorPalette::OnCreate(wxWindowCreateEvent& event)
{
    if (event.GetWindow() != this)
    {
        event.Skip();
        return;
    }

    SetBackgroundStyle(wxBG_STYLE_SYSTEM);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
    ComputeLayout();

    if (!SetupLineControl())
        AfxThrowMemoryException();

    m_bmapBar = CreateRGBColorBar(m_rctColorBar.GetWidth() - 2, m_rctColorBar.GetHeight() - 2);
    GenerateSVWash(FALSE);

    SetupToolTips(m_sizeClient.x);
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing...

void CColorPalette::SetupToolTips(int nMaxWidth)
{
    m_toolTip.SetMaxWidth(nMaxWidth);

    SetupToolTip(*m_comboLine, CB::string::LoadString(IDS_TT_LINEWIDTH_COMBO), CB::ToolTip::CENTER);

    SetupToolTip(m_rctNoColor, CB::string::LoadString(IDS_TT_NULLCOLOR));
    SetupToolTip(m_rctStdColors, CB::string::LoadString(IDS_TT_COLORCELLS), CB::ToolTip::CENTER);
    SetupToolTip(m_rctColorMix, CB::string::LoadString(IDS_TT_COLORCELLS), CB::ToolTip::CENTER);

    SetupToolTip(m_rctCustColors, CB::string::LoadString(IDS_TT_CUST_COLOR_CELLS), CB::ToolTip::CENTER);

    m_toolTip.Enable(TRUE);
}

void CColorPalette::SetupToolTip(const wxRect& rct, wxString tip, CB::ToolTip::Flags flags /*= CB::ToolTip::NONE*/)
{
    m_toolTip.Add(*this, rct, std::move(tip), flags);
}

void CColorPalette::SetupToolTip(wxWindow& pWnd, wxString tip, CB::ToolTip::Flags flags /*= CB::ToolTip::NONE*/)
{
    m_toolTip.Add(pWnd, std::move(tip), flags);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::ComputeLayout()
{
    // Standard color cell group...
    int x = FromDIP(sizeLeftInset);
    int y = FromDIP(sizeVertMargin);
    int nWd = FromDIP(sizeColorCell) * cellStdArrayCols + FromDIP(sizeCellGap) * (cellStdArrayCols - 1);
    int nHt = FromDIP(sizeColorCell) * cellArrayRows + FromDIP(sizeCellGap) * (cellArrayRows - 1);
    m_rctStdColors = wxRect(wxPoint(x, y), wxSize(nWd, nHt));

    // Custom color cell group...
    x = m_rctStdColors.GetRight()  + FromDIP(sizeGroupGap);
    nWd = FromDIP(sizeColorCell) * cellCustArrayCols + FromDIP(sizeCellGap) * (cellCustArrayCols - 1);
    m_rctCustColors = wxRect(wxPoint(x, y), wxSize(nWd, nHt));

    // Color mix preview area...
    x = m_rctCustColors.GetRight() + FromDIP(sizeGroupGap);
    nWd = FromDIP(sizeColorMixWidth);
    m_rctColorMix = wxRect(wxPoint(x, y), wxSize(nWd, nHt));

    // Color saturation, value picker area...
    x = m_rctColorMix.GetRight() + FromDIP(sizeGroupGap);
    nWd = nHt;          // Make it square
    m_rctSatValWash = wxRect(wxPoint(x, y), wxSize(nWd, nHt));

    // Hue picker area...
    x = FromDIP(sizeLeftInset);
    y = m_rctStdColors.GetBottom() + FromDIP(sizeGroupGap);
    nWd = m_rctSatValWash.GetRight() + 1 - m_rctStdColors.GetLeft();
    nHt = FromDIP(sizeColorBarHeight);
    m_rctColorBar = wxRect(wxPoint(x, y), wxSize(nWd, nHt));

    // Compute client size...
    m_sizeClient.x = m_rctColorBar.GetRight() + FromDIP(sizeHorzMargin);
    m_sizeClient.y = m_rctColorBar.GetBottom() + FromDIP(sizeVertMargin);

    // Compute various status and selector rectangles...
    m_rctNoColor = wxRect(FromDIP(wxPoint(posXNoColor, posYNoColor)), wxSize(FromDIP(sizeXNoColor),
        g_res.tm8ss.tmHeight + 2));

    int xOffset = FromDIP(2*sizeCellGap + 1);
    int yOffset = FromDIP(2*sizeCellGap + 1);
    m_rctForeColor = wxRect(wxPoint(xOffset, yOffset), FromDIP(wxSize(sizeXSelectCell,
        sizeYSelectCell)));

    m_rctBackColor = m_rctForeColor;
    m_rctBackColor.Offset(2 * xOffset, 2 * yOffset);

    yOffset += m_rctBackColor.GetBottom() + yOffset;
    m_rctTrans = wxRect(wxPoint(xOffset, yOffset), FromDIP(wxSize(sizeXTransCell,
        sizeYTransCell)));
}

/////////////////////////////////////////////////////////////////////////////

BOOL CColorPalette::SetupLineControl()
{
    if (!m_comboLine->Create(this,
        XRCID("IDC_W_COLORPAL_LINEWIDTH"),
        wxPoint(0, 0), wxSize(g_res.tm8ss.tmMaxCharWidth, wxDefaultCoord)))
    {
        return FALSE;
    }
    m_comboLine->SetFont(g_res.h8ssWx);
    wxRect rctCombo = m_comboLine->GetRect();   // Fetch result of create

    int y = m_sizeClient.y - rctCombo.GetHeight() - FromDIP(sizeCellGap * 2 + 1);
    m_comboLine->SetSize(wxRect(
        wxPoint(FromDIP(sizeCellGap * 2 + 1), y), 
        wxSize(FromDIP(sizeXTransCell), wxDefaultCoord)));
    for (int i = 0; i <= numLineWidths; i++)
    {
        CB::string strNum = std::format("{}", i);
        m_comboLine->AppendString(strNum);
    }
    m_comboLine->SetSelection(1);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Command UI Stuff

CColorCmdUI::CColorCmdUI(CColorPalette& cp) :
    wxCommandEvent(cbEVT_UPDATE_UI_COLOR),
    colorPalette(&cp)
{
}

// Only used for auto disable
void CColorCmdUI::Enable(BOOL bOn)
{
    m_bEnableChanged = TRUE;
    if (!bOn)
        colorPalette->SetIDColor(GetId(), wxNullColour);
}

void CColorCmdUI::SetColor(wxColour cr)
{
    m_bEnableChanged = TRUE;
    colorPalette->SetIDColor(GetId(), cr);
}

void CColorCmdUI::SetLineWidth(UINT uiLineWidth)
{
    m_bEnableChanged = TRUE;
    colorPalette->SetLineWidth(uiLineWidth);
}

void CColorCmdUI::SetCustomColors(const std::vector<wxColour>& pCustColors)
{
    m_bEnableChanged = TRUE;
    colorPalette->SetCustomColors(pCustColors);
}

void CColorPalette::OnUpdateCmdUI(wxUpdateUIEvent& /*pCmdUI*/)
{
    if (!IsShownOnScreen())             // Ignore if wnd is invisible
    {
        return;
    }

    CColorCmdUI state(*this);
    state.SetId(XRCID("ID_COLOR_FOREGROUND"));
    state.DoUpdate();
    state.SetId(XRCID("ID_COLOR_BACKGROUND"));
    state.DoUpdate();
    state.SetId(XRCID("ID_COLOR_TRANSPARENT"));
    state.DoUpdate();
    state.SetId(XRCID("ID_COLOR_CUSTOM"));
    state.DoUpdate();
    state.SetId(XRCID("ID_LINE_WIDTH"));
    state.DoUpdate();
}

void CColorCmdUI::DoUpdate()
{
    m_bEnableChanged = false;
    CB::GetMainWndWx().ProcessWindowEvent(*this);
    if (!m_bEnableChanged)
    {
        Enable(false);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Operations

/////////////////////////////////////////////////////////////////////////////
// CColorPalette message handlers

void CColorPalette::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this); // device context for painting

    DoPaint(dc);
}

void CColorPalette::DoPaint(wxDC& pDC)
{
    // Draw the no color selection area.
    if (CB::RectVisible(pDC, m_rctNoColor))
    {
        pDC.SetPen(*wxTRANSPARENT_PEN);
        pDC.SetBrush(*wxWHITE_BRUSH);
        pDC.DrawRectangle(m_rctNoColor);

#if 0   // TODO:  DrawEdge
        DrawEdge(pDC.m_hDC, m_rctNoColor, EDGE_BUMP, BF_RECT);
#else
        pDC.SetPen(*wxBLACK_PEN);
        pDC.SetBrush(*wxTRANSPARENT_BRUSH);
        pDC.DrawRectangle(m_rctNoColor);
#endif

        pDC.SetFont(g_res.h8ssWx);
        pDC.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
        pDC.SetTextForeground(*wxBLACK);
        pDC.DrawLabel("no color"_cbstring, m_rctNoColor, wxALIGN_CENTRE_HORIZONTAL | wxALIGN_TOP);
    }

    // Paint the selected colors...
    PaintSelections(pDC);

    // Paint the standard colors...
    PaintCellGroup(pDC, acrStdColors, m_rctStdColors.GetLeft(), m_rctStdColors.GetTop());

    // Paint the custom color selections...
    PaintCellGroup(pDC, m_pCustColors.data(), m_rctCustColors.GetLeft(), m_rctCustColors.GetTop());

    // Paint the color mix preview...
    wxBrush brColor(m_crCurMix);
    pDC.SetBrush(brColor);
    pDC.SetPen(*wxBLACK_PEN);
    pDC.DrawRectangle(m_rctColorMix);

    // Paint the color bar...
    wxMemoryDC dcMem;
    dcMem.SelectObjectAsSource(*m_bmapBar);
    pDC.Blit(m_rctColorBar.GetLeft() + 1, m_rctColorBar.GetTop() + 1,
        m_rctColorBar.GetWidth(), m_rctColorBar.GetHeight(), &dcMem, 0, 0, wxCOPY);
    pDC.SetBrush(*wxTRANSPARENT_BRUSH);
    pDC.SetPen(*wxBLACK_PEN);
    pDC.DrawRectangle(m_rctColorBar);

    // Paint the current hue saturation/value wash...
    dcMem.SelectObjectAsSource(*m_bmapWash);
    pDC.Blit(m_rctSatValWash.GetLeft() + 1, m_rctSatValWash.GetTop() + 1,
        m_rctSatValWash.GetWidth(), m_rctSatValWash.GetHeight(), &dcMem, 0, 0, wxCOPY);
    pDC.SetBrush(*wxTRANSPARENT_BRUSH);
    pDC.SetPen(*wxBLACK_PEN);
    pDC.DrawRectangle(m_rctSatValWash);

    // Mark current selections...
    int nHueMark, nSatMark, nValMark;

    MapHSVtoPixelLoc(nHueMark, nSatMark, nValMark);

    pDC.SetPen(*wxTRANSPARENT_PEN);
    pDC.SetLogicalFunction(wxINVERT);
    pDC.DrawRectangle(wxPoint(m_rctColorBar.GetLeft() + nHueMark + 1, m_rctColorBar.GetTop() + 1),
        wxSize(1, m_rctColorBar.GetHeight() - 2));
    pDC.DrawRectangle(wxPoint(m_rctSatValWash.GetLeft() + nValMark + 1, m_rctSatValWash.GetTop() + 1),
        wxSize(1, m_rctSatValWash.GetHeight() - 2));
    pDC.DrawRectangle(wxPoint(m_rctSatValWash.GetLeft() + 1, m_rctSatValWash.GetTop() + nSatMark + 1),
        wxSize(m_rctSatValWash.GetWidth() - 2, 1));
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::PaintSelections(wxDC& pDC)
{
    // draw background before foreground to avoid need for ExcludeClipRect
    PaintCell(pDC, m_rctBackColor, m_crBack);
#if 0   // TODO:  DrawEdge
    DrawEdge(pDC.m_hDC, m_rctBackColor, EDGE_SUNKEN, BF_RECT);
#else
    pDC.SetPen(*wxBLACK_PEN);
    pDC.SetBrush(*wxTRANSPARENT_BRUSH);
    pDC.DrawRectangle(m_rctBackColor);
#endif

    PaintCell(pDC, m_rctForeColor, m_crFore);
#if 0   // TODO:  DrawEdge
    DrawEdge(pDC.m_hDC, m_rctForeColor, EDGE_RAISED, BF_RECT);
#else
    pDC.SetPen(*wxBLACK_PEN);
    pDC.SetBrush(*wxTRANSPARENT_BRUSH);
    pDC.DrawRectangle(m_rctForeColor);
#endif

    if (m_crTrans != wxNullColour)
    {
        PaintCell(pDC, m_rctTrans, m_crTrans);
#if 0   // TODO:  DrawEdge
        DrawEdge(pDC.m_hDC, m_rctTrans, EDGE_BUMP, BF_RECT);
#else
        pDC.SetPen(*wxBLACK_PEN);
        pDC.SetBrush(*wxTRANSPARENT_BRUSH);
        pDC.DrawRectangle(m_rctTrans);
#endif
    }
}

/////////////////////////////////////////////////////////////////////////////
// xLoc and yLoc must have been DPI scaled to logical coords.

void CColorPalette::PaintCellGroup(wxDC& pDC, const wxColour* pArray, int xLoc, int yLoc)
{
    int x = xLoc;
    int y = yLoc;
    int nCols = pArray == acrStdColors ? cellStdArrayCols : cellCustArrayCols;
    int sizeCellGapFromDip = FromDIP(sizeCellGap);
    wxSize sizeCellFromDip(FromDIP(wxSize(sizeColorCell, sizeColorCell)));

    for (int nCol = 0; nCol < nCols; nCol++)
    {
        for (int nRow = 0; nRow < cellArrayRows; nRow++)
        {
            wxRect rct(wxPoint(x, y), sizeCellFromDip);
            const wxColour& cref = CellColor(pArray, nCol, nRow);
            if (cref != wxNullColour)
            {
                wxBrush brColor(cref);
                pDC.SetBrush(brColor);
                pDC.SetPen(*wxBLACK_PEN);
                pDC.DrawRectangle(rct);
            }
            else
            {
                pDC.SetPen(*wxGREY_PEN);
                pDC.SetBrush(*wxTRANSPARENT_BRUSH);
                pDC.DrawRectangle(rct);
            }
            y += sizeCellFromDip.GetHeight() + sizeCellGapFromDip;
        }
        x += sizeCellFromDip.GetWidth() + sizeCellGapFromDip;
        y = yLoc;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::PaintCell(wxDC& pDC, const wxRect& rct, wxColour cref)
{
    if (!CB::RectVisible(pDC, rct))
        return;

    wxBrush brColor;
    if (cref != wxNullColour)
        brColor.SetColour(cref);
    else
    {
        // appears that wx always treats hatch brushes as transparent?
        pDC.SetPen(*wxTRANSPARENT_PEN);
        pDC.SetBrush(*wxWHITE_BRUSH);
        pDC.DrawRectangle(rct);
        brColor = wxBrush(*wxBLACK, wxBRUSHSTYLE_CROSSDIAG_HATCH);
    }

    pDC.SetBrush(brColor);
    pDC.SetPen(*wxBLACK_PEN);
    pDC.DrawRectangle(rct);
}

/////////////////////////////////////////////////////////////////////////////

wxColour* CColorPalette::MapMouseToColorCell(wxColour* pArray, wxPoint pntClient, const wxRect& rctArray)
{
    if (!rctArray.Contains(pntClient))
        return NULL;
    // x & y have already been DIP to logical coord mapped.
    int x = rctArray.GetLeft();
    int y = rctArray.GetTop();
    int nCols = pArray == acrStdColors ? cellStdArrayCols : cellCustArrayCols;
    wxSize sizeColorCellFromDIP = FromDIP(wxSize(sizeColorCell, sizeColorCell));
    int xyStep = FromDIP(sizeColorCell + sizeCellGap);
    for (int nCol = 0; nCol < nCols; nCol++)
    {
        for (int nRow = 0; nRow < cellArrayRows; nRow++)
        {
            wxRect rct(wxPoint(x, y), sizeColorCellFromDIP);
            if (rct.Contains(pntClient))
                return &CellColor(pArray, nCol, nRow);
            y += xyStep;
        }
        x += xyStep;
        y = rctArray.GetTop();
    }
    return NULL;
}

void CColorPalette::MapHSVtoPixelLoc(int& pnHLoc, int& pnSLoc, int& pnVLoc) const
{
    pnHLoc = (m_nHue * (m_rctColorBar.GetWidth() - 2)) / 359;
    pnSLoc = ((255 - m_nSat) * (m_rctSatValWash.GetWidth() - 2)) / 255;
    pnVLoc = (m_nVal * (m_rctSatValWash.GetHeight() - 2)) / 255;
}

BOOL CColorPalette::MapMouseLocToH(wxPoint pntClient, int& nH, BOOL bCheckValidPoint) const
{
    if (bCheckValidPoint && !m_rctColorBar.Contains(pntClient))
        return FALSE;
    nH = (359 * (pntClient.x - m_rctColorBar.GetLeft() + 1)) / (m_rctColorBar.GetWidth() - 2);
    if (nH < 0) nH = 0;
    if (nH > 359) nH = 359;
    return m_rctColorBar.Contains(pntClient);
}

BOOL CColorPalette::MapMouseLocToSV(wxPoint pntClient, int& nS, int& nV, BOOL bCheckValidPoint) const
{
    if (bCheckValidPoint && !m_rctSatValWash.Contains(pntClient))
        return FALSE;
    nS = 255 - (255 * (pntClient.y - m_rctSatValWash.GetTop() + 1)) / (m_rctSatValWash.GetHeight() - 2);
    if (nS < 0) nS = 0;
    if (nS > 255) nS = 255;
    nV = (255 * (pntClient.x - m_rctSatValWash.GetLeft() + 1)) / (m_rctSatValWash.GetWidth() - 2);
    if (nV < 0) nV = 0;
    if (nV > 255) nV = 255;
    return m_rctSatValWash.Contains(pntClient);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::GenerateSVWash(BOOL bInvalidate /* = TRUE */)
{
    m_bmapWash = CreateRGBSaturationValueWash(m_nHue, m_rctSatValWash.GetWidth() - 2,
        m_rctSatValWash.GetHeight() - 2);
    if (bInvalidate)
        RefreshRect(m_rctSatValWash, FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::SetLineWidth(UINT nLineWidth)
{
    if (nLineWidth > numLineWidths)
        nLineWidth = (unsigned)-1;
    if (m_comboLine->GetSelection() != (int)nLineWidth)
        m_comboLine->SetSelection(nLineWidth);
}

void CColorPalette::SetIDColor(int nID, wxColour cr)
{
    if (nID == XRCID("ID_COLOR_FOREGROUND"))
    {
        if (cr == m_crFore) return;
        m_crFore = cr;
    }
    else if (nID == XRCID("ID_COLOR_BACKGROUND"))
    {
        if (cr == m_crBack) return;
        m_crBack = cr;
    }
    else if (nID == XRCID("ID_COLOR_TRANSPARENT"))
    {
        if (cr == m_crTrans) return;
        m_crTrans = cr;
        if (m_crTrans == wxNullColour)
            m_toolTip.Delete(*this, m_rctTrans);
        else
            SetupToolTip(m_rctTrans, CB::string::LoadString(IDS_TT_TRANSCOLOR));
        RefreshRect(m_rctNoColor);
    }
    else
        return;
    UpdateCurrentColors(FALSE);
}

void CColorPalette::SetCustomColors(const std::vector<wxColour>& pCustColors)
{
    if (m_pCustColors == pCustColors)
        return;
    m_pCustColors = pCustColors;
    RefreshRect(m_rctCustColors, FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::UpdateCurrentColors(BOOL bImmediate)
{
    wxRect rct = GetClientRect();
    rct.SetRight(FromDIP(sizeCellGap * 2 + sizeLeftMarg - 1));
    rct.SetBottom(FromDIP((rct.GetBottom() * 5) / 8));              // Miss combo box.
    RefreshRect(rct, FALSE);

    if (bImmediate)
        Update();
}

void CColorPalette::UpdateCurrentColorMix(BOOL bUpdate /* = TRUE*/)
{
    wxColour cref = CB::Convert(HSVtoRGB(m_nHue, m_nSat, m_nVal));
    m_crCurMix = cref;
    if (bUpdate)
        RefreshRect(m_rctColorMix, FALSE);
}

/////////////////////////////////////////////////////////////////////////////

/* static */
std::vector<wxColour> CColorPalette::CustomColorsAllocate()
{
    return std::vector<wxColour>(cellCustArraySize);
}

/* static */
void CColorPalette::CustomColorsSerialize(CArchive& ar, std::vector<wxColour>& pCustColors)
{
    wxColour* crTbl = pCustColors.data();
    if (ar.IsStoring())
    {
        ar << value_preserving_cast<uint16_t>(cellCustArraySize);
        for (int i = 0; i < cellCustArraySize; i++)
            ar << static_cast<uint32_t>(CB::Convert(crTbl[i]));
    }
    else
    {
        uint16_t wCount;
        uint32_t dwColor;

        ar >> wCount;
        for (int i = 0; i < wCount; i++)
        {
            ar >> dwColor;
            crTbl[i] = CB::Convert(static_cast<COLORREF>(dwColor));
        }
    }
}

/* static */
void CColorPalette::CustomColorsClear(std::vector<wxColour>& pCustColors)
{
    wxColour* crTbl = pCustColors.data();
    for (int i = 0; i < cellCustArraySize; i++)
        crTbl[i] = wxNullColour;
}

/////////////////////////////////////////////////////////////////////////////

BOOL CColorPalette::HandleButtonDown(const wxMouseEvent& event)
{
    const wxPoint& point = event.GetPosition();
    wxColour* pCRef = NULL;
    if (MapMouseLocToH(point, m_nHue))
    {
        GenerateSVWash();
        UpdateCurrentColorMix(TRUE);
        RefreshRect(m_rctColorBar, FALSE);   // So select line is updated
        if (event.LeftDown())
        {
            CaptureMouse();
            m_bTrackHue = TRUE;
        }
    }
    else if (MapMouseLocToSV(point, m_nSat, m_nVal))
    {
        UpdateCurrentColorMix(TRUE);
        RefreshRect(m_rctSatValWash, FALSE); // So select lines are updated
        if (event.LeftDown())
        {
            CaptureMouse();
            m_bTrackSV = TRUE;
        }
    }
    else if (m_rctColorMix.Contains(point))
    {
        NotifyColorChange(event, m_crCurMix);
    }
    else if (m_rctNoColor.Contains(point))
    {
        NotifyColorChange(event, wxNullColour);
    }
    else if (m_crTrans != wxNullColour && m_rctTrans.Contains(point))
    {
        NotifyColorChange(event, m_crTrans);
    }
    else if ((pCRef = MapMouseToColorCell(acrStdColors, point, m_rctStdColors)) != NULL)
    {
        NotifyColorChange(event, *pCRef);
    }
    else if ((pCRef = MapMouseToColorCell(m_pCustColors.data(), point, m_rctCustColors)) != NULL)
    {
        if (mouseSetCustomFromFore(event))
        {
            if (m_crFore != wxNullColour)
            {
                *pCRef = m_crFore;
                RefreshRect(m_rctCustColors, FALSE);
                NotifyCustomColorChange(m_pCustColors);
            }
        }
        else if (mouseSetCustomFromBack(event))
        {
            if (m_crBack != wxNullColour)
            {
                *pCRef = m_crBack;
                RefreshRect(m_rctCustColors, FALSE);
                NotifyCustomColorChange(m_pCustColors);
            }
        }
        else if (mouseSetCustomFromMix(event))
        {
            *pCRef = m_crCurMix;
            RefreshRect(m_rctCustColors, FALSE);
            NotifyCustomColorChange(m_pCustColors);
        }
        else if (mouseSetCustomClear(event))
        {
            *pCRef = wxNullColour;
            RefreshRect(m_rctCustColors, TRUE);
            NotifyCustomColorChange(m_pCustColors);
        }
        else if (*pCRef != wxNullColour)
            NotifyColorChange(event, *pCRef);
    }
    else
        return FALSE;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::NotifyColorChange(const wxMouseEvent& event, wxColour cref)
{
    wxView* pView = GetMainFrame()->GetActiveView();

    if (pView == NULL)
        return;

    if (mouseFore(event))
    {
        SetColorEvent event2(XRCID("ID_COLOR_FOREGROUND"), cref);
        pView->ProcessEvent(event2);
    }
    else if (mouseBack1(event) ||
        mouseBack2(event))
    {
        SetColorEvent event2(XRCID("ID_COLOR_BACKGROUND"), cref);
        pView->ProcessEvent(event2);
    }
}

void CColorPalette::NotifyCustomColorChange(const std::vector<wxColour>& pcrCustomColors)
{
    wxView* pView = GetMainFrame()->GetActiveView();

    if (pView == NULL)
        return;
    SetCustomColorEvent event2(pcrCustomColors);
    pView->ProcessEvent(event2);
}

/////////////////////////////////////////////////////////////////////////////


void CColorPalette::OnLButtonDblClk(wxMouseEvent& event)
{
    if (m_rctCustColors.Contains(event.GetPosition()))
    {
        // Double clicking in a color cell allows the user to
        // explicitly set the color using a color dialog.
        wxColour* pCRef = NULL;
        if ((pCRef = MapMouseToColorCell(m_pCustColors.data(), event.GetPosition(), m_rctCustColors)) == NULL)
            return;         // Just the mouse hit
        wxColour cr = *pCRef == wxNullColour ? *wxBLACK : *pCRef;

        wxColourData colourData;
        colourData.SetColour(cr);
        wxColourDialog dlg(this, &colourData);
        if (dlg.ShowModal() == wxID_OK)
        {
            *pCRef = dlg.GetColourData().GetColour();
            RefreshRect(m_rctCustColors, FALSE);
            NotifyCustomColorChange(m_pCustColors);
        }
    }
    else    // Let control bar processing handle it
        event.Skip();
}

void CColorPalette::OnLButtonDown(wxMouseEvent& event)
{
    if (!HandleButtonDown(event))
        event.Skip();
}

void CColorPalette::OnRButtonDown(wxMouseEvent& event)
{
    if (!HandleButtonDown(event))
    {
        event.Skip();
    }
    else
    {
        // in case user drags mouse to another window before RButtonUp
        CaptureMouse();
        m_bIgnoreRButtonUp = TRUE;
    }
}

void CColorPalette::OnMouseMove(wxMouseEvent& event)
{
    wxPoint point = event.GetPosition();
    if (GetCapture() == this)
    {
        if (m_bTrackHue)
        {
            MapMouseLocToH(point, m_nHue, FALSE);
            UpdateCurrentColorMix(TRUE);
            GenerateSVWash();
            RefreshRect(m_rctColorBar, FALSE);   // So select line is updated
        }
        else if (m_bTrackSV)
        {
            MapMouseLocToSV(point, m_nSat, m_nVal, FALSE);
            UpdateCurrentColorMix(TRUE);
            RefreshRect(m_rctSatValWash, FALSE); // So select lines are updated
        }
    }
    event.Skip();
}

void CColorPalette::OnLButtonUp(wxMouseEvent& event)
{
    if (m_bTrackHue || m_bTrackSV)
    {
        m_bTrackHue = FALSE;
        m_bTrackSV = FALSE;
        ReleaseMouse();
    }

    event.Skip();
}

void CColorPalette::OnRButtonUp(wxMouseEvent& event)
{
    if (m_bIgnoreRButtonUp)
    {
        m_bIgnoreRButtonUp = FALSE;
        ReleaseMouse();
        return;
    }

    event.Skip();
}

void CColorPalette::OnMouseCaptureLost(wxMouseCaptureLostEvent& /*event*/)
{
    m_bTrackHue = FALSE;
    m_bTrackSV = FALSE;
    m_bIgnoreRButtonUp = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::OnLineWidthCbnSelchange(wxCommandEvent& /*event*/)
{
    wxView* pView = GetMainFrame()->GetActiveView();
    SetLineWidthEvent event2(m_comboLine->GetSelection());
    pView->ProcessEvent(event2);
}

#if 0
BOOL CColorPalette::OnHelpInfo(HELPINFO* pHelpInfo)
{
    GetApp()->DoHelpTopic("gm-ref-pal-color.htm");
    return TRUE;
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CColorPalette::OnPaletteHide(wxCommandEvent& /*event*/)
{
#if 0
    GetMainFrame()->SendMessage(WM_COMMAND, ID_WINDOW_COLORPAL);
#else
    AfxThrowNotSupportedException();
#endif
}

