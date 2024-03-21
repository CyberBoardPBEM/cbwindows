// DlgPedt.cpp : implementation file
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

#include    "stdafx.h"
#include    "Gm.h"
#include    "GmDoc.h"
#include    "Pieces.h"
#include    "DlgPedt.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPieceEditDialog dialog


CPieceEditDialog::CPieceEditDialog(CGamDoc& doc, PieceID pid, wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CPieceEditDialog)
        CB_XRC_CTRL(m_chkTopOnlyOwnersToo)
        CB_XRC_CTRL(m_chkTopOnlyVisible)
        CB_XRC_CTRL(m_editTextBack)
        CB_XRC_CTRL(m_chkSameAsTop)
        CB_XRC_CTRL(m_editTextFront)
        CB_XRC_CTRL(m_listPieces)
        CB_XRC_CTRL(m_comboFtset)
        CB_XRC_CTRL(m_listFtile)
        CB_XRC_CTRL(m_comboBtset)
        CB_XRC_CTRL(m_listBtile)
        CB_XRC_CTRL(m_numSides)
        CB_XRC_CTRL(m_currSide)
        CB_XRC_CTRL(m_side_1)
    CB_XRC_END_CTRLS_DEFN(),
    m_pDoc(&doc),
    m_pid(pid),
    m_tbl(1, m_pid),
    m_pTMgr(*m_pDoc->GetTileManager()),
    m_pPMgr(m_pDoc->GetPieceManager())
{
}

wxBEGIN_EVENT_TABLE(CPieceEditDialog, wxDialog)
    EVT_CHOICE(XRCID("m_comboBtset"), OnSelchangeBtset)
    EVT_CHOICE(XRCID("m_comboFtset"), OnSelchangeFtset)
    EVT_LISTBOX_DCLICK(XRCID("m_listPieces"), OnDblclkPiece)
    EVT_CHECKBOX(XRCID("m_chkSameAsTop"), OnCheckSameAsTop)
    EVT_CHECKBOX(XRCID("m_chkTopOnlyVisible"), OnBtnClickTopVisible)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
    EVT_CHOICE(XRCID("m_numSides"), CPieceEditDialog::OnSelchangeNumSides)
    EVT_CHOICE(XRCID("m_currSide"), CPieceEditDialog::OnSelchangeCurrSide)
wxEND_EVENT_TABLE()

#if 0
/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_PEDIT_BTILE, IDH_D_PEDIT_BTILE,
    IDC_D_PEDIT_BTSET, IDH_D_PEDIT_BTSET,
    IDC_D_PEDIT_FTILE, IDH_D_PEDIT_FTILE,
    IDC_D_PEDIT_FTSET, IDH_D_PEDIT_FTSET,
    IDC_D_PEDIT_PIECE, IDH_D_PEDIT_PIECE,
    IDC_D_PEDIT_TEXT_BACK, IDH_D_PEDIT_TEXT_BACK,
    IDC_D_PEDIT_TEXT_FRONT, IDH_D_PEDIT_TEXT_FRONT,
    IDC_D_PEDIT_TEXT_SAMEASTOP, IDH_D_PEDIT_TEXT_SAMEASTOP,
    IDC_D_PEDIT_TOP_VISIBLE, IDH_D_PEDIT_TOP_VISIBLE,
    IDC_D_PEDIT_TOP_VISIBLE_OWNERS_TOO, IDH_D_PEDIT_TOP_VISIBLE_OWNERS_TOO,
    0,0
};

BOOL CPieceEditDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CPieceEditDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CPieceEditDialog::SetupTileListbox(const wxChoice& pCombo, CTileListBoxWx& pList) const
{
    int nCurSel = pCombo.GetSelection();
    if (nCurSel == wxNOT_FOUND)
    {
        pList.SetItemMap(NULL);
        return;
    }

    const CTileSet& pTSet = m_pTMgr.GetTileSet(value_preserving_cast<size_t>(nCurSel));
    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    pList.SetItemMap(&pLstMap);
}

void CPieceEditDialog::SetupTileSetNames(wxChoice& pCombo) const
{
    pCombo.Clear();

    for (size_t i = size_t(0); i < m_pTMgr.GetNumTileSets(); i++)
        pCombo.Append(m_pTMgr.GetTileSet(i).GetName());
    if (!m_pTMgr.IsEmpty())
        pCombo.SetSelection(wxNOT_FOUND);          // Select no entry yet
}

