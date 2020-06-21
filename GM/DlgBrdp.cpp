// DlgBrdp.cpp : implementation file
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
#include    "Gm.h"
#include    "BrdCell.h"
#include    "CellForm.h"
#include    "GdiTools.h"
#include    "DlgBrdp.h"
#include    "DlgBrdsz.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardPropDialog dialog


CBoardPropDialog::CBoardPropDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CBoardPropDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CBoardPropDialog)
    m_bCellLines = FALSE;
    m_bGridSnap = FALSE;
    m_bTrackCellNum = FALSE;
    m_nRowTrkOffset = 0;
    m_nColTrkOffset = 0;
    m_fXGridSnapOff = 0.0f;
    m_fYGridSnapOff = 0.0f;
    m_fXGridSnap = 16.0f;
    m_fYGridSnap = 16.0f;
    m_strName = "";
    m_nStyleNum = -1;
    m_bColTrkInvert = FALSE;
    m_bRowTrkInvert = FALSE;
    m_bCellBorderOnTop = FALSE;
    m_bEnableXParentCells = FALSE;
    //}}AFX_DATA_INIT

    m_xGridSnap = 16;
    m_yGridSnap = 16;
    m_xGridSnapOff = 0;
    m_yGridSnapOff = 0;
    m_bShapeChanged = FALSE;
    m_crCellFrame = RGB(0, 0, 0);
    m_eCellStyle = cformRect;
}

void CBoardPropDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CBoardPropDialog)
    DDX_Control(pDX, IDC_D_BRDPRP_HALF_PXLWIDTH, m_staticHalfPixelWidth);
    DDX_Control(pDX, IDC_D_BRDPRP_SMALL_PXLWIDTH, m_staticSmallPixelWidth);
    DDX_Control(pDX, IDC_D_BRDPRP_SMALL_PXLHEIGHT, m_staticSmallPixelHeight);
    DDX_Control(pDX, IDC_D_BRDPRP_HALF_PXLHEIGHT, m_staticHalfPixelHeight);
    DDX_Control(pDX, IDC_D_BRDPRP_PXLWIDTH, m_staticPixelWidth);
    DDX_Control(pDX, IDC_D_BRDPRP_PXLHEIGHT, m_staticPixelHeight);
    DDX_Control(pDX, IDC_D_BRDPRP_BOARDNAME, m_editBrdName);
    DDX_Control(pDX, IDC_D_BRDPRP_CELLCOLOR, m_cpCellFrame);
    DDX_Control(pDX, IDC_D_BRDPRP_WIDTH, m_staticWidth);
    DDX_Control(pDX, IDC_D_BRDPRP_HEIGHT, m_staticHeight);
    DDX_Control(pDX, IDC_D_BRDPRP_ROWS, m_staticRows);
    DDX_Control(pDX, IDC_D_BRDPRP_COLS, m_staticCols);
    DDX_Control(pDX, IDC_D_BRDPRP_TRKSTYLE, m_comboStyle);
    DDX_Check(pDX, IDC_D_BRDPRP_CELLBORDER, m_bCellLines);
    DDX_Check(pDX, IDC_D_BRDPRP_SNAPON, m_bGridSnap);
    DDX_Check(pDX, IDC_D_BRDPRP_TRACK, m_bTrackCellNum);
    DDX_Text(pDX, IDC_D_BRDPRP_TRKROFF, m_nRowTrkOffset);
    DDV_MinMaxInt(pDX, m_nRowTrkOffset, -250, 250);
    DDX_Text(pDX, IDC_D_BRDPRP_TRKCOFF, m_nColTrkOffset);
    DDV_MinMaxInt(pDX, m_nColTrkOffset, -250, 250);
    DDX_Text(pDX, IDC_D_BRDPRP_XOFFSET, m_fXGridSnapOff);
    DDV_MinMaxFloat(pDX, m_fXGridSnapOff, 0.f, 255.999f);
    DDX_Text(pDX, IDC_D_BRDPRP_YOFFSET, m_fYGridSnapOff);
    DDV_MinMaxFloat(pDX, m_fYGridSnapOff, 0.f, 255.999f);
    DDX_Text(pDX, IDC_D_BRDPRP_XPIXELS, m_fXGridSnap);
    DDV_MinMaxFloat(pDX, m_fXGridSnap, 2.f, 256.f);
    DDX_Text(pDX, IDC_D_BRDPRP_YPIXELS, m_fYGridSnap);
    DDV_MinMaxFloat(pDX, m_fYGridSnap, 2.f, 256.f);
    DDX_Text(pDX, IDC_D_BRDPRP_BOARDNAME, m_strName);
    DDV_MaxChars(pDX, m_strName, 32);
    DDX_CBIndex(pDX, IDC_D_BRDPRP_TRKSTYLE, m_nStyleNum);
    DDX_Check(pDX, IDC_D_BRDPRP_INVCOLS, m_bColTrkInvert);
    DDX_Check(pDX, IDC_D_BRDPRP_INVROWS, m_bRowTrkInvert);
    DDX_Check(pDX, IDC_D_BRDPRP_BORDERTOP, m_bCellBorderOnTop);
    DDX_Check(pDX, IDC_D_BRDPRP_XPARENTCELLS, m_bEnableXParentCells);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBoardPropDialog, CDialog)
    //{{AFX_MSG_MAP(CBoardPropDialog)
    ON_BN_CLICKED(IDC_D_BRDPRP_RESHAPE, OnReshape)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_BRDPRP_BOARDNAME, IDH_D_BRDPRP_BOARDNAME,
    IDC_D_BRDPRP_BORDERTOP, IDH_D_BRDPRP_BORDERTOP,
    IDC_D_BRDPRP_CELLBORDER, IDH_D_BRDPRP_CELLBORDER,
    IDC_D_BRDPRP_CELLCOLOR, IDH_D_BRDPRP_CELLCOLOR,
    IDC_D_BRDPRP_COLS, IDH_D_BRDPRP_COLS,
    IDC_D_BRDPRP_HEIGHT, IDH_D_BRDPRP_HEIGHT,
    IDC_D_BRDPRP_INVCOLS, IDH_D_BRDPRP_INVCOLS,
    IDC_D_BRDPRP_INVROWS, IDH_D_BRDPRP_INVROWS,
    IDC_D_BRDPRP_PXLHEIGHT, IDH_D_BRDPRP_PXLHEIGHT,
    IDC_D_BRDPRP_PXLWIDTH, IDH_D_BRDPRP_PXLWIDTH,
    IDC_D_BRDPRP_RESHAPE, IDH_D_BRDPRP_RESHAPE,
    IDC_D_BRDPRP_ROWS, IDH_D_BRDPRP_ROWS,
    IDC_D_BRDPRP_SNAPON, IDH_D_BRDPRP_SNAPON,
    IDC_D_BRDPRP_TRACK, IDH_D_BRDPRP_TRACK,
    IDC_D_BRDPRP_TRKCOFF, IDH_D_BRDPRP_TRKCOFF,
    IDC_D_BRDPRP_TRKROFF, IDH_D_BRDPRP_TRKROFF,
    IDC_D_BRDPRP_TRKSTYLE, IDH_D_BRDPRP_TRKSTYLE,
    IDC_D_BRDPRP_WIDTH, IDH_D_BRDPRP_WIDTH,
    IDC_D_BRDPRP_XOFFSET, IDH_D_BRDPRP_XOFFSET,
    IDC_D_BRDPRP_XPARENTCELLS, IDH_D_BRDPRP_XPARENTCELLS,
    IDC_D_BRDPRP_XPIXELS, IDH_D_BRDPRP_XPIXELS,
    IDC_D_BRDPRP_YOFFSET, IDH_D_BRDPRP_YOFFSET,
    IDC_D_BRDPRP_YPIXELS, IDH_D_BRDPRP_YPIXELS,
    0,0
};

BOOL CBoardPropDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CBoardPropDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CBoardPropDialog message handlers

