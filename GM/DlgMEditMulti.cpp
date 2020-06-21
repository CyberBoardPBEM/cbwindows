// DlgMEditMulti.cpp : implementation file
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
#include "DlgMEditMulti.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkerEditMultipleDialog dialog


CMarkerEditMultipleDialog::CMarkerEditMultipleDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CMarkerEditMultipleDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CMarkerEditMultipleDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_bSetText = FALSE;
    m_bSetPromptForText = FALSE;
    m_bPromptForText = FALSE;
}


void CMarkerEditMultipleDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMarkerEditMultipleDialog)
    DDX_Control(pDX, IDC_D_MMEDIT_CHG_TEXT, m_chkSetText);
    DDX_Control(pDX, IDC_D_MMEDIT_TEXT_LABEL, m_staticTextLabel);
    DDX_Control(pDX, IDC_D_MMEDIT_TEXTPROMPT, m_chkPromptForText);
    DDX_Control(pDX, IDC_D_MMEDIT_TEXT, m_editText);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMarkerEditMultipleDialog, CDialog)
    //{{AFX_MSG_MAP(CMarkerEditMultipleDialog)
    ON_BN_CLICKED(IDC_D_MEDIT_CHG_TEXT, OnBtnClickChangeText)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_MMEDIT_CHG_TEXT, IDH_D_MMEDIT_CHG_TEXT,
    IDC_D_MMEDIT_TEXTPROMPT, IDH_D_MMEDIT_TEXTPROMPT,
    IDC_D_MMEDIT_TEXT, IDH_D_MMEDIT_TEXT,
    0,0
};

BOOL CMarkerEditMultipleDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CMarkerEditMultipleDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerEditMultipleDialog::UpdateTextControls()
{
    if (m_chkSetText.GetCheck())
    {
        m_editText.EnableWindow(TRUE);
        m_staticTextLabel.EnableWindow(TRUE);
    }
    else
    {
        m_editText.EnableWindow(FALSE);
        m_editText.SetWindowText("");
        m_staticTextLabel.EnableWindow(FALSE);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CMarkerEditMultipleDialog message handlers

void CMarkerEditMultipleDialog::OnBtnClickChangeText()
{
    UpdateTextControls();
}

BOOL CMarkerEditMultipleDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_chkPromptForText.SetCheck(2);            // Show as indeterminate

    m_chkSetText.SetCheck(0);
    UpdateTextControls();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CMarkerEditMultipleDialog::OnOK()
{
    m_bSetPromptForText = m_chkPromptForText.GetCheck() != 2;
    m_bPromptForText = m_chkPromptForText.GetCheck() != 0;

    m_bSetText = m_chkSetText.GetCheck() != 0;
    m_editText.GetWindowText(m_strText);

    CDialog::OnOK();
}


