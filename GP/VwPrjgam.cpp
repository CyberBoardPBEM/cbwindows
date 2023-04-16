// VwPrjgam.cpp : Game Project View
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
#include    "Gamdoc.h"
#include    "Restbl.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "Trays.h"
#include    "Player.h"
#include    "MoveHist.h"

#include    "VwPrjgam.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGamProjView, CView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

const int XBORDER = 5;
const int YBORDER = 5;

const int BTN_GROUP_XGAP = 12;
const int BTN_GROUP_YGAP = 6;

const int BTN_PROJ_WD = 14 * 4;
const int BTN_PROJ_HT = 12;

const int PROJ_LIST_WIDTH = 300;

/////////////////////////////////////////////////////////////////////////////
// Button content tables - selected based on current type of item selected
//      in the project list box.

const int nNumButtons = 2;
const int nNumGroups = 8;

static UINT btnGroupNull[nNumButtons] =     // None selected
    { 0, 0 };
static UINT btnGroup0[nNumButtons] =        // Document type
    { 0, 0 };
static UINT btnGroup1[nNumButtons] =        // Board heading
    { 0, 0 };
static UINT btnGroup2[nNumButtons] =        // Boards
    { IDS_BTN_GAM_VIEW, IDS_BTN_GAM_PROPERTIES };
static UINT btnGroup3[nNumButtons] =        // History heading
    { IDS_BTN_GAM_LOAD, 0 };
static UINT btnGroup4[nNumButtons] =        // Current History
    { IDS_BTN_GAM_SAVE, IDS_BTN_GAM_DISCARD };
static UINT btnGroup5[nNumButtons] =        // Cur Playback History
    { IDS_BTN_GAM_ACCEPT, IDS_BTN_GAM_DISCARD };
static UINT btnGroup6[nNumButtons] =        // History Playback
    { IDS_BTN_GAM_DONE, 0 };
static UINT btnGroup7[nNumButtons] =        // "Past" History list
    { IDS_BTN_GAM_REPLAY, IDS_BTN_GAM_EXPORT };

