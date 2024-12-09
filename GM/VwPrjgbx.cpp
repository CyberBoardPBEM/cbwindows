// VwPrjgbx.cpp : implementation file
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
#include    <afxadv.h>
#include    "Gm.h"
#include    "GmDoc.h"
#include    "ResTbl.h"
#include    "Tile.h"
#include    "Board.h"
#include    "Pieces.h"
#include    "Marks.h"

#include    "VwPrjgbx.h"
#include    "LibMfc.h"
#include    "FrmMain.h"     // TODO:  remove?

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

wxIMPLEMENT_DYNAMIC_CLASS(CProjListBoxGm, CGrafixListBoxWx)
IMPLEMENT_DYNCREATE(CGbxProjViewContainer, CView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

#if 0
const int XBORDER = 5;
const int YBORDER = 5;

const int BTN_GROUP_XGAP = 12;
const int BTN_GROUP_YGAP = 6;

const int BTN_PROJ_WD = 14 * 4;
const int BTN_PROJ_HT = 12;

const int BTN_ITEM_WD = 10 * 4;
const int BTN_ITEM_HT = 12;

const int PROJ_LIST_WIDTH = 200;
#endif

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

wxBEGIN_EVENT_TABLE(CGbxProjView, wxPanel)
#if 0
    ON_WM_SIZE()
    ON_WM_CREATE()
#endif
    EVT_LISTBOX(XRCID("m_listProj"), OnSelChangeProjList)
    EVT_LISTBOX_DCLICK(XRCID("m_listProj"), OnDblClkProjList)
    EVT_LISTBOX_DCLICK(XRCID("m_listTiles"), OnDblClkTileList)
    EVT_LISTBOX_DCLICK(XRCID("m_listPieces"), OnDblClkPieceList)
    EVT_LISTBOX_DCLICK(XRCID("m_listMarks"), OnDblClkMarkList)
    EVT_BUTTON(XRCID("m_btnPrjA"), OnClickedProjBtnA)
    EVT_BUTTON(XRCID("m_btnPrjB"), OnClickedProjBtnB)
    EVT_BUTTON(XRCID("m_btnItmA"), OnClickedItemBtnA)
    EVT_BUTTON(XRCID("m_btnItmB"), OnClickedItemBtnB)
    EVT_BUTTON(XRCID("m_btnItmC"), OnClickedItemBtnC)
    EVT_BUTTON(XRCID("m_btnItmD"), OnClickedItemBtnD)
#if 0
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()
#endif
    EVT_MENU(wxID_COPY, OnEditCopy)
    EVT_UPDATE_UI(wxID_COPY, OnUpdateEditCopy)
    EVT_MENU(wxID_PASTE, OnEditPaste)
    EVT_UPDATE_UI(wxID_PASTE, OnUpdateEditPaste)
    EVT_MENU(XRCID("ID_EDIT_MOVE"), OnEditMove)
    EVT_UPDATE_UI(XRCID("ID_EDIT_MOVE"), OnUpdateEditMove)
    EVT_MENU(XRCID("ID_PROJECT_SAVETILEFILE"), OnProjectSaveTileFile)
    EVT_UPDATE_UI(XRCID("ID_PROJECT_SAVETILEFILE"), OnUpdateProjectSaveTileFile)
    EVT_MENU(XRCID("ID_PROJECT_LOADTILEFILE"), OnProjectLoadTileFile)
    EVT_UPDATE_UI(XRCID("ID_PROJECT_LOADTILEFILE"), OnUpdateProjectLoadTileFile)
    EVT_CONTEXT_MENU(OnContextMenu)
    EVT_MENU(XRCID("ID_PROJITEM_PROPERTIES"), OnProjItemProperties)
    EVT_UPDATE_UI(XRCID("ID_PROJITEM_PROPERTIES"), OnUpdateProjItemProperties)
    EVT_MENU(XRCID("ID_TILE_CLONE"), OnTileClone)
    EVT_UPDATE_UI(XRCID("ID_TILE_CLONE"), OnUpdateTileClone)
    EVT_MENU(XRCID("ID_TILE_DELETE"), OnTileDelete)
    EVT_UPDATE_UI(XRCID("ID_TILE_DELETE"), OnUpdateTileDelete)
    EVT_MENU(XRCID("ID_TILE_EDIT"), OnTileEdit)
    EVT_UPDATE_UI(XRCID("ID_TILE_EDIT"), OnUpdateTileEdit)
    EVT_MENU(XRCID("ID_TILE_NEW"), OnTileNew)
    EVT_MENU(XRCID("ID_PROJITEM_DELETE"), OnProjItemDelete)
    EVT_UPDATE_UI(XRCID("ID_PROJITEM_DELETE"), OnUpdateProjItemDelete)
    EVT_MENU(XRCID("ID_PIECE_NEW"), OnPieceNew)
    EVT_MENU(XRCID("ID_PIECE_EDIT"), OnPieceEdit)
    EVT_UPDATE_UI(XRCID("ID_PIECE_EDIT"), OnUpdatePieceEdit)
    EVT_MENU(XRCID("ID_PIECE_DELETE"), OnPieceDelete)
    EVT_UPDATE_UI(XRCID("ID_PIECE_DELETE"), OnUpdatePieceDelete)
    EVT_MENU(XRCID("ID_MARKER_NEW"), OnMarkerNew)
    EVT_MENU(XRCID("ID_MARKER_EDIT"), OnMarkerEdit)
    EVT_UPDATE_UI(XRCID("ID_MARKER_EDIT"), OnUpdateMarkerEdit)
    EVT_MENU(XRCID("ID_MARKER_DELETE"), OnMarkerDelete)
    EVT_UPDATE_UI(XRCID("ID_MARKER_DELETE"), OnUpdateMarkerDelete)
    EVT_MENU(XRCID("ID_PROJECT_CLONEBOARD"), OnProjectCloneBoard)
    EVT_UPDATE_UI(XRCID("ID_PROJECT_CLONEBOARD"), OnUpdateProjectCloneBoard)
    EVT_DRAGDROP(OnDragItem)
    EVT_GET_DRAG_SIZE(OnGetDragSize)
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CGbxProjViewContainer, CView)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGbxProjView

CGbxProjView::CGbxProjView(CGbxProjViewContainer& p) :
    CB_XRC_BEGIN_CTRLS_DEFN(static_cast<wxWindow*>(p), CGbxProjView)
        CB_XRC_CTRL(m_listProj)
        CB_XRC_CTRL(m_editInfo)
        CB_XRC_CTRL(m_listTiles)
        CB_XRC_CTRL(m_listPieces)
        CB_XRC_CTRL(m_listMarks)
        CB_XRC_CTRL(m_btnPrjA)
        CB_XRC_CTRL(m_btnPrjB)
        CB_XRC_CTRL(m_btnItmA)
        CB_XRC_CTRL(m_btnItmB)
        CB_XRC_CTRL(m_btnItmC)
        CB_XRC_CTRL(m_btnItmD)
    CB_XRC_END_CTRLS_DEFN(),
    parent(&p),
    document(CB::ToCGamDoc(parent->GetDocument()))
{
    m_nLastSel = -1;
    m_nLastGrp = -1;

    m_listTiles->SetDrawAllScales(TRUE);
    m_listTiles->EnableDrag();
    m_listTiles->EnableSelfDrop();
    m_listTiles->EnableDropScroll();
}

CGbxProjView::~CGbxProjView()
{
}

#if 0
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


    if (!CreateListbox(IDC_V_PIECELIST, m_listPieces, WS_HSCROLL | LBS_HASSTRINGS | LBS_EXTENDEDSEL, rctList))
        return -1;
    if (!CreateListbox(IDC_V_MARKLIST, m_listMarks, LBS_HASSTRINGS | LBS_EXTENDEDSEL, rctList))
        return -1;

    // Create an informational text box.
    if (!CreateEditbox(IDC_V_EDITINFO, m_editInfo, rctList))
        return -1;

    return 0;
}
#endif

/////////////////////////////////////////////////////////////////////////////

void CGbxProjView::OnInitialUpdate()
{
    m_listTiles->SetDocument(&GetDocument());
    m_listPieces->SetDocument(GetDocument());
    m_listMarks->SetDocument(&GetDocument());
}

void CGbxProjView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
    DoUpdateProjectList();
}

#if 0
/////////////////////////////////////////////////////////////////////////////
// CGbxProjView drawing

