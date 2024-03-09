// LBoxGrfx.h
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

#ifndef _LBOXGRFX_H
#define _LBOXGRFX_H

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

#include    "LBoxVHScrl.h"
#include    "MapStrng.h"

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

    void CheckType() const { CheckTypeBase<DRAG_MARKER>(); }

    MarkID& m_markID;
};

template<>
struct COverrideInfo<DRAG_TILE> : private COverrideInfoBase
{
    COverrideInfo(TileID& tid) : COverrideInfoBase(DRAG_TILE), m_tileID(tid) {}

    void CheckType() const { CheckTypeBase<DRAG_TILE>(); }

    TileID& m_tileID;
};

#define WM_OVERRIDE_SELECTED_ITEM_LIST  (WM_USER + 501) // WPARAM = std::vector<XxxxID<LPARAM>>*, LPARAM = XxxxID::PREFIX
/* We don't allow drop if the dropped objects are too big for
    the destination, but checking requires knowing the size of
    the dropped objects.  So, parallel to
    WM_OVERRIDE_SELECTED_ITEM, this message gets that size from
    parent window. */
#define WM_GET_DRAG_SIZE (WM_USER + 504) // WPARAM = CSize*

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
    CB::string GetText(int nIndex) const;
    int  GetTopSelectedItem() const;
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
    void SetNotificationTip(int nItem, const CB::string& pszTip);
    void ClearNotificationTip();
    static void CALLBACK NotificationTipTimeoutHandler(HWND hwnd, UINT uMsg,
        UINT_PTR idEvent, DWORD dwTime);

// Overrides - the subclass of this class must override these
public:
    virtual CSize OnItemSize(size_t nIndex) const /* override */ = 0;
    virtual void OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
        CRect rctItem) const /* override */ = 0;
    virtual BOOL OnDragSetup(DragInfo& pDI) const /* override */
    {
        pDI.SetDragType(DRAG_INVALID);
        return FALSE;
    }
    virtual void OnDragCleanup(const DragInfo& pDI) const /* override */ { }

    // For tool tip processing
    virtual BOOL OnIsToolTipsEnabled() const /* override */ { return FALSE; }
    virtual GameElement OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) const /* override */ { return Invalid_v<GameElement>; }
    virtual void OnGetTipTextForItemCode(GameElement nItemCode, CB::string& strTip) const /* override */ { }

    /* N.B.:  Conceptually, this declaration belongs to
        CTileBaseListBox, but it doesn't hurt much to declare it
        in general, and doing it here allows override checks of
        CGrafixListBoxData<>::OnGetItemDebugIDCode. */
    virtual int OnGetItemDebugIDCode(size_t nItem) const /* override */ = 0;

// Implementation
protected:
    // Tool tip support
    CToolTipCtrl m_toolMsgTip;      // Tooltip for notifications
    CToolTipCtrl m_toolTip;         // Tooltip of tile text popups
    GameElement m_nCurItemCode;         // current active tip item code

    // Drag and scroll support vars
    static DragInfo di;
    BOOL    m_bAllowDrag;
    BOOL    m_bAllowSelfDrop;       // Only if m_bAllowDrag == TRUE
    BOOL    m_bAllowDropScroll;     // Scroll on OnDragItem

    CPoint  m_clickPoint;
    uintptr_t m_nTimerID;
    BOOL    m_triggeredCursor;
    HWND    m_hLastWnd;

    int     m_nLastInsert;          // Last index with insert line

    void  DoInsertLineProcessing(const DragInfo& pdi);
    void  DoAutoScrollProcessing(const DragInfo& pdi);
    void  DoToolTipHitProcessing(CPoint point);

    CWnd* GetWindowFromPoint(CPoint point);
    int   SpecialItemFromPoint(CPoint pnt) const;
    void  DrawInsert(int nIndex);
    void  DrawSingle(int nIndex);

    CPoint ClientToItem(CPoint point) const;
    CRect ItemToClient(CRect rect) const;

    // Overrides
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) override;
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS) override;

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

    // send WM_GET_DRAG_SIZE to parent
    CSize GetDragSize() const;
    virtual void OnDragEnd(CPoint point) /* override */= 0;

    //{{AFX_MSG(CGrafixListBox)
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(uintptr_t nIDEvent);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnDragItem(WPARAM wParam, LPARAM lParam);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

