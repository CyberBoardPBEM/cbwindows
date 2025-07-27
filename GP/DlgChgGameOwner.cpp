// DlgChangeOwner.cpp : implementation file
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

#include "stdafx.h"
#include "Gp.h"
#include "Player.h"
#include "DlgChgGameOwner.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChgGameOwnerDialog dialog

CChgGameOwnerDialog::CChgGameOwnerDialog(CPlayerManager& pm, CWnd* pParent /*=NULL*/)
    : CDialog(CChgGameOwnerDialog::IDD, pParent),
    m_pPlayerMgr(pm)
{
    //{{AFX_DATA_INIT(CChgGameOwnerDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_nPlayer = -1;
}

void CChgGameOwnerDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CChgGameOwnerDialog)
    DDX_Control(pDX, IDOK, m_btnOK);
    DDX_Control(pDX, IDC_D_CHGGAMEOWN_LIST, m_listPlayers);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CChgGameOwnerDialog, CDialog)
    //{{AFX_MSG_MAP(CChgGameOwnerDialog)
    ON_LBN_SELCHANGE(IDC_D_CHGGAMEOWN_LIST, OnSelChangePlayerList)
    ON_LBN_DBLCLK(IDC_D_CHGGAMEOWN_LIST, OnDblClkPlayerList)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChgGameOwnerDialog message handlers

void CChgGameOwnerDialog::OnOK()
{
    m_nPlayer = m_listPlayers.GetCurSel();
    CDialog::OnOK();
}

BOOL CChgGameOwnerDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    for (int i = 0; i < m_pPlayerMgr.GetSize(); i++)
        m_listPlayers.AddString(m_pPlayerMgr.ElementAt(i).m_strName);

    if (m_nPlayer >= 0)
        m_listPlayers.SetCurSel(m_nPlayer);
    m_btnOK.EnableWindow(m_listPlayers.GetCurSel() >= 0);

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CChgGameOwnerDialog::OnSelChangePlayerList()
{
    m_btnOK.EnableWindow(m_listPlayers.GetCurSel() >= 0);
}

void CChgGameOwnerDialog::OnDblClkPlayerList()
{
    OnOK();
}