void CGbxProjView::OnDraw(CDC* pDC)
{
    CDocument& pDoc = GetDocument();
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
#endif

/////////////////////////////////////////////////////////////////////////////

void CGbxProjView::OnDragItem(DragDropEvent& event)
{
    if (event.GetProcessId() != wxGetProcessId())
    {
        return;
    }
    CGamDoc& pDoc = GetDocument();

    const DragInfoWx& pdi = event.GetDragInfo();

    if (pdi.GetDragType() != DRAG_TILELIST)
        return;               // Only tile list drops allowed

    if (pdi.GetSubInfo<DRAG_TILELIST>().m_gamDoc != &pDoc)
        return;               // Only pieces from our document.

    // no size restriction

    wxRect rct = m_listTiles->GetClientRect();
    if (!rct.Contains(pdi.m_pointClient))
        return;

    if (pdi.m_phase == PhaseDrag::Over)
    {
        event.SetCursor(pdi.m_hcsrSuggest);
        return;
    }
    else if (pdi.m_phase == PhaseDrag::Drop && pdi.GetDragType() == DRAG_TILELIST)
    {
        int nProjSel = m_listProj->GetSelection();
        wxASSERT(nProjSel >= 0 &&
            m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nProjSel)) == grpTile);
        size_t nGrpSel = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nProjSel));

        CTileManager& pTMgr = pDoc.GetTileManager();
        const CTileSet& pTGrp = pTMgr.GetTileSet(nGrpSel);

        // Force selection of item under the mouse
        m_listTiles->SetSelFromPoint(pdi.m_point);
        int nSel = [this]{
            std::vector<size_t> sels = m_listTiles->GetSelections();
            wxASSERT(sels.size() <= size_t(1));
            return !sels.empty() ? value_preserving_cast<int>(sels.front()) : wxNOT_FOUND;
        }();

        if (nSel >= 0)
        {
            // Check if the mouse is above or below the half point.
            // If above, insert before. If below, insert after.
            wxRect rct = m_listTiles->GetItemRect(value_preserving_cast<size_t>(nSel));
            if (pdi.m_point.y > (rct.GetTop() + rct.GetBottom()) / 2)
                nSel++;
        }

        pTMgr.MoveTileIDsToTileSet(nGrpSel, *pdi.GetSubInfo<DRAG_TILELIST>().m_tileIDList, value_preserving_cast<size_t>(nSel));
        DoUpdateTileList();
        pDoc.NotifyTileDatabaseChange();
        m_listTiles->SetCurSelsMapped(*pdi.GetSubInfo<DRAG_TILELIST>().m_tileIDList);
        m_listTiles->ShowFirstSelection();
        pDoc.SetModifiedFlag();
    }
}

void CGbxProjView::OnGetDragSize(GetDragSizeEvent& event)
{
    /* I don't expect this to be a source for any limited dest,
        so report max size since it is easy to generate and
        will make it obvious if any limited dest does receive
        this */
    wxSize retval;
    retval.x = std::numeric_limits<decltype(retval.x)>::max();
    retval.y = std::numeric_limits<decltype(retval.y)>::max();

    event.SetSize(retval);
}

/////////////////////////////////////////////////////////////////////////////
// Updates buttons for specified group

void CGbxProjView::UpdateButtons(int nGrp)
{
    nGrp++;                     // -1 means no selection so bump to zero
    UINT* pTbl = btnGroupTbl[nGrp];
    SetButtonState(*m_btnPrjA, pTbl[0]);
    SetButtonState(*m_btnPrjB, pTbl[1]);
    SetButtonState(*m_btnItmA, pTbl[2]);
    SetButtonState(*m_btnItmB, pTbl[3]);
    SetButtonState(*m_btnItmC, pTbl[4]);
    SetButtonState(*m_btnItmD, pTbl[5]);
}

void CGbxProjView::SetButtonState(wxButton& btn, UINT nStringID)
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

void CGbxProjView::UpdateItemControls(int nGrp)
{
    wxWindowUpdateLocker freezer(this);
    ChildrenRepositioningGuard crg(this);

    m_listTiles->Hide();
    m_listPieces->Hide();
    m_listMarks->Hide();
    m_editInfo->Hide();
    if (nGrp == grpTile)            // Tiles
    {
        m_listTiles->Show();
    }
    else if (nGrp == grpPce)        // Pieces
    {
        m_listPieces->Show();
    }
    else if (nGrp == grpMark)       // Markers
    {
        m_listMarks->Show();
    }
    else    // Board, headings and no selection
    {
        m_editInfo->Show();
    }
    Layout();
}

/////////////////////////////////////////////////////////////////////////////