protected:
    // #include "GamDoc.h" causes problems
    void AssignNewMoveGroup();
    void SetDocument(CGamDoc& doc) { m_pDoc = &doc; }

private:
    CB::propagate_const<CGamDoc*> m_pDoc = nullptr;
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

    const std::vector<T>* GetItemMap() const { return m_pItemMap; }
    T GetCurMapItem() const
    {
        ASSERT(!this->IsMultiSelect());
        ASSERT(m_pItemMap);
        int nItem = this->GetCurSel();
        ASSERT(nItem >= 0);
        ASSERT(value_preserving_cast<size_t>(nItem) < m_pItemMap->size());
        return m_pItemMap->at(value_preserving_cast<size_t>(nItem));
    }
    std::vector<T> GetCurMappedItemList() const
    {
        std::vector<T> pLst;
        ASSERT(this->IsMultiSelect());
        int nSels = this->GetSelCount();
        if (nSels == LB_ERR || nSels == 0)
            return pLst;
        std::vector<int> pSelTbl(value_preserving_cast<size_t>(nSels));
        this->GetSelItems(nSels, pSelTbl.data());
        pLst.reserve(pSelTbl.size());
        for (size_t i = 0; i < pSelTbl.size(); i++)
            pLst.push_back(MapIndexToItem(value_preserving_cast<size_t>(pSelTbl[i])));
        return pLst;
    }
    // Note: the following reference is only good during drag and drop.
    // the data is only good during the drop. It is essentially a
    // hack to have valid data when selecting items with Shift-Click.
    // Ask Microsoft why I had to do this. The number of selections
    // data in the case of a shift click isn't valid until the button
    // is released. Makes it tough to use a pre setup list during the
    // drag operation.
    const std::vector<T>& GetMappedMultiSelectList() const { return m_multiSelList; }

    void SetItemMap(const std::vector<T>* pMap, BOOL bKeepPosition = TRUE)
    {
        ASSERT(!pMap || pMap->size() < size_t(0x10000) || !"LB_ITEMFROMPOINT/ItemFromPoint() is WORD-limited");
        m_pItemMap = pMap;
        UpdateList(bKeepPosition);
    }
    // bKeepPosition == TRUE means current selection is maintained.
    void UpdateList(BOOL bKeepPosition = TRUE)
    {
        if (m_pItemMap == NULL)
        {
            this->ResetContent();
            return;
        }

        this->SetRedraw(FALSE);
        int nCurSel = this->IsMultiSelect() ? -1 : this->GetCurSel();
        int nTopIdx = this->GetTopIndex();
        int nFcsIdx = this->GetCaretIndex();
        int horzScroll = this->GetScrollPos(SB_HORZ);
        this->ResetContent();
        LONG width = 0;
        int nItem;
        for (nItem = 0; nItem < value_preserving_cast<int>(m_pItemMap->size()); nItem++)
        {
            this->AddString(" "_cbstring);             // Fill with dummy data
            width = CB::max(width, this->OnItemSize(value_preserving_cast<size_t>(nItem)).cx);
        }
        this->SetHorizontalExtent(value_preserving_cast<int>(width));
        if (bKeepPosition)
        {
            if (nTopIdx >= 0)
                this->SetTopIndex(CB::min(nTopIdx, nItem - 1));
            if (nFcsIdx >= 0)
                this->SetCaretIndex(CB::min(nFcsIdx, nItem - 1), FALSE);
            if (nCurSel >= 0)
                this->SetCurSel(CB::min(nCurSel, nItem - 1));
        }
        this->SetRedraw(TRUE);
        if (bKeepPosition)
        {
            this->SendMessage(WM_HSCROLL, MAKELONG(int16_t(SB_THUMBPOSITION), value_preserving_cast<int16_t>(horzScroll)), NULL);
        }
        this->Invalidate();
    }
    void SetCurSelMapped(T nMapVal)
    {
        ASSERT(m_pItemMap);
        for (size_t i = 0; i < m_pItemMap->size(); i++)
        {
            if (m_pItemMap->at(i) == nMapVal)
            {
                this->SetCurSel(value_preserving_cast<int>(i));
                this->SetTopIndex(value_preserving_cast<int>(i));
            }
        }
    }
    void SetCurSelsMapped(const std::vector<T>& items)
    {
        ASSERT(m_pItemMap);
        ASSERT(this->IsMultiSelect());

        this->SetSel(-1, FALSE);      // Deselect all
        for (size_t i = size_t(0); i < items.size(); i++)
        {
            for (size_t j = 0; j < m_pItemMap->size(); j++)
            {
                if (m_pItemMap->at(j) == items[i])
                    this->SetSel(value_preserving_cast<int>(j));
            }
        }
    }

    T MapIndexToItem(size_t nIndex) const
    {
        ASSERT(m_pItemMap);
        ASSERT(nIndex < m_pItemMap->size());
        return m_pItemMap->at(nIndex);
    }
    size_t MapItemToIndex(T nItem) const
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
        this->AssignNewMoveGroup();
        if (BASE_WND::di.GetDragType() == DRAG_INVALID)
        {
            // do nothing
        }
        else if (this->IsMultiSelect())
        {
            // Get the final selection results after the mouse was released.
            m_multiSelList = GetCurMappedItemList();

            CWnd* pWnd = this->GetParent();
            ASSERT(pWnd != NULL);
            pWnd->SendMessage(WM_OVERRIDE_SELECTED_ITEM_LIST, reinterpret_cast<WPARAM>(&m_multiSelList), T::PREFIX);
        }
        else
        {
            // The parent may want to override the value.
            if (BASE_WND::di.GetDragType() == DRAG_MARKER)
            {
                COverrideInfo<DRAG_MARKER> oi(BASE_WND::di.GetSubInfo<DRAG_MARKER>().m_markID);
                CWnd* pWnd = this->GetParent();
                ASSERT(pWnd != NULL);
                pWnd->SendMessage(WM_OVERRIDE_SELECTED_ITEM, reinterpret_cast<WPARAM>(&oi));
            }
            else if (BASE_WND::di.GetDragType() == DRAG_TILE)
            {
                COverrideInfo<DRAG_TILE> oi(BASE_WND::di.GetSubInfo<DRAG_TILE>().m_tileID);
                CWnd* pWnd = this->GetParent();
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

        CWnd* pWnd = this->GetWindowFromPoint(point);
        if (pWnd == NULL || (!this->m_bAllowSelfDrop && pWnd == this))
        {
            this->OnDragCleanup(BASE_WND::di);         // Tell subclass we're all done.
            return;
        }
        BASE_WND::di.m_point = point;
        BASE_WND::di.m_pointClient = point;       // list box relative
        this->ClientToScreen(&BASE_WND::di.m_point);
        pWnd->ScreenToClient(&BASE_WND::di.m_point);

        BASE_WND::di.m_phase = PhaseDrag::Drop;
        pWnd->SendMessage(WM_DRAGDROP, GetProcessId(GetCurrentProcess()),
            (LPARAM)(LPVOID)&BASE_WND::di);
        this->OnDragCleanup(BASE_WND::di);         // Tell subclass we're all done.
        m_multiSelList.clear();
    }

    /* N.B.:  Only CTileBaseListBox requires providing this, but
        it doesn't hurt much to provide it in general.  */
    virtual int OnGetItemDebugIDCode(size_t nItem) const override { return value_preserving_cast<int>(static_cast<T::UNDERLYING_TYPE>(MapIndexToItem(nItem))); }

private:
    const std::vector<T>* m_pItemMap;         // Maps index to item
    std::vector<T> m_multiSelList;      // Holds mapped multi select items on drop
};

