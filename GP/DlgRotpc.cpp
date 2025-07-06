// DlgRotpc.cpp : implementation file
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
#include "DlgRotpc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRotatePieceDialog dialog

CRotatePieceDialog::CRotatePieceDialog(CWnd& v, wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CRotatePieceDialog)
        CB_XRC_CTRL_VAL(m_editCurVal, m_nRelativeRotation, -359, 359, wxNUM_VAL_SIGN_PLUS)
    CB_XRC_END_CTRLS_DEFN(),
    view(&v)
{
    // KLUDGE:  don't see a way to use GetSizeFromText() in .xrc
    wxSize size = m_editCurVal->GetSizeFromText("+999"_cbstring);
    m_editCurVal->CacheBestSize(size);

    SetMinSize(wxDefaultSize);
    Layout();
    Fit();
    Centre();

    m_nRelativeRotation = 0;
}


wxBEGIN_EVENT_TABLE(CRotatePieceDialog, wxDialog)
    EVT_BUTTON(XRCID("OnRotatePieceCCW1"), OnRotatePieceCCW1)
    EVT_BUTTON(XRCID("OnRotatePieceCCW5"), OnRotatePieceCCW5)
    EVT_BUTTON(XRCID("OnRotatePieceCCW10"), OnRotatePieceCCW10)
    EVT_BUTTON(XRCID("OnRotatePieceCCW50"), OnRotatePieceCCW50)
    EVT_BUTTON(XRCID("OnRotatePieceCW1"), OnRotatePieceCW1)
    EVT_BUTTON(XRCID("OnRotatePieceCW5"), OnRotatePieceCW5)
    EVT_BUTTON(XRCID("OnRotatePieceCW10"), OnRotatePieceCW10)
    EVT_BUTTON(XRCID("OnRotatePieceCW50"), OnRotatePieceCW50)
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
#endif

/////////////////////////////////////////////////////////////////////////////
// CRotatePieceDialog message handlers

void CRotatePieceDialog::ApplyOffset(int nOffset)
{
    m_nRelativeRotation = (m_nRelativeRotation + nOffset) % 360;
    view->SendMessage(WM_ROTATEPIECE_DELTA, (WPARAM)m_nRelativeRotation);
    TransferDataToWindow();
}

void CRotatePieceDialog::OnRotatePieceCCW1(wxCommandEvent& /*event*/)
{
    ApplyOffset(-1);
}

void CRotatePieceDialog::OnRotatePieceCCW5(wxCommandEvent& /*event*/)
{
    ApplyOffset(-5);
}

void CRotatePieceDialog::OnRotatePieceCCW10(wxCommandEvent& /*event*/)
{
    ApplyOffset(-10);
}

void CRotatePieceDialog::OnRotatePieceCCW50(wxCommandEvent& /*event*/)
{
    ApplyOffset(-50);
}

void CRotatePieceDialog::OnRotatePieceCW1(wxCommandEvent& /*event*/)
{
    ApplyOffset(1);
}

void CRotatePieceDialog::OnRotatePieceCW5(wxCommandEvent& /*event*/)
{
    ApplyOffset(5);
}

void CRotatePieceDialog::OnRotatePieceCW10(wxCommandEvent& /*event*/)
{
    ApplyOffset(10);
}

void CRotatePieceDialog::OnRotatePieceCW50(wxCommandEvent& /*event*/)
{
    ApplyOffset(50);
}
