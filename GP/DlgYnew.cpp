// DlgYnew.cpp : implementation file
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
#include    "Gp.h"
#include    "Trays.h"
#include    "DlgYnew.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrayNewDialog dialog


CTrayNewDialog::CTrayNewDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CTrayNewDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CTrayNewDialog)
    m_strName = "";
    //}}AFX_DATA_INIT
}

void CTrayNewDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTrayNewDialog)
    DDX_Control(pDX, IDC_D_YNEW_NAME, m_editName);
    DDX_Text(pDX, IDC_D_YNEW_NAME, m_strName);
    DDV_MaxChars(pDX, m_strName, 32);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTrayNewDialog, CDialog)
    //{{AFX_MSG_MAP(CTrayNewDialog)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTrayNewDialog message handlers

void CTrayNewDialog::OnOK()
{
    m_editName.GetWindowText(m_strName);
    if (m_strName.IsEmpty())
    {
        AfxMessageBox(IDS_ERR_TRAYNAME, MB_OK | MB_ICONINFORMATION);
        m_editName.SetFocus();
        return;
    }
    size_t nSel = m_pYMgr->FindTrayByName(m_strName);
    if (nSel != Invalid_v<size_t>)
    {
        AfxMessageBox(IDS_ERR_TRAYNAMEUSED, MB_OK | MB_ICONINFORMATION);
        m_editName.SetFocus();
        return;
    }
    CDialog::OnOK();
}