/* I want CGrafixListBox::m_pItemMap to be type-specific.
    However, I don't want to replicate the message map for each
    different type of CGrafixListBox::m_pItemMap.  Therefore,
    split CGrafixListBox into a base class that contains the
    message map, and a templatized derived class that contains
    the type-specific CGrafixListBox::m_pItemMap. */

class CGrafixListBoxWx : public CB::VListBoxHScroll
{
    // Construction
public:
    CGrafixListBoxWx();
    ~CGrafixListBoxWx() override;

    // Attributes
public:
#if 0
    CB::string GetText(int nIndex) const;
    int  GetTopSelectedItem() const;
    void EnableDrag(BOOL bEnable = TRUE) { m_bAllowDrag = bEnable; }
    void EnableSelfDrop(BOOL bEnable = TRUE) { m_bAllowSelfDrop = bEnable; }
    void EnableDropScroll(BOOL bEnable = TRUE) { m_bAllowDropScroll = bEnable; }
#endif
    BOOL IsMultiSelect() const
        { return HasMultipleSelection(); }

    // Operations
public:
#if 0
    void SetSelFromPoint(wxPoint point);
    void ShowFirstSelection();
    void MakeItemVisible(int nItem);

    // Notification Tooltip Support
    void SetNotificationTip(int nItem, UINT nResID);
    void SetNotificationTip(int nItem, const CB::string& pszTip);
    void ClearNotificationTip();
    static void CALLBACK NotificationTipTimeoutHandler(HWND hwnd, UINT uMsg,
        UINT_PTR idEvent, DWORD dwTime);
#endif

