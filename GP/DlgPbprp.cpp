// DlgPbprp.cpp - Playing board properties
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
#include    "GdiTools.h"
#include    "Player.h"
#include    "DlgPbprp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPBrdPropDialog dialog


CPBrdPropDialog::CPBrdPropDialog(wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CPBrdPropDialog)
        CB_XRC_CTRL(m_chkAllowAccess)
        CB_XRC_CTRL(m_chkPrivate)
        CB_XRC_CTRL(m_comboOwners)
        CB_XRC_CTRL(m_staticOwnerLabel)
        CB_XRC_CTRL(m_cpPlotColor)
        CB_XRC_CTRL(m_comboPlotWd)
        CB_XRC_CTRL_VAL(m_chkGridSnap, m_bGridSnap)
        CB_XRC_CTRL_VAL(m_chkSmallCellBorders, m_bSmallCellBorders)
        CB_XRC_CTRL_VAL(m_chkCellBorders, m_bCellBorders)
        CB_XRC_CTRL_VAL(m_editXStackStagger, m_xStackStagger, -256, 256)
        CB_XRC_CTRL_VAL(m_editYStackStagger, m_yStackStagger, -256, 256)
        CB_XRC_CTRL_VAL(m_editBoardName, m_strBoardName)
        CB_XRC_CTRL_VAL(m_chkGridRectCenters, m_bGridRectCenters)
        CB_XRC_CTRL_VAL(m_chkSnapMovePlot, m_bSnapMovePlot)
        CB_XRC_CTRL_VAL(m_editXGridSnapOff, m_fXGridSnapOff, 0.f, 255.999f)
        CB_XRC_CTRL_VAL(m_editYGridSnapOff, m_fYGridSnapOff, 0.f, 255.999f)
        CB_XRC_CTRL_VAL(m_editXGridSnap, m_fXGridSnap, 2.f, 256.f)
        CB_XRC_CTRL_VAL(m_editYGridSnap, m_fYGridSnap, 2.f, 256.f)
        CB_XRC_CTRL_VAL(m_chkOpenBoardOnLoad, m_bOpenBoardOnLoad)
        CB_XRC_CTRL_VAL(m_chkShowSelListAndTinyMap, m_bShowSelListAndTinyMap)
        CB_XRC_CTRL_VAL(m_chkDrawLockedBeneath, m_bDrawLockedBeneath)
    CB_XRC_END_CTRLS_DEFN()
{
    m_bGridSnap = FALSE;
    m_bSmallCellBorders = FALSE;
    m_bCellBorders = FALSE;
    m_xStackStagger = 0;
    m_yStackStagger = 0;
    m_strBoardName = "";
    m_bGridRectCenters = FALSE;
    m_bSnapMovePlot = FALSE;
    m_fXGridSnapOff = 0.0f;
    m_fYGridSnapOff = 0.0f;
    m_fXGridSnap = 0.0f;
    m_fYGridSnap = 0.0f;
    m_bOpenBoardOnLoad = FALSE;
    m_bShowSelListAndTinyMap = FALSE;
    m_bDrawLockedBeneath = TRUE;
    m_crPlotColor = wxColour(0,0,0);
    m_nPlotWd = uint32_t(1);
    m_bNonOwnerAccess = FALSE;
    m_bPrivate = FALSE;
    m_nOwnerSel = INVALID_PLAYER;
    m_pPlayerMgr = NULL;
    m_bOwnerInfoIsReadOnly = FALSE;
}

