// DlgNtile.cpp : implementation file
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
#include    "DlgNtile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewTileDialog dialog

CNewTileDialog::CNewTileDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CNewTileDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CNewTileDialog)
    m_nHeight = 0;
    m_nWidth = 0;
    //}}AFX_DATA_INIT
    m_pBMgr = NULL;
    m_nHalfHeight = 0;          // Zero mean compute on OK
    m_nHalfWidth = 0;
}

void CNewTileDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CNewTileDialog)
    DDX_Control(pDX, IDC_D_NTILE_WIDTH, m_editWidth);
    DDX_Control(pDX, IDC_D_NTILE_HEIGHT, m_editHeight);
    DDX_Control(pDX, IDC_D_NTILE_BOARDNAME, m_comboBoard);
    DDX_Text(pDX, IDC_D_NTILE_HEIGHT, m_nHeight);
    DDV_MinMaxUInt(pDX, m_nHeight, 2, 512);
    DDX_Text(pDX, IDC_D_NTILE_WIDTH, m_nWidth);
    DDV_MinMaxUInt(pDX, m_nWidth, 2, 512);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNewTileDialog, CDialog)
    //{{AFX_MSG_MAP(CNewTileDialog)
    ON_CBN_SELCHANGE(IDC_D_NTILE_BOARDNAME, OnSelChangeBoardName)
    ON_EN_UPDATE(IDC_D_NTILE_HEIGHT, OnUpdateHeight)
    ON_EN_UPDATE(IDC_D_NTILE_WIDTH, OnUpdateWidth)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_NTILE_BOARDNAME, IDH_D_NTILE_BOARDNAME,
    IDC_D_NTILE_HEIGHT, IDH_D_NTILE_HEIGHT,
    IDC_D_NTILE_WIDTH, IDH_D_NTILE_WIDTH,
    0,0
};

BOOL CNewTileDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CNewTileDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}


/////////////////////////////////////////////////////////////////////////////

BOOL CNewTileDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(m_pBMgr != NULL);

    for (size_t i = 0; i < m_pBMgr->GetNumBoards(); i++)
        m_comboBoard.AddString(m_pBMgr->GetBoard(i).GetName());

    m_comboBoard.SetCurSel(-1);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CNewTileDialog message handlers

void CNewTileDialog::OnSelChangeBoardName()
{
    int nBrd = m_comboBoard.GetCurSel();
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

void CNewTileDialog::OnUpdateHeight()
{
    m_nHalfHeight = 0;
}

void CNewTileDialog::OnUpdateWidth()
{
    m_nHalfWidth = 0;
}

void CNewTileDialog::OnOK()
{
    if (UpdateData(TRUE))
    {
        if (m_nHalfWidth == 0)
            m_nHalfWidth = m_nWidth / 2 + 1;
        if (m_nHalfHeight == 0)
            m_nHalfHeight = m_nHeight / 2 + 1;
        EndDialog(IDOK);
    }
}