    // Overrides - the subclass of this class must override these
public:
#if 0
    virtual wxSize OnItemSize(size_t nIndex) const /* override */ = 0;
    virtual void OnItemDraw(wxDC& pDC, size_t nIndex, UINT nAction, UINT nState,
        wxRect rctItem) const /* override */ = 0;
    virtual BOOL OnDragSetup(DragInfo& pDI) const /* override */
    {
        pDI.SetDragType(DRAG_INVALID);
        return FALSE;
    }
    virtual void OnDragCleanup(const DragInfo& pDI) const /* override */ { }
#endif

    // For tool tip processing
    virtual BOOL OnIsToolTipsEnabled() const /* override */ { return FALSE; }
    virtual GameElement OnGetHitItemCodeAtPoint(wxPoint point, wxRect& rct) const /* override */ { return Invalid_v<GameElement>; }
    virtual void OnGetTipTextForItemCode(GameElement nItemCode, CB::string& strTip) const /* override */ { }

    /* N.B.:  Conceptually, this declaration belongs to
        CTileBaseListBox, but it doesn't hurt much to declare it
        in general, and doing it here allows override checks of
        CGrafixListBoxData<>::OnGetItemDebugIDCode. */
    virtual int OnGetItemDebugIDCode(size_t nItem) const /* override */ = 0;

    // Implementation
protected:
    // Tool tip support
#if 0
    CToolTipCtrl m_toolMsgTip;      // Tooltip for notifications
#endif
    wxTipWindow* m_toolTip = nullptr;         // Tooltip of tile text popups
    GameElement m_nCurItemCode;         // current active tip item code

#if 0
    // Drag and scroll support vars
    static DragInfo di;
    BOOL    m_bAllowDrag;
    BOOL    m_bAllowSelfDrop;       // Only if m_bAllowDrag == TRUE
    BOOL    m_bAllowDropScroll;     // Scroll on OnDragItem

    wxPoint m_clickPoint;
    uintptr_t m_nTimerID;
    BOOL    m_triggeredCursor;
    wxWindow* m_hLastWnd;

    int     m_nLastInsert;          // Last index with insert line

    void  DoInsertLineProcessing(const DragInfo& pdi);
    void  DoAutoScrollProcessing(const DragInfo& pdi);
#endif
    void  DoToolTipHitProcessing(wxPoint point);

#if 0
    wxWindow* GetWindowFromPoint(wxPoint point);
    int   SpecialItemFromPoint(wxPoint pnt) const;
    void  DrawInsert(int nIndex);
    void  DrawSingle(int nIndex);
#endif

    wxPoint ClientToItem(wxPoint point) const
    {
        return CalcUnscrolledPosition(point);
    }
    wxRect ItemToClient(wxRect rect) const
    {
        return wxRect(CalcScrolledPosition(rect.GetTopLeft()), rect.GetSize());
    }

#if 0
    // Overrides
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) override;
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS) override;

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

    // send WM_GET_DRAG_SIZE to parent
    wxSize GetDragSize() const;
    virtual void OnDragEnd(wxPoint point) /* override */ = 0;
#endif

    void OnLButtonDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnLButtonUp(wxMouseEvent& event);
    void OnTimer(wxTimerEvent& event);
#if 0
    int OnCreate(LPCREATESTRUCT lpCreateStruct);
    LRESULT OnDragItem(WPARAM wParam, LPARAM lParam);