wxBEGIN_EVENT_TABLE(CPBrdPropDialog, wxDialog)
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
    IDC_D_PBPRP_NONOWNER_ACCESS, IDH_D_PBPRP_NONOWNER_ACCESS,
    IDC_D_PBPRP_OWNER_LIST, IDH_D_PBPRP_OWNER_LIST,
    IDC_D_PBPRP_OWNER_LABEL, IDH_D_PBPRP_OWNER_LABEL,
    IDC_D_PBPRP_PLOTCOLOR, IDH_D_PBPRP_PLOTCOLOR,
    IDC_D_PBPRP_PLOTWIDTH, IDH_D_PBPRP_PLOTWIDTH,
    IDC_D_PBPRP_SNAPON, IDH_D_PBPRP_SNAPON,
    IDC_D_PBPRP_S_CELLBORDER, IDH_D_PBPRP_S_CELLBORDER,
    IDC_D_PBPRP_L_CELLBORDER, IDH_D_PBPRP_L_CELLBORDER,
    IDC_D_PBPRP_XPCE_STGGR, IDH_D_PBPRP_XPCE_STGGR,
    IDC_D_PBPRP_YPCE_STGGR, IDH_D_PBPRP_YPCE_STGGR,
    IDC_D_PBPRP_BOARDNAME, IDH_D_PBPRP_BOARDNAME,
    IDC_D_PBPRP_S_RECTCENTER, IDH_D_PBPRP_S_RECTCENTER,
    IDC_D_PBPRP_S_SNAPPLOTS, IDH_D_PBPRP_S_SNAPPLOTS,
    IDC_D_PBPRP_XOFFSET, IDH_D_PBPRP_XOFFSET,
    IDC_D_PBPRP_YOFFSET, IDH_D_PBPRP_YOFFSET,
    IDC_D_PBPRP_XPIXELS, IDH_D_PBPRP_XPIXELS,
    IDC_D_PBPRP_YPIXELS, IDH_D_PBPRP_YPIXELS,
    IDC_D_PBPRP_AUTOOPEN, IDH_D_PBPRP_AUTOOPEN,
    IDC_D_PBPRP_SHOW_SEL_AND_TINY, IDH_D_PBPRP_SHOW_SEL_AND_TINY,
    0,0
};

BOOL CPBrdPropDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CPBrdPropDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CPBrdPropDialog message handlers

bool CPBrdPropDialog::TransferDataFromWindow()
{
    m_crPlotColor = m_cpPlotColor->GetColour();

    int nIdx = m_comboPlotWd->GetSelection();
    if (nIdx != wxNOT_FOUND)
        m_nPlotWd = value_preserving_cast<uint32_t>(nIdx) + uint32_t(1);

    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    // Make sure these are within grid size

    m_xGridSnapOff = value_preserving_cast<uint32_t>(std::round(m_fXGridSnapOff * 1000));
    m_yGridSnapOff = value_preserving_cast<uint32_t>(std::round(m_fYGridSnapOff * 1000));
    m_xGridSnap = value_preserving_cast<uint32_t>(std::round(m_fXGridSnap * 1000));
    m_yGridSnap = value_preserving_cast<uint32_t>(std::round(m_fYGridSnap * 1000));

    wxASSERT(m_xGridSnap > 0u && m_yGridSnap > 0u);
    if (m_xGridSnap > 0u)
        m_xGridSnapOff = m_xGridSnapOff % m_xGridSnap;
    if (m_yGridSnap > 0u)
        m_yGridSnapOff = m_yGridSnapOff % m_yGridSnap;

    if (m_pPlayerMgr != NULL && !m_bOwnerInfoIsReadOnly)
    {
        m_nOwnerSel = PlayerId(m_comboOwners->GetSelection() - 1);
        m_bNonOwnerAccess = m_chkAllowAccess->GetValue();
        m_bPrivate = m_chkPrivate->GetValue();
    }

    return true;
}

bool CPBrdPropDialog::TransferDataToWindow()
{
    m_fXGridSnapOff = value_preserving_cast<float>(m_xGridSnapOff) / 1000.f;
    m_fYGridSnapOff = value_preserving_cast<float>(m_yGridSnapOff) / 1000.f;
    m_fXGridSnap = value_preserving_cast<float>(m_xGridSnap) / 1000.f;
    m_fYGridSnap = value_preserving_cast<float>(m_yGridSnap) / 1000.f;

    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    m_cpPlotColor->SetColour(m_crPlotColor);

    if (m_nPlotWd == 0u)
        m_nPlotWd = 1u;
    wxASSERT(m_nPlotWd <= m_comboPlotWd->GetCount());
    m_comboPlotWd->SetSelection(value_preserving_cast<int>(m_nPlotWd) - 1);

    if (m_pPlayerMgr == NULL)
    {
        m_comboOwners->Enable(FALSE);
        m_staticOwnerLabel->Enable(FALSE);
        m_chkAllowAccess->SetValue(false);
        m_chkAllowAccess->Enable(FALSE);
        m_chkPrivate->SetValue(false);
        m_chkPrivate->Enable(FALSE);
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
        m_chkAllowAccess->SetValue(m_bNonOwnerAccess);
        m_chkPrivate->SetValue(m_bPrivate);
    }
    if (m_bOwnerInfoIsReadOnly)
    {
        m_comboOwners->Enable(FALSE);
        m_chkAllowAccess->Enable(FALSE);
        m_chkPrivate->Enable(FALSE);
    }

    return TRUE;  // return TRUE  unless you set the focus to a control
}
