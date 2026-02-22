// PalTray.cpp : implementation file
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "Gp.h"
#include    "GamDoc.h"
#include    "GMisc.h"
#include    "FrmMain.h"
#include    "ResTbl.h"
#include    "Trays.h"
#include    "Player.h"
#include    "SelOPlay.h"
#include    "WinState.h"
#include    "DlgEdtEl.h"

#include    "Paltray.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

wxIMPLEMENT_CLASS(CTrayPalette, wxPanel)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

#define     ID_TIP_MENU         1

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CTrayPalette, wxPanel)
#if 0
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_WINDOWPOSCHANGING()
#endif
    EVT_CHOICE(XRCID("m_comboYGrp"), OnTrayNameCbnSelchange)
    EVT_LISTBOX_DCLICK(XRCID("m_listTray"), OnTrayListDoubleClick)
    EVT_DRAGDROP(OnDragItem)
    EVT_OVERRIDE_SELECTED_ITEM_LIST(OnOverrideSelectedItemList)
    EVT_GET_DRAG_SIZE(OnGetDragSize)
    EVT_CONTEXT_MENU(OnContextMenu)
    EVT_MENU(XRCID("ID_PTRAY_SHUFFLE"), OnPieceTrayShuffle)
    EVT_UPDATE_UI(XRCID("ID_PTRAY_SHUFFLE"), OnUpdatePieceTrayShuffle)
    EVT_BUTTON(XRCID("m_bpMenuBtn"), OnMenuButton)
#if 0
    ON_COMMAND(ID_PTRAY_SHUFFLE_SELECTED, OnPieceTrayShuffleSelected)
    ON_UPDATE_COMMAND_UI(ID_PTRAY_SHUFFLE_SELECTED, OnUpdatePieceTrayShuffleSelected)
    ON_COMMAND(ID_EDIT_ELEMENT_TEXT, OnEditElementText)
    ON_UPDATE_COMMAND_UI(ID_EDIT_ELEMENT_TEXT, OnUpdateEditElementText)
    ON_COMMAND_EX(ID_ACT_TURNOVER, OnActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_PREV, OnActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_RANDOM, OnActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER, OnUpdateActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_PREV, OnUpdateActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_RANDOM, OnUpdateActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_ALL, OnActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_ALL_PREV, OnActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_ALL_RANDOM, OnActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_ALL, OnUpdateActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_ALL_PREV, OnUpdateActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_ALL_RANDOM, OnUpdateActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_SELECT, OnActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_SELECT, OnUpdateActTurnOver)
    ON_COMMAND(ID_PTRAY_ABOUT, OnPieceTrayAbout)
    ON_UPDATE_COMMAND_UI(ID_PTRAY_ABOUT, OnUpdatePieceTrayAbout)
    ON_WM_HELPINFO()
    ON_WM_MOUSEMOVE()
#endif
    EVT_WINSTATE_RESTORE(OnMessageRestoreWinState)
    EVT_COMMAND(wxID_ANY, WM_PALETTE_HIDE_WX, OnPaletteHide)
#if 0
    ON_WM_INITMENUPOPUP()
#endif
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CTrayPaletteContainer, CWnd)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrayPalette

CTrayPalette::CTrayPalette(CTrayPaletteContainer& container, CGamDoc& pDoc, UINT palID) :
    m_pContainer(&container),
    m_pDoc(&pDoc),
    m_bpMenuBtn(nullptr),
    m_comboYGrp(nullptr),
    m_listTray(nullptr)
{
    m_dummyArray.push_back(PieceID(0));

    m_bStateVarsArmed = FALSE;
    m_nComboHeight = 0;
    m_pDockingFrame = NULL;
    SetPaletteID(palID);
}

BOOL CTrayPalette::Create(/*wxWindow & pOwnerWnd, DWORD dwStyle, UINT nID*/)
{
    LoadMenuButtonBitmap();

    if (!CB::XrcLoad(*this, *m_pContainer, "CTrayPalette"))
    {
        return false;
    }
    m_bpMenuBtn = XRCCTRL(*this, "m_bpMenuBtn", wxBitmapButton);
    m_bpMenuBtn->SetBitmap(m_bmpMenuBtn);
    m_comboYGrp = XRCCTRL(*this, "m_comboYGrp", wxChoice);
    m_listTray = XRCCTRL(*this, "m_listTray", CTrayListBoxWx);
    (*m_pContainer)->Layout();

    m_listTray->Init(*m_pDoc);
    m_listTray->EnableDrag();
    m_listTray->EnableSelfDrop();
    m_listTray->EnableDropScroll();
    m_listTray->SetTrayContentVisibility(trayVizAllSides);

    EnsureTooltipExistance();

    UpdatePaletteContents();
    // Queue up a message to finish up state restore.
    QueueEvent(WinStateRestoreEvent().Clone());
    return TRUE;
}

void CTrayPaletteContainer::SetDockingFrame(CDockTrayPalette* pDockingFrame)
{
    m_pDockingFrame = pDockingFrame;
    SetParent(pDockingFrame);
}

#if 0
int CTrayPalette::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    m_rctMenuBtn.left = 0;
    m_rctMenuBtn.top = 0;
    m_rctMenuBtn.right = m_rctMenuBtn.left + m_sizeMenuBtn.cx + 4;

    CRect rctCombo;
    GetClientRect(&rctCombo);
    rctCombo.top = 0;
    rctCombo.left = m_rctMenuBtn.Width() - 1;
    rctCombo.bottom = 10 * g_res.tm8ssb.tmHeight;

    if (!m_comboYGrp.Create(
        WS_CHILD | WS_VSCROLL | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT,
        rctCombo, this, IDC_W_TRAYNAMECOMBO))
    {
        TRACE("Failed to create tray name combo-box\n");
        return -1;
    }
    m_comboYGrp.SetFont(CFont::FromHandle(g_res.h8ssb));
    m_comboYGrp.SetDroppedWidth(12 * g_res.tm8ssb.tmMaxCharWidth);

    m_comboYGrp.GetWindowRect(&rctCombo);   // Fetch result of create
    m_nComboHeight = rctCombo.Height();     // Save for later use
    m_rctMenuBtn.bottom = m_nComboHeight;

    CRect rctLBox;
    GetClientRect(&rctLBox);
    rctLBox.top = m_nComboHeight;

    if (!m_listTray.Create(
        WS_CHILD | WS_BORDER | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LBS_EXTENDEDSEL |
            LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
        rctLBox, this, IDC_W_TRAYLIST))
    {
        TRACE("Failed to create tray list box.\n");
        return -1;
    }
    m_listTray.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

    EnsureTooltipExistance();

    return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Because of the way MFC and Windows re-parents the tooltip control they
