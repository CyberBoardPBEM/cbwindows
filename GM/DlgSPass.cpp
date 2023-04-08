// DlgSPass.cpp : implementation file
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
#include "DlgSPass.h"
#include "LibMfc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetGameboxPassword dialog

CSetGameboxPassword::CSetGameboxPassword(CWnd* pParent /*=NULL*/)
    : CDialog(CSetGameboxPassword::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSetGameboxPassword)
    m_strPass1 = "";
    m_strPass2 = "";
    //}}AFX_DATA_INIT
}

void CSetGameboxPassword::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSetGameboxPassword)
    DDX_Control(pDX, IDC_D_SETPASS_PASS_2, m_editPass2);
    DDX_Control(pDX, IDC_D_SETPASS_PASS_1, m_editPass1);
    DDX_Text(pDX, IDC_D_SETPASS_PASS_1, m_strPass1);
    DDX_Text(pDX, IDC_D_SETPASS_PASS_2, m_strPass2);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSetGameboxPassword, CDialog)
    //{{AFX_MSG_MAP(CSetGameboxPassword)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_SETPASS_PASS_1, IDH_D_SETPASS_PASS_1,
    IDC_D_SETPASS_PASS_2, IDH_D_SETPASS_PASS_2,
    0,0
};

BOOL CSetGameboxPassword::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CSetGameboxPassword::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CSetGameboxPassword message handlers

void CSetGameboxPassword::OnOK()
{
    m_strPass1 = CB::string::GetWindowText(m_editPass1);
    m_strPass2 = CB::string::GetWindowText(m_editPass2);

    if (m_strPass1 != m_strPass2)
    {
        AfxMessageBox(IDS_WARN_PASS_MUST_MATCH);
        m_editPass1.SetFocus();
        m_editPass1.SetSel(0, -1);
    }
    else
        CDialog::OnOK();
}
