// DlgTsetp.cpp - Tile properties dialog
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

#include    "stdafx.h"
#include    "Gm.h"
#include    "GdiTools.h"
#include    "DlgTsetp.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTSetPropDialog dialog


CTSetPropDialog::CTSetPropDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CTSetPropDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CTSetPropDialog)
    m_strName = "";
    //}}AFX_DATA_INIT
    m_crTrans = RGB(0,255,255);
}

void CTSetPropDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTSetPropDialog)
    DDX_Control(pDX, IDC_D_TGRP_TRANSCOLOR, m_cpTrans);
    DDX_Text(pDX, IDC_D_TGRP_NAME, m_strName);
    DDV_MaxChars(pDX, m_strName, 40);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTSetPropDialog, CDialog)
    //{{AFX_MSG_MAP(CTSetPropDialog)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_TGRP_NAME, IDH_D_TGRP_NAME,
    IDC_D_TGRP_TRANSCOLOR, IDH_D_TGRP_TRANSCOLOR,
    0,0
};

BOOL CTSetPropDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CTSetPropDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CTSetPropDialog message handlers

void CTSetPropDialog::OnOK()
{
    m_crTrans = m_cpTrans.GetColor();
    CDialog::OnOK();
}

BOOL CTSetPropDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_cpTrans.SetColor(m_crTrans);

    return TRUE;  // return TRUE  unless you set the focus to a control
}
