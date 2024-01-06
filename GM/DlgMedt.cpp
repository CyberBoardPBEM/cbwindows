// dlgmedt.cpp : implementation file
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
#include    "DlgMedt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarkerEditDialog dialog


CMarkerEditDialog::CMarkerEditDialog(wxWindow* parent /*= &CB::GetMainWndWx()*/) :
    CB_XRC_BEGIN_CTRLS_DEFN(parent, CMarkerEditDialog)
        CB_XRC_CTRL(m_chkPromptText)
        CB_XRC_CTRL(m_editMarkerText)
        CB_XRC_CTRL(m_listMarks)
        CB_XRC_CTRL(m_listTiles)
        CB_XRC_CTRL(m_comboTSet)
    CB_XRC_END_CTRLS_DEFN()
{
}

wxBEGIN_EVENT_TABLE(CMarkerEditDialog, wxDialog)
    EVT_CHOICE(XRCID("m_comboTSet"), OnSelchangeTSet)
    EVT_LISTBOX_DCLICK(XRCID("m_listMarks"), OnDblclkMarkers)
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
#endif

/////////////////////////////////////////////////////////////////////////////

void CMarkerEditDialog::SetupTileListbox()
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

void CMarkerEditDialog::SetupTileSetNames()
{
    ASSERT(m_pTMgr);
    m_comboTSet->Clear();

    for (size_t i = size_t(0); i < m_pTMgr->GetNumTileSets(); i++)
        m_comboTSet->Append(m_pTMgr->GetTileSet(i).GetName());
    if (!m_pTMgr->IsEmpty())
        m_comboTSet->SetSelection(0);           // Select the first entry
}

TileID CMarkerEditDialog::GetTileID()
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

/////////////////////////////////////////////////////////////////////////////

void CMarkerEditDialog::SetupMarkerTile()
{
    MarkDef& pDef = m_pMMgr->GetMark(m_mid);

    size_t nSet = m_pTMgr->FindTileSetFromTileID(pDef.m_tid);
    ASSERT(nSet != Invalid_v<size_t>);

    m_comboTSet->SetSelection(value_preserving_cast<int>(nSet));
    SetupTileListbox();
    m_listTiles->SetCurSelMapped(pDef.m_tid);
}

/////////////////////////////////////////////////////////////////////////////
// CMarkerEditDialog message handlers

bool CMarkerEditDialog::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow())
    {
        return false;
    }

    MarkDef& pDef = m_pMMgr->GetMark(m_mid);
    TileID tid = GetTileID();
    pDef.m_tid = tid;
    if (m_chkPromptText->GetValue())
        pDef.m_flags |= MarkDef::flagPromptText;
    else
        pDef.m_flags &= ~MarkDef::flagPromptText;

    CB::string strMarkText = m_editMarkerText->GetValue();
    if (strMarkText.empty())
        m_pDoc->GetGameStringMap().RemoveKey(MakeMarkerElement(m_mid));
    else
        m_pDoc->GetGameStringMap().SetAt(MakeMarkerElement(m_mid), strMarkText);

    return true;
}

void CMarkerEditDialog::OnSelchangeTSet(wxCommandEvent& /*event*/)
{
    SetupTileListbox();
}

void CMarkerEditDialog::OnDblclkMarkers(wxCommandEvent& /*event*/)
{
    SetupMarkerTile();
}


bool CMarkerEditDialog::TransferDataToWindow()
{
    ASSERT(m_pDoc);
    m_pMMgr = m_pDoc->GetMarkManager();
    ASSERT(m_pMMgr);

    m_listMarks->SetDocument(m_pDoc);

    m_tbl.push_back(m_mid);
    m_listMarks->SetItemMap(&m_tbl);

    CB::string strMarkText;
    m_pDoc->GetGameStringMap().Lookup(MakeMarkerElement(m_mid), strMarkText);
    if (!strMarkText.empty())
        m_editMarkerText->SetValue(strMarkText);

    WORD wMarkFlags = m_pMMgr->GetMark(m_mid).m_flags;
    m_chkPromptText->SetValue(wMarkFlags & MarkDef::flagPromptText);

    m_pTMgr = m_pDoc->GetTileManager();
    ASSERT(m_pTMgr);

    m_listTiles->SetDocument(m_pDoc);

    SetupTileSetNames();

    SetupMarkerTile();

    return wxDialog::TransferDataToWindow();
}

