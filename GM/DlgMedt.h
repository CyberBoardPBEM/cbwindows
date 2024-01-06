// dlgmedt.h : header file
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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

class CMarkerEditDialog : public wxDialog
{
// Construction
public:
    CMarkerEditDialog(wxWindow* parent = &CB::GetMainWndWx());    // standard constructor

// Dialog Data
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxCheckBox> m_chkPromptText;
        RefPtr<wxTextCtrl> m_editMarkerText;
        RefPtr<CMarkListBoxWx> m_listMarks;
        RefPtr<CTileListBoxWx> m_listTiles;
        RefPtr<wxChoice> m_comboTSet;
    CB_XRC_END_CTRLS_DECL()

public:
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

    bool TransferDataFromWindow() override;
    void OnSelchangeTSet(wxCommandEvent& event);
    void OnDblclkMarkers(wxCommandEvent& event);
    bool TransferDataToWindow() override;
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    wxDECLARE_EVENT_TABLE();
};
