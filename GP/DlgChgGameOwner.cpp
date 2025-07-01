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

CChgGameOwnerDialog::CChgGameOwnerDialog(CPlayerManager& pm, wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CChgGameOwnerDialog)
        , m_btnOK(XRCCTRL(*this, "wxID_OK", std::remove_reference_t<decltype(*m_btnOK)>))
        CB_XRC_CTRL(m_listPlayers)
    CB_XRC_END_CTRLS_DEFN(),
    m_pPlayerMgr(&pm)
{
    m_nPlayer = INVALID_PLAYER;
}

wxBEGIN_EVENT_TABLE(CChgGameOwnerDialog, wxDialog)
    EVT_LISTBOX(XRCID("m_listPlayers"), OnSelChangePlayerList)
    EVT_LISTBOX_DCLICK(XRCID("m_listPlayers"), OnDblClkPlayerList)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////
// CChgGameOwnerDialog message handlers

bool CChgGameOwnerDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    m_nPlayer = PlayerId(m_listPlayers->GetSelection());

    return true;
}

bool CChgGameOwnerDialog::TransferDataToWindow()
{
    m_listPlayers->Clear();
    for (const Player& player : *m_pPlayerMgr)
        m_listPlayers->AppendString(player.m_strName);

    if (m_nPlayer != INVALID_PLAYER)
        m_listPlayers->SetSelection(static_cast<int>(m_nPlayer));
    m_btnOK->Enable(m_listPlayers->GetSelection() != wxNOT_FOUND);

    return wxDialog::TransferDataToWindow();
}

void CChgGameOwnerDialog::OnSelChangePlayerList(wxCommandEvent& /*event*/)
{
    m_btnOK->Enable(m_listPlayers->GetSelection() != wxNOT_FOUND);
}

void CChgGameOwnerDialog::OnDblClkPlayerList(wxCommandEvent& /*event*/)
{
    wxCommandEvent event(wxEVT_BUTTON, m_btnOK->GetId());
    event.SetEventObject(&*m_btnOK);
    m_btnOK->HandleWindowEvent(event);
}
