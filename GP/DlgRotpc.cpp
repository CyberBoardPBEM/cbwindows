// DlgRotpc.cpp : implementation file
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
#include "DlgRotpc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRotatePieceDialog dialog

CRotatePieceDialog::CRotatePieceDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CRotatePieceDialog::IDD, pParent)
{
    m_nRelativeRotation = 0;
}


void CRotatePieceDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CRotatePieceDialog)
    DDX_Control(pDX, IDC_D_ROTPCE_CURVAL, m_editCurVal);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRotatePieceDialog, CDialog)
    //{{AFX_MSG_MAP(CRotatePieceDialog)
    ON_BN_CLICKED(IDC_D_ROTPCE_CCW1, OnRotatePieceCCW1)
    ON_BN_CLICKED(IDC_D_ROTPCE_CCW5, OnRotatePieceCCW5)
    ON_BN_CLICKED(IDC_D_ROTPCE_CCW10, OnRotatePieceCCW10)
    ON_BN_CLICKED(IDC_D_ROTPCE_CCW50, OnRotatePieceCCW50)
    ON_BN_CLICKED(IDC_D_ROTPCE_CW1, OnRotatePieceCW1)
    ON_BN_CLICKED(IDC_D_ROTPCE_CW5, OnRotatePieceCW5)
    ON_BN_CLICKED(IDC_D_ROTPCE_CW10, OnRotatePieceCW10)
    ON_BN_CLICKED(IDC_D_ROTPCE_CW50, OnRotatePieceCW50)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_ROTPCE_CCW1, IDH_D_ROTPCE_CCW1,
    IDC_D_ROTPCE_CCW5, IDH_D_ROTPCE_CCW5,
    IDC_D_ROTPCE_CCW10, IDH_D_ROTPCE_CCW10,
    IDC_D_ROTPCE_CCW50, IDH_D_ROTPCE_CCW50,
    IDC_D_ROTPCE_CW1, IDH_D_ROTPCE_CW1,
    IDC_D_ROTPCE_CW5, IDH_D_ROTPCE_CW5,
    IDC_D_ROTPCE_CW10, IDH_D_ROTPCE_CW10,
    IDC_D_ROTPCE_CW50, IDH_D_ROTPCE_CW50,
    IDC_D_ROTPCE_CURVAL, IDH_D_ROTPCE_CURVAL,
    0,0
};

BOOL CRotatePieceDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CRotatePieceDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// CRotatePieceDialog message handlers

void CRotatePieceDialog::UpdateRotationValue()
{
    TCHAR szBfr[40];
    if (m_nRelativeRotation != 0)
    {
        wsprintf(szBfr, "%c%d", m_nRelativeRotation < 0 ? '-' : '+',
                abs(m_nRelativeRotation));
    }
    else
        lstrcpy(szBfr, "0");
    m_editCurVal.SetWindowText(szBfr);
}

void CRotatePieceDialog::ApplyOffset(int nOffset)
{
    m_nRelativeRotation = (m_nRelativeRotation + nOffset) % 360;
    m_pParentWnd->SendMessage(WM_ROTATEPIECE_DELTA, (WPARAM)m_nRelativeRotation);
    UpdateRotationValue();
}

void CRotatePieceDialog::OnRotatePieceCCW1()
{
    ApplyOffset(-1);
}

void CRotatePieceDialog::OnRotatePieceCCW5()
{
    ApplyOffset(-5);
}

void CRotatePieceDialog::OnRotatePieceCCW10()
{
    ApplyOffset(-10);
}

void CRotatePieceDialog::OnRotatePieceCCW50()
{
    ApplyOffset(-50);
}

void CRotatePieceDialog::OnRotatePieceCW1()
{
    ApplyOffset(1);
}

void CRotatePieceDialog::OnRotatePieceCW5()
{
    ApplyOffset(5);
}

void CRotatePieceDialog::OnRotatePieceCW10()
{
    ApplyOffset(10);
}

void CRotatePieceDialog::OnRotatePieceCW50()
{
    ApplyOffset(50);
}

BOOL CRotatePieceDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    UpdateRotationValue();
    return TRUE;
}
