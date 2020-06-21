// DlgSvisi.cpp : implementation file
//
// Copyright (c) 1994-2020 By Dale L. Larson, All Rights Reserved.
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

CSetScaleVisibilityDialog::CSetScaleVisibilityDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CSetScaleVisibilityDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSetScaleVisibilityDialog)
    m_bFullScale = FALSE;
    m_bHalfScale = FALSE;
    m_bSmallScale = FALSE;
    m_bNaturalScale = FALSE;
    //}}AFX_DATA_INIT
}

void CSetScaleVisibilityDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSetScaleVisibilityDialog)
    DDX_Control(pDX, IDC_D_SETVISI_NATURAL, m_chkNaturalScale);
    DDX_Control(pDX, IDC_D_SETVISI_SMALL, m_chkSmallScale);
    DDX_Control(pDX, IDC_D_SETVISI_HALF, m_chkHalfScale);
    DDX_Control(pDX, IDC_D_SETVISI_FULL, m_chkFullScale);
    DDX_Check(pDX, IDC_D_SETVISI_FULL, m_bFullScale);
    DDX_Check(pDX, IDC_D_SETVISI_HALF, m_bHalfScale);
    DDX_Check(pDX, IDC_D_SETVISI_SMALL, m_bSmallScale);
    DDX_Check(pDX, IDC_D_SETVISI_NATURAL, m_bNaturalScale);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSetScaleVisibilityDialog, CDialog)
    //{{AFX_MSG_MAP(CSetScaleVisibilityDialog)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_BN_CLICKED(IDC_D_SETVISI_HALF, OnClickHalf)
    ON_BN_CLICKED(IDC_D_SETVISI_FULL, OnClickFull)
    ON_BN_CLICKED(IDC_D_SETVISI_SMALL, OnClickSmall)
    ON_BN_CLICKED(IDC_D_SETVISI_NATURAL, OnClickNatural)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

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

/////////////////////////////////////////////////////////////////////////////
// CSetScaleVisibilityDialog message handlers

void CSetScaleVisibilityDialog::OnClickFull()
{
    // Enforce radio button nature of scales if natural scale active
    if (m_chkNaturalScale.GetCheck() != 0)
    {
        m_chkHalfScale.SetCheck(0);
        m_chkSmallScale.SetCheck(0);
    }
}

void CSetScaleVisibilityDialog::OnClickHalf()
{
    // Enforce radio button nature of scales if natural scale active
    if (m_chkNaturalScale.GetCheck() != 0)
    {
        m_chkFullScale.SetCheck(0);
        m_chkSmallScale.SetCheck(0);
    }
}

void CSetScaleVisibilityDialog::OnClickSmall()
{
    // Enforce radio button nature of scales if natural scale active
    if (m_chkNaturalScale.GetCheck() != 0)
    {
        m_chkFullScale.SetCheck(0);
        m_chkHalfScale.SetCheck(0);
    }
}

void CSetScaleVisibilityDialog::OnClickNatural()
{
    if (m_chkNaturalScale.GetCheck() != 0)
    {
        // If it is set, then only one scale can be checked.
        if (m_chkFullScale.GetCheck() != 0)
            OnClickFull();
        else if (m_chkHalfScale.GetCheck() != 0)
            OnClickHalf();
        else if (m_chkSmallScale.GetCheck() != 0)
            OnClickSmall();
    }
}

void CSetScaleVisibilityDialog::OnOK()
{
    if (!UpdateData())
        return;
    if (m_bFullScale || m_bHalfScale || m_bSmallScale)
        CDialog::OnOK();
    else
        AfxMessageBox(IDS_ERR_NEEDONEOBJECT, MB_OK | MB_ICONEXCLAMATION);
}

BOOL CSetScaleVisibilityDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    OnClickNatural();           // Ensure proper state of checks

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