TileID CPieceEditDialog::GetTileID(const wxChoice& pCombo, const CTileListBoxWx& pList) const
{
    int nCurSel = pCombo.GetSelection();
    if (nCurSel == wxNOT_FOUND)
        return nullTid;

    int nCurTile = pList.GetSelection();
    if (nCurTile == wxNOT_FOUND)
        return nullTid;

    const CTileSet& pTSet = m_pTMgr.GetTileSet(value_preserving_cast<size_t>(nCurSel));

    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    return pLstMap.at(value_preserving_cast<size_t>(nCurTile));
}

/////////////////////////////////////////////////////////////////////////////

void CPieceEditDialog::SetupPieceTiles()
{
    PieceDef& pDef = m_pPMgr->GetPiece(m_pid);

    m_sideTids = pDef.GetTIDs();
    wxASSERT(!m_sideTids.empty());

    size_t nSet = m_pTMgr.FindTileSetFromTileID(pDef.GetFrontTID());
    wxASSERT(nSet != Invalid_v<size_t>);

    m_comboFtset->SetSelection(value_preserving_cast<int>(nSet));
    SetupTileListbox(*m_comboFtset, *m_listFtile);
    m_listFtile->SetCurSelMapped(pDef.GetFrontTID());

    if (pDef.GetSides() >= size_t(2))
    {
        int currSide = m_currSide->GetSelection() + 1;
        ASSERT(1 <= currSide && currSide < value_preserving_cast<int>(pDef.GetSides()));
        TileID tid = pDef.GetTIDs()[value_preserving_cast<size_t>(currSide)];
        nSet = m_pTMgr.FindTileSetFromTileID(tid);
        wxASSERT(nSet != Invalid_v<size_t>);
        m_comboBtset->SetSelection(value_preserving_cast<int>(nSet));
        SetupTileListbox(*m_comboBtset, *m_listBtile);
        m_listBtile->SetCurSelMapped(tid);
    }
    else
    {
        m_comboBtset->SetSelection(wxNOT_FOUND);
        SetupTileListbox(*m_comboBtset, *m_listBtile);
    }
}

void CPieceEditDialog::SetupPieceTexts()
{
    PieceDef& pDef = m_pPMgr->GetPiece(m_pid);

    m_sideTexts.clear();
    m_sideTexts.reserve(pDef.GetSides());
    for (size_t i = size_t(0); i < pDef.GetSides(); ++i)
    {
        CB::string strText;
        m_pDoc->GetGameStringMap().Lookup(MakePieceElement(m_pid, value_preserving_cast<unsigned>(i)), strText);
        m_sideTexts.push_back(std::move(strText));
    }

    CB::string strText;
    m_pDoc->GetGameStringMap().Lookup(MakePieceElement(m_pid, unsigned(0)), strText);
    if (!strText.empty())
        m_editTextFront->SetValue(strText);
    m_chkSameAsTop->SetValue(false);

    if (pDef.GetSides() >= size_t(2))
    {
        bool match = true;
        for (size_t i = size_t(1) ; i < pDef.GetSides() ; ++i)
        {
            if (m_sideTexts[i] != m_sideTexts[size_t(0)])
            {
                match = false;
                break;
            }
        }
        if (match)
        {
            m_chkSameAsTop->SetValue(true);
        }
    }

    if (pDef.GetSides() >= size_t(2))
    {
        size_t currSide = value_preserving_cast<size_t>(m_currSide->GetSelection()) + size_t(1);
        wxASSERT(size_t(1) <= currSide && currSide < pDef.GetSides());
        m_editTextBack->SetValue(m_sideTexts[currSide]);
    }
    else
    {
        m_editTextBack->Clear();
    }

    OnCheckSameAsTop();
}

/////////////////////////////////////////////////////////////////////////////
// CPieceEditDialog message handlers

void CPieceEditDialog::OnBtnClickTopVisible(wxCommandEvent& /*event*/)
{
    m_chkTopOnlyOwnersToo->Enable(m_chkTopOnlyVisible->GetValue());
}

