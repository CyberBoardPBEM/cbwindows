// DlgEdtEl.cpp : implementation file
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
#include "DlgEdtEl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditElementTextDialog dialog


CEditElementTextDialog::CEditElementTextDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CEditElementTextDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CEditElementTextDialog)
    m_strText = "";
    m_bSetBothSides = FALSE;
    //}}AFX_DATA_INIT
    m_bDoubleSided = FALSE;
}


void CEditElementTextDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CEditElementTextDialog)
    DDX_Control(pDX, IDC_D_EDITELEM_SETBOTH, m_chkBothSides);
    DDX_Control(pDX, IDC_D_EDITELEM_TEXT, m_editText);
    DDX_Text(pDX, IDC_D_EDITELEM_TEXT, m_strText);
    DDX_Check(pDX, IDC_D_EDITELEM_SETBOTH, m_bSetBothSides);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditElementTextDialog, CDialog)
    //{{AFX_MSG_MAP(CEditElementTextDialog)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_EDITELEM_TEXT, IDH_D_EDITELEM_TEXT,
    IDC_D_EDITELEM_SETBOTH, IDH_D_EDITELEM_SETBOTH,
    0,0
};

BOOL CEditElementTextDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CEditElementTextDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CEditElementTextDialog message handlers

BOOL CEditElementTextDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    int nChars = m_editText.GetWindowTextLength();
    m_editText.SetFocus();
    m_editText.SetSel(nChars, nChars, TRUE);
    m_chkBothSides.EnableWindow(m_bDoubleSided);
    return FALSE;
}