static UINT * btnGroupTbl[nNumGroups + 1] =
    { btnGroupNull, btnGroup0, btnGroup1, btnGroup2, btnGroup3, btnGroup4,
      btnGroup5, btnGroup6, btnGroup7 };

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGamProjView, CView)
    //{{AFX_MSG_MAP(CGamProjView)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_LBN_SELCHANGE(IDC_V_GAM_PROJLIST, OnSelChangeProjList)
    ON_LBN_DBLCLK(IDC_V_GAM_PROJLIST, OnDblClkProjList)
    ON_BN_CLICKED(IDC_V_GAM_BTN_PRJA, OnClickedProjBtnA)
    ON_BN_CLICKED(IDC_V_GAM_BTN_PRJB, OnClickedProjBtnB)
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_EDIT_BRDPROP, OnEditBoardProperties)
    ON_UPDATE_COMMAND_UI(ID_EDIT_BRDPROP, OnUpdateEditBoardProperties)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_PPROJITEM_VIEW, OnProjItemView)
    ON_UPDATE_COMMAND_UI(ID_PPROJITEM_VIEW, OnUpdateProjItemView)
    ON_COMMAND(ID_PPROJITEM_REPLAY, OnProjItemReplay)
    ON_UPDATE_COMMAND_UI(ID_PPROJITEM_REPLAY, OnUpdateProjItemReplay)
    ON_COMMAND(ID_PPROJITEM_EXPORT, OnProjItemExport)
    ON_UPDATE_COMMAND_UI(ID_PPROJITEM_EXPORT, OnUpdateProjItemExport)
    ON_COMMAND(ID_PPROJITEM_PROPERTIES, OnProjItemProperties)
    ON_UPDATE_COMMAND_UI(ID_PPROJITEM_PROPERTIES, OnUpdateProjItemProperties)
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_SHOWPLAYINGBOARD, OnMessageShowPlayingBoard)
    ON_MESSAGE(WM_WINSTATE_RESTORE, OnMessageRestoreWinState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGamProjView

CGamProjView::CGamProjView()
{
    m_nLastSel = -1;
    m_nLastGrp = -1;
}

CGamProjView::~CGamProjView()
{
}

int CGamProjView::Find(BoardID bid) const
{
    const CGamDoc& doc = CheckedDeref(GetDocument());
    for (int i = 0 ; i < m_listProj.GetCount() ; ++i)
    {
        if (m_listProj.GetItemGroupCode(i) == grpBrd)
        {
            size_t nBrd = m_listProj.GetItemSourceCode(i);
            const CPlayBoard& pPBoard = doc.GetPBoardManager()->GetPBoard(nBrd);
            if (pPBoard.GetBoard()->GetSerialNumber() == bid)
            {
                return i;
            }
        }
    }
    ASSERT(!"unknown BoardID");
    return -1;
}

int CGamProjView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Create various controls...
    CRect rctClient;
    GetClientRect(&rctClient);

    // Buttons associated with the project listbox...
    CPoint posBtn(XBORDER, YBORDER);
    CSize  sizeBtn(BTN_PROJ_WD, BTN_PROJ_HT);
    posBtn.x = CreateButton(IDC_V_GAM_BTN_PRJA, m_btnPrjA, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out
    posBtn.x = CreateButton(IDC_V_GAM_BTN_PRJB, m_btnPrjB, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out

    // Save for list box construction...
    int xProjListRight = posBtn.x + (7 * sizeBtn.cx) / 2;

    xProjListRight = CB::max(xProjListRight, PROJ_LIST_WIDTH);

    // The project list box.
    CRect rctList(XBORDER, YBORDER, xProjListRight, YBORDER);

    if (!CreateListbox(IDC_V_GAM_PROJLIST, m_listProj,
            WS_HSCROLL | WS_VISIBLE | LBS_SORT | LBS_HASSTRINGS, rctList))
        return -1;

    // OK...now create the remaining item controls...

    rctList.left = rctList.right + BTN_GROUP_XGAP;
    rctList.right = rctClient.right - XBORDER;

    if (!CreateEditbox(IDC_V_GAM_EDITINFO, m_editInfo, rctList))
        return -1;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CGamProjView::OnInitialUpdate()
{
    GetDocument()->DoInitialUpdate();   // Since UpdateAllViews isn't virtual
    CView::OnInitialUpdate();
    CGamDoc* pDoc = GetDocument();
    CPBoardManager* pPBMgr = pDoc->GetPBoardManager();
    ASSERT(pPBMgr);
    // Only honor the open-on-load flags if the save window state
    // is disabled.
    if (!pDoc->m_bSaveWindowPositions)
    {
        for (size_t i = 0; i < pPBMgr->GetNumPBoards(); i++)
        {
            CPlayBoard& pPBoard = pPBMgr->GetPBoard(i);
            if (pPBoard.m_bOpenBoardOnLoad)
            {
                // Defer opening the view until our view init
                // in done.
                PostMessage(WM_SHOWPLAYINGBOARD, value_preserving_cast<WPARAM>(i));
            }
        }
    }
    else
        PostMessage(WM_WINSTATE_RESTORE);
}

void CGamProjView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    if (lHint == HINT_ALWAYSUPDATE || lHint == HINT_GAMPROPCHANGE ||
        lHint == HINT_GAMESTATEUSED)
        DoUpdateProjectList();
}

/////////////////////////////////////////////////////////////////////////////
// CGamProjView drawing

void CGamProjView::OnDraw(CDC* pDC)
{
    CDocument* pDoc = GetDocument();
    // TODO: add draw code here
}

BOOL CGamProjView::OnEraseBkgnd(CDC* pDC)
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
}

/////////////////////////////////////////////////////////////////////////////

void CGamProjView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    LayoutView();
}

/////////////////////////////////////////////////////////////////////////////

void CGamProjView::LayoutView()
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

    hDwp = DeferWindowPos(hDwp, m_editInfo.m_hWnd, NULL, rct.left, rct.top,
        rct.Width(), rct.Height(), SWP_NOZORDER);

    EndDeferWindowPos(hDwp);
}

/////////////////////////////////////////////////////////////////////////////
// Returns the x pos on the right of the button. relsize.cx is in 1/4 avg
// character width. relsize.cy is 1/8 char height. If returned zero,
// create failed.

