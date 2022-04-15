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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPieceNewDialog dialog


CPieceNewDialog::CPieceNewDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CPieceNewDialog::IDD, pParent)
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
    DDX_Control(pDX, IDC_D_PCREATE_BACK, m_chkBack);
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
    ON_BN_CLICKED(IDC_D_PCREATE_BACK, OnBackCheck)
    ON_LBN_DBLCLK(IDC_D_PCREATE_FTILE, OnDblClkFrontTile)
    ON_BN_CLICKED(IDC_D_PCREATE_TEXT_SAMEASTOP, OnCheckTextSameAsTop)
    ON_BN_CLICKED(IDC_D_PCREATE_TOP_VISIBLE, OnBtnClickTopVisible)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_PCREATE_BACK, IDH_D_PCREATE_BACK,
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

void CPieceNewDialog::SetupTileListbox(CComboBox *pCombo, CTileListBox *pList)
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

void CPieceNewDialog::SetupTileSetNames(CComboBox* pCombo)
{
    ASSERT(m_pTMgr);
    pCombo->ResetContent();

    for (size_t i = 0; i < m_pTMgr->GetNumTileSets(); i++)
        pCombo->AddString(m_pTMgr->GetTileSet(i).GetName());
    if (!m_pTMgr->IsEmpty())
        pCombo->SetCurSel(0);           // Select the first entry
}

void CPieceNewDialog::CreatePiece()
{
    TileID tidFront = GetTileID(&m_comboFtset, &m_listFtile);
    TileID tidBack  = GetTileID(&m_comboBtset, &m_listBtile);

    if (tidFront == nullTid)
        return;

    CString str;
    m_editQty.GetWindowText(str);
    int nNumPieces = atoi(str);
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

    BOOL bBackChecked = m_chkBack.GetCheck() == 1;

    for (int i = 0; i < nNumPieces; i++)
    {
        PieceID pid = m_pPMgr->CreatePiece(m_nPSet, tidFront,
            bBackChecked ? tidBack : nullTid);
        CString strText;
        m_editTextFront.GetWindowText(strText);
        if (!strText.IsEmpty())
        {
            GameElement ge = MakePieceElement(pid, unsigned(0));
            m_pDoc->GetGameStringMap().SetAt(ge, strText);
        }
        if (m_chkSameAsTop.GetCheck() == 0)
            m_editTextBack.GetWindowText(strText);
        if (bBackChecked && !strText.IsEmpty())
        {
            GameElement ge = MakePieceElement(pid, unsigned(1));
            m_pDoc->GetGameStringMap().SetAt(ge, strText);
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

TileID CPieceNewDialog::GetTileID(CComboBox *pCombo, CTileListBox *pList)
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
    SetupTileListbox(&m_comboBtset, &m_listBtile);
}

void CPieceNewDialog::OnSelchangeFtset()
{
    SetupTileListbox(&m_comboFtset, &m_listFtile);
}

void CPieceNewDialog::OnBtnClickTopVisible()
{
    m_chkTopOnlyOwnersToo.EnableWindow(m_chkTopOnlyVisible.GetCheck() != 0);
}

void CPieceNewDialog::OnCreate()
{
    CreatePiece();
    m_editQty.SetWindowText("1");       // Always set quantity back to 1

    OnCheckTextSameAsTop();

    m_pDoc->SetModifiedFlag();
}

void CPieceNewDialog::OnBackCheck()
{
    BOOL bBackChecked = m_chkBack.GetCheck() != 0;
    if (bBackChecked)
    {
        m_comboBtset.EnableWindow(TRUE);
        m_listBtile.EnableWindow(TRUE);
        m_chkSameAsTop.EnableWindow(TRUE);
        m_chkTopOnlyVisible.EnableWindow(TRUE);
        m_chkTopOnlyOwnersToo.EnableWindow(m_chkTopOnlyVisible.GetCheck() != 0);
        OnCheckTextSameAsTop();
    }
    else
    {
        m_comboBtset.EnableWindow(FALSE);
        m_listBtile.EnableWindow(FALSE);
        m_editTextBack.EnableWindow(FALSE);
        m_chkSameAsTop.EnableWindow(FALSE);
        m_chkTopOnlyVisible.EnableWindow(FALSE);
        m_chkTopOnlyOwnersToo.EnableWindow(FALSE);
        m_editTextBack.SetWindowText("");
    }
}

BOOL CPieceNewDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(m_pDoc);
    m_pPMgr = m_pDoc->GetPieceManager();
    ASSERT(m_pPMgr);

    // Add piece set name to dialog title
    CString strTitle;
    GetWindowText(strTitle);
    strTitle += " - ";
    strTitle += m_pPMgr->GetPieceSet(m_nPSet).GetName();
    SetWindowText(strTitle);

    m_listPieces.SetDocument(*m_pDoc);

    m_pTMgr = m_pDoc->GetTileManager();
    ASSERT(m_pTMgr);

    m_listFtile.SetDocument(m_pDoc);
    m_listBtile.SetDocument(m_pDoc);

    SetupTileSetNames(&m_comboFtset);
    SetupTileListbox(&m_comboFtset, &m_listFtile);
    SetupTileSetNames(&m_comboBtset);
    SetupTileListbox(&m_comboBtset, &m_listBtile);

    RefreshPieceList();

    m_chkBack.SetCheck(0);

    m_editQty.SetWindowText("1");

    m_chkSameAsTop.SetCheck(0);
    m_chkTopOnlyVisible.SetCheck(0);
    m_chkTopOnlyOwnersToo.SetCheck(0);
    m_editTextFront.SetWindowText("");
    m_editTextBack.SetWindowText("");

    OnBackCheck();          // To reflect state of check box.

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
        m_editTextBack.SetWindowText("");
        m_editTextBack.EnableWindow(FALSE);
    }
    else
        m_editTextBack.EnableWindow(m_chkBack.GetCheck() != 0);
}

