// FrmBited.cpp
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
#include    "Gm.h"
#include    "GmDoc.h"
#include    "FrmBited.h"
#include    "FrmMain.h"
#include    "VwBitedt.h"
#include    "VwTilesl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

wxIMPLEMENT_DYNAMIC_CLASS(wxBitEditView, wxView);

namespace {
    TileID createParam = nullTid;
}

/////////////////////////////////////////////////////////////////////////////
// CBitEditFrame

CBitEditFrame::CBitEditFrame(wxDocument& doc,
                wxBitEditView& view,
                wxAuiMDIParentFrame& parent) :
    BASE(&doc, &view, &parent, wxID_ANY,
        doc.GetUserReadableName() + " - Tile Editor"),
    m_wndSplitter([this, &view]{
        wxSplitterWindow* retval = new wxSplitterWindow(this,
            wxID_ANY,
            wxDefaultPosition,
            wxDefaultSize,
            /* this seems to be the only combo of flags
                that creates a visible sash */
            wxSP_3D | wxSP_NO_XP_THEME);
        retval->SetMinimumPaneSize(20);
        retval->SetSashGravity(0.0);
        CTileSelView* pSelView = new CTileSelView(*retval, view, createParam);
        CBitEditView* pBitView = new CBitEditView(*retval, view);
        // don't steal Ctrl-Tab from MDI client
        wxASSERT(retval->HasFlag(wxTAB_TRAVERSAL));
        retval->ToggleWindowStyle(wxTAB_TRAVERSAL);
        if (!retval->SplitVertically(pSelView, pBitView, 90))
        {
            AfxThrowMemoryException();
        }
        // Link the biteditor view to the tile selector and vice versa
        pSelView->SetBitEditor(*pBitView);
        pBitView->SetTileSelectView(*pSelView);
        pSelView->OnInitialUpdate();
        pBitView->OnInitialUpdate();
        return retval;
    }())
{
    SetIcon(wxIcon(std::format("#{}", IDR_BITEDITOR),
                            wxBITMAP_TYPE_ICO_RESOURCE,
                            16, 16));
    /* KLUDGE:  giving each frame its own menu
        seems to avoid crashes on process close */
    wxXmlResource::Get()->LoadMenuBar(this, "IDR_GAMEBOX"_cbstring);
    Layout();
}

#if 0
CBitEditFrame::~CBitEditFrame()
{
}
#endif


wxBEGIN_EVENT_TABLE(CBitEditFrame, CBitEditFrame::BASE)
#if 0
    ON_WM_CLOSE()
#endif
    EVT_MENU(XRCID("ID_IMAGE_DISCARD"), OnImageDiscard)
    EVT_MENU(XRCID("ID_IMAGE_UPDATE"), OnImageUpdate)
    EVT_MENU(XRCID("ID_TOOLS_RESIZETILE"), OnToolsResizeTile)
    EVT_UPDATE_UI(XRCID("ID_IMAGE_DISCARD"), OnUpdateEnable)
    EVT_UPDATE_UI(XRCID("ID_IMAGE_UPDATE"), OnUpdateEnable)
    EVT_UPDATE_UI(XRCID("ID_TOOLS_RESIZETILE"), OnUpdateEnable)
#if 0
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
#endif
    EVT_MENU(XRCID("ID_TOOLS_ROT90"), OnRotateTile)
    EVT_MENU(XRCID("ID_TOOLS_ROT180"), OnRotateTile)
    EVT_MENU(XRCID("ID_TOOLS_ROT270"), OnRotateTile)
    EVT_UPDATE_UI(XRCID("ID_TOOLS_ROT90"), OnUpdateRotateTile)
    EVT_UPDATE_UI(XRCID("ID_TOOLS_ROT180"), OnUpdateRotateTile)
    EVT_UPDATE_UI(XRCID("ID_TOOLS_ROT270"), OnUpdateRotateTile)
wxEND_EVENT_TABLE()


/////////////////////////////////////////////////////////////////////////////

#if 0
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
#endif

void CBitEditFrame::OnImageDiscard(wxCommandEvent& WXUNUSED(event))
{
    GetTileSelView().SetNoUpdate();
    Close();
}

