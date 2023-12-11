// DlgBmask.cpp : implementation file
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
#include "Board.h"
#include "DlgBmask.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardMaskDialog dialog


CBoardMaskDialog::CBoardMaskDialog(CBoardManager& bmgr, wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    m_pBMgr(&bmgr),
    /* m_dummy is a way to call LoadDialog()
        before the Refs are initialized */
    m_dummy(wxXmlResource::Get()->LoadDialog(this, parent, "CBoardMaskDialog") ? this : nullptr),
    m_lboxBoard(XRCCTRL(*this, "m_lboxBoard", wxListBox))
{
    wxASSERT(!m_pBMgr->IsEmpty());
    m_lboxBoard->SetValidator(wxGenericValidator(&m_nBrdNum));

    m_lboxBoard->Clear();
    for (size_t i = 0; i < m_pBMgr->GetNumBoards(); i++)
        m_lboxBoard->Append(m_pBMgr->GetBoard(i).GetName());
}

wxBEGIN_EVENT_TABLE(CBoardMaskDialog, wxDialog)
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
    IDC_D_BRDMSK_BOARDS, IDH_D_BRDMSK_BOARDS,
    0,0
};

BOOL CBoardMaskDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CBoardMaskDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoardMaskDialog message handlers
