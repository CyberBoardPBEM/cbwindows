// DlgRmsg.cpp : implementation file
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
#include    "Gp.h"
#include    "GamDoc.h"
#include    "ResTbl.h"
#include    "DlgRmsg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CReadMsgDialog, CDialog)

BEGIN_MESSAGE_MAP(CReadMsgDialog, CDialog)
    //{{AFX_MSG_MAP(CReadMsgDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_RMSG_MESSAGE, IDH_D_RMSG_MESSAGE,
    0,0
};

BOOL CReadMsgDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CReadMsgDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////
// Make sure we don't eat accelerators!

BOOL CReadMsgDialog::PreTranslateMessage(MSG* pMsg)
{
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CReadMsgDialog dialog

CReadMsgDialog::CReadMsgDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CReadMsgDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CReadMsgDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_pDoc = NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CReadMsgDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CReadMsgDialog)
    DDX_Control(pDX, IDC_D_RMSG_MESSAGE, m_strMsg);
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

void CReadMsgDialog::SetText()
{
    CString str = m_pDoc->MsgGetMessageText();
    // Search the message string backwards for a "\xBB\r\n" or start of buffer.
    // This would mark the start of a newly appended message. We want to
    // position the editbox to the start of the new message.
    for (int i = str.GetLength()-2; i > 0; i--)
    {
        if (str.GetAt(i) == '\xBB' && str.GetAt(i+1) == '\r') // 0xBB = Chevron
        {
            i += 3;             // Position to start of next line
            break;
        }
    }
    if (i < 0) i = 0;
    m_strMsg.SetWindowText(str);
    // To ensure the new message is at (or as close as can be) the
    // top line in the edit box, first set the selection to the last
    // character then to the actual line.
    m_strMsg.SetSel(str.GetLength(), str.GetLength());
    m_strMsg.SetSel(i, i);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CReadMsgDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(m_pDoc != NULL);
    m_strMsg.SetFont(CFont::FromHandle(g_res.h8ss));
    SetText();
    return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CReadMsgDialog message handlers

void CReadMsgDialog::OnOK()
{
    CString str;
    m_strMsg.GetWindowText(str);
    m_pDoc->MsgDialogClose(str);
}

void CReadMsgDialog::OnSysCommand(UINT nID, LPARAM lParam)
{
    if (nID == SC_CLOSE)
        OnOK();
    else
        CDialog::OnSysCommand(nID, lParam);
}