// can be destroyed even though the C++ class exists in the tool palette
// class. To get around this we will recreate the control if it doesn't
// exist. Mouse moves within the window will generally call this code.

BOOL CTrayPalette::EnsureTooltipExistance()
{
    // Add a tip to the menu button.
    m_toolTipMenu.SetBalloonMode(true);
    m_toolTipMenu.Enable(true);
    m_toolTipMenu.Add(*m_bpMenuBtn, CB::string::LoadString(IDS_TIP_CLICK_MENU));

    m_toolTipCombo.Enable(true);
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::DoEditSelectedPieceText()
{
    std::vector<size_t> nSelItem = m_listTray->GetSelections();
    wxASSERT(nSelItem.size() == size_t(1));

    PieceID pid = m_listTray->MapIndexToItem(nSelItem.front());

    m_pDoc->DoEditPieceText(pid);
}

/////////////////////////////////////////////////////////////////////////////

size_t CTrayPalette::GetSelectedTray() const
{
    int nSel = m_comboYGrp->GetSelection();
    if (nSel == wxNOT_FOUND)
        return Invalid_v<size_t>;
    return value_preserving_cast<size_t>(reinterpret_cast<uintptr_t>(m_comboYGrp->GetClientData(value_preserving_cast<unsigned>(nSel))));
}

int CTrayPalette::FindTrayIndex(size_t nTrayNum) const
{
    if (m_comboYGrp->GetCount() <= 0)
        return wxNOT_FOUND;
    // @@@@@ TRACE1("m_comboYGrp.GetCount() = %d\n",  m_comboYGrp.GetCount());
    for (unsigned nIdx = unsigned(0) ; nIdx < m_comboYGrp->GetCount() ; ++nIdx)
    {
        if (value_preserving_cast<size_t>(reinterpret_cast<uintptr_t>(m_comboYGrp->GetClientData(nIdx))) == nTrayNum)
            return value_preserving_cast<int>(nIdx);
    }
    // CAN HAPPEN! ASSERT(nIdx < m_comboYGrp.GetCount());
    return wxNOT_FOUND;
}

///////////////////////////////////////////////////////////////////////
// This method handles the custom message WM_WINSTATE_RESTORE. The
// message is posted during view initial update if the state of
// the windows should be restored.

void CTrayPalette::OnMessageRestoreWinState(WinStateRestoreEvent& /*event*/)
{
    UpdatePaletteContents();

    if (!m_bStateVarsArmed)
        return;

    m_comboYGrp->SetSelection(value_preserving_cast<int>(m_nComboIndex));

    UpdateTrayList();

    for (size_t i = size_t(0) ; i < m_tblListBoxSel.size() ; ++i)
        m_listTray->SetSelection(m_tblListBoxSel[i]);
    m_listTray->ScrollToRow(m_nListTopindex);

    m_bStateVarsArmed = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::OnPaletteHide(wxCommandEvent& /*event*/)
{
    GetMainFrame()->SendMessage(WM_COMMAND,
        (WPARAM)(m_nID));
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::DoMenu(wxPoint point)
{
    // remember clicked side in case of ID_ACT_TURNOVER_SELECT
    wxWindow* child = wxFindWindowAtPoint(point);
    wxASSERT(child == &*m_bpMenuBtn || child == &*m_listTray);
    if (child == &*m_listTray)
    {
        wxRect rect;
        wxPoint clientPoint = m_listTray->ScreenToClient(point);
        /* KLUDGE:  OnGetHitItemCodeAtPoint is public in base
            CGrafixListBox, but protected in derived
            CTrayListBox.  Why? */
        menuGameElement = static_cast<CGrafixListBoxWx&>(*m_listTray).OnGetHitItemCodeAtPoint(clientPoint, rect);
        // ASSERT(menuGameElement != Invalid_v<GameElement> --> menuGameElement.IsAPiece()
        wxASSERT(menuGameElement == Invalid_v<GameElement> ||
                menuGameElement.IsAPiece());
    }
    else
    {
        menuGameElement = Invalid_v<GameElement>;
    }

    std::unique_ptr<wxMenuBar> bar(wxXmlResource::Get()->LoadMenuBar("IDR_MENU_PLAYER_POPUPS"));
    if (bar)
    {
        int index = bar->FindMenu("5=PV_PIECE_TRAY");
        wxASSERT(index != wxNOT_FOUND);
        std::unique_ptr<wxMenu> popup(bar->Remove(value_preserving_cast<size_t>(index)));

        // Make sure we clean up even if exception is tossed.
        try
        {
            PopupMenu(&*popup, ScreenToClient(point));
        }
        catch (...)
        {
            wxASSERT(!"exception");
        }
    }
}

void CTrayPalette::OnContextMenu(wxContextMenuEvent& event)
{
    DoMenu(event.GetPosition());
}

void CTrayPalette::OnMenuButton(wxCommandEvent& /*event*/)
{
    // Use the list box as a guide of where to place the menu
    wxRect rct = m_listTray->GetScreenRect();
    DoMenu(rct.GetTopLeft());
}

/////////////////////////////////////////////////////////////////////////////
// This is called when this tray is dragging and dropping a list
// of items. It gives the tray the chance to mess with the list's
// contents prior to commiting it.

void CTrayPalette::OnOverrideSelectedItemList(OverrideSelectedItemListEvent& event)
{
    OverrideSelectedItemListEvent& oil = event;
    size_t nSel = GetSelectedTray();
    if (nSel == Invalid_v<size_t>)
        return;

    CTrayManager& pYMgr = m_pDoc->GetTrayManager();
    CTraySet& pYSet = pYMgr.GetTraySet(nSel);

    if (pYSet.IsRandomPiecePull() ||
        pYSet.IsRandomSidePull())
    {
        std::vector<PieceID>& pPceArray = oil.Vector<PieceID>();

        uint32_t nRandSeed = m_pDoc->GetRandomNumberSeed();

        std::vector<size_t> pnIndices;
        if (pYSet.IsRandomPiecePull())
        {
            pnIndices = AllocateAndCalcRandomIndexVector(pPceArray.size(),
                pYSet.GetPieceIDTable().size(), nRandSeed, &nRandSeed);
        }
        else
        {
            pnIndices.reserve(pPceArray.size());
            for (size_t i = size_t(0) ; i < pPceArray.size() ; ++i)
            {
                pnIndices.push_back(i);
            }
        }

        CPieceTable& pieceTable = m_pDoc->GetPieceTable();
        for (size_t i = size_t(0); i < pPceArray.size(); i++)
        {
            pPceArray.at(i) = pYSet.GetPieceIDTable().at(value_preserving_cast<size_t>(pnIndices[i]));
            if (pYSet.IsRandomSidePull())
            {
                size_t sides = pieceTable.GetSides(pPceArray[i]);
                if (sides >= size_t(2))
                {
                    int32_t side = CalcRandomNumberUsingSeed(0, value_preserving_cast<uint32_t>(sides),
                                                                        nRandSeed, &nRandSeed);
                    m_pDoc->InvertPlayingPieceInTray(pPceArray[i],
                                                    CPieceTable::fSelect,
                                                    value_preserving_cast<size_t>(side),
                                                    false,
                                                    true);
                }
            }
        }

        m_pDoc->SetRandomNumberSeed(nRandSeed);
    }

    return;
}

void CTrayPalette::OnGetDragSize(GetDragSizeEvent& event)
{
    size_t nSel = GetSelectedTray();
    if (nSel == Invalid_v<size_t>)
    {
        wxASSERT(!"bad tray");
        event.Skip();
        return;
    }
    CTrayManager& pYMgr = m_pDoc->GetTrayManager();
    CTraySet& pYSet = pYMgr.GetTraySet(nSel);

    std::vector<size_t> items;
    if (pYSet.IsRandomPiecePull() ||
        pYSet.IsRandomSidePull())
    {
        items.reserve(m_listTray->GetItemCount());
        for (size_t i = size_t(0) ; i < m_listTray->GetItemCount() ; ++i)
        {
            items.push_back(i);
        }
    }
    else
    {
        items = m_listTray->GetSelections();
    }

    // check all sides of all items
    CPieceTable& pieceTable = m_pDoc->GetPieceTable();
    CTileManager& tileMgr = m_pDoc->GetTileManager();
    PlayerMask player = m_pDoc->GetCurrentPlayerMask();
    wxSize retval(0, 0);
    for (size_t item : items)
    {
        PieceID pid = m_listTray->MapIndexToItem(item);
        std::vector<TileID> tids = pieceTable.GetInactiveTileIDs(pid, TRUE);
        tids.push_back(pieceTable.GetActiveTileID(pid, TRUE));
        for (TileID tid : tids)
        {
            wxASSERT(tid != nullTid);
            wxSize size = CB::Convert(tileMgr.GetTile(tid).GetSize());
            retval.x = CB::max(retval.x, size.x);
            retval.y = CB::max(retval.y, size.y);
        }
    }

    event.SetSize(retval);
    return;
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(m_comboYGrp->GetSelection()));
        ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(m_listTray->GetVisibleRowsBegin()));

        // Save the indexes of all the selected items.
        m_tblListBoxSel.clear();

        std::vector<size_t> selections = m_listTray->GetSelections();
        m_tblListBoxSel.reserve(selections.size());
        for (size_t i = size_t(0); i < selections.size(); ++i)
        {
            m_tblListBoxSel.push_back(value_preserving_cast<int>(selections[i]));
        }
        ar << m_tblListBoxSel;
    }
    else
    {
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))       // V2.90
        {
            uint32_t dwTmp;
            ar >> dwTmp; m_nComboIndex = dwTmp;
            ar >> dwTmp; m_nListTopindex = dwTmp;
            ar >> m_tblListBoxSel;
            m_bStateVarsArmed = TRUE;           // Inform Create() data is good
        }
        else if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))  // V2.0
        {
            uint32_t dwTmp;
            ar >> dwTmp; m_nComboIndex = dwTmp;
            ar >> dwTmp; m_nListTopindex = dwTmp;
            ar >> m_tblListBoxSel;
            CWinPlacement wndSink;
            ar >> wndSink;                      // Eat this puppy
            m_bStateVarsArmed = TRUE;           // Inform Create() data is good
        }
        else                                                        // Pre 2.0
        {
            uint16_t sTmp;     // Eat the old data and go with the default values
            ar >> sTmp;
            ar >> sTmp;
            ar >> sTmp;
            ar >> sTmp;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::LoadTrayNameList()
{
    CTrayManager& pYMgr = m_pDoc->GetTrayManager();

    m_comboYGrp->Clear();
    for (size_t i = size_t(0); i < pYMgr.GetNumTraySets(); i++)
    {
        CTraySet& pYSet = pYMgr.GetTraySet(i);
        CB::string strTrayName = pYSet.GetName();
        if (pYSet.IsOwned())
        {
            CB::string strOwner = m_pDoc->GetPlayerManager()->GetPlayerUsingMask(
                pYSet.GetOwnerMask()).m_strName;
            strTrayName += " - " + strOwner;
        }
        int nIdx = m_comboYGrp->Append(strTrayName);
        m_comboYGrp->SetClientData(value_preserving_cast<unsigned>(nIdx), reinterpret_cast<void*>(value_preserving_cast<uintptr_t>(i)));    // Store the tray index in the data item
    }
    m_comboYGrp->SetSelection(0);
    UpdateTrayList();
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::UpdatePaletteContents(const CTraySet* pTray)
{
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t> && pTray != NULL)
    {
        CTrayManager& pYMgr = m_pDoc->GetTrayManager();
        if (&pYMgr.GetTraySet(nSel) != pTray)
            return;             // Not visible in palette at present.
    }
    if (nSel == Invalid_v<size_t>)
        nSel = 0;               // Force first entry (if any)
    if (pTray == NULL)
    {
        LoadTrayNameList();
        int nComboIndex = FindTrayIndex(nSel);
        if (nComboIndex == wxNOT_FOUND)
            nComboIndex = 0;
        m_comboYGrp->SetSelection(nComboIndex);
    }
    UpdateTrayList();
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::ShowTrayIndex(size_t nGroup, size_t nPos)
{
    size_t nSel = GetSelectedTray();
    if (nSel != nGroup)
    {
        m_comboYGrp->SetSelection(FindTrayIndex(nGroup));
        UpdateTrayList();
    }
    m_listTray->ShowListIndex(nPos);
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::DeselectAll()
{
    wxASSERT(m_listTray);
    if (m_listTray)
        m_listTray->DeselectAll();
}

void CTrayPalette::SelectTrayPiece(size_t nGroup, PieceID pid,
    const CB::string* pszNotificationTip)
{
    size_t nSel = GetSelectedTray();
    if (nSel != nGroup)
    {
        m_comboYGrp->SetSelection(FindTrayIndex(nGroup));
        UpdateTrayList();
    }
    size_t nItem = m_listTray->SelectTrayPiece(pid);
    if (pszNotificationTip != NULL)
    {
        m_listTray->SetNotificationTip(nItem, *pszNotificationTip);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::UpdateTrayList()
{
    CTrayManager& pYMgr = m_pDoc->GetTrayManager();

    m_toolTipCombo.Delete(*m_comboYGrp);   // Always delete current tool

    size_t nSel = GetSelectedTray();
    if (nSel == Invalid_v<size_t>)
        return;

    // Get the name from the combo box since it has all the ownership
    // information added.
    CB::string strTrayName = m_comboYGrp->GetString(m_comboYGrp->GetSelection());

    m_toolTipCombo.Add(*m_comboYGrp, strTrayName, CB::ToolTip::CENTER);

    CTraySet& pYSet = pYMgr.GetTraySet(nSel);
    const std::vector<PieceID>* pPieceTbl = &pYSet.GetPieceIDTable();

    CB::string str = "";
    m_listTray->EnableDrag(TRUE);
    m_listTray->EnableSelfDrop(TRUE);
    m_listTray->SetTipsAllowed(TRUE);

    TrayViz eViz = pYSet.GetTrayContentVisibility();

    // If a tray is owned and the current player is the owner,
    // then the two sided view is forced. Otherwise the default
    // visibility is used. Also, if in scenario mode and if
    // a tray is owned it to will be fully visible.

    if (pYSet.IsOwned() && (m_pDoc->IsScenario() ||
         pYSet.IsOwnedBy(m_pDoc->GetCurrentPlayerMask()) &&
        !pYSet.IsEnforcingVisibilityForOwnerToo()))
    {
        eViz = trayVizAllSides;              // Override visibility
    }
    else if (eViz == trayVizEachGeneric)
    {
        if (pYSet.IsRandomPiecePull())
        {
            str = CB::string::LoadString(IDS_TRAY_RANDHIDDEN);
            m_listTray->EnableSelfDrop(FALSE);
        }
        else
            str = CB::string::LoadString(IDS_TRAY_HIDDEN);
    }
    else if (eViz == trayVizNone)
    {
        if (pYSet.IsRandomPiecePull())
        {
            str = CB::string::LoadString(IDS_TRAY_ALLRANDHIDDEN);
            m_listTray->EnableSelfDrop(FALSE);
        }
        else
            str = CB::string::LoadString(IDS_TRAY_ALLHIDDEN);
        // Set the first (and only) element in the dummy array
        // to the first element in actual tray.
        m_dummyArray.clear();
        if (!pPieceTbl->empty())
            m_dummyArray.push_back(pPieceTbl->front());
        pPieceTbl = &m_dummyArray;
    }
    if (!m_pDoc->IsScenario() && pYSet.IsOwnedButNotByCurrentPlayer(*m_pDoc))
        m_listTray->SetTipsAllowed(FALSE);

    if (!m_pDoc->IsScenario() && pYSet.IsOwned() &&
        !pYSet.IsOwnedBy(m_pDoc->GetCurrentPlayerMask()) &&
        !pYSet.IsNonOwnerAccessAllowed())
    {
        m_listTray->EnableDrag(FALSE);
        m_listTray->EnableSelfDrop(FALSE);
    }
    m_listTray->SetTrayContentVisibility(eViz, str);

    m_listTray->SetItemMap(pPieceTbl);
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::OnDragItem(DragDropEvent& event)
{
    if (event.GetProcessId() != wxGetProcessId())
    {
        return;
    }
    if (m_pDoc->IsPlaying())
        return;                       // Drags not supported during play

    const DragInfoWx& pdi = event.GetDragInfo();

    if (pdi.GetDragType() != DRAG_PIECE && pdi.GetDragType() != DRAG_SELECTLIST &&
        pdi.GetDragType() != DRAG_PIECELIST)
        return;                       // Only piece drops allowed

    if (pdi.GetDragType() == DRAG_PIECE && pdi.GetSubInfo<DRAG_PIECE>().m_gamDoc != &*m_pDoc ||
        pdi.GetDragType() == DRAG_SELECTLIST && pdi.GetSubInfo<DRAG_SELECTLIST>().m_gamDoc != &*m_pDoc ||
        pdi.GetDragType() == DRAG_PIECELIST && pdi.GetSubInfo<DRAG_PIECELIST>().m_gamDoc != &*m_pDoc)
        return;                       // Only pieces from our document.

    if (pdi.GetDragType() == DRAG_SELECTLIST)
    {
        if (!pdi.GetSubInfo<DRAG_SELECTLIST>().m_selectList->HasPieces())
            return;                   // Only piece drops allowed
    }

    // no size restriction

    size_t nGrpSel = GetSelectedTray();
    if (nGrpSel == Invalid_v<size_t>)
        return;                       // No tray to drop on

    if (pdi.m_phase == PhaseDrag::Over)
    {
        event.SetCursor(pdi.m_hcsrSuggest);
        return;
    }
    else if (pdi.m_phase == PhaseDrag::Drop)
    {
        CTrayManager& pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYGrp = pYMgr.GetTraySet(nGrpSel);

        // Force selection of item under the mouse
        m_listTray->SetSelFromPoint(pdi.m_point);
        int nSel = m_listTray->GetSelectedCount() == size_t(0) ? wxNOT_FOUND : value_preserving_cast<int>(m_listTray->GetSelections().front());

        if (!m_pDoc->IsScenario() && pYGrp.IsOwnedButNotByCurrentPlayer(*m_pDoc) &&
                pYGrp.GetTrayContentVisibility() == trayVizNone)
            nSel = wxNOT_FOUND;  // Always append pieces when dropping on single line view

        if (nSel != wxNOT_FOUND)
        {
            // Check if the mouse is above or below the half point.
            // If above, insert before. If below, insert after.
            wxRect rct = m_listTray->GetItemRect(value_preserving_cast<size_t>(nSel));
            if (pdi.m_point.y > GetMidRect(rct).y)
                nSel++;
        }
        size_t dropCount;
        if (pdi.GetDragType() == DRAG_PIECE)
        {
            wxASSERT(!"untested code");
            dropCount = size_t(1);
            m_pDoc->AssignNewMoveGroup();
            m_pDoc->PlacePieceInTray(pdi.GetSubInfo<DRAG_PIECE>().m_pieceID, pYGrp, nSel == wxNOT_FOUND ? Invalid_v<size_t> : value_preserving_cast<size_t>(nSel));
            // Select the last piece that was inserted
            nSel = value_preserving_cast<int>(pYGrp.GetPieceIDIndex(pdi.GetSubInfo<DRAG_PIECE>().m_pieceID));
        }
        else if (pdi.GetDragType() == DRAG_PIECELIST)
        {
            m_pDoc->AssignNewMoveGroup();
            const std::vector<PieceID>& pieceIDList = CheckedDeref(pdi.GetSubInfo<DRAG_PIECELIST>().m_pieceIDList);
            dropCount = pieceIDList.size();
            size_t temp = m_pDoc->PlacePieceListInTray(pieceIDList,
                pYGrp, nSel == wxNOT_FOUND ? Invalid_v<size_t> : value_preserving_cast<size_t>(nSel));
            nSel = temp == Invalid_v<size_t> ? wxNOT_FOUND : value_preserving_cast<int>(temp);
        }
        else        // DRAG_SELECTLIST
        {
            wxASSERT(pdi.GetDragType() == DRAG_SELECTLIST);
            std::vector<RefPtr<CDrawObj>> m_listPtr;
            CSelList* pSLst = pdi.GetSubInfo<DRAG_SELECTLIST>().m_selectList;
            pSLst->LoadTableWithObjectPtrs(m_listPtr, CSelList::otAll, FALSE);
            pSLst->PurgeList(FALSE);
            dropCount = m_listPtr.size();
            m_pDoc->AssignNewMoveGroup();
            size_t temp = m_pDoc->PlaceObjectTableInTray(m_listPtr,
                pYGrp, nSel == wxNOT_FOUND ? Invalid_v<size_t> : value_preserving_cast<size_t>(nSel));
            nSel = temp == Invalid_v<size_t> ? wxNOT_FOUND : value_preserving_cast<int>(temp);
            m_pDoc->UpdateAllViews(NULL, HINT_UPDATESELECTLIST);
        }
        if (nSel != wxNOT_FOUND)
        {
            for (size_t i = size_t(0) ; i < dropCount ; ++i)
            {
                m_listTray->SetSelection(nSel - value_preserving_cast<int>(i));
            }
        }

        m_listTray->SetSelection(nSel);

        if (nSel != wxNOT_FOUND)
        {
            // If the selection is out of view, force it into view.
            wxRect rctLBoxClient = m_listTray->GetClientRect();
            wxRect rct = m_listTray->GetItemRect(value_preserving_cast<size_t>(nSel));
            if (!rct.Intersects(rctLBoxClient))
                m_listTray->ScrollToRow(value_preserving_cast<size_t>(nSel));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Load the menu icon image and fill upper-right transparent area with
// current button face color.

void CTrayPalette::LoadMenuButtonBitmap()
{
    wxBitmap hBMap(std::format("#{}", IDB_MENU_ICON),
        wxBITMAP_TYPE_BMP_RESOURCE);
    wxASSERT(hBMap.IsOk());
    m_bmpMenuBtn = hBMap;

    m_sizeMenuBtn.x = m_bmpMenuBtn.GetWidth();
    m_sizeMenuBtn.y = m_bmpMenuBtn.GetHeight();

    wxMemoryDC dc;
    dc.SelectObject(m_bmpMenuBtn);
    wxBrush brush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE), wxBRUSHSTYLE_SOLID);
    wxDCBrushChanger setBrush(dc, brush);
    dc.FloodFill(m_sizeMenuBtn.x - 1, 0, wxColour(0, 255, 255), wxFLOOD_SURFACE);
}

/////////////////////////////////////////////////////////////////////////////
// CTrayPalette message handlers

#if 0
BOOL CTrayPalette::OnEraseBkgnd(CDC* pDC)
{
    // Erase behind menu button only...
    pDC->IntersectClipRect(m_rctMenuBtn);
    CBrush brush;
    brush.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    pDC->FillRect(m_rctMenuBtn, &brush);

    CDC dc;
    dc.CreateCompatibleDC(pDC);
    CBitmap* prvBMap = dc.SelectObject(&m_bmpMenuBtn);
    int x = (m_rctMenuBtn.Width() - m_sizeMenuBtn.cx) / 2;
    int y = (m_rctMenuBtn.Height() - m_sizeMenuBtn.cy) / 2 - 1;
    pDC->BitBlt(x, y, m_sizeMenuBtn.cx, m_sizeMenuBtn.cy,
        &dc, 0, 0, SRCCOPY);
    dc.SelectObject(prvBMap);

    return TRUE;        // controls take care of erase
}

#if 0
LRESULT CTrayPalette::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST)
    {
        MSG msg;
        memset(&msg, 0, sizeof(MSG));
        msg.hwnd = m_hWnd;
        msg.message = message;
        msg.wParam = wParam;
        msg.lParam = lParam;
        if (m_toolTipMenu.m_hWnd != NULL)
            m_toolTipMenu.RelayEvent(&msg);
        if (m_toolTipCombo.m_hWnd != NULL)
            m_toolTipCombo.RelayEvent(&msg);
    }
    return CWnd::WindowProc(message, wParam, lParam);
}
#endif

void CTrayPalette::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    CRect rctCombo;

    if (m_comboYGrp.m_hWnd != NULL)
    {
        m_comboYGrp.GetWindowRect(&rctCombo);
        m_comboYGrp.MoveWindow(m_rctMenuBtn.Width() - 1, 0,
            cx - m_rctMenuBtn.Width(), 10 * m_nComboHeight, TRUE);
    }
    if (m_listTray.m_hWnd != NULL)
    {
        m_listTray.MoveWindow(0, m_nComboHeight, cx, cy - m_nComboHeight, TRUE);
    }
}

void CTrayPaletteContainer::PostNcDestroy()
{
    /* DO NOTHING - FRAME CLASS WOULD DELETE SELF! */
}

void CTrayPalette::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
    CWnd::OnWindowPosChanging(lpwndpos);

    // Need this code to avoid a detached combo list box.
    if (m_comboYGrp.m_hWnd != NULL)
        m_comboYGrp.ShowDropDown(FALSE);
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::OnTrayNameCbnSelchange(wxCommandEvent& /*event*/)
{
    UpdateTrayList();
}

void CTrayPalette::OnTrayListDoubleClick(wxCommandEvent& /*event*/)
{
    if (!m_listTray->IsShowingTileImages())
        return;

    int nIndex = m_listTray->GetCurrent();
    if (nIndex == wxNOT_FOUND)
        return;

    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager& pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr.GetTraySet(nSel);
        if (pYSet.IsOwned() &&
            !pYSet.IsOwnedBy(m_pDoc->GetCurrentPlayerMask()))
            return;
    }

    PieceID pid = m_listTray->MapIndexToItem(value_preserving_cast<size_t>(nIndex));
    m_pDoc->DoEditPieceText(pid);
}

void CTrayPalette::OnPieceTrayShuffle(wxCommandEvent& /*event*/)
{
    // Generate a shuffled index vector
    uint32_t nRandSeed = m_pDoc->GetRandomNumberSeed();
    size_t nNumIndices = m_listTray->GetItemCount();
    std::vector<size_t> pnIndices = AllocateAndCalcRandomIndexVector(nNumIndices,
        nNumIndices, nRandSeed, &nRandSeed);
    m_pDoc->SetRandomNumberSeed(nRandSeed);

    // Build table of shuffled pieces
    std::vector<PieceID> tblPids;
    tblPids.reserve(value_preserving_cast<size_t>(nNumIndices));
    for (size_t i = size_t(0) ; i < nNumIndices ; ++i)
        tblPids.push_back(m_listTray->MapIndexToItem(pnIndices[i]));

    m_pDoc->AssignNewMoveGroup();

    size_t nSel = GetSelectedTray();
    CTrayManager& pYMgr = m_pDoc->GetTrayManager();

    if (m_pDoc->IsRecording())
    {
        CB::string strMsg = CB::string::Format(IDS_TIP_TRAY_SHUFFLED, tblPids.size());
        m_pDoc->RecordEventMessage(strMsg, nSel, tblPids.front());
    }

    m_pDoc->PlacePieceListInTray(tblPids, pYMgr.GetTraySet(nSel), 0);
}

void CTrayPalette::OnUpdatePieceTrayShuffle(wxUpdateUIEvent& pCmdUI)
{
    BOOL bNoOwnerRestrictions = TRUE;
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager& pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr.GetTraySet(nSel);
        bNoOwnerRestrictions = !(pYSet.IsOwnedButNotByCurrentPlayer(*m_pDoc) &&
            !pYSet.IsNonOwnerAccessAllowed());
    }
    pCmdUI.Enable((m_pDoc->IsScenario() || bNoOwnerRestrictions) &&
        m_listTray->GetItemCount() > 1);
}

#if 0
void CTrayPalette::OnPieceTrayShuffleSelected()
{
    size_t nNumSelected = value_preserving_cast<size_t>(m_listTray.GetSelCount());
    std::vector<INT> tblListSel(nNumSelected);
    m_listTray.GetSelItems(value_preserving_cast<int>(nNumSelected), tblListSel.data());

    INT nTopSel = tblListSel[size_t(0)];        // This is where the shuffle is inserted

    // Generate a shuffled index vector for the number of selected items
    uint32_t nRandSeed = m_pDoc->GetRandomNumberSeed();
    ASSERT(nNumSelected == tblListSel.size());
    std::vector<size_t> pnIndices = AllocateAndCalcRandomIndexVector(nNumSelected,
        nNumSelected, nRandSeed, &nRandSeed);
    m_pDoc->SetRandomNumberSeed(nRandSeed);

    // Build table of shuffled pieces
    std::vector<PieceID> tblPids;
    tblPids.reserve(value_preserving_cast<size_t>(nNumSelected));
    for (size_t i = size_t(0) ; i < nNumSelected ; ++i)
        tblPids.push_back(m_listTray.MapIndexToItem(value_preserving_cast<size_t>(tblListSel[pnIndices[i]])));

    m_pDoc->AssignNewMoveGroup();

    size_t nSel = GetSelectedTray();
    CTrayManager& pYMgr = m_pDoc->GetTrayManager();

    if (m_pDoc->IsRecording())
    {
        CB::string strMsg = CB::string::Format(IDS_TIP_TRAY_SHUFFLED, tblPids.size());
        m_pDoc->RecordEventMessage(strMsg, nSel, tblPids.front());
    }

    m_pDoc->PlacePieceListInTray(tblPids, pYMgr.GetTraySet(nSel), value_preserving_cast<size_t>(nTopSel));
}

void CTrayPalette::OnUpdatePieceTrayShuffleSelected(CCmdUI* pCmdUI)
{
    BOOL bNoOwnerRestrictions = TRUE;
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager& pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr.GetTraySet(nSel);
        bNoOwnerRestrictions = !(pYSet.IsOwnedButNotByCurrentPlayer(*m_pDoc) &&
            !pYSet.IsNonOwnerAccessAllowed());
    }

    pCmdUI->Enable((m_pDoc->IsScenario() || bNoOwnerRestrictions) &&
        m_listTray.GetSelCount() > 1 && m_listTray.IsShowingTileImages());
}

void CTrayPalette::OnEditElementText()
{
    DoEditSelectedPieceText();
}

void CTrayPalette::OnUpdateEditElementText(CCmdUI* pCmdUI)
{
    BOOL bNoOwnerRestrictions = TRUE;
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager& pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr.GetTraySet(nSel);
        bNoOwnerRestrictions = !pYSet.IsOwnedButNotByCurrentPlayer(*m_pDoc);
    }
    pCmdUI->Enable((m_pDoc->IsScenario() || bNoOwnerRestrictions) &&
        m_listTray.GetSelCount() == 1 && m_listTray.IsShowingTileImages());
}

