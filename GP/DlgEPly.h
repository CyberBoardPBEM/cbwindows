// DlgEPly.h : header file
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

#ifndef _DLGEPLY_H_
#define _DLGEPLY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CEditPlayersDialog dialog
class CPlayerManager;

class CEditPlayersDialog : public CDialog
{
// Construction
public:
    CEditPlayersDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CEditPlayersDialog)
    enum { IDD = IDD_EDIT_PLAYERS };
    CButton m_btnUpdate;
    CListBox    m_listNames;
    CEdit   m_editName;
    //}}AFX_DATA

    void SetDialogsPlayerNames(CPlayerManager* pPlayerMgr);
    void GetPlayerNamesFromDialog(CPlayerManager* pPlayerMgr);

    std::vector<CB::string> m_tblNames;

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CEditPlayersDialog)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CEditPlayersDialog)
    virtual void OnOK();
    afx_msg void OnSelChangeNameList();
    afx_msg void OnBtnPressUpdateName();
    virtual BOOL OnInitDialog();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif

