// DlgSmsg.cpp : implementation file
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

/////////////////////////////////////////////////////////////////////////////
// CSendMsgDialog dialog

CSendMsgDialog::CSendMsgDialog(CGamDoc& doc, wxWindow* pParent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(pParent, CSendMsgDialog)
        CB_XRC_CTRL(m_editMsg2)
        CB_XRC_CTRL(m_btnDefer)
        , m_btnCancel(XRCCTRL(*this, "wxID_CANCEL", std::remove_reference_t<decltype(*m_btnCancel)>))
        CB_XRC_CTRL(m_editMsg)
    CB_XRC_END_CTRLS_DEFN(),
    m_pDoc(&doc)
{
    // KLUDGE:  Send & Close isn't coming through correctly
    wxButton* sendClose = XRCCTRL(*this, "wxID_OK", wxButton);
    wxString label = sendClose->GetLabel();
    label.Replace("_", "&&");
    sendClose->SetLabel(label);

    /* layout done with both editboxes visible
        so sizers produce proper result */
    m_bReadOnlyView = TRUE;
    // now hide second editbox
    TeardownReadOnlyView();

    m_bShowDieRoller = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CSendMsgDialog, wxDialog)
    EVT_BUTTON(wxID_OK, OnSendMsgSendAndClose)
    EVT_BUTTON(XRCID("OnSendMsgSend"), OnSendMsgSend)
    EVT_BUTTON(XRCID("m_btnDefer"), OnSendMsgClose)
    EVT_BUTTON(wxID_CANCEL, OnSendMsgCancel)
    EVT_BUTTON(XRCID("OnRollDice"), OnRollDice)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
    EVT_TEXT(XRCID("m_editMsg"), OnChangeEditMessage)
    EVT_TEXT(XRCID("m_editMsg2"), OnChangeEdit2Message)
wxEND_EVENT_TABLE()

#if 0
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
#endif

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::TransferToReadOnlyView()
{
    if (!m_bReadOnlyView)
        SetupReadOnlyView();

    CB::string str = m_editMsg2->GetValue();
    AppendStringToEditBox(*m_editMsg, str, TRUE);
    m_editMsg2->SetValue(""_cbstring);
    m_btnCancel->Enable(FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::SetupReadOnlyView()
{
    if (!m_bReadOnlyView)
    {
        m_bReadOnlyView = TRUE;
        // Convert the current edit view to read-only and
        // layout the new edit window.
        m_editMsg->SetEditable(false);
        m_editMsg2->Show();
        Layout();
    }
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::TeardownReadOnlyView()
{
    if (m_bReadOnlyView)
    {
        m_bReadOnlyView = FALSE;
        // Undo the above:
        // Convert the current edit view to read-only and
        // layout the new edit window.
        m_editMsg->SetEditable(true);
        m_editMsg2->Hide();
        Layout();
    }
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::FillEditBoxes(const CB::string& str)
{
    m_editMsg->SetValue(""_cbstring);
    m_editMsg2->SetValue(""_cbstring);

    CB::string strReadOnly;
    CB::string strEditable;

    CGamDoc::MsgSeperateIntoPieces(str, strReadOnly, strEditable);
    if (strReadOnly.empty())
    {
        m_editMsg->SetValue(strEditable);
        m_editMsg->SetFocus();
        m_editMsg->SetInsertionPointEnd();
    }
    else
    {
        SetupReadOnlyView();
        m_editMsg->SetValue(strReadOnly);
        wxTextPos nChars = m_editMsg->GetLastPosition();
        m_editMsg->ShowPosition(nChars);

        m_editMsg2->SetValue(strEditable);
        m_editMsg2->SetFocus();
        m_editMsg->SetInsertionPointEnd();
        if (!strReadOnly.empty())
        {
            // If there is a read-only part in the message, there
            // must have been a die roll. We need to disable cancel.
            m_btnCancel->Enable(FALSE);
        }
        else
            m_btnCancel->Enable(TRUE);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CSendMsgDialog::OnChangeEditMessage(wxCommandEvent& /*event*/)
{
    m_pDoc->SetModifiedFlag();
}

void CSendMsgDialog::OnChangeEdit2Message(wxCommandEvent& /*event*/)
{
    m_pDoc->SetModifiedFlag();
}

/////////////////////////////////////////////////////////////////////////////
// CSendMsgDialog message handlers

bool CSendMsgDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }
    if (m_pDoc->GetDieRollState() != NULL)
    {
        ASSERT(m_pRollState == NULL);
        // Make a copy of the Doc's die roll state
        m_pRollState = MakeOwner<CRollState>(*m_pDoc->GetDieRollState());
    }

#if 0   // why override default?
    m_editMsg.SetFont(CFont::FromHandle(g_res.h8ss));
    m_editMsg2.SetFont(CFont::FromHandle(g_res.h8ss));
#endif

    FillEditBoxes(m_pDoc->MsgGetMessageText());

    if (m_bShowDieRoller)
    {
        QueueEvent(new wxCommandEvent(wxEVT_BUTTON, XRCID("OnRollDice")));
    }

    return true;
}

void CSendMsgDialog::OnSendMsgSendAndClose(wxCommandEvent& /*event*/)
{
    if (m_bReadOnlyView)
        TransferToReadOnlyView();

    CB::string str = m_editMsg->GetValue();
    m_pDoc->SetDieRollState(std::move(m_pRollState));
    m_pDoc->MsgDialogSend(str, TRUE);
}

void CSendMsgDialog::OnSendMsgSend(wxCommandEvent& /*event*/)
{
    if (m_bReadOnlyView)
        TransferToReadOnlyView();

    CB::string str = m_editMsg->GetValue();
    m_pDoc->MsgDialogSend(str, FALSE);      // Don't close us

    TeardownReadOnlyView();                 // Back to original layout
    // Set up with new history data.
    FillEditBoxes(m_pDoc->MsgGetMessageText());
}

void CSendMsgDialog::OnSendMsgClose(wxCommandEvent& /*event*/)
{
    CB::string str;
    if (m_bReadOnlyView)
    {
        CB::string strReadOnly = m_editMsg->GetValue();
        CB::string strEditable = m_editMsg2->GetValue();
        str = CGamDoc::MsgEncodeFromPieces(strReadOnly, strEditable);
    }
    else
    {
        // No read-only part yet
        CB::string strEditable = m_editMsg->GetValue();
        str = CGamDoc::MsgEncodeFromPieces("", strEditable);
    }
    m_pDoc->MsgDialogClose(str);
}

void CSendMsgDialog::OnSendMsgCancel(wxCommandEvent& /*event*/)
{
    m_pDoc->MsgDialogCancel();
}

void CSendMsgDialog::OnRollDice(wxCommandEvent& /*event*/)
{
    CDieRollerDlg dlg;
    if (m_pRollState)
        dlg.SetRollState(*m_pRollState);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_pRollState = dlg.GetRollState();

        CB::string str = m_editMsg2->GetValue();
        if (str != "" && str[str.a_size() - size_t(1)] != '\n')
            AppendStringToEditBox(*m_editMsg2, "\n", FALSE);

        m_editMsg2->AppendText(dlg.GetFormattedRollResult());
        m_btnCancel->Enable(FALSE);

        TransferToReadOnlyView();
    }
}


