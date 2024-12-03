// FrmBited.cpp
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

#include    "stdafx.h"
#include    "Gm.h"
#include    "GmDoc.h"
#include    "FrmBited.h"
#include    "VwBitedt.h"
#include    "VwTilesl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitEditFrame

IMPLEMENT_DYNCREATE(CBitEditFrame, CMDIChildWndEx)

CBitEditFrame::CBitEditFrame() :
    CB::wxNativeContainerWindowMixin(static_cast<CWnd&>(*this))
{
}

CBitEditFrame::~CBitEditFrame()
{
}


BEGIN_MESSAGE_MAP(CBitEditFrame, CMDIChildWndEx)
    //{{AFX_MSG_MAP(CBitEditFrame)
    ON_WM_CLOSE()
    ON_COMMAND(ID_IMAGE_DISCARD, OnImageDiscard)
    ON_COMMAND(ID_IMAGE_UPDATE, OnImageUpdate)
    ON_COMMAND(ID_TOOLS_RESIZETILE, OnToolsResizeTile)
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
    ON_COMMAND_EX(ID_TOOLS_ROT90, OnRotateTile)
    ON_COMMAND_EX(ID_TOOLS_ROT180, OnRotateTile)
    ON_COMMAND_EX(ID_TOOLS_ROT270, OnRotateTile)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_ROT90, OnUpdateRotateTile)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_ROT180, OnUpdateRotateTile)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_ROT270, OnUpdateRotateTile)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////

BOOL CBitEditFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CMDIChildWndEx::PreCreateWindow(cs))
        return FALSE;

    cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW),
        (HBRUSH)GetStockObject(LTGRAY_BRUSH),
        AfxGetApp()->LoadIconW(IDR_BITEDITOR));

    cs.style |= WS_CLIPCHILDREN;
    cs.style &= ~(DWORD)FWS_ADDTOTITLE;
    return TRUE;
}

BOOL CBitEditFrame::OnCreateClient(LPCREATESTRUCT,
     CCreateContext* pContext)
{
    // Create a splitter with 1 row, 2 columns
    if (!m_wndSplitter.CreateStatic(this, 1, 2))
    {
        TRACE("Failed to Bit Editor Splitter\n");
        return FALSE;
    }

    // Add the first splitter pane - the tile editor selector
    if (!m_wndSplitter.CreateView(0, 0,
        RUNTIME_CLASS(CTileSelViewContainer), CSize(90, 380), pContext))
    {
        TRACE("Failed to create tile editor selector pane.\n");
        return FALSE;
    }

    // Add the second splitter pane - the default view which is the
    // the actual bitmap editor.
    if (!m_wndSplitter.CreateView(0, 1,
        pContext->m_pNewViewClass, CSize(380, 380), pContext))
    {
        TRACE("Failed to create bitmap edit pane.\n");
        return FALSE;
    }

    // Activate the editor view
    CBitEditViewContainer& pBitViewContainer = CheckedDeref(dynamic_cast<CBitEditViewContainer*>(m_wndSplitter.GetPane(0, 1)));
    CBitEditView& pBitView = pBitViewContainer.GetChild();
    SetActiveView(&pBitViewContainer);

    // Link the biteditor view to the tile selector and vice versa
    CTileSelView& pSelView = GetTileSelView();
    pSelView.SetBitEditor(pBitView);
    pBitView.SetTileSelectView(pSelView);

    SetWindowPos(NULL, 0, 0, 480, 380, SWP_NOACTIVATE | SWP_NOMOVE |
        SWP_NOREDRAW | SWP_NOZORDER);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBitEditFrame message handlers

void CBitEditFrame::OnClose()
{
    CWnd::OnClose();            // Short circuit frame's doc close code
}

void CBitEditFrame::OnImageDiscard()
{
    GetTileSelView().SetNoUpdate();
    PostMessage(WM_CLOSE, 0, 0L);
}

void CBitEditFrame::OnImageUpdate()
{
    GetTileSelView().UpdateDocumentTiles();
}

void CBitEditFrame::OnToolsResizeTile()
{
    GetTileSelView().DoTileResizeDialog();
}

void CBitEditFrame::OnEditUndo()
{
    wxASSERT(!"unreachable code?");
#if 0
    // The Tile Select view gets first whack at this...
    GetTileSelView().OnEditUndo();
#endif
}

void CBitEditFrame::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
    wxASSERT(!"unreachable code?");
#if 0
    // The Tile Select view gets first whack at this...
    GetTileSelView().OnUpdateEditUndo(pCmdUI);
#endif
}

BOOL CBitEditFrame::OnRotateTile(UINT id)
{
    int nAngle = (id == ID_TOOLS_ROT90) ? 90 :
        ((id == ID_TOOLS_ROT180) ? 180 : 270);
    GetTileSelView().DoTileRotation(nAngle);
    return TRUE;
}

void CBitEditFrame::OnUpdateRotateTile(CCmdUI* pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL && pCmdUI->m_nID == ID_TOOLS_ROT90)
    {
        // Need to enable menu that the submenu is connected to.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | MF_ENABLED);
    }

    pCmdUI->Enable(TRUE);
}

CTileSelView& CBitEditFrame::GetTileSelView()
{
    CWnd& wnd = CheckedDeref(m_wndSplitter.GetPane(0, 0));
    CTileSelViewContainer& tsvc = dynamic_cast<CTileSelViewContainer&>(wnd);
    return tsvc.GetChild();
}
