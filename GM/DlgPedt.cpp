// DlgPedt.cpp : implementation file
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


CPieceEditDialog::CPieceEditDialog(CGamDoc& doc, PieceID pid, CWnd* pParent /*=NULL*/)
    : CDialog(CPieceEditDialog::IDD, pParent),
    m_pDoc(&doc),
    m_pid(pid),
    m_tbl(1, m_pid),
    m_pTMgr(*m_pDoc->GetTileManager()),
    m_pPMgr(m_pDoc->GetPieceManager())
{
    //{{AFX_DATA_INIT(CPieceEditDialog)
    //}}AFX_DATA_INIT
}

void CPieceEditDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPieceEditDialog)
    DDX_Control(pDX, IDC_D_PEDIT_TOP_VISIBLE_OWNERS_TOO, m_chkTopOnlyOwnersToo);
    DDX_Control(pDX, IDC_D_PEDIT_TOP_VISIBLE, m_chkTopOnlyVisible);
    DDX_Control(pDX, IDC_D_PEDIT_TEXT_BACK, m_editTextBack);
    DDX_Control(pDX, IDC_D_PEDIT_TEXT_SAMEASTOP, m_chkSameAsTop);
    DDX_Control(pDX, IDC_D_PEDIT_TEXT_FRONT, m_editTextFront);
    DDX_Control(pDX, IDC_D_PEDIT_PIECE, m_listPieces);
    DDX_Control(pDX, IDC_D_PEDIT_FTSET, m_comboFtset);
    DDX_Control(pDX, IDC_D_PEDIT_FTILE, m_listFtile);
    DDX_Control(pDX, IDC_D_PEDIT_BTSET, m_comboBtset);
    DDX_Control(pDX, IDC_D_PEDIT_BTILE, m_listBtile);
    DDX_Control(pDX, IDC_NUM_SIDES, m_numSides);
    DDX_Control(pDX, IDC_CURR_SIDE, m_currSide);
    DDX_Control(pDX, IDC_STATIC_SIDE_1, m_side_1);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPieceEditDialog, CDialog)
    //{{AFX_MSG_MAP(CPieceEditDialog)
    ON_CBN_SELCHANGE(IDC_D_PEDIT_BTSET, OnSelchangeBtset)
    ON_CBN_SELCHANGE(IDC_D_PEDIT_FTSET, OnSelchangeFtset)
    ON_LBN_DBLCLK(IDC_D_PEDIT_PIECE, OnDblclkPiece)
    ON_BN_CLICKED(IDC_D_PEDIT_TEXT_SAMEASTOP, OnCheckSameAsTop)
    ON_BN_CLICKED(IDC_D_PEDIT_TOP_VISIBLE, OnBtnClickTopVisible)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_CBN_SELCHANGE(IDC_NUM_SIDES, &CPieceEditDialog::OnSelchangeNumSides)
    ON_CBN_SELCHANGE(IDC_CURR_SIDE, &CPieceEditDialog::OnSelchangeCurrSide)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

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

/////////////////////////////////////////////////////////////////////////////

void CPieceEditDialog::SetupTileListbox(const CComboBox& pCombo, CTileListBox& pList) const
{
    int nCurSel = pCombo.GetCurSel();
    if (nCurSel < 0)
    {
        pList.SetItemMap(NULL);
        return;
    }

    const CTileSet& pTSet = m_pTMgr.GetTileSet(value_preserving_cast<size_t>(nCurSel));
    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    pList.SetItemMap(&pLstMap);
}

void CPieceEditDialog::SetupTileSetNames(CComboBox& pCombo) const
{
    pCombo.ResetContent();

    for (size_t i = size_t(0); i < m_pTMgr.GetNumTileSets(); i++)
        pCombo.AddString(m_pTMgr.GetTileSet(i).GetName());
    if (!m_pTMgr.IsEmpty())
        pCombo.SetCurSel(-1);          // Select no entry yet
}

