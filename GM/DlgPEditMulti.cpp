// DlgPEditMulti.cpp : implementation file
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
#include "GmHelp.h"
#include "DlgPEditMulti.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPieceEditMultipleDialog dialog

CPieceEditMultipleDialog::CPieceEditMultipleDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CPieceEditMultipleDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CPieceEditMultipleDialog)
    //}}AFX_DATA_INIT
    m_bSetTopOnlyVisible = FALSE;
    m_bTopOnlyOwnersToo = FALSE;
    m_bSetFrontText = FALSE;
    m_bSetBackText = FALSE;
}

void CPieceEditMultipleDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPieceEditMultipleDialog)
    DDX_Control(pDX, IDC_D_MPEDIT_TOP_VISIBLE_OWNERS_TOO, m_chkTopVisibleOwnersToo);
    DDX_Control(pDX, IDC_D_MPEDIT_FRONT_LABEL, m_staticFrontLabel);
    DDX_Control(pDX, IDC_D_MPEDIT_CHG_BACK_TEXT, m_chkSetBackText);
    DDX_Control(pDX, IDC_D_MPEDIT_CHG_TOP_TEXT, m_chkSetFrontText);
    DDX_Control(pDX, IDC_D_MPEDIT_BACK_LABEL, m_staticBackLabel);
    DDX_Control(pDX, IDC_D_MPEDIT_TOP_VISIBLE, m_chkTopVisible);
    DDX_Control(pDX, IDC_D_MPEDIT_TEXT_BACK, m_editBack);
    DDX_Control(pDX, IDC_D_MPEDIT_TEXT_FRONT, m_editFront);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPieceEditMultipleDialog, CDialog)
    //{{AFX_MSG_MAP(CPieceEditMultipleDialog)
    ON_BN_CLICKED(IDC_D_MPEDIT_CHG_BACK_TEXT, OnBtnClickChangeBack)
    ON_BN_CLICKED(IDC_D_MPEDIT_CHG_TOP_TEXT, OnBtnClickChangeFront)
    ON_BN_CLICKED(IDC_D_MPEDIT_TOP_VISIBLE, OnBtnClickTopVisible)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_MPEDIT_TOP_VISIBLE_OWNERS_TOO, IDH_D_MPEDIT_TOP_VISIBLE_OWNERS_TOO,
    IDC_D_MPEDIT_CHG_BACK_TEXT, IDH_D_MPEDIT_CHG_BACK_TEXT,
    IDC_D_MPEDIT_CHG_TOP_TEXT, IDH_D_MPEDIT_CHG_TOP_TEXT,
    IDC_D_MPEDIT_TOP_VISIBLE, IDH_D_MPEDIT_TOP_VISIBLE,
    IDC_D_MPEDIT_TEXT_BACK, IDH_D_MPEDIT_TEXT_BACK,
    IDC_D_MPEDIT_TEXT_FRONT, IDH_D_MPEDIT_TEXT_FRONT,
    0,0
};

/////////////////////////////////////////////////////////////////////////////

void CPieceEditMultipleDialog::UpdateTextControls()
{
    if (m_chkSetFrontText.GetCheck())
    {
        m_editFront.EnableWindow(TRUE);
        m_staticFrontLabel.EnableWindow(TRUE);
    }
    else
    {
        m_editFront.EnableWindow(FALSE);
        m_editFront.SetWindowText("");
        m_staticFrontLabel.EnableWindow(FALSE);
    }
    if (m_chkSetBackText.GetCheck())
    {
        m_editBack.EnableWindow(TRUE);
        m_staticBackLabel.EnableWindow(TRUE);
    }
    else
    {
        m_editBack.EnableWindow(FALSE);
        m_editBack.SetWindowText("");
        m_staticBackLabel.EnableWindow(FALSE);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CPieceEditMultipleDialog message handlers

void CPieceEditMultipleDialog::OnBtnClickTopVisible()
{
    m_chkTopVisibleOwnersToo.EnableWindow(m_chkTopVisible.GetCheck() == 1);
}

void CPieceEditMultipleDialog::OnBtnClickChangeBack()
{
    UpdateTextControls();
}

void CPieceEditMultipleDialog::OnBtnClickChangeFront()
{
    UpdateTextControls();
}

BOOL CPieceEditMultipleDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_chkTopVisible.SetCheck(2);                    // Show as indeterminate
    m_chkTopVisibleOwnersToo.SetCheck(0);
    m_chkTopVisibleOwnersToo.EnableWindow(FALSE);

    m_chkSetFrontText.SetCheck(0);
    m_chkSetBackText.SetCheck(0);

    UpdateTextControls();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPieceEditMultipleDialog::OnOK()
{
    m_bSetTopOnlyVisible = m_chkTopVisible.GetCheck() != 2;
    m_bTopOnlyVisible = m_chkTopVisible.GetCheck() != 0;
    m_bTopOnlyOwnersToo = m_chkTopVisibleOwnersToo.GetCheck() != 0;

    m_bSetFrontText = m_chkSetFrontText.GetCheck() != 0;
    m_editFront.GetWindowText(m_strFront);
    m_bSetBackText = m_chkSetBackText.GetCheck() != 0;
    m_editBack.GetWindowText(m_strBack);

    CDialog::OnOK();
}

BOOL CPieceEditMultipleDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CPieceEditMultipleDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}



