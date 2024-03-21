// DlgTilsz.cpp : implementation file
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
#include    "DlgTilsz.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizeTileDialog dialog


CResizeTileDialog::CResizeTileDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CResizeTileDialog)
        CB_XRC_CTRL_VAL(m_editHeight, m_nHeight, 2u, 512u)
        CB_XRC_CTRL_VAL(m_editWidth, m_nWidth, 2u, 512u)
        CB_XRC_CTRL(m_staticCurWd)
        CB_XRC_CTRL(m_staticCurHt)
        CB_XRC_CTRL(m_comboBrdName)
        CB_XRC_CTRL_VAL(m_chkRescaleBMaps, m_bRescaleBMaps)
    CB_XRC_END_CTRLS_DEFN()
{
    m_bRescaleBMaps = false;
    m_nWidth = 0;
    m_nHeight = 0;
    m_nHalfWidth = 0;
    m_nHalfHeight = 0;
    m_pBMgr = NULL;

    // KLUDGE:  don't see a way to use GetSizeFromText() in .xrc
    wxSize editSize = m_editHeight->GetSizeFromText("999");
    m_editHeight->SetInitialSize(editSize);
    m_editWidth->SetInitialSize(editSize);
    SetMinSize(wxDefaultSize);
    Layout();
    Fit();
    Centre();
}

wxBEGIN_EVENT_TABLE(CResizeTileDialog, wxDialog)
    EVT_CHOICE(XRCID("m_comboBrdName"), OnSelchangeBrdName)
    EVT_TEXT(XRCID("m_editWidth"), OnChangeTileWd)
    EVT_TEXT(XRCID("m_editHeight"), OnChangeTileHt)
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
    IDC_D_RETILE_BOARDNAME, IDH_D_RETILE_BOARDNAME,
    IDC_D_RETILE_CURHT, IDH_D_RETILE_CURHT,
    IDC_D_RETILE_CURWD, IDH_D_RETILE_CURWD,
    IDC_D_RETILE_RESCALE, IDH_D_RETILE_RESCALE,
    IDC_D_RETILE_TILEHT, IDH_D_RETILE_TILEHT,
    IDC_D_RETILE_TILEWD, IDH_D_RETILE_TILEWD,
    0,0
};

BOOL CResizeTileDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CResizeTileDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CResizeTileDialog message handlers

bool CResizeTileDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    ASSERT(m_pBMgr != NULL);

    for (size_t i = size_t(0) ; i < m_pBMgr->GetNumBoards() ; ++i)
    {
        m_comboBrdName->Append(m_pBMgr->GetBoard(i).GetName());
    }
    m_comboBrdName->SetSelection(wxNOT_FOUND);

    CB::string szNum = std::format("{}", m_nHeight);
    m_staticCurHt->SetLabel(szNum);
    szNum = std::format("{}", m_nWidth);
    m_staticCurWd->SetLabel(szNum);

    return true;
}

void CResizeTileDialog::OnSelchangeBrdName(wxCommandEvent& /*event*/)
{
    int nBrd = m_comboBrdName->GetSelection();
    if (nBrd != wxNOT_FOUND)
    {
        CBoard& pBoard = m_pBMgr->GetBoard(value_preserving_cast<size_t>(nBrd));

        CSize size = pBoard.GetCellSize(fullScale);

        CB::string szNum = std::format("{}", size.cx);
        m_editWidth->ChangeValue(szNum);
        szNum = std::format("{}", size.cy);
        m_editHeight->ChangeValue(szNum);

        size = pBoard.GetCellSize(halfScale);
        m_nHalfWidth = size.cx;
        m_nHalfHeight = size.cy;
    }
}

void CResizeTileDialog::OnChangeTileWd(wxCommandEvent& /*event*/)
{
    m_nHalfWidth = 0;
}

void CResizeTileDialog::OnChangeTileHt(wxCommandEvent& /*event*/)
{
    m_nHalfHeight = 0;
}

bool CResizeTileDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }
    // If not set by the board, calc half size tile.
    if (m_nHalfWidth == 0)
        m_nHalfWidth = m_nWidth / 2 + 1;
    if (m_nHalfHeight == 0)
        m_nHalfHeight = m_nHeight / 2 + 1;

    return true;
}
