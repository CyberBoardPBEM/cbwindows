// DlgGboxp.cpp : implementation file
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
#include    "StrLib.h"
#include    "DlgGboxp.h"
#include    "zlib.h"
#include    "DlgSpass.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGmBoxPropsDialog dialog


CGmBoxPropsDialog::CGmBoxPropsDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CGmBoxPropsDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CGmBoxPropsDialog)
    m_strAuthor = "";
    m_strDescr = "";
    m_strTitle = "";
    //}}AFX_DATA_INIT
    m_nCompressLevel = Z_BEST_SPEED;
    m_bPropEdit = TRUE;

    m_bPassSet = FALSE;
    m_strPassword = "";
}

void CGmBoxPropsDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGmBoxPropsDialog)
    DDX_Control(pDX, IDC_D_GBXPRP_COMPRESSION, m_comboCompress);
    DDX_Control(pDX, IDC_D_GBXPRP_AUTHOR, m_editAuthor);
    DDX_Text(pDX, IDC_D_GBXPRP_AUTHOR, m_strAuthor);
    DDV_MaxChars(pDX, m_strAuthor, 40);
    DDX_Text(pDX, IDC_D_GBXPRP_DESCR, m_strDescr);
    DDV_MaxChars(pDX, m_strDescr, 2000);
    DDX_Text(pDX, IDC_D_GBXPRP_TITLE, m_strTitle);
    DDV_MaxChars(pDX, m_strTitle, 60);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGmBoxPropsDialog, CDialog)
    //{{AFX_MSG_MAP(CGmBoxPropsDialog)
    ON_BN_CLICKED(IDC_D_SET_PASSWORD, OnSetPassword)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_GBXPRP_AUTHOR, IDH_D_GBXPRP_AUTHOR,
    IDC_D_GBXPRP_COMPRESSION, IDH_D_GBXPRP_COMPRESSION,
    IDC_D_GBXPRP_DESCR, IDH_D_GBXPRP_DESCR,
    IDC_D_GBXPRP_TITLE, IDH_D_GBXPRP_TITLE,
    IDC_D_SET_PASSWORD, IDH_D_SET_PASSWORD,
    0,0
};

BOOL CGmBoxPropsDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CGmBoxPropsDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}


/////////////////////////////////////////////////////////////////////////////
// CGmBoxPropsDialog message handlers

void CGmBoxPropsDialog::OnOK()
{
    CDialog::OnOK();
    m_nCompressLevel = value_preserving_cast<int>(m_comboCompress.GetItemData(m_comboCompress.GetCurSel()));
}


BOOL CGmBoxPropsDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString str;
    str.LoadString(IDS_COMPRESS_NONE);
    int nItem = m_comboCompress.AddString(str);
    m_comboCompress.SetItemData(nItem, Z_NO_COMPRESSION);

    str.LoadString(IDS_COMPRESS_FASTEST);
    nItem = m_comboCompress.AddString(str);
    m_comboCompress.SetItemData(nItem, Z_BEST_SPEED);

    str.LoadString(IDS_COMPRESS_MEDIUM);
    nItem = m_comboCompress.AddString(str);
    m_comboCompress.SetItemData(nItem, 6);

    str.LoadString(IDS_COMPRESS_MOST);
    nItem = m_comboCompress.AddString(str);
    m_comboCompress.SetItemData(nItem, Z_BEST_COMPRESSION);

    switch (m_nCompressLevel)
    {
        case Z_NO_COMPRESSION:      m_comboCompress.SetCurSel(0); break;
        case Z_BEST_SPEED:          m_comboCompress.SetCurSel(1); break;
        case 6:                     m_comboCompress.SetCurSel(2); break;
        case Z_BEST_COMPRESSION:    m_comboCompress.SetCurSel(3); break;
        default: ASSERT(FALSE);     m_comboCompress.SetCurSel(1); break;// Shouldn't happen
    }

//  if (m_bPropEdit)
//  {
//      m_editAuthor.SetFocus();
//      return FALSE;
//  }
    return TRUE;  // Return TRUE  unless you set the focus to a control
}

void CGmBoxPropsDialog::OnSetPassword()
{
    CSetGameboxPassword dlg;
    if (dlg.DoModal() == IDOK)
    {
        m_strPassword = dlg.m_strPass1;
        m_bPassSet = TRUE;
    }
}