BOOL CTrayPalette::OnActTurnOver(UINT id)
{
    std::vector<int> tblListSel;
    int nNumSelected = m_listTray.GetSelCount();
    if (nNumSelected)
    {
        tblListSel.resize(value_preserving_cast<size_t>(nNumSelected));
        m_listTray.GetSelItems(nNumSelected, &tblListSel.front());
    }

    std::vector<int> tblListSubjects;
    std::vector<int>* chosen;

    switch (id)
    {
        case ID_ACT_TURNOVER:
        case ID_ACT_TURNOVER_PREV:
        case ID_ACT_TURNOVER_RANDOM:
            // operate on selected pieces
            chosen = &tblListSel;
            break;
        case ID_ACT_TURNOVER_ALL:
        case ID_ACT_TURNOVER_ALL_PREV:
        case ID_ACT_TURNOVER_ALL_RANDOM:
            // operate on all pieces
            tblListSubjects.resize(value_preserving_cast<size_t>(m_listTray.GetCount()));
            for (int i = 0; i < m_listTray.GetCount(); i++)
            {
                tblListSubjects[value_preserving_cast<size_t>(i)] = i;
            }
            chosen = &tblListSubjects;
            break;
        case ID_ACT_TURNOVER_SELECT:
            // operate on clicked side
            tblListSubjects.push_back(value_preserving_cast<int>(m_listTray.MapItemToIndex(static_cast<PieceID>(menuGameElement))));
            chosen = &tblListSubjects;
            break;
        default:
            AfxThrowInvalidArgException();
    }

    CPieceTable::Flip flip;
    switch (id)
    {
        case ID_ACT_TURNOVER:
        case ID_ACT_TURNOVER_ALL:
            flip = CPieceTable::fNext;
            break;
        case ID_ACT_TURNOVER_PREV:
        case ID_ACT_TURNOVER_ALL_PREV:
            flip = CPieceTable::fPrev;
            break;
        case ID_ACT_TURNOVER_RANDOM:
        case ID_ACT_TURNOVER_ALL_RANDOM:
            flip = CPieceTable::fRandom;
            break;
        case ID_ACT_TURNOVER_SELECT:
            flip = CPieceTable::fSelect;
            break;
        default:
            AfxThrowInvalidArgException();
    }

    m_pDoc->AssignNewMoveGroup();

    size_t nSel = GetSelectedTray();
    if (m_pDoc->IsRecording() && flip == CPieceTable::fRandom)
    {
        CB::string strMsg = CB::string::Format(IDS_TIP_TRAY_FLIPPED_RANDOM, chosen->size());
        m_pDoc->RecordEventMessage(strMsg, nSel, m_listTray.MapIndexToItem(value_preserving_cast<size_t>(chosen->front())));
    }

    for (size_t i = size_t(0) ; i < chosen->size() ; ++i)
    {
        PieceID pid = m_listTray.MapIndexToItem(value_preserving_cast<size_t>((*chosen)[i]));
        size_t side;
        if (flip == CPieceTable::fSelect)
        {
            side = menuGameElement.GetSide();
        }
        else
        {
            side = Invalid_v<size_t>;
        }
        m_pDoc->InvertPlayingPieceInTray(pid, flip, side, false, false);
    }
    CTrayManager& pYMgr = m_pDoc->GetTrayManager();
    CGamDocHint hint;
    hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = &pYMgr.GetTraySet(nSel);
    m_pDoc->UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);

    /* flipping pieces shouldn't change tray content,
        so restore selections */
    for (size_t i = size_t(0) ; i < tblListSel.size() ; ++i)
    {
        m_listTray.SetSel(tblListSel[i], true);
    }

    return true;
}