int CGamProjView::CreateButton(UINT nCtrlID, CButton& btn, CPoint llpos,
    CSize relsize)
{
    CRect rct(llpos, CSize((relsize.cx * g_res.tm8ss.tmAveCharWidth) / 4,
        (relsize.cy * g_res.tm8ss.tmHeight) / 8));
    if (!btn.Create("", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
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

BOOL CGamProjView::CreateListbox(UINT nCtrlID, CListBox& lbox, DWORD dwStyle,
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

BOOL CGamProjView::CreateEditbox(UINT nCtrlID, CEdit& ebox, CRect& rct)
{
    BOOL bOk = ebox.Create(WS_CHILD | WS_BORDER | WS_VSCROLL | WS_TABSTOP |
        ES_LEFT | ES_MULTILINE | ES_READONLY, rct, this, nCtrlID);
    ::SetWindowLong(ebox.m_hWnd, GWL_EXSTYLE,
        ::GetWindowLong(ebox.m_hWnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
    ebox.SetFont(CFont::FromHandle(g_res.h8ss));
    return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// Updates buttons for specified group

void CGamProjView::UpdateButtons(int nGrp)
{
    nGrp++;                     // -1 means no selection so bump to zero
    UINT* pTbl = btnGroupTbl[nGrp];
    SetButtonState(m_btnPrjA, pTbl[0]);
    SetButtonState(m_btnPrjB, pTbl[1]);
}

void CGamProjView::SetButtonState(CButton& btn, UINT nStringID)
{
    if (nStringID == 0)
        btn.SetWindowText("");
    else
    {
        CB::string str = CB::string::LoadString(nStringID);
        btn.SetWindowText(str);
    }
    btn.EnableWindow(nStringID != 0);
}

/////////////////////////////////////////////////////////////////////////////
// Updates item controls for specified group

void CGamProjView::UpdateItemControls(int nGrp)
{
    HDWP hDwp = BeginDeferWindowPos(4);
    #define EzDefer(h, c, flg) \
        DeferWindowPos(h, c.m_hWnd, NULL, 0, 0, 0, 0, \
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | flg)
    //!!!! Really don't need this stuff yet
    hDwp = EzDefer(hDwp, m_editInfo, SWP_SHOWWINDOW);
    EndDeferWindowPos(hDwp);
}

/////////////////////////////////////////////////////////////////////////////

void CGamProjView::DoUpdateProjectList(BOOL bUpdateItem /* = TRUE */)
{
    CGamDoc* pDoc = GetDocument();
    ASSERT(pDoc);

    m_listProj.SetRedraw(FALSE);

    // Preserve the current selection
    int nTopIdx = m_listProj.GetTopIndex();
    int nCurSel = m_listProj.GetCurSel();

    m_listProj.ResetContent();

    // Document type....
    CB::string str = CB::string::LoadString(IDS_PHEAD_GAM_DOCTYPE);
    if (pDoc->HasPlayers())
    {
        if (pDoc->GetCurrentPlayerMask() == 0)
        {
            CB::string strSpec = CB::string::LoadString(IDS_PHEAD_SPECTATOR_OWNED);
            str += strSpec;
        }
        else if (pDoc->IsCurrentPlayerReferee())
        {
            CB::string strSpec = CB::string::LoadString(IDS_PHEAD_REFEREE_OWNED);
            str += strSpec;
        }
        else
        {
            CB::string strOwner = pDoc->GetPlayerManager()->GetPlayerUsingMask(
                pDoc->GetCurrentPlayerMask()).m_strName;
            CB::string strOwnedBy = CB::string::Format(IDS_TIP_OWNED_BY_PROJ, strOwner);
            str += strOwnedBy;
        }
    }
    m_listProj.AddItem(grpDoc, str);

    // Boards....
    str = CB::string::LoadString(IDS_PHEAD_GAM_BOARDS);
    m_listProj.AddItem(grpBrdHdr, str);

    CPBoardManager* pPBMgr = pDoc->GetPBoardManager();
    ASSERT(pPBMgr);
    for (size_t i = 0; i < pPBMgr->GetNumPBoards(); i++)
    {
        static int bDisplayIDs = -1;
        if (bDisplayIDs == -1)
        {
            bDisplayIDs = GetApp()->GetProfileInt("Settings", "DisplayIDs", 0);
        }
        CPlayBoard& pPBoard = pPBMgr->GetPBoard(i);
        str = pPBoard.GetBoard()->GetName().mfc_str();
        if (bDisplayIDs)
        {
            CB::string strTmp = std::move(str);
            str = std::format("[{}] {}",
                pPBMgr->GetPBoard(i).GetBoard()->GetSerialNumber(), strTmp);
        }
        if (pPBoard.IsOwned())
        {
            CB::string strOwner = pDoc->GetPlayerManager()->GetPlayerUsingMask(
                pPBoard.GetOwnerMask()).m_strName;
            CB::string strOwnedBy = CB::string::Format(IDS_TIP_OWNED_BY_PROJ, strOwner);
            str += strOwnedBy;
        }
        m_listProj.AddItem(grpBrd, str, i);
    }

    // History Heading....
    str = CB::string::LoadString(IDS_PHEAD_GAM_HISTORY);
    m_listProj.AddItem(grpHistHdr, str);

    // Current History....!!!!!ACCOUNT FOR PLAYBACK!!!!!
    if (pDoc->GetRecordMoveList() != NULL)
    {
        //if (pDoc->IsRecording())
        if (pDoc->GetGameState() == CGamDoc::stateRecording)
        {
            str = CB::string::LoadString(IDS_PHEAD_GAM_CURRENT);
            m_listProj.AddItem(grpCurHist, str, size_t(0));
        }
        else if (pDoc->GetGameState() == CGamDoc::stateHistPlay)
        {
            str = CB::string::LoadString(IDS_PHEAD_GAM_SUSPEND);
            m_listProj.AddItem(grpCurHist, str, size_t(0));
        }
        else
        {
            ASSERT(pDoc->GetGameState() == CGamDoc::stateMovePlay);
            str = CB::string::LoadString(IDS_PHEAD_GAM_MOVEFILE);
            if (!pDoc->m_pPlayHist->m_strTitle.empty())
                str += " - " + pDoc->m_pPlayHist->m_strTitle;
            m_listProj.AddItem(grpCurPlay, str, 0);
        }
    }
    if (pDoc->GetGameState() == CGamDoc::stateHistPlay)
    {
        str = CB::string::LoadString(IDS_PHEAD_GAM_HISTPLAY);
        m_listProj.AddItem(grpHistPlay, str, size_t(0));
    }

    // Load rest of game history
    m_listProj.MarkGroupItem();
    CHistoryTable* pHTbl = pDoc->GetHistoryTable();

    if (pHTbl != NULL)
    {
        // TODO:  why not use locale's time instead of US?
        CB::string strTimeFmt = CB::string::LoadString(IDS_GAME_USA_TIMEFMT);
        for (size_t i = size_t(0); i < pHTbl->GetNumHistRecords(); i++)
        {
            CHistRecord& pRcd = pHTbl->GetHistRecord(i);
            CB::string str = pRcd.m_timeAbsorbed.Format(strTimeFmt);
            str += " - ";
            str += pRcd.m_strTitle;
            m_listProj.AddSeqItem(grpHist, str, value_preserving_cast<int>(i), i);
            if (pDoc->IsPlayingHistory() &&
                pDoc->GetCurrentHistoryRecNum() == i)
            {
                m_listProj.MarkGroupItem(grpHist, i);
            }
        }
    }

    // OK...Show the updates
    m_listProj.SetRedraw(TRUE);
    m_listProj.Invalidate();

    m_listProj.SetTopIndex(nTopIdx);
    if (nCurSel >= 0)
    {
        if (nCurSel >= m_listProj.GetCount())
            nCurSel = m_listProj.GetCount() - 1;
        m_listProj.SetCurSel(nCurSel);
    }
    else
        m_listProj.SetCurSel(0);

    if (bUpdateItem)
    {
        m_nLastSel = -1;
        m_nLastGrp = -1;
        OnSelChangeProjList();
    }
}

/////////////////////////////////////////////////////////////////////////////
// List box notifications

void CGamProjView::OnSelChangeProjList()
{
    int nSel = m_listProj.GetCurSel();
    if (m_nLastSel == nSel)
        return;

    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoUpdateGamInfo(); break;
        case grpBrdHdr: DoUpdateBoardHelpInfo(); break;
        case grpBrd:    DoUpdateBoardInfo(); break;
        case grpHistHdr:DoUpdateHistoryHelpInfo(); break;
        case grpCurPlay:DoUpdateCurPlayInfo(); break;
        case grpHist:   DoUpdateHistoryInfo(); break;
        default:
            m_editInfo.SetWindowText("");
    }
    if (nGrp != m_nLastGrp)
    {
        UpdateItemControls(nGrp);
        UpdateButtons(nGrp);
        m_nLastGrp = nGrp;
    }
}

void CGamProjView::OnDblClkProjList()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0)
        return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardView(); break;
        case grpHistHdr:DoHistoryLoad(); break;
        case grpCurHist:DoHistorySave(); break;
        case grpHist:   DoHistoryReplay(); break;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Button notifications

void CGamProjView::OnClickedProjBtnA()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardView(); break;
        case grpHistHdr:DoHistoryLoad(); break;
        case grpCurHist:DoHistorySave(); break;
        case grpCurPlay:DoHistoryDone(); break;
        case grpHistPlay: DoHistoryReplayDone(); break;
        case grpHist:   DoHistoryReplay(); break;
    }
}

void CGamProjView::OnClickedProjBtnB()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardProperty(); break;
        case grpHist:   DoHistoryExport(); break;
        case grpCurHist:DoHistoryDiscard(); break;
        case grpCurPlay:DoHistoryDiscard(); break;
    }
}

void CGamProjView::OnEditBoardProperties()
{
    DoBoardProperty();
}

void CGamProjView::OnUpdateEditBoardProperties(CCmdUI* pCmdUI)
{
    int nSel = m_listProj.GetCurSel();
    pCmdUI->Enable(nSel >= 0 &&
        m_listProj.GetItemGroupCode(nSel) == grpBrd);
}

void CGamProjView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    // Make sure window is active.
    GetParentFrame()->ActivateFrame();
    UINT nID = (UINT)-1;

    if (pWnd->GetDlgCtrlID() == IDC_V_GAM_PROJLIST)
        nID = MENU_PJ_GAM_DEFAULT;

    if ((int)nID < 0)
        return;

    CMenu bar;
    if (bar.LoadMenu(IDR_MENU_PLAYER_POPUPS))
    {
        CMenu& popup = *bar.GetSubMenu(nID);
        ASSERT(popup.m_hMenu != NULL);

        // Make sure we clean up even if exception is tossed.
        TRY
        {
            popup.TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y,
                AfxGetMainWnd()); // Route commands through main window
            // Make sure command is dispatched BEFORE we clear m_bInRightMouse.
            GetApp()->DispatchMessages();
        }
        END_TRY
    }
}

