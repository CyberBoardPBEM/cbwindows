// FrmMain.cpp : implementation of the CMainFrame class
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
#include    "FrmMain.h"
#include    "VwEdtbrd.h"
#include    "VwBitedt.h"
#include    "VwTilesl.h"
#include    "PalColor.h"
#include    "LibMfc.h"

#include    "HtmlHelp.h"

#include    "afxpriv.h"     // For WM_IDLEUPDATECMDUI

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////
// CMainFrame

wxBEGIN_EVENT_TABLE(CMainFrame, wxDocParentFrameAny<CB::wxAuiMDIParentFrame>)
#if 0
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_COMMAND(ID_WINDOW_TOOLPAL, OnWindowToolPal)
    ON_COMMAND(ID_WINDOW_ITOOLPAL, OnWindowIToolPal)
    ON_COMMAND(ID_WINDOW_COLORPAL, OnWindowColorPal)
    ON_UPDATE_COMMAND_UI(ID_WINDOW_TOOLPAL, OnUpdateWindowToolPal)
    ON_UPDATE_COMMAND_UI(ID_WINDOW_ITOOLPAL, OnUpdateWindowIToolPal)
    ON_UPDATE_COMMAND_UI(ID_WINDOW_COLORPAL, OnUpdateWindowColorPal)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LAYER_BASE, OnUpdateDisable)
    ON_WM_HELPINFO()
    ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LAYER_TILE, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_EDIT_LAYER_TOP, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_VIEW_GRIDLINES, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_BRDSNAPGRID, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_ROT90, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_ITOOL_PENCIL, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_TOOL_ARROW, OnUpdateDisable)
    ON_COMMAND(ID_WINDOW_TILEPAL, OnWindowTilePalette)
    ON_UPDATE_COMMAND_UI(ID_WINDOW_TILEPAL, OnUpdateWindowTilePalette)
    ON_COMMAND(IDW_COLOR_PALETTE, OnToggleColorPalette)
    ON_COMMAND(IDW_TILE_PALETTE, OnToggleTilePalette)
    ON_COMMAND(ID_HELP_FINDER, CMDIFrameWndEx::OnHelpFinder)
    ON_COMMAND(ID_HELP, CMDIFrameWndEx::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWndEx::OnContextHelp)
#endif
    EVT_MENU(XRCID("ID_WINDOW_TILE_HORZ"), OnTile)
    EVT_MENU(XRCID("ID_WINDOW_TILE_VERT"), OnTile)
    EVT_UPDATE_UI(XRCID("ID_WINDOW_TILE_HORZ"), OnUpdateTile)
    EVT_UPDATE_UI(XRCID("ID_WINDOW_TILE_VERT"), OnUpdateTile)
    EVT_MENU(XRCID("ID_VIEW_STATUS_BAR"), OnViewStatusBar)
    EVT_UPDATE_UI(XRCID("ID_VIEW_STATUS_BAR"), OnUpdateViewStatusBar)
wxEND_EVENT_TABLE()

static const int indicators[] =
{
    value_preserving_cast<int>(wxID_SEPARATOR),           // status line indicator
    /* N.B.:  do not use XRCID with these because they are
        also string identifiers in .rc */
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_CELLNUM,
};

static UINT toolbars[] =
{
    IDR_MAINFRAME,
    IDB_DRAWLAYERTOOLS,
    IDB_IMAGETOOLS
};

///////////////////////////////////////////////////////////////////////
// These are used to qualify palette visiblility...

static const CRuntimeClass *tblColor[] = {
    RUNTIME_CLASS(CBrdEditViewContainer),
    RUNTIME_CLASS(CBitEditViewContainer),
    RUNTIME_CLASS(CTileSelViewContainer), NULL };

static const CRuntimeClass *tblBrd[] = {
    RUNTIME_CLASS(CBrdEditViewContainer),
    NULL
};

