// VwPrjgbx.cpp : implementation file
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
#include    <afxadv.h>
#include    "Gm.h"
#include    "GmDoc.h"
#include    "ResTbl.h"
#include    "Tile.h"
#include    "Board.h"
#include    "Pieces.h"
#include    "Marks.h"
#include    "DibApi.h"          // For CopyHandle()

#include    "VwPrjgbx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGbxProjView, CView)

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

const int BTN_ITEM_WD = 10 * 4;
const int BTN_ITEM_HT = 12;

const int PROJ_LIST_WIDTH = 200;

/////////////////////////////////////////////////////////////////////////////
// Button content tables - selected based on current type of item selected
//      in the project list box.

const int nNumButtons = 6;
const int nNumGroups = 9;

static UINT btnGroupNull[nNumButtons] = // None selected
    { 0, 0, 0, 0, 0, 0, };
static UINT btnGroup0[nNumButtons] =        // Document heading
    { IDS_BTN_PROPERTIES, 0, 0, 0, 0, 0 };
static UINT btnGroup1[nNumButtons] =        // Board heading
    { IDS_BTN_CREATE, 0, 0, 0, 0, 0 };
static UINT btnGroup2[nNumButtons] =        // Boards
    { IDS_BTN_PROPERTIES, IDS_BTN_DELETE, IDS_BTN_EDIT, 0, 0, 0 };
static UINT btnGroup3[nNumButtons] =        // Tile heading
    { IDS_BTN_CREATE, IDS_BTN_PROPERTIES, 0, 0, 0, 0 };
static UINT btnGroup4[nNumButtons] =        // Tile groups
    { IDS_BTN_PROPERTIES, IDS_BTN_DELETE, IDS_BTN_NEW, IDS_BTN_EDIT,
      IDS_BTN_CLONE, IDS_BTN_DELETE };
static UINT btnGroup5[nNumButtons] =        // Piece heading
    { IDS_BTN_CREATE, 0, 0, 0, 0, 0 };
static UINT btnGroup6[nNumButtons] =        // Piece groups
    { IDS_BTN_PROPERTIES, IDS_BTN_DELETE, IDS_BTN_NEW, IDS_BTN_EDIT,
      IDS_BTN_DELETE, 0 };
static UINT btnGroup7[nNumButtons] =        // Marker heading
    { IDS_BTN_CREATE, 0, 0, 0, 0, 0 };
static UINT btnGroup8[nNumButtons] =        // Marker groups
    { IDS_BTN_PROPERTIES, IDS_BTN_DELETE, IDS_BTN_NEW, IDS_BTN_EDIT,
      IDS_BTN_DELETE, 0 };

static UINT * btnGroupTbl[nNumGroups + 1] =
    { btnGroupNull, btnGroup0, btnGroup1, btnGroup2, btnGroup3, btnGroup4,
      btnGroup5, btnGroup6, btnGroup7, btnGroup8 };

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGbxProjView, CView)
    //{{AFX_MSG_MAP(CGbxProjView)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_LBN_SELCHANGE(IDC_V_PROJLIST, OnSelChangeProjList)
    ON_LBN_DBLCLK(IDC_V_PROJLIST, OnDblClkProjList)
    ON_LBN_DBLCLK(IDC_V_TILELIST, OnDblClkTileList)
    ON_LBN_DBLCLK(IDC_V_PIECELIST, OnDblClkPieceList)
    ON_LBN_DBLCLK(IDC_V_MARKLIST, OnDblClkMarkList)
    ON_BN_CLICKED(IDC_V_BTN_PRJA, OnClickedProjBtnA)
    ON_BN_CLICKED(IDC_V_BTN_PRJB, OnClickedProjBtnB)
    ON_BN_CLICKED(IDC_V_BTN_ITMA, OnClickedItemBtnA)
    ON_BN_CLICKED(IDC_V_BTN_ITMB, OnClickedItemBtnB)
    ON_BN_CLICKED(IDC_V_BTN_ITMC, OnClickedItemBtnC)
    ON_BN_CLICKED(IDC_V_BTN_ITMD, OnClickedItemBtnD)
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
    ON_COMMAND(ID_EDIT_MOVE, OnEditMove)
    ON_UPDATE_COMMAND_UI(ID_EDIT_MOVE, OnUpdateEditMove)
    ON_COMMAND(ID_PROJECT_SAVETILEFILE, OnProjectSaveTileFile)
    ON_UPDATE_COMMAND_UI(ID_PROJECT_SAVETILEFILE, OnUpdateProjectSaveTileFile)
    ON_COMMAND(ID_PROJECT_LOADTILEFILE, OnProjectLoadTileFile)
    ON_UPDATE_COMMAND_UI(ID_PROJECT_LOADTILEFILE, OnUpdateProjectLoadTileFile)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_PROJITEM_PROPERTIES, OnProjItemProperties)
    ON_UPDATE_COMMAND_UI(ID_PROJITEM_PROPERTIES, OnUpdateProjItemProperties)
    ON_COMMAND(ID_TILE_CLONE, OnTileClone)
    ON_UPDATE_COMMAND_UI(ID_TILE_CLONE, OnUpdateTileClone)
    ON_COMMAND(ID_TILE_DELETE, OnTileDelete)
    ON_UPDATE_COMMAND_UI(ID_TILE_DELETE, OnUpdateTileDelete)
    ON_COMMAND(ID_TILE_EDIT, OnTileEdit)
    ON_UPDATE_COMMAND_UI(ID_TILE_EDIT, OnUpdateTileEdit)
    ON_COMMAND(ID_TILE_NEW, OnTileNew)
    ON_COMMAND(ID_PROJITEM_DELETE, OnProjItemDelete)
    ON_UPDATE_COMMAND_UI(ID_PROJITEM_DELETE, OnUpdateProjItemDelete)
    ON_COMMAND(ID_PIECE_NEW, OnPieceNew)
    ON_COMMAND(ID_PIECE_EDIT, OnPieceEdit)
    ON_UPDATE_COMMAND_UI(ID_PIECE_EDIT, OnUpdatePieceEdit)
    ON_COMMAND(ID_PIECE_DELETE, OnPieceDelete)
    ON_UPDATE_COMMAND_UI(ID_PIECE_DELETE, OnUpdatePieceDelete)
    ON_COMMAND(ID_MARKER_NEW, OnMarkerNew)
    ON_COMMAND(ID_MARKER_EDIT, OnMarkerEdit)
    ON_UPDATE_COMMAND_UI(ID_MARKER_EDIT, OnUpdateMarkerEdit)
    ON_COMMAND(ID_MARKER_DELETE, OnMarkerDelete)
    ON_UPDATE_COMMAND_UI(ID_MARKER_DELETE, OnUpdateMarkerDelete)
    ON_COMMAND(ID_PROJECT_CLONEBOARD, OnProjectCloneBoard)
    ON_UPDATE_COMMAND_UI(ID_PROJECT_CLONEBOARD, OnUpdateProjectCloneBoard)
    //}}AFX_MSG_MAP
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragItem)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGbxProjView

CGbxProjView::CGbxProjView()
{
    m_nLastSel = -1;
    m_nLastGrp = -1;
}

CGbxProjView::~CGbxProjView()
{
}

int CGbxProjView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Create various controls...
    CRect rctClient;
    GetClientRect(&rctClient);

    // Buttons associated with the project listbox...
    CPoint posBtn(XBORDER, YBORDER);
    CSize  sizeBtn(BTN_PROJ_WD, BTN_PROJ_HT);
    posBtn.x = CreateButton(IDC_V_BTN_PRJA, m_btnPrjA, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out
    posBtn.x = CreateButton(IDC_V_BTN_PRJB, m_btnPrjB, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out

    int xProjListRight = posBtn.x - 1;  // Save for list box construction

    xProjListRight = CB::max(xProjListRight, PROJ_LIST_WIDTH);

    // Buttons associated with the project item area...
    posBtn.x = BTN_GROUP_XGAP + xProjListRight;
    sizeBtn = CSize(BTN_ITEM_WD, BTN_ITEM_HT);

    posBtn.x = CreateButton(IDC_V_BTN_ITMA, m_btnItmA, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out
    posBtn.x = CreateButton(IDC_V_BTN_ITMB, m_btnItmB, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out
    posBtn.x = CreateButton(IDC_V_BTN_ITMC, m_btnItmC, posBtn, sizeBtn);
    if (posBtn.x == 0) return -1;       // Error...bail out
    if (CreateButton(IDC_V_BTN_ITMD, m_btnItmD, posBtn, sizeBtn) == 0)
        return -1;                      // Error...bail out

    // The project list box.
    CRect rctList(XBORDER, YBORDER, xProjListRight, YBORDER);

    if (!CreateListbox(IDC_V_PROJLIST, m_listProj,
            WS_HSCROLL | WS_VISIBLE | LBS_SORT | LBS_HASSTRINGS, rctList))
        return -1;

    // OK...now create the remaining item lists.

    rctList.left = rctList.right + BTN_GROUP_XGAP;
    rctList.right = rctClient.right - XBORDER;

    if (!CreateListbox(IDC_V_TILELIST, m_listTiles,
            LBS_EXTENDEDSEL | LBS_HASSTRINGS, rctList))
        return -1;
    m_listTiles.SetDrawAllScales(TRUE);
    m_listTiles.EnableDrag();
    m_listTiles.EnableSelfDrop();
    m_listTiles.EnableDropScroll();


    if (!CreateListbox(IDC_V_PIECELIST, m_listPieces, LBS_HASSTRINGS | LBS_EXTENDEDSEL, rctList))
        return -1;
    if (!CreateListbox(IDC_V_MARKLIST, m_listMarks, LBS_HASSTRINGS | LBS_EXTENDEDSEL, rctList))
        return -1;

    // Create an informational text box.
    if (!CreateEditbox(IDC_V_EDITINFO, m_editInfo, rctList))
        return -1;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CGbxProjView::OnInitialUpdate()
{
    m_listTiles.SetDocument(GetDocument());
    m_listPieces.SetDocument(CheckedDeref(GetDocument()));
    m_listMarks.SetDocument(GetDocument());

    CView::OnInitialUpdate();
}

void CGbxProjView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    DoUpdateProjectList();
//  CView::OnUpdate(pSender, lHint, pHint);
}

/////////////////////////////////////////////////////////////////////////////
// CGbxProjView drawing

void CGbxProjView::OnDraw(CDC* pDC)
{
    CDocument* pDoc = GetDocument();
    // TODO: add draw code here
}

BOOL CGbxProjView::OnEraseBkgnd(CDC* pDC)
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

HBRUSH CGbxProjView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    return CView::OnCtlColor(pDC, pWnd, nCtlColor);
}

/////////////////////////////////////////////////////////////////////////////

void CGbxProjView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    LayoutView();
}

/////////////////////////////////////////////////////////////////////////////

void CGbxProjView::LayoutView()
{
    CRect rctClient;
    GetClientRect(&rctClient);
    HDWP hDwp = BeginDeferWindowPos(16);

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

    // Item button group...
    m_btnItmA.GetWindowRect(&rct);
    ScreenToClient(&rct);
    hDwp = DeferWindowPos(hDwp, m_btnItmA.m_hWnd, NULL, rct.left, yBtnTop,
        0, 0, SWP_NOSIZE | SWP_NOZORDER);
    m_btnItmB.GetWindowRect(&rct);
    ScreenToClient(&rct);
    hDwp = DeferWindowPos(hDwp, m_btnItmB.m_hWnd, NULL, rct.left, yBtnTop,
        0, 0, SWP_NOSIZE | SWP_NOZORDER);
    m_btnItmC.GetWindowRect(&rct);
    ScreenToClient(&rct);
    hDwp = DeferWindowPos(hDwp, m_btnItmC.m_hWnd, NULL, rct.left, yBtnTop,
        0, 0, SWP_NOSIZE | SWP_NOZORDER);
    m_btnItmD.GetWindowRect(&rct);
    ScreenToClient(&rct);
    hDwp = DeferWindowPos(hDwp, m_btnItmD.m_hWnd, NULL, rct.left, yBtnTop,
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

    hDwp = DeferWindowPos(hDwp, m_listTiles.m_hWnd, NULL, rct.left, rct.top,
        rct.Width(), rct.Height(), SWP_NOZORDER);
    hDwp = DeferWindowPos(hDwp, m_listPieces.m_hWnd, NULL, rct.left, rct.top,
        rct.Width(), rct.Height(), SWP_NOZORDER);
    hDwp = DeferWindowPos(hDwp, m_listMarks.m_hWnd, NULL, rct.left, rct.top,
        rct.Width(), rct.Height(), SWP_NOZORDER);
    hDwp = DeferWindowPos(hDwp, m_editInfo.m_hWnd, NULL, rct.left, rct.top,
        rct.Width(), rct.Height(), SWP_NOZORDER);

    EndDeferWindowPos(hDwp);
}

/////////////////////////////////////////////////////////////////////////////
// Returns the x pos on the right of the button. relsize.cx is in 1/4 avg
// character width. relsize.cy is 1/8 char height. If returned zero,
// create failed.

int CGbxProjView::CreateButton(UINT nCtrlID, CButton& btn, CPoint llpos,
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

BOOL CGbxProjView::CreateListbox(UINT nCtrlID, CListBox& lbox, DWORD dwStyle,
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

BOOL CGbxProjView::CreateEditbox(UINT nCtrlID, CEdit& ebox, CRect& rct)
{
    BOOL bOk = ebox.Create(WS_CHILD | WS_BORDER | WS_VSCROLL | WS_TABSTOP |
        ES_LEFT | ES_MULTILINE | ES_READONLY, rct, this, nCtrlID);
    ::SetWindowLong(ebox.m_hWnd, GWL_EXSTYLE,
        ::GetWindowLong(ebox.m_hWnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
    ebox.SetFont(CFont::FromHandle(g_res.h8ss));
    return bOk;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CGbxProjView::OnDragItem(WPARAM wParam, LPARAM lParam)
{
    CGamDoc* pDoc = GetDocument();
    ASSERT(pDoc);

    DragInfo* pdi = (DragInfo*)lParam;

    if (pdi->m_dragType != DRAG_TILELIST)
        return 0;               // Only tile list drops allowed

    if (pdi->GetSubInfo<DRAG_TILELIST>().m_gamDoc != pDoc)
        return 0;               // Only pieces from our document.

    CRect rct;
    m_listTiles.GetClientRect(&rct);
    if (!rct.PtInRect(pdi->m_pointClient))
        return 0;

    if (wParam == phaseDragOver)
        return (LRESULT)(LPVOID)pdi->m_hcsrSuggest;
    else if (wParam == phaseDragDrop && pdi->m_dragType == DRAG_TILELIST)
    {
        int nProjSel = m_listProj.GetCurSel();
        ASSERT(nProjSel >= 0 &&
            m_listProj.GetItemGroupCode(nProjSel) == grpTile);
        size_t nGrpSel = m_listProj.GetItemSourceCode(nProjSel);

        CTileManager* pTMgr = pDoc->GetTileManager();
        const CTileSet& pTGrp = pTMgr->GetTileSet(nGrpSel);

        // Force selection of item under the mouse
        m_listTiles.SetSelFromPoint(pdi->m_point);
        int nSel = m_listTiles.GetCurSel();

        if (nSel >= 0)
        {
            // Check if the mouse is above or below the half point.
            // If above, insert before. If below, insert after.
            CRect rct;
            m_listTiles.GetItemRect(nSel, &rct);
            if (pdi->m_point.y > (rct.top + rct.bottom) / 2)
                nSel++;
        }

        pTMgr->MoveTileIDsToTileSet(nGrpSel, *pdi->GetSubInfo<DRAG_TILELIST>().m_tileIDList, value_preserving_cast<size_t>(nSel));
        DoUpdateTileList();
        GetDocument()->NotifyTileDatabaseChange();
        m_listTiles.SetCurSelsMapped(*pdi->GetSubInfo<DRAG_TILELIST>().m_tileIDList);
        m_listTiles.ShowFirstSelection();
        pDoc->SetModifiedFlag();
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Updates buttons for specified group

void CGbxProjView::UpdateButtons(int nGrp)
{
    nGrp++;                     // -1 means no selection so bump to zero
    UINT* pTbl = btnGroupTbl[nGrp];
    SetButtonState(m_btnPrjA, pTbl[0]);
    SetButtonState(m_btnPrjB, pTbl[1]);
    SetButtonState(m_btnItmA, pTbl[2]);
    SetButtonState(m_btnItmB, pTbl[3]);
    SetButtonState(m_btnItmC, pTbl[4]);
    SetButtonState(m_btnItmD, pTbl[5]);
}

void CGbxProjView::SetButtonState(CButton& btn, UINT nStringID)
{
    if (nStringID == 0)
        btn.SetWindowText("");
    else
    {
        CString str;
        str.LoadString(nStringID);
        btn.SetWindowText(str);
    }
    btn.EnableWindow(nStringID != 0);
}

/////////////////////////////////////////////////////////////////////////////
// Updates item controls for specified group

void CGbxProjView::UpdateItemControls(int nGrp)
{
    HDWP hDwp = BeginDeferWindowPos(8);
    #define EzDefer(h, c, flg) \
        DeferWindowPos(h, c.m_hWnd, NULL, 0, 0, 0, 0, \
            SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | flg)

    if (nGrp == grpTile)            // Tiles
    {
        hDwp = EzDefer(hDwp, m_listTiles, SWP_SHOWWINDOW);
        hDwp = EzDefer(hDwp, m_listPieces, SWP_HIDEWINDOW);
        hDwp = EzDefer(hDwp, m_listMarks, SWP_HIDEWINDOW);
        hDwp = EzDefer(hDwp, m_editInfo, SWP_HIDEWINDOW);
    }
    else if (nGrp == grpPce)        // Pieces
    {
        hDwp = EzDefer(hDwp, m_listPieces, SWP_SHOWWINDOW);
        hDwp = EzDefer(hDwp, m_listTiles, SWP_HIDEWINDOW);
        hDwp = EzDefer(hDwp, m_listMarks, SWP_HIDEWINDOW);
        hDwp = EzDefer(hDwp, m_editInfo, SWP_HIDEWINDOW);
    }
    else if (nGrp == grpMark)       // Markers
    {
        hDwp = EzDefer(hDwp, m_listMarks, SWP_SHOWWINDOW);
        hDwp = EzDefer(hDwp, m_listTiles, SWP_HIDEWINDOW);
        hDwp = EzDefer(hDwp, m_listPieces, SWP_HIDEWINDOW);
        hDwp = EzDefer(hDwp, m_editInfo, SWP_HIDEWINDOW);
    }
    else    // Board, headings and no selection
    {
        hDwp = EzDefer(hDwp, m_editInfo, SWP_SHOWWINDOW);
        hDwp = EzDefer(hDwp, m_listTiles, SWP_HIDEWINDOW);
        hDwp = EzDefer(hDwp, m_listPieces, SWP_HIDEWINDOW);
        hDwp = EzDefer(hDwp, m_listMarks, SWP_HIDEWINDOW);
    }
    EndDeferWindowPos(hDwp);
}

/////////////////////////////////////////////////////////////////////////////

void CGbxProjView::DoUpdateProjectList(BOOL bUpdateItem /* = TRUE */)
{
    CGamDoc* pDoc = GetDocument();
    ASSERT(pDoc);

    m_listProj.SetRedraw(FALSE);

    // Preserve the current selection
    int nTopIdx = m_listProj.GetTopIndex();
    int nCurSel = m_listProj.GetCurSel();

    m_listProj.ResetContent();

    // Document type....
    CString str;
    str.LoadString(IDS_PHEAD_DOCTYPE);
    m_listProj.AddItem(grpDoc, str);

    // Boards....
    str.LoadString(IDS_PHEAD_BOARDS);
    m_listProj.AddItem(grpBHdr, str);

    CBoardManager* pBMgr = pDoc->GetBoardManager();
    ASSERT(pBMgr);
    for (size_t i = 0; i < pBMgr->GetNumBoards(); i++)
    {
        static int bDisplayIDs = -1;
        if (bDisplayIDs == -1)
        {
            bDisplayIDs = GetApp()->GetProfileInt("Settings", "DisplayIDs", 0);
        }
        str = pBMgr->GetBoard(i).GetName();
        if (bDisplayIDs)
        {
            CString strTmp = str;
            str.Format("[%u] %s",
                value_preserving_cast<unsigned>(static_cast<BoardID::UNDERLYING_TYPE>(pBMgr->GetBoard(i).GetSerialNumber())), (LPCTSTR)strTmp);
        }
        m_listProj.AddItem(grpBrd, str, i);
    }

    // Tiles....
    str.LoadString(IDS_PHEAD_TILES);
    m_listProj.AddItem(grpTHdr, str);

    CTileManager* pTMgr = pDoc->GetTileManager();
    ASSERT(pTMgr);
    for (size_t i = 0; i < pTMgr->GetNumTileSets(); i++)
        m_listProj.AddItem(grpTile, pTMgr->GetTileSet(i).GetName(), i);

    // Pieces....
    str.LoadString(IDS_PHEAD_PIECES);
    m_listProj.AddItem(grpPHdr, str);

    CPieceManager* pPMgr = pDoc->GetPieceManager();
    ASSERT(pPMgr);
    for (size_t i = 0; i < pPMgr->GetNumPieceSets(); i++)
        m_listProj.AddItem(grpPce, pPMgr->GetPieceSet(i).GetName(), i);

    // Marks....
    str.LoadString(IDS_PHEAD_MARKS);
    m_listProj.AddItem(grpMHdr, str);

    CMarkManager* pMMgr = pDoc->GetMarkManager();
    ASSERT(pMMgr);
    for (size_t i = 0; i < pMMgr->GetNumMarkSets(); i++)
        m_listProj.AddItem(grpMark, pMMgr->GetMarkSet(i).GetName(), i);

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

void CGbxProjView::OnSelChangeProjList()
{
    int nSel = m_listProj.GetCurSel();
    if (m_nLastSel == nSel)
        return;

    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoUpdateGbxInfo(); break;
        case grpBHdr:   DoUpdateBoardHelpInfo(); break;
        case grpBrd:    DoUpdateBoardInfo(); break;
        case grpTHdr:   DoUpdateTileHelpInfo(); break;
        case grpTile:   DoUpdateTileList(); break;
        case grpPHdr:   DoUpdatePieceHelpInfo(); break;
        case grpPce:    DoUpdatePieceList(); break;
        case grpMHdr:   DoUpdateMarkHelpInfo(); break;
        case grpMark:   DoUpdateMarkList(); break;
    }
    if (nGrp != m_nLastGrp)
    {
        UpdateItemControls(nGrp);
        UpdateButtons(nGrp);
        m_nLastGrp = nGrp;
    }
}

void CGbxProjView::OnDblClkProjList()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0)
        return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoGbxProperty(); break;
        case grpBHdr:   DoBoardCreate(); break;
        case grpBrd:    DoBoardEdit(); break;
        case grpTHdr:   DoTileGroupCreate(); break;
        case grpTile:   DoTileGroupProperty(); break;
        case grpPHdr:   DoPieceGroupCreate(); break;
        case grpPce:    DoPieceGroupProperty(); break;
        case grpMHdr:   DoMarkGroupCreate(); break;
        case grpMark:   DoMarkGroupProperty(); break;
    }
}

void CGbxProjView::OnDblClkTileList()
{
    DoTileEdit();
}

void CGbxProjView::OnDblClkPieceList()
{
    if (m_listPieces.GetSelCount() > 0)
        DoPieceEdit();
}

void CGbxProjView::OnDblClkMarkList()
{
    DoMarkEdit();
}

/////////////////////////////////////////////////////////////////////////////
// Button notifications

void CGbxProjView::OnClickedProjBtnA()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoGbxProperty(); break;
        case grpBHdr:   DoBoardCreate(); break;
        case grpBrd:    DoBoardProperty(); break;
        case grpTHdr:   DoTileGroupCreate(); break;
        case grpTile:   DoTileGroupProperty(); break;
        case grpPHdr:   DoPieceGroupCreate(); break;
        case grpPce:    DoPieceGroupProperty(); break;
        case grpMHdr:   DoMarkGroupCreate(); break;
        case grpMark:   DoMarkGroupProperty(); break;
    }
}

void CGbxProjView::OnClickedProjBtnB()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardDelete(); break;
        case grpTHdr:   DoTileManagerProperty(); break;
        case grpTile:   DoTileGroupDelete(); break;
        case grpPce:    DoPieceGroupDelete(); break;
        case grpMark:   DoMarkGroupDelete(); break;
    }
}

void CGbxProjView::OnClickedItemBtnA()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardEdit(); break;
        case grpTile:   DoTileNew(); break;
        case grpPce:    DoPieceNew(); break;
        case grpMark:   DoMarkNew(); break;
    }
}