#endif
    wxDECLARE_EVENT_TABLE();

protected:
#if 0
    // #include "GamDoc.h" causes problems
    void AssignNewMoveGroup();
#endif
    void SetDocument(CGamDoc& doc) { m_pDoc = &doc; }

private:
    CB::propagate_const<CGamDoc*> m_pDoc = nullptr;
};

/* Some derived classes (e.g., CMarkListBox) derive from
    (previous) CGrafixListBox indirectly through another class
    (e.g., CTileBaseListBox), so allow that by letting derived
    class specify another base class */
template<typename BASE_WND, typename T>
class CGrafixListBoxDataWx : public BASE_WND
{
    static_assert(std::is_same_v<XxxxID<T::PREFIX>, T>, "requires XxxxID");
public:
    CGrafixListBoxDataWx() : m_pItemMap(NULL) {}

    const std::vector<T>* GetItemMap() const { return m_pItemMap; }
    T GetCurMapItem() const
    {
        ASSERT(!this->IsMultiSelect());
        ASSERT(m_pItemMap);
        int nItem = this->GetSelection();
        wxASSERT(nItem != wxNOT_FOUND);
        wxASSERT(value_preserving_cast<size_t>(nItem) < m_pItemMap->size());
        return m_pItemMap->at(value_preserving_cast<size_t>(nItem));
    }
#if 0
    std::vector<T> GetCurMappedItemList() const
    {
        std::vector<T> pLst;
        ASSERT(this->IsMultiSelect());
        int nSels = this->GetSelCount();
        if (nSels == LB_ERR || nSels == 0)
            return pLst;
        std::vector<int> pSelTbl(value_preserving_cast<size_t>(nSels));
        this->GetSelItems(nSels, pSelTbl.data());
        pLst.reserve(pSelTbl.size());
        for (size_t i = size_t(0); i < pSelTbl.size(); i++)
            pLst.push_back(MapIndexToItem(value_preserving_cast<size_t>(pSelTbl[i])));
        return pLst;
    }
    // Note: the following reference is only good during drag and drop.
    // the data is only good during the drop. It is essentially a
    // hack to have valid data when selecting items with Shift-Click.
    // Ask Microsoft why I had to do this. The number of selections
    // data in the case of a shift click isn't valid until the button
    // is released. Makes it tough to use a pre setup list during the
    // drag operation.
    const std::vector<T>& GetMappedMultiSelectList() const { return m_multiSelList; }
#endif

    void SetItemMap(const std::vector<T>* pMap, BOOL bKeepPosition = TRUE)
    {
        ASSERT(!pMap || pMap->size() < size_t(0x10000) || !"LB_ITEMFROMPOINT/ItemFromPoint() is WORD-limited");
        m_pItemMap = pMap;
        UpdateList(bKeepPosition);
    }
    // bKeepPosition == TRUE means current selection is maintained.
    void UpdateList(BOOL bKeepPosition = TRUE)
    {
        if (m_pItemMap == NULL)
        {
            this->Clear();
            return;
        }

        int nCurSel = this->IsMultiSelect() ? wxNOT_FOUND : this->GetSelection();
        size_t nTopIdx = this->GetVisibleRowsBegin();
        int nFcsIdx = this->GetCurrent();
        int horzScroll = this->GetScrollPos(wxHORIZONTAL);
    {
        wxWindowUpdateLocker freezer(this);
        this->Clear();
        size_t nItem = CB::max(size_t(1), m_pItemMap->size());
        this->SetItemCount(m_pItemMap->size());
        if (bKeepPosition)
        {
            this->ScrollToRow(CB::min(nTopIdx, nItem - size_t(1)));
            if (nFcsIdx != wxNOT_FOUND)
                this->SetCurrent(CB::min(nFcsIdx, value_preserving_cast<int>(nItem - size_t(1))));
            if (nCurSel != wxNOT_FOUND)
                this->SetSelection(CB::min(nCurSel, value_preserving_cast<int>(nItem - size_t(1))));
        }
    }
        if (bKeepPosition)
        {
            this->SetScrollPos(wxHORIZONTAL, horzScroll);
        }
        this->Refresh();
    }
    void SetCurSelMapped(T nMapVal)
    {
        ASSERT(m_pItemMap);
        for (size_t i = size_t(0); i < m_pItemMap->size(); i++)
        {
            if (m_pItemMap->at(i) == nMapVal)
            {
                this->SetSelection(value_preserving_cast<int>(i));
                this->ScrollToRow(i);
            }
        }
    }
#if 0
    void SetCurSelsMapped(const std::vector<T>& items)
    {
        ASSERT(m_pItemMap);
        ASSERT(this->IsMultiSelect());

        this->SetSel(-1, FALSE);      // Deselect all
        for (size_t i = size_t(0); i < items.size(); i++)
        {
            for (size_t j = size_t(0); j < m_pItemMap->size(); j++)
            {
                if (m_pItemMap->at(j) == items[i])
                    this->SetSel(value_preserving_cast<int>(j));
            }
        }
    }
#endif

