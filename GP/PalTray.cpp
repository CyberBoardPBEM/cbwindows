// PalTray.cpp : implementation file
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

IMPLEMENT_DYNCREATE(CTrayPalette, CWnd)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

#define     ID_TIP_MENU         1

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTrayPalette, CWnd)
    //{{AFX_MSG_MAP(CTrayPalette)
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_WINDOWPOSCHANGING()
    ON_CBN_SELCHANGE(IDC_W_TRAYNAMECOMBO, OnTrayNameCbnSelchange)
    ON_LBN_DBLCLK(IDC_W_TRAYLIST, OnTrayListDoubleClick)
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragItem)
    ON_MESSAGE(WM_OVERRIDE_SELECTED_ITEM_LIST, OnOverrideSelectedItemList)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_PTRAY_SHUFFLE, OnPieceTrayShuffle)
    ON_UPDATE_COMMAND_UI(ID_PTRAY_SHUFFLE, OnUpdatePieceTrayShuffle)
    ON_WM_LBUTTONUP()
    ON_COMMAND(ID_PTRAY_SHUFFLE_SELECTED, OnPieceTrayShuffleSelected)
    ON_UPDATE_COMMAND_UI(ID_PTRAY_SHUFFLE_SELECTED, OnUpdatePieceTrayShuffleSelected)
    ON_COMMAND(ID_EDIT_ELEMENT_TEXT, OnEditElementText)
    ON_UPDATE_COMMAND_UI(ID_EDIT_ELEMENT_TEXT, OnUpdateEditElementText)
    ON_COMMAND(ID_ACT_TURNOVER, OnActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER, OnUpdateActTurnOver)
    ON_COMMAND(ID_ACT_TURNOVER_ALL, OnActTurnoverAllPieces)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_ALL, OnUpdateActTurnoverAllPieces)
    ON_COMMAND(ID_PTRAY_ABOUT, OnPieceTrayAbout)
    ON_UPDATE_COMMAND_UI(ID_PTRAY_ABOUT, OnUpdatePieceTrayAbout)
    ON_WM_HELPINFO()
    ON_WM_MOUSEMOVE()
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_WINSTATE_RESTORE, OnMessageRestoreWinState)
    ON_MESSAGE(WM_PALETTE_HIDE, OnPaletteHide)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrayPalette

CTrayPalette::CTrayPalette()
{
    m_pDoc = NULL;

    m_listTray.EnableDrag();
    m_listTray.EnableSelfDrop();
    m_listTray.EnableDropScroll();
    m_listTray.SetTrayContentVisibility(trayVizTwoSide);

    m_dummyArray.push_back(PieceID(0));

    m_bStateVarsArmed = FALSE;
    m_nComboHeight = 0;
    m_pDockingFrame = NULL;
}

BOOL CTrayPalette::Create(CWnd* pOwnerWnd, DWORD dwStyle, UINT nID)
{
    LoadMenuButtonBitmap();

    dwStyle |= WS_CHILD | WS_VISIBLE;
    if (!CWnd::Create(AfxRegisterWndClass(0), NULL, dwStyle,
        CRect(0, 0, 200, 100), pOwnerWnd, nID))
    {
        TRACE("Failed to create Tray palette window.\n");
        return FALSE;
    }

    UpdatePaletteContents();
    // Queue up a message to finish up state restore.
    PostMessage(WM_WINSTATE_RESTORE);
    return TRUE;
}

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
        WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | LBS_EXTENDEDSEL |
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

/////////////////////////////////////////////////////////////////////////////
// Because of the way MFC and Windows re-parents the tooltip control they
// can be destroyed even though the C++ class exists in the tool palette
// class. To get around this we will recreate the control if it doesn't
// exist. Mouse moves within the window will generally call this code.