void CGbxProjView::OnClickedItemBtnB()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpTile:   DoTileEdit(); break;
        case grpPce:    DoPieceEdit(); break;
        case grpMark:   DoMarkEdit(); break;
    }
}

void CGbxProjView::OnClickedItemBtnC()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpTile:   DoTileClone(); break;
        case grpPce:    DoPieceDelete(); break;
        case grpMark:   DoMarkDelete(); break;
    }
}

void CGbxProjView::OnClickedItemBtnD()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0) return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    if (nGrp == grpTile)
        DoTileDelete();
}

void CGbxProjView::OnEditCopy()
{
#ifdef _DEBUG
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpTile &&
        m_listTiles.GetSelCount() > 0);
#endif

    CGamDoc* pDoc = GetDocument();
    CTileManager* pTMgr = pDoc->GetTileManager();

    std::vector<TileID> tidtbl = m_listTiles.GetCurMappedItemList();

    BeginWaitCursor();
    TRY
    {
        CSharedFile file;
        CArchive ar(&file, CArchive::store);
        pTMgr->CopyTileImagesToArchive(ar, tidtbl);
        ar.Close();

        CSharedFile file2;
        CArchive ar2(&file2, CArchive::store);
        ar2 << tidtbl;
        ar2.Close();

        CSharedFile file3(GMEM_DDESHARE | GMEM_MOVEABLE, 16);
        CArchive ar3(&file3, CArchive::store, 16);
        ar3 << pDoc->GetGameBoxID();
        ar3 << GetCurrentProcessId();// To distinguish between same GBox in other files
        ar3.Close();

        if (OpenClipboard())
        {
            EmptyClipboard();
            SetClipboardData(CF_TILEIMAGES, file.Detach());
            SetClipboardData(CF_TIDLIST, file2.Detach());
            SetClipboardData(CF_GBOXID, file3.Detach());
            CloseClipboard();
        }
    }
    END_TRY
    EndWaitCursor();
}