///////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame() :
    wxDocParentFrameAny<wxAuiMDIParentFrame>(wxDocManager::GetDocumentManager(),
                                            nullptr, wxID_ANY,
                                            wxTheApp->GetAppDisplayName())
{
    auiManager.SetManagedWindow(this);
    SetIcon(wxIcon(std::format("#{}", IDR_MAINFRAME)));
    GetClientWindow()->SetWindowStyleFlag(
                            GetClientWindow()->GetWindowStyleFlag() |
                            wxAUI_NB_WINDOWLIST_BUTTON);
    m_bColorPalOn = TRUE;
    m_bTilePalOn = TRUE;

    wxMenuBar& menubar = CheckedDeref(wxXmlResource::Get()->LoadMenuBar(this, "IDR_MAINFRAME"_cbstring));
    // mru File menu
    wxMenu& menuFile = CheckedDeref(menubar.GetMenu(size_t(0)));
    wxDocManager& docMgr = CheckedDeref(wxDocManager::GetDocumentManager());
    docMgr.FileHistoryUseMenu(&menuFile);
    docMgr.FileHistoryAddFilesToMenu(&menuFile);

    /* KLUDGE:  wx wants to construct Window menu itself, so we
                can't put Split commands in .xrc */
    wxMenu& wndMenu = CheckedDeref(GetWindowMenu());
    wndMenu.AppendSeparator();
    wndMenu.Append(XRCID("ID_WINDOW_TILE_HORZ"), "Split Tabs &Horizontally"_cbstring, "Split active tab into a new group horizontally"_cbstring);
    wndMenu.Append(XRCID("ID_WINDOW_TILE_VERT"), "Split Tabs &Vertically"_cbstring, "Split active tab into a new group vertically"_cbstring);

    // Build the main window tool bar.
    static const CB::ToolArgs standardArgs[] = {
        { wxID_NEW, ID_FILE_NEW },
        { wxID_OPEN, ID_FILE_OPEN },
        { wxID_SAVE, ID_FILE_SAVE },
        { wxID_SEPARATOR },
        { wxID_CUT, ID_EDIT_CUT },
        { wxID_COPY, ID_EDIT_COPY },
        { wxID_PASTE, ID_EDIT_PASTE },
        { wxID_SEPARATOR },
        { XRCID("ID_WINDOW_TILEPAL"), ID_WINDOW_TILEPAL, wxITEM_CHECK },
        { XRCID("ID_WINDOW_TOOLPAL"), ID_WINDOW_TOOLPAL, wxITEM_CHECK },
        { XRCID("ID_WINDOW_COLORPAL"), ID_WINDOW_COLORPAL, wxITEM_CHECK },
        { XRCID("ID_WINDOW_ITOOLPAL"), ID_WINDOW_ITOOLPAL, wxITEM_CHECK },
        { wxID_SEPARATOR },
        { XRCID("ID_VIEW_TOGGLE_SCALE"), ID_VIEW_TOGGLE_SCALE },
        { wxID_SEPARATOR },
        { XRCID("ID_EDIT_LAYER_BASE"), ID_EDIT_LAYER_BASE, wxITEM_CHECK },
        { XRCID("ID_EDIT_LAYER_TILE"), ID_EDIT_LAYER_TILE, wxITEM_CHECK },
        { XRCID("ID_EDIT_LAYER_TOP"), ID_EDIT_LAYER_TOP, wxITEM_CHECK },
        { wxID_SEPARATOR },
        { XRCID("ID_VIEW_GRIDLINES"), ID_VIEW_GRIDLINES, wxITEM_CHECK },
        { XRCID("ID_TOOLS_BRDSNAPGRID"), ID_TOOLS_BRDSNAPGRID, wxITEM_CHECK },
        { wxID_SEPARATOR },
        { XRCID("ID_DWG_TOFRONT"), ID_DWG_TOFRONT },
        { XRCID("ID_DWG_TOBACK"), ID_DWG_TOBACK },
        { wxID_SEPARATOR },
        { wxID_HELP_CONTEXT, ID_CONTEXT_HELP },
    };
    m_wndToolBar = &CB::CreateToolbar(*this,
                                        standardArgs,
                                        IDR_MAINFRAME);
    auiManager.AddPane(m_wndToolBar, wxAuiPaneInfo().
                        Name("IDR_MAINFRAME"_cbstring).Caption("Standard"_cbstring).
                        ToolbarPane().Top().Layer(0));

    m_wndToolPal = &CBrdEditView::CreateToolbar(*this);
    auiManager.AddPane(m_wndToolPal, wxAuiPaneInfo().
                        Name("IDB_DRAWLAYERTOOLS"_cbstring).Caption("Board Tools"_cbstring).
                        ToolbarPane().Right().Layer(1));

    m_wndIToolPal = &CBitEditView::CreateToolbar(*this);
    auiManager.AddPane(m_wndIToolPal, wxAuiPaneInfo().
                        Name("IDB_IMAGETOOLS").Caption("Tile Tools"_cbstring).
                        ToolbarPane().Right().Layer(2));

    m_wndStatusBar = nullptr;
    wxCommandEvent dummy;
    OnViewStatusBar(dummy);

    auiManager.Update();
}

CMainFrame::~CMainFrame()
{
}

#if 0
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    CMDITabInfo mdiTabParams;
    mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_VS2005; // other styles available...
    mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
    mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI taba
    mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-coloring of MDI tabs
    mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
    EnableMDITabbedGroups(TRUE, mdiTabParams);

    if (!m_wndMenuBar.Create(this))
    {
        TRACE0("Failed to create menubar\n");
        return -1;      // fail to create
    }
    m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

    // Prevent the menu bar from taking the focus on activation
    CMFCPopupMenu::SetForceMenuFocus(FALSE);

    // Build the main window tool bar.
    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT,
            CBRS_GRIPPER | WS_CHILD | WS_VISIBLE | CBRS_TOP |
            CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
            CRect(0, 0, 0, 0), IDW_TOOLBAR_MAIN) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if (!m_wndToolPal.CreateEx(this, TBSTYLE_FLAT,
            CBRS_GRIPPER | WS_CHILD | WS_VISIBLE | CBRS_RIGHT |
            CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
            CRect(0, 0, 0, 0), IDW_TOOLBAR_BOARD_DRAW) ||
        !m_wndToolPal.LoadToolBar(IDB_DRAWLAYERTOOLS))
    {
        TRACE("Failed to create drawing tool palette\n");
        return FALSE;
    }

    if (!m_wndIToolPal.CreateEx(this, TBSTYLE_FLAT,
            CBRS_GRIPPER | WS_CHILD | WS_VISIBLE | CBRS_RIGHT |
            CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
            CRect(0, 0, 0, 0), IDW_TOOLBAR_IMAGE_DRAW) ||
        !m_wndIToolPal.LoadToolBar(IDB_IMAGETOOLS))
    {
        TRACE("Failed to create image editor tool palette\n");
        return FALSE;
    }

    if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators,
        sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }
    m_wndStatusBar.SetPaneStyle(0, SBPS_STRETCH);

    // The starting sizes of the Color and Tile palette windows are suggestions. They
    // will be automatically recomputed when all windows are created and displayed.

    if (!m_wndColorPal.Create("Colors"_cbstring, this, CSize(100, 100), TRUE, IDW_COLOR_PALETTE,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create color palette window\n");
        return -1;      // fail to create
    }

    if (!m_wndTilePal.Create("Tiles"_cbstring, this, CSize(100, 800), TRUE, IDW_TILE_PALETTE,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create tile palette dock window\n");
        return -1;      // fail to create
    }

    m_wndToolBar.SetWindowText("Standard"_cbstring);
    m_wndToolPal.SetWindowText("Board Tools"_cbstring);
    m_wndIToolPal.SetWindowText("Tile Tools"_cbstring);

    m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    m_wndToolPal.EnableDocking(CBRS_ALIGN_ANY);
    m_wndIToolPal.EnableDocking(CBRS_ALIGN_ANY);
    m_wndTilePal.EnableDocking(CBRS_ALIGN_ANY);
    m_wndColorPal.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);

    DockPane(&m_wndMenuBar);
    DockPane(&m_wndToolBar);

    DockPane(&m_wndIToolPal, AFX_IDW_DOCKBAR_RIGHT);
    DockPaneLeftOf(&m_wndToolPal, &m_wndIToolPal);

    DockPane(&m_wndTilePal, AFX_IDW_DOCKBAR_RIGHT);
    m_wndColorPal.DockToWindow(&m_wndTilePal, CBRS_ALIGN_BOTTOM);

    RestoreProfileSettings();

    return 0;
}