void CPieceEditDialog::OnSelchangeNumSides(wxCommandEvent& /*event*/)
{
    size_t numSides = value_preserving_cast<size_t>(m_numSides->GetSelection()) + size_t(1);
    wxASSERT(size_t(1) <= numSides &&
            numSides <= PieceDef::maxSides);

    if (numSides >= size_t(2))
    {
        m_currSide->Enable(TRUE);
        m_comboBtset->Enable(TRUE);
        m_listBtile->Enable(TRUE);
        m_chkSameAsTop->Enable(TRUE);
        m_chkTopOnlyVisible->Enable(TRUE);
        m_chkTopOnlyOwnersToo->Enable(m_chkTopOnlyVisible->GetValue());
        OnCheckSameAsTop();
    }
    else
    {
        m_currSide->Enable(FALSE);
        m_comboBtset->Enable(FALSE);
        m_listBtile->Enable(FALSE);
        m_chkSameAsTop->Enable(FALSE);
        m_editTextBack->Enable(FALSE);
        m_chkTopOnlyVisible->Enable(FALSE);
        m_chkTopOnlyOwnersToo->Enable(FALSE);
        m_editTextBack->Clear();
    }

    // fix up m_currSides, m_sideTids, and m_sideTexts content
    size_t currSides = value_preserving_cast<size_t>(m_currSide->GetCount() + 1);
    wxASSERT(m_sideTids.size() == currSides &&
            m_sideTexts.size() == currSides);
    if (currSides < numSides)
    {
        CB::string frontStr;
        BOOL bSameAsTop = m_chkSameAsTop->GetValue();
        if (bSameAsTop)
        {
            frontStr = m_sideTexts[size_t(0)];
        }
        for (size_t i = currSides ; i < numSides ; ++i)
        {
            CB::string str = AfxFormatString1(IDS_SIDE_N, std::format("{}", i + size_t(1)));
            if (m_currSide->Append(str) != value_preserving_cast<int>(i - size_t(1)))
            {
                AfxThrowMemoryException();
            }
            m_sideTids.push_back(m_sideTids[size_t(0)]);
            m_sideTexts.push_back(frontStr);
        }
    }
    else if (currSides > numSides)
    {
        for (size_t i = currSides ; i > numSides ; --i)
        {
            unsigned i2 = value_preserving_cast<unsigned>(i - size_t(2));
            if (m_currSide->Delete(i2), m_currSide->GetCount() != i2)
            {
                AfxThrowMemoryException();
            }
        }
        m_sideTids.resize(numSides);
        m_sideTexts.resize(numSides);
    }

    wxASSERT(m_sideTids.size() == numSides &&
            m_sideTexts.size() == numSides);
    if (m_currSide->IsEmpty())
    {
        m_currSide->Enable(false);
        OnSelchangeCurrSide();
    }
    else if (m_currSide->GetSelection() == wxNOT_FOUND)
    {
        m_currSide->SetSelection(0);
        OnSelchangeCurrSide();
    }
}

bool CPieceEditDialog::TransferDataFromWindow()
{
    PieceDef& pDef = m_pPMgr->GetPiece(m_pid);
    TileID tidFront = GetTileID(*m_comboFtset, *m_listFtile);
    // read current non-front tileID and text
    OnSelchangeCurrSide();
    if (tidFront == nullTid)
        return false;

    size_t numSides = value_preserving_cast<size_t>(m_numSides->GetSelection() + 1);
    wxASSERT(numSides <= PieceDef::maxSides);
    wxASSERT(m_sideTids.size() == numSides &&
            m_sideTexts.size() == numSides);

    pDef.SetTIDs(std::move(m_sideTids));

    CB::string strText = m_editTextFront->GetValue();
    m_sideTexts[size_t(0)] = strText;

    for (size_t i = size_t(0) ; i < m_sideTexts.size() ; ++i)
    {
        GameElement ge = MakePieceElement(m_pid, value_preserving_cast<unsigned>(i));
        CB::string strText;
        if (!m_chkSameAsTop->GetValue())
        {
            strText = m_sideTexts[i];
        }
        else
        {
            strText = m_sideTexts[size_t(0)];
        }
        if (!strText.empty())
            m_pDoc->GetGameStringMap().SetAt(ge, strText);
        else
            m_pDoc->GetGameStringMap().RemoveKey(ge);
    }

    pDef.m_flags &= ~PieceDef::flagShowOnlyVisibleSide;      // Initially clear the flags
    pDef.m_flags &= ~PieceDef::flagShowOnlyOwnersToo;
    if (numSides >= size_t(2) && m_chkTopOnlyVisible->GetValue())
    {
        pDef.m_flags |= PieceDef::flagShowOnlyVisibleSide;     // Set the flag
        if (m_chkTopOnlyOwnersToo->GetValue())
            pDef.m_flags |= PieceDef::flagShowOnlyOwnersToo;   // Set the other flag
    }

    return wxDialog::TransferDataFromWindow();
}

