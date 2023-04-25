// DlgEPly.cpp : implementation file
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
#include "DlgEply.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditPlayersDialog dialog

CEditPlayersDialog::CEditPlayersDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CEditPlayersDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CEditPlayersDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void CEditPlayersDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CEditPlayersDialog)
    DDX_Control(pDX, IDC_D_EPLAY_UPDATE_NAME, m_btnUpdate);
    DDX_Control(pDX, IDC_D_EPLAY_NAME_LIST, m_listNames);
    DDX_Control(pDX, IDC_D_EPLAY_EDIT_NAME, m_editName);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditPlayersDialog, CDialog)
    //{{AFX_MSG_MAP(CEditPlayersDialog)
    ON_LBN_SELCHANGE(IDC_D_EPLAY_NAME_LIST, OnSelChangeNameList)
    ON_BN_CLICKED(IDC_D_EPLAY_UPDATE_NAME, OnBtnPressUpdateName)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_EPLAY_UPDATE_NAME, IDH_D_EPLAY_UPDATE_NAME,
    IDC_D_EPLAY_NAME_LIST, IDH_D_EPLAY_NAME_LIST,
    IDC_D_EPLAY_EDIT_NAME, IDH_D_EPLAY_EDIT_NAME,
    0,0
};

BOOL CEditPlayersDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CEditPlayersDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////

void CEditPlayersDialog::SetDialogsPlayerNames(CPlayerManager* pPlayerMgr)
{
    m_tblNames.reserve(m_tblNames.size() + value_preserving_cast<size_t>(pPlayerMgr->GetSize()));
    for (int i = 0; i < pPlayerMgr->GetSize(); i++)
        m_tblNames.push_back(pPlayerMgr->ElementAt(i).m_strName);
}

void CEditPlayersDialog::GetPlayerNamesFromDialog(CPlayerManager* pPlayerMgr)
{
    for (int i = 0; i < pPlayerMgr->GetSize(); i++)
        pPlayerMgr->ElementAt(i).m_strName = m_tblNames[value_preserving_cast<size_t>(i)];
}

/////////////////////////////////////////////////////////////////////////////
// CEditPlayersDialog message handlers

void CEditPlayersDialog::OnSelChangeNameList()
{
    int nSel = m_listNames.GetCurSel();
    if (nSel >= 0)
    {
        CB::string strEdit = CB::string::GetText(m_listNames, nSel);
        m_editName.SetWindowText(strEdit);
    }
}

void CEditPlayersDialog::OnBtnPressUpdateName()
{
    CB::string strEdit = CB::string::GetWindowText(m_editName);
    if (!strEdit.empty())
    {
        int nSel = m_listNames.GetCurSel();
        if (nSel >= 0)
        {
            m_listNames.DeleteString(nSel);
            m_listNames.InsertString(nSel, strEdit);
        }
    }
}

BOOL CEditPlayersDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    for (size_t i = size_t(0) ; i < m_tblNames.size() ; ++i)
        m_listNames.AddString(m_tblNames[i]);

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CEditPlayersDialog::OnOK()
{
    m_tblNames.clear();
    m_tblNames.reserve(value_preserving_cast<size_t>(m_listNames.GetCount()));
    for (int i = 0; i < m_listNames.GetCount(); i++)
    {
        CB::string strName = CB::string::GetText(m_listNames, i);
        m_tblNames.push_back(std::move(strName));
    }

    CDialog::OnOK();
}


