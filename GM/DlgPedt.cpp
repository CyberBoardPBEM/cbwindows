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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPieceEditDialog dialog


CPieceEditDialog::CPieceEditDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CPieceEditDialog::IDD, pParent)
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
    DDX_Control(pDX, IDC_D_PEDIT_BACK, m_chkBack);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPieceEditDialog, CDialog)
    //{{AFX_MSG_MAP(CPieceEditDialog)
    ON_BN_CLICKED(IDC_D_PEDIT_BACK, OnBackCheck)
    ON_CBN_SELCHANGE(IDC_D_PEDIT_BTSET, OnSelchangeBtset)
    ON_CBN_SELCHANGE(IDC_D_PEDIT_FTSET, OnSelchangeFtset)
    ON_LBN_DBLCLK(IDC_D_PEDIT_PIECE, OnDblclkPiece)
    ON_BN_CLICKED(IDC_D_PEDIT_TEXT_SAMEASTOP, OnCheckSameAsTop)
    ON_BN_CLICKED(IDC_D_PEDIT_TOP_VISIBLE, OnBtnClickTopVisible)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_PEDIT_BACK, IDH_D_PEDIT_BACK,
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

void CPieceEditDialog::SetupTileListbox(CComboBox *pCombo, CTileListBox *pList)
{
    int nCurSel = pCombo->GetCurSel();
    if (nCurSel < 0)
    {
        pList->SetItemMap(NULL);
        return;
    }

    const CTileSet& pTSet = m_pTMgr->GetTileSet(value_preserving_cast<size_t>(nCurSel));
    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    pList->SetItemMap(&pLstMap);
}

void CPieceEditDialog::SetupTileSetNames(CComboBox* pCombo)
{
    ASSERT(m_pTMgr);
    pCombo->ResetContent();

    for (size_t i = 0; i < m_pTMgr->GetNumTileSets(); i++)
        pCombo->AddString(m_pTMgr->GetTileSet(i).GetName());
    if (!m_pTMgr->IsEmpty())
        pCombo->SetCurSel(-1);          // Select no entry yet
}

TileID CPieceEditDialog::GetTileID(CComboBox *pCombo, CTileListBox *pList)
{
    int nCurSel = pCombo->GetCurSel();
    if (nCurSel < 0)
        return nullTid;

    int nCurTile = pList->GetCurSel();
    if (nCurTile < 0)
        return nullTid;

    const CTileSet& pTSet = m_pTMgr->GetTileSet(value_preserving_cast<size_t>(nCurSel));

    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    return pLstMap.at(value_preserving_cast<size_t>(nCurTile));
}

/////////////////////////////////////////////////////////////////////////////

void CPieceEditDialog::SetupPieceTiles()
{
    PieceDef& pDef = m_pPMgr->GetPiece(m_pid);

    size_t nSet = m_pTMgr->FindTileSetFromTileID(pDef.m_tidFront);
    ASSERT(nSet != Invalid_v<size_t>);

    m_comboFtset.SetCurSel(value_preserving_cast<int>(nSet));
    SetupTileListbox(&m_comboFtset, &m_listFtile);
    m_listFtile.SetCurSelMapped(pDef.m_tidFront);

    if (pDef.m_tidBack != nullTid)
    {
        nSet = m_pTMgr->FindTileSetFromTileID(pDef.m_tidBack);
        ASSERT(nSet != Invalid_v<size_t>);
        m_comboBtset.SetCurSel(value_preserving_cast<int>(nSet));
        SetupTileListbox(&m_comboBtset, &m_listBtile);
        m_listBtile.SetCurSelMapped(pDef.m_tidBack);
    }
    else
    {
        m_comboBtset.SetCurSel(-1);
        SetupTileListbox(&m_comboBtset, &m_listBtile);
    }
    m_chkBack.SetCheck(pDef.m_tidBack != nullTid ? 1 : 0);

    OnBackCheck();          // To reflect state of check box.
}

/////////////////////////////////////////////////////////////////////////////
// CPieceEditDialog message handlers

void CPieceEditDialog::OnBtnClickTopVisible()
{
    m_chkTopOnlyOwnersToo.EnableWindow(m_chkTopOnlyVisible.GetCheck() != 0);
}

