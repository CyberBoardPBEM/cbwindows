// DlgMPly.cpp
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
#include "DlgMPly.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiplayerGameDialog dialog

CMultiplayerGameDialog::CMultiplayerGameDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CMultiplayerGameDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CMultiplayerGameDialog)
    m_bCreateReferee = FALSE;
    //}}AFX_DATA_INIT
    m_pPlayerMgr = NULL;
}

void CMultiplayerGameDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMultiplayerGameDialog)
    DDX_Control(pDX, IDC_D_MPGAME_PLAYER_NAME, m_editPlayer);
    DDX_Control(pDX, IDC_D_MPGAME_PLAYER_LIST, m_listPlayers);
    DDX_Check(pDX, IDC_D_MPGAME_CREATE_REFEREE, m_bCreateReferee);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMultiplayerGameDialog, CDialog)
    //{{AFX_MSG_MAP(CMultiplayerGameDialog)
    ON_BN_CLICKED(IDC_D_MPGAME, OnBtnPressUpdateName)
    ON_LBN_SELCHANGE(IDC_D_MPGAME_PLAYER_LIST, OnSelChangePlayerList)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_MPGAME_PLAYER_NAME, IDH_D_MPGAME_PLAYER_NAME,
    IDC_D_MPGAME_PLAYER_LIST, IDH_D_MPGAME_PLAYER_LIST,
    IDC_D_MPGAME_CREATE_REFEREE, IDH_D_MPGAME_CREATE_REFEREE,
    IDC_D_MPGAME, IDH_D_MPGAME,
    0,0
};

BOOL CMultiplayerGameDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CMultiplayerGameDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CMultiplayerGameDialog message handlers

void CMultiplayerGameDialog::OnBtnPressUpdateName()
{
    CString strEdit;
    m_editPlayer.GetWindowText(strEdit);
    if (!strEdit.IsEmpty())
    {
        int nSel = m_listPlayers.GetCurSel();
        if (nSel >= 0)
        {
            m_listPlayers.DeleteString(nSel);
            m_listPlayers.InsertString(nSel, strEdit);
        }
    }
}

void CMultiplayerGameDialog::OnSelChangePlayerList()
{
    int nSel = m_listPlayers.GetCurSel();
    if (nSel >= 0)
    {
        CString strEdit;
        m_listPlayers.GetText(nSel, strEdit);
        m_editPlayer.SetWindowText(strEdit);
    }
}

BOOL CMultiplayerGameDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    ASSERT(m_pPlayerMgr != NULL);

    for (int i = 0; i < m_pPlayerMgr->GetSize(); i++)
        m_listPlayers.AddString(m_pPlayerMgr->ElementAt(i).m_strName);

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CMultiplayerGameDialog::OnOK()
{
    for (int i = 0; i < m_pPlayerMgr->GetSize(); i++)
        m_pPlayerMgr->ElementAt(i).m_strName = CB::string::GetText(m_listPlayers, i);

    CDialog::OnOK();
}

