// VwSelpce.cpp : Selected Piece List View
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

// KLUDGE:  compile fails for base CSelectedPieceViewContainer::BASE
IMPLEMENT_DYNCREATE(CSelectedPieceViewContainer, CView)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

wxBEGIN_EVENT_TABLE(CSelectedPieceView, CSelectedPieceView::BASE)
#if 0
    ON_WM_CREATE()
#endif
    EVT_SIZE(OnSize)
#if 0
    ON_WM_MOUSEACTIVATE()
#endif
    /* see ctor
    ON_WM_VKEYTOITEM()
    */
    EVT_WINSTATE(OnMessageWindowState)
wxEND_EVENT_TABLE()

BEGIN_MESSAGE_MAP(CSelectedPieceViewContainer, CSelectedPieceViewContainer::BASE)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_MOUSEACTIVATE()
    ON_MESSAGE(WM_WINSTATE, OnMessageWindowState)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectedPieceView

CSelectedPieceView::CSelectedPieceView(CSelectedPieceViewContainer& p) :
    parent(&p),
    document(dynamic_cast<CGamDoc*>(parent->GetDocument())),
    m_pPBoard(static_cast<CPlayBoard*>(document->GetNewViewParameter())),
    m_listSel(new CSelectListBox)
{
    BASE::Create(*parent, 0);
    m_listSel->Create(this, wxID_ANY,
                        wxDefaultPosition, wxDefaultSize,
                        wxLB_MULTIPLE);
    /* wx doesn't support WM_VKEYTOITEM,
        and wxEVT_CHAR doesn't propagate to parent */
    m_listSel->Bind(wxEVT_CHAR, &CSelectedPieceView::OnVKeyToItem, this);

    CB::string str = CB::string::LoadString(IDS_TIP_SELLIST_HELP);
    m_toolTip.Add(*m_listSel, str, CB::ToolTip::CENTER);

    m_toolTip.Enable(TRUE);

    OnInitialUpdate();
}

CSelectedPieceView::~CSelectedPieceView()
{
}

/////////////////////////////////////////////////////////////////////////////

#if 0
int CSelectedPieceView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_listSel.Create(
        WS_CHILD | WS_BORDER | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
            LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT |
            LBS_EXTENDEDSEL | LBS_WANTKEYBOARDINPUT,
        CRect(0, 0, 0, 0), this, (UINT)-1))
    {
        TRACE("Failed to create selection listbox.\n");
        return -1;
    }
    m_toolTip.Create(this, TTS_NOPREFIX);

    CB::string str = CB::string::LoadString(IDS_TIP_SELLIST_HELP);

    TOOLINFO ti;
    m_toolTip.FillInToolInfo(ti, &m_listSel, 0);
    ti.lpszText = const_cast<CB::string::value_type*>(str.v_str());
    ti.uFlags |= TTF_CENTERTIP;
    m_toolTip.SendMessage(TTM_ADDTOOL, (WPARAM)0, (LPARAM)&ti);

    m_toolTip.Activate(TRUE);

    return 0;
}
#endif

void CSelectedPieceView::OnSize(wxSizeEvent& event)
{
    event.Skip();
    m_listSel->SetSize(0, 0, event.GetSize().x, event.GetSize().y);
    m_toolTip.SetMaxWidth(4 * event.GetSize().x);
}

/////////////////////////////////////////////////////////////////////////////

void CSelectedPieceView::OnInitialUpdate()
{
    parent->OnInitialUpdate();
    m_listSel->SetDocument(GetDocument());
}

void CSelectedPieceView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CGamDocHint* ph = (CGamDocHint*)pHint;
    if (lHint == HINT_UPDATESELECT && ph->GetArgs<HINT_UPDATESELECT>().m_pPBoard == m_pPBoard)
    {
        wxASSERT(ph->GetArgs<HINT_UPDATESELECT>().m_pSelList != NULL);
        CSelList* pSLst = ph->GetArgs<HINT_UPDATESELECT>().m_pSelList;

        if (!pSLst->HasPieces() && !pSLst->HasMarkers())
        {
            m_listSel->SetItemMap(NULL);
            m_tblSel.clear();
            return;
        }
        pSLst->LoadTableWithObjectPtrs(m_tblSel, CSelList::otPiecesMarks, TRUE);

        m_listSel->SetItemMap(&m_tblSel);
    }
    else if (lHint == HINT_GAMESTATEUSED)
    {
        m_listSel->SetItemMap(NULL);
        m_tblSel.clear();
        return;
    }
    else if (lHint == HINT_UPDATEOBJECT && ph->GetArgs<HINT_UPDATEOBJECT>().m_pPBoard == m_pPBoard)
    {
        Refresh();
    }
}

