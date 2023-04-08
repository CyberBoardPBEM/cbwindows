// DlgMnew.cpp : implementation file
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
#include    "DlgMnew.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkerCreateDialog dialog

CMarkerCreateDialog::CMarkerCreateDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CMarkerCreateDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CMarkerCreateDialog)
    //}}AFX_DATA_INIT
    m_pMMgr = NULL;
    m_nMSet = 0;
}

void CMarkerCreateDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMarkerCreateDialog)
    DDX_Control(pDX, IDC_D_MCREATE_TEXTPROMPT, m_chkPromptText);
    DDX_Control(pDX, IDC_D_MCREATE_TEXT, m_editMarkerText);
    DDX_Control(pDX, IDC_D_MCREATE_TSET, m_comboTSet);
    DDX_Control(pDX, IDC_D_MCREATE_TILES, m_listTiles);
    DDX_Control(pDX, IDC_D_MCREATE_MARKERS, m_listMarks);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMarkerCreateDialog, CDialog)
    //{{AFX_MSG_MAP(CMarkerCreateDialog)
    ON_BN_CLICKED(IDC_D_MCREATE_CREATE, OnCreateMarker)
    ON_CBN_SELCHANGE(IDC_D_MCREATE_TSET, OnSelchangeTSet)
    ON_LBN_DBLCLK(IDC_D_MCREATE_TILES, OnDblclkTiles)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Html Help control ID Map

static DWORD adwHelpMap[] =
{
    IDC_D_MCREATE_CREATE, IDH_D_MCREATE_CREATE,
    IDC_D_MCREATE_MARKERS, IDH_D_MCREATE_MARKERS,
    IDC_D_MCREATE_TEXT, IDH_D_MCREATE_TEXT,
    IDC_D_MCREATE_TEXTPROMPT, IDH_D_MCREATE_TEXTPROMPT,
    IDC_D_MCREATE_TILES, IDH_D_MCREATE_TILES,
    IDC_D_MCREATE_TSET, IDH_D_MCREATE_TSET,
    0,0
};

BOOL CMarkerCreateDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return GetApp()->DoHelpTipHelp(pHelpInfo, adwHelpMap);
}

void CMarkerCreateDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
    GetApp()->DoHelpWhatIsHelp(pWnd, adwHelpMap);
}

/////////////////////////////////////////////////////////////////////////////

void CMarkerCreateDialog::SetupTileListbox()
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

void CMarkerCreateDialog::SetupTileSetNames()
{
    ASSERT(m_pTMgr);
    m_comboTSet.ResetContent();

    for (size_t i = 0; i < m_pTMgr->GetNumTileSets(); i++)
        m_comboTSet.AddString(m_pTMgr->GetTileSet(i).GetName());
    if (!m_pTMgr->IsEmpty())
        m_comboTSet.SetCurSel(0);           // Select the first entry
}

void CMarkerCreateDialog::CreateMarker()
{
    TileID tid = GetTileID();
    if (tid == nullTid)
        return;
    WORD wMarkFlags = m_chkPromptText.GetCheck() != 0 ? MarkDef::flagPromptText : 0;
    MarkID mid = m_pMMgr->CreateMark(m_nMSet, tid, wMarkFlags);
    CB::string strMarkText = CB::string::GetWindowText(m_editMarkerText);
    if (!strMarkText.empty())
        m_pDoc->GetGameStringMap().SetAt(MakeMarkerElement(mid), strMarkText);
    RefreshMarkerList();
}

TileID CMarkerCreateDialog::GetTileID() const
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

void CMarkerCreateDialog::RefreshMarkerList()
{
    CMarkSet& pMSet = m_pMMgr->GetMarkSet(m_nMSet);
    const std::vector<MarkID>& pLstMap = pMSet.GetMarkIDTable();
    m_listMarks.SetItemMap(&pLstMap);
}

/////////////////////////////////////////////////////////////////////////////
// CMarkerCreateDialog message handlers

void CMarkerCreateDialog::OnCreateMarker()
{
    CreateMarker();
}

BOOL CMarkerCreateDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(m_pDoc);
    m_pMMgr = m_pDoc->GetMarkManager();
    ASSERT(m_pMMgr);

    m_listMarks.SetDocument(m_pDoc);

    m_pTMgr = m_pDoc->GetTileManager();
    ASSERT(m_pTMgr);

    m_listTiles.SetDocument(m_pDoc);

    RefreshMarkerList();

    SetupTileSetNames();
    SetupTileListbox();

    m_chkPromptText.SetCheck(0);
    m_editMarkerText.SetWindowText("");

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMarkerCreateDialog::OnSelchangeTSet()
{
    SetupTileListbox();
}


void CMarkerCreateDialog::OnDblclkTiles()
{
    CreateMarker();
}
