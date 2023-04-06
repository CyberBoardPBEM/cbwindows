// dlgmedt.cpp : implementation file
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
#include    "Marks.h"
#include    "DlgMedt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkerEditDialog dialog


CMarkerEditDialog::CMarkerEditDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CMarkerEditDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CMarkerEditDialog)
    //}}AFX_DATA_INIT
}

void CMarkerEditDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMarkerEditDialog)
    DDX_Control(pDX, IDC_D_MEDIT_TEXTPROMPT, m_chkPromptText);
    DDX_Control(pDX, IDC_D_MEDIT_TEXT, m_editMarkerText);
    DDX_Control(pDX, IDC_D_MEDIT_MARKERS, m_listMarks);
    DDX_Control(pDX, IDC_D_MEDIT_TILES, m_listTiles);
    DDX_Control(pDX, IDC_D_MEDIT_TSET, m_comboTSet);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMarkerEditDialog, CDialog)
    //{{AFX_MSG_MAP(CMarkerEditDialog)
    ON_CBN_SELCHANGE(IDC_D_MEDIT_TSET, OnSelchangeTSet)
    ON_LBN_DBLCLK(IDC_D_MEDIT_MARKERS, OnDblclkMarkers)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_MEDIT_MARKERS, IDH_D_MEDIT_MARKERS,
    IDC_D_MEDIT_TEXT, IDH_D_MEDIT_TEXT,
    IDC_D_MEDIT_TEXTPROMPT, IDH_D_MEDIT_TEXTPROMPT,
    IDC_D_MEDIT_TILES, IDH_D_MEDIT_TILES,
    IDC_D_MEDIT_TSET, IDH_D_MEDIT_TSET,
    0,0
};

BOOL CMarkerEditDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CMarkerEditDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerEditDialog::SetupTileListbox()
{
    int nCurSel = m_comboTSet.GetCurSel();
    if (nCurSel < 0)
    {
        m_listTiles.SetItemMap(NULL);
        return;
    }

    const CTileSet& pTSet = m_pTMgr->GetTileSet(value_preserving_cast<size_t>(nCurSel));
    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    m_listTiles.SetItemMap(&pLstMap);
}

void CMarkerEditDialog::SetupTileSetNames()
{
    ASSERT(m_pTMgr);
    m_comboTSet.ResetContent();

    for (size_t i = 0; i < m_pTMgr->GetNumTileSets(); i++)
        m_comboTSet.AddString(m_pTMgr->GetTileSet(i).GetName());
    if (!m_pTMgr->IsEmpty())
        m_comboTSet.SetCurSel(0);           // Select the first entry
}

TileID CMarkerEditDialog::GetTileID()
{
    int nCurSel = m_comboTSet.GetCurSel();
    if (nCurSel < 0)
        return nullTid;

    int nCurTile = m_listTiles.GetCurSel();
    if (nCurTile < 0)
        return nullTid;

    const CTileSet& pTSet = m_pTMgr->GetTileSet(value_preserving_cast<size_t>(nCurSel));

    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    return pLstMap.at(value_preserving_cast<size_t>(nCurTile));
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerEditDialog::SetupMarkerTile()
{
    MarkDef& pDef = m_pMMgr->GetMark(m_mid);

    size_t nSet = m_pTMgr->FindTileSetFromTileID(pDef.m_tid);
    ASSERT(nSet != Invalid_v<size_t>);

    m_comboTSet.SetCurSel(value_preserving_cast<int>(nSet));
    SetupTileListbox();
    m_listTiles.SetCurSelMapped(pDef.m_tid);
}

/////////////////////////////////////////////////////////////////////////////
// CMarkerEditDialog message handlers

void CMarkerEditDialog::OnOK()
{
    MarkDef& pDef = m_pMMgr->GetMark(m_mid);
    TileID tid = GetTileID();
    pDef.m_tid = tid;
    if (m_chkPromptText.GetCheck() != 0)
        pDef.m_flags |= MarkDef::flagPromptText;
    else
        pDef.m_flags &= ~MarkDef::flagPromptText;

    CString strMarkText;
    m_editMarkerText.GetWindowText(strMarkText);
    if (strMarkText.IsEmpty())
        m_pDoc->GetGameStringMap().RemoveKey(MakeMarkerElement(m_mid));
    else
        m_pDoc->GetGameStringMap().SetAt(MakeMarkerElement(m_mid), strMarkText);

    CDialog::OnOK();
}

void CMarkerEditDialog::OnSelchangeTSet()
{
    SetupTileListbox();
}

void CMarkerEditDialog::OnDblclkMarkers()
{
    SetupMarkerTile();
}


BOOL CMarkerEditDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(m_pDoc);
    m_pMMgr = m_pDoc->GetMarkManager();
    ASSERT(m_pMMgr);

    m_listMarks.SetDocument(m_pDoc);

    m_tbl.push_back(m_mid);
    m_listMarks.SetItemMap(&m_tbl);

    CB::string strMarkText;
    m_pDoc->GetGameStringMap().Lookup(MakeMarkerElement(m_mid), strMarkText);
    if (!strMarkText.empty())
        m_editMarkerText.SetWindowText(strMarkText);

    WORD wMarkFlags = m_pMMgr->GetMark(m_mid).m_flags;
    m_chkPromptText.SetCheck(wMarkFlags & MarkDef::flagPromptText ? 1 : 0);

    m_pTMgr = m_pDoc->GetTileManager();
    ASSERT(m_pTMgr);

    m_listTiles.SetDocument(m_pDoc);

    SetupTileSetNames();

    SetupMarkerTile();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

