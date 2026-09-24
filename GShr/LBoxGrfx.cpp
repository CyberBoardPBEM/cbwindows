// LBoxGrfx.cpp
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
#include    "LBoxGrfx.h"
#include    "ResTbl.h"
#include    "GdiTools.h"
#if defined(GPLAY)
#include    "GamDoc.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

wxDEFINE_EVENT(WM_OVERRIDE_SELECTED_ITEM_WX, OverrideSelectedItemEvent);
wxDEFINE_EVENT(WM_OVERRIDE_SELECTED_ITEM_LIST_WX, OverrideSelectedItemListEvent);
wxDEFINE_EVENT(WM_GET_DRAG_SIZE_WX, GetDragSizeEvent);

DragInfoWx CGrafixListBoxWx::di;

const int TRIGGER_THRESHOLD = 3;
const unsigned defaultItemHeight = 16;

const int scrollZonePixels = 7;         // size of autoscroll trigger zone
const int timerScrollStart = 180;
const int timerScroll = 125;
const uintptr_t timerScrollIDStart = 900;
const int timerScrollID = 901;

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CGrafixListBoxWx, CB::VListBoxHScroll)
    EVT_LEFT_DOWN(OnLButtonDown)
    EVT_LEFT_DCLICK(OnLButtonDblClk)
    EVT_MOTION(OnMouseMove)
    EVT_LEFT_UP(OnLButtonUp)
    EVT_TIMER(XRCID("ID_SCROLL_TIMER"), OnTimer)
#if 0
    ON_WM_CREATE()
#endif
    EVT_DRAGDROP(OnDragItem)
    EVT_TIMER(XRCID("ID_TIP_LISTITEM_MSG_TIMER"), NotificationTipTimeoutHandler)
wxEND_EVENT_TABLE()

/////////////////////////////////////////////////////////////////////////////

CGrafixListBoxWx::CGrafixListBoxWx() :
    m_toolMsgTipTimer(this, XRCID("ID_TIP_LISTITEM_MSG_TIMER")),
    m_nCurItemCode(Invalid_v<GameElement>),
    m_nTimerID(this, XRCID("ID_SCROLL_TIMER"))
{
    m_nLastInsert = wxNOT_FOUND;
    m_bAllowDrag = FALSE;
    m_bAllowSelfDrop = FALSE;
    m_bAllowDropScroll = FALSE;

    m_toolMsgTip.SetBalloonMode(true);
    m_toolMsgTip.SetMaxWidth(MAX_LISTITEM_TIP_WIDTH);
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

#if 0
void CGrafixListBoxWx::SetNotificationTip(int nItem, UINT nResID)
{
    CB::string str = CB::string::LoadString(nResID);
    SetNotificationTip(nItem, str);
}
#endif

// Shows a notification tip over a specified item. If the item
// doesn't exist, the center of the listbox receives the tip.
void CGrafixListBoxWx::SetNotificationTip(size_t nItem, const CB::string& pszTip)
{
    ClearNotificationTip();

    m_toolMsgTip.Add(*this, pszTip, CB::ToolTip::TRACK);

    MakeItemVisible(nItem);

    wxRect rctItem = GetItemRect(nItem);
    if (rctItem.IsEmpty())
        rctItem = GetClientRect();

    wxPoint pntClient = GetMidRect(rctItem);

    wxPoint pntScreen = ClientToScreen(pntClient);

    m_toolMsgTip.Enable(TRUE);
    m_toolMsgTip.TrackActivate(*this, true);
    m_toolMsgTip.TrackPosition(pntScreen);

    m_toolMsgTipTimer.Start(MAX_TIP_LISTITEM_MSG_TIME, wxTIMER_ONE_SHOT);
}

void CGrafixListBoxWx::ClearNotificationTip()
{
    m_toolMsgTipTimer.Stop();

    m_toolMsgTip.TrackActivate(*this, false);
    m_toolMsgTip.Delete(*this);
    m_toolMsgTip.Enable(FALSE);
}

void CGrafixListBoxWx::NotificationTipTimeoutHandler(wxTimerEvent& /*event*/)
{
    ClearNotificationTip();
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBoxWx::ShowFirstSelection()
{
    int nTopSel = GetTopSelectedItem();
    wxASSERT(nTopSel != wxNOT_FOUND);
    wxRect rctLBoxClient = GetClientRect();
    wxRect rct = GetItemRect(value_preserving_cast<size_t>(nTopSel));
    if (!rct.Intersects(rctLBoxClient))
    {
        ScrollToRow(value_preserving_cast<size_t>(nTopSel));
    }
}

#if 0
CB::string CGrafixListBoxWx::GetText(int nIndex) const
{
    CString temp;
    CListBox::GetText(nIndex, temp);
    return temp;
}
#endif

int CGrafixListBoxWx::GetTopSelectedItem() const
{
    if (IsMultiSelect())
    {
        if (GetSelectedCount() == 0)
        {
            return wxNOT_FOUND;
        }
        return value_preserving_cast<int>(GetSelections().front());
    }
    else
    {
        return GetSelection();
    }
}

/////////////////////////////////////////////////////////////////////////////
// If the selection is out of view, force it into view.

void CGrafixListBoxWx::MakeItemVisible(size_t nItem)
{
    wxRect rctLBoxClient = GetClientRect();
    wxRect rct = GetItemRect(nItem);
    if (!rct.Intersects(rctLBoxClient))
        ScrollToRow(nItem);
}

/////////////////////////////////////////////////////////////////////////////

void CGrafixListBoxWx::SetSelFromPoint(wxPoint point)
{
    wxASSERT(postProcessEvents.empty());
    if (GetItemCount() == size_t(0))
    {
        return;
    }
    int itemFromPoint = VirtualHitTest(point.y);
    if (itemFromPoint != wxNOT_FOUND)
    {
        SetSelection(itemFromPoint);
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
                m_toolTip = wxTipWindow::New(this, strTip, MAX_LISTITEM_TIP_WIDTH, &screenTool);
            }
        }
    }
}

