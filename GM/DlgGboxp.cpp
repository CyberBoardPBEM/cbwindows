// DlgGboxp.cpp : implementation file
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

#include    "stdafx.h"
#include    "Gm.h"
#include    "StrLib.h"
#include    "DlgGboxp.h"
#include    "DlgSpass.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace {
    const int compressLevels[] = {
        wxZ_NO_COMPRESSION,
        wxZ_BEST_SPEED,
        6,
        wxZ_BEST_COMPRESSION,
    };
}

/////////////////////////////////////////////////////////////////////////////
// CGmBoxPropsDialog dialog


CGmBoxPropsDialog::CGmBoxPropsDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    /* m_dummy is a way to call LoadDialog()
        before the Refs are initialized */
    m_dummy(wxXmlResource::Get()->LoadDialog(this, parent, "CGmBoxPropsDialog") ? this : nullptr),
    m_editAuthor(XRCCTRL(*this, "m_editAuthor", wxTextCtrl)),
    m_editTitle(XRCCTRL(*this, "m_editTitle", wxTextCtrl)),
    m_editDescr(XRCCTRL(*this, "m_editDescr", wxTextCtrl)),
    m_comboCompress(XRCCTRL(*this, "m_comboCompress", wxChoice))
{
    m_editAuthor->SetValidator(wxTextValidator(wxFILTER_NONE, &m_strAuthor));
    m_editAuthor->SetMaxLength(40);
    m_editTitle->SetValidator(wxTextValidator(wxFILTER_NONE, &m_strTitle));
    m_editTitle->SetMaxLength(60);
    m_editDescr->SetValidator(wxTextValidator(wxFILTER_NONE, &m_strDescr));
    m_editDescr->SetMaxLength(2000);
    m_comboCompress->SetValidator(wxGenericValidator(&m_nCompressLevelIndex));
}

bool CGmBoxPropsDialog::TransferDataToWindow()
{
    m_nCompressLevelIndex = value_preserving_cast<int>(std::find(std::begin(compressLevels), std::end(compressLevels), m_nCompressLevel) - std::begin(compressLevels));
    return wxDialog::TransferDataToWindow();
}

bool CGmBoxPropsDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }
    m_nCompressLevel = compressLevels[m_nCompressLevelIndex];
    return true;
}

wxBEGIN_EVENT_TABLE(CGmBoxPropsDialog, wxDialog)
    EVT_BUTTON(XRCID("OnSetPassword"), CGmBoxPropsDialog::OnSetPassword)
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
#endif


/////////////////////////////////////////////////////////////////////////////
// CGmBoxPropsDialog message handlers

void CGmBoxPropsDialog::OnSetPassword(wxCommandEvent& /*event*/)
{
    CSetGameboxPassword dlg;
    if (dlg.DoModal() == IDOK)
    {
        m_strPassword = dlg.m_strPass1.wx_str();
        m_bPassSet = true;
    }
}