    T MapIndexToItem(size_t nIndex) const
    {
        ASSERT(m_pItemMap);
        ASSERT(nIndex < m_pItemMap->size());
        return m_pItemMap->at(nIndex);
    }
    size_t MapItemToIndex(T nItem) const
    {
        ASSERT(m_pItemMap);
        for (size_t i = size_t(0); i < m_pItemMap->size(); i++)
        {
            if (nItem == m_pItemMap->at(i))
                return i;
        }
        return Invalid_v<size_t>;                  // Failed to find it
    }

protected:
#if 0
    virtual void OnDragEnd(CPoint point) override
    {
        this->AssignNewMoveGroup();
        if (BASE_WND::di.GetDragType() == DRAG_INVALID)
        {
            // do nothing
        }
        else if (this->IsMultiSelect())
        {
            // Get the final selection results after the mouse was released.
            m_multiSelList = GetCurMappedItemList();

            CWnd* pWnd = this->GetParent();
            ASSERT(pWnd != NULL);
            pWnd->SendMessage(WM_OVERRIDE_SELECTED_ITEM_LIST, reinterpret_cast<WPARAM>(&m_multiSelList), T::PREFIX);
        }
        else
        {
            // The parent may want to override the value.
            if (BASE_WND::di.GetDragType() == DRAG_MARKER)
            {
                COverrideInfo<DRAG_MARKER> oi(BASE_WND::di.GetSubInfo<DRAG_MARKER>().m_markID);
                CWnd* pWnd = this->GetParent();
                ASSERT(pWnd != NULL);
                pWnd->SendMessage(WM_OVERRIDE_SELECTED_ITEM, reinterpret_cast<WPARAM>(&oi));
            }
            else if (BASE_WND::di.GetDragType() == DRAG_TILE)
            {
                COverrideInfo<DRAG_TILE> oi(BASE_WND::di.GetSubInfo<DRAG_TILE>().m_tileID);
                CWnd* pWnd = this->GetParent();
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

        CWnd* pWnd = this->GetWindowFromPoint(point);
        if (pWnd == NULL || (!this->m_bAllowSelfDrop && pWnd == this))
        {
            this->OnDragCleanup(BASE_WND::di);         // Tell subclass we're all done.
            return;
        }
        BASE_WND::di.m_point = point;
        BASE_WND::di.m_pointClient = point;       // list box relative
        this->ClientToScreen(&BASE_WND::di.m_point);
        pWnd->ScreenToClient(&BASE_WND::di.m_point);

        BASE_WND::di.m_phase = PhaseDrag::Drop;
        pWnd->SendMessage(WM_DRAGDROP, GetProcessId(GetCurrentProcess()),
            (LPARAM)(LPVOID)&BASE_WND::di);
        this->OnDragCleanup(BASE_WND::di);         // Tell subclass we're all done.
        m_multiSelList.clear();
    }
#endif

    /* N.B.:  Only CTileBaseListBox requires providing this, but
        it doesn't hurt much to provide it in general.  */
    virtual int OnGetItemDebugIDCode(size_t nItem) const override { return value_preserving_cast<int>(static_cast<T::UNDERLYING_TYPE>(MapIndexToItem(nItem))); }

private:
    const std::vector<T>* m_pItemMap;         // Maps index to item
    std::vector<T> m_multiSelList;      // Holds mapped multi select items on drop
};

#endif

