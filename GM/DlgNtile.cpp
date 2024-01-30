// DlgNtile.cpp : implementation file
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "Gm.h"
#include    "Board.h"
#include    "DlgNtile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewTileDialog dialog

CNewTileDialog::CNewTileDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CNewTileDialog)
        CB_XRC_CTRL_VAL(m_editWidth, m_nWidth, UINT(2), UINT(512))
        CB_XRC_CTRL_VAL(m_editHeight, m_nHeight, UINT(2), UINT(512))
        CB_XRC_CTRL(m_comboBoard)
    CB_XRC_END_CTRLS_DEFN()
{
    m_nHeight = 0;
    m_nWidth = 0;
    m_pBMgr = NULL;
    m_nHalfHeight = 0;          // Zero mean compute on OK
    m_nHalfWidth = 0;

    // KLUDGE:  don't see a way to use GetSizeFromText() in .xrc
    wxSize editSize = m_editWidth->GetSizeFromText("999");
    m_editWidth->SetInitialSize(editSize);
    m_editHeight->SetInitialSize(editSize);
    SetMinSize(wxDefaultSize);
    Layout();
    Fit();
    Centre();
}

wxBEGIN_EVENT_TABLE(CNewTileDialog, wxDialog)
    EVT_CHOICE(XRCID("m_comboBoard"), OnSelChangeBoardName)
    EVT_TEXT(XRCID("m_editHeight"), OnUpdateHeight)
    EVT_TEXT(XRCID("m_editWidth"), OnUpdateWidth)
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
    IDC_D_NTILE_BOARDNAME, IDH_D_NTILE_BOARDNAME,
    IDC_D_NTILE_HEIGHT, IDH_D_NTILE_HEIGHT,
    IDC_D_NTILE_WIDTH, IDH_D_NTILE_WIDTH,
    0,0
};

BOOL CNewTileDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CNewTileDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif


/////////////////////////////////////////////////////////////////////////////

bool CNewTileDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    wxASSERT(m_pBMgr != NULL);

    wxASSERT(m_comboBoard->IsEmpty());
    for (size_t i = 0; i < m_pBMgr->GetNumBoards(); i++)
    {
        m_comboBoard->Append(m_pBMgr->GetBoard(i).GetName());
    }

    m_comboBoard->SetSelection(wxNOT_FOUND);

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// CNewTileDialog message handlers

void CNewTileDialog::OnSelChangeBoardName(wxCommandEvent& event)
{
    int nBrd = m_comboBoard->GetSelection();
    if (nBrd != wxNOT_FOUND)
    {
        CBoard& pBoard = m_pBMgr->GetBoard(value_preserving_cast<size_t>(nBrd));

        CSize size = pBoard.GetCellSize(fullScale);

        CB::string szNum = std::format("{}", size.cx);
        m_editWidth->SetValue(szNum);
        szNum = std::format("{}", size.cy);
        m_editHeight->SetValue(szNum);

        size = pBoard.GetCellSize(halfScale);
        m_nHalfWidth = size.cx;
        m_nHalfHeight = size.cy;
    }
}

void CNewTileDialog::OnUpdateHeight(wxCommandEvent& event)
{
    m_nHalfHeight = 0;
}

void CNewTileDialog::OnUpdateWidth(wxCommandEvent& event)
{
    m_nHalfWidth = 0;
}

bool CNewTileDialog::TransferDataFromWindow()
{
    if (wxDialog::TransferDataFromWindow())
    {
        if (m_nHalfWidth == 0)
        {
            m_nHalfWidth = m_nWidth / 2 + 1;
        }
        if (m_nHalfHeight == 0)
        {
            m_nHalfHeight = m_nHeight / 2 + 1;
        }
        return true;
    }

    return false;
}
