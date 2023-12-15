// DlgSPass.cpp : implementation file
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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

CSetGameboxPassword::CSetGameboxPassword(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CSetGameboxPassword)
        CB_XRC_CTRL_VAL(m_editPass2, m_strPass2, wxFILTER_NONE, 0)
        CB_XRC_CTRL_VAL(m_editPass1, m_strPass1, wxFILTER_NONE, 0)
    CB_XRC_END_CTRLS_DEFN()
{
}

wxBEGIN_EVENT_TABLE(CSetGameboxPassword, wxDialog)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
wxEND_EVENT_TABLE()

#if 0
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
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetGameboxPassword message handlers

bool CSetGameboxPassword::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    if (m_strPass1 != m_strPass2)
    {
        wxMessageBox(CB::string::LoadString(IDS_WARN_PASS_MUST_MATCH), CB::GetAppName());
        m_editPass1->SetFocus();
        m_editPass1->SetSelection(-1, -1);
        return false;
    }

    return true;
}