void CGbxProjView::DoUpdateProjectList(BOOL bUpdateItem /* = TRUE */)
{
    CGamDoc& pDoc = GetDocument();

    // Preserve the current selection
    size_t nTopIdx = m_listProj->GetVisibleRowsBegin();
    int nCurSel = m_listProj->GetSelection();

    {
    wxWindowUpdateLocker freezer(&*m_listProj);

    m_listProj->Clear();

    // Document type....
    CB::string str = CB::string::LoadString(IDS_PHEAD_DOCTYPE);
    m_listProj->AddItem(grpDoc, str);

    // Boards....
    str = CB::string::LoadString(IDS_PHEAD_BOARDS);
    m_listProj->AddItem(grpBHdr, str);

    CBoardManager& pBMgr = pDoc.GetBoardManager();
    for (size_t i = size_t(0); i < pBMgr.GetNumBoards(); i++)
    {
        static int bDisplayIDs = -1;
        if (bDisplayIDs == -1)
        {
            bDisplayIDs = GetApp()->GetProfileInt("Settings"_cbstring, "DisplayIDs"_cbstring, 0);
        }
        str = pBMgr.GetBoard(i).GetName();
        if (bDisplayIDs)
        {
            CB::string strTmp = std::move(str);
            str = std::format(L"[{}] {}",
                pBMgr.GetBoard(i).GetSerialNumber(), strTmp);
        }
        m_listProj->AddItem(grpBrd, str, i);
    }

    // Tiles....
    str = CB::string::LoadString(IDS_PHEAD_TILES);
    m_listProj->AddItem(grpTHdr, str);

    CTileManager& pTMgr = pDoc.GetTileManager();
    for (size_t i = size_t(0); i < pTMgr.GetNumTileSets(); i++)
        m_listProj->AddItem(grpTile, pTMgr.GetTileSet(i).GetName(), i);

    // Pieces....
    str = CB::string::LoadString(IDS_PHEAD_PIECES);
    m_listProj->AddItem(grpPHdr, str);

    CPieceManager& pPMgr = pDoc.GetPieceManager();
    for (size_t i = size_t(0); i < pPMgr.GetNumPieceSets(); i++)
        m_listProj->AddItem(grpPce, pPMgr.GetPieceSet(i).GetName(), i);

    // Marks....
    str = CB::string::LoadString(IDS_PHEAD_MARKS);
    m_listProj->AddItem(grpMHdr, str);

    CMarkManager& pMMgr = pDoc.GetMarkManager();
    for (size_t i = size_t(0); i < pMMgr.GetNumMarkSets(); i++)
        m_listProj->AddItem(grpMark, pMMgr.GetMarkSet(i).GetName(), i);

    // OK...Show the updates
    }
    m_listProj->Refresh();

    m_listProj->ScrollToRow(nTopIdx);
    if (nCurSel >= 0)
    {
        if (nCurSel >= value_preserving_cast<int>(m_listProj->GetItemCount()))
            nCurSel = value_preserving_cast<int>(m_listProj->GetItemCount()) - 1;
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

void CGbxProjView::OnSelChangeProjList(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (m_nLastSel == nSel)
        return;

    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
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
        UpdateItemControls(value_preserving_cast<int>(nGrp));
        UpdateButtons(value_preserving_cast<int>(nGrp));
        m_nLastGrp = value_preserving_cast<int>(nGrp);
    }
}

void CGbxProjView::OnDblClkProjList(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (nSel < 0)
        return;
    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
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

void CGbxProjView::OnDblClkTileList(wxCommandEvent& /*event*/)
{
    DoTileEdit();
}

void CGbxProjView::OnDblClkPieceList(wxCommandEvent& /*event*/)
{
    if (m_listPieces->GetSelectedCount() > 0)
        DoPieceEdit();
}

void CGbxProjView::OnDblClkMarkList(wxCommandEvent& /*event*/)
{
    DoMarkEdit();
}

/////////////////////////////////////////////////////////////////////////////
// Button notifications

void CGbxProjView::OnClickedProjBtnA(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
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

void CGbxProjView::OnClickedProjBtnB(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardDelete(); break;
        case grpTHdr:   DoTileManagerProperty(); break;
        case grpTile:   DoTileGroupDelete(); break;
        case grpPce:    DoPieceGroupDelete(); break;
        case grpMark:   DoMarkGroupDelete(); break;
    }
}

void CGbxProjView::OnClickedItemBtnA(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardEdit(); break;
        case grpTile:   DoTileNew(); break;
        case grpPce:    DoPieceNew(); break;
        case grpMark:   DoMarkNew(); break;
    }
}

void CGbxProjView::OnClickedItemBtnB(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpTile:   DoTileEdit(); break;
        case grpPce:    DoPieceEdit(); break;
        case grpMark:   DoMarkEdit(); break;
    }
}

void CGbxProjView::OnClickedItemBtnC(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    switch (nGrp)
    {
        case grpTile:   DoTileClone(); break;
        case grpPce:    DoPieceDelete(); break;
        case grpMark:   DoMarkDelete(); break;
    }
}

void CGbxProjView::OnClickedItemBtnD(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND) return;
    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(nGrp >= 0);
    if (nGrp == grpTile)
        DoTileDelete();
}

void CGbxProjView::OnEditCopy(wxCommandEvent& /*event*/)
{
#ifdef _DEBUG
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0 && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile &&
        m_listTiles->GetSelectedCount() > size_t(0));