void CGbxProjView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
    int nSel = m_listProj.GetCurSel();
    pCmdUI->Enable(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpTile &&
        m_listTiles.GetSelCount() > 0);
}

void CGbxProjView::OnEditPaste()
{
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpTile &&
        IsClipboardFormatAvailable(CF_TILEIMAGES));
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CGamDoc* pDoc = GetDocument();
    CTileManager* pTMgr = pDoc->GetTileManager();

    BeginWaitCursor();
    TRY
    {
        CSharedFile file;
        std::vector<TileID> tidtbl;
        if (OpenClipboard())
        {
            file.SetHandle(CopyHandle(::GetClipboardData(CF_TILEIMAGES)), FALSE);
            CloseClipboard();

            CArchive ar(&file, CArchive::load);
            int nCurSel = m_listTiles.GetTopSelectedItem();
            size_t nCurSel2 = nCurSel == LB_ERR ? Invalid_v<size_t> : value_preserving_cast<size_t>(nCurSel);
            pTMgr->CreateTilesFromTileImageArchive(ar, nGrp, &tidtbl, nCurSel2);
            ar.Close();
        }
        DoUpdateTileList();
        pDoc->NotifyTileDatabaseChange();
        for (size_t i = size_t(0); i < tidtbl.size(); i++)
        {
            CGmBoxHint hint;
            hint.GetArgs<HINT_TILECREATED>().m_tid = tidtbl[i];
            pDoc->UpdateAllViews(NULL, HINT_TILECREATED, &hint);
        }
        m_listTiles.SetCurSelsMapped(tidtbl);
        m_listTiles.ShowFirstSelection();
    }
    END_TRY
    pDoc->SetModifiedFlag();
    EndWaitCursor();
}

