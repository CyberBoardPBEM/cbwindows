// DlgNewGeoBoard.h : header file
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

#ifndef _DLGNEWGEOBOARD_H_
#define _DLGNEWGEOBOARD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCreateGeomorphicBoardDialog dialog

class CGamDoc;
class CGeomorphicBoard;

class CCreateGeomorphicBoardDialog : public CDialog
{
// Construction
public:
    CCreateGeomorphicBoardDialog(CGamDoc& doc, CWnd* pParent = NULL);   // standard constructor

// Operations
public:
    // Caller must delete the returned object when finished with it.
    CGeomorphicBoard* DetachGeomorphicBoard();

// Dialog Data
    //{{AFX_DATA(CCreateGeomorphicBoardDialog)
    enum { IDD = IDD_GEOMORPHIC_BOARD };
    CButton m_btnOK;
    CButton m_btnClearList;
    CButton m_btnAddBreak;
    CButton m_btnAddBoard;
    CListBox    m_listGeo;
    CEdit   m_editBoardName;
    CListBox    m_listBoard;
    //}}AFX_DATA

private:
    RefPtr<CGamDoc> m_pDoc;
public:


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CCreateGeomorphicBoardDialog)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation - vars
protected:

    const CCellForm* m_pRootMapCellForm;
    size_t      m_nCurrentRowHeight;                    // Height of current row maps
    size_t      m_nRowNumber;

    size_t      m_nCurrentColumn;
    size_t      m_nMaxColumns;                       // Set when first row break added
    std::vector<size_t> m_tblColWidth;

    CGeomorphicBoard*   m_pGeoBoard;                    // Set if OK pressed

// Implementation - methods
protected:
    void LoadBoardListWithCompliantBoards();
    void UpdateButtons();

    // Generated message map functions
    //{{AFX_MSG(CCreateGeomorphicBoardDialog)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnSelChangeBoardList();
    afx_msg void OnBtnPressedAddBoard();
    afx_msg void OnBtnPressedAddBreak();
    afx_msg void OnBtnPressClear();
    afx_msg void OnChangeBoardName();
    afx_msg void OnDblClickBoardList();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnBtnPressedHelp();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif

