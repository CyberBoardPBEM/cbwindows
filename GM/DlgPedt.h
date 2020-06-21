// DlgPedt.h : header file
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

#ifndef     _LBOXTILE_H
#include    "LBoxTile.h"
#endif

#ifndef     _LBOXPIEC_H
#include    "LBoxPiec.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CPieceEditDialog dialog

class CPieceEditDialog : public CDialog
{
// Construction
public:
    CPieceEditDialog(CWnd* pParent = NULL); // standard constructor

// Dialog Data
    //{{AFX_DATA(CPieceEditDialog)
    enum { IDD = IDD_PIECEEDIT };
    CButton m_chkTopOnlyOwnersToo;
    CButton m_chkTopOnlyVisible;
    CEdit   m_editTextBack;
    CButton m_chkSameAsTop;
    CEdit   m_editTextFront;
    CPieceListBox   m_listPieces;
    CComboBox   m_comboFtset;
    CTileListBox    m_listFtile;
    CComboBox   m_comboBtset;
    CTileListBox    m_listBtile;
    CButton m_chkBack;
    //}}AFX_DATA

    CGamDoc*        m_pDoc;         // Caller must set this
    PieceID         m_pid;          // Piece to edit

// Implementation
protected:
    CWordArray      m_tbl;          // Need to use list box.
    CTileManager*   m_pTMgr;        // Set internally from m_pDoc
    CPieceManager*  m_pPMgr;        // Set internally from m_pDoc
    // -------- //
    void SetupPieceTiles();
    void SetupTileSetNames(CComboBox* pCombo);
    void SetupTileListbox(CComboBox *pCombo, CTileListBox *pList);
    TileID GetTileID(CComboBox *pCombo, CTileListBox *pList);
    // -------- //
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CPieceEditDialog)
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnBackCheck();
    afx_msg void OnSelchangeBtset();
    afx_msg void OnSelchangeFtset();
    afx_msg void OnDblclkPiece();
    afx_msg void OnCheckSameAsTop();
    afx_msg void OnBtnClickTopVisible();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
