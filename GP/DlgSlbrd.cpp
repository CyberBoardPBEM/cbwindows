// DlgSlbrd.cpp : implementation file
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
#include    "GamDoc.h"
#include    "Board.h"
#include    "DlgSlbrd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectBoardsDialog dialog


CSelectBoardsDialog::CSelectBoardsDialog(const CBoardManager& bm, wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CSelectBoardsDialog)
        CB_XRC_CTRL(m_listBoards)
    CB_XRC_END_CTRLS_DEFN(),
    m_pBMgr(bm)
{
}

wxBEGIN_EVENT_TABLE(CSelectBoardsDialog, wxDialog)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
    EVT_BUTTON(XRCID("OnBtnClickedSelectAll"), OnBtnClickedSelectAll)
    EVT_BUTTON(XRCID("OnBtnClickedClearAll"), OnBtnClickedClearAll)
wxEND_EVENT_TABLE()

#if 0
/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_SETBRD_LIST, IDH_D_SETBRD_LIST,
    0,0
};

BOOL CSelectBoardsDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CSelectBoardsDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////

int CSelectBoardsDialog::FindSerialNumberListIndex(BoardID nSerial) const
{
    for (unsigned int i = 0u ; i < m_listBoards->GetCount() ; ++i)
    {
        if (static_cast<BoardID>(reinterpret_cast<uintptr_t>(m_listBoards->GetClientData(i))) == nSerial)
        {
            return value_preserving_cast<int>(i);
        }
    }
    return wxNOT_FOUND;
}

/////////////////////////////////////////////////////////////////////////////
// CSelectBoardsDialog message handlers

bool CSelectBoardsDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    m_listBoards->Clear();
    for (size_t i = size_t(0); i < m_pBMgr.GetNumBoards(); i++)
    {
        const CBoard& pBoard = m_pBMgr.GetBoard(i);
        int nIdx = m_listBoards->Append(pBoard.GetName());
        m_listBoards->SetClientData(value_preserving_cast<unsigned int>(nIdx), reinterpret_cast<void*>(value_preserving_cast<uintptr_t>(static_cast<BoardID::UNDERLYING_TYPE>(pBoard.GetSerialNumber()))));
        m_listBoards->Check(value_preserving_cast<unsigned int>(nIdx), false);
    }

    // If there are actively selected boards. Reselect them.

    for (size_t i = 0; i < m_tblBrds.size(); i++)
    {
        int nIdx = FindSerialNumberListIndex(m_tblBrds[i]);
        if (nIdx == wxNOT_FOUND)
        {
            wxMessageBox(CB::string::LoadString(IDS_ERR_BOARDNOTEXIST),
                            CB::GetAppName(),
                            wxOK | wxICON_EXCLAMATION);
        }
        m_listBoards->Check(value_preserving_cast<unsigned int>(nIdx), true);
    }

    return true;
}

bool CSelectBoardsDialog::TransferDataFromWindow()
{
    m_tblBrds.clear();          // Clear the board table

    m_tblBrds.reserve(value_preserving_cast<size_t>(m_listBoards->GetCount()));
    for (unsigned int i = 0u ; i < m_listBoards->GetCount() ; ++i)
    {
        if (m_listBoards->IsChecked(i))
        {
            // Add serial numbers for selected boards
            m_tblBrds.push_back(static_cast<BoardID>(reinterpret_cast<uintptr_t>(m_listBoards->GetClientData(i))));
        }
    }
    return wxDialog::TransferDataFromWindow();
}

void CSelectBoardsDialog::OnBtnClickedSelectAll(wxCommandEvent& /*event*/)
{
    for (unsigned int i = 0u ; i < m_listBoards->GetCount() ; ++i)
    {
        m_listBoards->Check(i, true);
    }
}

void CSelectBoardsDialog::OnBtnClickedClearAll(wxCommandEvent& /*event*/)
{
    for (unsigned int i = 0u ; i < m_listBoards->GetCount() ; ++i)
    {
        m_listBoards->Check(i, false);
    }
}