void CBoardPropDialog::OnOK()
{
    CString str;
    m_editBrdName.GetWindowText(str);
    if (str.IsEmpty())
    {
        AfxMessageBox(IDS_ERR_BOARDNAME, MB_OK | MB_ICONEXCLAMATION);
        m_editBrdName.SetFocus();
        return;
    }

    m_crCellFrame = m_cpCellFrame.GetColor();

    CDialog::OnOK();

    // Make sure these are within grid size

    m_xGridSnapOff = (UINT)(m_fXGridSnapOff * 1000 + 0.5);
    m_yGridSnapOff = (UINT)(m_fYGridSnapOff * 1000 + 0.5);
    m_xGridSnap = (UINT)(m_fXGridSnap * 1000 + 0.5);
    m_yGridSnap = (UINT)(m_fYGridSnap * 1000 + 0.5);

    ASSERT(m_xGridSnap > 0 && m_yGridSnap > 0);
    if (m_xGridSnap > 0)
        m_xGridSnapOff = m_xGridSnapOff % m_xGridSnap;
    if (m_yGridSnap > 0)
        m_yGridSnapOff = m_yGridSnapOff % m_yGridSnap;
}

BOOL CBoardPropDialog::OnInitDialog()
{
    m_fXGridSnapOff = (float)m_xGridSnapOff / 1000;
    m_fYGridSnapOff = (float)m_yGridSnapOff / 1000;
    m_fXGridSnap = (float)m_xGridSnap / 1000;
    m_fYGridSnap = (float)m_yGridSnap / 1000;

    CDialog::OnInitDialog();

    UpdateInfoArea();

    m_cpCellFrame.SetColor(m_crCellFrame);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBoardPropDialog::OnReshape()
{
    if (AfxMessageBox(IDS_WARN_RESHAPE, MB_OKCANCEL |
        MB_ICONEXCLAMATION) == IDOK)
    {
        CBoardReshapeDialog dlg;

        dlg.m_eCellStyle = m_eCellStyle;
        dlg.m_nRows = m_nRows;
        dlg.m_nCols = m_nCols;
        dlg.m_nCellHt = m_nCellHt;
        dlg.m_nCellWd = m_nCellWd;
        dlg.m_bStaggerIn = m_bStaggerIn;

        if (dlg.DoModal() == IDOK)
        {
            m_bShapeChanged = TRUE;

            m_nRows = dlg.m_nRows;
            m_nCols = dlg.m_nCols;
            m_nCellHt = dlg.m_nCellHt;
            m_nCellWd = dlg.m_nCellWd;

            m_bStaggerIn = dlg.m_bStaggerIn;

            UpdateInfoArea();
        }
    }
}

//////////////////////////////////////////////////////////////////

void CBoardPropDialog::UpdateInfoArea()
{
    char szNum[40];
    _itoa(m_nRows, szNum, 10);
    m_staticRows.SetWindowText(szNum);
    _itoa(m_nCols, szNum, 10);
    m_staticCols.SetWindowText(szNum);

    if (m_bShapeChanged && m_eCellStyle == cformHexPnt)
        strcpy(szNum, "??");
    else
        _itoa(m_nCellHt, szNum, 10);
    m_staticHeight.SetWindowText(szNum);

    if (m_bShapeChanged && m_eCellStyle == cformHexFlat)
        strcpy(szNum, "??");
    else
        _itoa(m_nCellWd, szNum, 10);

    m_staticWidth.SetWindowText(szNum);

    CCellForm cfFull;
    CCellForm cfHalf;
    CCellForm cfSmall;

    CBoardArray::GenerateCellDefs(m_eCellStyle,
        m_eCellStyle == cformHexPnt ? m_nCellWd : m_nCellHt, m_nCellWd, m_bStaggerIn,
        cfFull, cfHalf, cfSmall);

    CSize size = cfFull.CalcBoardSize(m_nRows, m_nCols);
    _itoa(size.cx, szNum, 10);
    m_staticPixelWidth.SetWindowText(szNum);
    _itoa(size.cy, szNum, 10);
    m_staticPixelHeight.SetWindowText(szNum);

    size = cfHalf.CalcBoardSize(m_nRows, m_nCols);
    _itoa(size.cx, szNum, 10);
    m_staticHalfPixelWidth.SetWindowText(szNum);
    _itoa(size.cy, szNum, 10);
    m_staticHalfPixelHeight.SetWindowText(szNum);

    size = cfSmall.CalcBoardSize(m_nRows, m_nCols);
    _itoa(size.cx, szNum, 10);
    m_staticSmallPixelWidth.SetWindowText(szNum);
    _itoa(size.cy, szNum, 10);
    m_staticSmallPixelHeight.SetWindowText(szNum);
}