TileID CPieceEditDialog::GetTileID(const CComboBox& pCombo, const CTileListBox& pList) const
{
    int nCurSel = pCombo.GetCurSel();
    if (nCurSel < 0)
        return nullTid;

    int nCurTile = pList.GetCurSel();
    if (nCurTile < 0)
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
    ASSERT(!m_sideTids.empty());

    size_t nSet = m_pTMgr.FindTileSetFromTileID(pDef.GetFrontTID());
    ASSERT(nSet != Invalid_v<size_t>);

    m_comboFtset.SetCurSel(value_preserving_cast<int>(nSet));
    SetupTileListbox(m_comboFtset, m_listFtile);
    m_listFtile.SetCurSelMapped(pDef.GetFrontTID());

    if (pDef.GetSides() >= size_t(2))
    {
        int currSide = m_currSide.GetCurSel() + 1;
        ASSERT(1 <= currSide && currSide < value_preserving_cast<int>(pDef.GetSides()));
        TileID tid = pDef.GetTIDs()[value_preserving_cast<size_t>(currSide)];
        nSet = m_pTMgr.FindTileSetFromTileID(tid);
        ASSERT(nSet != Invalid_v<size_t>);
        m_comboBtset.SetCurSel(value_preserving_cast<int>(nSet));
        SetupTileListbox(m_comboBtset, m_listBtile);
        m_listBtile.SetCurSelMapped(tid);
    }
    else
    {
        m_comboBtset.SetCurSel(-1);
        SetupTileListbox(m_comboBtset, m_listBtile);
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
        m_editTextFront.SetWindowText(strText);
    m_chkSameAsTop.SetCheck(0);

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
            m_chkSameAsTop.SetCheck(1);
        }
    }

    if (pDef.GetSides() >= size_t(2))
    {
        size_t currSide = value_preserving_cast<size_t>(m_currSide.GetCurSel()) + size_t(1);
        ASSERT(size_t(1) <= currSide && currSide < pDef.GetSides());
        m_editTextBack.SetWindowText(m_sideTexts[currSide]);
    }
    else
    {
        m_editTextBack.SetWindowText(""_cbstring);
    }

    OnCheckSameAsTop();
}

/////////////////////////////////////////////////////////////////////////////
// CPieceEditDialog message handlers

void CPieceEditDialog::OnBtnClickTopVisible()
{
    m_chkTopOnlyOwnersToo.EnableWindow(m_chkTopOnlyVisible.GetCheck() != 0);
}

void CPieceEditDialog::OnSelchangeNumSides()
{
    size_t numSides = value_preserving_cast<size_t>(m_numSides.GetCurSel()) + size_t(1);
    ASSERT(size_t(1) <= numSides &&
            numSides <= PieceDef::maxSides);

    if (numSides >= size_t(2))
    {
        m_currSide.EnableWindow(TRUE);
        m_comboBtset.EnableWindow(TRUE);
        m_listBtile.EnableWindow(TRUE);
        m_chkSameAsTop.EnableWindow(TRUE);
        m_chkTopOnlyVisible.EnableWindow(TRUE);
        m_chkTopOnlyOwnersToo.EnableWindow(m_chkTopOnlyVisible.GetCheck() != 0);
        OnCheckSameAsTop();
    }
    else
    {
        m_currSide.EnableWindow(FALSE);
        m_comboBtset.EnableWindow(FALSE);
        m_listBtile.EnableWindow(FALSE);
        m_chkSameAsTop.EnableWindow(FALSE);
        m_editTextBack.EnableWindow(FALSE);
        m_chkTopOnlyVisible.EnableWindow(FALSE);
        m_chkTopOnlyOwnersToo.EnableWindow(FALSE);
        m_editTextBack.SetWindowText(""_cbstring);
    }

    // fix up m_currSides, m_sideTids, and m_sideTexts content
    size_t currSides = value_preserving_cast<size_t>(m_currSide.GetCount() + 1);
    ASSERT(m_sideTids.size() == currSides &&
            m_sideTexts.size() == currSides);
    if (currSides < numSides)
    {
        CB::string frontStr;
        BOOL bSameAsTop = m_chkSameAsTop.GetCheck() != 0;
        if (bSameAsTop)
        {
            frontStr = m_sideTexts[size_t(0)];
        }
        for (size_t i = currSides ; i < numSides ; ++i)
        {
            CB::string str = AfxFormatString1(IDS_SIDE_N, std::format("{}", i + size_t(1)));
            if (m_currSide.AddString(str) != value_preserving_cast<int>(i - size_t(1)))
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
            if (m_currSide.DeleteString(i2) != value_preserving_cast<int>(i2))
            {
                AfxThrowMemoryException();
            }
        }
        m_sideTids.resize(numSides);
        m_sideTexts.resize(numSides);
    }

    ASSERT(m_sideTids.size() == numSides &&
            m_sideTexts.size() == numSides);
    if (m_currSide.GetCount() == 0)
    {
        m_currSide.EnableWindow(false);
        OnSelchangeCurrSide();
    }
    else if (m_currSide.GetCurSel() == CB_ERR)
    {
        m_currSide.SetCurSel(0);
        OnSelchangeCurrSide();
    }
}

