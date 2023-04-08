// DlgTilsz.cpp : implementation file
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
#include    "Board.h"
#include    "DlgTilsz.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizeTileDialog dialog


CResizeTileDialog::CResizeTileDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CResizeTileDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CResizeTileDialog)
    m_bRescaleBMaps = FALSE;
    m_nWidth = 0;
    m_nHeight = 0;
    //}}AFX_DATA_INIT
    m_nHalfWidth = 0;
    m_nHalfHeight = 0;
    m_pBMgr = NULL;
}

void CResizeTileDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CResizeTileDialog)
    DDX_Control(pDX, IDC_D_RETILE_TILEHT, m_editHeight);
    DDX_Control(pDX, IDC_D_RETILE_TILEWD, m_editWidth);
    DDX_Control(pDX, IDC_D_RETILE_CURWD, m_staticCurWd);
    DDX_Control(pDX, IDC_D_RETILE_CURHT, m_staticCurHt);
    DDX_Control(pDX, IDC_D_RETILE_BOARDNAME, m_comboBrdName);
    DDX_Check(pDX, IDC_D_RETILE_RESCALE, m_bRescaleBMaps);
    DDX_Text(pDX, IDC_D_RETILE_TILEWD, m_nWidth);
    DDV_MinMaxUInt(pDX, m_nWidth, 2, 512);
    DDX_Text(pDX, IDC_D_RETILE_TILEHT, m_nHeight);
    DDV_MinMaxUInt(pDX, m_nHeight, 2, 512);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CResizeTileDialog, CDialog)
    //{{AFX_MSG_MAP(CResizeTileDialog)
    ON_CBN_SELCHANGE(IDC_D_RETILE_BOARDNAME, OnSelchangeBrdName)
    ON_EN_CHANGE(IDC_D_RETILE_TILEWD, OnChangeTileWd)
    ON_EN_CHANGE(IDC_D_RETILE_TILEHT, OnChangeTileHt)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_RETILE_BOARDNAME, IDH_D_RETILE_BOARDNAME,
    IDC_D_RETILE_CURHT, IDH_D_RETILE_CURHT,
    IDC_D_RETILE_CURWD, IDH_D_RETILE_CURWD,
    IDC_D_RETILE_RESCALE, IDH_D_RETILE_RESCALE,
    IDC_D_RETILE_TILEHT, IDH_D_RETILE_TILEHT,
    IDC_D_RETILE_TILEWD, IDH_D_RETILE_TILEWD,
    0,0
};

BOOL CResizeTileDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CResizeTileDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CResizeTileDialog message handlers

BOOL CResizeTileDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(m_pBMgr != NULL);

    for (size_t i = 0; i < m_pBMgr->GetNumBoards(); i++)
        m_comboBrdName.AddString(m_pBMgr->GetBoard(i).GetName());
    m_comboBrdName.SetCurSel(-1);

    CB::string szNum = std::format("{}", m_nHeight);
    m_staticCurHt.SetWindowText(szNum);
    szNum = std::format("{}", m_nWidth);
    m_staticCurWd.SetWindowText(szNum);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CResizeTileDialog::OnSelchangeBrdName()
{
    int nBrd = m_comboBrdName.GetCurSel();
    if (nBrd >= 0)
    {
        CBoard& pBoard = m_pBMgr->GetBoard(value_preserving_cast<size_t>(nBrd));

        CSize size = pBoard.GetCellSize(fullScale);

        CB::string szNum = std::format("{}", size.cx);
        m_editWidth.SetWindowText(szNum);
        szNum = std::format("{}", size.cy);
        m_editHeight.SetWindowText(szNum);

        size = pBoard.GetCellSize(halfScale);
        m_nHalfWidth = size.cx;
        m_nHalfHeight = size.cy;
    }
}

void CResizeTileDialog::OnChangeTileWd()
{
    m_nHalfWidth = 0;
}

void CResizeTileDialog::OnChangeTileHt()
{
    m_nHalfHeight = 0;
}

void CResizeTileDialog::OnOK()
{
    CDialog::OnOK();
    // If not set by the board, calc half size tile.
    if (m_nHalfWidth == 0)
        m_nHalfWidth = m_nWidth / 2 + 1;
    if (m_nHalfHeight == 0)
        m_nHalfHeight = m_nHeight / 2 + 1;
}