void CGbxProjView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
    int nSel = m_listProj.GetCurSel();
    pCmdUI->Enable(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpTile &&
        IsClipboardFormatAvailable(CF_TILEIMAGES));
}

void CGbxProjView::OnEditMove()
{
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpTile &&
        IsClipboardFormatAvailable(CF_TIDLIST));
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CGamDoc* pDoc = GetDocument();
    CTileManager* pTMgr = pDoc->GetTileManager();

    BeginWaitCursor();
    TRY
    {
        CSharedFile file;
        if (OpenClipboard())
        {
            // First check if the tid list is ours....
            LPDWORD pGBoxID = (LPDWORD)GlobalLock(::GetClipboardData(CF_GBOXID));
            DWORD dwGBoxID = pGBoxID[0];
            DWORD dwProcID = pGBoxID[1];
            GlobalUnlock(::GetClipboardData(CF_GBOXID));

            if (dwGBoxID == pDoc->GetGameBoxID() &&
                dwProcID == GetCurrentProcessId())
            {
                // It's our stuff alright!
                file.SetHandle(CopyHandle(::GetClipboardData(CF_TIDLIST)), FALSE);
                CloseClipboard();

                CArchive ar(&file, CArchive::load);
                std::vector<TileID> tidtbl;
                ar >> tidtbl;
                ar.Close();
                int nCurSel = m_listTiles.GetTopSelectedItem();
                size_t nCurSel2 = nCurSel == LB_ERR ? Invalid_v<size_t> : value_preserving_cast<size_t>(nCurSel);
                pTMgr->MoveTileIDsToTileSet(nGrp, tidtbl, nCurSel2);
                DoUpdateTileList();
                pDoc->NotifyTileDatabaseChange();
                m_listTiles.SetCurSelsMapped(tidtbl);
                m_listTiles.ShowFirstSelection();
            }
            CloseClipboard();           // Just to be safe
        }
    }
    END_TRY
