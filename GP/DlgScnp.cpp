// DlgScnp.cpp : implementation file
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
#include "Gp.h"
#include "DlgScnp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CScnPropDialog dialog

CScnPropDialog::CScnPropDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CScnPropDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CScnPropDialog)
    m_strAuthor = "";
    m_strTitle = "";
    m_strDescr = "";
    //}}AFX_DATA_INIT
}

void CScnPropDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CScnPropDialog)
    DDX_Text(pDX, IDC_D_SCNPRP_AUTHOR, m_strAuthor);
    DDV_MaxChars(pDX, m_strAuthor, 60);
    DDX_Text(pDX, IDC_D_SCNPRP_TITLE, m_strTitle);
    DDV_MaxChars(pDX, m_strTitle, 60);
    DDX_Text(pDX, IDC_D_SCNPRP_DESCR, m_strDescr);
    DDV_MaxChars(pDX, m_strDescr, 2500);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScnPropDialog, CDialog)
    //{{AFX_MSG_MAP(CScnPropDialog)
        // NOTE: the ClassWizard will add message map macros here
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_SCNPRP_AUTHOR, IDH_D_SCNPRP_AUTHOR,
    IDC_D_SCNPRP_DESCR, IDH_D_SCNPRP_DESCR,
    IDC_D_SCNPRP_TITLE, IDH_D_SCNPRP_TITLE,
    0,0
};

BOOL CScnPropDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CScnPropDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

