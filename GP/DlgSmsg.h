// DlgSmsg.h : header file
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

#include "Editnocv.h"

/////////////////////////////////////////////////////////////////////////////
// CSendMsgDialog dialog

class CRollState;

class CSendMsgDialog : public CDialog
{
    DECLARE_DYNAMIC(CSendMsgDialog)
// Construction
public:
    CSendMsgDialog(CGamDoc& doc, CWnd* pParent = NULL);
    ~CSendMsgDialog() = default;

// Dialog Data
public:
    //{{AFX_DATA(CSendMsgDialog)
    enum { IDD = IDD_SENDMESSAGE };
    CEditNoChevron  m_editMsg2;
    CButton m_btnDefer;
    CButton m_btnCancel;
    CEditNoChevron  m_editMsg;
    //}}AFX_DATA

    RefPtr<CGamDoc> m_pDoc;
    bool        m_bShowDieRoller;

// Implementation
protected:
    void TransferToReadOnlyView();
    void SetupReadOnlyView();
    void TeardownReadOnlyView();
    void FillEditBoxes(const std::string& str);

    OwnerOrNullPtr<CRollState> m_pRollState;

    bool    m_bReadOnlyView;            // A die roll has occurred

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    // Generated message map functions
    //{{AFX_MSG(CSendMsgDialog)
    afx_msg void OnSendMsgSendAndClose();
    afx_msg void OnSendMsgSend();
    afx_msg void OnSendMsgClose();
    afx_msg void OnSendMsgCancel();
    afx_msg void OnRollDice();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnChangeEditMessage();
    afx_msg void OnChangeEdit2Message();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
