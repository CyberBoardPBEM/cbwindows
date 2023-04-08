// DlgPEditMulti.h : header file
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

#ifndef _DLGPEDITMULTI_H_
#define _DLGPEDITMULTI_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPieceEditMultipleDialog dialog

class CPieceEditMultipleDialog : public CDialog
{
// Construction
public:
    CPieceEditMultipleDialog(size_t sides, CWnd* pParent = NULL);   // standard constructor
    const size_t m_sides;

// Dialog Data
    //{{AFX_DATA(CPieceEditMultipleDialog)
    enum { IDD = IDD_PIECE_EDIT_MULTIPLE };
    CButton m_chkTopVisibleOwnersToo;
    CStatic m_staticFrontLabel;
    CButton m_chkSetBackText;
    CButton m_chkSetFrontText;
    CStatic m_staticBackLabel;
    CButton m_chkTopVisible;
    CEdit   m_editBack;
    CEdit   m_editFront;
    //}}AFX_DATA

    BOOL    m_bSetTopOnlyVisible;               // TRUE if m_bTopOnlyVisible changed
    BOOL    m_bTopOnlyVisible;
    BOOL    m_bTopOnlyOwnersToo;

    std::vector<bool> m_bSetTexts;                      // TRUE if text should change
    std::vector<CB::string> m_strs;

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CPieceEditMultipleDialog)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    void UpdateTextControls();

    // Generated message map functions
    //{{AFX_MSG(CPieceEditMultipleDialog)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnBtnClickChangeBack();
    afx_msg void OnBtnClickChangeFront();
    afx_msg void OnBtnClickTopVisible();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnSelchangeCurrSide();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    /* sides are 0-based since they are vector indices, but we
        will display them 1-based for human readability */
    // m_currSide should contain 2 - sides
    CComboBox m_currSide;
    size_t m_prevSide = std::numeric_limits<size_t>::max();
};

#endif