void CMainFrame::OnClose()
{
    SaveProfileSettings();
    CMDIFrameWndEx::OnClose();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return TRUE;
}

void CMainFrame::WinHelp(DWORD_PTR dwData, UINT nCmd)
{
    if (dwData != 0)
        GetApp()->DoHelpContext(dwData);
    else
        GetApp()->DoHelpContents();
}

void CMainFrame::OnHelpIndex()
{
    GetApp()->DoHelpContents();
}
#endif

///////////////////////////////////////////////////////////////////////

CDocument* CMainFrame::GetCurrentDocument()
{
#if 0
    CMDIChildWndEx* pMDIChild = (CMDIChildWndEx*)MDIGetActive();
    if (pMDIChild != NULL)
    {
        CView *pView = pMDIChild->GetActiveView();
        ASSERT(pView != NULL);
        ASSERT(pView->IsKindOf(RUNTIME_CLASS(CView)));
        return pView->GetDocument();
    }
    return NULL;
#else
    wxASSERT(!"untested code");
    wxDocManager& docMgr = CheckedDeref(wxDocManager::GetDocumentManager());
    wxDocument* doc = docMgr.GetCurrentDocument();
    if (!doc)
    {
        return nullptr;
    }
    return dynamic_cast<CGamDoc&>(*doc);
#endif
}

CView* CMainFrame::GetActiveView() const
{
#if 0
    CMDIChildWndEx* pMDIChild = (CMDIChildWndEx*)MDIGetActive();
    if (pMDIChild != NULL)
        return pMDIChild->GetActiveView();
    return NULL;
#else
    AfxThrowNotSupportedException();
#endif
}

///////////////////////////////////////////////////////////////////////

static const CB::string szSectSettings = "Settings";
static const CB::string szSectControlBars = "ControlBars";
static const CB::string szEntryToolPal = "ToolPal";
static const CB::string szEntryIToolPal = "ImageToolPal";
static const CB::string szEntryColorPal = "ColorPal";
static const CB::string szEntryTilePal = "TilePal";
static const CB::string szEntryToolBar = "ToolBar";
static const CB::string szEntryStatusBar = "StatusBar";

void CMainFrame::SaveProfileSettings()
{
#if 0
    SaveBarState(szSectControlBars);

    GetApp()->WriteProfileInt(szSectSettings, szEntryColorPal, m_bColorPalOn);
    GetApp()->WriteProfileInt(szSectSettings, szEntryTilePal, m_bTilePalOn);

    GetApp()->WriteProfileInt(szSectSettings, szEntryStatusBar,
        (m_wndStatusBar.GetStyle() & WS_VISIBLE) ? 1 : 0);
#else
    AfxThrowNotSupportedException();
#endif
}

void CMainFrame::RestoreProfileSettings()
{
#if 0
    // Note: I only send a message to turn off the tool and status bars
    // since MFC sets them on by default.
    if (!GetApp()->GetProfileInt(szSectSettings, szEntryStatusBar, 1))
        SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0L);

    m_bColorPalOn = GetApp()->GetProfileInt(szSectSettings, szEntryColorPal, TRUE);
    m_bTilePalOn = GetApp()->GetProfileInt(szSectSettings, szEntryTilePal, TRUE);