void CTrayPalette::OnUpdateActTurnOver(CCmdUI* pCmdUI)
{
    bool eligible;
    switch (pCmdUI->m_nID)
    {
        case ID_ACT_TURNOVER:
        case ID_ACT_TURNOVER_PREV:
        case ID_ACT_TURNOVER_RANDOM:
            eligible = m_listTray.GetSelCount() > 0;
            break;
        case ID_ACT_TURNOVER_ALL:
        case ID_ACT_TURNOVER_ALL_PREV:
        case ID_ACT_TURNOVER_ALL_RANDOM:
            eligible = m_listTray.GetCount() > 0;
            break;
        case ID_ACT_TURNOVER_SELECT:
            eligible = menuGameElement != Invalid_v<GameElement>;
            break;
        default:
            AfxThrowInvalidArgException();
    }

    BOOL bNoOwnerRestrictions = TRUE;
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager& pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr.GetTraySet(nSel);
        bNoOwnerRestrictions = !pYSet.IsOwnedButNotByCurrentPlayer(*m_pDoc);
    }

    bool enable = (m_pDoc->IsScenario() || bNoOwnerRestrictions) &&
                    eligible &&
                    m_listTray.IsShowingTileImages();
    pCmdUI->Enable(enable);
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // Need to handle menu that the submenu is connected to.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | (enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
    }
}

