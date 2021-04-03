// DlgBmask.cpp : implementation file
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

#include "stdafx.h"
#include "Gm.h"
#include "Board.h"
#include "DlgBmask.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardMaskDialog dialog


CBoardMaskDialog::CBoardMaskDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CBoardMaskDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CBoardMaskDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_pBMgr = NULL;
}

void CBoardMaskDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CBoardMaskDialog)
    DDX_Control(pDX, IDC_D_BRDMSK_BOARDS, m_lboxBoard);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBoardMaskDialog, CDialog)
    //{{AFX_MSG_MAP(CBoardMaskDialog)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_BRDMSK_BOARDS, IDH_D_BRDMSK_BOARDS,
    0,0
};

BOOL CBoardMaskDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CBoardMaskDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CBoardMaskDialog message handlers

void CBoardMaskDialog::OnOK()
{
    CDialog::OnOK();
    m_nBrdNum = value_preserving_cast<size_t>(m_lboxBoard.GetCurSel());
}

BOOL CBoardMaskDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(m_pBMgr);

    for (size_t i = 0; i < m_pBMgr->GetNumBoards(); i++)
        m_lboxBoard.AddString(m_pBMgr->GetBoard(i).GetName());

    m_lboxBoard.SetCurSel(0);

    return TRUE;  // return TRUE  unless you set the focus to a control
}
