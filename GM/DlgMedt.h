// dlgmedt.h : header file
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

#ifndef     _LBOXMARK_H
#include    "LBoxMark.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkerEditDialog dialog

class CMarkerEditDialog : public CDialog
{
// Construction
public:
    CMarkerEditDialog(CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    //{{AFX_DATA(CMarkerEditDialog)
    enum { IDD = IDD_MARKEREDIT };
    CButton m_chkPromptText;
    CEdit   m_editMarkerText;
    CMarkListBox    m_listMarks;
    CTileListBox    m_listTiles;
    CComboBox       m_comboTSet;
    //}}AFX_DATA

    CGamDoc*        m_pDoc;         // Document
    MarkID          m_mid;          // marker to edit

// Implementation
protected:
    std::vector<MarkID> m_tbl;      // Need to use list box.
    CMarkManager*   m_pMMgr;
    CTileManager*   m_pTMgr;        // Set internally from m_pPMgr
    // --------- //
    void SetupTileListbox();
    void SetupTileSetNames();
    TileID GetTileID();
    void SetupMarkerTile();
    // --------- //
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CMarkerEditDialog)
    virtual void OnOK();
    afx_msg void OnSelchangeTSet();
    afx_msg void OnDblclkMarkers();
    virtual BOOL OnInitDialog();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