bool CPieceEditDialog::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow())
    {
        return false;
    }

    m_listPieces->SetDocument(*m_pDoc);

    for (size_t i = size_t(0) ; i < PieceDef::maxSides ; ++i)
    {
        int rc = m_numSides->Append(CB::string(std::format("{}", i + size_t(1))));
        if (rc != value_preserving_cast<int>(i))
        {
            AfxThrowMemoryException();
        }
    }
    CB::string str = AfxFormatString1(IDS_SIDE_N, std::format("{}", 1));
    m_side_1->SetLabel(str);

    m_listPieces->SetItemMap(&m_tbl);

    m_listFtile->SetDocument(&*m_pDoc);
    m_listBtile->SetDocument(&*m_pDoc);

    SetupTileSetNames(*m_comboFtset);
    SetupTileSetNames(*m_comboBtset);

    PieceDef& pPce = m_pPMgr->GetPiece(m_pid);
    ASSERT(pPce.GetSides() > size_t(0));

    m_numSides->SetSelection(value_preserving_cast<int>(pPce.GetSides() - size_t(1)));
    // satisfy OnSelchangeNumSides() preconditions
    m_sideTids.clear();
    m_sideTids.push_back(pPce.GetFrontTID());
    CB::string strText;
    m_pDoc->GetGameStringMap().Lookup(MakePieceElement(m_pid, unsigned(0)), strText);
    m_sideTexts.clear();
    m_sideTexts.push_back(std::move(strText));
    OnSelchangeNumSides();

    SetupPieceTiles();

    SetupPieceTexts();

    m_chkTopOnlyVisible->SetValue(false);
    m_chkTopOnlyOwnersToo->SetValue(false);
    if (pPce.GetSides() >= size_t(2))
    {
        m_chkTopOnlyVisible->SetValue(
            bool(pPce.m_flags & PieceDef::flagShowOnlyVisibleSide));
        m_chkTopOnlyOwnersToo->SetValue(
            bool(pPce.m_flags & PieceDef::flagShowOnlyOwnersToo));
    }
    OnSelchangeNumSides();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPieceEditDialog::OnSelchangeBtset(wxCommandEvent& /*event*/)
{
    SetupTileListbox(*m_comboBtset, *m_listBtile);
}

void CPieceEditDialog::OnSelchangeFtset(wxCommandEvent& /*event*/)
{
    SetupTileListbox(*m_comboFtset, *m_listFtile);
}

void CPieceEditDialog::OnDblclkPiece(wxCommandEvent& /*event*/)
{
    SetupPieceTiles();
    SetupPieceTexts();
}

void CPieceEditDialog::OnCheckSameAsTop(wxCommandEvent& /*event*/)
{
    BOOL bSameAsTop = m_chkSameAsTop->GetValue();
    if (bSameAsTop)
    {
        m_editTextBack->Clear();
        m_editTextBack->Enable(FALSE);
    }
    else
        m_editTextBack->Enable(m_numSides->GetSelection() + 1 >= 2);
}

void CPieceEditDialog::OnSelchangeCurrSide(wxCommandEvent& /*event*/)
{
    wxASSERT(m_sideTids.size() == value_preserving_cast<size_t>(m_numSides->GetSelection() + 1));
    wxASSERT(m_sideTexts.size() == m_sideTids.size());
    if (m_prevSide < m_sideTids.size())
    {
        TileID tidBack = GetTileID(*m_comboBtset, *m_listBtile);
        m_sideTids[m_prevSide] = tidBack;
        if (!m_chkSameAsTop->GetValue())
        {
            CB::string strText = m_editTextBack->GetValue();
            m_sideTexts[m_prevSide] = strText;
        }
        else
        {
            m_sideTexts[m_prevSide] = m_sideTexts[size_t(0)];
        }
    }

    size_t currSide = std::numeric_limits<size_t>::max();

    if (m_currSide->GetSelection() != wxNOT_FOUND &&
        value_preserving_cast<size_t>(m_currSide->GetSelection()) + size_t(1) >= size_t(1))
    {
        currSide = value_preserving_cast<size_t>(m_currSide->GetSelection()) + size_t(1);
        wxASSERT(size_t(1) <= currSide &&
                currSide < value_preserving_cast<size_t>(m_numSides->GetSelection()) + size_t(1));
        TileID tid = m_sideTids[currSide];
        if (tid != nullTid)
        {
            size_t nSet = m_pTMgr.FindTileSetFromTileID(tid);
            ASSERT(nSet != Invalid_v<size_t>);
            m_comboBtset->SetSelection(value_preserving_cast<int>(nSet));
            SetupTileListbox(*m_comboBtset, *m_listBtile);
            m_listBtile->SetCurSelMapped(tid);
        }
        m_editTextBack->SetValue(m_sideTexts[currSide]);
    }
    else
    {
        m_comboBtset->SetSelection(wxNOT_FOUND);
        SetupTileListbox(*m_comboBtset, *m_listBtile);
    }

    m_prevSide = currSide;
}

