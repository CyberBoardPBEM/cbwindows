// DlgSvisi.cpp : implementation file
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

#include "stdafx.h"
#include "Gm.h"
#include "DlgSvisi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetScaleVisibilityDialog dialog

CSetScaleVisibilityDialog::CSetScaleVisibilityDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CSetScaleVisibilityDialog)
        CB_XRC_CTRL_VAL(m_chkNaturalScale, m_bNaturalScale)
        CB_XRC_CTRL_VAL(m_chkSmallScale, m_bSmallScale)
        CB_XRC_CTRL_VAL(m_chkHalfScale, m_bHalfScale)
        CB_XRC_CTRL_VAL(m_chkFullScale, m_bFullScale)
    CB_XRC_END_CTRLS_DEFN()
{
    m_bFullScale = FALSE;
    m_bHalfScale = FALSE;
    m_bSmallScale = FALSE;
    m_bNaturalScale = FALSE;
}

wxBEGIN_EVENT_TABLE(CSetScaleVisibilityDialog, wxDialog)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
    EVT_CHECKBOX(XRCID("m_chkHalfScale"), OnClickHalf)
    EVT_CHECKBOX(XRCID("m_chkFullScale"), OnClickFull)
    EVT_CHECKBOX(XRCID("m_chkSmallScale"), OnClickSmall)
    EVT_CHECKBOX(XRCID("m_chkNaturalScale"), OnClickNatural)
wxEND_EVENT_TABLE()

#if 0
/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_SETVISI_FULL, IDH_D_SETVISI_FULL,
    IDC_D_SETVISI_HALF, IDH_D_SETVISI_HALF,
    IDC_D_SETVISI_SMALL, IDH_D_SETVISI_SMALL,
    IDC_D_SETVISI_NATURAL, IDH_D_SETVISI_NATURAL,
    0,0
};

BOOL CSetScaleVisibilityDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CSetScaleVisibilityDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetScaleVisibilityDialog message handlers

void CSetScaleVisibilityDialog::OnClickFull(wxCommandEvent& /*event*/)
{
    // Enforce radio button nature of scales if natural scale active
    if (m_chkNaturalScale->GetValue())
    {
        m_chkHalfScale->SetValue(false);
        m_chkSmallScale->SetValue(false);
    }
}

void CSetScaleVisibilityDialog::OnClickHalf(wxCommandEvent& /*event*/)
{
    // Enforce radio button nature of scales if natural scale active
    if (m_chkNaturalScale->GetValue())
    {
        m_chkFullScale->SetValue(false);
        m_chkSmallScale->SetValue(false);
    }
}

void CSetScaleVisibilityDialog::OnClickSmall(wxCommandEvent& /*event*/)
{
    // Enforce radio button nature of scales if natural scale active
    if (m_chkNaturalScale->GetValue())
    {
        m_chkFullScale->SetValue(false);
        m_chkHalfScale->SetValue(false);
    }
}

void CSetScaleVisibilityDialog::OnClickNatural(wxCommandEvent& /*event*/)
{
    if (m_chkNaturalScale->GetValue())
    {
        // If it is set, then only one scale can be checked.
        if (m_chkFullScale->GetValue())
            OnClickFull();
        else if (m_chkHalfScale->GetValue())
            OnClickHalf();
        else if (m_chkSmallScale->GetValue())
            OnClickSmall();
    }
}

bool CSetScaleVisibilityDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }
    if (m_bFullScale || m_bHalfScale || m_bSmallScale)
    {
        return true;
    }
    else
    {
        wxMessageBox(CB::string::LoadString(IDS_ERR_NEEDONEOBJECT),
                    CB::GetAppName(),
                    wxOK | wxICON_EXCLAMATION);
        return false;
    }
}

bool CSetScaleVisibilityDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }
    OnClickNatural();           // Ensure proper state of checks

    return true;
}