#if 0
LRESULT CGrafixListBoxWx::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
#endif

wxSize CGrafixListBoxWx::GetDragSize() const
{
    wxWindow& parent = CheckedDeref(GetParent());
    GetDragSizeEvent event;
    if (!parent.ProcessWindowEvent(event))
    {
        wxASSERT(!"parent didn't provide drag size");
        /* parent didn't provide drag size,
            so assume worst case scenario */
        wxSize retval;
        retval.x = std::numeric_limits<decltype(retval.x)>::max();
        retval.y = std::numeric_limits<decltype(retval.y)>::max();
        event.SetSize(retval);
    }
    return event.GetSize();
}

/////////////////////////////////////////////////////////////////////////////

#if 0
void CGrafixListBoxWx::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
    unsigned nHt = value_preserving_cast<unsigned>(OnItemSize(value_preserving_cast<size_t>(lpMIS->itemID)).cy);

    if (nHt >= 256) nHt = 255;
    if (nHt == 0) nHt = defaultItemHeight;

    lpMIS->itemHeight = value_preserving_cast<UINT>(nHt);
}

void CGrafixListBoxWx::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    size_t nIndex = value_preserving_cast<size_t>(lpDIS->itemID);
    CDC& pDC = CheckedDeref(CDC::FromHandle(lpDIS->hDC));

    CRect rct(lpDIS->rcItem);
    OnItemDraw(pDC, nIndex, lpDIS->itemAction, lpDIS->itemState, rct);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CGrafixListBox Message Processing

void CGrafixListBoxWx::PushPostProcessEvent(std::function<void ()>&& f)
{
    // I'm pretty sure we should have at most one partial event
    /* TODO:  after confirming at most one,
                convert to std::optional<> instead of queue<> */
    wxASSERT(postProcessEvents.empty());
    postProcessEvents.push(std::move(f));
    CallAfter([this]{ ExecutePostProcessEvents(); });
}

void CGrafixListBoxWx::ExecutePostProcessEvents()
{
    // I'm pretty sure we should have at most one partial event
    wxASSERT(postProcessEvents.size() <= 1);
    while (!postProcessEvents.empty())
    {
        // need to pop before executing to avoid recursion
        std::function<void ()> f = std::move(postProcessEvents.front());
        postProcessEvents.pop();
        f();
    }
}

void CGrafixListBoxWx::OnLButtonDown(wxMouseEvent& event)
{
    ExecutePostProcessEvents();
    wxASSERT(!GetCapture());
    event.Skip(); // Allow field selection

    PushPostProcessEvent([this, event]{
        wxASSERT(GetCapture() == this);
        if (GetSelectedCount() == 0)
            return;
        if (m_bAllowDrag)
        {
            m_hLastWnd = NULL;
            m_clickPoint = event.GetPosition();       // For hysteresis
            m_triggeredCursor = FALSE;  // -Ditto-
        }
    });
}

// keep event processing in correct order
void CGrafixListBoxWx::OnLButtonDblClk(wxMouseEvent& event)
{
    ExecutePostProcessEvents();
    event.Skip();
}

