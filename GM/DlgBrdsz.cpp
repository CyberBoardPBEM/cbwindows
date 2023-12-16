// dlgbrdsz.cpp : implementation file
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "CellForm.h"
#include    "DlgBrdsz.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardReshapeDialog dialog


CBoardReshapeDialog::CBoardReshapeDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CBoardReshapeDialog)
        CB_XRC_CTRL_VAL(m_checkStaggerIn, m_bStaggerAdapter)
        CB_XRC_CTRL_VAL(m_editCellWd, m_nCellWd, 4u, 32000u)
        CB_XRC_CTRL_VAL(m_editCellHt, m_nCellHt, 4u, 32000u)
        CB_XRC_CTRL_VAL(m_editCols, m_nCols, size_t(1), size_t(1000))
        CB_XRC_CTRL_VAL(m_editRows, m_nRows, size_t(1), size_t(1000))
    CB_XRC_END_CTRLS_DEFN()
{
    m_nCellHt = 0u;
    m_nCellWd = 0u;
    m_nCols = size_t(0);
    m_nRows = size_t(0);
    m_bStagger = CellStagger::Invalid;

    m_eCellStyle = cformRect;
}

wxBEGIN_EVENT_TABLE(CBoardReshapeDialog, wxDialog)
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
    IDC_D_BRDSHP_CELLHT, IDH_D_BRDSHP_CELLHT,
    IDC_D_BRDSHP_CELLWD, IDH_D_BRDSHP_CELLWD,
    IDC_D_BRDSHP_COLS, IDH_D_BRDSHP_COLS,
    IDC_D_BRDSHP_ROWS, IDH_D_BRDSHP_ROWS,
    IDC_D_BRDSHP_STAGGERIN, IDH_D_BRDSHP_STAGGERIN,
    0,0
};

BOOL CBoardReshapeDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CBoardReshapeDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardReshapeDialog message handlers

bool CBoardReshapeDialog::TransferDataToWindow()
{
    m_bStaggerAdapter = static_cast<bool>(m_bStagger);
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    if (m_eCellStyle == cformHexPnt)
        m_editCellHt->Enable(false);
    else if (m_eCellStyle == cformHexFlat)
        m_editCellWd->Enable(false);
    else if (m_eCellStyle == cformRect)
        m_checkStaggerIn->Enable(false); // disable stagger
    return true;
}

bool CBoardReshapeDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }
    m_bStagger = static_cast<CellStagger>(m_bStaggerAdapter);

    return true;
}
