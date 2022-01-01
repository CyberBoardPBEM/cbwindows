// dlgbrdsz.cpp : implementation file
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
#include    "DlgBrdsz.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardReshapeDialog dialog


CBoardReshapeDialog::CBoardReshapeDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CBoardReshapeDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CBoardReshapeDialog)
    m_nCellHt = 0;
    m_nCellWd = 0;
    m_nCols = size_t(0);
    m_nRows = size_t(0);
    m_bStagger = CellStagger::Invalid;
    //}}AFX_DATA_INIT
    m_eCellStyle = cformRect;
}

void CBoardReshapeDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CBoardReshapeDialog)
    DDX_Control(pDX, IDC_D_BRDSHP_STAGGERIN, m_checkStaggerIn);
    DDX_Control(pDX, IDC_D_BRDSHP_CELLWD, m_editCellWd);
    DDX_Control(pDX, IDC_D_BRDSHP_CELLHT, m_editCellHt);
    DDX_Text(pDX, IDC_D_BRDSHP_CELLHT, m_nCellHt);
    DDV_MinMaxUInt(pDX, m_nCellHt, 4, 32000);
    DDX_Text(pDX, IDC_D_BRDSHP_CELLWD, m_nCellWd);
    DDV_MinMaxUInt(pDX, m_nCellWd, 4, 32000);
    DDX_Text(pDX, IDC_D_BRDSHP_COLS, m_nCols);
    DDV_MinMaxUInt(pDX, value_preserving_cast<UINT>(m_nCols), 1, 1000);
    DDX_Text(pDX, IDC_D_BRDSHP_ROWS, m_nRows);
    DDV_MinMaxUInt(pDX, value_preserving_cast<UINT>(m_nRows), 1, 1000);
    int temp = static_cast<int>(m_bStagger);
    DDX_Check(pDX, IDC_D_BRDSHP_STAGGERIN, temp);
    m_bStagger = static_cast<CellStagger>(temp);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBoardReshapeDialog, CDialog)
    //{{AFX_MSG_MAP(CBoardReshapeDialog)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_BRDSHP_CELLHT, IDH_D_BRDSHP_CELLHT,
    IDC_D_BRDSHP_CELLWD, IDH_D_BRDSHP_CELLWD,
    IDC_D_BRDSHP_COLS, IDH_D_BRDSHP_COLS,
    IDC_D_BRDSHP_ROWS, IDH_D_BRDSHP_ROWS,
    IDC_D_BRDSHP_STAGGERIN, IDH_D_BRDSHP_STAGGERIN,
    0,0
};

BOOL CBoardReshapeDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CBoardReshapeDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CBoardReshapeDialog message handlers

BOOL CBoardReshapeDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_eCellStyle == cformHexPnt)
        m_editCellHt.EnableWindow(FALSE);
    else if (m_eCellStyle == cformHexFlat)
        m_editCellWd.EnableWindow(FALSE);
    else if (m_eCellStyle == cformRect)
        m_checkStaggerIn.EnableWindow(FALSE); // disable stagger
    return TRUE;  // return TRUE  unless you set the focus to a control
}
