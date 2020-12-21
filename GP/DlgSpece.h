// DlgSpece.h : header file
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

#ifndef     _LBOXPIEC_H
#include    "LBoxPiec.h"
#endif

#ifndef     _LBOXTRAY_H
#include    "LBoxTray.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetPiecesDialog dialog

class CSetPiecesDialog : public CDialog
{
// Construction
public:
    CSetPiecesDialog(CWnd* pParent = NULL); // standard constructor

// Dialog Data
    //{{AFX_DATA(CSetPiecesDialog)
    enum { IDD = IDD_SETPIECES };
    CComboBox       m_comboPGrp;
    CPieceListBox   m_listPiece;
    CComboBox       m_comboYGrp;
    CTrayListBox    m_listTray;
    //}}AFX_DATA

    CGamDoc*        m_pDoc;         // Must be set by creator
    int             m_nYSel;        // Initial tray number

// Implementation
protected:
    CPieceTable*    m_pPTbl;        // Loaded using doc pointer
    CTrayManager*   m_pYMgr;        // Loaded using doc pointer

    std::vector<PieceID> m_tblPiece;
    std::vector<PieceID> m_tblTray;

    // -------- //
    void LoadPieceNameList();
    void LoadTrayNameList();
    void UpdatePieceList();
    void UpdateTrayList();

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    // Generated message map functions
    //{{AFX_MSG(CSetPiecesDialog)
    afx_msg void OnSelChangePieceGroup();
    afx_msg void OnRemoveAll();
    afx_msg void OnCopyAll();
    afx_msg void OnCopySelections();
    afx_msg void OnRemoveSelections();
    afx_msg void OnSelChangeTrayName();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
