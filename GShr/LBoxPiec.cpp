// LBoxPiec.cpp
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
#include    "ResTbl.h"
#include    "Pieces.h"

#ifdef GPLAY
#include    "Gp.h"
#include    "GamDoc.h"
#else
#include    "Gm.h"
#include    "GmDoc.h"
#endif

#include    "LBoxPiec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CPieceListBox, CTileBaseListBox)
    //{{AFX_MSG_MAP(CPieceListBox)
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CPieceListBox::CPieceListBox()
{
    m_pPMgr = NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CPieceListBox::SetDocument(CGamDoc* pDoc)
{
    ResetContent();
    ASSERT(pDoc != NULL);
    m_pDoc = pDoc;
    m_pPMgr = pDoc->GetPieceManager();
}

CTileManager* CPieceListBox::GetTileManager()
{
    ASSERT(m_pDoc != NULL);
    ASSERT(m_pDoc->GetTileManager() != NULL);
    return m_pDoc->GetTileManager();
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing

BOOL CPieceListBox::OnIsToolTipsEnabled()
{
#ifdef GPLAY
    return m_pDoc->IsShowingObjectTips();
#else
    return TRUE;                // Always show in designer program
#endif
}

int  CPieceListBox::OnGetHitItemCodeAtPoint(CPoint point, CRect& rct)
{
    BOOL bOutsideClient;
    UINT nIndex = ItemFromPoint(point, bOutsideClient);
    if (nIndex >= 65535 || GetCount() <= 0)
        return -1;

    ASSERT(m_pDoc != NULL);

    UINT nPid = MapIndexToItem(nIndex);

    TileID tidLeft = m_pPMgr->GetPiece((PieceID)nPid)->GetFrontTID();
    ASSERT(tidLeft != nullTid);            // Should exist
    TileID tidRight = m_pPMgr->GetPiece((PieceID)nPid)->GetBackTID();

    CRect rctLeft;
    CRect rctRight;
    GetTileRectsForItem(nIndex, tidLeft, tidRight, rctLeft, rctRight);

    if (!rctLeft.IsRectEmpty() && rctLeft.PtInRect(point))
        rct = rctLeft;
    else if (!rctRight.IsRectEmpty() && rctRight.PtInRect(point))
    {
        rct = rctRight;
        nPid |= 0x10000;             // Set flag bit indicating right side rect
    }
    else
        return -1;

    return (int)nPid;
}

void CPieceListBox::OnGetTipTextForItemCode(int nItemCode,
    CString& strTip, CString& strTitle)
{
    if (nItemCode < 0)
        return;
    PieceID pid = (PieceID)(nItemCode & 0xFFFF);
    BOOL bRightRect = (nItemCode & 0x10000) != 0;
    GameElement elem = MakePieceElement(pid, bRightRect ? 1 : 0);
    strTip = m_pDoc->GetGameElementString(elem);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CPieceListBox::OnDoesItemHaveTipText(int nItem)
{
    PieceID pid = (PieceID)MapIndexToItem(nItem);
    return m_pDoc->HasGameElementString(MakePieceElement(pid, 0)) ||
        m_pDoc->HasGameElementString(MakePieceElement(pid, 1));
}

/////////////////////////////////////////////////////////////////////////////

int CPieceListBox::OnItemHeight(int nIndex)
{
    PieceID pid = (PieceID)MapIndexToItem(nIndex);

    TileID tidLeft = m_pPMgr->GetPiece(pid)->GetFrontTID();
    ASSERT(tidLeft != nullTid);            // Should exist
    TileID tidRight = m_pPMgr->GetPiece(pid)->GetBackTID();
    return DoOnItemHeight(tidLeft, tidRight);
}

void CPieceListBox::OnItemDraw(CDC* pDC, int nIndex, UINT nAction, UINT nState,
    CRect rctItem)
{
    if (nIndex < 0)
        return;

    PieceID pid = (PieceID)MapIndexToItem(nIndex);

    TileID tidLeft = m_pPMgr->GetPiece(pid)->GetFrontTID();
    ASSERT(tidLeft != nullTid);            // Should exist
    TileID tidRight = m_pPMgr->GetPiece(pid)->GetBackTID();

    DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tidLeft, tidRight);
}

BOOL CPieceListBox::OnDragSetup(DragInfo* pDI)
{
    pDI->m_dragType = DRAG_PIECE;
    pDI->m_dwVal = (DWORD)GetCurMapItem();          // The PieceID
    pDI->m_pObj = (void*)m_pPMgr;
    pDI->m_hcsrSuggest = g_res.hcrDragTile;
    return TRUE;
}

int CPieceListBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CGrafixListBox::OnCreate(lpCreateStruct) == -1)
        return -1;
    m_pPMgr = NULL;
    m_pDoc = NULL;
    return 0;
}
