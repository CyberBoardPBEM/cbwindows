// DlgSmsg.cpp : implementation file
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
#include    "LibMfc.h"
#include    "DlgSmsg.h"
#include    "DlgDice.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSendMsgDialog, CDialog)

/////////////////////////////////////////////////////////////////////////////
// CSendMsgDialog dialog

CSendMsgDialog::CSendMsgDialog(CGamDoc& doc, CWnd* pParent /*=NULL*/)
    : CDialog(CSendMsgDialog::IDD, pParent),
    m_pDoc(&doc)
{
    //{{AFX_DATA_INIT(CSendMsgDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_bReadOnlyView = FALSE;
    m_bShowDieRoller = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSendMsgDialog)
    DDX_Control(pDX, IDC_D_SMSG_MESSAGE2, m_editMsg2);
    DDX_Control(pDX, IDC_D_SMSG_CLOSE, m_btnDefer);
    DDX_Control(pDX, IDCANCEL, m_btnCancel);
    DDX_Control(pDX, IDC_D_SMSG_MESSAGE, m_editMsg);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSendMsgDialog, CDialog)
    //{{AFX_MSG_MAP(CSendMsgDialog)
    ON_BN_CLICKED(IDOK, OnSendMsgSendAndClose)
    ON_BN_CLICKED(IDC_D_SMSG_SEND, OnSendMsgSend)
    ON_BN_CLICKED(IDC_D_SMSG_CLOSE, OnSendMsgClose)
    ON_BN_CLICKED(IDCANCEL, OnSendMsgCancel)
    ON_BN_CLICKED(IDC_D_SMSG_ROLLDICE, OnRollDice)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_EN_CHANGE(IDC_D_SMSG_MESSAGE, OnChangeEditMessage)
    ON_EN_CHANGE(IDC_D_SMSG_MESSAGE2, OnChangeEdit2Message)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_SMSG_MESSAGE2, IDH_D_SMSG_MESSAGE2,
    IDC_D_SMSG_CLOSE, IDH_D_SMSG_CLOSE,
    IDC_D_SMSG_MESSAGE, IDH_D_SMSG_MESSAGE,
    IDC_D_SMSG_MESSAGE2, IDH_D_SMSG_MESSAGE2,
    IDC_D_SMSG_SEND, IDH_D_SMSG_SEND,
    IDCANCEL, IDH_D_SMSG_CANCEL,
    IDOK, IDH_D_SMSG_OK,
    0,0
};

