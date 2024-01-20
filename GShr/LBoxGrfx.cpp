// LBoxGrfx.cpp
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
#include    "LBoxGrfx.h"
#include    "ResTbl.h"
#include    "GdiTools.h"
#if defined(GPLAY)
#include    "GamDoc.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DragInfo CGrafixListBox::di;

const int TRIGGER_THRESHOLD = 3;
const unsigned defaultItemHeight = 16;

const int scrollZonePixels = 7;         // size of autoscroll trigger zone
const int timerScrollStart = 180;
const int timerScroll = 125;
const uintptr_t timerScrollIDStart = 900;
const int timerScrollID = 901;

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGrafixListBox, CListBox)
    //{{AFX_MSG_MAP(CGrafixListBox)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_TIMER()
    ON_WM_CREATE()
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragItem)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CGrafixListBox::CGrafixListBox() :
    m_nCurItemCode(Invalid_v<GameElement>)
{
    m_nLastInsert = -1;
    m_nTimerID = uintptr_t(0);
    m_bAllowDrag = FALSE;
    m_bAllowSelfDrop = FALSE;
    m_bAllowDropScroll = FALSE;
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBox::SetNotificationTip(int nItem, UINT nResID)
{
    CB::string str = CB::string::LoadString(nResID);
    SetNotificationTip(nItem, str);
}

// Shows a notification tip over a specified item. If the item
// doesn't exist, the center of the listbox receives the tip.
void CGrafixListBox::SetNotificationTip(int nItem, const CB::string& pszTip)
{
    if (m_toolMsgTip.m_hWnd == NULL)
    {
        m_toolMsgTip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX);
        m_toolMsgTip.SetMaxTipWidth(MAX_LISTITEM_TIP_WIDTH);
    }

    ClearNotificationTip();

    TOOLINFO ti;
    m_toolMsgTip.FillInToolInfo(ti, this, ID_TIP_LISTITEM_MSG);
    ti.uFlags |= TTF_TRACK;
    ti.lpszText = const_cast<CB::string::value_type*>(pszTip.v_str());

    m_toolMsgTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);

    MakeItemVisible(nItem);

    CRect rctItem;
    if (!GetItemRect(nItem, rctItem))
        GetClientRect(rctItem);

    CPoint pntClient = rctItem.CenterPoint();

    CPoint pntScreen(pntClient);
    ClientToScreen(&pntScreen);

    m_toolMsgTip.Activate(TRUE);
    m_toolMsgTip.SendMessage(TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
    m_toolMsgTip.SendMessage(TTM_TRACKPOSITION, 0,
        (LPARAM)MAKELONG(static_cast<int16_t>(pntScreen.x), static_cast<int16_t>(pntScreen.y)));

    SetTimer(ID_TIP_LISTITEM_MSG_TIMER, MAX_TIP_LISTITEM_MSG_TIME,
        NotificationTipTimeoutHandler);
}

void CGrafixListBox::ClearNotificationTip()
{
    KillTimer(ID_TIP_LISTITEM_MSG_TIMER);  // Kill it in case it's still running

    CToolInfo ti;
    m_toolMsgTip.GetToolInfo(ti, this, ID_TIP_LISTITEM_MSG);
    m_toolMsgTip.SendMessage(TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&ti);
    m_toolMsgTip.DelTool(this, ID_TIP_LISTITEM_MSG);
    m_toolMsgTip.Activate(FALSE);
}

void CALLBACK CGrafixListBox::NotificationTipTimeoutHandler(HWND hwnd,
    UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    CGrafixListBox* pWnd = (CGrafixListBox*)CWnd::FromHandle(hwnd);
    ASSERT(pWnd != NULL);
    pWnd->ClearNotificationTip();
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBox::ShowFirstSelection()
{
    int nTopSel = GetTopSelectedItem();
    CRect rctLBoxClient;
    GetClientRect(&rctLBoxClient);
    CRect rct;
    GetItemRect(nTopSel, &rct);
    if (!rct.IntersectRect(rct, rctLBoxClient))
        SetTopIndex(nTopSel);
}

CB::string CGrafixListBox::GetText(int nIndex) const
{
    CString temp;
    CListBox::GetText(nIndex, temp);
    return temp;
}

int CGrafixListBox::GetTopSelectedItem() const
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

void CGrafixListBox::MakeItemVisible(int nItem)
{
    CRect rctLBoxClient;
    GetClientRect(&rctLBoxClient);
    CRect rct;
    GetItemRect(nItem, &rct);
    if (!rct.IntersectRect(rct, rctLBoxClient))
        SetTopIndex(nItem);
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBox::SetSelFromPoint(CPoint point)
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

void CGrafixListBox::DoToolTipHitProcessing(CPoint point)
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

    if (nItemCode != m_nCurItemCode) // && nItemCode >= 0)
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
            OnGetTipTextForItemCode(nItemCode, strTip);

            if (!strTip.empty())
            {
                m_toolTip.AddTool(this, strTip, rctTool, ID_TIP_LISTITEM_HIT);
                m_toolTip.Activate(TRUE);
            }
        }
        else
            m_toolTip.Activate(FALSE);
    }
}

