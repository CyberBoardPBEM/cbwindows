// VwSelpce.cpp : Selected Piece List View
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
#include    "Board.h"
#include    "PBoard.h"
#include    "SelOPlay.h"
#include    "VwSelpce.h"
#include    "FrmPbrd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CSelectedPieceView, CView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSelectedPieceView, CView)
    //{{AFX_MSG_MAP(CSelectedPieceView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEACTIVATE()
    ON_WM_VKEYTOITEM()
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_WINSTATE, OnMessageWindowState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectedPieceView

CSelectedPieceView::CSelectedPieceView()
{
    m_pPBoard = NULL;
}

CSelectedPieceView::~CSelectedPieceView()
{
}

/////////////////////////////////////////////////////////////////////////////

int CSelectedPieceView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_listSel.Create(
        WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |
            LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT |
            LBS_EXTENDEDSEL | LBS_WANTKEYBOARDINPUT,
        CRect(0, 0, 0, 0), this, (UINT)-1))
    {
        TRACE("Failed to create selection listbox.\n");
        return -1;
    }
    m_toolTip.Create(this, TTS_NOPREFIX);

    CString str;
    str.LoadString(IDS_TIP_SELLIST_HELP);

    TOOLINFO ti;
    m_toolTip.FillInToolInfo(ti, &m_listSel, 0);
    ti.lpszText = (LPSTR)(LPCSTR)str;
    ti.uFlags |= TTF_CENTERTIP;
    m_toolTip.SendMessage(TTM_ADDTOOL, (WPARAM)0, (LPARAM)&ti);

    m_toolTip.Activate(TRUE);

    return 0;
}

void CSelectedPieceView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);
    m_listSel.MoveWindow(-1, -1, cx + 1, cy + 1, TRUE);
    m_toolTip.SetMaxTipWidth(4 * cx);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CSelectedPieceView::PreTranslateMessage(MSG* pMsg)
{
    // RelayEvent is required for CToolTipCtrl objects -
    // it passes mouse messages on to the tool tip control
    // so it can decide when to show the tool tip
    m_toolTip.RelayEvent(pMsg);

    return CView::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////

void CSelectedPieceView::OnInitialUpdate()
{
    m_pPBoard = (CPlayBoard*)GetDocument()->GetNewViewParameter();
    CView::OnInitialUpdate();
    m_listSel.SetDocument(GetDocument());
}

void CSelectedPieceView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CGamDocHint* ph = (CGamDocHint*)pHint;
    if (lHint == HINT_UPDATESELECT && ph->m_pPBoard == m_pPBoard)
    {
        ASSERT(ph->m_pSelList != NULL);
        CSelList* pSLst = ph->m_pSelList;

        if (!pSLst->HasPieces() && !pSLst->HasMarkers())
        {
            m_listSel.SetItemMap(NULL);
            m_tblSel.clear();
            return;
        }
        pSLst->LoadTableWithObjectPtrs(m_tblSel);

        m_listSel.SetItemMap(&m_tblSel);
    }
    else if (lHint == HINT_GAMESTATEUSED)
    {
        m_listSel.SetItemMap(NULL);
        m_tblSel.clear();
        return;
    }
}

///////////////////////////////////////////////////////////////////////
// This message is sent when a document is being saved.
// WPARAM = CArchive*, LPARAM = 0 if save, 1 if restore

LRESULT CSelectedPieceView::OnMessageWindowState(WPARAM wParam, LPARAM lParam)
{
    return (LRESULT)0;
}

/////////////////////////////////////////////////////////////////////////////
// CSelectedPieceView drawing

void CSelectedPieceView::OnDraw(CDC* pDC)
{
    // Eat this since view is filled with a list box.
}

BOOL CSelectedPieceView::OnEraseBkgnd(CDC* pDC)
{
    // Eat this since view is filled with a list box.
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
CGamDoc* CSelectedPieceView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGamDoc)));
    return (CGamDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSelectedPieceView message handlers

int CSelectedPieceView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    // We don't want the frame to ever consider this view to be the
    // "active" view.
    return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

int CSelectedPieceView::OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex)
{
    if (nKey == VK_DELETE)
    {
        ModifySelectionsBasedOnListItems(TRUE);
        return -2;
    }
    else if (nKey == VK_INSERT)
    {
        ModifySelectionsBasedOnListItems(FALSE);
        return -2;
    }
    else
        return CView::OnVKeyToItem(nKey, pListBox, nIndex);
}

// Either removes or keeps items selected in listbox.
void CSelectedPieceView::ModifySelectionsBasedOnListItems(BOOL bRemoveSelectedItems)
{
    // Get the indexes of all the selected items.
    int nCount = m_listSel.GetSelCount();
    CArray<int, int> tblListBoxSel;

    tblListBoxSel.SetSize(nCount);
    m_listSel.GetSelItems(nCount, tblListBoxSel.GetData());

    // Create a list containing all items that are *not* selected.
    // Rather than try to be real clever I'll just brute force the search.
    CPtrList listDObj;
    for (int nItem = 0; nItem < m_listSel.GetCount(); nItem++)
    {
        // Loop and see if item is in selected list.
        int nSelItem;
        for (nSelItem = 0; nSelItem < tblListBoxSel.GetSize(); nSelItem++)
        {
            if (tblListBoxSel.GetAt(nSelItem) == nItem)
                break;
        }
        if (bRemoveSelectedItems && nSelItem == tblListBoxSel.GetSize())
        {
            // Not a listbox selection so add it to new select list.
            listDObj.AddTail(&m_listSel.MapIndexToItem(value_preserving_cast<size_t>(nItem)));
        }
        else if (!bRemoveSelectedItems && nSelItem < tblListBoxSel.GetSize())
        {
            // It is a listbox selection so add it to new select list.
            listDObj.AddTail(&m_listSel.MapIndexToItem(value_preserving_cast<size_t>(nItem)));
        }
    }
    CPlayBoardFrame* pFrame = (CPlayBoardFrame*)GetParentFrame();
    pFrame->SendMessageToActiveBoardPane(WM_SELECT_BOARD_OBJLIST, (WPARAM)m_pPBoard,
        (LPARAM)&listDObj);
}

