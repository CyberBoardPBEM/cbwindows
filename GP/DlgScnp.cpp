// DlgScnp.cpp : implementation file
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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

CScnPropDialog::CScnPropDialog(wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CScnPropDialog)
        CB_XRC_CTRL_VAL(m_editAuthor, m_strAuthor, wxFILTER_NONE, 60)
        CB_XRC_CTRL_VAL(m_editTitle, m_strTitle, wxFILTER_NONE, 60)
        CB_XRC_CTRL_VAL(m_editDescr, m_strDescr, wxFILTER_NONE, 2500)
    CB_XRC_END_CTRLS_DEFN()
{
    // KLUDGE:  don't see a way to use GetSizeFromText() in .xrc
    wxSize size = m_editTitle->GetSizeFromText(m_editTitle->GetValue());
    m_editTitle->CacheBestSize(size);

    SetMinSize(wxDefaultSize);
    Layout();
    Fit();
    Centre();

    m_strAuthor = "";
    m_strTitle = "";
    m_strDescr = "";
}

wxBEGIN_EVENT_TABLE(CScnPropDialog, wxDialog)
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
#endif

