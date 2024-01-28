// DlgMrkp.cpp : implementation file
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

#include "stdafx.h"
#include "Gm.h"
#include "DlgMrkp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkerPropDialog dialog

CMarkerPropDialog::CMarkerPropDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CMarkerPropDialog)
        CB_XRC_CTRL_VAL(m_editName, m_strName, wxFILTER_EMPTY)
        CB_XRC_CTRL_VAL(m_radioMarkerViz, m_nMarkerVizHelper)
    CB_XRC_END_CTRLS_DEFN()
{
    m_strName = "";
    m_nMarkerViz = mtrayVizNormal;
}

#if 0
void CMarkerPropDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMarkerPropDialog)
    DDX_Control(pDX, IDC_D_MARKGRP_NAME, m_editName);
    DDX_Text(pDX, IDC_D_MARKGRP_NAME, m_strName);
    DDX_Radio(pDX, IDC_D_MARKGRP_VIZFULL, m_nMarkerViz);
    //}}AFX_DATA_MAP
}
#endif

wxBEGIN_EVENT_TABLE(CMarkerPropDialog, wxDialog)
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
    IDC_D_MARKGRP_NAME, IDH_D_MARKGRP_NAME,
    IDC_D_MARKGRP_VIZFULL, IDH_D_MARKGRP_VIZFULL,
    IDC_D_MARKGRP_RAND_VIZHIDDEN, IDH_D_MARKGRP_RAND_VIZHIDDEN,
    IDC_D_MARKGRP_RAND_VIZALLHIDDEN, IDH_D_MARKGRP_RAND_VIZALLHIDDEN,
    0,0
};

BOOL CMarkerPropDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CMarkerPropDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkerPropDialog message handlers

bool CMarkerPropDialog::TransferDataToWindow()
{
    m_nMarkerVizHelper = m_nMarkerViz;
    return wxDialog::TransferDataToWindow();
}

bool CMarkerPropDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }
    m_nMarkerViz = static_cast<MarkerTrayViz>(m_nMarkerVizHelper);
    return true;
}