BOOL CSendMsgDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CSendMsgDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::TransferToReadOnlyView()
{
    if (!m_bReadOnlyView)
        SetupReadOnlyView();

    CString str;
    m_editMsg2.GetWindowText(str);
    AppendStringToEditBox(m_editMsg, str, TRUE);
    m_editMsg2.SetWindowText("");
    m_btnCancel.EnableWindow(FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::SetupReadOnlyView()
{
    if (!m_bReadOnlyView)
    {
        m_bReadOnlyView = TRUE;
        // Convert the current edit view to read-only and
        // layout the new edit window.
        CRect rctTop;
        m_editMsg.GetWindowRect(&rctTop);
        ScreenToClient(&rctTop);
        CRect rctBot = rctTop;

        rctTop.bottom = rctTop.top + (2 * rctTop.Height()) / 3 - 1;
        rctBot.top = rctTop.bottom;

        m_editMsg.SetReadOnly(TRUE);
        m_editMsg.MoveWindow(&rctTop);
        m_editMsg2.MoveWindow(&rctBot);
        m_editMsg2.ShowWindow(SW_SHOW);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::TeardownReadOnlyView()
{
    if (m_bReadOnlyView)
    {
        m_bReadOnlyView = FALSE;
        // Convert the current edit view to read-only and
        // layout the new edit window.
        CRect rctTop;
        m_editMsg.GetWindowRect(&rctTop);
        ScreenToClient(&rctTop);
        CRect rctBot;
        m_editMsg2.GetWindowRect(&rctBot);
        ScreenToClient(&rctBot);
        rctTop.bottom = rctBot.bottom;

        m_editMsg.SetReadOnly(FALSE);
        m_editMsg.MoveWindow(&rctTop);
        m_editMsg2.ShowWindow(SW_HIDE);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::FillEditBoxes(const std::string& str)
{
    m_editMsg.SetWindowText("");
    m_editMsg2.SetWindowText("");

    CString strReadOnly;
    CString strEditable;

    CGamDoc::MsgSeperateIntoPieces(str.c_str(), strReadOnly, strEditable);
    if (strReadOnly.IsEmpty())
    {
        m_editMsg.SetWindowText(strEditable);
        m_editMsg.SetFocus();
        m_editMsg.SetSel(m_editMsg.GetWindowTextLength(),
            m_editMsg.GetWindowTextLength());
    }
    else
    {
        SetupReadOnlyView();
        m_editMsg.SetWindowText(strReadOnly);
        int nChars = m_editMsg.GetWindowTextLength();
        int nLine = m_editMsg.LineFromChar(nChars);
        m_editMsg.LineScroll(nLine);

        m_editMsg2.SetWindowText(strEditable);
        m_editMsg2.SetFocus();
        m_editMsg2.SetSel(m_editMsg2.GetWindowTextLength(),
            m_editMsg2.GetWindowTextLength());
        if (!strReadOnly.IsEmpty())
        {
            // If there is a read-only part in the message, there
            // must have been a die roll. We need to disable cancel.
            m_btnCancel.EnableWindow(FALSE);
        }
        else
            m_btnCancel.EnableWindow(TRUE);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::OnChangeEditMessage()
{
    m_pDoc->SetModifiedFlag();
}

void CSendMsgDialog::OnChangeEdit2Message()
{
    m_pDoc->SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////
// CSendMsgDialog message handlers

BOOL CSendMsgDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    if (m_pDoc->GetDieRollState() != NULL)
    {
        ASSERT(m_pRollState == NULL);
        // Make a copy of the Doc's die roll state
        m_pRollState = MakeOwner<CRollState>(*m_pDoc->GetDieRollState());
    }

    m_editMsg.SetFont(CFont::FromHandle(g_res.h8ss));
    m_editMsg2.SetFont(CFont::FromHandle(g_res.h8ss));

    FillEditBoxes(m_pDoc->MsgGetMessageText().GetString());

    if (m_bShowDieRoller)
    {
        PostMessage(WM_COMMAND, MAKEWPARAM(uint16_t(IDC_D_SMSG_ROLLDICE), uint16_t(BN_CLICKED)),
            (LPARAM)::GetDlgItem(m_hWnd, IDC_D_SMSG_ROLLDICE));
    }

    return FALSE;  // return TRUE  unless you set the focus to a control
}

void CSendMsgDialog::OnSendMsgSendAndClose()
{
    if (m_bReadOnlyView)
        TransferToReadOnlyView();

    CString str;
    m_editMsg.GetWindowText(str);
    m_pDoc->SetDieRollState(std::move(m_pRollState));
    m_pDoc->MsgDialogSend(str, TRUE);
}

void CSendMsgDialog::OnSendMsgSend()
{
    if (m_bReadOnlyView)
        TransferToReadOnlyView();

    CString str;
    m_editMsg.GetWindowText(str);
    m_pDoc->MsgDialogSend(str, FALSE);      // Don't close us

    TeardownReadOnlyView();                 // Back to original layout
    // Set up with new history data.
    FillEditBoxes(m_pDoc->MsgGetMessageText().GetString());
}

void CSendMsgDialog::OnSendMsgClose()
{
    CString str;
    if (m_bReadOnlyView)
    {
        CString strReadOnly;
        CString strEditable;
        m_editMsg.GetWindowText(strReadOnly);
        m_editMsg2.GetWindowText(strEditable);
        str = CGamDoc::MsgEncodeFromPieces(strReadOnly, strEditable);
    }
    else
    {
        // No read-only part yet
        CString strEditable;
        m_editMsg.GetWindowText(strEditable);
        str = CGamDoc::MsgEncodeFromPieces(CString(""), strEditable);
    }
    m_pDoc->MsgDialogClose(str);
}

void CSendMsgDialog::OnSendMsgCancel()
{
    m_pDoc->MsgDialogCancel();
}

void CSendMsgDialog::OnRollDice()
{
    CDieRollerDlg dlg;
    if (m_pRollState)
        dlg.SetRollState(*m_pRollState);

    if (dlg.DoModal() == IDOK)
    {
        m_pRollState = dlg.GetRollState();

        CString str;
        m_editMsg2.GetWindowText(str);
        if (str != "" && str.GetAt(str.GetLength() - 1) != '\n')
            AppendStringToEditBox(m_editMsg2, "\r\n", FALSE);

        int nLen = m_editMsg2.GetWindowTextLength();
        m_editMsg2.SetSel(nLen, nLen);
        m_editMsg2.ReplaceSel(dlg.GetFormattedRollResult().c_str());
        m_btnCancel.EnableWindow(FALSE);

        TransferToReadOnlyView();
    }
}