#endif

    CGamDoc& pDoc = GetDocument();
    CTileManager& pTMgr = pDoc.GetTileManager();

    std::vector<TileID> tidtbl = m_listTiles->GetCurMappedItemList();

    wxBusyCursor busyCursor;
    TRY
    {
        CMemFile file;
        CArchive ar(&file, CArchive::store);
        pTMgr.CopyTileImagesToArchive(ar, tidtbl);
        ar.Close();

        CMemFile file2;
        CArchive ar2(&file2, CArchive::store);
        ar2 << tidtbl;
        ar2.Close();

        CMemFile file3;
        CArchive ar3(&file3, CArchive::store);
        ar3 << value_preserving_cast<uintptr_t>(pDoc.GetGameBoxID());
        ar3 << value_preserving_cast<uintptr_t>(wxGetProcessId());// To distinguish between same GBox in other files
        ar3.Close();

        /* WARNING:  CMemFile and wxCustomDataObject disagree on
            heap API.  This adapter is not general purpose
            because the wxCustomDataObject members are private */
        class CustomDataObjectFree : public wxCustomDataObject
        {
        public:
            using wxCustomDataObject::wxCustomDataObject;
            void Free() override { free(GetData()); }
        };

        std::unique_ptr<wxCustomDataObject> tiles(new CustomDataObjectFree(CF_TILEIMAGES));
        // paranoid about undefined order of function arg eval
        size_t len = value_preserving_cast<size_t>(file.GetLength());
        tiles->TakeData(len, file.Detach());

        std::unique_ptr<wxCustomDataObject> tids(new CustomDataObjectFree(CF_TIDLIST));
        // paranoid about undefined order of function arg eval
        len = value_preserving_cast<size_t>(file2.GetLength());
        tids->TakeData(len, file2.Detach());

        std::unique_ptr<wxCustomDataObject> gbox(new CustomDataObjectFree(CF_GBOXID));
        // paranoid about undefined order of function arg eval
        len = value_preserving_cast<size_t>(file3.GetLength());
        gbox->TakeData(len, file3.Detach());

        std::unique_ptr<wxDataObjectComposite> compos(new wxDataObjectComposite);
        compos->Add(tiles.release());
        compos->Add(tids.release());
        compos->Add(gbox.release());

        LockWxClipboard lockClipbd(std::try_to_lock);
        if (lockClipbd)
        {
            wxTheClipboard->SetData(compos.release());
        }
    }
    END_TRY
}

void CGbxProjView::OnUpdateEditCopy(wxUpdateUIEvent& pCmdUI)
{
    int nSel = m_listProj->GetSelection();
    pCmdUI.Enable(nSel >= 0 && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile &&
        m_listTiles->GetSelectedCount() > 0);
}

void CGbxProjView::OnEditPaste(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0 && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile &&
        wxIsClipboardFormatAvailable(CF_TILEIMAGES));
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CGamDoc& pDoc = GetDocument();
    CTileManager& pTMgr = pDoc.GetTileManager();

    wxBusyCursor busyCursor;
    TRY
    {
        CMemFile file;
        std::vector<TileID> tidtbl;
        LockWxClipboard lockClipbd(std::try_to_lock);
        if (lockClipbd)
        {
            wxCustomDataObject tiles(CF_TILEIMAGES);
            ASSERT(wxTheClipboard->GetData(tiles));
            file.Attach(static_cast<BYTE*>(tiles.GetData()), value_preserving_cast<UINT>(tiles.GetDataSize()));
            lockClipbd.Unlock();

            CArchive ar(&file, CArchive::load);
            int nCurSel = m_listTiles->GetTopSelectedItem();
            size_t nCurSel2 = nCurSel == wxNOT_FOUND ? Invalid_v<size_t> : value_preserving_cast<size_t>(nCurSel);
            pTMgr.CreateTilesFromTileImageArchive(ar, nGrp, &tidtbl, nCurSel2);
            ar.Close();
        }
        DoUpdateTileList();
        pDoc.NotifyTileDatabaseChange();
        for (size_t i = size_t(0); i < tidtbl.size(); i++)
        {
            CGmBoxHint hint;
            hint.GetArgs<HINT_TILECREATED>().m_tid = tidtbl[i];
            pDoc.UpdateAllViews(NULL, HINT_TILECREATED, &hint);
        }
        m_listTiles->SetCurSelsMapped(tidtbl);
        m_listTiles->ShowFirstSelection();
    }
    END_TRY
    pDoc.SetModifiedFlag();
}

void CGbxProjView::OnUpdateEditPaste(wxUpdateUIEvent& pCmdUI)
{
    int nSel = m_listProj->GetSelection();
    pCmdUI.Enable(nSel >= 0 && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile &&
        wxIsClipboardFormatAvailable(CF_TILEIMAGES));
}

