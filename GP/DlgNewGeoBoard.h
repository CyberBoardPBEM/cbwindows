// DlgNewGeoBoard.h : header file
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

#ifndef _DLGNEWGEOBOARD_H_
#define _DLGNEWGEOBOARD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCreateGeomorphicBoardDialog dialog

class CGamDoc;
class CGeomorphicBoard;

class CCreateGeomorphicBoardDialog : public wxDialog
{
// Construction
public:
    CCreateGeomorphicBoardDialog(CGamDoc& doc, wxWindow* pParent = &CB::GetMainWndWx());   // standard constructor
    // see https://docs.wxwidgets.org/latest/classwx_window_destroy_event.html
    ~CCreateGeomorphicBoardDialog();

// Operations
public:
    // Caller must delete the returned object when finished with it.
    OwnerPtr<CGeomorphicBoard> DetachGeomorphicBoard();

// Dialog Data
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxButton> m_btnOK;
        RefPtr<wxButton> m_btnClearList;
        RefPtr<wxButton> m_btnAddBreak;
        RefPtr<wxButton> m_btnAddBoard;
        RefPtr<wxListBox>    m_listGeo;
        RefPtr<wxTextCtrl>   m_editBoardName;
        RefPtr<wxListBox>    m_listBoard;
    CB_XRC_END_CTRLS_DECL()

    RefPtr<CGamDoc> m_pDoc;
public:


// Overrides

// Implementation - vars
protected:

    // clone *ItemDataPtr so ResetContent doesn't destroy root info
    std::unique_ptr<const CGeoBoardElement> m_pRootBoard;
    size_t      m_nCurrentRowHeight;                    // Height of current row maps
    size_t      m_nRowNumber;

    size_t      m_nCurrentColumn;
    size_t      m_nMaxColumns;                       // Set when first row break added
    std::vector<size_t> m_tblColWidth;

    OwnerOrNullPtr<CGeomorphicBoard> m_pGeoBoard;                    // Set if OK pressed

// Implementation - methods
protected:
    void LoadBoardListWithCompliantBoards();
    void UpdateButtons();

    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
    void OnSelChangeBoardList(wxCommandEvent& event);
    void OnBtnPressedAddBoard(wxCommandEvent& event);
    void OnBtnPressedAddBreak(wxCommandEvent& event);
    void OnBtnPressClear(wxCommandEvent& event);
    void OnChangeBoardName(wxCommandEvent& event);
    void OnDblClickBoardList(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnBtnPressedHelp();
#endif
    wxDECLARE_EVENT_TABLE();

    static void ResetContent(wxListBox& lb);
};

#endif

