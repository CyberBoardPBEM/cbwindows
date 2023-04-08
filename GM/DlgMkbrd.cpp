// DlbMkbrd.cpp
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
#include    "CellForm.h"
#include    "DlgMkbrd.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridType dialog

CGridType::CGridType(CWnd* pParent /*=NULL*/)
    : CDialog(CGridType::IDD, pParent)
{
    //{{AFX_DATA_INIT(CGridType)
    m_iCellWd = 0;
    m_iCellHt = 0;
    m_iCols = size_t(0);
    m_iRows = size_t(0);
    m_strBoardName = "";
    m_bStagger = CellStagger::Invalid;
    m_nBoardType = -1;
    //}}AFX_DATA_INIT
}

void CGridType::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGridType)
    DDX_Control(pDX, IDC_D_NEWBRD_PIXSIZE, m_staticPixelSize);
    DDX_Control(pDX, IDC_D_NEWBRD_STAGGERIN, m_chkStagger);
    DDX_Control(pDX, IDC_D_NEWBRD_GRIDROWS, m_editRows);
    DDX_Control(pDX, IDC_D_NEWBRD_GRIDCOLS, m_editCols);
    DDX_Control(pDX, IDC_D_NEWBRD_CELLWIDTH, m_editCellWd);
    DDX_Control(pDX, IDC_D_NEWBRD_CELLHEIGHT, m_editCellHt);
    DDX_Text(pDX, IDC_D_NEWBRD_CELLWIDTH, m_iCellWd);
    DDV_MinMaxInt(pDX, m_iCellWd, 4, 32000);
    DDX_Text(pDX, IDC_D_NEWBRD_CELLHEIGHT, m_iCellHt);
    DDV_MinMaxInt(pDX, m_iCellHt, 4, 32000);
    DDX_Text(pDX, IDC_D_NEWBRD_GRIDCOLS, m_iCols);
    DDV_MinMaxUInt(pDX, value_preserving_cast<unsigned>(m_iCols), 1, 1000);
    DDX_Text(pDX, IDC_D_NEWBRD_GRIDROWS, m_iRows);
    DDV_MinMaxUInt(pDX, value_preserving_cast<unsigned>(m_iRows), 1, 1000);
    DDX_Text(pDX, IDC_D_NEWBRD_BOARDNAME, m_strBoardName);
    DDV_MaxChars(pDX, m_strBoardName, 32);
    int temp = static_cast<int>(m_bStagger);
    DDX_Check(pDX, IDC_D_NEWBRD_STAGGERIN, temp);
    m_bStagger = static_cast<CellStagger>(temp);
    DDX_Radio(pDX, IDC_D_NEWBRD_RECT, m_nBoardType);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGridType, CDialog)
    //{{AFX_MSG_MAP(CGridType)
    ON_BN_CLICKED(IDC_D_NEWBRD_HBRICK, OnHBrick)
    ON_BN_CLICKED(IDC_D_NEWBRD_VBRICK, OnVBrick)
    ON_BN_CLICKED(IDC_D_NEWBRD_RECT, OnRectCell)
    ON_BN_CLICKED(IDC_D_NEWBRD_HEXFLAT, OnHexFlat)
    ON_BN_CLICKED(IDC_D_NEWBRD_HEXPNT, OnHexPnt)
    ON_EN_CHANGE(IDC_D_NEWBRD_CELLHEIGHT, OnChangeDNewbrdCellheight)
    ON_EN_CHANGE(IDC_D_NEWBRD_CELLWIDTH, OnChangeDNewbrdCellwidth)
    ON_EN_CHANGE(IDC_D_NEWBRD_GRIDCOLS, OnChangeDNewbrdGridcols)
    ON_EN_CHANGE(IDC_D_NEWBRD_GRIDROWS, OnChangeDNewbrdGridrows)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_NEWBRD_BOARDNAME, IDH_D_NEWBRD_BOARDNAME,
    IDC_D_NEWBRD_CELLHEIGHT, IDH_D_NEWBRD_CELLHEIGHT,
    IDC_D_NEWBRD_CELLWIDTH, IDH_D_NEWBRD_CELLWIDTH,
    IDC_D_NEWBRD_GRIDCOLS, IDH_D_NEWBRD_GRIDCOLS,
    IDC_D_NEWBRD_GRIDROWS, IDH_D_NEWBRD_GRIDROWS,
    IDC_D_NEWBRD_HBRICK, IDH_D_NEWBRD_HBRICK,
    IDC_D_NEWBRD_HEXFLAT, IDH_D_NEWBRD_HEXFLAT,
    IDC_D_NEWBRD_HEXPNT, IDH_D_NEWBRD_HEXPNT,
    IDC_D_NEWBRD_PIXSIZE, IDH_D_NEWBRD_PIXSIZE,
    IDC_D_NEWBRD_RECT, IDH_D_NEWBRD_RECT,
    IDC_D_NEWBRD_STAGGERIN, IDH_D_NEWBRD_STAGGERIN,
    IDC_D_NEWBRD_VBRICK, IDH_D_NEWBRD_VBRICK,
    0,0
};

BOOL CGridType::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CGridType::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CGridType message handlers

void CGridType::OnOK()
{
    UpdateData(TRUE);

    if (m_strBoardName.empty())
    {
        AfxMessageBox(IDS_ERR_BOARDNAME, MB_OK | MB_ICONEXCLAMATION);
        CWnd* pWnd = GetDlgItem(IDC_D_NEWBRD_BOARDNAME);
        ASSERT(pWnd != NULL);
        pWnd->SetFocus();
        return;
    }

    // Now check if board is too large....

    CCellForm cf;           // Use a CellForm to do this
    if (m_nBoardType == cformHexPnt)// Only first param is used in this case
        m_iCellHt = m_iCellWd;

    cf.CreateCell((CellFormType)m_nBoardType, m_iCellHt, m_iCellWd);

    if (!cf.CalcTrialBoardSize(m_iRows, m_iCols))
    {
        AfxMessageBox(IDS_ERR_BOARDSIZE, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    CDialog::OnOK();
}

void CGridType::OnHBrick()
{
    m_editCellHt.EnableWindow(TRUE);
    m_editCellWd.EnableWindow(TRUE);
    m_chkStagger.EnableWindow(TRUE);
    UpdateBoardDimensions();
}

void CGridType::OnVBrick()
{
    m_editCellHt.EnableWindow(TRUE);
    m_editCellWd.EnableWindow(TRUE);
    m_chkStagger.EnableWindow(TRUE);
    UpdateBoardDimensions();
}

void CGridType::OnRectCell()
{
    m_editCellHt.EnableWindow(TRUE);
    m_editCellWd.EnableWindow(TRUE);
    m_chkStagger.EnableWindow(FALSE);
    UpdateBoardDimensions();
}

void CGridType::OnHexFlat()
{
    m_editCellHt.EnableWindow(TRUE);
    m_editCellWd.EnableWindow(FALSE);
    m_chkStagger.EnableWindow(TRUE);
    UpdateBoardDimensions();
}

void CGridType::OnHexPnt()
{
    m_editCellHt.EnableWindow(FALSE);
    m_editCellWd.EnableWindow(TRUE);
    m_chkStagger.EnableWindow(TRUE);
    UpdateBoardDimensions();
}

void CGridType::UpdateBoardDimensions()
{
    m_iCellWd = GetDlgItemInt(IDC_D_NEWBRD_CELLWIDTH);
    m_iCellHt = GetDlgItemInt(IDC_D_NEWBRD_CELLHEIGHT);
    m_iRows = GetDlgItemInt(IDC_D_NEWBRD_GRIDROWS);
    m_iCols = GetDlgItemInt(IDC_D_NEWBRD_GRIDCOLS);

    CDataExchange dx(this, TRUE);
    int temp = static_cast<int>(m_bStagger);
    DDX_Check(&dx, IDC_D_NEWBRD_STAGGERIN, temp);
    m_bStagger = static_cast<CellStagger>(temp);
    DDX_Radio(&dx, IDC_D_NEWBRD_RECT, m_nBoardType);

    if (m_nBoardType == cformRect)
    {
        m_bStagger = CellStagger::Invalid;
    }

    if (m_nBoardType == cformHexPnt)    // Only first param is used
        m_iCellHt = m_iCellWd;

    if (!(m_iRows > size_t(0) && m_iCols > size_t(0) && m_iCellHt >= 4 && m_iCellWd >= 4))
    {
        CB::string str = CB::string::LoadString(IDS_BSIZE_INVALID);
        m_staticPixelSize.SetWindowText(str);
        return;
    }

    CCellForm cf;
    cf.CreateCell((CellFormType)m_nBoardType, m_iCellHt, m_iCellWd,
        m_bStagger);
    if (cf.CalcTrialBoardSize(m_iRows, m_iCols))
    {
        CB::string str = CB::string::LoadString(IDS_BSIZE_PATTERN);
        CSize size = cf.CalcBoardSize(m_iRows, m_iCols);
        CB::string szBfr = std::vformat(str, std::make_wformat_args(size.cy, size.cx));
        m_staticPixelSize.SetWindowText(szBfr);
    }
    else
    {
        CB::string str = CB::string::LoadString(IDS_BSIZE_INVALID);
        m_staticPixelSize.SetWindowText(str);
    }
}

BOOL CGridType::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_nBoardType == 3)      // Hex flat up
        OnHexFlat();
    else if (m_nBoardType == 4)
        OnHexPnt();

    UpdateBoardDimensions();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGridType::OnChangeDNewbrdCellheight()
{
    UpdateBoardDimensions();
}

void CGridType::OnChangeDNewbrdCellwidth()
{
    UpdateBoardDimensions();
}

void CGridType::OnChangeDNewbrdGridcols()
{
    UpdateBoardDimensions();
}

void CGridType::OnChangeDNewbrdGridrows()
{
    UpdateBoardDimensions();
}
