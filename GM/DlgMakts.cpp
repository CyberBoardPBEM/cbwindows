// DlbMakts.cpp
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
#include "DlgMakts.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateTSet dialog

CCreateTSet::CCreateTSet(CWnd* pParent /*=NULL*/)
    : CDialog(CCreateTSet::IDD, pParent)
{
    //{{AFX_DATA_INIT(CCreateTSet)
    m_strTSName = "";
    //}}AFX_DATA_INIT
}

void CCreateTSet::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CCreateTSet)
    DDX_Control(pDX, IDC_D_CRTGRP_TSETNAME, m_editTSName);
    DDX_Text(pDX, IDC_D_CRTGRP_TSETNAME, m_strTSName);
    DDV_MaxChars(pDX, m_strTSName, 32);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCreateTSet, CDialog)
    //{{AFX_MSG_MAP(CCreateTSet)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateTSet message handlers

void CCreateTSet::OnOK()
{
    CString str;
    m_editTSName.GetWindowText(str);
    if (str.IsEmpty())
    {
        AfxMessageBox(IDS_ERR_TILESETNAME, MB_OK | MB_ICONEXCLAMATION);
        m_editTSName.SetFocus();
        return;
    }
    CDialog::OnOK();
}

