// LBoxGrfx.h
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

#ifndef _LBOXGRFX_H
#define _LBOXGRFX_H

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

/////////////////////////////////////////////////////////////////////////////

#define     ID_TIP_LISTITEM_HIT             1       // ID used for normal tips
#define     ID_TIP_LISTITEM_MSG             2       // ID used for notify tips

#define     MAX_LISTITEM_TIP_WIDTH          200     // Max pixel width of tips

#define     ID_TIP_LISTITEM_MSG_TIMER       42      // ID used in tip timer
#define     MAX_TIP_LISTITEM_MSG_TIME       4000    // Max time for tip in ms.

/////////////////////////////////////////////////////////////////////////////
// These messages are sent by the CGrafixListBox to its parent window.
// It allows the list's content to be overridden. It's primary use
// is to deliver a random selection of pieces or markers.
#define WM_OVERRIDE_SELECTED_ITEM       (WM_USER + 500) // WPARAM = COverrideInfoBase<>*
struct COverrideInfoBase
{
protected:
    COverrideInfoBase(DragType dt) : m_dragType(dt) {}

    template<DragType DT>
    void CheckTypeBase() const
    {
        ASSERT(m_dragType == DT);
        /* TODO:  This check could be removed to improve release
            build performance if we trust ourselves to always
            catch any m_dragType mistakes in testing.  */
        if (m_dragType != DT)
        {
            AfxThrowInvalidArgException();
        }
    }

private:
    const DragType m_dragType;
};

template<DragType DT>
struct COverrideInfo
{
};

template<>
struct COverrideInfo<DRAG_MARKER> : private COverrideInfoBase
{
    COverrideInfo(MarkID& mid) : COverrideInfoBase(DRAG_MARKER), m_markID(mid) {}

    void CheckType() { CheckTypeBase<DRAG_MARKER>(); }

    MarkID& m_markID;
};

template<>
struct COverrideInfo<DRAG_TILE> : private COverrideInfoBase
{
    COverrideInfo(TileID& tid) : COverrideInfoBase(DRAG_TILE), m_tileID(tid) {}

    void CheckType() { CheckTypeBase<DRAG_TILE>(); }

    TileID& m_tileID;
};

#define WM_OVERRIDE_SELECTED_ITEM_LIST  (WM_USER + 501) // WPARAM = std::vector<XxxxID<LPARAM>>*, LPARAM = XxxxID::PREFIX

/////////////////////////////////////////////////////////////////////////////
// Custom Listbox - containing colors

/* I want CGrafixListBox::m_pItemMap to be type-specific.
    However, I don't want to replicate the message map for each
    different type of CGrafixListBox::m_pItemMap.  Therefore,
    split CGrafixListBox into a base class that contains the
    message map, and a templatized derived class that contains
    the type-specific CGrafixListBox::m_pItemMap. */

class CGrafixListBox : public CListBox
{
// Construction
public:
    CGrafixListBox();

// Attributes
public:
    int  GetTopSelectedItem();
    void EnableDrag(BOOL bEnable = TRUE) { m_bAllowDrag = bEnable; }
    void EnableSelfDrop(BOOL bEnable = TRUE) { m_bAllowSelfDrop = bEnable; }
    void EnableDropScroll(BOOL bEnable = TRUE) { m_bAllowDropScroll = bEnable; }
    BOOL IsMultiSelect() const
        { return (GetStyle() & (LBS_EXTENDEDSEL | LBS_MULTIPLESEL)) != 0; }

// Operations
public:
    void SetSelFromPoint(CPoint point);
    void ShowFirstSelection();
    void MakeItemVisible(int nItem);

    // Notification Tooltip Support
    void SetNotificationTip(int nItem, UINT nResID);
    void SetNotificationTip(int nItem, LPCTSTR pszTip);
    void ClearNotificationTip();
    static void CALLBACK NotificationTipTimeoutHandler(HWND hwnd, UINT uMsg,
        UINT_PTR idEvent, DWORD dwTime);

// Overrides - the subclass of this class must override these
public:
    virtual unsigned OnItemHeight(size_t nIndex) /* override */ = 0;
    virtual void OnItemDraw(CDC* pDC, size_t nIndex, UINT nAction, UINT nState,
        CRect rctItem) /* override */ = 0;
    virtual BOOL OnDragSetup(DragInfo* pDI) /* override */
    {
        pDI->m_dragType = DRAG_INVALID;
        return FALSE;
    }
    virtual void OnDragCleanup(DragInfo* pDI) /* override */ { }