void CPieceEditDialog::OnOK()
{
    PieceDef& pDef = m_pPMgr->GetPiece(m_pid);
    TileID tidFront = GetTileID(m_comboFtset, m_listFtile);
    // read current non-front tileID and text
    OnSelchangeCurrSide();
    if (tidFront == nullTid)
        return;

    size_t numSides = value_preserving_cast<size_t>(m_numSides.GetCurSel() + 1);
    ASSERT(numSides <= PieceDef::maxSides);
    ASSERT(m_sideTids.size() == numSides &&
            m_sideTexts.size() == numSides);

    pDef.SetTIDs(std::move(m_sideTids));

    CB::string strText = CB::string::GetWindowText(m_editTextFront);
    m_sideTexts[size_t(0)] = strText;

    for (size_t i = size_t(0) ; i < m_sideTexts.size() ; ++i)
    {
        GameElement ge = MakePieceElement(m_pid, value_preserving_cast<unsigned>(i));
        CB::string strText;
        if (m_chkSameAsTop.GetCheck() == 0)
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
    if (numSides >= size_t(2) && m_chkTopOnlyVisible.GetCheck() != 0)
    {
        pDef.m_flags |= PieceDef::flagShowOnlyVisibleSide;     // Set the flag
        if (m_chkTopOnlyOwnersToo.GetCheck() != 0)
            pDef.m_flags |= PieceDef::flagShowOnlyOwnersToo;   // Set the other flag
    }

    CDialog::OnOK();
}

BOOL CPieceEditDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_listPieces.SetDocument(*m_pDoc);

    for (size_t i = size_t(0) ; i < PieceDef::maxSides ; ++i)
    {
        int rc = m_numSides.AddString(CB::string(std::format("{}", i + size_t(1))));
        if (rc != value_preserving_cast<int>(i))
        {
            AfxThrowMemoryException();
        }
    }
    CB::string str = AfxFormatString1(IDS_SIDE_N, std::format("{}", 1));
    m_side_1.SetWindowText(str);

    m_listPieces.SetItemMap(&m_tbl);

    m_listFtile.SetDocument(&*m_pDoc);
    m_listBtile.SetDocument(&*m_pDoc);

    SetupTileSetNames(m_comboFtset);
    SetupTileSetNames(m_comboBtset);

    PieceDef& pPce = m_pPMgr->GetPiece(m_pid);
    ASSERT(pPce.GetSides() > size_t(0));

    m_numSides.SetCurSel(value_preserving_cast<int>(pPce.GetSides() - size_t(1)));
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

    m_chkTopOnlyVisible.SetCheck(0);
    m_chkTopOnlyOwnersToo.SetCheck(0);
    if (pPce.GetSides() >= size_t(2))
    {
        m_chkTopOnlyVisible.SetCheck(
            (pPce.m_flags & PieceDef::flagShowOnlyVisibleSide) ? 1 : 0);
        m_chkTopOnlyOwnersToo.SetCheck(
            (pPce.m_flags & PieceDef::flagShowOnlyOwnersToo) ? 1 : 0);
    }
    OnSelchangeNumSides();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPieceEditDialog::OnSelchangeBtset()
{
    SetupTileListbox(m_comboBtset, m_listBtile);
}

void CPieceEditDialog::OnSelchangeFtset()
{
    SetupTileListbox(m_comboFtset, m_listFtile);
}

void CPieceEditDialog::OnDblclkPiece()
{
    SetupPieceTiles();
    SetupPieceTexts();
}

void CPieceEditDialog::OnCheckSameAsTop()
{
    BOOL bSameAsTop = m_chkSameAsTop.GetCheck() != 0;
    if (bSameAsTop)
    {
        m_editTextBack.SetWindowText(""_cbstring);
        m_editTextBack.EnableWindow(FALSE);
    }
    else
        m_editTextBack.EnableWindow(m_numSides.GetCurSel() + 1 >= 2);
}

void CPieceEditDialog::OnSelchangeCurrSide()
{
    ASSERT(m_sideTids.size() == value_preserving_cast<size_t>(m_numSides.GetCurSel() + 1));
    ASSERT(m_sideTexts.size() == m_sideTids.size());
    if (m_prevSide < m_sideTids.size())
    {
        TileID tidBack = GetTileID(m_comboBtset, m_listBtile);
        m_sideTids[m_prevSide] = tidBack;
        if (m_chkSameAsTop.GetCheck() == 0)
        {
            CB::string strText = CB::string::GetWindowText(m_editTextBack);
            m_sideTexts[m_prevSide] = strText;
        }
        else
        {
            m_sideTexts[m_prevSide] = m_sideTexts[size_t(0)];
        }
    }

    size_t currSide = std::numeric_limits<size_t>::max();

    if (m_currSide.GetCurSel() != CB_ERR &&
        value_preserving_cast<size_t>(m_currSide.GetCurSel()) + size_t(1) >= size_t(1))
    {
        currSide = value_preserving_cast<size_t>(m_currSide.GetCurSel()) + size_t(1);
        ASSERT(size_t(1) <= currSide &&
                currSide < value_preserving_cast<size_t>(m_numSides.GetCurSel()) + size_t(1));
        TileID tid = m_sideTids[currSide];
        if (tid != nullTid)
        {
            size_t nSet = m_pTMgr.FindTileSetFromTileID(tid);
            ASSERT(nSet != Invalid_v<size_t>);
            m_comboBtset.SetCurSel(value_preserving_cast<int>(nSet));
            SetupTileListbox(m_comboBtset, m_listBtile);
            m_listBtile.SetCurSelMapped(tid);
        }
        m_editTextBack.SetWindowText(m_sideTexts[currSide]);
    }
    else
    {
        m_comboBtset.SetCurSel(-1);
        SetupTileListbox(m_comboBtset, m_listBtile);
    }

    m_prevSide = currSide;
}