void CGrafixListBoxWx::OnLButtonUp(wxMouseEvent& event)
{
    ExecutePostProcessEvents();
    if (m_nTimerID.IsRunning())
    {
        m_nTimerID.Stop();
    }
    if (m_bAllowDrag)
    {
        BOOL bWasDragging = wxWindow::GetCapture() == this;
        event.Skip();

        if (bWasDragging && m_triggeredCursor)
        {
            PushPostProcessEvent([this, event]{
                if (GetSelectedCount() != 0)
                {
                    OnDragEnd(event);
                }
            });
        }
    }
    else
        event.Skip();
}

void CGrafixListBoxWx::OnMouseMove(wxMouseEvent& event)
{
    ExecutePostProcessEvents();
    if (!HasCapture())
    {
        // Only process tool tips when we aren't draggin stuff around
        DoToolTipHitProcessing(event.GetPosition());
    }

    if (m_bAllowDrag)
    {
        // unlike MFC, wx can have no selection here
        if (wxWindow::GetCapture() != this ||
            GetSelectedCount() == 0)
        {
            return;
        }
        wxPoint point = event.GetPosition();
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
        wxWindow* pWnd = GetWindowFromPoint(point);
        if (pWnd == NULL)
        {
            // No window underneath
            SetCursor(g_res.hcrNoDropWx);
            return;
        }
        if (pWnd == this && !m_bAllowSelfDrop)
        {
            // The mouse is over us...show drop cursor
            SetCursor(g_res.hcrDragTileWx);
            return;
        }

        di.m_point = point;
        di.m_pointClient = point;       // list box relative
        di.m_point = ClientToScreen(di.m_point);
        di.m_point = pWnd->ScreenToClient(di.m_point);

        if (pWnd != m_hLastWnd)
        {
            if (m_hLastWnd != NULL)
            {
                // Inform previous window we are leaving them
                di.m_phase = PhaseDrag::Exit;
                DragDropEvent dragDropEvent(wxGetProcessId(), di);
                m_hLastWnd->ProcessWindowEvent(dragDropEvent);
            }
            di.m_phase = PhaseDrag::Enter;
            DragDropEvent dragDropEvent(wxGetProcessId(), di);
            pWnd->ProcessWindowEvent(dragDropEvent);
        }
        m_hLastWnd = pWnd;
        di.m_phase = PhaseDrag::Over;
        DragDropEvent dragDropEvent(wxGetProcessId(), di);
        pWnd->ProcessWindowEvent(dragDropEvent);
        wxCursor hCursor = dragDropEvent.GetCursor();
        if (hCursor.IsOk())
            SetCursor(hCursor);
        else
            SetCursor(g_res.hcrNoDropWx);
    }
    else
        event.Skip();
}

/////////////////////////////////////////////////////////////////////////////

wxWindow* CGrafixListBoxWx::GetWindowFromPoint(wxPoint point)
{
    point = ClientToScreen(point);
    return wxFindWindowAtPoint(point);
}

#if 0
int CGrafixListBoxWx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CListBox::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_nTimerID = uintptr_t(0);

    return 0;
}
#endif

/////////////////////////////////////////////////////////////////

void CGrafixListBoxWx::DoInsertLineProcessing(const DragInfoWx& pdi)
{
    if (m_bAllowDropScroll)
    {
        // Handle drawing of insert line
        wxPoint pnt = pdi.m_point;
        int nSel = SpecialItemFromPoint(pnt);
        if (pdi.m_phase == PhaseDrag::Enter)
        {
            m_nLastInsert = nSel;
            DrawSingle(m_nLastInsert);      // Turn line on
        }
        else if (pdi.m_phase == PhaseDrag::Exit || pdi.m_phase == PhaseDrag::Drop)
        {
            EraseSingle();          // Turn line off
            m_nLastInsert = wxNOT_FOUND;
        }
        else
        {
            DrawInsert(nSel);                   // Move insert line
        }
    }
}

/////////////////////////////////////////////////////////////////

void CGrafixListBoxWx::DoAutoScrollProcessing(const DragInfoWx& pdi)
{
    if (m_bAllowDropScroll && !m_nTimerID.IsRunning())
    {
        wxRect rct = GetClientRect();
        CB::InflateAndNormalize(rct, 0, -scrollZonePixels);
        if (!rct.Contains(pdi.m_point))
        {
            // Trigger time is usually longer
            CB_VERIFY(m_nTimerID.Start(timerScrollStart));
        }
    }
}

/////////////////////////////////////////////////////////////////
// Pass it up to the parent by default.

void CGrafixListBoxWx::OnDragItem(DragDropEvent& event)
{
    if (event.GetProcessId() != wxGetProcessId())
    {
        wxASSERT(!"bad event process");
        return;
    }
    const DragInfoWx& pdi = event.GetDragInfo();
    DoInsertLineProcessing(pdi);

    wxWindow& pWnd = CheckedDeref(GetParent());
    pWnd.ProcessWindowEvent(event);

    if (pdi.m_phase == PhaseDrag::Over && event.GetCursor().IsOk())
    {
        DoAutoScrollProcessing(pdi);
    }
}

