// DlgPedt.h : header file
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

#ifndef     _LBOXPIEC_H
#include    "LBoxPiec.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CPieceEditDialog dialog

class CPieceEditDialog : public wxDialog
{
// Construction
public:
    CPieceEditDialog(CGamDoc& doc, PieceID pid, wxWindow* parent = &CB::GetMainWndWx()); // standard constructor

// Dialog Data
private:
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxCheckBox> m_chkTopOnlyOwnersToo;
        RefPtr<wxCheckBox> m_chkTopOnlyVisible;
        RefPtr<wxTextCtrl> m_editTextBack;
        RefPtr<wxCheckBox> m_chkSameAsTop;
        RefPtr<wxTextCtrl> m_editTextFront;
        RefPtr<CPieceListBoxWx> m_listPieces;
        RefPtr<wxChoice> m_comboFtset;
        RefPtr<CTileListBoxWx> m_listFtile;
        RefPtr<wxChoice> m_comboBtset;
        RefPtr<CTileListBoxWx> m_listBtile;
        /* sides are 0-based since they are vector indices, but we
            will display them 1-based for human readability */
        RefPtr<wxChoice> m_numSides;
        // m_currSide should contain 2 - sides
        RefPtr<wxChoice> m_currSide;
        RefPtr<wxStaticText> m_side_1;
    CB_XRC_END_CTRLS_DECL()

    RefPtr<CGamDoc> m_pDoc;         // Caller must set this
    const PieceID m_pid;          // Piece to edit

// Implementation
protected:
    const CTileManager& m_pTMgr;        // Set internally from m_pDoc
    RefPtr<CPieceManager> m_pPMgr;        // Set internally from m_pDoc
    const std::vector<PieceID> m_tbl;     // Need to use list box.
    size_t m_prevSide = std::numeric_limits<size_t>::max();
    std::vector<TileID> m_sideTids;
    std::vector<CB::string> m_sideTexts;
    // -------- //
    void SetupPieceTiles();
    void SetupPieceTexts();
    void SetupTileSetNames(wxChoice& pCombo) const;
    void SetupTileListbox(const wxChoice& pCombo, CTileListBoxWx& pList) const;
    TileID GetTileID(const wxChoice& pCombo, const CTileListBoxWx& pList) const;
    // -------- //

    bool TransferDataFromWindow() override;
    bool TransferDataToWindow() override;
    void OnSelchangeNumSides(wxCommandEvent& event);
    void OnSelchangeNumSides()
    {
        wxCommandEvent dummy;
        OnSelchangeNumSides(dummy);
    }
    void OnSelchangeBtset(wxCommandEvent& event);
    void OnSelchangeFtset(wxCommandEvent& event);
    void OnDblclkPiece(wxCommandEvent& event);
    void OnCheckSameAsTop(wxCommandEvent& event);
    void OnCheckSameAsTop()
    {
        wxCommandEvent dummy;
        OnCheckSameAsTop(dummy);
    }
    void OnBtnClickTopVisible(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
#endif
    void OnSelchangeCurrSide(wxCommandEvent& event);
    void OnSelchangeCurrSide()
    {
        wxCommandEvent dummy;
        OnSelchangeCurrSide(dummy);
    }
    wxDECLARE_EVENT_TABLE();
};