BOOL CTrayPalette::EnsureTooltipExistance()
{
    if (m_toolTipMenu.m_hWnd == NULL)
    {
        // Add a tip to the menu icon click area.
        m_toolTipMenu.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX);
        m_toolTipMenu.AddTool(this, IDS_TIP_CLICK_MENU, m_rctMenuBtn, ID_TIP_MENU);

        m_toolTipCombo.Create(this, TTS_ALWAYSTIP | TTS_NOPREFIX);
        return TRUE;
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::DoEditSelectedPieceText(BOOL bEditTop)
{
    ASSERT(m_listTray.GetSelCount() == 1);
    int nSelItem;
    m_listTray.GetSelItems(1, &nSelItem);

    PieceID pid = m_listTray.MapIndexToItem(value_preserving_cast<size_t>(nSelItem));

    m_pDoc->DoEditPieceText(pid, bEditTop);
}

/////////////////////////////////////////////////////////////////////////////

size_t CTrayPalette::GetSelectedTray()
{
    int nSel = m_comboYGrp.GetCurSel();
    if (nSel < 0)
        return Invalid_v<size_t>;
    return value_preserving_cast<size_t>(m_comboYGrp.GetItemData(nSel));
}

int CTrayPalette::FindTrayIndex(size_t nTrayNum)
{
    if (m_comboYGrp.GetCount() <= 0)
        return -1;
    // @@@@@ TRACE1("m_comboYGrp.GetCount() = %d\n",  m_comboYGrp.GetCount());
    for (int nIdx = 0; nIdx < m_comboYGrp.GetCount(); nIdx++)
    {
        if (value_preserving_cast<size_t>(m_comboYGrp.GetItemData(nIdx)) == nTrayNum)
            return nIdx;
    }
    // CAN HAPPEN! ASSERT(nIdx < m_comboYGrp.GetCount());
    return -1;
}

///////////////////////////////////////////////////////////////////////
// This method handles the custom message WM_WINSTATE_RESTORE. The
// message is posted during view initial update if the state of
// the windows should be restored.

LRESULT CTrayPalette::OnMessageRestoreWinState(WPARAM, LPARAM)
{
    UpdatePaletteContents();

    if (!m_bStateVarsArmed)
        return (LRESULT)0;

    m_comboYGrp.SetCurSel(m_nComboIndex);

    UpdateTrayList();

    for (int i = 0; i < m_tblListBoxSel.GetSize(); i++)
        m_listTray.SetSel(m_tblListBoxSel[i]);
    m_listTray.SetTopIndex(m_nListTopindex);

    m_bStateVarsArmed = FALSE;
    return (LRESULT)0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CTrayPalette::OnPaletteHide(WPARAM, LPARAM)
{
    GetMainFrame()->SendMessage(WM_COMMAND,
        (WPARAM)(m_nID == 0 ? ID_VIEW_TRAYA : ID_VIEW_TRAYB));
    return (LRESULT)0;
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::OnContextMenu(CWnd* pWnd, CPoint point)
{
    CMenu bar;
    if (bar.LoadMenu(IDR_MENU_PLAYER_POPUPS))
    {
        CMenu& popup = *bar.GetSubMenu(MENU_PV_PIECE_TRAY);
        ASSERT(popup.m_hMenu != NULL);

        // Make sure we clean up even if exception is tossed.
        TRY
        {
            popup.TrackPopupMenu(TPM_RIGHTBUTTON,
                point.x, point.y, this); // Route commands through tray window
        }
        END_TRY
    }
}

/////////////////////////////////////////////////////////////////////////////
// This is called when this tray is dragging and dropping a list
// of items. It gives the tray the chance to mess with the list's
// contents prior to commiting it.

LRESULT CTrayPalette::OnOverrideSelectedItemList(WPARAM wParam, LPARAM lParam)
{
    char prefix = value_preserving_cast<char>(lParam);
    ASSERT(prefix == 'P' && wParam);
    size_t nSel = GetSelectedTray();
    if (nSel == Invalid_v<size_t> || !wParam || prefix != 'P')
        return (LRESULT)0;

    CTrayManager* pYMgr = m_pDoc->GetTrayManager();
    CTraySet& pYSet = pYMgr->GetTraySet(nSel);

    if (pYSet.IsRandomPiecePull())
    {
        std::vector<PieceID>& pPceArray = *reinterpret_cast<std::vector<PieceID>*>(wParam);

        UINT nRandSeed = m_pDoc->GetRandomNumberSeed();

        std::vector<int> pnIndices = AllocateAndCalcRandomIndexVector(value_preserving_cast<int>(pPceArray.size()),
            value_preserving_cast<int>(pYSet.GetPieceIDTable().size()), nRandSeed, &nRandSeed);

        for (size_t i = 0; i < pPceArray.size(); i++)
            pPceArray.at(i) = pYSet.GetPieceIDTable().at(value_preserving_cast<size_t>(pnIndices[value_preserving_cast<int>(i)]));

        m_pDoc->SetRandomNumberSeed(nRandSeed);
    }

    return (LRESULT)1;
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (DWORD)m_comboYGrp.GetCurSel();
        ar << (DWORD)m_listTray.GetTopIndex();

        // Save the indexes of all the selected items.
        m_tblListBoxSel.RemoveAll();

        int nNumSelected = m_listTray.GetSelCount();
        m_tblListBoxSel.SetSize(nNumSelected);
        m_listTray.GetSelItems(nNumSelected, m_tblListBoxSel.GetData());
        ar << m_tblListBoxSel;
    }
    else
    {
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 90))       // V2.90
        {
            DWORD dwTmp;
            ar >> dwTmp; m_nComboIndex = (int)dwTmp;
            ar >> dwTmp; m_nListTopindex = (int)dwTmp;
            ar >> m_tblListBoxSel;
            m_bStateVarsArmed = TRUE;           // Inform Create() data is good
        }
        else if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))  // V2.0
        {
            DWORD dwTmp;
            ar >> dwTmp; m_nComboIndex = (int)dwTmp;
            ar >> dwTmp; m_nListTopindex = (int)dwTmp;
            ar >> m_tblListBoxSel;
            CWinPlacement wndSink;
            ar >> wndSink;                      // Eat this puppy
            m_bStateVarsArmed = TRUE;           // Inform Create() data is good
        }
        else                                                        // Pre 2.0
        {
            short sTmp;     // Eat the old data and go with the default values
            ar >> sTmp;
            ar >> sTmp;
            ar >> sTmp;
            ar >> sTmp;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::SetDocument(CGamDoc *pDoc)
{
    ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CGamDoc)));
    m_pDoc = pDoc;
    m_listTray.SetDocument(pDoc);
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::LoadTrayNameList()
{
    ASSERT(m_pDoc);
    CTrayManager* pYMgr = m_pDoc->GetTrayManager();
    ASSERT(pYMgr != NULL);

    m_comboYGrp.ResetContent();
    for (size_t i = 0; i < pYMgr->GetNumTraySets(); i++)
    {
        CTraySet& pYSet = pYMgr->GetTraySet(i);
        CString strTrayName = pYSet.GetName();
        if (pYSet.IsOwned())
        {
            CString strOwner = m_pDoc->GetPlayerManager()->GetPlayerUsingMask(
                pYSet.GetOwnerMask()).m_strName;
            strTrayName += " - " + strOwner;
        }
        int nIdx = m_comboYGrp.AddString(strTrayName);
        m_comboYGrp.SetItemData(nIdx, value_preserving_cast<DWORD_PTR>(i));    // Store the tray index in the data item
    }
    m_comboYGrp.SetCurSel(0);
    UpdateTrayList();
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::UpdatePaletteContents(const CTraySet* pTray)
{
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t> && pTray != NULL)
    {
        CTrayManager* pYMgr = m_pDoc->GetTrayManager();
        if (&pYMgr->GetTraySet(nSel) != pTray)
            return;             // Not visible in palette at present.
    }
    if (nSel == Invalid_v<size_t>)
        nSel = 0;               // Force first entry (if any)
    if (pTray == NULL)
    {
        LoadTrayNameList();
        int nComboIndex = FindTrayIndex(nSel);
        if (nComboIndex < 0)
            nComboIndex = 0;
        m_comboYGrp.SetCurSel(nComboIndex);
    }
    UpdateTrayList();
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::ShowTrayIndex(size_t nGroup, int nPos)
{
    size_t nSel = GetSelectedTray();
    if (nSel != nGroup)
    {
        m_comboYGrp.SetCurSel(FindTrayIndex(nGroup));
        UpdateTrayList();
    }
    m_listTray.ShowListIndex(nPos);
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::DeselectAll()
{
    if (m_hWnd != NULL)
        m_listTray.DeselectAll();
}

void CTrayPalette::SelectTrayPiece(size_t nGroup, PieceID pid,
    LPCTSTR pszNotificationTip /* = NULL */)
{
    size_t nSel = GetSelectedTray();
    if (nSel != nGroup)
    {
        m_comboYGrp.SetCurSel(FindTrayIndex(nGroup));
        UpdateTrayList();
    }
    size_t nItem = m_listTray.SelectTrayPiece(pid);
    if (pszNotificationTip != NULL)
    {
        m_listTray.SetNotificationTip(value_preserving_cast<int>(nItem), pszNotificationTip);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::UpdateTrayList()
{
    ASSERT(m_pDoc);
    CTrayManager* pYMgr = m_pDoc->GetTrayManager();
    ASSERT(pYMgr != NULL);

    m_toolTipCombo.DelTool(&m_comboYGrp);   // Always delete current tool

    size_t nSel = GetSelectedTray();
    if (nSel == Invalid_v<size_t>)
        return;

    // Get the name from the combo box since it has all the ownership
    // information added.
    CString strTrayName;
    m_comboYGrp.GetLBText(m_comboYGrp.GetCurSel(), strTrayName);

    TOOLINFO ti;
    m_toolTipCombo.FillInToolInfo(ti, &m_comboYGrp, 0);
    ti.uFlags |= TTF_SUBCLASS | TTF_CENTERTIP;
    ti.lpszText = (LPTSTR)(LPCTSTR)strTrayName;
    m_toolTipCombo.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);

    CTraySet& pYSet = pYMgr->GetTraySet(nSel);
    const std::vector<PieceID>* pPieceTbl = &pYSet.GetPieceIDTable();

    CString str = "";
    m_listTray.EnableDrag(TRUE);
    m_listTray.EnableSelfDrop(TRUE);
    m_listTray.SetTipsAllowed(TRUE);

    TrayViz eViz = pYSet.GetTrayContentVisibility();

    // If a tray is owned and the current player is the owner,
    // then the two sided view is forced. Otherwise the default
    // visibility is used. Also, if in scenario mode and if
    // a tray is owned it to will be fully visible.

    if (pYSet.IsOwned() && (m_pDoc->IsScenario() ||
         pYSet.IsOwnedBy(m_pDoc->GetCurrentPlayerMask()) &&
        !pYSet.IsEnforcingVisibilityForOwnerToo()))
    {
        eViz = trayVizTwoSide;              // Override visibility
    }
    else if (eViz == trayVizEachGeneric)
    {
        if (pYSet.IsRandomPiecePull())
        {
            str.LoadString(IDS_TRAY_RANDHIDDEN);
            m_listTray.EnableSelfDrop(FALSE);
        }
        else
            str.LoadString(IDS_TRAY_HIDDEN);
    }
    else if (eViz == trayVizNone)
    {
        if (pYSet.IsRandomPiecePull())
        {
            str.LoadString(IDS_TRAY_ALLRANDHIDDEN);
            m_listTray.EnableSelfDrop(FALSE);
        }
        else
            str.LoadString(IDS_TRAY_ALLHIDDEN);
        // Set the first (and only) element in the dummy array
        // to the first element in actual tray.
        m_dummyArray.clear();
        if (!pPieceTbl->empty())
            m_dummyArray.push_back(pPieceTbl->front());
        pPieceTbl = &m_dummyArray;
    }
    if (!m_pDoc->IsScenario() && pYSet.IsOwnedButNotByCurrentPlayer(m_pDoc))
        m_listTray.SetTipsAllowed(FALSE);

    if (!m_pDoc->IsScenario() && pYSet.IsOwned() &&
        !pYSet.IsOwnedBy(m_pDoc->GetCurrentPlayerMask()) &&
        !pYSet.IsNonOwnerAccessAllowed())
    {
        m_listTray.EnableDrag(FALSE);
        m_listTray.EnableSelfDrop(FALSE);
    }
    m_listTray.SetTrayContentVisibility(eViz, str);

    m_listTray.SetItemMap(pPieceTbl);
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CTrayPalette::OnDragItem(WPARAM wParam, LPARAM lParam)
{
    if (m_pDoc->IsPlaying())
        return 0;                       // Drags not supported during play

    DragInfo* pdi = (DragInfo*)lParam;

    if (pdi->m_dragType != DRAG_PIECE && pdi->m_dragType != DRAG_SELECTLIST &&
        pdi->m_dragType != DRAG_PIECELIST)
        return 0;                       // Only piece drops allowed

    if (pdi->m_dragType == DRAG_PIECE && pdi->GetSubInfo<DRAG_PIECE>().m_gamDoc != m_pDoc ||
        pdi->m_dragType == DRAG_SELECTLIST && pdi->GetSubInfo<DRAG_SELECTLIST>().m_gamDoc != m_pDoc ||
        pdi->m_dragType == DRAG_PIECELIST && pdi->GetSubInfo<DRAG_PIECELIST>().m_gamDoc != m_pDoc)
        return 0;                       // Only pieces from our document.

    if (pdi->m_dragType == DRAG_SELECTLIST)
    {
        if (!pdi->GetSubInfo<DRAG_SELECTLIST>().m_selectList->HasPieces())
            return 0;                   // Only piece drops allowed
    }

    size_t nGrpSel = GetSelectedTray();
    if (nGrpSel == Invalid_v<size_t>)
        return 0;                       // No tray to drop on

    if (wParam == phaseDragOver)
        return (LRESULT)(LPVOID)pdi->m_hcsrSuggest;
    else if (wParam == phaseDragDrop)
    {
        CTrayManager* pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYGrp = pYMgr->GetTraySet(nGrpSel);

        // Force selection of item under the mouse
        m_listTray.SetSelFromPoint(pdi->m_point);
        int nSel = m_listTray.GetCount() <= 0 ? -1 : m_listTray.GetCurSel();

        if (!m_pDoc->IsScenario() && pYGrp.IsOwnedButNotByCurrentPlayer(m_pDoc) &&
                pYGrp.GetTrayContentVisibility() == trayVizNone)
            nSel = -1;  // Always append pieces when dropping on single line view

        if (nSel >= 0)
        {
            // Check if the mouse is above or below the half point.
            // If above, insert before. If below, insert after.
            CRect rct;
            m_listTray.GetItemRect(nSel, &rct);
            if (pdi->m_point.y > (rct.top + rct.bottom) / 2)
                nSel++;
        }
        size_t dropCount;
        if (pdi->m_dragType == DRAG_PIECE)
        {
            ASSERT(!"untested code");
            dropCount = size_t(1);
            m_pDoc->AssignNewMoveGroup();
            m_pDoc->PlacePieceInTray(pdi->GetSubInfo<DRAG_PIECE>().m_pieceID, pYGrp, nSel < 0 ? Invalid_v<size_t> : value_preserving_cast<size_t>(nSel));
            // Select the last piece that was inserted
            nSel = value_preserving_cast<int>(pYGrp.GetPieceIDIndex(pdi->GetSubInfo<DRAG_PIECE>().m_pieceID));
        }
        else if (pdi->m_dragType == DRAG_PIECELIST)
        {
            m_pDoc->AssignNewMoveGroup();
            const std::vector<PieceID>& pieceIDList = CheckedDeref(pdi->GetSubInfo<DRAG_PIECELIST>().m_pieceIDList);
            dropCount = pieceIDList.size();
            size_t temp = m_pDoc->PlacePieceListInTray(pieceIDList,
                pYGrp, nSel < 0 ? Invalid_v<size_t> : value_preserving_cast<size_t>(nSel));
            nSel = temp == Invalid_v<size_t> ? -1 : value_preserving_cast<int>(temp);
        }
        else        // DRAG_SELECTLIST
        {
            ASSERT(pdi->m_dragType == DRAG_SELECTLIST);
            std::vector<CB::not_null<CDrawObj*>> m_listPtr;
            CSelList* pSLst = pdi->GetSubInfo<DRAG_SELECTLIST>().m_selectList;
            pSLst->LoadTableWithObjectPtrs(m_listPtr, CSelList::otAll, FALSE);
            pSLst->PurgeList(FALSE);
            dropCount = m_listPtr.size();
            m_pDoc->AssignNewMoveGroup();
            size_t temp = m_pDoc->PlaceObjectTableInTray(m_listPtr,
                pYGrp, nSel < 0 ? Invalid_v<size_t> : value_preserving_cast<size_t>(nSel));
            nSel = temp == Invalid_v<size_t> ? -1 : value_preserving_cast<int>(temp);
            m_pDoc->UpdateAllViews(NULL, HINT_UPDATESELECTLIST);
        }
        if (nSel >= 0)
        {
            for (size_t i = size_t(0) ; i < dropCount ; ++i)
            {
                m_listTray.SetSel(nSel - value_preserving_cast<int>(i));
            }
        }

        m_listTray.SetCurSel(nSel);

        // If the selection is out of view, force it into view.
        CRect rctLBoxClient;
        m_listTray.GetClientRect(&rctLBoxClient);
        CRect rct;
        m_listTray.GetItemRect(nSel, &rct);
        if (!rct.IntersectRect(rct, rctLBoxClient))
            m_listTray.SetTopIndex(nSel);
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Load the menu icon image and fill upper-right transparent area with
// current button face color.

void CTrayPalette::LoadMenuButtonBitmap()
{
    HBITMAP hBMap = (HBITMAP)LoadImage(AfxGetResourceHandle(),
        MAKEINTRESOURCE(IDB_MENU_ICON), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
    ASSERT(hBMap != NULL);
    m_bmpMenuBtn.Attach(hBMap);

    BITMAP bmap;
    m_bmpMenuBtn.GetBitmap(&bmap);
    m_sizeMenuBtn.cx = bmap.bmWidth;
    m_sizeMenuBtn.cy = bmap.bmHeight;

    CDC dc;
    dc.CreateCompatibleDC(NULL);
    CBitmap* prvBMap = dc.SelectObject(&m_bmpMenuBtn);
    CBrush brush;
    brush.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    CBrush* prvBrush = dc.SelectObject(&brush);
    dc.ExtFloodFill(m_sizeMenuBtn.cx - 1, 0, RGB(0, 255, 255), FLOODFILLSURFACE);
    dc.SelectObject(prvBrush);
    dc.SelectObject(prvBMap);
}

/////////////////////////////////////////////////////////////////////////////
// CTrayPalette message handlers

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

void CTrayPalette::PostNcDestroy()
{
    /* DO NOTHING - FRAME CLASS WOULD DELETE SELF! */
}

void CTrayPalette::OnLButtonUp(UINT nFlags, CPoint point)
{
    CMenu bar;
    if (bar.LoadMenu(IDR_MENU_PLAYER_POPUPS))
    {
        CMenu& popup = *bar.GetSubMenu(MENU_PV_PIECE_TRAY);
        ASSERT(popup.m_hMenu != NULL);

        // Make sure we clean up even if exception is tossed.
        TRY
        {
            CRect rct;
            // Use the list box as a guide of where to place the menu
            m_listTray.GetWindowRect(rct);
            popup.TrackPopupMenu(TPM_LEFTBUTTON | TPM_LEFTALIGN,
                rct.left, rct.top, this); // Route commands through tray window
        }
        END_TRY
    }
}

void CTrayPalette::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
    CWnd::OnWindowPosChanging(lpwndpos);

    // Need this code to avoid a detached combo list box.
    if (m_comboYGrp.m_hWnd != NULL)
        m_comboYGrp.ShowDropDown(FALSE);
}

/////////////////////////////////////////////////////////////////////////////

void CTrayPalette::OnTrayNameCbnSelchange()
{
    UpdateTrayList();
}

void CTrayPalette::OnTrayListDoubleClick()
{
    if (!m_listTray.IsShowingTileImages())
        return;

    int nIndex = m_listTray.GetCaretIndex();
    if (nIndex < 0 || nIndex > 65535)
        return;

    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager* pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr->GetTraySet(nSel);
        if (pYSet.IsOwned() &&
            !pYSet.IsOwnedBy(m_pDoc->GetCurrentPlayerMask()))
            return;
    }

    PieceID pid = m_listTray.MapIndexToItem(value_preserving_cast<size_t>(nIndex));
    m_pDoc->DoEditPieceText(pid, TRUE);
}

void CTrayPalette::OnPieceTrayShuffle()
{
    // Generate a shuffled index vector
    UINT nRandSeed = m_pDoc->GetRandomNumberSeed();
    int nNumIndices = m_listTray.GetCount();
    std::vector<int> pnIndices = AllocateAndCalcRandomIndexVector(nNumIndices,
        nNumIndices, nRandSeed, &nRandSeed);
    m_pDoc->SetRandomNumberSeed(nRandSeed);

    // Build table of shuffled pieces
    std::vector<PieceID> tblPids;
    tblPids.reserve(value_preserving_cast<size_t>(nNumIndices));
    for (int i = 0; i < nNumIndices; i++)
        tblPids.push_back(m_listTray.MapIndexToItem(value_preserving_cast<size_t>(pnIndices[i])));

    m_pDoc->AssignNewMoveGroup();

    size_t nSel = GetSelectedTray();
    CTrayManager* pYMgr = m_pDoc->GetTrayManager();

    if (m_pDoc->IsRecording())
    {
        CString strMsg;
        strMsg.Format(IDS_TIP_TRAY_SHUFFLED, tblPids.size());
        m_pDoc->RecordEventMessage(strMsg, nSel, tblPids.front());
    }

    m_pDoc->PlacePieceListInTray(tblPids, pYMgr->GetTraySet(nSel), 0);
}

void CTrayPalette::OnUpdatePieceTrayShuffle(CCmdUI* pCmdUI)
{
    BOOL bNoOwnerRestrictions = TRUE;
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager* pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr->GetTraySet(nSel);
        bNoOwnerRestrictions = !(pYSet.IsOwnedButNotByCurrentPlayer(m_pDoc) &&
            !pYSet.IsNonOwnerAccessAllowed());
    }
    pCmdUI->Enable((m_pDoc->IsScenario() || bNoOwnerRestrictions) &&
        m_listTray.GetCount() > 1);
}

void CTrayPalette::OnPieceTrayShuffleSelected()
{
    CArray<int, int> tblListSel;
    int nNumSelected = m_listTray.GetSelCount();
    tblListSel.SetSize(nNumSelected);
    m_listTray.GetSelItems(nNumSelected, tblListSel.GetData());

    int nTopSel = tblListSel[0];        // This is where the shuffle is inserted

    // Generate a shuffled index vector for the number of selected items
    UINT nRandSeed = m_pDoc->GetRandomNumberSeed();
    int nNumIndices = tblListSel.GetSize();
    std::vector<int> pnIndices = AllocateAndCalcRandomIndexVector(nNumIndices,
        nNumIndices, nRandSeed, &nRandSeed);
    m_pDoc->SetRandomNumberSeed(nRandSeed);

    // Build table of shuffled pieces
    std::vector<PieceID> tblPids;
    tblPids.reserve(value_preserving_cast<size_t>(nNumIndices));
    for (int i = 0; i < nNumIndices; i++)
        tblPids.push_back(m_listTray.MapIndexToItem(value_preserving_cast<size_t>(tblListSel[pnIndices[i]])));

    m_pDoc->AssignNewMoveGroup();

    size_t nSel = GetSelectedTray();
    CTrayManager* pYMgr = m_pDoc->GetTrayManager();

    if (m_pDoc->IsRecording())
    {
        CString strMsg;
        strMsg.Format(IDS_TIP_TRAY_SHUFFLED, tblPids.size());
        m_pDoc->RecordEventMessage(strMsg, nSel, tblPids.front());
    }

    m_pDoc->PlacePieceListInTray(tblPids, pYMgr->GetTraySet(nSel), value_preserving_cast<size_t>(nTopSel));
}

void CTrayPalette::OnUpdatePieceTrayShuffleSelected(CCmdUI* pCmdUI)
{
    BOOL bNoOwnerRestrictions = TRUE;
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager* pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr->GetTraySet(nSel);
        bNoOwnerRestrictions = !(pYSet.IsOwnedButNotByCurrentPlayer(m_pDoc) &&
            !pYSet.IsNonOwnerAccessAllowed());
    }

    pCmdUI->Enable((m_pDoc->IsScenario() || bNoOwnerRestrictions) &&
        m_listTray.GetSelCount() > 1 && m_listTray.IsShowingTileImages());
}

void CTrayPalette::OnEditElementText()
{
    DoEditSelectedPieceText(TRUE);
}

void CTrayPalette::OnUpdateEditElementText(CCmdUI* pCmdUI)
{
    BOOL bNoOwnerRestrictions = TRUE;
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager* pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr->GetTraySet(nSel);
        bNoOwnerRestrictions = !pYSet.IsOwnedButNotByCurrentPlayer(m_pDoc);
    }
    pCmdUI->Enable((m_pDoc->IsScenario() || bNoOwnerRestrictions) &&
        m_listTray.GetSelCount() == 1 && m_listTray.IsShowingTileImages());
}

void CTrayPalette::OnActTurnOver()
{
    CArray<int, int> tblListSel;
    int nNumSelected = m_listTray.GetSelCount();
    tblListSel.SetSize(nNumSelected);
    m_listTray.GetSelItems(nNumSelected, tblListSel.GetData());

    m_pDoc->AssignNewMoveGroup();
    for (int i = 0; i < tblListSel.GetSize(); i++)
    {
        PieceID pid = m_listTray.MapIndexToItem(value_preserving_cast<size_t>(tblListSel[i]));
        m_pDoc->InvertPlayingPieceInTray(pid, FALSE);
    }
    size_t nSel = GetSelectedTray();
    CTrayManager* pYMgr = m_pDoc->GetTrayManager();
    CGamDocHint hint;
    hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = &pYMgr->GetTraySet(nSel);
    m_pDoc->UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
}

void CTrayPalette::OnUpdateActTurnOver(CCmdUI* pCmdUI)
{
    BOOL bNoOwnerRestrictions = TRUE;
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager* pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr->GetTraySet(nSel);
        bNoOwnerRestrictions = !pYSet.IsOwnedButNotByCurrentPlayer(m_pDoc);
    }

    pCmdUI->Enable((m_pDoc->IsScenario() || bNoOwnerRestrictions) &&
        m_listTray.GetSelCount() > 0 && m_listTray.IsShowingTileImages());
}

void CTrayPalette::OnActTurnoverAllPieces()
{
    m_pDoc->AssignNewMoveGroup();
    for (int i = 0; i < m_listTray.GetCount(); i++)
    {
        PieceID pid = m_listTray.MapIndexToItem(value_preserving_cast<size_t>(i));
        m_pDoc->InvertPlayingPieceInTray(pid, FALSE);
    }
    size_t nSel = GetSelectedTray();
    CTrayManager* pYMgr = m_pDoc->GetTrayManager();
    CGamDocHint hint;
    hint.GetArgs<HINT_TRAYCHANGE>().m_pTray = &pYMgr->GetTraySet(nSel);
    m_pDoc->UpdateAllViews(NULL, HINT_TRAYCHANGE, &hint);
}

void CTrayPalette::OnUpdateActTurnoverAllPieces(CCmdUI* pCmdUI)
{
    BOOL bNoOwnerRestrictions = TRUE;
    size_t nSel = GetSelectedTray();
    if (nSel != Invalid_v<size_t>)
    {
        CTrayManager* pYMgr = m_pDoc->GetTrayManager();
        CTraySet& pYSet = pYMgr->GetTraySet(nSel);
        bNoOwnerRestrictions = !pYSet.IsOwnedButNotByCurrentPlayer(m_pDoc);
    }

    pCmdUI->Enable((m_pDoc->IsScenario() ||
        bNoOwnerRestrictions) && m_listTray.IsShowingTileImages());
}

void CTrayPalette::OnPieceTrayAbout()
{
    size_t nSel = GetSelectedTray();
    if (nSel == Invalid_v<size_t>)
        return;
    CTrayManager* pYMgr = m_pDoc->GetTrayManager();
    CTraySet& pYSet = pYMgr->GetTraySet(nSel);
    CString strMsg;
    CString strTmp;
    strMsg.Format(IDS_MSG_TRAY_INFO, pYSet.GetName());

    switch (pYSet.GetTrayContentVisibility())
    {
        case trayVizTwoSide:
            strTmp.LoadString(IDS_MSG_TVIZ_TWO_SIDE);
            break;
        case trayVizOneSide:
            strTmp.LoadString(IDS_MSG_TVIZ_ONE_SIDE);
            break;
        case trayVizEachGeneric:
            strTmp.LoadString(IDS_MSG_TVIZ_HIDDEN);
            break;
        case trayVizNone:
            strTmp.LoadString(IDS_MSG_TVIZ_ALL_HIDDEN);
            break;
        default:
            ASSERT(FALSE);
            strTmp.LoadString(IDS_ERR_TRAY_VIZ);
    }

    strMsg += strTmp + '\n';

    if (pYSet.IsRandomPiecePull())
    {
        strTmp.LoadString(IDS_MSG_RANDOM_PULL);
        strMsg += strTmp + '\n';
    }

    if (pYSet.IsOwned())
    {
        strTmp.Format(IDS_MSG_OWNED_BY, (LPCTSTR)m_pDoc->GetPlayerManager()->
            GetPlayerUsingMask(pYSet.GetOwnerMask()).m_strName);
    }
    else
        strTmp.LoadString(IDS_MSG_NOT_OWNED);

    strMsg += strTmp;
    if (pYSet.IsOwned())
    {
        if (pYSet.IsNonOwnerAccessAllowed())
            strTmp.LoadString(IDS_MSG_NONOWN_ALLOWED);
        else
            strTmp.LoadString(IDS_MSG_NONOWN_NOT_ALLOWED);
        strMsg += '\n' + strTmp;

        if (pYSet.IsEnforcingVisibilityForOwnerToo())
            strTmp.LoadString(IDS_MSG_TVIZ_OWNER_TOO);
        else
            strTmp.LoadString(IDS_MSG_TVIZ_OWNER_FULL);
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
