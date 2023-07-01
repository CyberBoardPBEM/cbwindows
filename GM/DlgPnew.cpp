// DlgPnew.cpp : implementation file
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
#include    "DlgPnew.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPieceNewDialog dialog


CPieceNewDialog::CPieceNewDialog(CGamDoc& doc, size_t nPSet, CWnd* pParent /*=NULL*/)
    : CDialog(CPieceNewDialog::IDD, pParent),
    m_pDoc(&doc),
    m_nPSet(nPSet),
    m_pTMgr(*m_pDoc->GetTileManager()),
    m_pPMgr(m_pDoc->GetPieceManager())
{
    //{{AFX_DATA_INIT(CPieceNewDialog)
    //}}AFX_DATA_INIT
}

void CPieceNewDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPieceNewDialog)
    DDX_Control(pDX, IDC_D_PCREATE_TOP_VISIBLE_OWNERS_TOO, m_chkTopOnlyOwnersToo);
    DDX_Control(pDX, IDC_D_PCREATE_TOP_VISIBLE, m_chkTopOnlyVisible);
    DDX_Control(pDX, IDC_D_PCREATE_TEXT_FRONT, m_editTextFront);
    DDX_Control(pDX, IDC_D_PCREATE_TEXT_BACK, m_editTextBack);
    DDX_Control(pDX, IDC_D_PCREATE_TEXT_SAMEASTOP, m_chkSameAsTop);
    DDX_Control(pDX, IDC_NUM_SIDES, m_numSides);
    DDX_Control(pDX, IDC_CURR_SIDE, m_currSide);
    DDX_Control(pDX, IDC_STATIC_SIDE_1, m_side_1);
    DDX_Control(pDX, IDC_D_PCREATE_FTILE, m_listFtile);
    DDX_Control(pDX, IDC_D_PCREATE_BTILE, m_listBtile);
    DDX_Control(pDX, IDC_D_PCREATE_BTSET, m_comboBtset);
    DDX_Control(pDX, IDC_D_PCREATE_FTSET, m_comboFtset);
    DDX_Control(pDX, IDC_D_PCREATE_PIECES, m_listPieces);
    DDX_Control(pDX, IDC_D_PIECE_QTY, m_editQty);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPieceNewDialog, CDialog)
    //{{AFX_MSG_MAP(CPieceNewDialog)
    ON_CBN_SELCHANGE(IDC_D_PCREATE_BTSET, OnSelchangeBtset)
    ON_CBN_SELCHANGE(IDC_D_PCREATE_FTSET, OnSelchangeFtset)
    ON_BN_CLICKED(IDC_D_PIECE_CREATE, OnCreate)
    ON_LBN_DBLCLK(IDC_D_PCREATE_FTILE, OnDblClkFrontTile)
    ON_BN_CLICKED(IDC_D_PCREATE_TEXT_SAMEASTOP, OnCheckTextSameAsTop)
    ON_BN_CLICKED(IDC_D_PCREATE_TOP_VISIBLE, OnBtnClickTopVisible)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_CBN_SELCHANGE(IDC_NUM_SIDES, &CPieceNewDialog::OnSelchangeNumSides)
    ON_CBN_SELCHANGE(IDC_CURR_SIDE, &CPieceNewDialog::OnSelchangeCurrSide)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_PCREATE_BTILE, IDH_D_PCREATE_BTILE,
    IDC_D_PCREATE_BTSET, IDH_D_PCREATE_BTSET,
    IDC_D_PCREATE_FTILE, IDH_D_PCREATE_FTILE,
    IDC_D_PCREATE_FTSET, IDH_D_PCREATE_FTSET,
    IDC_D_PCREATE_PIECES, IDH_D_PCREATE_PIECES,
    IDC_D_PCREATE_TEXT_BACK, IDH_D_PCREATE_TEXT_BACK,
    IDC_D_PCREATE_TEXT_FRONT, IDH_D_PCREATE_TEXT_FRONT,
    IDC_D_PCREATE_TEXT_SAMEASTOP, IDH_D_PCREATE_TEXT_SAMEASTOP,
    IDC_D_PCREATE_TOP_VISIBLE, IDH_D_PCREATE_TOP_VISIBLE,
    IDC_D_PCREATE_TOP_VISIBLE_OWNERS_TOO, IDH_D_PCREATE_TOP_VISIBLE_OWNERS_TOO,
    IDC_D_PIECE_CREATE, IDH_D_PIECE_CREATE,
    IDC_D_PIECE_QTY, IDH_D_PIECE_QTY,
    0,0
};

BOOL CPieceNewDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CPieceNewDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////

void CPieceNewDialog::SetupTileListbox(const CComboBox& pCombo, CTileListBox& pList) const
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

void CPieceNewDialog::SetupTileSetNames(CComboBox& pCombo) const
{
    pCombo.ResetContent();

    for (size_t i = size_t(0); i < m_pTMgr.GetNumTileSets(); i++)
        pCombo.AddString(m_pTMgr.GetTileSet(i).GetName());
    if (!m_pTMgr.IsEmpty())
        pCombo.SetCurSel(0);           // Select the first entry
}