LRESULT CGrafixListBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST)
    {
        MSG msg;
        memset(&msg, 0, sizeof(MSG));
        msg.hwnd = m_hWnd;
        msg.message = message;
        msg.wParam = wParam;
        msg.lParam = lParam;
        if (m_toolTip.m_hWnd != NULL)
            m_toolTip.RelayEvent(&msg);
        if (m_toolMsgTip.m_hWnd != NULL)
            m_toolMsgTip.RelayEvent(&msg);
    }
    return CListBox::WindowProc(message, wParam, lParam);
}

CSize CGrafixListBox::GetDragSize() const
{
    CWnd& parent = CheckedDeref(GetParent());
    CSize retval;
    if (!parent.SendMessage(WM_GET_DRAG_SIZE, reinterpret_cast<WPARAM>(&retval)))
    {
        ASSERT(!"parent didn't provide drag size");
        /* parent didn't provide drag size,
            so assume worst case scenario */
        retval.cx = std::numeric_limits<decltype(retval.cx)>::max();
        retval.cy = std::numeric_limits<decltype(retval.cy)>::max();
    }
    return retval;
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
    unsigned nHt = value_preserving_cast<unsigned>(OnItemSize(value_preserving_cast<size_t>(lpMIS->itemID)).cy);

    if (nHt >= 256) nHt = 255;
    if (nHt == 0) nHt = defaultItemHeight;

    lpMIS->itemHeight = value_preserving_cast<UINT>(nHt);
}

void CGrafixListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    size_t nIndex = value_preserving_cast<size_t>(lpDIS->itemID);
    CDC& pDC = CheckedDeref(CDC::FromHandle(lpDIS->hDC));

    CRect rct(lpDIS->rcItem);
    OnItemDraw(pDC, nIndex, lpDIS->itemAction, lpDIS->itemState, rct);
}

/////////////////////////////////////////////////////////////////////////////
// CGrafixListBox Message Processing

void CGrafixListBox::OnLButtonDown(UINT nFlags, CPoint point)
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

void CGrafixListBox::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_nTimerID)
    {
        KillTimer(m_nTimerID);
        m_nTimerID = uintptr_t(0);
    }
    if (m_bAllowDrag)
    {
        BOOL bWasDragging = CWnd::GetCapture() == this;
        CListBox::OnLButtonUp(nFlags, point);

        if (bWasDragging && m_triggeredCursor)
        {
            OnDragEnd(point);
        }
    }
    else
        CListBox::OnLButtonUp(nFlags, point);
}

