// DlgYprop.cpp : implementation file
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
#include    "Gp.h"
#include    "Trays.h"
#include    "Player.h"
#include    "DlgYprop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrayPropDialog dialog

CTrayPropDialog::CTrayPropDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CTrayPropDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CTrayPropDialog)
    m_strName = "";
    m_nVizOpts = -1;
    m_bRandomSel = FALSE;
    m_bRandomSide = false;
    //}}AFX_DATA_INIT
    m_nYSel = Invalid_v<size_t>;
    m_nOwnerSel = -1;
    m_bNonOwnerAccess = FALSE;
    m_pPlayerMgr = NULL;
    m_pYMgr = NULL;
    m_bEnforceVizForOwnerToo = FALSE;
}

void CTrayPropDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTrayPropDialog)
    DDX_Control(pDX, IDC_D_YPRP_OWNER_TOO, m_chkVizOwnerToo);
    DDX_Control(pDX, IDC_D_YPRP_NONOWNER_ACCESS, m_chkAllowAccess);
    DDX_Control(pDX, IDC_D_YPRP_OWNER_LABEL, m_staticOwnerLabel);
    DDX_Control(pDX, IDC_D_YPRP_OWNER_LIST, m_comboOwners);
    DDX_Control(pDX, IDC_D_YPRP_NAME, m_editName);
    DDX_Text(pDX, IDC_D_YPRP_NAME, m_strName);
    DDV_MaxChars(pDX, m_strName, 32);
    DDX_Radio(pDX, IDC_D_YPRP_VIZFULL, m_nVizOpts);
    DDX_Check(pDX, IDC_D_YPRP_RANDSEL, m_bRandomSel);
    DDX_Check(pDX, IDC_D_YPRP_RANDSIDE, m_bRandomSide);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTrayPropDialog, CDialog)
    //{{AFX_MSG_MAP(CTrayPropDialog)
    ON_CBN_SELCHANGE(IDC_D_YPRP_OWNER_LIST, OnSelChangeOwnerList)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_YPRP_OWNER_TOO, IDH_D_YPRP_OWNER_TOO,
    IDC_D_YPRP_NONOWNER_ACCESS, IDH_D_YPRP_NONOWNER_ACCESS,
    IDC_D_YPRP_OWNER_LABEL, IDH_D_YPRP_OWNER_LABEL,
    IDC_D_YPRP_OWNER_LIST, IDH_D_YPRP_OWNER_LIST,
    IDC_D_YPRP_NAME, IDH_D_YPRP_NAME,
    IDC_D_YPRP_NAME, IDH_D_YPRP_NAME,
    IDC_D_YPRP_VIZFULL, IDH_D_YPRP_VIZFULL,
    IDC_D_YPRP_VIZTOP, IDH_D_YPRP_VIZTOP,
    IDC_D_YPRP_VIZHIDDEN, IDH_D_YPRP_VIZHIDDEN,
    IDC_D_YPRP_VIZALLHIDDEN, IDH_D_YPRP_VIZALLHIDDEN,
    IDC_D_YPRP_RANDSEL, IDH_D_YPRP_RANDSEL,
    0,0
};

BOOL CTrayPropDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CTrayPropDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}


/////////////////////////////////////////////////////////////////////////////
// CTrayPropDialog message handlers

void CTrayPropDialog::OnSelChangeOwnerList()
{
    if (m_comboOwners.GetCurSel() <= 0)
    {
        m_chkAllowAccess.SetCheck(0);
        m_chkVizOwnerToo.SetCheck(0);
        m_chkAllowAccess.EnableWindow(FALSE);
        m_chkVizOwnerToo.EnableWindow(FALSE);
    }
    else
    {
        m_chkAllowAccess.EnableWindow(TRUE);
        m_chkVizOwnerToo.EnableWindow(TRUE);
    }
}

void CTrayPropDialog::OnOK()
{
    m_strName = CB::string::GetWindowText(m_editName);
    if (m_strName.empty())
    {
        AfxMessageBox(IDS_ERR_TRAYNAME, MB_OK | MB_ICONINFORMATION);
        m_editName.SetFocus();
        return;
    }
    size_t nSel = m_pYMgr->FindTrayByName(m_strName);
    if (nSel != Invalid_v<size_t> && nSel != m_nYSel)
    {
        AfxMessageBox(IDS_ERR_TRAYNAMEUSED, MB_OK | MB_ICONINFORMATION);
        m_editName.SetFocus();
        return;
    }
    if (m_pPlayerMgr != NULL)
    {
        m_nOwnerSel = m_comboOwners.GetCurSel() - 1;
        m_bNonOwnerAccess = m_chkAllowAccess.GetCheck() != 0;
        m_bEnforceVizForOwnerToo = m_chkVizOwnerToo.GetCheck() != 0;
        if (m_nOwnerSel < 0)
        {
            m_bNonOwnerAccess = FALSE;
            m_bEnforceVizForOwnerToo = FALSE;
        }
    }

    CDialog::OnOK();
}

BOOL CTrayPropDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    ASSERT(m_pYMgr);
    ASSERT(m_nYSel != Invalid_v<size_t>);

    m_editName.SetWindowText(m_pYMgr->GetTraySet(m_nYSel).GetName());
    if (m_pPlayerMgr == NULL)
    {
        m_comboOwners.EnableWindow(FALSE);
        m_staticOwnerLabel.EnableWindow(FALSE);
        m_chkAllowAccess.SetCheck(0);
        m_chkAllowAccess.EnableWindow(FALSE);
    }
    else
    {
        CB::string str = CB::string::LoadString(IDS_LBL_NO_OWNER);
        m_comboOwners.AddString(str);
        for (int i = 0; i < m_pPlayerMgr->GetSize(); i++)
            m_comboOwners.AddString(m_pPlayerMgr->GetAt(i).m_strName);
        m_comboOwners.SetCurSel(m_nOwnerSel + 1);
        if (m_comboOwners.GetCurSel() <= 0)
        {
            m_chkAllowAccess.SetCheck(0);
            m_chkAllowAccess.EnableWindow(FALSE);
            m_chkVizOwnerToo.SetCheck(0);
            m_chkVizOwnerToo.EnableWindow(FALSE);
        }
        else
        {
            m_chkAllowAccess.EnableWindow(TRUE);
            m_chkVizOwnerToo.EnableWindow(TRUE);
            m_chkAllowAccess.SetCheck(m_bNonOwnerAccess ? 1 : 0);
            m_chkVizOwnerToo.SetCheck(m_bEnforceVizForOwnerToo ? 1 : 0);
        }
    }
    return TRUE;  // return TRUE  unless you set the focus to a control
}