void CTrayPalette::OnPieceTrayAbout()
{
    size_t nSel = GetSelectedTray();
    if (nSel == Invalid_v<size_t>)
        return;
    CTrayManager& pYMgr = m_pDoc->GetTrayManager();
    CTraySet& pYSet = pYMgr.GetTraySet(nSel);
    CB::string strMsg = CB::string::Format(IDS_MSG_TRAY_INFO, pYSet.GetName());
    CB::string strTmp;

    switch (pYSet.GetTrayContentVisibility())
    {
        case trayVizAllSides:
            strTmp = CB::string::LoadString(IDS_MSG_TVIZ_ALL_SIDE);
            break;
        case trayVizOneSide:
            strTmp = CB::string::LoadString(IDS_MSG_TVIZ_ONE_SIDE);
            break;
        case trayVizEachGeneric:
            strTmp = CB::string::LoadString(IDS_MSG_TVIZ_HIDDEN);
            break;
        case trayVizNone:
            strTmp = CB::string::LoadString(IDS_MSG_TVIZ_ALL_HIDDEN);
            break;
        default:
            ASSERT(FALSE);
            strTmp = CB::string::LoadString(IDS_ERR_TRAY_VIZ);
    }

    strMsg += strTmp + '\n';

    if (pYSet.IsRandomPiecePull())
    {
        strTmp = CB::string::LoadString(IDS_MSG_RANDOM_PULL);
        strMsg += strTmp + '\n';
    }

    if (pYSet.IsRandomSidePull())
    {
        strTmp = CB::string::LoadString(IDS_MSG_RANDOM_SIDE_PULL);
        strMsg += strTmp + '\n';
    }

    if (pYSet.IsOwned())
    {
        strTmp = CB::string::Format(IDS_MSG_OWNED_BY, m_pDoc->GetPlayerManager()->
            GetPlayerUsingMask(pYSet.GetOwnerMask()).m_strName);
    }
    else
        strTmp = CB::string::LoadString(IDS_MSG_NOT_OWNED);

    strMsg += strTmp;
    if (pYSet.IsOwned())
    {
        if (pYSet.IsNonOwnerAccessAllowed())
            strTmp = CB::string::LoadString(IDS_MSG_NONOWN_ALLOWED);
        else
            strTmp = CB::string::LoadString(IDS_MSG_NONOWN_NOT_ALLOWED);
        strMsg += '\n' + strTmp;

        if (pYSet.IsEnforcingVisibilityForOwnerToo())
            strTmp = CB::string::LoadString(IDS_MSG_TVIZ_OWNER_TOO);
        else
            strTmp = CB::string::LoadString(IDS_MSG_TVIZ_OWNER_FULL);
        strMsg += '\n' + strTmp;
    }
    AfxMessageBox(strMsg, MB_ICONINFORMATION);
}