//  pDoc->UpdateAllViews(NULL);
    pDoc->SetModifiedFlag();
    EndWaitCursor();
}

void CGbxProjView::OnUpdateEditMove(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;

    int nSel = m_listProj.GetCurSel();
    BOOL bFirstRequirement = nSel >= 0 &&
        m_listProj.GetItemGroupCode(nSel) == grpTile &&
        IsClipboardFormatAvailable(CF_TIDLIST);

    if (bFirstRequirement)
    {
        // Final requirement is the tid list has to be this gamebox's
        if (OpenClipboard())
        {
            // First check if the tid list is ours....
            LPDWORD pGBoxID = (LPDWORD)GlobalLock(::GetClipboardData(CF_GBOXID));
            DWORD dwGBoxID = pGBoxID[0];
            DWORD dwProcID = pGBoxID[1];
            GlobalUnlock(::GetClipboardData(CF_GBOXID));
            CloseClipboard();
            if (dwGBoxID == GetDocument()->GetGameBoxID() && dwProcID == GetCurrentProcessId())
                bEnable = TRUE;
        }
    }
    pCmdUI->Enable(bEnable);
}

///////////////////////////////////////////////////////////////////////

void CGbxProjView::OnProjectCloneBoard()
{
    DoBoardClone();
}

void CGbxProjView::OnUpdateProjectCloneBoard(CCmdUI* pCmdUI)
{
    int nSel = m_listProj.GetCurSel();
    pCmdUI->Enable(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpBrd);
}