///////////////////////////////////////////////////////////////////////
// This message is sent when a document is being saved.
// WPARAM = CArchive*, LPARAM = 0 if save, 1 if restore

void CSelectedPieceView::OnMessageWindowState(WinStateEvent& /*event*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CSelectedPieceView drawing

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
CGamDoc& CSelectedPieceView::GetDocument() // non-debug version is inline
{
    return *document;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSelectedPieceView message handlers

#if 0
int CSelectedPieceView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    // We don't want the frame to ever consider this view to be the
    // "active" view.
    return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}
#endif

void CSelectedPieceView::OnVKeyToItem(wxKeyEvent& event)
{
    int nKey = event.GetKeyCode();
    if (nKey == WXK_DELETE)
    {
        ModifySelectionsBasedOnListItems(TRUE);
    }
    else if (nKey == WXK_INSERT)
    {
        ModifySelectionsBasedOnListItems(FALSE);
    }
    else
    {
        event.Skip();
    }
}

// Either removes or keeps items selected in listbox.
void CSelectedPieceView::ModifySelectionsBasedOnListItems(BOOL bRemoveSelectedItems)
{
    // Get the indexes of all the selected items.
    std::vector<size_t> tblListBoxSel = m_listSel->GetSelections();

    // Create a list containing all items that are *not* selected.
    // Rather than try to be real clever I'll just brute force the search.
    std::vector<CB::not_null<CDrawObj*>> listDObj;
    for (size_t nItem = size_t(0) ; nItem < m_listSel->GetItemCount() ; ++nItem)
    {
        // Loop and see if item is in selected list.
        size_t nSelItem;
        for (nSelItem = size_t(0) ; nSelItem < tblListBoxSel.size() ; ++nSelItem)
        {
            if (tblListBoxSel.at(nSelItem) == nItem)
                break;
        }
        if (bRemoveSelectedItems && nSelItem == tblListBoxSel.size())
        {
            // Not a listbox selection so add it to new select list.
            listDObj.push_back(&m_listSel->MapIndexToItem(nItem));
        }
        else if (!bRemoveSelectedItems && nSelItem < tblListBoxSel.size())
        {
            // It is a listbox selection so add it to new select list.
            listDObj.push_back(&m_listSel->MapIndexToItem(nItem));
        }
    }
    CPlayBoardFrame* pFrame = static_cast<CPlayBoardFrame*>(parent->GetParentFrame());
    pFrame->SendMessageToActiveBoardPane(WM_SELECT_BOARD_OBJLIST, (WPARAM)&*m_pPBoard,
        (LPARAM)&listDObj);
}

void CSelectedPieceViewContainer::OnDraw(CDC* pDC)
{
    // do nothing because child covers entire client rect
}

void CSelectedPieceViewContainer::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    child->OnUpdate(pSender, lHint, pHint);

    BASE::OnUpdate(pSender, lHint, pHint);
}

CSelectedPieceViewContainer::CSelectedPieceViewContainer() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

int CSelectedPieceViewContainer::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (BASE::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    child = new CSelectedPieceView(*this);

    return 0;
}

void CSelectedPieceViewContainer::OnSize(UINT nType, int cx, int cy)
{
    child->SetSize(0, 0, cx, cy);
    return BASE::OnSize(nType, cx, cy);
}

int CSelectedPieceViewContainer::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
    // We don't want the frame to ever consider this view to be the
    // "active" view.
    return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

LRESULT CSelectedPieceViewContainer::OnMessageWindowState(WPARAM wParam, LPARAM lParam)
{
    WinStateEvent event(*reinterpret_cast<CArchive*>(wParam), bool(lParam));
    child->ProcessWindowEvent(event);
    return (LRESULT)1;
}

