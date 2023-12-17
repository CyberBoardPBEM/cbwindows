// DlgMEditMulti.cpp : implementation file
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
#include "DlgMEditMulti.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkerEditMultipleDialog dialog


CMarkerEditMultipleDialog::CMarkerEditMultipleDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CMarkerEditMultipleDialog)
        CB_XRC_CTRL_VAL(m_chkSetText, m_bSetText)
        CB_XRC_CTRL(m_staticTextLabel)
        CB_XRC_CTRL_VAL(m_chkPromptForText, m_bPromptForText)
        CB_XRC_CTRL_VAL(m_editText, m_strText, wxFILTER_NONE, 0)
    CB_XRC_END_CTRLS_DEFN()
{
    m_bSetText = false;
    m_bPromptForText = wxCHK_UNDETERMINED;
}


wxBEGIN_EVENT_TABLE(CMarkerEditMultipleDialog, wxDialog)
    EVT_INIT_DIALOG(OnInitDialog)
    EVT_CHECKBOX(XRCID("m_chkSetText"), OnBtnClickChangeText)
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
    IDC_D_MMEDIT_CHG_TEXT, IDH_D_MMEDIT_CHG_TEXT,
    IDC_D_MMEDIT_TEXTPROMPT, IDH_D_MMEDIT_TEXTPROMPT,
    IDC_D_MMEDIT_TEXT, IDH_D_MMEDIT_TEXT,
    0,0
};

BOOL CMarkerEditMultipleDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CMarkerEditMultipleDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CMarkerEditMultipleDialog::UpdateTextControls()
{
    if (m_chkSetText->IsChecked())
    {
        m_editText->Enable(true);
        m_staticTextLabel->Enable(true);
    }
    else
    {
        m_editText->Enable(false);
        m_editText->Clear();
        m_staticTextLabel->Enable(false);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CMarkerEditMultipleDialog message handlers

void CMarkerEditMultipleDialog::OnBtnClickChangeText(wxCommandEvent& /*event*/)
{
    UpdateTextControls();
}

void CMarkerEditMultipleDialog::OnInitDialog(wxInitDialogEvent& event)
{
    /* UpdateTextControls should be called only after base
        class does its work.  event.Skip() ensures base
        class does its work */
    event.Skip();

    CallAfter(&CMarkerEditMultipleDialog::UpdateTextControls);
}
