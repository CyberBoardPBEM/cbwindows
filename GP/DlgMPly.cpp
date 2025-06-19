// DlgMPly.cpp
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
#include "DlgMPly.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiplayerGameDialog dialog

CMultiplayerGameDialog::CMultiplayerGameDialog(CPlayerManager& pm, wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CMultiplayerGameDialog)
        CB_XRC_CTRL(m_static1)
        CB_XRC_CTRL_VAL(m_chkCreateReferee, m_bCreateReferee)
        CB_XRC_CTRL(m_static2)
        CB_XRC_CTRL(m_editPlayer)
        CB_XRC_CTRL(m_listPlayers)
    CB_XRC_END_CTRLS_DEFN(),
    m_pPlayerMgr(&pm)
{
    m_bCreateReferee = FALSE;

    wxSize chkSize = m_chkCreateReferee->GetBestSize();
    m_static1->Wrap(chkSize.GetWidth());
    m_static2->Wrap(chkSize.GetWidth());
    /* adding content to listbox may change its size, so no
        reason to adjust layout yet despite changing static
        wrap */
}

wxBEGIN_EVENT_TABLE(CMultiplayerGameDialog, wxDialog)
    EVT_BUTTON(XRCID("OnBtnPressUpdateName"), OnBtnPressUpdateName)
    EVT_LISTBOX(XRCID("m_listPlayers"), OnSelChangePlayerList)
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
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiplayerGameDialog message handlers

void CMultiplayerGameDialog::OnBtnPressUpdateName(wxCommandEvent& /*event*/)
{
    CB::string strEdit = m_editPlayer->GetValue();
    if (!strEdit.empty())
    {
        int nSel = m_listPlayers->GetSelection();
        if (nSel != wxNOT_FOUND)
        {
            m_listPlayers->Delete(value_preserving_cast<unsigned int>(nSel));
            m_listPlayers->Insert(strEdit, value_preserving_cast<unsigned int>(nSel));
        }
    }
}

void CMultiplayerGameDialog::OnSelChangePlayerList(wxCommandEvent& /*event*/)
{
    int nSel = m_listPlayers->GetSelection();
    if (nSel != wxNOT_FOUND)
    {
        CB::string strEdit = m_listPlayers->GetString(value_preserving_cast<unsigned int>(nSel));
        m_editPlayer->SetValue(strEdit);
    }
}

bool CMultiplayerGameDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    for (int i = 0 ; i < m_pPlayerMgr->GetSize() ; ++i)
    {
        m_listPlayers->Append(m_pPlayerMgr->ElementAt(i).m_strName);
    }

    // all controls are ready, so now adjust layout
    SetMinSize(wxDefaultSize);
    Fit();
    Centre();

    return TRUE;
}

bool CMultiplayerGameDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    for (int i = 0 ; i < m_pPlayerMgr->GetSize() ; ++i)
    {
        m_pPlayerMgr->ElementAt(i).m_strName = m_listPlayers->GetString(value_preserving_cast<unsigned int>(i));
    }

    return true;
}

