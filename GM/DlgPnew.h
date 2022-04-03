// DlgPnew.h : header file
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
// CPieceNewDialog dialog

class CPieceNewDialog : public CDialog
{
// Construction
public:
    CPieceNewDialog(CGamDoc& doc, size_t nPSet, CWnd* pParent = NULL);  // standard constructor

// Dialog Data
    //{{AFX_DATA(CPieceNewDialog)
    enum { IDD = IDD_PIECEFACTORY };
    CButton m_chkTopOnlyOwnersToo;
    CButton m_chkTopOnlyVisible;
    CEdit   m_editTextFront;
    CEdit   m_editTextBack;
    CButton m_chkSameAsTop;
    CTileListBox    m_listFtile;
    CTileListBox    m_listBtile;
    CComboBox   m_comboBtset;
    CComboBox   m_comboFtset;
    CPieceListBox m_listPieces;
    CEdit   m_editQty;
    //}}AFX_DATA

    RefPtr<CGamDoc> m_pDoc;             // Pointer to document
    const size_t m_nPSet;            // Number of piece set

// Implementation
protected:
    /* sides are 0-based since they are vector indices, but we
        will display them 1-based for human readability */
    CComboBox m_numSides;
    // m_currSide should contain 2 - sides
    CComboBox m_currSide;
    CStatic m_side_1;

    const CTileManager& m_pTMgr;        // Set internally from m_pDoc
    RefPtr<CPieceManager> m_pPMgr;        // Set internally from m_pDoc
    size_t m_prevSide = std::numeric_limits<size_t>::max();
    std::vector<TileID> m_sideTids;
    std::vector<std::string> m_sideTexts;

    // -------- //
    void RefreshPieceList();
    void SetupTileSetNames(CComboBox& pCombo) const;
    void SetupTileListbox(const CComboBox& pCombo, CTileListBox& pList) const;
    void CreatePiece();
    TileID GetTileID(const CComboBox& pCombo, const CTileListBox& pList) const;

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CPieceNewDialog)
    afx_msg void OnSelchangeBtset();
    afx_msg void OnSelchangeFtset();
    afx_msg void OnCreate();
    afx_msg void OnSelchangeNumSides();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnDblClkFrontTile();
    afx_msg void OnCheckTextSameAsTop();
    afx_msg void OnBtnClickTopVisible();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnSelchangeCurrSide();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