///////////////////////////////////////////////////////////////////////

void CGbxProjView::OnProjectSaveTileFile()
{
#ifdef _DEBUG
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpTile &&
        m_listTiles.GetSelCount() > 0);
#endif

    CGamDoc* pDoc = GetDocument();
    CTileManager* pTMgr = pDoc->GetTileManager();

    CString strFilter;
    strFilter.LoadString(IDS_GTL_FILTER);
    CString strTitle;
    strTitle.LoadString(IDS_SEL_SAVETILELIBRARY);

    CFileDialog dlg(FALSE, FILEEXT_GTLB, NULL, OFN_HIDEREADONLY |
        OFN_OVERWRITEPROMPT, strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return;

    std::vector<TileID> tidtbl = m_listTiles.GetCurMappedItemList();

    BeginWaitCursor();
    TRY
    {
        CFile file;
        CFileException fe;

        if (!file.Open(dlg.GetPathName(),
            CFile::modeCreate | CFile::modeWrite, &fe))
        {
            EndWaitCursor();
            AfxMessageBox(IDP_ERR_GTLBCREATE, MB_ICONEXCLAMATION);
            return;
        }
        CArchive ar(&file, CArchive::store);
        ar.Write(FILEGTLSIGNATURE, 4);
        ar << (WORD)NumVersion(fileGtlVerMajor, fileGtlVerMinor);
        pTMgr->CopyTileImagesToArchive(ar, tidtbl);
        ar.Close();
        file.Close();
        EndWaitCursor();
    }
    CATCH_ALL (e)
    {
        EndWaitCursor();
        AfxMessageBox(IDP_ERR_GTLBWRITE, MB_ICONEXCLAMATION);
    }
    END_CATCH_ALL
}

void CGbxProjView::OnUpdateProjectSaveTileFile(CCmdUI* pCmdUI)
{
    int nSel = m_listProj.GetCurSel();
    pCmdUI->Enable(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpTile &&
        m_listTiles.GetSelCount() > 0);
}

