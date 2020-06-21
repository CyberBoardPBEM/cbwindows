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
#define WM_OVERRIDE_SELECTED_ITEM       (WM_USER + 500) // WPARAM = int*
#define WM_OVERRIDE_SELECTED_ITEM_LIST  (WM_USER + 501) // WPARAM = CWordArray*

/////////////////////////////////////////////////////////////////////////////
// Custom Listbox - containing colors

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
    CWordArray* GetItemMap() { return m_pItemMap; }
    WORD GetCurMapItem();
    void GetCurMappedItemList(CWordArray* pLst);
    BOOL IsMultiSelect()
        { return (GetStyle() & (LBS_EXTENDEDSEL | LBS_MULTIPLESEL)) != 0; }
    // Note: the following pointer is only good during drag and drop.
    // the data is only good during the drop. It is essentially a
    // hack to have valid data when selecting items with Shift-Click.
    // Ask Microsoft why I had to do this. The number of selections
    // data in the case of a shift click isn't valid until the button
    // is released. Makes it tough to use a pre setup list during the
    // drag operation.
    CWordArray* GetMappedMultiSelectList() { return &m_multiSelList; }

// Operations
public:
    void SetItemMap(CWordArray* pMap, BOOL bKeepPosition = TRUE);
    void UpdateList(BOOL bKeepPosition = TRUE);
    void SetCurSelMapped(WORD nMapVal);
    void SetCurSelsMapped(CWordArray& items);
    void SetSelFromPoint(CPoint point);
    void ShowFirstSelection();
    int  MapIndexToItem(int nIndex);
    int  MapItemToIndex(int nItem);
    void MakeItemVisible(int nItem);

    // Notification Tooltip Support
    void SetNotificationTip(int nItem, UINT nResID);
    void SetNotificationTip(int nItem, LPCTSTR pszTip);
    void ClearNotificationTip();
    static void CALLBACK NotificationTipTimeoutHandler(HWND hwnd, UINT uMsg,
        UINT_PTR idEvent, DWORD dwTime);

// Overrides - the subclass of this class must override these
public:
    virtual int OnItemHeight(int nIndex) = 0;
    virtual void OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
        CRect rctItem) = 0;
    virtual BOOL OnDragSetup(DragInfo* pDI) { return FALSE; }
    virtual void OnDragCleanup(DragInfo* pDI) { }

    // For tool tip processing
    virtual BOOL OnIsToolTipsEnabled() { return FALSE; }
    virtual int  OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) { return -1; }
    virtual void OnGetTipTextForItemCode(int nItemCode, CString& strTip, CString& strTitle) { }

// Implementation
protected:
    CWordArray* m_pItemMap;         // Maps index to item
    CWordArray m_multiSelList;      // Holds mapped multi select items on drop

    // Tool tip support
    CToolTipCtrl m_toolMsgTip;      // Tooltip for notifications
    CToolTipCtrl m_toolTip;         // Tooltip of tile text popups
    int     m_nCurItemCode;         // current active tip item code

    // Drag and scroll support vars
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
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

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

#endif