void CPieceNewDialog::CreatePiece()
{
    m_sideTids[size_t(0)] = GetTileID(m_comboFtset, m_listFtile);
    // read current non-front tileID and text
    OnSelchangeCurrSide();

    if (std::find(m_sideTids.begin(), m_sideTids.end(), nullTid) != m_sideTids.end())
        return;

    CB::string str = CB::string::GetWindowText(m_editQty);
    int nNumPieces = std::stoi(str.std_wstr());
    if (nNumPieces <= 0)
        return;
    // due to LB_ITEMFROMPOINT limit
    CPieceSet& pPSet = m_pPMgr->GetPieceSet(m_nPSet);
    // value_preserving_cast unnecessary due to <= 0 check above
    if (static_cast<size_t>(nNumPieces) > size_t(0xFFFF) - pPSet.GetPieceIDTable().size())
    {
        AfxMessageBox(IDS_ERR_PIECESETSIZE, MB_OK | MB_ICONEXCLAMATION);
        m_editQty.SetFocus();
        return;
    }

    for (int i = 0; i < nNumPieces; i++)
    {
        PieceID pid = m_pPMgr->CreatePiece(m_nPSet, std::vector<TileID>(m_sideTids));
        CB::string strText = CB::string::GetWindowText(m_editTextFront);
        m_sideTexts[size_t(0)] = strText;

        for (size_t i = size_t(0) ; i < m_sideTexts.size() ; ++i)
        {
            GameElement ge = MakePieceElement(pid, value_preserving_cast<unsigned>(i));
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
        PieceDef& pPce = m_pPMgr->GetPiece(pid);
        // Initially clear this
        pPce.m_flags &= ~PieceDef::flagShowOnlyOwnersToo;
        if (m_chkTopOnlyVisible.GetCheck() != 0)
        {
            pPce.m_flags |= PieceDef::flagShowOnlyVisibleSide;
            // only makes sense if show only is activated
            if (m_chkTopOnlyOwnersToo.GetCheck() != 0)
                pPce.m_flags |= PieceDef::flagShowOnlyOwnersToo;
        }
        else
            pPce.m_flags &= ~PieceDef::flagShowOnlyVisibleSide;
    }

    RefreshPieceList();

    if (m_listPieces.GetCount() > 0)
        m_listPieces.SetTopIndex(m_listPieces.GetCount()-1);
}

TileID CPieceNewDialog::GetTileID(const CComboBox& pCombo, const CTileListBox& pList) const
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

void CPieceNewDialog::RefreshPieceList()
{
    CPieceSet& pPSet = m_pPMgr->GetPieceSet(m_nPSet);
    const std::vector<PieceID>& pLstMap = pPSet.GetPieceIDTable();
    m_listPieces.SetItemMap(&pLstMap);
}

/////////////////////////////////////////////////////////////////////////////
// CPieceNewDialog message handlers

void CPieceNewDialog::OnSelchangeBtset()
{
    SetupTileListbox(m_comboBtset, m_listBtile);
}

void CPieceNewDialog::OnSelchangeFtset()
{
    SetupTileListbox(m_comboFtset, m_listFtile);
}

void CPieceNewDialog::OnBtnClickTopVisible()
{
    m_chkTopOnlyOwnersToo.EnableWindow(m_chkTopOnlyVisible.GetCheck() != 0);
}

void CPieceNewDialog::OnCreate()
{
    CreatePiece();
    m_editQty.SetWindowText("1"_cbstring);       // Always set quantity back to 1

    OnCheckTextSameAsTop();

    m_pDoc->SetModifiedFlag();
}

void CPieceNewDialog::OnSelchangeNumSides()
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
        OnCheckTextSameAsTop();
    }
    else
    {
        m_currSide.EnableWindow(FALSE);
        m_comboBtset.EnableWindow(FALSE);
        m_listBtile.EnableWindow(FALSE);
        m_editTextBack.EnableWindow(FALSE);
        m_chkSameAsTop.EnableWindow(FALSE);
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

BOOL CPieceNewDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Add piece set name to dialog title
    CB::string strTitle = CB::string::GetWindowText(*this);
    strTitle += " - ";
    strTitle += m_pPMgr->GetPieceSet(m_nPSet).GetName();
    SetWindowText(strTitle);

    m_listPieces.SetDocument(*m_pDoc);

    for (size_t i = size_t(0); i < PieceDef::maxSides; ++i)
    {
        int rc = m_numSides.AddString(static_cast<CB::string>(std::format("{}", i + size_t(1))));
        if (rc != value_preserving_cast<int>(i))
        {
            AfxThrowMemoryException();
        }
    }
    CB::string str = AfxFormatString1(IDS_SIDE_N, std::format("{}", 1));
    m_side_1.SetWindowText(str);

    m_listFtile.SetDocument(&*m_pDoc);
    m_listBtile.SetDocument(&*m_pDoc);

    SetupTileSetNames(m_comboFtset);
    SetupTileListbox(m_comboFtset, m_listFtile);
    SetupTileSetNames(m_comboBtset);
    SetupTileListbox(m_comboBtset, m_listBtile);

    RefreshPieceList();

    m_numSides.SetCurSel(0);
    // satisfy OnSelchangeNumSides() preconditions
    m_sideTids.push_back(nullTid);
    m_sideTexts.push_back("");
    OnSelchangeNumSides();

    m_editQty.SetWindowText("1"_cbstring);

    m_chkSameAsTop.SetCheck(0);
    m_chkTopOnlyVisible.SetCheck(0);
    m_chkTopOnlyOwnersToo.SetCheck(0);
    m_editTextFront.SetWindowText(""_cbstring);
    m_editTextBack.SetWindowText(""_cbstring);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPieceNewDialog::OnDblClkFrontTile()
{
    OnCreate();
}

void CPieceNewDialog::OnOK()
{
    // TODO: Add extra validation here
    CDialog::OnOK();
}

void CPieceNewDialog::OnCheckTextSameAsTop()
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

void CPieceNewDialog::OnSelchangeCurrSide()
{
    if (m_prevSide < m_sideTids.size())
    {
        TileID tidBack = GetTileID(m_comboBtset, m_listBtile);
        m_sideTids[m_prevSide] = tidBack;
        ASSERT(m_sideTexts.size() == m_sideTids.size());
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

