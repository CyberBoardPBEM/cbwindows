// DlgEPly.cpp : implementation file
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
#include "DlgEply.h"
#include <numeric>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditPlayersDialog dialog

CEditPlayersDialog::CEditPlayersDialog(wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CEditPlayersDialog)
        CB_XRC_CTRL(m_btnUpdate)
        CB_XRC_CTRL(m_listNames)
        CB_XRC_CTRL(m_editName)
    CB_XRC_END_CTRLS_DEFN()
{
}

wxBEGIN_EVENT_TABLE(CEditPlayersDialog, wxDialog)
    EVT_LISTBOX(XRCID("m_listNames"), OnSelChangeNameList)
    EVT_BUTTON(XRCID("m_btnUpdate"), OnBtnPressUpdateName)
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
#endif

/////////////////////////////////////////////////////////////////////////////

void CEditPlayersDialog::SetDialogsPlayerNames(const CPlayerManager& pPlayerMgr)
{
    m_tblNames.reserve(m_tblNames.size() + value_preserving_cast<size_t>(pPlayerMgr.GetSize()));
    for (int i = 0; i < pPlayerMgr.GetSize(); i++)
        m_tblNames.push_back(pPlayerMgr.ElementAt(i).m_strName);
}

void CEditPlayersDialog::GetPlayerNamesFromDialog(CPlayerManager& pPlayerMgr) const
{
    for (int i = 0; i < pPlayerMgr.GetSize(); i++)
        pPlayerMgr.ElementAt(i).m_strName = m_tblNames[value_preserving_cast<size_t>(i)];
}

/////////////////////////////////////////////////////////////////////////////
// CEditPlayersDialog message handlers

void CEditPlayersDialog::OnSelChangeNameList(wxCommandEvent& /*event*/)
{
    int nSel = m_listNames->GetSelection();
    if (nSel != wxNOT_FOUND)
    {
        wxString strEdit = m_listNames->GetString(value_preserving_cast<unsigned>(nSel));
        m_editName->SetValue(strEdit);
    }
}

void CEditPlayersDialog::OnBtnPressUpdateName(wxCommandEvent& /*event*/)
{
    CB::string strEdit = m_editName->GetValue();
    if (!strEdit.empty())
    {
        int nSel = m_listNames->GetSelection();
        if (nSel != wxNOT_FOUND)
        {
            m_listNames->Delete(value_preserving_cast<unsigned>(nSel));
            m_listNames->Insert(strEdit, value_preserving_cast<unsigned>(nSel));
        }
    }
}

bool CEditPlayersDialog::TransferDataToWindow()
{
    /* display the player names initially sorted,
        but don't actually reorder them */
    m_tblNamesSorted.resize(m_tblNames.size());
    std::iota(m_tblNamesSorted.begin(), m_tblNamesSorted.end(), size_t(0));
    std::sort(m_tblNamesSorted.begin(), m_tblNamesSorted.end(),
                [this](const size_t left, const size_t right)
                {
                    return m_tblNames[left] < m_tblNames[right];
                });

    for (size_t i = size_t(0) ; i < m_tblNames.size() ; ++i)
        m_listNames->AppendString(m_tblNames[m_tblNamesSorted[i]]);

    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }
    return true;
}

bool CEditPlayersDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    m_tblNames.clear();
    m_tblNames.reserve(m_listNames->GetCount());
    for (size_t i = size_t(0) ; i < m_tblNamesSorted.size() ; ++i)
    {
        unsigned j = value_preserving_cast<unsigned>(m_tblNamesSorted[i]);
        CB::string strName = m_listNames->GetString(j);
        m_tblNames.push_back(std::move(strName));
    }

    return true;
}


