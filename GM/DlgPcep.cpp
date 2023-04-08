// DlgPcep.cpp : implementation file
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
#include "DlgPcep.h"
#include "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPiecePropDialog dialog

CPiecePropDialog::CPiecePropDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CPiecePropDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CPiecePropDialog)
    m_strName = "";
    //}}AFX_DATA_INIT
}

void CPiecePropDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPiecePropDialog)
    DDX_Control(pDX, IDC_D_PGRP_NAME, m_editName);
    DDX_Text(pDX, IDC_D_PGRP_NAME, m_strName);
    DDV_MaxChars(pDX, m_strName, 32);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPiecePropDialog, CDialog)
    //{{AFX_MSG_MAP(CPiecePropDialog)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPiecePropDialog message handlers

void CPiecePropDialog::OnOK()
{
    CB::string str = CB::string::GetWindowText(m_editName);
    if (str.empty())
    {
        AfxMessageBox(IDS_ERR_PIECESETNAME, MB_OK | MB_ICONEXCLAMATION);
        m_editName.SetFocus();
        return;
    }
    CDialog::OnOK();
}
