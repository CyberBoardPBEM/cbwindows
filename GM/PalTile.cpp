// PalTile.cpp - Tile palette window
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
#include    "Gm.h"
#include    "GmDoc.h"
#include    "Frmmain.h"
#include    "ResTbl.h"
#include    "Tile.h"
#include    "PalTile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTilePalette, CWnd)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTilePalette, CWnd)
    //{{AFX_MSG_MAP(CTilePalette)
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_CBN_SELCHANGE(IDC_W_TPAL_TSETS, OnTileNameCbnSelchange)
    ON_WM_HELPINFO()
    ON_WM_CREATE()
    //}}AFX_MSG_MAP
    ON_MESSAGE(WM_PALETTE_HIDE, OnPaletteHide)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

CTilePalette::CTilePalette()
{
    m_pDoc = NULL;
    m_pDockingFrame = NULL;
    m_listTile.EnableDrag(TRUE);
    m_nComboHeight = 0;
}

BOOL CTilePalette::Create(CWnd* pOwnerWnd, DWORD dwStyle, UINT nID)
{
    dwStyle |= WS_CHILD | WS_VISIBLE;
    if (!CWnd::Create(AfxRegisterWndClass(0), NULL, dwStyle,
        CRect(0, 0, 200, 100), pOwnerWnd, nID))
    {
        TRACE("Failed to create Tile palette window.\n");
        return FALSE;
    }

    UpdatePaletteContents();
    return TRUE;
}

int CTilePalette::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rctCombo;
    GetClientRect(&rctCombo);
    rctCombo.bottom = 5 * g_res.tm8ssb.tmHeight;

    if (!m_comboTGrp.Create(
        WS_CHILD | WS_VSCROLL | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
        rctCombo, this, IDC_W_TPAL_TSETS))
    {
        TRACE("Failed to create Tile name combo-box\n");
        return -1;
    }
    m_comboTGrp.SetFont(CFont::FromHandle(g_res.h8ssb));

    m_comboTGrp.GetWindowRect(&rctCombo);   // Fetch result of create
    m_comboTGrp.MoveWindow(0, 0, rctCombo.Width(),
        5 * rctCombo.Height(), TRUE);
    m_comboTGrp.SetDroppedWidth(8 * g_res.tm8ssb.tmMaxCharWidth);

    m_nComboHeight = rctCombo.Height();     // Save for later use

    CRect rctLBox;
    GetClientRect(&rctLBox);
    rctLBox.top = rctCombo.Height() - 1;

    if (!m_listTile.Create(
        WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL |
            LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT,
        rctLBox, this, 0))
    {
        TRACE("Failed to create Tile list box.\n");
        return -1;
    }
    m_listTile.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CTilePalette::OnPaletteHide(WPARAM, LPARAM)
{
    GetMainFrame()->SendMessage(WM_COMMAND, ID_WINDOW_TILEPAL);
    return (LRESULT)0;
}

/////////////////////////////////////////////////////////////////////////////

TileID CTilePalette::GetCurrentTileID()
{
    if (m_hWnd == NULL)
        return nullTid;
    if (m_listTile.GetCurSel() == LB_ERR)
        return nullTid;
    return (TileID)m_listTile.GetCurMapItem();
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::Serialize(CArchive& ar)
{
#pragma message("TODO--> Serialize current indices <--TODO")
    if (ar.IsStoring())
    {
        ar << CRect();
    }
    else
    {
        CRect junk;
        ar >> junk;
    }
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::SetDocument(CGamDoc *pDoc)
{
    ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(CGamDoc)));
    m_pDoc = pDoc;
    m_listTile.SetDocument(pDoc);
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::LoadTileNameList()
{
    ASSERT(m_pDoc);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    m_comboTGrp.ResetContent();
    for (int i = 0; i < pTMgr->GetNumTileSets(); i++)
        m_comboTGrp.AddString(pTMgr->GetTileSet(i)->GetName());
    m_comboTGrp.SetCurSel(0);
    UpdateTileList();
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::UpdatePaletteContents()
{
    int nSel = m_comboTGrp.GetCurSel();
    if (nSel < 0)
        nSel = 0;               // Force first entry (if any)
    LoadTileNameList();
    if (nSel < m_comboTGrp.GetCount())
        m_comboTGrp.SetCurSel(nSel);
    UpdateTileList();
}

/////////////////////////////////////////////////////////////////////////////

void CTilePalette::UpdateTileList()
{
    ASSERT(m_pDoc);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    int nSel = m_comboTGrp.GetCurSel();
    if (nSel < 0)
    {
        m_listTile.SetItemMap(NULL);
        return;
    }
    CWordArray* pPieceTbl = pTMgr->GetTileSet(nSel)->GetTileIDTable();
    ASSERT(pPieceTbl != NULL);
    m_listTile.SetItemMap(pPieceTbl);
}

/////////////////////////////////////////////////////////////////////////////
// CTilePalette message handlers

BOOL CTilePalette::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;        // controls take care of erase
}

void CTilePalette::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    CRect rctCombo;

    if (m_comboTGrp.m_hWnd != NULL)
    {
        m_comboTGrp.GetWindowRect(&rctCombo);
        m_comboTGrp.MoveWindow(0, 0, cx, 5 * m_nComboHeight, TRUE);
    }
    if (m_listTile.m_hWnd != NULL)
    {
        m_listTile.MoveWindow(0, m_nComboHeight, cx, cy - m_nComboHeight, TRUE);
    }
}

void CTilePalette::PostNcDestroy()
{
    /* DO NOTHING - FRAME CLASS WOULD DELETE SELF! */
}

void CTilePalette::OnTileNameCbnSelchange()
{
    if (!m_comboTGrp.GetDroppedState())
        UpdateTileList();
}

BOOL CTilePalette::OnHelpInfo(HELPINFO* pHelpInfo)
{
    GetApp()->DoHelpTopic("gm-ref-pal-tile.htm");
    return TRUE;
}

