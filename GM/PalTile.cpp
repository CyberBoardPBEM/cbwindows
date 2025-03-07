// PalTile.cpp - Tile palette window
//
// Copyright (c) 1994-2025 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "Frmmain.h"
#include    "ResTbl.h"
#include    "Tile.h"
#include    "PalTile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CTilePalette, wxPanel)
    EVT_CHOICE(XRCID("m_comboTGrp"), OnTileNameCbnSelchange)
    EVT_GET_DRAG_SIZE(OnGetDragSize)
#if 0
    ON_WM_HELPINFO()
#endif
    EVT_COMMAND(wxID_ANY, WM_PALETTE_HIDE_WX, OnPaletteHide)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////

CTilePalette::CTilePalette(const CGamDoc& pDoc, wxWindow& pOwnerWnd) :
    m_pDoc(pDoc),
    CB_XRC_BEGIN_CTRLS_DEFN(&pOwnerWnd, CTilePalette)
        CB_XRC_CTRL(m_comboTGrp)
        CB_XRC_CTRL(m_listTile)
    CB_XRC_END_CTRLS_DEFN()
{
    wxInfoDC dc(&*m_comboTGrp);
    int minX = dc.GetTextExtent("MMMMMMMM"_cbstring).x;
    int minY = m_comboTGrp->GetBestSize().y * 3;
    SetMinClientSize(wxSize(minX, minY));
    m_pDockingFrame = NULL;
    m_listTile->SetDocument(&pDoc);
    m_listTile->EnableDrag(TRUE);

    UpdatePaletteContents();
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::OnPaletteHide(wxCommandEvent& /*event*/)
{
    wxCommandEvent event(wxEVT_MENU, XRCID("ID_WINDOW_TILEPAL"));
    GetMainFrame()->ProcessWindowEvent(event);
}

/////////////////////////////////////////////////////////////////////////////

TileID CTilePalette::GetCurrentTileID() const
{
    wxASSERT(GetHandle());
    if (!GetHandle())
    {
        return nullTid;
    }
    if (m_listTile->GetSelection() == wxNOT_FOUND)
        return nullTid;
    return m_listTile->GetCurMapItem();
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::Serialize(CArchive& ar)
{
#pragma message("TODO--> Serialize current indices <--TODO")
    if (ar.IsStoring())
    {
        ar << CRect();
    }
    else
    {
        CRect junk;
        ar >> junk;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::LoadTileNameList()
{
    const CTileManager& pTMgr = m_pDoc.GetTileManager();

    m_comboTGrp->Clear();
    for (size_t i = size_t(0); i < pTMgr.GetNumTileSets(); i++)
        m_comboTGrp->Append(pTMgr.GetTileSet(i).GetName());
    m_comboTGrp->SetSelection(0);
    UpdateTileList();
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::UpdatePaletteContents()
{
    int nSel = m_comboTGrp->GetSelection();
    if (nSel == wxNOT_FOUND)
        nSel = 0;               // Force first entry (if any)
    LoadTileNameList();
    if (value_preserving_cast<size_t>(nSel) < m_comboTGrp->GetCount())
        m_comboTGrp->SetSelection(nSel);
    UpdateTileList();
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::UpdateTileList()
{
    const CTileManager& pTMgr = m_pDoc.GetTileManager();

    int nSel = m_comboTGrp->GetSelection();
    if (nSel == wxNOT_FOUND)
    {
        m_listTile->SetItemMap(NULL);
        return;
    }
    const std::vector<TileID>& pPieceTbl = pTMgr.GetTileSet(value_preserving_cast<size_t>(nSel)).GetTileIDTable();
    m_listTile->SetItemMap(&pPieceTbl);
}

wxSize CTilePalette::DoGetBestClientSize() const
{
    wxSize choiceBestSize = m_comboTGrp->GetBestSize();
    wxSize tilesBestSize = m_listTile->GetBestSize();
    wxSize bestClientSize(std::max(choiceBestSize.x, tilesBestSize.x),
                            m_listTile->GetPosition().y + tilesBestSize.y);
    // limit best size to no more than 1/4 of screen dims
    bestClientSize.x = std::min(bestClientSize.x, wxSystemSettings::GetMetric(wxSYS_SCREEN_X, this)/4);
    bestClientSize.y = std::min(bestClientSize.y, wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, this)/4);
    return bestClientSize;
}

/////////////////////////////////////////////////////////////////////////////
// CTilePalette message handlers

void CTilePalette::OnTileNameCbnSelchange(wxCommandEvent& /*event*/)
{
    UpdateTileList();
}

void CTilePalette::OnGetDragSize(GetDragSizeEvent& event)
{
    TileID tid = GetCurrentTileID();
    CTile tile = m_pDoc.GetTileManager().GetTile(tid, fullScale);
    CSize size = tile.GetSize();
    event.SetSize(wxSize(size.cx, size.cy));
}

#if 0
BOOL CTilePalette::OnHelpInfo(HELPINFO* pHelpInfo)
{
    GetApp()->DoHelpTopic("gm-ref-pal-tile.htm");
    return TRUE;
}
#endif

