// DlgTexto.cpp : implementation file
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "Font.h"
#include    "GdiTools.h"
#include    "DlgTexto.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextObjDialog dialog


CTextObjDialog::CTextObjDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CTextObjDialog)
        CB_XRC_CTRL(m_btnTxtPropFont)
        CB_XRC_CTRL_VAL(m_editText, m_strText, wxFILTER_NONE, 80)
    CB_XRC_END_CTRLS_DEFN()
{
    m_fontID = 0;
    m_pFontMgr = NULL;
    m_strText = "";

    // KLUDGE:  don't see a way to use GetSizeFromText() in .xrc
    wxSize editSize = m_editText->GetSizeFromText("MMMMMMMMMMMMMMMMMMMMMMMM");
    m_editText->SetInitialSize(editSize);
    SetMinSize(wxDefaultSize);
    Layout();
    Fit();
    Centre();
}

CTextObjDialog::~CTextObjDialog()
{
}

wxBEGIN_EVENT_TABLE(CTextObjDialog, wxDialog)
    EVT_BUTTON(XRCID("m_btnTxtPropFont"), OnBtnTxtPropFont)
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
    IDC_D_TXTPRP_TEXT, IDH_D_TXTPRP_TEXT,
    IDC_D_TXTPRP_FONT, IDH_D_TXTPRP_FONT,
    0,0
};

BOOL CTextObjDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CTextObjDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CTextObjDialog::SetFontID(FontID fontID)
{
    if (fontID != (FontID)0)
    {
        m_fontID = fontID;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CTextObjDialog message handlers

void CTextObjDialog::OnBtnTxtPropFont(wxCommandEvent& /*event*/)
{
    ASSERT(m_pFontMgr != NULL);
    FontID newFontID = DoFontDialog(m_fontID, this);
    if (newFontID != (FontID)0)
        SetFontID(newFontID);
}