void CGbxProjView::OnEditMove(wxCommandEvent& event)
{
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0 && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile &&
        wxIsClipboardFormatAvailable(CF_TIDLIST));
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CGamDoc& pDoc = GetDocument();
    CTileManager& pTMgr = pDoc.GetTileManager();

    wxBusyCursor busyCursor;
    TRY
    {
        CMemFile file;
        LockWxClipboard lockClipbd(std::try_to_lock);
        if (lockClipbd)
        {
            /* WARNING:  wxDataObjectComposite allows setting
                multiple data types, but will only get one */

            // First check if the tid list is ours....
            wxCustomDataObject gbox(CF_GBOXID);
            wxASSERT(wxTheClipboard->GetData(gbox));
            uintptr_t* pGBoxID = static_cast<uintptr_t*>(gbox.GetData());
            uintptr_t dwGBoxID = pGBoxID[0];
            uintptr_t dwProcID = pGBoxID[1];

            if (dwGBoxID == pDoc.GetGameBoxID() &&
                dwProcID == wxGetProcessId())
            {
                // It's our stuff alright!
                wxCustomDataObject tids(CF_TIDLIST);
                wxASSERT(wxTheClipboard->GetData(tids));
                file.Attach(static_cast<BYTE*>(tids.GetData()), value_preserving_cast<UINT>(tids.GetDataSize()));
                lockClipbd.Unlock();

                CArchive ar(&file, CArchive::load);
                std::vector<TileID> tidtbl;
                ar >> tidtbl;
                ar.Close();
                int nCurSel = m_listTiles->GetTopSelectedItem();
                size_t nCurSel2 = nCurSel == LB_ERR ? Invalid_v<size_t> : value_preserving_cast<size_t>(nCurSel);
                pTMgr.MoveTileIDsToTileSet(nGrp, tidtbl, nCurSel2);
                DoUpdateTileList();
                pDoc.NotifyTileDatabaseChange();
                m_listTiles->SetCurSelsMapped(tidtbl);
                m_listTiles->ShowFirstSelection();
            }
        }
    }
    END_TRY
//  pDoc->UpdateAllViews(NULL);
    pDoc.SetModifiedFlag();
}

void CGbxProjView::OnUpdateEditMove(wxUpdateUIEvent& pCmdUI)
{
    BOOL bEnable = FALSE;

    int nSel = m_listProj->GetSelection();
    BOOL bFirstRequirement = nSel >= 0 &&
        m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile &&
        wxIsClipboardFormatAvailable(CF_TIDLIST);

    if (bFirstRequirement)
    {
        // Final requirement is the tid list has to be this gamebox's
        LockWxClipboard lockClipbd(std::try_to_lock);
        if (lockClipbd)
        {
            // First check if the tid list is ours....
            wxCustomDataObject gbox(CF_GBOXID);
            wxASSERT(wxTheClipboard->GetData(gbox));
            uintptr_t* pGBoxID = static_cast<uintptr_t*>(gbox.GetData());
            uintptr_t dwGBoxID = pGBoxID[0];
            uintptr_t dwProcID = pGBoxID[1];
            if (dwGBoxID == GetDocument().GetGameBoxID() && dwProcID == wxGetProcessId())
                bEnable = TRUE;
        }
    }
    pCmdUI.Enable(bEnable);
}

///////////////////////////////////////////////////////////////////////

void CGbxProjView::OnProjectCloneBoard(wxCommandEvent& /*event*/)
{
    DoBoardClone();
}

void CGbxProjView::OnUpdateProjectCloneBoard(wxUpdateUIEvent& pCmdUI)
{
    int nSel = m_listProj->GetSelection();
    pCmdUI.Enable(nSel != wxNOT_FOUND && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpBrd);
}

///////////////////////////////////////////////////////////////////////

void CGbxProjView::OnProjectSaveTileFile(wxCommandEvent& /*event*/)
{
#ifdef _DEBUG
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0 && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile &&
        m_listTiles->GetSelectedCount() > 0);
#endif

    CGamDoc& pDoc = GetDocument();
    CTileManager& pTMgr = pDoc.GetTileManager();

    CB::string strFilter = CB::string::LoadString(IDS_GTL_FILTER);
    CB::string strTitle = CB::string::LoadString(IDS_SEL_SAVETILELIBRARY);

    wxFileDialog dlg(this, strTitle,
                    wxEmptyString, wxEmptyString,
                    strFilter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    std::vector<TileID> tidtbl = m_listTiles->GetCurMappedItemList();

    wxBusyCursor busyCursor;
    TRY
    {
        CFile file;
        CFileException fe;

        if (!file.Open(dlg.GetPath(),
            CFile::modeCreate | CFile::modeWrite, &fe))
        {
            wxMessageBox(CB::string::LoadString(IDP_ERR_GTLBCREATE),
                        CB::GetAppName(),
                        wxICON_EXCLAMATION);
            return;
        }
        CArchive ar(&file, CArchive::store);
        ar.Write(FILEGTLSIGNATURE, 4);
        // TODO:  if new format ever needed, use Features
        ar << (WORD)NumVersion(fileGtlVerMajor, fileGtlVerMinor);
        pTMgr.CopyTileImagesToArchive(ar, tidtbl);
        ar.Close();
        file.Close();
    }
    CATCH_ALL (e)
    {
        wxMessageBox(CB::string::LoadString(IDP_ERR_GTLBWRITE),
                    CB::GetAppName(),
                    wxICON_EXCLAMATION);
    }
    END_CATCH_ALL
}