void CGbxProjView::OnProjectLoadTileFile()
{
    int nSel = m_listProj.GetCurSel();
    ASSERT(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpTile);
    size_t nGrp = m_listProj.GetItemSourceCode(nSel);

    CGamDoc* pDoc = GetDocument();
    CTileManager* pTMgr = pDoc->GetTileManager();

    CString strFilter;
    strFilter.LoadString(IDS_GTL_FILTER);
    CString strTitle;
    strTitle.LoadString(IDS_SEL_LOADTILELIBRARY);

    CFileDialog dlg(TRUE, FILEEXT_GTLB, NULL,
        OFN_HIDEREADONLY|OFN_PATHMUSTEXIST, strFilter, NULL, 0);
    dlg.m_ofn.lpstrTitle = strTitle;

    if (dlg.DoModal() != IDOK)
        return;

    CFile file;
    CFileException fe;

    if (!file.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite,
        &fe))
    {
        CString strErr;
        AfxFormatString1(strErr, AFX_IDP_FAILED_TO_OPEN_DOC, dlg.GetPathName());
        AfxMessageBox(strErr, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    BeginWaitCursor();
    TRY
    {
        CArchive ar(&file, CArchive::load);
        BYTE chSig[4];
        if (ar.Read(chSig, 4) != 4)
            AfxThrowMemoryException();

        if (memcmp(chSig, FILEGTLSIGNATURE, 4) != 0)
        {
            EndWaitCursor();
            AfxMessageBox(IDP_ERR_NOTGTLB, MB_ICONEXCLAMATION);
            return;

        }
        WORD wVersion;
        ar >> wVersion;
        if (wVersion > (WORD)NumVersion(fileGtlVerMajor, fileGtlVerMinor))
        {
            EndWaitCursor();
            AfxMessageBox(IDP_ERR_GTLBVERSION, MB_ICONEXCLAMATION);
            return;
        }
        std::vector<TileID> tidtbl;
        int nCurSel = m_listTiles.GetTopSelectedItem();
        size_t nCurSel2 = nCurSel == LB_ERR ? Invalid_v<size_t> : value_preserving_cast<size_t>(nCurSel);
        pTMgr->CreateTilesFromTileImageArchive(ar, nGrp, &tidtbl, nCurSel2);
        ar.Close();

        DoUpdateTileList();
        pDoc->NotifyTileDatabaseChange();
        for (size_t i = size_t(0); i < tidtbl.size(); i++)
        {
            CGmBoxHint hint;
            hint.GetArgs<HINT_TILECREATED>().m_tid = tidtbl[i];
            pDoc->UpdateAllViews(NULL, HINT_TILECREATED, &hint);
        }
        m_listTiles.SetCurSelsMapped(tidtbl);
        m_listTiles.ShowFirstSelection();
        EndWaitCursor();
    }
    CATCH_ALL (e)
    {
        EndWaitCursor();
        AfxMessageBox(IDP_ERR_GTLBREAD, MB_ICONEXCLAMATION);
    }
    END_CATCH_ALL
}

void CGbxProjView::OnUpdateProjectLoadTileFile(CCmdUI* pCmdUI)
{
    int nSel = m_listProj.GetCurSel();
    pCmdUI->Enable(nSel >= 0 && m_listProj.GetItemGroupCode(nSel) == grpTile);
}

///////////////////////////////////////////////////////////////////////

void CGbxProjView::OnContextMenu(CWnd* pWnd, CPoint point)
{
    // Make sure window is active.
    GetParentFrame()->ActivateFrame();
    UINT nID = (UINT)-1;

    if (pWnd->GetDlgCtrlID() == IDC_V_PROJLIST)
        nID = MENU_PJ_DEFAULT;
    else if (pWnd->GetDlgCtrlID() == IDC_V_TILELIST)
        nID = MENU_PJ_TILELIST;
    else if (pWnd->GetDlgCtrlID() == IDC_V_PIECELIST)
        nID = MENU_PJ_PIECELIST;
    else if (pWnd->GetDlgCtrlID() == IDC_V_MARKLIST)
        nID = MENU_PJ_MARKERLIST;

    if ((int)nID < 0)
        return;

    CMenu bar;
    if (bar.LoadMenu(IDR_MENU_DESIGN_POPUPS))
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

void CGbxProjView::OnProjItemProperties()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0)
        return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpDoc:    DoGbxProperty(); break;
        case grpBrd:    DoBoardProperty(); break;
        case grpTHdr:   DoTileManagerProperty(); break;
        case grpTile:   DoTileGroupProperty(); break;
        case grpPce:    DoPieceGroupProperty(); break;
        case grpMark:   DoMarkGroupProperty(); break;
        default: ;
    }
}

void CGbxProjView::OnUpdateProjItemProperties(CCmdUI* pCmdUI)
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
            case grpTile:
            case grpPce:
            case grpMark:
            case grpTHdr:
                bEnable = TRUE;
                break;
            default: ;
        }
    }
    pCmdUI->Enable(bEnable);
}

void CGbxProjView::OnProjItemDelete()
{
    int nSel = m_listProj.GetCurSel();
    if (nSel < 0)
        return;
    int nGrp = m_listProj.GetItemGroupCode(nSel);
    ASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardDelete(); break;
        case grpTile:   DoTileGroupDelete(); break;
        case grpPce:    DoPieceGroupDelete(); break;
        case grpMark:   DoMarkGroupDelete(); break;
        default: ;
    }
}

void CGbxProjView::OnUpdateProjItemDelete(CCmdUI* pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj.GetCurSel();
    if (nSel >= 0)
    {
        int nGrp = m_listProj.GetItemGroupCode(nSel);
        ASSERT(nGrp >= 0);
        switch (nGrp)
        {
        case grpBrd:
        case grpTile:
        case grpPce:
        case grpMark:
                bEnable = TRUE;
                break;
            default: ;
        }
    }
    pCmdUI->Enable(bEnable);
}

void CGbxProjView::OnTileClone()
{
    DoTileClone();
}

void CGbxProjView::OnUpdateTileClone(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_listTiles.GetSelCount() > 0);
}

void CGbxProjView::OnTileDelete()
{
    DoTileDelete();
}

void CGbxProjView::OnUpdateTileDelete(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_listTiles.GetSelCount() > 0);
}

void CGbxProjView::OnTileEdit()
{
    DoTileEdit();
}

void CGbxProjView::OnUpdateTileEdit(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_listTiles.GetSelCount() > 0);
}

void CGbxProjView::OnTileNew()
{
    DoTileNew();
}

void CGbxProjView::OnPieceNew()
{
    DoPieceNew();
}

void CGbxProjView::OnPieceEdit()
{
    DoPieceEdit();
}

void CGbxProjView::OnUpdatePieceEdit(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_listPieces.GetSelCount() > 0);
}

void CGbxProjView::OnPieceDelete()
{
    DoPieceDelete();
}

void CGbxProjView::OnUpdatePieceDelete(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_listPieces.GetSelCount() > 0);
}

void CGbxProjView::OnMarkerNew()
{
    DoMarkNew();
}

void CGbxProjView::OnMarkerEdit()
{
    DoMarkEdit();
}

void CGbxProjView::OnUpdateMarkerEdit(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_listMarks.GetSelCount() >= 0);
}

void CGbxProjView::OnMarkerDelete()
{
    DoMarkDelete();
}

void CGbxProjView::OnUpdateMarkerDelete(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_listMarks.GetSelCount() >= 0);
}

