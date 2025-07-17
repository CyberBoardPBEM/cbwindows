// DlgSpece.h : header file
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

#ifndef     _LBOXPIEC_H
#include    "LBoxPiec.h"
#endif

#ifndef     _LBOXTRAY_H
#include    "LBoxTray.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetPiecesDialog dialog

class CSetPiecesDialog : public wxDialog
{
// Construction
public:
    CSetPiecesDialog(CGamDoc& doc, wxWindow* pParent = &CB::GetMainWndWx()); // standard constructor

// Dialog Data
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxChoice> m_comboPGrp;
        RefPtr<CPieceListBoxWx> m_listPiece;
        RefPtr<wxChoice> m_comboYGrp;
        RefPtr<CTrayListBoxWx> m_listTray;
    CB_XRC_END_CTRLS_DECL()

    RefPtr<CGamDoc> m_pDoc;         // Must be set by creator
public:
    int             m_nYSel;        // Initial tray number

// Implementation
protected:
    RefPtr<CPieceTable>  m_pPTbl;   // Loaded using doc pointer
    RefPtr<CTrayManager> m_pYMgr;   // Loaded using doc pointer

    std::vector<PieceID> m_tblPiece;
    std::vector<PieceID> m_tblTray;

    // -------- //
    void LoadPieceNameList();
    void LoadTrayNameList();
    void UpdatePieceList();
    void UpdateTrayList();

// Implementation
protected:
    bool TransferDataToWindow() override;

    void OnSelChangePieceGroup(wxCommandEvent& event);
    void OnRemoveAll(wxCommandEvent& event);
    void OnCopyAll(wxCommandEvent& event);
    void OnCopySelections(wxCommandEvent& event);
    void OnRemoveSelections(wxCommandEvent& event);
    void OnSelChangeTrayName(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();
};