void CGrafixListBox::OnMouseMove(UINT nFlags, CPoint point)
{
    if (CWnd::GetCapture() != this)
    {
        // Only process tool tips when we aren't draggin stuff around
        DoToolTipHitProcessing(point);
    }

    if (m_bAllowDrag)
    {
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

CWnd* CGrafixListBox::GetWindowFromPoint(CPoint point)
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

int CGrafixListBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CListBox::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_nTimerID = uintptr_t(0);

    return 0;
}

/////////////////////////////////////////////////////////////////

void CGrafixListBox::DoInsertLineProcessing(const DragInfo& pdi)
{
    if (m_bAllowDropScroll)
    {
        // Handle drawing of insert line
        CPoint pnt = pdi.m_point;
        int nSel = SpecialItemFromPoint(pnt);
        if (pdi.m_phase == PhaseDrag::Enter)
        {
            m_nLastInsert = nSel;
            DrawSingle(m_nLastInsert);      // Turn line on
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

void CGrafixListBox::DoAutoScrollProcessing(const DragInfo& pdi)
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

LRESULT CGrafixListBox::OnDragItem(WPARAM wParam, LPARAM lParam)
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
        DoAutoScrollProcessing(pdi);
    return lResult;
}

void CGrafixListBox::OnTimer(uintptr_t nIDEvent)
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

int CGrafixListBox::SpecialItemFromPoint(CPoint pnt) const
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

void CGrafixListBox::DrawInsert(int nIndex)
{
    if (m_nLastInsert != nIndex)
    {
        DrawSingle(m_nLastInsert);
        DrawSingle(nIndex);
        m_nLastInsert = nIndex;
    }
}

void CGrafixListBox::DrawSingle(int nIndex)
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

CPoint CGrafixListBox::ClientToItem(CPoint point) const
{
    // account for horz scroll
    int xOffset = GetScrollPos(SB_HORZ);
    ASSERT(xOffset >= 0);
    point.x += xOffset;
    return point;
}

CRect CGrafixListBox::ItemToClient(CRect rect) const
{
    // account for horz scroll
    int xOffset = GetScrollPos(SB_HORZ);
    ASSERT(xOffset >= 0);
    rect.OffsetRect(-xOffset, 0);
    return rect;
}

void CGrafixListBox::AssignNewMoveGroup()
{
#if defined(GPLAY)
    CheckedDeref(m_pDoc).AssignNewMoveGroup();
#endif
}

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CGrafixListBoxWx, CB::VListBoxHScroll)
    EVT_LEFT_DOWN(OnLButtonDown)
    EVT_MOTION(OnMouseMove)
    EVT_LEFT_UP(OnLButtonUp)
    EVT_TIMER(wxID_ANY, OnTimer)
#if 0
    ON_WM_CREATE()
    ON_REGISTERED_MESSAGE(WM_DRAGDROP, OnDragItem)
#endif
wxEND_EVENT_TABLE()

CGrafixListBoxWx::CGrafixListBoxWx() //:
#if 0
    m_nCurItemCode(Invalid_v<GameElement>)
#endif
{
#if 0
    m_nLastInsert = -1;
    m_nTimerID = uintptr_t(0);
    m_bAllowDrag = FALSE;
    m_bAllowSelfDrop = FALSE;
    m_bAllowDropScroll = FALSE;
#endif
}

CGrafixListBoxWx::~CGrafixListBoxWx()
{
    if (m_toolTip)
    {
        m_toolTip->Close();
        wxASSERT(false);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBoxWx::DoToolTipHitProcessing(wxPoint point)
{
    if (!OnIsToolTipsEnabled())
        return;

    wxRect rctTool;
    GameElement nItemCode = OnGetHitItemCodeAtPoint(point, rctTool);

    if (nItemCode != m_nCurItemCode) // && nItemCode >= 0)
    {
        // Object changed so delete previous tool definition
        if (m_toolTip)
        {
            m_toolTip->Close();
            wxASSERT(!m_toolTip);
        }
        m_nCurItemCode = nItemCode;
        if (nItemCode != Invalid_v<GameElement>)
        {
            // New object found so create a new tip
            CB::string strTip;

            // Call subclass for info
            OnGetTipTextForItemCode(nItemCode, strTip);

            if (!strTip.empty())
            {
                wxRect screenTool(ClientToScreen(rctTool.GetTopLeft()), rctTool.GetSize());
                m_toolTip = new wxTipWindow(this, strTip, MAX_LISTITEM_TIP_WIDTH, &m_toolTip, &screenTool);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CGrafixListBoxWx Message Processing

void CGrafixListBoxWx::OnLButtonDown(wxMouseEvent& event)
{
    event.Skip(); // Allow field selection

#if 0
    /* N.B.:  this may need to be moved to a later msg handler
                due to needing Skip() */
    int nIdx;
    if ((nIdx = GetCurSel()) == -1)
        return;
    if (m_bAllowDrag)
    {
        m_hLastWnd = NULL;
        m_clickPoint = point;       // For hysteresis
        m_triggeredCursor = FALSE;  // -Ditto-
    }
#endif
}

void CGrafixListBoxWx::OnLButtonUp(wxMouseEvent& event)
{
#if 0
    if (m_nTimerID)
    {
        KillTimer(m_nTimerID);
        m_nTimerID = uintptr_t(0);
    }
    if (m_bAllowDrag)
    {
        BOOL bWasDragging = CWnd::GetCapture() == this;
        CListBox::OnLButtonUp(nFlags, point);

        if (bWasDragging && m_triggeredCursor)
        {
            OnDragEnd(point);
        }
    }
    else
#endif
        event.Skip();
}

void CGrafixListBoxWx::OnMouseMove(wxMouseEvent& event)
{
    if (!HasCapture())
    {
        // Only process tool tips when we aren't draggin stuff around
        DoToolTipHitProcessing(event.GetPosition());
    }

#if 0
    if (m_bAllowDrag)
    {
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
#endif
        event.Skip();
}

void CGrafixListBoxWx::OnTimer(wxTimerEvent& event)
{
#if 0
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
#endif
}