void CGrafixListBoxWx::OnTimer(wxTimerEvent& event)
{
    wxASSERT(m_nTimerID.IsRunning() &&
                event.GetId() == m_nTimerID.GetId());
    if (!m_nTimerID.IsRunning())
        return;

    wxPoint point = wxGetMouseState().GetPosition();
    point = ScreenToClient(point);
    wxRect rctClient = GetClientRect();
    wxRect rct = rctClient;
    CB::InflateAndNormalize(rct, 0, -scrollZonePixels);

    if (rctClient.Contains(point) && !rct.Contains(point))
    {
        // Restart the timer.
        if (m_nTimerID.GetInterval() == timerScrollStart)
        {
            CB_VERIFY(m_nTimerID.Start(timerScroll));
        }
        BOOL bHaveFocus = HasCapture();
        if (bHaveFocus)
        {
            GetParent()->SetFocus();
            wxASSERT(HasCapture());
        }

        // Hide insert line
        EraseSingle();

        size_t nTopIndex = GetVisibleRowsBegin();
        if (point.y <= rct.GetTop())
        {
            if (nTopIndex > size_t(0))
                ScrollToRow(nTopIndex - size_t(1));
        }
        else
        {
            if (nTopIndex < GetItemCount() - size_t(1))
                ScrollToRow(nTopIndex + size_t(1));
        }

        // Restore insert line in new position
        wxPoint pnt = wxGetMouseState().GetPosition();
        pnt = ScreenToClient(pnt);
        m_nLastInsert = SpecialItemFromPoint(pnt);
        DrawSingle(m_nLastInsert);

        if (bHaveFocus)
            SetFocus();
    }
    else
    {
        m_nTimerID.Stop();
    }
}

/////////////////////////////////////////////////////////////////
// This routine selects the next item if the point is past
// the y midpoint of the item.

int CGrafixListBoxWx::SpecialItemFromPoint(wxPoint pnt) const
{
    int nSel = VirtualHitTest(pnt.y);
    if (nSel == wxNOT_FOUND)
    {
        return nSel;
    }
    wxRect rct = GetItemRect(value_preserving_cast<size_t>(nSel));
    // Note: it is possible for the item number to exceed the
    // number of items in the listbox. This is figured into our
    // coding.
    if (pnt.y > (rct.GetTop() + rct.GetBottom()) / 2)
        nSel++;                                 // Step to next item
    return nSel;
}

/////////////////////////////////////////////////////////////////

void CGrafixListBoxWx::DrawInsert(int nIndex)
{
    if (m_nLastInsert != nIndex)
    {
        EraseSingle();
        DrawSingle(nIndex);
        m_nLastInsert = nIndex;
    }
}

void CGrafixListBoxWx::DrawSingle(int nIndex)
{
    if (nIndex == wxNOT_FOUND)
        return;

    // Account for possibility of nIndex equal to number
    // of listbox items....
    wxRect rect;
    if (nIndex < value_preserving_cast<int>(GetItemCount()))
    {
        rect = GetItemRect(value_preserving_cast<size_t>(nIndex));
        rect = wxRect(wxPoint(rect.GetLeft(), rect.GetTop() - 2),
                        wxSize(rect.GetWidth(), 4));
    }
    else
    {
        rect = GetItemRect(value_preserving_cast<size_t>(nIndex - 1));
        rect = wxRect(wxPoint(rect.GetLeft(), rect.GetBottom() - 2),
                        wxSize(rect.GetWidth(), 4));
    }

    wxOverlayDC dc(overlay, this);
    dc.Clear();
    wxRendererNative& rend = wxRendererNative::Get();
    rend.DrawItemSelectionRect(this, dc, rect, wxCONTROL_FOCUSED | wxCONTROL_SELECTED);
}

void CGrafixListBoxWx::EraseSingle()
{
    overlay.Reset();
}

#if 0
CPoint CGrafixListBoxWx::ClientToItem(CPoint point) const
{
    // account for horz scroll
    int xOffset = GetScrollPos(SB_HORZ);
    ASSERT(xOffset >= 0);
    point.x += xOffset;
    return point;
}

CRect CGrafixListBoxWx::ItemToClient(CRect rect) const
{
    // account for horz scroll
    int xOffset = GetScrollPos(SB_HORZ);
    ASSERT(xOffset >= 0);
    rect.OffsetRect(-xOffset, 0);
    return rect;
}
#endif

void CGrafixListBoxWx::AssignNewMoveGroup()
{
#if defined(GPLAY)
    CheckedDeref(m_pDoc).AssignNewMoveGroup();
#endif
}
