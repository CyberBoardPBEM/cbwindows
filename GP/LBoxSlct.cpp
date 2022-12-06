// LBoxSlct.cpp
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
#include    "ResTbl.h"
#include    "LBoxSlct.h"
#include    "DrawObj.h"
#include    "PPieces.h"
#include    "FrmPbrd.h"
#include    "GMisc.h"
#include    "VwPbrd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

const int tileBorder = 3;
const int tileGap = 6;

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSelectListBox, CTileBaseListBox2)
    //{{AFX_MSG_MAP(CSelectListBox)
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragItem)
    ON_WM_CONTEXTMENU()
    ON_WM_INITMENUPOPUP()
    ON_COMMAND_EX(ID_ACT_TURNOVER, OnActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_PREV, OnActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_RANDOM, OnActTurnOver)
    ON_COMMAND_EX(ID_ACT_TURNOVER_SELECT, OnActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER, OnUpdateActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_PREV, OnUpdateActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_RANDOM, OnUpdateActTurnOver)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER_SELECT, OnUpdateActTurnOver)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

const CTileManager& CSelectListBox::GetTileManager() const
{
    ASSERT(m_pDoc != NULL);
    return CheckedDeref(m_pDoc->GetTileManager());
}

/////////////////////////////////////////////////////////////////////////////

BOOL CSelectListBox::OnDragSetup(DragInfo& pDI) const
{
    ASSERT(!"untested code");
    if (GetCount() <= 1)
    {
        pDI.m_dragType = DRAG_INVALID;
        return FALSE;
    }

    if (!IsMultiSelect())
    {
        ASSERT(!"unreachable code");
        /* if this ever happens, rewrite like LBoxGrfx w/
            list/single distinction
        m_multiSelList.clear();
        m_multiSelList.push_back(&GetCurMapItem());
        */
    }
    pDI.m_dragType = DRAG_SELECTVIEW;
    pDI.GetSubInfo<DRAG_SELECTVIEW>().m_ptrArray = &GetMappedMultiSelectList();
    pDI.m_hcsrSuggest = g_res.hcrDragTile;
    pDI.GetSubInfo<DRAG_SELECTVIEW>().m_gamDoc = m_pDoc;
    return TRUE;
}

LRESULT CSelectListBox::OnDragItem(WPARAM wParam, LPARAM lParam)
{
    if (wParam != GetProcessId(GetCurrentProcess()))
    {
        return -1;
    }
    const DragInfo& pdi = CheckedDeref(reinterpret_cast<const DragInfo*>(lParam));

    DoInsertLineProcessing(pdi);

    if (pdi.m_dragType != DRAG_SELECTVIEW)
        return -1;               // Only our drops allowed

    ASSERT(!"untested code");
    if (pdi.GetSubInfo<DRAG_SELECTVIEW>().m_gamDoc != m_pDoc)
        return -1;               // Only pieces from our document.

    DoAutoScrollProcessing(pdi);

    if (pdi.m_phase == PhaseDrag::Over)
        return (LRESULT)(LPVOID)pdi.m_hcsrSuggest;
    else if (pdi.m_phase == PhaseDrag::Drop)
    {
        int nSel = SpecialItemFromPoint(pdi.m_point);

        if (nSel < GetCount())
        {
            // If the selection is out of view, force it into view.
            MakeItemVisible(nSel);
        }
    }
    return 1;
}

void CSelectListBox::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    // remember clicked side in case of ID_ACT_TURNOVER_SELECT
    CPoint clientPoint(point);
    ScreenToClient(&clientPoint);
    CRect rect;
    menuGameElement = OnGetHitItemCodeAtPoint(&CGamDoc::GetGameElementCodeForObject, clientPoint, rect);

    CMenu bar;
    if (bar.LoadMenu(IDR_MENU_PLAYER_POPUPS))
    {
        CMenu& popup = *bar.GetSubMenu(MENU_PV_SELCT_BOX);
        ASSERT(popup.m_hMenu != NULL);

        // Make sure we clean up even if exception is tossed.
        TRY
        {
            popup.TrackPopupMenu(TPM_LEFTBUTTON |
                                    TPM_LEFTALIGN |
                                    TPM_RIGHTBUTTON,
                point.x, point.y, this); // Route commands through tray window
        }
        END_TRY
    }
    else
    {
        ASSERT(!"LoadMenu error");
    }
}

void CSelectListBox::OnInitMenuPopup(CMenu* pMenu, UINT /*nIndex*/, BOOL bSysMenu)
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