void CPieceEditDialog::OnBackCheck()
{
    BOOL bChecked = m_chkBack.GetCheck() != 0;
    if (bChecked)
    {
        m_comboBtset.EnableWindow(TRUE);
        m_listBtile.EnableWindow(TRUE);
        m_chkSameAsTop.EnableWindow(TRUE);
        m_chkTopOnlyVisible.EnableWindow(TRUE);
        m_chkTopOnlyOwnersToo.EnableWindow(m_chkTopOnlyVisible.GetCheck() != 0);
        OnCheckSameAsTop();
    }
    else
    {
        m_comboBtset.EnableWindow(FALSE);
        m_listBtile.EnableWindow(FALSE);
        m_chkSameAsTop.EnableWindow(FALSE);
        m_editTextBack.EnableWindow(FALSE);
        m_chkTopOnlyVisible.EnableWindow(FALSE);
        m_chkTopOnlyOwnersToo.EnableWindow(FALSE);
        m_editTextBack.SetWindowText("");
    }
}

void CPieceEditDialog::OnOK()
{
    PieceDef& pDef = m_pPMgr->GetPiece(m_pid);
    TileID tidFront = GetTileID(&m_comboFtset, &m_listFtile);
    TileID tidBack  = GetTileID(&m_comboBtset, &m_listBtile);
    if (tidFront == nullTid)
        return;

    BOOL bBackChecked = m_chkBack.GetCheck() == 1;

    pDef.m_tidFront = tidFront;
    pDef.m_tidBack = bBackChecked ? tidBack : nullTid;

    CString strText;
    m_editTextFront.GetWindowText(strText);

    GameElement ge = MakePieceElement(m_pid, unsigned(0));
    if (!strText.IsEmpty())
        m_pDoc->GetGameStringMap().SetAt(ge, strText);
    else
        m_pDoc->GetGameStringMap().RemoveKey(ge);

    if (m_chkSameAsTop.GetCheck() == 0)
        m_editTextBack.GetWindowText(strText);

    ge = MakePieceElement(m_pid, unsigned(1));
    if (bBackChecked && !strText.IsEmpty())
        m_pDoc->GetGameStringMap().SetAt(ge, strText);
    else
        m_pDoc->GetGameStringMap().RemoveKey(ge);

    pDef.m_flags &= ~PieceDef::flagShowOnlyVisibleSide;      // Initially clear the flags
    pDef.m_flags &= ~PieceDef::flagShowOnlyOwnersToo;
    if (bBackChecked && m_chkTopOnlyVisible.GetCheck() != 0)
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

    ASSERT(m_pDoc);
    m_listPieces.SetDocument(m_pDoc);

    m_pPMgr = m_pDoc->GetPieceManager();

    m_tbl.push_back(m_pid);
    m_listPieces.SetItemMap(&m_tbl);

    m_pTMgr = m_pDoc->GetTileManager();
    ASSERT(m_pTMgr);

    m_listFtile.SetDocument(m_pDoc);
    m_listBtile.SetDocument(m_pDoc);

    SetupTileSetNames(&m_comboFtset);
    SetupTileSetNames(&m_comboBtset);

    SetupPieceTiles();

    CString strText;
    m_pDoc->GetGameStringMap().Lookup(MakePieceElement(m_pid, unsigned(0)), strText);
    if (!strText.IsEmpty())
        m_editTextFront.SetWindowText(strText);
    m_chkSameAsTop.SetCheck(0);
    m_chkTopOnlyVisible.SetCheck(0);
    m_chkTopOnlyOwnersToo.SetCheck(0);

    PieceDef& pPce = m_pPMgr->GetPiece(m_pid);
    if (pPce.Is2Sided())
    {
        CString strTextBack;
        m_pDoc->GetGameStringMap().Lookup(MakePieceElement(m_pid, unsigned(1)), strTextBack);
        if (strText == strTextBack)
            m_chkSameAsTop.SetCheck(1);
        if (!strTextBack.IsEmpty())
            m_editTextBack.SetWindowText(strTextBack);
        m_chkTopOnlyVisible.SetCheck(
            (pPce.m_flags & PieceDef::flagShowOnlyVisibleSide) ? 1 : 0);
        m_chkTopOnlyOwnersToo.SetCheck(
            (pPce.m_flags & PieceDef::flagShowOnlyOwnersToo) ? 1 : 0);
    }
    OnCheckSameAsTop();
    OnBackCheck();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPieceEditDialog::OnSelchangeBtset()
{
    SetupTileListbox(&m_comboBtset, &m_listBtile);
}

void CPieceEditDialog::OnSelchangeFtset()
{
    SetupTileListbox(&m_comboFtset, &m_listFtile);
}

void CPieceEditDialog::OnDblclkPiece()
{
    SetupPieceTiles();
}

void CPieceEditDialog::OnCheckSameAsTop()
{
    BOOL bSameAsTop = m_chkSameAsTop.GetCheck() != 0;
    if (bSameAsTop)
    {
        m_editTextBack.SetWindowText("");
        m_editTextBack.EnableWindow(FALSE);
    }
    else
        m_editTextBack.EnableWindow(m_chkBack.GetCheck() != 0);
}