void CTrayPalette::OnUpdatePieceTrayAbout(CCmdUI* pCmdUI)
{
    int nSel = m_comboYGrp.GetCurSel();
    pCmdUI->Enable(nSel >= 0);
}

BOOL CTrayPalette::OnHelpInfo(HELPINFO* pHelpInfo)
{
    GetApp()->DoHelpTopic("gp-ref-pal-tray.htm");
    return TRUE;
}

void CTrayPalette::OnMouseMove(UINT nFlags, CPoint point)
{
    if (EnsureTooltipExistance())
        UpdateTrayList();

    CWnd::OnMouseMove(nFlags, point);
}

void CTrayPalette::OnInitMenuPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu)
{
    // based on CFrameWnd::OnInitMenuPopup()
    ASSERT(!bSysMenu);

    CCmdUI state;
    state.m_pMenu = pMenu;
    ASSERT(state.m_pOther == NULL);
    ASSERT(state.m_pParentMenu == NULL);

    state.m_nIndexMax = pMenu->GetMenuItemCount();
    for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
        state.m_nIndex++)
    {
        state.m_nID = pMenu->GetMenuItemID(state.m_nIndex);
        if (state.m_nID == 0)
            continue; // menu separator or invalid cmd - ignore it

        ASSERT(state.m_pOther == NULL);
        ASSERT(state.m_pMenu != NULL);
        if (state.m_nID == (UINT)-1)
        {
            // possibly a popup menu, route to first item of that popup
            state.m_pSubMenu = pMenu->GetSubMenu(state.m_nIndex);
            if (state.m_pSubMenu == NULL ||
                (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
                state.m_nID == (UINT)-1)
            {
                continue;       // first item of popup can't be routed to
            }
            state.DoUpdate(this, FALSE);    // popups are never auto disabled
        }
        else
        {
            // normal menu item
            // Auto enable/disable if frame window has 'm_bAutoMenuEnable'
            //    set and command is _not_ a system command.
            state.m_pSubMenu = NULL;
            state.DoUpdate(this, true);
        }

        // adjust for menu deletions and additions
        UINT nCount = pMenu->GetMenuItemCount();
        if (nCount < state.m_nIndexMax)
        {
            state.m_nIndex -= (state.m_nIndexMax - nCount);
            while (state.m_nIndex < nCount &&
                pMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
            {
                state.m_nIndex++;
            }
        }
        state.m_nIndexMax = nCount;
    }
}
#endif

CTrayPaletteContainer::CTrayPaletteContainer(CGamDoc& pDoc, UINT palID) :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this)),
    child(new CTrayPalette(*this, pDoc, palID))
{
}

BOOL CTrayPaletteContainer::Create(CWnd& pOwnerWnd/*, DWORD dwStyle = 0, UINT nID = 0*/)
{
    DWORD dwStyle = WS_CHILD | WS_VISIBLE;
    if (!CWnd::Create(AfxRegisterWndClass(0), NULL, dwStyle,
        CRect(0, 0, 200, 100), &pOwnerWnd, 0))
    {
        TRACE("Failed to create Tray palette container window.\n");
        return FALSE;
    }

    return TRUE;
}

int CTrayPaletteContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    if (!child->Create())
    {
        TRACE("Failed to create Tray palette window.\n");
        return -1;
    }

    return 0;
}

// MFC puts the focus here, so move it to the useful window
void CTrayPaletteContainer::OnSetFocus(CWnd* pOldWnd)
{
    CWnd::OnSetFocus(pOldWnd);
    child->SetFocus();
}

void CTrayPaletteContainer::OnSize(UINT nType, int cx, int cy)
{
    child->SetSize(0, 0, cx, cy);
    return CWnd::OnSize(nType, cx, cy);
}