BOOL CSelectListBox::OnActTurnOver(UINT id)
{
    CPlayBoardView& view = GetBoardView();
    switch (id)
    {
        case ID_ACT_TURNOVER:
        case ID_ACT_TURNOVER_PREV:
        case ID_ACT_TURNOVER_RANDOM:
        {
            bool b = view.OnCmdMsg(id, CN_COMMAND, nullptr, nullptr);
            ASSERT(b);
            return b;
        }
        case ID_ACT_TURNOVER_SELECT:
        {
            const CPlayBoard& playBoard = CheckedDeref(view.GetPlayBoard());

            m_pDoc->AssignNewMoveGroup();

            PieceID pid = static_cast<PieceID>(menuGameElement);
            auto it = std::find_if(GetItemMap()->begin(),
                                    GetItemMap()->end(),
                                    [pid](CB::not_null<CDrawObj*> drawObj)
                                    {
                                        if (drawObj->GetType() != CDrawObj::drawPieceObj)
                                        {
                                            return false;
                                        }
                                        CPieceObj& pieceObj = static_cast<CPieceObj&>(*drawObj);
                                        return pieceObj.m_pid == pid;
                                    });
            ASSERT(it != GetItemMap()->end());
            CDrawObj& drawObj = **it;
            ASSERT(drawObj.GetType() == CDrawObj::drawPieceObj);
            CPieceObj& pieceObj = static_cast<CPieceObj&>(drawObj);
            size_t side = menuGameElement.GetSide();
            m_pDoc->InvertPlayingPieceOnBoard(pieceObj, playBoard, CPieceTable::fSelect, side);

            return true;
        }
        default:
            AfxThrowInvalidArgException();
    }
}

void CSelectListBox::OnUpdateActTurnOver(CCmdUI* pCmdUI)
{
    switch (pCmdUI->m_nID)
    {
        case ID_ACT_TURNOVER:
        case ID_ACT_TURNOVER_PREV:
        case ID_ACT_TURNOVER_RANDOM:
            pCmdUI->DoUpdate(&GetBoardView(), TRUE);
            break;
        case ID_ACT_TURNOVER_SELECT:
        {
            bool enable = menuGameElement != Invalid_v<GameElement> &&
                        menuGameElement.IsAPiece();
            pCmdUI->Enable(enable);
            if (pCmdUI->m_pSubMenu != NULL)
            {
                // Need to handle menu that the submenu is connected to.
                pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
                    MF_BYPOSITION | (enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
            }
            break;
        }
        default:
            AfxThrowInvalidArgException();
    }
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing

BOOL CSelectListBox::OnIsToolTipsEnabled() const
{
    return m_pDoc->IsShowingObjectTips();
}

GameElement CSelectListBox::OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) const
{
    return OnGetHitItemCodeAtPoint(&CGamDoc::GetVerifiedGameElementCodeForObject, point, rct);
}

GameElement CSelectListBox::OnGetHitItemCodeAtPoint(GetGameElementCodeForObject_t func, CPoint point, CRect& rct) const
{
    point = ClientToItem(point);

    BOOL bOutsideClient;
    UINT nIndex = ItemFromPoint(point, bOutsideClient);
    if (nIndex >= 65535 || GetCount() <= 0)
        return Invalid_v<GameElement>;

    std::vector<TileID> tids = GetTileIDs(nIndex);
    ASSERT(!tids.empty() && tids[size_t(0)] != nullTid);

    std::vector<CRect> rcts = GetTileRectsForItem(value_preserving_cast<size_t>(nIndex), tids);

    for (size_t i = size_t(0); i < rcts.size(); ++i)
    {
        ASSERT(!rcts[i].IsRectEmpty());
        if (rcts[i].PtInRect(point))
        {
            rct = ItemToClient(rcts[i]);
            const CDrawObj& pObj = MapIndexToItem(nIndex);
            if (pObj.GetType() == CDrawObj::drawPieceObj)
            {
                const CPieceObj& pieceObj = static_cast<const CPieceObj&>(pObj);
                CPieceTable& pieceTbl = CheckedDeref(m_pDoc->GetPieceTable());
                uint8_t side = pieceTbl.GetSide(pieceObj.m_pid, i);
                return (m_pDoc->*func)(pieceObj, side);
            }
            else
            {
                return (m_pDoc->*func)(pObj, Invalid_v<size_t>);
            }
        }
    }

    return Invalid_v<GameElement>;
}

void CSelectListBox::OnGetTipTextForItemCode(GameElement nItemCode,
    CString& strTip, CString& strTitle) const
{
    if (nItemCode == Invalid_v<GameElement>)
        return;
    strTip = m_pDoc->GetGameElementString(nItemCode);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CSelectListBox::OnDoesItemHaveTipText(size_t nItem) const
{
    const CDrawObj& pDObj = MapIndexToItem(nItem);
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        const CPieceObj& pObj = static_cast<const CPieceObj&>(pDObj);
        CPieceTable& pieceTbl = CheckedDeref(m_pDoc->GetPieceTable());
        size_t sides = pieceTbl.GetSides(pObj.m_pid);
        for (size_t i = size_t(0); i < sides; ++i)
        {
            GameElement elem = m_pDoc->GetVerifiedGameElementCodeForObject(pObj, i);
            if (elem != Invalid_v<GameElement>)
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        GameElement elem = m_pDoc->GetVerifiedGameElementCodeForObject(pDObj);
        return elem != Invalid_v<GameElement>;
    }
}

/////////////////////////////////////////////////////////////////////////////

std::string CSelectListBox::OnGetItemDebugString(size_t nIndex) const
{
    const CDrawObj& pDObj = MapIndexToItem(nIndex);
    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        PieceID pid = static_cast<const CPieceObj&>(pDObj).m_pid;
        return CB::Sprintf("[pid:%u] ", value_preserving_cast<UINT>(static_cast<PieceID::UNDERLYING_TYPE>(pid)));
    }
    else if (pDObj.GetType() == CDrawObj::drawMarkObj)
    {
        MarkID mid = static_cast<const CMarkObj&>(pDObj).m_mid;
        return CB::Sprintf("[mid:%u] ", value_preserving_cast<UINT>(static_cast<MarkID::UNDERLYING_TYPE>(mid)));
    }
    return std::string();
}

/////////////////////////////////////////////////////////////////////////////

CSize CSelectListBox::OnItemSize(size_t nIndex) const
{
    ASSERT(m_pDoc != NULL);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    std::vector<TileID> tids = GetTileIDs(nIndex);
    ASSERT(!tids.empty() && tids[size_t(0)] != nullTid);

    return DoOnItemSize(nIndex, tids);
}

void CSelectListBox::OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
    CRect rctItem) const
{
    // see https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-drawitemstruct
    if (nIndex == size_t(UINT(-1)))
        return;                 // Nothing to draw.

    std::vector<TileID> tids = GetTileIDs(nIndex);
    ASSERT(!tids.empty() && tids[size_t(0)] != nullTid);
    DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tids);
}