    // For tool tip processing
    virtual BOOL OnIsToolTipsEnabled() /* override */ { return FALSE; }
    virtual int  OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) /* override */ { return -1; }
    virtual void OnGetTipTextForItemCode(int nItemCode, CString& strTip, CString& strTitle) /* override */ { }

    /* N.B.:  Conceptually, this declaration belongs to
        CTileBaseListBox, but it doesn't hurt much to declare it
        in general, and doing it here allows override checks of
        CGrafixListBoxData<>::OnGetItemDebugIDCode. */
    virtual int OnGetItemDebugIDCode(size_t nItem) /* override */ = 0;

// Implementation
protected:
    // Tool tip support
    CToolTipCtrl m_toolMsgTip;      // Tooltip for notifications
    CToolTipCtrl m_toolTip;         // Tooltip of tile text popups
    int     m_nCurItemCode;         // current active tip item code

    // Drag and scroll support vars
    static DragInfo di;
    BOOL    m_bAllowDrag;
    BOOL    m_bAllowSelfDrop;       // Only if m_bAllowDrag == TRUE
    BOOL    m_bAllowDropScroll;     // Scroll on OnDragItem

    CPoint  m_clickPoint;
    int     m_nTimerID;
    BOOL    m_triggeredCursor;
    HWND    m_hLastWnd;

    int     m_nLastInsert;          // Last index with insert line

    void  DoInsertLineProcessing(UINT nPhase, DragInfo* pdi);
    void  DoAutoScrollProcessing(DragInfo* pdi);
    void  DoToolTipHitProcessing(CPoint point);

    CWnd* GetWindowFromPoint(CPoint point);
    int   SpecialItemFromPoint(CPoint pnt);
    void  DrawInsert(int nIndex);
    void  DrawSingle(int nIndex);

    // Overrides
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) override;
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS) override;

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

    virtual void OnDragEnd(CPoint point) /* override */= 0;

    //{{AFX_MSG(CGrafixListBox)
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnDragItem(WPARAM wParam, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/* Some derived classes (e.g., CMarkListBox) derive from
    (previous) CGrafixListBox indirectly through another class
    (e.g., CTileBaseListBox), so allow that by letting derived
    class specify another base class */
template<typename BASE_WND, typename T>
class CGrafixListBoxData : public BASE_WND
{
    static_assert(std::is_same_v<XxxxID<T::PREFIX>, T>, "requires XxxxID");
public:
    CGrafixListBoxData() : m_pItemMap(NULL) {}

    const std::vector<T>* GetItemMap() { return m_pItemMap; }
    T GetCurMapItem() const
    {
        ASSERT(!IsMultiSelect());
        ASSERT(m_pItemMap);
        int nItem = GetCurSel();
        ASSERT(nItem >= 0);
        ASSERT(value_preserving_cast<size_t>(nItem) < m_pItemMap->size());
        return m_pItemMap->at(value_preserving_cast<size_t>(nItem));
    }
    void GetCurMappedItemList(std::vector<T>& pLst)
    {
        pLst.clear();
        ASSERT(IsMultiSelect());
        int nSels = GetSelCount();
        if (nSels == LB_ERR || nSels == 0)
            return;
        std::vector<int> pSelTbl(value_preserving_cast<size_t>(nSels));
        GetSelItems(nSels, pSelTbl.data());
        pLst.reserve(pSelTbl.size());
        for (size_t i = 0; i < pSelTbl.size(); i++)
            pLst.push_back(MapIndexToItem(value_preserving_cast<size_t>(pSelTbl[i])));
        return;
    }
    // Note: the following reference is only good during drag and drop.
    // the data is only good during the drop. It is essentially a
    // hack to have valid data when selecting items with Shift-Click.
    // Ask Microsoft why I had to do this. The number of selections
    // data in the case of a shift click isn't valid until the button
    // is released. Makes it tough to use a pre setup list during the
    // drag operation.
    const std::vector<T>& GetMappedMultiSelectList() { return m_multiSelList; }

    void SetItemMap(const std::vector<T>* pMap, BOOL bKeepPosition = TRUE)
    {
        m_pItemMap = pMap;
        UpdateList(bKeepPosition);
    }
    // bKeepPosition == TRUE means current selection is maintained.
    void UpdateList(BOOL bKeepPosition = TRUE)
    {
        if (m_pItemMap == NULL)
        {
            ResetContent();
            return;
        }

        SetRedraw(FALSE);
        int nCurSel = IsMultiSelect() ? -1 : GetCurSel();
        int nTopIdx = GetTopIndex();
        int nFcsIdx = GetCaretIndex();
        ResetContent();
        int nItem;
        for (nItem = 0; nItem < value_preserving_cast<int>(m_pItemMap->size()); nItem++)
            AddString(" ");             // Fill with dummy data
        if (bKeepPosition)
        {
            if (nTopIdx >= 0)
                SetTopIndex(CB::min(nTopIdx, nItem - 1));
            if (nFcsIdx >= 0)
                SetCaretIndex(CB::min(nFcsIdx, nItem - 1), FALSE);
            if (nCurSel >= 0)
                SetCurSel(CB::min(nCurSel, nItem - 1));
        }
        SetRedraw(TRUE);
        Invalidate();
    }
    void SetCurSelMapped(T nMapVal)
    {
        ASSERT(m_pItemMap);
        for (size_t i = 0; i < m_pItemMap->size(); i++)
        {
            if (m_pItemMap->at(i) == nMapVal)
            {
                SetCurSel(value_preserving_cast<int>(i));
                SetTopIndex(value_preserving_cast<int>(i));
            }
        }
    }
    void SetCurSelsMapped(const std::vector<T>& items)
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

    T MapIndexToItem(size_t nIndex)
    {
        ASSERT(m_pItemMap);
        ASSERT(nIndex < m_pItemMap->size());
        return m_pItemMap->at(nIndex);
    }
    size_t MapItemToIndex(T nItem)
    {
        ASSERT(m_pItemMap);
        for (size_t i = 0; i < m_pItemMap->size(); i++)
        {
            if (nItem == m_pItemMap->at(i))
                return i;
        }
        return Invalid_v<size_t>;                  // Failed to find it
    }

protected:
    virtual void OnDragEnd(CPoint point) override
    {
        if (di.m_dragType == DRAG_INVALID)
        {
            // do nothing
        }
        else if (IsMultiSelect())
        {
            // Get the final selection results after the mouse was released.
            GetCurMappedItemList(m_multiSelList);

            CWnd* pWnd = GetParent();
            ASSERT(pWnd != NULL);
            pWnd->SendMessage(WM_OVERRIDE_SELECTED_ITEM_LIST, reinterpret_cast<WPARAM>(&m_multiSelList), T::PREFIX);
        }
        else
        {
            // The parent may want to override the value.
            if (di.m_dragType == DRAG_MARKER)
            {
                COverrideInfo<DRAG_MARKER> oi(di.GetSubInfo<DRAG_MARKER>().m_markID);
                CWnd* pWnd = GetParent();
                ASSERT(pWnd != NULL);
                pWnd->SendMessage(WM_OVERRIDE_SELECTED_ITEM, reinterpret_cast<WPARAM>(&oi));
            }
            else if (di.m_dragType == DRAG_TILE)
            {
                COverrideInfo<DRAG_TILE> oi(di.GetSubInfo<DRAG_TILE>().m_tileID);
                CWnd* pWnd = GetParent();
                ASSERT(pWnd != NULL);
                pWnd->SendMessage(WM_OVERRIDE_SELECTED_ITEM, reinterpret_cast<WPARAM>(&oi));
            }
            else
            {
                ASSERT(!"unexpected dragType");
            }
        }

        ReleaseCapture();
        SetCursor(LoadCursor(NULL, IDC_ARROW));

        CWnd* pWnd = GetWindowFromPoint(point);
        if (pWnd == NULL || (!m_bAllowSelfDrop && pWnd == this))
        {
            OnDragCleanup(&di);         // Tell subclass we're all done.
            return;
        }
        di.m_point = point;
        di.m_pointClient = point;       // list box relative
        ClientToScreen(&di.m_point);
        pWnd->ScreenToClient(&di.m_point);

        pWnd->SendMessage(WM_DRAGDROP, phaseDragDrop,
            (LPARAM)(LPVOID)&di);
        OnDragCleanup(&di);         // Tell subclass we're all done.
        m_multiSelList.clear();
    }

    /* N.B.:  Only CTileBaseListBox requires providing this, but
        it doesn't hurt much to provide it in general.  */
    virtual int OnGetItemDebugIDCode(size_t nItem) override { return value_preserving_cast<int>(static_cast<WORD>(MapIndexToItem(nItem))); }

private:
    const std::vector<T>* m_pItemMap;         // Maps index to item
    std::vector<T> m_multiSelList;      // Holds mapped multi select items on drop
};

#endif

