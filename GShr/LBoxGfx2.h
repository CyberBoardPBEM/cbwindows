// LBoxGfx2.h
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

#ifndef _LBOXGFX2_H
#define _LBOXGFX2_H

#include <queue>

#ifndef     _DRAGDROP_H
#include    "DragDrop.h"
#endif

#include    "MapStrng.h"
#include    "LBoxVHScrl.h"

class CDrawObj;

/////////////////////////////////////////////////////////////////////////////

#define ID_TIP_LISTITEM_HIT             1
#define MAX_LISTITEM_TIP_WIDTH          200     // Max pixel width of tips

/////////////////////////////////////////////////////////////////////////////
// Custom Listbox - containing colors

class CGrafixListBox2 : public CB::VListBoxHScroll
{
// Construction
public:
    CGrafixListBox2();

// Attributes
public:
    int  GetTopSelectedItem() const;
    void EnableDrag(BOOL bEnable = TRUE) { m_bAllowDrag = bEnable; }
    void EnableSelfDrop(BOOL bEnable = TRUE) { m_bAllowSelfDrop = bEnable; }
    void EnableDropScroll(BOOL bEnable = TRUE) { m_bAllowDropScroll = bEnable; }
    const std::vector<RefPtr<CDrawObj>>* GetItemMap() const { return m_pItemMap; }
    const CDrawObj& GetCurMapItem() const;
    std::vector<CB::not_null<const CDrawObj*>> GetCurMappedItemList() const;
    BOOL IsMultiSelect() const
        { return HasMultipleSelection(); }
    // Note: the following pointer is only good during drag and drop.
    // the data is only good during the drop. It is essentially a
    // hack to have valid data when selecting items with Shift-Click.
    // Ask Microsoft why I had to do this. The number of selections
    // data in the case of a shift click isn't valid until the button
    // is released. Makes it tough to use a pre setup list during the
    // drag operation.
    const std::vector<CB::not_null<const CDrawObj*>>& GetMappedMultiSelectList() const { return m_multiSelList; }

// Operations
public:
    void SetItemMap(const std::vector<RefPtr<CDrawObj>>* pMap, BOOL bKeepPosition = TRUE);
    void UpdateList(BOOL bKeepPosition = TRUE);
    void SetCurSelMapped(const CDrawObj& pMapVal);
    void SetCurSelsMapped(const std::vector<CB::not_null<CDrawObj*>>& items);
    void SetSelFromPoint(wxPoint point);
    void ShowFirstSelection();
    const CDrawObj& MapIndexToItem(size_t nIndex) const;
    CDrawObj& MapIndexToItem(size_t nIndex)
    {
        return const_cast<CDrawObj&>(std::as_const(*this).MapIndexToItem(nIndex));
    }
    size_t MapItemToIndex(const CDrawObj& pItem) const;
    void MakeItemVisible(int nItem);

// Overrides - the subclass of this class must override these
public:
#if 0
    virtual CSize OnItemSize(size_t nIndex) const /* override */ = 0;
    virtual void OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
        CRect rctItem) const /* override */ = 0;
#endif
    virtual BOOL OnDragSetup(DragInfoWx& pDI) const /* override */
    {
        pDI.SetDragType(DRAG_INVALID);
        return FALSE;
    }
    virtual void OnDragCleanup(const DragInfoWx& pDI) const /* override */ { }

    // For tool tip processing
    virtual BOOL OnIsToolTipsEnabled() const /* override */ { return FALSE; }
    virtual GameElement OnGetHitItemCodeAtPoint(wxPoint point, wxRect& rct) const /* override */ { return Invalid_v<GameElement>; }
    virtual CB::string OnGetTipTextForItemCode(GameElement nItemCode) const /* override */ { return CB::string(); }

// Implementation
protected:
    /* N.B.:  this class could be templatized to hold any pointer,
                but that generality isn't actually needed yet */
    const std::vector<RefPtr<CDrawObj>>* m_pItemMap;          // Maps index to item
    std::vector<CB::not_null<const CDrawObj*>> m_multiSelList;      // Holds mapped multi select items on drop

    // Tool tip support
    CB::ToolTip m_toolTip;         // Tooltip of tile text popups
    GameElement m_nCurItemCode = Invalid_v<GameElement>;
    wxRect      m_rectToolTip = wxRect();

    // Drag and scroll support vars
    BOOL    m_bAllowDrag;
    BOOL    m_bAllowSelfDrop;       // Only if m_bAllowDrag == TRUE
    BOOL    m_bAllowDropScroll;     // Scroll on OnDragItem

    wxPoint  m_clickPoint;
#if 0
    wxTimer m_nTimerID;
#endif
    BOOL    m_triggeredCursor;
    wxWindow* m_hLastWnd;

    int     m_nLastInsert;          // Last index with insert line

    void  SetDocument(CGamDoc& doc) { m_pDoc = &doc; }
    void  DoInsertLineProcessing(const DragInfoWx& pdi);
    void  DoAutoScrollProcessing(const DragInfoWx& pdi);
    void  DoToolTipHitProcessing(wxPoint point);

    wxWindow* GetWindowFromPoint(wxPoint point);
    int   SpecialItemFromPoint(wxPoint pnt) const;
    void  DrawInsert(int nIndex);
    void  DrawSingle(int nIndex);

    wxPoint ClientToItem(wxPoint point) const
    {
        return CalcUnscrolledPosition(point);
    }
    wxRect ItemToClient(wxRect rect) const
    {
        return wxRect(CalcScrolledPosition(rect.GetTopLeft()), rect.GetSize());
    }

    // Overrides
#if 0
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS) override;
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS) override;
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;
#endif

    void OnLButtonDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnLButtonUp(wxMouseEvent& event);
#if 0
    void OnTimer(wxTimerEvent& event);
#endif
    void OnCreate(wxWindowCreateEvent& event);
    void OnDragItem(DragDropEvent& event);
    wxDECLARE_EVENT_TABLE();

private:
    CB::propagate_const<CGamDoc*> m_pDoc = nullptr;

    /* wxWidgets does not support post-processing of events
        (see https://docs.wxwidgets.org/stable/overview_events.html#overview_events_virtual),
        so we need to use wxEvtHandler::CallAfter() to
        approximate post-processing. */
    void PushPostProcessEvent(std::function<void()>&& f);
    void ExecutePostProcessEvents();
    std::queue<std::function<void()>> postProcessEvents;
};
#endif

