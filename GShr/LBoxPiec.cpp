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

void CPieceListBox::SetDocument(CGamDoc& pDoc)
{
    CGrafixListBoxData::SetDocument(pDoc);
    ResetContent();
    m_pDoc = &pDoc;
    m_pPMgr = pDoc.GetPieceManager();
}

const CTileManager& CPieceListBox::GetTileManager() const
{
    ASSERT(m_pDoc != NULL);
    return CheckedDeref(m_pDoc->GetTileManager());
}

/////////////////////////////////////////////////////////////////////////////
// Tool tip processing

BOOL CPieceListBox::OnIsToolTipsEnabled() const
{
#ifdef GPLAY
    return m_pDoc->IsShowingObjectTips();
#else
    return TRUE;                // Always show in designer program
#endif
}

GameElement CPieceListBox::OnGetHitItemCodeAtPoint(CPoint point, CRect& rct) const
{
    point = ClientToItem(point);

    BOOL bOutsideClient;
    UINT nIndex = ItemFromPoint(point, bOutsideClient);
    if (nIndex >= 65535 || GetCount() <= 0)
        return Invalid_v<GameElement>;

    ASSERT(m_pDoc != NULL);

    PieceID nPid = MapIndexToItem(value_preserving_cast<size_t>(nIndex));

    const PieceDef& piecedef = m_pPMgr->GetPiece(nPid);
    std::vector<TileID> tids = piecedef.GetTIDs();

    std::vector<CRect> rects = GetTileRectsForItem(value_preserving_cast<int>(nIndex), tids);

    for (size_t i = size_t(0) ; i < rects.size() ; ++i)
    {
        ASSERT(!rects[i].IsRectEmpty());
        if (!rects[i].IsRectEmpty() && rects[i].PtInRect(point))
        {
            rct = ItemToClient(rects[i]);
            return GameElement(nPid, value_preserving_cast<unsigned>(i));
        }
    }

    return Invalid_v<GameElement>;
}

void CPieceListBox::OnGetTipTextForItemCode(GameElement nItemCode,
    CString& strTip, CString& strTitle) const
{
    if (nItemCode == Invalid_v<GameElement>)
        return;
    strTip = m_pDoc->GetGameElementString(nItemCode);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CPieceListBox::OnDoesItemHaveTipText(size_t nItem) const
{
    PieceID pid = MapIndexToItem(nItem);
    const PieceDef& piecedef = m_pPMgr->GetPiece(pid);
    for (size_t i = size_t(0) ; i < piecedef.GetSides() ; ++i)
    {
        if (m_pDoc->HasGameElementString(MakePieceElement(pid, value_preserving_cast<unsigned>(i))))
        {
            return true;
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////

CSize CPieceListBox::OnItemSize(size_t nIndex) const
{
    PieceID pid = MapIndexToItem(nIndex);

    const PieceDef& piecedef = m_pPMgr->GetPiece(pid);
    const std::vector<TileID>& tids = piecedef.GetTIDs();
    ASSERT(!tids.empty() && tids[size_t(0)] != nullTid);    // Should exist
    return DoOnItemSize(nIndex, tids);
}

void CPieceListBox::OnItemDraw(CDC& pDC, size_t nIndex, UINT nAction, UINT nState,
    CRect rctItem) const
{
    // see https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-drawitemstruct
    if (nIndex == size_t(UINT(-1)))
        return;

    PieceID pid = MapIndexToItem(nIndex);

    const PieceDef& piecedef = m_pPMgr->GetPiece(pid);
    const std::vector<TileID>& tids = piecedef.GetTIDs();
    ASSERT(!tids.empty() && tids[size_t(0)] != nullTid);    // Should exist

    DoOnDrawItem(pDC, nIndex, nAction, nState, rctItem, tids);
}

BOOL CPieceListBox::OnDragSetup(DragInfo& pDI) const
{
    ASSERT(!"untested code");
    pDI.SetDragType(DRAG_PIECE);
    pDI.GetSubInfo<DRAG_PIECE>().m_pieceID = GetCurMapItem();
    pDI.GetSubInfo<DRAG_PIECE>().m_size = GetDragSize();
    ASSERT(!"code look wrong, but I think this is unreachable");
    /* this is the original code, but it looks wrong since
        everything else seems to say DRAG_PIECE should have
        m_pObj contain CGamDoc*
    pDI.m_pObj = (void*)m_pPMgr;
    */
    pDI.m_hcsrSuggest = g_res.hcrDragTile;
    return TRUE;
}

int CPieceListBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CTileBaseListBox::OnCreate(lpCreateStruct) == -1)
        return -1;
    m_pPMgr = NULL;
    m_pDoc = NULL;
    return 0;
}
