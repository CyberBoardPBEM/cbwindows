// DlgYprop.cpp : implementation file
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

CTrayPropDialog::CTrayPropDialog(const CTrayManager& yMgr,
                                    const CPlayerManager* playerMgr,
                                    wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CTrayPropDialog)
        CB_XRC_CTRL(m_chkVizOwnerToo)
        CB_XRC_CTRL(m_chkAllowAccess)
        CB_XRC_CTRL(m_staticOwnerLabel)
        CB_XRC_CTRL(m_comboOwners)
        CB_XRC_CTRL_VAL(m_editName, m_strName, wxFILTER_EMPTY, 32)
        CB_XRC_CTRL_VAL(m_radioVizOpts, m_nVizOpts)
        CB_XRC_CTRL_VAL(m_checkRandomSel, m_bRandomSel)
        CB_XRC_CTRL_VAL(m_checkRandomSide, m_bRandomSide)
    CB_XRC_END_CTRLS_DEFN(),
    m_pYMgr(yMgr),
    m_pPlayerMgr(playerMgr)
{
    m_strName = "";
    m_nVizOpts = -1;
    m_bRandomSel = FALSE;
    m_bRandomSide = false;
    m_nYSel = Invalid_v<size_t>;
    m_nOwnerSel = INVALID_PLAYER;
    m_bNonOwnerAccess = FALSE;
    m_bEnforceVizForOwnerToo = FALSE;
}

wxBEGIN_EVENT_TABLE(CTrayPropDialog, wxDialog)
    EVT_CHOICE(XRCID("m_comboOwners"), OnSelChangeOwnerList)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
wxEND_EVENT_TABLE()

#if 0
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
#endif


/////////////////////////////////////////////////////////////////////////////
// CTrayPropDialog message handlers

void CTrayPropDialog::OnSelChangeOwnerList(wxCommandEvent& /*event*/)
{
    if (m_comboOwners->GetSelection() <= 0)
    {
        m_chkAllowAccess->SetValue(false);
        m_chkVizOwnerToo->SetValue(false);
        m_chkAllowAccess->Enable(FALSE);
        m_chkVizOwnerToo->Enable(FALSE);
    }
    else
    {
        m_chkAllowAccess->Enable(TRUE);
        m_chkVizOwnerToo->Enable(TRUE);
    }
}

bool CTrayPropDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    size_t nSel = m_pYMgr.FindTrayByName(m_strName);
    if (nSel != Invalid_v<size_t> && nSel != m_nYSel)
    {
        wxMessageBox(CB::string::LoadString(IDS_ERR_TRAYNAMEUSED),
                    CB::GetAppName(),
                    wxOK | wxICON_INFORMATION);
        m_editName->SetFocus();
        return false;
    }
    if (m_pPlayerMgr != NULL)
    {
        m_nOwnerSel = PlayerId(m_comboOwners->GetSelection() - 1);
        m_bNonOwnerAccess = m_chkAllowAccess->GetValue();
        m_bEnforceVizForOwnerToo = m_chkVizOwnerToo->GetValue();
        if (m_nOwnerSel == INVALID_PLAYER)
        {
            m_bNonOwnerAccess = FALSE;
            m_bEnforceVizForOwnerToo = FALSE;
        }
    }

    return true;
}

bool CTrayPropDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }
    wxASSERT(m_nYSel != Invalid_v<size_t>);

    m_editName->SetValue(m_pYMgr.GetTraySet(m_nYSel).GetName());
    if (m_pPlayerMgr == NULL)
    {
        m_comboOwners->Enable(FALSE);
        m_staticOwnerLabel->Enable(FALSE);
        m_chkAllowAccess->SetValue(false);
        m_chkAllowAccess->Enable(FALSE);
    }
    else
    {
        CB::string str = CB::string::LoadString(IDS_LBL_NO_OWNER);
        m_comboOwners->Append(str);
        for (const Player& player : *m_pPlayerMgr)
        {
            m_comboOwners->Append(player.m_strName);
        }
        m_comboOwners->SetSelection(static_cast<int>(m_nOwnerSel) + 1);
        if (m_comboOwners->GetSelection() <= 0)
        {
            m_chkAllowAccess->SetValue(false);
            m_chkAllowAccess->Enable(FALSE);
            m_chkVizOwnerToo->SetValue(false);
            m_chkVizOwnerToo->Enable(FALSE);
        }
        else
        {
            m_chkAllowAccess->Enable(TRUE);
            m_chkVizOwnerToo->Enable(TRUE);
            m_chkAllowAccess->SetValue(m_bNonOwnerAccess);
            m_chkVizOwnerToo->SetValue(m_bEnforceVizForOwnerToo);
        }
    }
    return TRUE;
}