void CGbxProjView::OnUpdateProjectSaveTileFile(wxUpdateUIEvent& pCmdUI)
{
    int nSel = m_listProj->GetSelection();
    pCmdUI.Enable(nSel >= 0 && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile &&
        m_listTiles->GetSelectedCount() > 0);
}

void CGbxProjView::OnProjectLoadTileFile(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    wxASSERT(nSel >= 0 && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile);
    size_t nGrp = m_listProj->GetItemSourceCode(value_preserving_cast<size_t>(nSel));

    CGamDoc& pDoc = GetDocument();
    CTileManager& pTMgr = pDoc.GetTileManager();

    CB::string strFilter = CB::string::LoadString(IDS_GTL_FILTER);
    CB::string strTitle = CB::string::LoadString(IDS_SEL_LOADTILELIBRARY);

    wxFileDialog dlg(this, strTitle,
                    wxEmptyString, wxEmptyString,
                    strFilter, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() != wxID_OK)
        return;

    CFile file;
    CFileException fe;

    if (!file.Open(dlg.GetPath(), CFile::modeRead | CFile::shareDenyWrite,
        &fe))
    {
        CB::string strErr = AfxFormatString1(AFX_IDP_FAILED_TO_OPEN_DOC, dlg.GetPath());
        wxMessageBox(strErr,
                    CB::GetAppName(),
                    wxOK | wxICON_EXCLAMATION);
        return;
    }

    wxBusyCursor busyCursor;
    TRY
    {
        CArchive ar(&file, CArchive::load);
        BYTE chSig[4];
        if (ar.Read(chSig, 4) != 4)
            AfxThrowMemoryException();

        if (memcmp(chSig, FILEGTLSIGNATURE, 4) != 0)
        {
            wxMessageBox(CB::string::LoadString(IDP_ERR_NOTGTLB),
                        CB::GetAppName(),
                        wxICON_EXCLAMATION);
            return;

        }
        WORD wVersion;
        ar >> wVersion;
        if (wVersion > (WORD)NumVersion(fileGtlVerMajor, fileGtlVerMinor))
        {
            wxMessageBox(CB::string::LoadString(IDP_ERR_GTLBVERSION),
                        CB::GetAppName(),
                        wxICON_EXCLAMATION);
            return;
        }
        std::vector<TileID> tidtbl;
        int nCurSel = m_listTiles->GetTopSelectedItem();
        size_t nCurSel2 = nCurSel == wxNOT_FOUND ? Invalid_v<size_t> : value_preserving_cast<size_t>(nCurSel);
        pTMgr.CreateTilesFromTileImageArchive(ar, nGrp, &tidtbl, nCurSel2);
        ar.Close();

        DoUpdateTileList();
        pDoc.NotifyTileDatabaseChange();
        for (size_t i = size_t(0); i < tidtbl.size(); i++)
        {
            CGmBoxHint hint;
            hint.GetArgs<HINT_TILECREATED>().m_tid = tidtbl[i];
            pDoc.UpdateAllViews(NULL, HINT_TILECREATED, &hint);
        }
        m_listTiles->SetCurSelsMapped(tidtbl);
        m_listTiles->ShowFirstSelection();
    }
    CATCH_ALL (e)
    {
        wxMessageBox(CB::string::LoadStringW(IDP_ERR_GTLBREAD),
                    CB::GetAppName(),
                    wxICON_EXCLAMATION);
    }
    END_CATCH_ALL
}

void CGbxProjView::OnUpdateProjectLoadTileFile(wxUpdateUIEvent& pCmdUI)
{
    int nSel = m_listProj->GetSelection();
    pCmdUI.Enable(nSel >= 0 && m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel)) == grpTile);
}

///////////////////////////////////////////////////////////////////////

