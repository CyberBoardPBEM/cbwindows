// LBoxGfx2.cpp
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

//  This class does the same things as CGrafixListBox except it uses
//  void* sized item maps.

#include    "stdafx.h"
#include    "LBoxGfx2.h"
#include    "ResTbl.h"
#include    "GdiTools.h"
#if defined(GPLAY)
#include    "GamDoc.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static DragInfo di;

const int TRIGGER_THRESHOLD = 3;
const unsigned defaultItemHeight = 16;

const int scrollZonePixels = 7;         // size of autoscroll trigger zone
const int timerScrollStart = 180;
const int timerScroll = 125;
const uintptr_t timerScrollIDStart = 900;
const int timerScrollID = 901;

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGrafixListBox2, CListBox)
    //{{AFX_MSG_MAP(CGrafixListBox2)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_WM_CREATE()
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragItem)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CGrafixListBox2::CGrafixListBox2()
{
    m_nLastInsert = -1;
    m_nTimerID = uintptr_t(0);
    m_bAllowDrag = FALSE;
    m_bAllowSelfDrop = FALSE;
    m_bAllowDropScroll = FALSE;
    m_pItemMap = NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBox2::SetItemMap(const std::vector<CB::not_null<CDrawObj*>>* pMap,
    BOOL bKeepPosition /* = TRUE */)
{
    m_pItemMap = pMap;
    UpdateList(bKeepPosition);
}

// bKeepPosition == TRUE means current selection is maintained.
void CGrafixListBox2::UpdateList(BOOL bKeepPosition /* = TRUE */)
{
    if (m_pItemMap == NULL)
    {
        ResetContent();
        return;
    }
    int nCurSel = GetCurSel();
    int nTopIdx = GetTopIndex();
    int nFcsIdx = GetCaretIndex();
    int horzScroll = GetScrollPos(SB_HORZ);
    SetRedraw(FALSE);
    ResetContent();

    LONG width = 0;
    int nItem;
    for (nItem = 0; nItem < value_preserving_cast<int>(m_pItemMap->size()); nItem++)
    {
        AddString(" ");             // Fill with dummy data
        width = std::max(width, OnItemSize(value_preserving_cast<size_t>(nItem)).cx);
    }
    SetHorizontalExtent(value_preserving_cast<int>(width));
    if (bKeepPosition)
    {
        if (nTopIdx >= 0)
            SetTopIndex(std::min(nTopIdx, nItem - 1));
        if (nFcsIdx >= 0)
            SetCaretIndex(std::min(nFcsIdx, nItem - 1), FALSE);
        if (nCurSel >= 0)
            SetCurSel(std::min(nCurSel, nItem - 1));
    }
    SetRedraw(TRUE);
    if (bKeepPosition)
    {
        SendMessage(WM_HSCROLL, MAKELONG(int16_t(SB_THUMBPOSITION), value_preserving_cast<int16_t>(horzScroll)), NULL);
    }
    Invalidate();
}

/////////////////////////////////////////////////////////////////////////////

const CDrawObj& CGrafixListBox2::MapIndexToItem(size_t nIndex) const
{
    ASSERT(m_pItemMap);
    ASSERT(nIndex < m_pItemMap->size());
    return *m_pItemMap->at(nIndex);
}

size_t CGrafixListBox2::MapItemToIndex(const CDrawObj& pItem) const
{
    ASSERT(m_pItemMap);
    for (size_t i = 0; i < m_pItemMap->size(); i++)
    {
        if (&pItem == m_pItemMap->at(i))
            return i;
    }
    return Invalid_v<size_t>;                  // Failed to find it
}

const CDrawObj& CGrafixListBox2::GetCurMapItem() const
{
    ASSERT(!IsMultiSelect());
    ASSERT(m_pItemMap);
    int nItem = GetCurSel();
    ASSERT(nItem >= 0);
    ASSERT(value_preserving_cast<size_t>(nItem) < m_pItemMap->size());
    return *m_pItemMap->at(value_preserving_cast<size_t>(nItem));
}

std::vector<CB::not_null<const CDrawObj*>> CGrafixListBox2::GetCurMappedItemList() const
{
    std::vector<CB::not_null<const CDrawObj*>> pLst;
    ASSERT(IsMultiSelect());
    int nSels = GetSelCount();
    if (nSels == LB_ERR || nSels == 0)
        return pLst;
    std::vector<int> pSelTbl(value_preserving_cast<size_t>(nSels));
    GetSelItems(nSels, pSelTbl.data());
    pLst.reserve(pSelTbl.size());
    for (size_t i = size_t(0); i < pSelTbl.size(); i++)
        pLst.push_back(&MapIndexToItem(value_preserving_cast<size_t>(pSelTbl[i])));
    return pLst;
}

void CGrafixListBox2::SetCurSelMapped(const CDrawObj& nMapVal)
{
    ASSERT(m_pItemMap);
    for (size_t i = 0; i < m_pItemMap->size(); i++)
    {
        if (m_pItemMap->at(i) == &nMapVal)
        {
            SetCurSel(value_preserving_cast<int>(i));
            SetTopIndex(value_preserving_cast<int>(i));
        }
    }
}

void CGrafixListBox2::SetCurSelsMapped(const std::vector<CB::not_null<CDrawObj*>>& items)
{
    ASSERT(m_pItemMap);
    ASSERT(IsMultiSelect());

    SetSel(-1, FALSE);      // Deselect all
    for (size_t i = 0; i < items.size(); i++)
    {
        for (size_t j = 0; j < m_pItemMap->size(); j++)
        {
            if (m_pItemMap->at(j) == items[i])
                SetSel(value_preserving_cast<int>(j));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBox2::ShowFirstSelection()
{
    int nTopSel = GetTopSelectedItem();
    CRect rctLBoxClient;
    GetClientRect(&rctLBoxClient);
    CRect rct;
    GetItemRect(nTopSel, &rct);
    if (!rct.IntersectRect(rct, rctLBoxClient))
        SetTopIndex(nTopSel);
}

int CGrafixListBox2::GetTopSelectedItem() const
{
    int nTopSel;
    if (IsMultiSelect())
    {
        if (GetSelCount() == 0)
            return LB_ERR;
        VERIFY(GetSelItems(1, &nTopSel));
    }
    else
    {
        nTopSel = GetCurSel();
        if (nTopSel == LB_ERR)
            return LB_ERR;
    }
    return nTopSel;
}

/////////////////////////////////////////////////////////////////////////////
// If the selection is out of view, force it into view.

void CGrafixListBox2::MakeItemVisible(int nItem)
{
    CRect rctLBoxClient;
    GetClientRect(&rctLBoxClient);
    CRect rct;
    GetItemRect(nItem, &rct);
    if (!rct.IntersectRect(rct, rctLBoxClient))
        SetTopIndex(nItem);
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBox2::SetSelFromPoint(CPoint point)
{
    // Short circuit drag processing
    m_bAllowDrag = FALSE;
    SendMessage(WM_LBUTTONDOWN, (WPARAM)MK_LBUTTON,
        MAKELPARAM(static_cast<int16_t>(point.x), static_cast<int16_t>(point.y)));
    SendMessage(WM_LBUTTONUP, (WPARAM)MK_LBUTTON,
        MAKELPARAM(static_cast<int16_t>(point.x), static_cast<int16_t>(point.y)));
    m_bAllowDrag = TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBox2::DoToolTipHitProcessing(CPoint point)
{
    if (!OnIsToolTipsEnabled())
        return;

    if (m_toolTip.m_hWnd == NULL)
    {
        m_toolTip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX);
        m_toolTip.SetMaxTipWidth(MAX_LISTITEM_TIP_WIDTH);
    }

    CRect rctTool;
    GameElement nItemCode = OnGetHitItemCodeAtPoint(point, rctTool);

    if (nItemCode != m_nCurItemCode)
    {
        // Object changed so delete previous tool definition
        m_toolTip.DelTool(this, ID_TIP_LISTITEM_HIT);
        m_nCurItemCode = nItemCode;
        if (nItemCode != Invalid_v<GameElement>)
        {
            // New object found so create a new tip
            CB::string strTip;
            CB::string strTitle;

            // Call subclass for info
            OnGetTipTextForItemCode(nItemCode, strTip, strTitle);

            if (!strTip.empty())
            {
                m_toolTip.AddTool(this, strTip, rctTool, ID_TIP_LISTITEM_HIT);

                if (!strTitle.empty())
                    m_toolTip.SendMessage(TTM_SETTITLE, 0, (LPARAM)strTitle.v_str());

                m_toolTip.Activate(TRUE);
            }
        }
        else
            m_toolTip.Activate(FALSE);
    }
}

LRESULT CGrafixListBox2::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (m_toolTip.m_hWnd != NULL && message >= WM_MOUSEFIRST &&
        message <= WM_MOUSELAST)
    {
        MSG msg;
        memset(&msg, 0, sizeof(MSG));
        msg.hwnd = m_hWnd;
        msg.message = message;
        msg.wParam = wParam;
        msg.lParam = lParam;

        m_toolTip.RelayEvent(&msg);
    }
    return CListBox::WindowProc(message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBox2::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
    unsigned nHt = value_preserving_cast<unsigned>(OnItemSize(lpMIS->itemID).cy);

    if (nHt >= 256) nHt = 255;
    if (nHt == 0) nHt = defaultItemHeight;

    lpMIS->itemHeight = nHt;
}

void CGrafixListBox2::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    size_t nIndex = lpDIS->itemID;
    CDC* pDC = CDC::FromHandle(lpDIS->hDC);

    CRect rct(lpDIS->rcItem);
    OnItemDraw(CheckedDeref(pDC), nIndex, lpDIS->itemAction, lpDIS->itemState, rct);
}

/////////////////////////////////////////////////////////////////////////////
// CGrafixListBox2 Message Processing

void CGrafixListBox2::OnLButtonDown(UINT nFlags, CPoint point)
{
    CListBox::OnLButtonDown(nFlags, point); // Allow field selection

    int nIdx;
    if ((nIdx = GetCurSel()) == -1)
        return;
    if (m_bAllowDrag)
    {
        m_hLastWnd = NULL;
        m_clickPoint = point;       // For hysteresis
        m_triggeredCursor = FALSE;  // -Ditto-
    }
}

void CGrafixListBox2::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_nTimerID)
    {
        KillTimer(m_nTimerID);
        m_nTimerID = uintptr_t(0);
    }
    if (m_bAllowDrag)
    {
        ASSERT(!"unreachable code");
        BOOL bWasDragging = CWnd::GetCapture() == this;
        CListBox::OnLButtonUp(nFlags, point);

        // Get the final selection results after the mouse was released.
        if (IsMultiSelect())
            m_multiSelList = GetCurMappedItemList();

        if (bWasDragging && m_triggeredCursor)
        {
#if defined(GPLAY)
            m_pDoc->AssignNewMoveGroup();
#endif
            if (IsMultiSelect())
            {
                CWnd *pWnd = GetParent();
                ASSERT(pWnd != NULL);
                pWnd->SendMessage(WM_OVERRIDE_SELECTED_ITEM_LIST2, (WPARAM)&m_multiSelList);
            }
            else
            {
                ASSERT(!"unreachable code");
                ASSERT(!"what is m_dragType here?");
#if 0
                // The parent may want to override the value.
                int nValueOverride = di.m_dwVal;
                CWnd *pWnd = GetParent();
                ASSERT(pWnd != NULL);
                pWnd->SendMessage(WM_OVERRIDE_SELECTED_ITEM2, (WPARAM)&nValueOverride);
                di.m_dwVal = nValueOverride;
#endif
            }

            ReleaseCapture();
            SetCursor(LoadCursor(NULL, IDC_ARROW));

            CWnd* pWnd = GetWindowFromPoint(point);
            if (pWnd == NULL || (!m_bAllowSelfDrop && pWnd == this))
            {
                OnDragCleanup(di);         // Tell subclass we're all done.
                return;
            }
            di.m_point = point;
            di.m_pointClient = point;       // list box relative
            ClientToScreen(&di.m_point);
            pWnd->ScreenToClient(&di.m_point);

            di.m_phase = PhaseDrag::Drop;
            pWnd->SendMessage(WM_DRAGDROP, GetProcessId(GetCurrentProcess()),
                (LPARAM)(LPVOID)&di);
            OnDragCleanup(di);             // Tell subclass we're all done.
            m_multiSelList.clear();
        }
    }
    else
        CListBox::OnLButtonUp(nFlags, point);
}

void CGrafixListBox2::OnMouseMove(UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() != this)
    {
        // Only process tool tips when we aren't draggin stuff around
        DoToolTipHitProcessing(point);
    }

    if (m_bAllowDrag)
    {
        ASSERT(!"unreachable code");
        if (CWnd::GetCapture() != this)
            return;
        // OK...We are dragging. Let's check if the cursor has been
        // triggered. If not, check if we've moved far enough from
        // the initial mouse down position.
        if (!m_triggeredCursor)
        {
            if (abs(point.x - m_clickPoint.x) < TRIGGER_THRESHOLD &&
                abs(point.y - m_clickPoint.y) < TRIGGER_THRESHOLD)
            {
                return;
            }
            m_triggeredCursor = TRUE;
        }
        OnDragSetup(di);           // Get stuff from subclass

        // If we got here, dragging is under way....
        CWnd* pWnd = GetWindowFromPoint(point);
        if (pWnd == NULL)
        {
            // No window underneath
            SetCursor(g_res.hcrNoDrop);
            return;
        }
        if (pWnd == this && !m_bAllowSelfDrop)
        {
            // The mouse is over us...show drop cursor
            SetCursor(g_res.hcrDragTile);
            return;
        }
        HWND hWnd = pWnd->m_hWnd;       // Get actual window handle
        HCURSOR hCursor = NULL;

        di.m_point = point;
        di.m_pointClient = point;       // list box relative
        ClientToScreen(&di.m_point);
        pWnd->ScreenToClient(&di.m_point);

        if (hWnd != m_hLastWnd)
        {
            if (m_hLastWnd != NULL)
            {
                // Inform previous window we are leaving them
                CWnd* pLstWnd = CWnd::FromHandle(m_hLastWnd);
                di.m_phase = PhaseDrag::Exit;
                pLstWnd->SendMessage(WM_DRAGDROP, GetProcessId(GetCurrentProcess()),
                    (LPARAM)(LPVOID)&di);
            }
            di.m_phase = PhaseDrag::Enter;
            pWnd->SendMessage(WM_DRAGDROP, GetProcessId(GetCurrentProcess()), (LPARAM)(LPVOID)&di);
        }
        m_hLastWnd = pWnd->m_hWnd;
        di.m_phase = PhaseDrag::Over;
        hCursor = (HCURSOR)pWnd->SendMessage(WM_DRAGDROP, GetProcessId(GetCurrentProcess()),
                (LPARAM)(LPVOID)&di);
        if (hCursor)
            SetCursor(hCursor);
        else
            SetCursor(g_res.hcrNoDrop);
    }
    else
        CListBox::OnMouseMove(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////

CWnd* CGrafixListBox2::GetWindowFromPoint(CPoint point) const
{
    ClientToScreen(&point);
    CWnd* pWnd = WindowFromPoint(point);

    if (pWnd == NULL)
        return NULL;

    pWnd->ScreenToClient(&point);
    pWnd = pWnd->ChildWindowFromPoint(point);

    if (pWnd == NULL)
        return NULL;

    return pWnd->IsWindowVisible() ? pWnd : NULL;
}

int CGrafixListBox2::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CListBox::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_pItemMap = NULL;
    m_nTimerID = uintptr_t(0);

    return 0;
}

/////////////////////////////////////////////////////////////////

void CGrafixListBox2::DoInsertLineProcessing(const DragInfo& pdi)
{
    if (m_bAllowDropScroll)
    {
        // Handle drawing of insert line
        CPoint pnt = pdi.m_point;
        int nSel = SpecialItemFromPoint(pnt);
        if (pdi.m_phase == PhaseDrag::Enter)
        {
            m_nLastInsert = nSel;
            DrawSingle(m_nLastInsert);          // Turn line on
        }
        else if (pdi.m_phase == PhaseDrag::Exit || pdi.m_phase == PhaseDrag::Drop)
        {
            DrawSingle(m_nLastInsert);          // Turn line off
            m_nLastInsert = -1;
        }
        else
        {
            DrawInsert(nSel);                   // Move insert line
        }
    }
}

/////////////////////////////////////////////////////////////////

void CGrafixListBox2::DoAutoScrollProcessing(const DragInfo& pdi)
{
    if (m_bAllowDropScroll && m_nTimerID == uintptr_t(0))
    {
        CRect rct;
        GetClientRect(&rct);
        rct.InflateRect(0, -scrollZonePixels);
        rct.NormalizeRect();
        if (!rct.PtInRect(pdi.m_point))
        {
            // Trigger time is usually longer
            m_nTimerID = SetTimer(timerScrollIDStart, timerScrollStart, NULL);
        }
    }
}

/////////////////////////////////////////////////////////////////
// Pass it up to the parent by default.

LRESULT CGrafixListBox2::OnDragItem(WPARAM wParam, LPARAM lParam)
{
    if (wParam != GetProcessId(GetCurrentProcess()))
    {
        return -1;
    }
    const DragInfo& pdi = CheckedDeref(reinterpret_cast<const DragInfo*>(lParam));
    DoInsertLineProcessing(pdi);

    CWnd *pWnd = GetParent();
    ASSERT(pWnd != NULL);
    LRESULT lResult = pWnd->SendMessage(WM_DRAGDROP, wParam, lParam);

    if (pdi.m_phase == PhaseDrag::Over && lResult != 0)
        DoAutoScrollProcessing(di);
    return lResult;
}

void CGrafixListBox2::OnTimer(uintptr_t nIDEvent)
{
    if (nIDEvent != m_nTimerID)
        return;

    CPoint point;
    CRect  rctClient;
    CRect  rct;

    GetCursorPos(&point);
    ScreenToClient(&point);
    GetClientRect(&rctClient);
    rct = rctClient;
    rct.InflateRect(0, -scrollZonePixels);
    rct.NormalizeRect();

    if (rctClient.PtInRect(point) && !rct.PtInRect(point))
    {
        // Restart the timer.
        if (m_nTimerID == timerScrollIDStart)
        {
            KillTimer(m_nTimerID);
            m_nTimerID = SetTimer(timerScrollID, timerScroll, NULL);
        }
        BOOL bHaveFocus = GetFocus() == this;
        if (bHaveFocus)
        {
            GetParent()->SetFocus();
            SetCapture();           // Reestablish the mouse capture
        }

        // Hide insert line
        DrawSingle(m_nLastInsert);

        int nTopIndex = GetTopIndex();
        if (point.y <= rct.top)
        {
            if (nTopIndex > 0)
                SetTopIndex(nTopIndex - 1);
        }
        else
        {
            if (nTopIndex < GetCount() - 1 && nTopIndex >= 0)
                SetTopIndex(nTopIndex + 1);
        }
        // Restore insert line in new position
        CPoint pnt;
        GetCursorPos(&pnt);
        ScreenToClient(&pnt);
        m_nLastInsert = SpecialItemFromPoint(pnt);
        DrawSingle(m_nLastInsert);

        if (bHaveFocus)
            SetFocus();
    }
    else
    {
        KillTimer(m_nTimerID);
        m_nTimerID = uintptr_t(0);
    }
}

/////////////////////////////////////////////////////////////////
// This routine selects the next item if the point is past
// the y midpoint of the item.

int CGrafixListBox2::SpecialItemFromPoint(CPoint pnt) const
{
    BOOL bBucket;
    int nSel = (int)ItemFromPoint(pnt, bBucket);
    CRect rct;
    GetItemRect(nSel, &rct);
    // Note: it is possible for the item number to exceed the
    // number of items in the listbox. This is figured into our
    // coding.
    if (pnt.y > (rct.top + rct.bottom) / 2)
        nSel++;                                 // Step to next item
    return nSel;
}

/////////////////////////////////////////////////////////////////

void CGrafixListBox2::DrawInsert(int nIndex)
{
    if (m_nLastInsert != nIndex)
    {
        DrawSingle(m_nLastInsert);
        DrawSingle(nIndex);
        m_nLastInsert = nIndex;
    }
}

void CGrafixListBox2::DrawSingle(int nIndex)
{
    if (nIndex == -1)
        return;
    CBrush* pBrush = CDC::GetHalftoneBrush();
    CRect rect;
    GetClientRect(&rect);
    CRgn rgn;
    rgn.CreateRectRgnIndirect(&rect);

    CDC* pDC = GetDC();
    // Prevent drawing outside of listbox. This can happen at the
    // top of the listbox since the listbox's DC is the parent's DC.
    pDC->SelectClipRgn(&rgn);

    // Account for possibility of nIndex equal to number
    // of listbox items....
    if (nIndex < GetCount())
    {
        GetItemRect(nIndex, &rect);
        rect.bottom = rect.top + 2;
        rect.top -= 2;
    }
    else
    {
        GetItemRect(nIndex - 1, &rect);
        rect.top = rect.bottom - 2;
        rect.bottom += 2;
    }

    CBrush* pBrushOld = pDC->SelectObject(pBrush);
    // Draw main line
    pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);

    pDC->SelectObject(pBrushOld);
    ReleaseDC(pDC);
}

CPoint CGrafixListBox2::ClientToItem(CPoint point) const
{
    // account for horz scroll
    int xOffset = GetScrollPos(SB_HORZ);
    ASSERT(xOffset >= 0);
    point.x += xOffset;
    return point;
}

CRect CGrafixListBox2::ItemToClient(CRect rect) const
{
    // account for horz scroll
    int xOffset = GetScrollPos(SB_HORZ);
    ASSERT(xOffset >= 0);
    rect.OffsetRect(-xOffset, 0);
    return rect;
}