void CGamProjView::OnProjItemProperties()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0)
        return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoGamProperty(); break;
        case grpBrd:    DoBoardProperty(); break;
    }
}

void CGamProjView::OnUpdateProjItemProperties(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj.GetCurSel();
    if (nSel >= 0)
    {
        int nGrp = m_listProj.GetItemGroupCode(nSel);
        ASSERT(nGrp >= 0);
        switch (nGrp)
        {
            case grpDoc:
            case grpBrd:
                bEnable = TRUE;
                break;
            default: ;
        }
    }
    pCmdUI->Enable(bEnable);
}
void CGamProjView::OnProjItemView()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    if (nGrp == grpBrd)
        DoBoardView();
}

void CGamProjView::OnUpdateProjItemView(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj.GetCurSel();
    if (nSel >= 0)
    {
        int nGrp = m_listProj.GetItemGroupCode(nSel);
        ASSERT(nGrp >= 0);
        if (nGrp == grpBrd)
            bEnable = TRUE;
    }
    pCmdUI->Enable(bEnable);
}

void CGamProjView::OnProjItemReplay()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    if (nGrp == grpHist)
        DoHistoryReplay();
}

void CGamProjView::OnUpdateProjItemReplay(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj.GetCurSel();
    if (nSel >= 0)
    {
        int nGrp = m_listProj.GetItemGroupCode(nSel);
        ASSERT(nGrp >= 0);
        if (nGrp == grpHist)
            bEnable = TRUE;
    }
    pCmdUI->Enable(bEnable);
}