// retval[0] is active face, followed by inactives
std::vector<TileID> CSelectListBox::GetTileIDs(size_t nIndex) const
{
    const CDrawObj& pDObj = MapIndexToItem(nIndex);

    if (pDObj.GetType() == CDrawObj::drawPieceObj)
    {
        CPieceTable* pPTbl = m_pDoc->GetPieceTable();
        ASSERT(pPTbl != NULL);

        PieceID pid = static_cast<const CPieceObj&>(pDObj).m_pid;

        if (!m_pDoc->IsScenario() && pPTbl->IsPieceOwned(pid) &&
            !pPTbl->IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask()))
        {
            std::vector<TileID> retval;
            retval.push_back(pPTbl->GetFrontTileID(pid));
            return retval;
        }

        const PieceDef& pPce = m_pDoc->GetPieceManager()->GetPiece(pid);

        std::vector<TileID> retval;
        if ((pPce.m_flags & PieceDef::flagShowOnlyVisibleSide) &&
            (!pPTbl->IsPieceOwnedBy(pid, m_pDoc->GetCurrentPlayerMask()) ||
             pPce.m_flags & PieceDef::flagShowOnlyOwnersToo))
        {
            retval.push_back(pPTbl->GetActiveTileID(pid));
        }
        else
        {
            retval.reserve(pPTbl->GetSides(pid));
            retval.push_back(pPTbl->GetActiveTileID(pid));
            std::vector<TileID> inactives = pPTbl->GetInactiveTileIDs(pid);
            retval.insert(retval.end(), inactives.begin(), inactives.end());
        }
        return retval;

    }
    else if (pDObj.GetType() == CDrawObj::drawMarkObj)
    {
        MarkID mid = static_cast<const CMarkObj&>(pDObj).m_mid;
        CMarkManager& pMMgr = CheckedDeref(m_pDoc->GetMarkManager());
        std::vector<TileID> retval;
        retval.push_back(pMMgr.GetMark(mid).m_tid);
        return retval;
    }
    else
    {
        ASSERT(FALSE);                              // Shouldn't happen
        return std::vector<TileID>();
    }
}

const CPlayBoardView& CSelectListBox::GetBoardView() const
{
    CFrameWnd& frame = CheckedDeref(AFXGetParentFrame(this));
    const CPlayBoardFrame& pbrdFrame = CheckedDeref(DYNAMIC_DOWNCAST(CPlayBoardFrame, &frame));
    return pbrdFrame.GetActiveBoardView();
}

