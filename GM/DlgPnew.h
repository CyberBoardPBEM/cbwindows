// DlgPnew.h : header file
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
// CPieceNewDialog dialog

class CPieceNewDialog : public wxDialog
{
// Construction
public:
    CPieceNewDialog(CGamDoc& doc, size_t nPSet, wxWindow* parent = &CB::GetMainWndWx());  // standard constructor

// Dialog Data
private:
    size_t m_qty;
    CB_XRC_BEGIN_CTRLS_DECL()
        RefPtr<wxCheckBox> m_chkTopOnlyOwnersToo;
        RefPtr<wxCheckBox> m_chkTopOnlyVisible;
        RefPtr<wxTextCtrl> m_editTextFront;
        RefPtr<wxTextCtrl> m_editTextBack;
        RefPtr<wxCheckBox> m_chkSameAsTop;
        RefPtr<CTileListBoxWx> m_listFtile;
        RefPtr<CTileListBoxWx> m_listBtile;
        RefPtr<wxChoice> m_comboBtset;
        RefPtr<wxChoice> m_comboFtset;
        RefPtr<CPieceListBoxWx> m_listPieces;
        RefPtr<wxTextCtrl> m_editQty;
        /* sides are 0-based since they are vector indices, but we
            will display them 1-based for human readability */
        RefPtr<wxChoice> m_numSides;
        // m_currSide should contain 2 - sides
        RefPtr<wxChoice> m_currSide;
        RefPtr<wxStaticText> m_side_1;
        RefPtr<wxButton> m_create;
    CB_XRC_END_CTRLS_DECL()

    RefPtr<CGamDoc> m_pDoc;             // Pointer to document
    const size_t m_nPSet;            // Number of piece set

// Implementation
protected:
    const CTileManager& m_pTMgr;        // Set internally from m_pDoc
    RefPtr<CPieceManager> m_pPMgr;        // Set internally from m_pDoc
    size_t m_prevSide = std::numeric_limits<size_t>::max();
    std::vector<TileID> m_sideTids;
    std::vector<CB::string> m_sideTexts;

    // -------- //
    void RefreshPieceList();
    void SetupTileSetNames(wxChoice& pCombo) const;
    void SetupTileListbox(const wxChoice& pCombo, CTileListBoxWx& pList) const;
    void CreatePiece();
    TileID GetTileID(const wxChoice& pCombo, const CTileListBoxWx& pList) const;

// Implementation
protected:
    void OnSelchangeBtset(wxCommandEvent& event);
    void OnSelchangeFtset(wxCommandEvent& event);
    void OnCreate(wxCommandEvent& event);
    void OnCreate()
    {
        wxCommandEvent dummy;
        OnCreate(dummy);
    }
    void OnSelchangeNumSides(wxCommandEvent& event);
    void OnSelchangeNumSides()
    {
        wxCommandEvent dummy;
        OnSelchangeNumSides(dummy);
    }
    bool TransferDataToWindow() override;
    void OnDblClkFrontTile(wxCommandEvent& event);
    void OnCheckTextSameAsTop(wxCommandEvent& event);
    void OnCheckTextSameAsTop()
    {
        wxCommandEvent dummy;
        OnCheckTextSameAsTop(dummy);
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