void CGamProjView::OnProjItemExport()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    if (nGrp == grpHist)
        DoHistoryExport();
}

void CGamProjView::OnUpdateProjItemExport(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj.GetCurSel();
    if (nSel >= 0)
    {
        int nGrp = m_listProj.GetItemGroupCode(nSel);
        ASSERT(nGrp >= 0);
        if (nGrp == grpHist)
            bEnable = TRUE;
    }
    pCmdUI->Enable(bEnable);
}

///////////////////////////////////////////////////////////////////////
// This method handles the cutom message WM_SHOWPLAYINGBOARD. The
// message is posted during view initial update if the playing
// board m_bOpenBoardOnLoad option is set. (wParam = the board index)

LRESULT CGamProjView::OnMessageShowPlayingBoard(WPARAM wParam, LPARAM)
{
    CGamDoc* pDoc = GetDocument();
    CPlayBoard& pPBoard = pDoc->GetPBoardManager()->GetPBoard(value_preserving_cast<size_t>(wParam));
    ASSERT(pPBoard.m_bOpenBoardOnLoad);
    pDoc->CreateNewFrame(GetApp()->m_pBrdViewTmpl,
        pPBoard.GetBoard()->GetName(), &pPBoard);
    return (LRESULT)0;
}

///////////////////////////////////////////////////////////////////////
// This method handles the custom message WM_WINSTATE_RESTORE. The
// message is posted during view initial update if the state of
// the windows should be restored.

LRESULT CGamProjView::OnMessageRestoreWinState(WPARAM, LPARAM)
{
    GetDocument()->RestoreWindowState();
    return (LRESULT)0;
}


