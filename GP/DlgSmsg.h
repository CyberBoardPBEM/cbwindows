// DlgSmsg.h : header file
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

#include "Editnocv.h"

/////////////////////////////////////////////////////////////////////////////
// CSendMsgDialog dialog

class CRollState;

class CSendMsgDialog : public wxDialog
{
// Construction
public:
    CSendMsgDialog(CGamDoc& doc, wxWindow* pParent = &CB::GetMainWndWx());
    ~CSendMsgDialog() = default;

// Dialog Data
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<CEditNoChevron> m_editMsg2;
        RefPtr<wxButton> m_btnDefer;
        RefPtr<wxButton> m_btnCancel;
        RefPtr<CEditNoChevron> m_editMsg;
    CB_XRC_END_CTRLS_DECL()

    RefPtr<CGamDoc> m_pDoc;
public:
    bool        m_bShowDieRoller;

// Implementation
protected:
    void TransferToReadOnlyView();
    void SetupReadOnlyView();
    void TeardownReadOnlyView();
    void FillEditBoxes(const CB::string& str);

    OwnerOrNullPtr<CRollState> m_pRollState;

    bool    m_bReadOnlyView;            // A die roll has occurred

// Implementation
protected:
    bool TransferDataToWindow() override;

    void OnSendMsgSendAndClose(wxCommandEvent& event);
    void OnSendMsgSend(wxCommandEvent& event);
    void OnSendMsgClose(wxCommandEvent& event);
    void OnSendMsgCancel(wxCommandEvent& event);
    void OnRollDice(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    void OnChangeEditMessage(wxCommandEvent& event);
    void OnChangeEdit2Message(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};
