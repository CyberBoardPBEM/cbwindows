// VwPrjgsn.cpp : Scenario Project View
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
#include    "Gp.h"
#include    "GamDoc.h"
#include    "ResTbl.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "Trays.h"
#include    "Player.h"

#include    "VwPrjgsn.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

wxIMPLEMENT_DYNAMIC_CLASS(CProjListBoxGsn, CProjListBoxBaseWx)
IMPLEMENT_DYNCREATE(CGsnProjViewContainer, CView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#if 0
/////////////////////////////////////////////////////////////////////////////

const int XBORDER = 5;
const int YBORDER = 5;

const int BTN_GROUP_XGAP = 12;
const int BTN_GROUP_YGAP = 6;

const int BTN_PROJ_WD = 14 * 4;
const int BTN_PROJ_HT = 12;

const int PROJ_LIST_WIDTH = 250;
#endif

/////////////////////////////////////////////////////////////////////////////
// Button content tables - selected based on current type of item selected
//      in the project list box.

const int nNumButtons = 3;
const int nNumGroups = 5;

static UINT btnGroupNull[nNumButtons] =     // None selected
    { 0, 0, 0 };
static UINT btnGroup0[nNumButtons] =        // Document type
    { IDS_BTN_GSN_PROPERTIES, 0, 0, };
static UINT btnGroup1[nNumButtons] =        // Board heading
    { IDS_BTN_GSN_SELECT, 0, 0, };
static UINT btnGroup2[nNumButtons] =        // Boards
    { IDS_BTN_GSN_VIEW, IDS_BTN_GSN_PROPERTIES, IDS_BTN_GSN_REMOVE };
static UINT btnGroup3[nNumButtons] =        // Tray heading
    { IDS_BTN_GSN_CREATE, 0, 0 };
static UINT btnGroup4[nNumButtons] =        // Tray groups
    { IDS_BTN_GSN_EDIT, IDS_BTN_GSN_PROPERTIES, IDS_BTN_GSN_DELETE };

static UINT * btnGroupTbl[nNumGroups + 1] =
    { btnGroupNull, btnGroup0, btnGroup1, btnGroup2, btnGroup3, btnGroup4 };

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CGsnProjView, wxPanel)
#if 0
    ON_WM_SIZE()
    ON_WM_CREATE()
#endif
    EVT_LISTBOX(XRCID("m_listProj"), OnSelChangeProjList)
    EVT_LISTBOX_DCLICK(XRCID("m_listProj"), OnDblClkProjList)
    EVT_BUTTON(XRCID("m_btnPrjA"), OnClickedProjBtnA)
    EVT_BUTTON(XRCID("m_btnPrjB"), OnClickedProjBtnB)
    EVT_BUTTON(XRCID("m_btnPrjC"), OnClickedProjBtnC)
#if 0
    ON_WM_ERASEBKGND()
#endif
    EVT_MENU(XRCID("ID_EDIT_BRDPROP"), OnEditBoardProperties)
    EVT_UPDATE_UI(XRCID("ID_EDIT_BRDPROP"), OnUpdateEditBoardProperties)
    EVT_CONTEXT_MENU(OnContextMenu)
    EVT_MENU(XRCID("ID_PPROJITEM_EDIT"), OnProjItemEdit)
    EVT_UPDATE_UI(XRCID("ID_PPROJITEM_EDIT"), OnUpdateProjItemEdit)
    EVT_MENU(XRCID("ID_PPROJITEM_DELETE"), OnProjItemDelete)
    EVT_UPDATE_UI(XRCID("ID_PPROJITEM_DELETE"), OnUpdateProjItemDelete)
    EVT_MENU(XRCID("ID_PPROJITEM_PROPERTIES"), OnProjItemProperties)
    EVT_UPDATE_UI(XRCID("ID_PPROJITEM_PROPERTIES"), OnUpdateProjItemProperties)
    EVT_MENU(XRCID("ID_PPROJITEM_VIEW"), OnProjItemView)
    EVT_UPDATE_UI(XRCID("ID_PPROJITEM_VIEW"), OnUpdateProjItemView)
    EVT_SHOWPLAYINGBOARD(OnMessageShowPlayingBoard)
    EVT_WINSTATE_RESTORE(OnMessageRestoreWinState)
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CGsnProjViewContainer, CView)
    ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGsnProjView

CGsnProjView::CGsnProjView(CGsnProjViewContainer& p) :
    CB_XRC_BEGIN_CTRLS_DEFN(static_cast<wxWindow*>(p), CGsnProjView)
        CB_XRC_CTRL(m_listProj)
        CB_XRC_CTRL(m_editInfo)
        CB_XRC_CTRL(m_listTrays)
        CB_XRC_CTRL(m_btnPrjA)
        CB_XRC_CTRL(m_btnPrjB)
        CB_XRC_CTRL(m_btnPrjC)
    CB_XRC_END_CTRLS_DEFN(),
    parent(&p),
    document(dynamic_cast<CGamDoc*>(parent->GetDocument()))
{
    m_nLastSel = -1;
    m_nLastGrp = -1;

    m_listTrays->Init(GetDocument());
    m_listTrays->SetTrayContentVisibility(trayVizAllSides);
}

CGsnProjView::~CGsnProjView()
{
}

#if 0
int CGsnProjView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Create various controls...
    CRect rctClient;
    GetClientRect(&rctClient);

    // Buttons associated with the project listbox...
    CPoint posBtn(XBORDER, YBORDER);
    CSize  sizeBtn(BTN_PROJ_WD, BTN_PROJ_HT);
    posBtn.x = CreateButton(IDC_V_GSN_BTN_PRJA, m_btnPrjA, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out
    posBtn.x = CreateButton(IDC_V_GSN_BTN_PRJB, m_btnPrjB, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out
    posBtn.x = CreateButton(IDC_V_GSN_BTN_PRJC, m_btnPrjC, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out

    int xProjListRight = posBtn.x - 1;  // Save for list box construction

    xProjListRight = CB::max(xProjListRight, PROJ_LIST_WIDTH);

    // The project list box.
    CRect rctList(XBORDER, YBORDER, xProjListRight, YBORDER);

    if (!CreateListbox(IDC_V_GSN_PROJLIST, m_listProj,
            WS_HSCROLL | WS_VISIBLE | LBS_SORT | LBS_HASSTRINGS, rctList))
        return -1;

    // OK...now create the remaining item controls...

    rctList.left = rctList.right + BTN_GROUP_XGAP;
    rctList.right = rctClient.right - XBORDER;

    m_listTrays = MakeOwner<CTrayListBox>(GetDocument());
    if (!CreateListbox(IDC_V_GSN_TRAYLIST, *m_listTrays, WS_HSCROLL | LBS_HASSTRINGS, rctList))
        return -1;
    m_listTrays->SetTrayContentVisibility(trayVizAllSides);
    if (!CreateEditbox(IDC_V_GSN_EDITINFO, m_editInfo, rctList))
        return -1;

    return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CGsnProjView::OnInitialUpdate()
{
    GetDocument().DoInitialUpdate();   // Since UpdateAllViews isn't virtual
    CGamDoc& pDoc = GetDocument();
    CPBoardManager& pPBMgr = pDoc.GetPBoardManager();
    // Only honor the open-on-load flags if the save window state
    // is disabled.
    if (!pDoc.m_bSaveWindowPositions)
    {
        for (size_t i = size_t(0); i < pPBMgr.GetNumPBoards(); i++)
        {
            CPlayBoard& pPBoard = pPBMgr.GetPBoard(i);
            if (pPBoard.m_bOpenBoardOnLoad)
            {
                // Defer opening the view until our view init
                // in done.
                ShowPlayingBoardEvent event(i);
                AddPendingEvent(event);
            }
        }
    }
    else
    {
        WinStateRestoreEvent event;
        AddPendingEvent(event);
    }
}

void CGsnProjView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    if (lHint == HINT_ALWAYSUPDATE || lHint == HINT_BOARDCHANGE ||
            lHint == HINT_TRAYCHANGE || lHint == HINT_GSNPROPCHANGE)
        DoUpdateProjectList();
}

/////////////////////////////////////////////////////////////////////////////
// CGsnProjView drawing

void CGsnProjView::OnDraw(CDC* pDC)
{
    CDocument& pDoc = GetDocument();
    // TODO: add draw code here
}

#if 0
BOOL CGsnProjView::OnEraseBkgnd(CDC* pDC)
{
    CBrush brFill(GetSysColor(COLOR_BTNFACE));
    CBrush brHilite(GetSysColor(COLOR_BTNHIGHLIGHT));
    CBrush brShadow(GetSysColor(COLOR_BTNSHADOW));

    CRect rctProj;
    m_listProj.GetWindowRect(&rctProj);
    ScreenToClient(&rctProj);
    int xDvdr = rctProj.right + BTN_GROUP_XGAP / 2 - 1;

    CRect rct;
    GetClientRect(&rct);
    pDC->FillRect(&rct, &brFill);

    CBrush* prvBrush = pDC->SelectObject(&brHilite);
    pDC->PatBlt(rct.left, rct.top, rct.Width(), 1, PATCOPY);
    pDC->PatBlt(rct.left, rct.top, 1, rct.Height(), PATCOPY);
    pDC->PatBlt(xDvdr+1, rct.top, 1, rct.Height(), PATCOPY);

    pDC->SelectObject(&brShadow);
    pDC->PatBlt(rct.right - 1, rct.top, 1, rct.Height(), PATCOPY);
    pDC->PatBlt(rct.left, rct.bottom - 1, rct.Width(), 1, PATCOPY);
    pDC->PatBlt(xDvdr, rct.top + 1, 1, rct.Height() - 2, PATCOPY);

    pDC->SelectObject(prvBrush);
    return TRUE;

//!!return CView::OnEraseBkgnd(pDC);
}

/////////////////////////////////////////////////////////////////////////////

void CGsnProjView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    LayoutView();
}

/////////////////////////////////////////////////////////////////////////////

void CGsnProjView::LayoutView()
{
    CRect rctClient;
    GetClientRect(&rctClient);
    HDWP hDwp = BeginDeferWindowPos(10);

    CRect rct;
    m_btnPrjA.GetWindowRect(&rct);
    ScreenToClient(&rct);
    int yBtnTop = rctClient.bottom - YBORDER - rct.Height();

    // Project button group...
    hDwp = DeferWindowPos(hDwp, m_btnPrjA.m_hWnd, NULL, rct.left, yBtnTop,
        0, 0, SWP_NOSIZE | SWP_NOZORDER);
    m_btnPrjB.GetWindowRect(&rct);
    ScreenToClient(&rct);
    hDwp = DeferWindowPos(hDwp, m_btnPrjB.m_hWnd, NULL, rct.left, yBtnTop,
        0, 0, SWP_NOSIZE | SWP_NOZORDER);
    m_btnPrjC.GetWindowRect(&rct);
    ScreenToClient(&rct);
    hDwp = DeferWindowPos(hDwp, m_btnPrjC.m_hWnd, NULL, rct.left, yBtnTop,
        0, 0, SWP_NOSIZE | SWP_NOZORDER);

    // Project list box

    m_listProj.GetWindowRect(&rct);
    ScreenToClient(&rct);
    rct.bottom = yBtnTop - BTN_GROUP_YGAP;
    rct.top = rct.bottom <= YBORDER ? rct.bottom : YBORDER;

    hDwp = DeferWindowPos(hDwp, m_listProj.m_hWnd, NULL, rct.left, rct.top,
        rct.Width(), rct.Height(), SWP_NOZORDER);

    rct.left = rct.right + BTN_GROUP_XGAP;
    rct.right = rct.left >= rctClient.right - XBORDER ? rct.left :
        rctClient.right - XBORDER;
    rct.bottom = rctClient.bottom - YBORDER;
    rct.top = rct.bottom <= YBORDER ? rct.bottom : YBORDER;

    hDwp = DeferWindowPos(hDwp, m_listTrays->m_hWnd, NULL, rct.left, rct.top,
        rct.Width(), rct.Height(), SWP_NOZORDER);
    hDwp = DeferWindowPos(hDwp, m_editInfo.m_hWnd, NULL, rct.left, rct.top,
        rct.Width(), rct.Height(), SWP_NOZORDER);

    EndDeferWindowPos(hDwp);
}

/////////////////////////////////////////////////////////////////////////////
// Returns the x pos on the right of the button. relsize.cx is in 1/4 avg
// character width. relsize.cy is 1/8 char height. If returned zero,
// create failed.

int CGsnProjView::CreateButton(UINT nCtrlID, CButton& btn, CPoint llpos,
    CSize relsize)
{
    CRect rct(llpos, CSize((relsize.cx * g_res.tm8ss.tmAveCharWidth) / 4,
        (relsize.cy * g_res.tm8ss.tmHeight) / 8));
    if (!btn.Create(""_cbstring, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            rct, this, nCtrlID))
        return 0;
    btn.SetFont(CFont::FromHandle(g_res.h8ss));
    btn.GetWindowRect(&rct);
    ScreenToClient(&rct);
    return rct.right + 1;
}

/////////////////////////////////////////////////////////////////////////////
// Returns the x pos on the right of the button. relsize.cx is in 1/4 avg
// character width. relsize.cy is 1/8 char height.

BOOL CGsnProjView::CreateListbox(UINT nCtrlID, CListBox& lbox, DWORD dwStyle,
    CRect& rct)
{
    BOOL bOk = lbox.Create(dwStyle | WS_CHILD | WS_BORDER | WS_VSCROLL |
        WS_TABSTOP | LBS_NOTIFY | LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT,
        rct, this, nCtrlID);
    ::SetWindowLong(lbox.m_hWnd, GWL_EXSTYLE,
        ::GetWindowLong(lbox.m_hWnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
    return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// Returns the x pos on the right of the button. relsize.cx is in 1/4 avg
// character width. relsize.cy is 1/8 char height.

BOOL CGsnProjView::CreateEditbox(UINT nCtrlID, CEdit& ebox, CRect& rct)
{
    BOOL bOk = ebox.Create(WS_CHILD | WS_BORDER | WS_VSCROLL | WS_TABSTOP |
        ES_LEFT | ES_MULTILINE | ES_READONLY, rct, this, nCtrlID);
    ::SetWindowLong(ebox.m_hWnd, GWL_EXSTYLE,
        ::GetWindowLong(ebox.m_hWnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
    ebox.SetFont(CFont::FromHandle(g_res.h8ss));
    return bOk;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Updates buttons for specified group

void CGsnProjView::UpdateButtons(int nGrp)
{
    nGrp++;                     // -1 means no selection so bump to zero
    UINT* pTbl = btnGroupTbl[nGrp];
    SetButtonState(*m_btnPrjA, pTbl[0]);
    SetButtonState(*m_btnPrjB, pTbl[1]);
    SetButtonState(*m_btnPrjC, pTbl[2]);
}

void CGsnProjView::SetButtonState(wxButton& btn, UINT nStringID)
{
    if (nStringID == 0)
        btn.SetLabel(""_cbstring);
    else
    {
        CB::string str = CB::string::LoadString(nStringID);
        btn.SetLabel(str);
    }
    btn.Enable(nStringID != 0);
}

/////////////////////////////////////////////////////////////////////////////
// Updates item controls for specified group

void CGsnProjView::UpdateItemControls(int nGrp)
{
    wxWindowUpdateLocker freezer(this);
    ChildrenRepositioningGuard crg(this);

    m_listTrays->Hide();
    m_editInfo->Hide();
    if (nGrp == grpTray)        // Trays
    {
        m_listTrays->Show();
    }
    else    // Board, headings and no selection
    {
        m_editInfo->Show();
    }
    Layout();
}

/////////////////////////////////////////////////////////////////////////////

void CGsnProjView::DoUpdateProjectList(BOOL bUpdateItem /* = TRUE */)
{
    static int bDisplayIDs = -1;
    if (bDisplayIDs == -1)
    {
        bDisplayIDs = GetApp()->GetProfileInt("Settings"_cbstring, "DisplayIDs"_cbstring, 0);
    }
    CGamDoc& pDoc = GetDocument();

    // Preserve the current selection
    size_t nTopIdx = m_listProj->GetVisibleRowsBegin();
    int nCurSel = m_listProj->GetSelection();

    {
    wxWindowUpdateLocker freezer(&*m_listProj);

    m_listProj->Clear();

    // Document type....
    CB::string str = CB::string::LoadString(IDS_PHEAD_GSN_DOCTYPE);
    m_listProj->AddItem(grpDoc, str);

    // Boards....
    str = CB::string::LoadString(IDS_PHEAD_GSN_BOARDS);
    m_listProj->AddItem(grpBrdHdr, str);

    CPBoardManager& pPBMgr = pDoc.GetPBoardManager();
    for (size_t i = size_t(0); i < pPBMgr.GetNumPBoards(); i++)
    {
        CPlayBoard& pPBoard = pPBMgr.GetPBoard(i);
        str = pPBoard.GetBoard()->GetName();
        if (bDisplayIDs)
        {
            CB::string strTmp = std::move(str);
            str = std::format("[{}] {}",
                pPBoard.GetBoard()->GetSerialNumber(), strTmp);
        }
        if (pPBoard.IsOwned())
        {
            CB::string strOwner = pDoc.GetPlayerManager()->GetPlayerUsingMask(
                pPBoard.GetOwnerMask()).m_strName;
            CB::string strOwnedBy = CB::string::Format(IDS_TIP_OWNED_BY_PROJ, strOwner);
            str += strOwnedBy;
        }
        m_listProj->AddItem(grpBrd, str, i);
    }

    // Trays....
    str = CB::string::LoadString(IDS_PHEAD_GSN_TRAYS);
    m_listProj->AddItem(grpTrayHdr, str);

    CTrayManager& pYMgr = pDoc.GetTrayManager();
    for (size_t i = size_t(0); i < pYMgr.GetNumTraySets(); i++)
    {
        CTraySet& pYSet = pYMgr.GetTraySet(i);
        str = pYSet.GetName();
        if (bDisplayIDs)
        {
            CB::string strTmp = std::move(str);
            str = std::format("[{}] {}", i, strTmp);
        }
        if (pYSet.IsOwned())
        {
            CB::string strOwner = pDoc.GetPlayerManager()->GetPlayerUsingMask(
                pYSet.GetOwnerMask()).m_strName;
            CB::string strOwnedBy = CB::string::Format(IDS_TIP_OWNED_BY_PROJ, strOwner);
            str += strOwnedBy;
        }
        m_listProj->AddItem(grpTray, str, i);
    }

    // OK...Show the updates
    }
    m_listProj->Refresh();

    m_listProj->ScrollToRow(nTopIdx);
    if (nCurSel != wxNOT_FOUND)
    {
        if (nCurSel >= m_listProj->GetItemCount())
            nCurSel = value_preserving_cast<int>(m_listProj->GetItemCount() - size_t(1));
        m_listProj->SetSelection(nCurSel);
    }
    else
        m_listProj->SetSelection(0);

    if (bUpdateItem)
    {
        m_nLastSel = -1;
        m_nLastGrp = -1;
        OnSelChangeProjList();
    }
}

/////////////////////////////////////////////////////////////////////////////
// List box notifications

void CGsnProjView::OnSelChangeProjList(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (m_nLastSel == nSel)
        return;

    decltype(grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoUpdateGsnInfo(); break;
        case grpBrdHdr: DoUpdateBoardHelpInfo(); break;
        case grpBrd:    DoUpdateBoardInfo(); break;
        case grpTrayHdr:DoUpdateTrayHelpInfo(); break;
        case grpTray:   DoUpdateTrayList(); break;
    }
    if (nGrp != m_nLastGrp)
    {
        UpdateItemControls(nGrp);
        UpdateButtons(nGrp);
        m_nLastGrp = nGrp;
    }
}

void CGsnProjView::OnDblClkProjList(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND)
        return;
    decltype(grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoGsnProperty(); break;
        case grpBrdHdr: DoBoardSelection(); break;
        case grpBrd:    DoBoardView(); break;
        case grpTrayHdr:DoTrayCreate(); break;
        case grpTray:   DoTrayEdit(); break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Button notifications

void CGsnProjView::OnClickedProjBtnA(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoGsnProperty(); break;
        case grpBrdHdr: DoBoardSelection(); break;
        case grpBrd:    DoBoardView(); break;
        case grpTrayHdr:DoTrayCreate(); break;
        case grpTray:   DoTrayEdit(); break;
    }
}

void CGsnProjView::OnClickedProjBtnB(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardProperty(); break;
        case grpTray:   DoTrayProperty(); break;
    }
}

void CGsnProjView::OnClickedProjBtnC(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardRemove(); break;
        case grpTray:   DoTrayDelete(); break;
    }
}

void CGsnProjView::OnEditBoardProperties(wxCommandEvent& event)
{
    DoBoardProperty();
}

void CGsnProjView::OnUpdateEditBoardProperties(wxUpdateUIEvent& pCmdUI)
{
    int nSel = m_listProj->GetSelection();
    pCmdUI.Enable(nSel != wxNOT_FOUND &&
        m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpBrd);
}

void CGsnProjView::OnContextMenu(wxContextMenuEvent& event)
{
    const char* nID = nullptr;

    if (event.GetEventObject() == &*m_listProj)
        nID = "3=PJ_GSN_DEFAULT";

    if (!nID)
        return;

    std::unique_ptr<wxMenuBar> bar(wxXmlResource::Get()->LoadMenuBar("IDR_MENU_PLAYER_POPUPS"));
    if (bar)
    {
        int index = bar->FindMenu(nID);
        wxASSERT(index != wxNOT_FOUND);
        std::unique_ptr<wxMenu> popup(bar->Remove(value_preserving_cast<size_t>(index)));

        // Make sure we clean up even if exception is tossed.
        try
        {
            PopupMenu(&*popup);
        }
        catch (...)
        {
            wxASSERT(!"exception");
        }
    }
}

void CGsnProjView::OnProjItemEdit(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND)
        return;
    decltype(grpTray) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    if (nGrp == grpTray)
        DoTrayEdit();
}

void CGsnProjView::OnUpdateProjItemEdit(wxUpdateUIEvent& pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj->GetSelection();
    if (nSel != wxNOT_FOUND)
    {
        decltype(grpTray) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
        wxASSERT(nGrp >= 0);
        if (nGrp == grpTray)
            bEnable = TRUE;
    }
    pCmdUI.Enable(bEnable);
}

void CGsnProjView::OnProjItemDelete(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND)
        return;
    decltype(grpBrd) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardRemove(); break;
        case grpTray:   DoTrayDelete(); break;
    }
}

void CGsnProjView::OnUpdateProjItemDelete(wxUpdateUIEvent& pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj->GetSelection();
    if (nSel != wxNOT_FOUND)
    {
        decltype(grpBrd) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
        wxASSERT(nGrp >= 0);
        switch (nGrp)
        {
            case grpBrd:
            case grpTray:
                bEnable = TRUE;
                break;
            default: ;
        }
    }
    pCmdUI.Enable(bEnable);
}

void CGsnProjView::OnProjItemProperties(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND)
        return;
    decltype(grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoGsnProperty(); break;
        case grpBrd:    DoBoardProperty(); break;
        case grpTray:   DoTrayProperty(); break;
    }
}

void CGsnProjView::OnUpdateProjItemProperties(wxUpdateUIEvent& pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj->GetSelection();
    if (nSel != wxNOT_FOUND)
    {
        decltype(grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
        wxASSERT(nGrp >= 0);
        switch (nGrp)
        {
            case grpDoc:
            case grpBrd:
            case grpTray:
                bEnable = TRUE;
                break;
            default: ;
        }
    }
    pCmdUI.Enable(bEnable);
}

void CGsnProjView::OnProjItemView(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(grpBrd) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    if (nGrp == grpBrd)
        DoBoardView();
}

void CGsnProjView::OnUpdateProjItemView(wxUpdateUIEvent& pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj->GetSelection();
    if (nSel != wxNOT_FOUND)
    {
        decltype(grpBrd) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
        wxASSERT(nGrp >= 0);
        if (nGrp == grpBrd)
            bEnable = TRUE;
    }
    pCmdUI.Enable(bEnable);
}

///////////////////////////////////////////////////////////////////////
// This method handles the custom message WM_SHOWPLAYINGBOARD. The
// message is posted during view initial update if the playing
// board m_bOpenBoardOnLoad option is set. (wParam = the board index)

void CGsnProjView::OnMessageShowPlayingBoard(ShowPlayingBoardEvent& event)
{
    CGamDoc& pDoc = GetDocument();
    CPlayBoard& pPBoard = pDoc.GetPBoardManager().GetPBoard(event.GetPlayingBoardIndex());
    wxASSERT(pPBoard.m_bOpenBoardOnLoad);
    pDoc.CreateNewFrame(GetApp()->m_pBrdViewTmpl,
        pPBoard.GetBoard()->GetName(), &pPBoard);
}

///////////////////////////////////////////////////////////////////////
// This method handles the custom message WM_WINSTATE_RESTORE. The
// message is posted during view initial update if the state of
// the windows should be restored.

void CGsnProjView::OnMessageRestoreWinState(WinStateRestoreEvent& event)
{
    GetDocument().RestoreWindowState();
}

void CGsnProjViewContainer::OnDraw(CDC* pDC)
{
    // do nothing because child covers entire client rect
}

void CGsnProjViewContainer::OnInitialUpdate()
{
    child->OnInitialUpdate();

    BASE::OnInitialUpdate();
}

void CGsnProjViewContainer::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    child->OnUpdate(pSender, lHint, pHint);

    BASE::OnUpdate(pSender, lHint, pHint);
}

CGsnProjViewContainer::CGsnProjViewContainer() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

int CGsnProjViewContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    child = new CGsnProjView(*this);

    return 0;
}