#else
    AfxThrowNotSupportedException();
#endif
}

///////////////////////////////////////////////////////////////////////
// Default attribute handlers. If no views respond to certain update
// commands, the main frame window then gets a whack at it. We (what
// do you mean 'we'? Is there anyone else here?) simply disable
// the 'items'.

#if 0
void CMainFrame::OnUpdateDisable(CCmdUI* pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // Need to handle menu that the submenu is connected to.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
    }
    pCmdUI->SetCheck(0);
    pCmdUI->Enable(0);
}
#endif

///////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#if 0
#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////
// System color palette message handlers

void CMainFrame::OnToggleColorPalette()
{
    SendMessage(WM_COMMAND, ID_WINDOW_COLORPAL);
}

void CMainFrame::OnToggleTilePalette()
{
    SendMessage(WM_COMMAND, ID_WINDOW_TILEPAL);
}
#endif

void CMainFrame::OnTile(wxCommandEvent& event)
{
    Tile(event.GetId() == XRCID("ID_WINDOW_TILE_HORZ") ? wxHORIZONTAL : wxVERTICAL);
}

void CMainFrame::OnUpdateTile(wxUpdateUIEvent& event)
{
    event.Enable(GetClientWindow()->GetPageCount() >= 2);
}

void CMainFrame::OnViewStatusBar(wxCommandEvent& event)
{
    if (!m_wndStatusBar)
    {
        m_wndStatusBar = &CreateStatusBar(indicators);
    }
    else
    {
        SetStatusBar(nullptr);
        delete m_wndStatusBar;
        m_wndStatusBar = nullptr;
    }
}

void CMainFrame::OnUpdateViewStatusBar(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
    pCmdUI.Check(m_wndStatusBar);
}

///////////////////////////////////////////////////////////////////////
// pRtc points to a NULL terminated list of CRuntimeClass pointers
// specifying qualifying active views for the specified palette.

void CMainFrame::UpdatePaletteWindow(CWnd& pWnd, const CRuntimeClass** pRtc, BOOL bIsOn)
{
#if 0
    if (pWnd.m_hWnd != NULL)       // Handle exists if palette allowed
    {
        BOOL bIsControlBar = pWnd.IsKindOf(RUNTIME_CLASS(CBasePane));
        BOOL bVisible = ((pWnd.GetStyle() & WS_VISIBLE) != 0);

        CMDIChildWndEx* pMDIChild = (CMDIChildWndEx*)MDIGetActive();
        if (pMDIChild == NULL || pMDIChild->IsIconic())
        {
            if (!bIsControlBar && bVisible)
                pWnd.ShowWindow(SW_HIDE);
            else if (bIsControlBar && bVisible)
                ShowPane((CBasePane*)&pWnd, FALSE, FALSE, FALSE);
        }
        else
        {
            CView *pView = pMDIChild->GetActiveView();
            ASSERT(pView != NULL);
            wxASSERT(typeid(*pView) != typeid(CTileSelView) &&
                        typeid(*pView) != typeid(CTileSelViewContainer) &&
                        typeid(*pView) != typeid(CBitEditView));

            while (*pRtc != NULL)
            {
                if (pView->IsKindOf(*pRtc))
                {
                    if (bIsOn && !bVisible)
                    {
                        if (!bIsControlBar)
                            pWnd.ShowWindow(SW_SHOW);
                        else
                            ShowPane((CBasePane*)&pWnd, TRUE, FALSE, FALSE);
                    }
                    else if (!bIsOn && bVisible)
                    {
                        if (!bIsControlBar)
                            pWnd.ShowWindow(SW_HIDE);
                        else
                            ShowPane((CBasePane*)&pWnd, FALSE, FALSE, FALSE);
                    }
                    return;
                }
                pRtc++;
            }
            if (!bIsControlBar && bVisible)
                pWnd.ShowWindow(SW_HIDE);
            else if (bIsControlBar && bVisible)
                ShowPane((CBasePane*)&pWnd, FALSE, FALSE, FALSE);
        }
    }
#else
    AfxThrowNotSupportedException();
#endif
}

