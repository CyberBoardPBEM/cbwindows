// DlgMPly.h : header file
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

#ifndef _DLGMPLY_H_
#define _DLGMPLY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMultiplayerGameDialog dialog

class CPlayerManager;

class CMultiplayerGameDialog : public wxDialog
{
// Construction
public:
    CMultiplayerGameDialog(CPlayerManager& pm, wxWindow* pParent = &CB::GetMainWndWx());   // standard constructor

// Dialog Data
    bool    m_bCreateReferee;
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxStaticText> m_static1;
        RefPtr<wxCheckBox> m_chkCreateReferee;
        RefPtr<wxStaticText> m_static2;
        RefPtr<wxTextCtrl> m_editPlayer;
        RefPtr<wxListBox>  m_listPlayers;
    CB_XRC_END_CTRLS_DECL()

    RefPtr<CPlayerManager> m_pPlayerMgr;

// Overrides

// Implementation
protected:

    void OnBtnPressUpdateName(wxCommandEvent& event);
    void OnSelChangePlayerList(wxCommandEvent& event);
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();
};

#endif
