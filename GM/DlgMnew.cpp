// DlgMnew.cpp : implementation file
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
#include    "Marks.h"
#include    "DlgMnew.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkerCreateDialog dialog

CMarkerCreateDialog::CMarkerCreateDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CMarkerCreateDialog)
        CB_XRC_CTRL(m_chkPromptText)
        CB_XRC_CTRL(m_editMarkerText)
        CB_XRC_CTRL(m_comboTSet)
        CB_XRC_CTRL(m_listTiles)
        CB_XRC_CTRL(m_listMarks)
        CB_XRC_CTRL(m_btnCreateMarker)
    CB_XRC_END_CTRLS_DEFN()
{
    m_pMMgr = NULL;
    m_nMSet = 0;
}

wxBEGIN_EVENT_TABLE(CMarkerCreateDialog, wxDialog)
    EVT_BUTTON(XRCID("m_btnCreateMarker"), OnCreateMarker)
    EVT_INIT_DIALOG(OnInitDialog)
    EVT_CHOICE(XRCID("m_comboTSet"), OnSelchangeTSet)
    EVT_LISTBOX_DCLICK(XRCID("m_listTiles"), OnDblclkTiles)
#if 0
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
#endif
wxEND_EVENT_TABLE()

#if 0
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
#endif

/////////////////////////////////////////////////////////////////////////////

void CMarkerCreateDialog::SetupTileListbox()
{
    int nCurSel = m_comboTSet->GetSelection();
    if (nCurSel == wxNOT_FOUND)
    {
        m_listTiles->SetItemMap(NULL);
        return;
    }

    const CTileSet& pTSet = m_pTMgr->GetTileSet(value_preserving_cast<size_t>(nCurSel));
    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    m_listTiles->SetItemMap(&pLstMap);
}

void CMarkerCreateDialog::SetupTileSetNames()
{
    ASSERT(m_pTMgr);
    m_comboTSet->Clear();

    for (size_t i = size_t(0); i < m_pTMgr->GetNumTileSets(); i++)
        m_comboTSet->Append(m_pTMgr->GetTileSet(i).GetName());
    if (!m_pTMgr->IsEmpty())
        m_comboTSet->SetSelection(0);           // Select the first entry
}

void CMarkerCreateDialog::CreateMarker()
{
    TileID tid = GetTileID();
    if (tid == nullTid)
        return;
    WORD wMarkFlags = m_chkPromptText->GetValue() ? MarkDef::flagPromptText : 0;
    MarkID mid = m_pMMgr->CreateMark(m_nMSet, tid, wMarkFlags);
    CB::string strMarkText = m_editMarkerText->GetValue();
    if (!strMarkText.empty())
        m_pDoc->GetGameStringMap().SetAt(MakeMarkerElement(mid), strMarkText);
    RefreshMarkerList();
}

TileID CMarkerCreateDialog::GetTileID() const
{
    int nCurSel = m_comboTSet->GetSelection();
    if (nCurSel == wxNOT_FOUND)
        return nullTid;

    int nCurTile = m_listTiles->GetSelection();
    if (nCurTile == wxNOT_FOUND)
        return nullTid;

    const CTileSet& pTSet = m_pTMgr->GetTileSet(value_preserving_cast<size_t>(nCurSel));

    const std::vector<TileID>& pLstMap = pTSet.GetTileIDTable();
    return pLstMap.at(value_preserving_cast<size_t>(nCurTile));
}

void CMarkerCreateDialog::RefreshMarkerList()
{
    CMarkSet& pMSet = m_pMMgr->GetMarkSet(m_nMSet);
    const std::vector<MarkID>& pLstMap = pMSet.GetMarkIDTable();
    m_listMarks->SetItemMap(&pLstMap);
}

/////////////////////////////////////////////////////////////////////////////
// CMarkerCreateDialog message handlers

void CMarkerCreateDialog::OnCreateMarker(wxCommandEvent& event)
{
    CreateMarker();
}

void CMarkerCreateDialog::OnInitDialog(wxInitDialogEvent& event)
{
    ASSERT(m_pDoc);
    m_pMMgr = m_pDoc->GetMarkManager();
    ASSERT(m_pMMgr);

    m_listMarks->SetDocument(m_pDoc);

    m_pTMgr = m_pDoc->GetTileManager();
    ASSERT(m_pTMgr);

    m_listTiles->SetDocument(m_pDoc);

    RefreshMarkerList();

    SetupTileSetNames();
    SetupTileListbox();

    m_chkPromptText->SetValue(false);
    m_editMarkerText->SetValue(""_cbstring);

    event.Skip();
}

void CMarkerCreateDialog::OnSelchangeTSet(wxCommandEvent& event)
{
    SetupTileListbox();
}


void CMarkerCreateDialog::OnDblclkTiles(wxCommandEvent& event)
{
    CreateMarker();
}
