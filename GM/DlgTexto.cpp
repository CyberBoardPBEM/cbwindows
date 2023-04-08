// DlgTexto.cpp : implementation file
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


CTextObjDialog::CTextObjDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CTextObjDialog::IDD, pParent)
{
    m_fontID = 0;
    m_pFontMgr = NULL;
    //{{AFX_DATA_INIT(CTextObjDialog)
    m_strText = "";
    //}}AFX_DATA_INIT
}

CTextObjDialog::~CTextObjDialog()
{
}

void CTextObjDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTextObjDialog)
    DDX_Text(pDX, IDC_D_TXTPRP_TEXT, m_strText);
    DDV_MaxChars(pDX, m_strText, 80);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTextObjDialog, CDialog)
    //{{AFX_MSG_MAP(CTextObjDialog)
    ON_BN_CLICKED(IDC_D_TXTPRP_FONT, OnBtnTxtPropFont)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

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

void CTextObjDialog::OnBtnTxtPropFont()
{
    ASSERT(m_pFontMgr != NULL);
    FontID newFontID = DoFontDialog(m_fontID, this);
    if (newFontID != (FontID)0)
        SetFontID(newFontID);
}
