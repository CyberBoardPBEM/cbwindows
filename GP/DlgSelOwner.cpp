// DlgSelOwner.cpp : implementation file
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
#include "Gp.h"
#include "Player.h"
#include "DlgSelOwner.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectNewOwnerDialog dialog

CSelectNewOwnerDialog::CSelectNewOwnerDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CSelectNewOwnerDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSelectNewOwnerDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_nPlayer = -1;
    m_pPlayerMgr = NULL;
}

void CSelectNewOwnerDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSelectNewOwnerDialog)
    DDX_Control(pDX, IDOK, m_btnOK);
    DDX_Control(pDX, IDC_D_SELOWN_LIST, m_listPlayers);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSelectNewOwnerDialog, CDialog)
    //{{AFX_MSG_MAP(CSelectNewOwnerDialog)
    ON_LBN_SELCHANGE(IDC_D_SELOWN_LIST, OnSelChangePlayerList)
    ON_LBN_DBLCLK(IDC_D_SELOWN_LIST, OnDblClkPlayerList)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_SELOWN_LIST, IDH_D_SELOWN_LIST,
    0,0
};

BOOL CSelectNewOwnerDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CSelectNewOwnerDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CSelectNewOwnerDialog message handlers

void CSelectNewOwnerDialog::OnOK()
{
    m_nPlayer = m_listPlayers.GetCurSel();
    CDialog::OnOK();
}

BOOL CSelectNewOwnerDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    for (int i = 0; i < m_pPlayerMgr->GetSize(); i++)
        m_listPlayers.AddString(m_pPlayerMgr->ElementAt(i).m_strName);

    if (m_nPlayer >= 0)
        m_listPlayers.SetCurSel(m_nPlayer);
    m_btnOK.EnableWindow(m_listPlayers.GetCurSel() >= 0);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectNewOwnerDialog::OnSelChangePlayerList()
{
    m_btnOK.EnableWindow(m_listPlayers.GetCurSel() >= 0);
}

void CSelectNewOwnerDialog::OnDblClkPlayerList()
{
    OnOK();
}
