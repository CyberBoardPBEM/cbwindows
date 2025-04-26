// DlgEdtEl.cpp : implementation file
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
#include "DlgEdtEl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditElementTextDialog dialog


CEditElementTextDialog::CEditElementTextDialog(wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CEditElementTextDialog)
        CB_XRC_CTRL_VAL(m_chkAllSides, m_bSetAllSides)
        CB_XRC_CTRL_VAL(m_editText, m_strTextTemp)
    CB_XRC_END_CTRLS_DEFN()
{
    m_strText = "";
    m_bSetAllSides = FALSE;
    m_nSides = size_t(1);
}


wxBEGIN_EVENT_TABLE(CEditElementTextDialog, wxDialog)
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
    IDC_D_EDITELEM_TEXT, IDH_D_EDITELEM_TEXT,
    IDC_D_EDITELEM_SETALL, IDH_D_EDITELEM_SETALL,
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
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditElementTextDialog message handlers

bool CEditElementTextDialog::TransferDataToWindow()
{
    m_strTextTemp = m_strText.wx_str();
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    wxTextPos nChars = m_editText->GetLastPosition();
    m_editText->SetFocus();
    m_editText->SetSelection(value_preserving_cast<long>(nChars), value_preserving_cast<long>(nChars));
    m_chkAllSides->Enable(m_nSides >= size_t(2));
    return true;
}

bool CEditElementTextDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }
    m_strText = m_strTextTemp;
    return true;
}