void CGbxProjView::OnContextMenu(wxContextMenuEvent& event)
{
    const char* nID = nullptr;

    if (event.GetEventObject() == &*m_listProj)
        nID = "2=PJ_DEFAULT";
    else if (event.GetEventObject() == &*m_listTiles)
        nID = "3=PJ_TILELIST";
    else if (event.GetEventObject() == &*m_listPieces)
        nID = "4=PJ_PIECELIST";
    else if (event.GetEventObject() == &*m_listMarks)
        nID = "5=PJ_MARKERLIST";

    if (!nID)
        return;

    std::unique_ptr<wxMenuBar> bar(wxXmlResource::Get()->LoadMenuBar("IDR_MENU_DESIGN_POPUPS"));
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

void CGbxProjView::OnProjItemProperties(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (nSel < 0)
        return;
    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(static_cast<int>(nGrp) >= 0);
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

void CGbxProjView::OnUpdateProjItemProperties(wxUpdateUIEvent& pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj->GetSelection();
    if (nSel >= 0)
    {
        decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
        wxASSERT(static_cast<int>(nGrp) >= 0);
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
    pCmdUI.Enable(bEnable);
}

void CGbxProjView::OnProjItemDelete(wxCommandEvent& /*event*/)
{
    int nSel = m_listProj->GetSelection();
    if (nSel == wxNOT_FOUND)
        return;
    decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
    wxASSERT(static_cast<int>(nGrp) >= 0);
    switch (nGrp)
    {
        case grpBrd:    DoBoardDelete(); break;
        case grpTile:   DoTileGroupDelete(); break;
        case grpPce:    DoPieceGroupDelete(); break;
        case grpMark:   DoMarkGroupDelete(); break;
        default: ;
    }
}

void CGbxProjView::OnUpdateProjItemDelete(wxUpdateUIEvent& pCmdUI)
{
    BOOL bEnable = FALSE;
    int nSel = m_listProj->GetSelection();
    if (nSel != wxNOT_FOUND)
    {
        decltype(CB::Impl::CGbxProjViewBase::grpDoc) nGrp = m_listProj->GetItemGroupCode(value_preserving_cast<size_t>(nSel));
        wxASSERT(static_cast<int>(nGrp) >= 0);
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
    pCmdUI.Enable(bEnable);
}

void CGbxProjView::OnTileClone(wxCommandEvent& /*event*/)
{
    DoTileClone();
}

void CGbxProjView::OnUpdateTileClone(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_listTiles->GetSelectedCount() > 0);
}

void CGbxProjView::OnTileDelete(wxCommandEvent& /*event*/)
{
    DoTileDelete();
}

void CGbxProjView::OnUpdateTileDelete(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_listTiles->GetSelectedCount() > 0);
}

void CGbxProjView::OnTileEdit(wxCommandEvent& /*event*/)
{
    DoTileEdit();
}

void CGbxProjView::OnUpdateTileEdit(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_listTiles->GetSelectedCount() > 0);
}

void CGbxProjView::OnTileNew(wxCommandEvent& /*event*/)
{
    DoTileNew();
}

void CGbxProjView::OnPieceNew(wxCommandEvent& /*event*/)
{
    DoPieceNew();
}

void CGbxProjView::OnPieceEdit(wxCommandEvent& /*event*/)
{
    DoPieceEdit();
}

void CGbxProjView::OnUpdatePieceEdit(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_listPieces->GetSelectedCount() > 0);
}

void CGbxProjView::OnPieceDelete(wxCommandEvent& /*event*/)
{
    DoPieceDelete();
}

void CGbxProjView::OnUpdatePieceDelete(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_listPieces->GetSelectedCount() > 0);
}

void CGbxProjView::OnMarkerNew(wxCommandEvent& /*event*/)
{
    DoMarkNew();
}

void CGbxProjView::OnMarkerEdit(wxCommandEvent& /*event*/)
{
    DoMarkEdit();
}

void CGbxProjView::OnUpdateMarkerEdit(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_listMarks->GetSelectedCount() >= 0);
}

void CGbxProjView::OnMarkerDelete(wxCommandEvent& /*event*/)
{
    DoMarkDelete();
}

void CGbxProjView::OnUpdateMarkerDelete(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(m_listMarks->GetSelectedCount() >= 0);
}

#if 0
void CGbxProjViewContainer::OnActivateView(BOOL bActivate,
    CView* pActivateView,
    CView* /*pDeactiveView*/)
{
    WXUNUSED_UNLESS_DEBUG(pActivateView);
    if (bActivate)
    {
        wxASSERT(pActivateView == this);
        GetParentFrame()->SetActiveView(&*child);
    }
}
#endif

void CGbxProjViewContainer::OnDraw(CDC* /*pDC*/)
{
    // do nothing because child covers entire client rect
}

void CGbxProjViewContainer::OnInitialUpdate()
{
    child->OnInitialUpdate();

    CB::OnCmdMsgOverride<CView>::OnInitialUpdate();
}

void CGbxProjViewContainer::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    child->OnUpdate(pSender, lHint, pHint);

    CB::OnCmdMsgOverride<CView>::OnUpdate(pSender, lHint, pHint);
}

CGbxProjViewContainer::CGbxProjViewContainer() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

int CGbxProjViewContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    child = new CGbxProjView(*this);

    return 0;
}

// MFC puts the focus here, so move it to the useful window
void CGbxProjViewContainer::OnSetFocus(CWnd* pOldWnd)
{
    CView::OnSetFocus(pOldWnd);
    child->SetFocus();
}