///////////////////////////////////////////////////////////////////////

BOOL CMainFrame::IsQualifyingView(CWnd& pWnd, const CRuntimeClass** pRtc)
{
#if 0
    if (pWnd.m_hWnd != NULL)       // Handle exists if palette allowed
    {
        CMDIChildWndEx* pMDIChild = (CMDIChildWndEx*)MDIGetActive();
        if (pMDIChild != NULL)
        {
            CView *pView = pMDIChild->GetActiveView();
            ASSERT(pView != NULL);
            wxASSERT(typeid(*pView) != typeid(CTileSelView) &&
                        typeid(*pView) != typeid(CTileSelViewContainer) &&
                        typeid(*pView) != typeid(CBitEditView));

            while (*pRtc != NULL)
            {
                if (pView->IsKindOf(*pRtc))
                    return TRUE;
                pRtc++;
            }
        }
    }
    return FALSE;
#else
    AfxThrowNotSupportedException();
#endif
}

///////////////////////////////////////////////////////////////////////

void CMainFrame::OnIdle()
{
    if (IsIconized())         // No window palette processing if app minimized
        return;

#if 0
    UpdatePaletteWindow(m_wndColorPal, tblColor, m_bColorPalOn);

    if (GetCurrentDocument() == NULL)
        ShowPane(&m_wndTilePal, FALSE, FALSE, TRUE);

    if (m_wndColorPal.m_hWnd != NULL && m_wndColorPal.IsWindowVisible())
        m_wndColorPal.SendMessage(WM_IDLEUPDATECMDUI, (WPARAM)TRUE);
#endif
}

namespace {
    /* CB is currently a mix of MFC and wx,
        so need to send both kinds of msg */
    BOOL OnClosePalette(CWnd& pWnd)
    {
        pWnd.SendMessage(WM_PALETTE_HIDE);
        pWnd.SendMessageToDescendants(WM_PALETTE_HIDE, true, true);
        wxWindow* wxWnd = CB::FindWxWindow(pWnd);
        if (wxWnd)
        {
            wxCommandEvent event(WM_PALETTE_HIDE_WX);
            wxWnd->GetEventHandler()->ProcessEventLocally(event);
            CB::SendEventToDescendants(*wxWnd, event, true);
        }
        return true;
    }
}

#if 0
BOOL CMainFrame::OnCloseMiniFrame(CPaneFrameWnd* pWnd)
{
    return OnClosePalette(CheckedDeref(pWnd));
}

BOOL CMainFrame::OnCloseDockingPane(CDockablePane* pWnd)
{
    return OnClosePalette(CheckedDeref(pWnd));
}

/////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowToolPal()
{
    ShowPane(&m_wndToolPal, (m_wndToolPal.GetStyle() & WS_VISIBLE) == 0, FALSE, TRUE);
}

void CMainFrame::OnUpdateWindowToolPal(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck((m_wndToolPal.GetStyle() & WS_VISIBLE) != 0);
}

/////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowIToolPal()
{
    ShowPane(&m_wndIToolPal, (m_wndIToolPal.GetStyle() & WS_VISIBLE) == 0, FALSE, TRUE);
}

void CMainFrame::OnUpdateWindowIToolPal(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck((m_wndIToolPal.GetStyle() & WS_VISIBLE) != 0);
}

/////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowColorPal()
{
    m_bColorPalOn = !m_bColorPalOn;
}

void CMainFrame::OnUpdateWindowColorPal(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bColorPalOn);
    pCmdUI->Enable(IsQualifyingView(m_wndColorPal, tblColor));
}

/////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowTilePalette()
{
    m_bTilePalOn = !m_bTilePalOn;
}

void CMainFrame::OnUpdateWindowTilePalette(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bTilePalOn);
    pCmdUI->Enable(IsQualifyingView(m_wndTilePal, tblBrd));
}
#endif