void CBitEditFrame::OnImageUpdate(wxCommandEvent& WXUNUSED(event))
{
    GetTileSelView().UpdateDocumentTiles();
}

void CBitEditFrame::OnToolsResizeTile(wxCommandEvent& WXUNUSED(event))
{
    GetTileSelView().DoTileResizeDialog();
}

#if 0
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
#endif

void CBitEditFrame::OnRotateTile(wxCommandEvent& event)
{
    int id = event.GetId();
    int nAngle = (id == XRCID("ID_TOOLS_ROT90")) ? 90 :
        ((id == XRCID("ID_TOOLS_ROT180")) ? 180 : 270);
    GetTileSelView().DoTileRotation(nAngle);
}

void CBitEditFrame::OnUpdateRotateTile(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(TRUE);
}

void CBitEditFrame::OnUpdateEnable(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
}

wxBitEditView* wxBitEditView::New(CGamDoc& doc, TileID tid)
{
    wxDocManager& docMgr = CheckedDeref(wxDocManager::GetDocumentManager());
    wxDocTemplate& templ = CB::FindDocTemplateByView(*CLASSINFO(wxBitEditView));

    class CreateParamManager
    {
    public:
        CreateParamManager(TileID tid)
        {
            wxASSERT(createParam == nullTid);
            createParam = tid;
        }
        ~CreateParamManager()
        {
            createParam = nullTid;
        }
    } createParamMgr(tid);
    ::wxView* retval = templ.CreateView(&doc);
    wxASSERT(dynamic_cast<wxBitEditView*>(retval));
    return static_cast<wxBitEditView*>(retval);
}

CGamDoc& wxBitEditView::GetDocument()
{
    wxDocument& doc = CheckedDeref(wxView::GetDocument());
    wxASSERT(dynamic_cast<CGamDoc*>(&doc));
    return static_cast<CGamDoc&>(doc);
}

CBitEditFrame& wxBitEditView::GetFrame()
{
    wxWindow& frame = CheckedDeref(GetDocChildFrame()->GetWindow());
    wxASSERT(dynamic_cast<CBitEditFrame*>(&frame));
    return static_cast<CBitEditFrame&>(frame);
}

CBitEditView& CBitEditFrame::GetBitEditView()
{
    wxWindow& wnd = CheckedDeref(m_wndSplitter->GetWindow2());
    CBitEditView& bev = dynamic_cast<CBitEditView&>(wnd);
    return bev;
}

CTileSelView& CBitEditFrame::GetTileSelView()
{
    wxWindow& wnd = CheckedDeref(m_wndSplitter->GetWindow1());
    CTileSelView& tsvc = dynamic_cast<CTileSelView&>(wnd);
    return tsvc;
}

void wxBitEditView::OnActivateView(bool activate, ::wxView* activeView, ::wxView* deactiveView)
{
    wxASSERT(activeView == this);
    if (!activate)
    {
        return;
    }
    // wx tries to activate this before it's ready
    if (ready)
    {
        GetBitEditView().SetFocus();
    }
}

bool wxBitEditView::OnClose(bool /*deleteWindow*/)
{
    /* doc's life determined by wxGbxProjView, not this,
        so bypass wxView::OnClose() */
    FileHistoryRemoveMenu();
    return true;
}

bool wxBitEditView::OnCreate(wxDocument* doc, long flags)
{
    if (!wxView::OnCreate(doc, flags))
    {
        return false;
    }

    new CBitEditFrame(CheckedDeref(doc),
                    *this,
                    CheckedDeref(GetMainFrame()));
    FileHistoryAddMenu();
    /* wx tried to activate this before it was ready,
        so do it now */
    ready = true;
    Activate(true);

    return true;
}

void wxBitEditView::OnUpdate(::wxView* sender, wxObject* hint /*= nullptr*/)
{
    CB::wxView::OnUpdate(sender, hint);

    CGmBoxHintWx& hint2 = dynamic_cast<CGmBoxHintWx&>(CheckedDeref(hint));
    GetTileSelView().OnUpdate(nullptr, hint2.hint, hint2.hintObj);
    GetBitEditView().OnUpdate(nullptr, hint2.hint, hint2.hintObj);
}
