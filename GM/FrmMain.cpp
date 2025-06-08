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
#include    "FrmBited.h"
#include    "FrmMain.h"
#include    "VwEdtbrd.h"
#include    "VwPrjgbx.h"
#include    "VwTilesl.h"
#include    "PalColor.h"
#include    "LibMfc.h"

#include    "HtmlHelp.h"

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
#endif
    EVT_MENU(XRCID("ID_WINDOW_TOOLPAL"), OnWindowToolPal)
    EVT_MENU(XRCID("ID_WINDOW_ITOOLPAL"), OnWindowIToolPal)
    EVT_MENU(XRCID("ID_WINDOW_COLORPAL"), OnWindowColorPal)
    EVT_UPDATE_UI(XRCID("ID_WINDOW_TOOLPAL"), OnUpdateWindowToolPal)
    EVT_UPDATE_UI(XRCID("ID_WINDOW_ITOOLPAL"), OnUpdateWindowIToolPal)
    EVT_UPDATE_UI(XRCID("ID_WINDOW_COLORPAL"), OnUpdateWindowColorPal)
    EVT_UPDATE_UI(XRCID("ID_EDIT_LAYER_BASE"), OnUpdateDisable)
#if 0
    ON_WM_HELPINFO()
    ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
#endif
    EVT_UPDATE_UI(XRCID("ID_EDIT_LAYER_TILE"), OnUpdateDisable)
    EVT_UPDATE_UI(XRCID("ID_EDIT_LAYER_TOP"), OnUpdateDisable)
    EVT_UPDATE_UI(XRCID("ID_VIEW_GRIDLINES"), OnUpdateDisable)
    EVT_UPDATE_UI(XRCID("ID_TOOLS_BRDSNAPGRID"), OnUpdateDisable)
    EVT_UPDATE_UI(XRCID("ID_TOOLS_ROT90"), OnUpdateDisable)
    EVT_UPDATE_UI(XRCID("ID_ITOOL_PENCIL"), OnUpdateDisable)
    EVT_UPDATE_UI(XRCID("ID_TOOL_ARROW"), OnUpdateDisable)
    EVT_MENU(XRCID("ID_WINDOW_TILEPAL"), OnWindowTilePalette)
    EVT_UPDATE_UI(XRCID("ID_WINDOW_TILEPAL"), OnUpdateWindowTilePalette)
    EVT_MENU(XRCID("IDW_COLOR_PALETTE"), OnToggleColorPalette)
    EVT_MENU(XRCID("IDW_TILE_PALETTE"), OnToggleTilePalette)
#if 0
    ON_COMMAND(ID_HELP_FINDER, CMDIFrameWndEx::OnHelpFinder)
    ON_COMMAND(ID_HELP, CMDIFrameWndEx::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWndEx::OnContextHelp)
#endif
    EVT_MENU(XRCID("ID_WINDOW_TILE_HORZ"), OnTile)
    EVT_MENU(XRCID("ID_WINDOW_TILE_VERT"), OnTile)
    EVT_UPDATE_UI(XRCID("ID_WINDOW_TILE_HORZ"), OnUpdateTile)
    EVT_UPDATE_UI(XRCID("ID_WINDOW_TILE_VERT"), OnUpdateTile)
    EVT_AUI_PANE_CLOSE(OnPaneClose)
    EVT_UPDATE_UI(wxID_EXIT, OnUpdateEnable)
    EVT_UPDATE_UI_RANGE(wxID_FILE1, wxID_FILE9, OnUpdateEnable)
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

static const wxClassInfo *tblColor[] = {
    wxCLASSINFO(wxBrdEditView),
    wxCLASSINFO(wxBitEditView),
    NULL };

static const wxClassInfo *tblBrd[] = {
    wxCLASSINFO(wxBrdEditView),
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
    static const struct
    {
        int xrcId;
        CB::string menuString;
        int helpId;
        wxItemKind kind;
    } windowMenuArgs[] = {
        { wxID_SEPARATOR },
        { XRCID("ID_WINDOW_TILE_HORZ"), "Split Tabs &Horizontally"_cbstring, ID_WINDOW_TILE_HORZ, wxITEM_NORMAL },
        { XRCID("ID_WINDOW_TILE_VERT"), "Split Tabs &Vertically"_cbstring, ID_WINDOW_TILE_VERT, wxITEM_NORMAL },
        { wxID_SEPARATOR },
        { XRCID("ID_WINDOW_TOOLPAL"), "T&ool Palette"_cbstring, ID_WINDOW_TOOLPAL, wxITEM_CHECK },
        { XRCID("ID_WINDOW_COLORPAL"), "Co&lor Palette"_cbstring, ID_WINDOW_TOOLPAL, wxITEM_CHECK },
        { XRCID("ID_WINDOW_TILEPAL"), "T&ile Palette"_cbstring, ID_WINDOW_TOOLPAL, wxITEM_CHECK },
        { XRCID("ID_WINDOW_ITOOLPAL"), "I&mage Palette"_cbstring, ID_WINDOW_TOOLPAL, wxITEM_CHECK },
    };
    for (const auto& arg : windowMenuArgs)
    {
        if (arg.xrcId != wxID_SEPARATOR)
        {
            CB::string str = CB::string::LoadString(arg.helpId);
            std::vector<wxString> tokens;
            wxStringTokenizer tokenizer(str, "\n");
            while (tokenizer.HasMoreTokens())
            {
                tokens.push_back(tokenizer.GetNextToken());
            }
            wxASSERT(!tokens.empty());
            wndMenu.Append(arg.xrcId, arg.menuString, tokens.front(), arg.kind);
        }
        else
        {
            wndMenu.AppendSeparator();
        }
    }

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

    m_wndColorPal = new CColorPalette;
    m_wndColorPal->Create(this);
    auiManager.AddPane(m_wndColorPal, wxAuiPaneInfo().
                        Name("CColorPalette").Caption("Colors"_cbstring).
                        MinSize(m_wndColorPal->GetBestSize()).
                        BestSize(m_wndColorPal->GetBestSize()).
                        MaxSize(m_wndColorPal->GetBestSize()).
                        FloatingClientSize(m_wndColorPal->GetBestSize()).
                        Right().Layer(0).Position(0).
                        Hide());

    m_wndTilePal = new CDockTilePalette;
    m_wndTilePal->Create(this);
    auiManager.AddPane(m_wndTilePal, wxAuiPaneInfo().
                        Name("CTilePalette").Caption("Tiles"_cbstring).
                        BestSize(m_wndTilePal->GetBestSize()).
                        FloatingClientSize(m_wndTilePal->GetBestSize()).
                        Right().Layer(0).Position(1).
                        Hide());

    // default width doesn't leave enough space for color palette
    SetClientSize(4*m_wndColorPal->GetBestSize().x, GetClientSize().y);
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

wxDocument* CMainFrame::GetCurrentDocument()
{
    return GetDocumentManager()->GetCurrentDocument();
}

wxView* CMainFrame::GetActiveView() const
{
    return GetDocumentManager()->GetCurrentView();
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

void CMainFrame::OnUpdateEnable(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(true);
}

///////////////////////////////////////////////////////////////////////
// Default attribute handlers. If no views respond to certain update
// commands, the main frame window then gets a whack at it. We (what
// do you mean 'we'? Is there anyone else here?) simply disable
// the 'items'.

void CMainFrame::OnUpdateDisable(wxUpdateUIEvent& pCmdUI)
{
    if (pCmdUI.IsCheckable())
    {
        pCmdUI.Check(false);
    }
    pCmdUI.Enable(false);
}

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
#endif

///////////////////////////////////////////////////////////////////////
// System color palette message handlers

void CMainFrame::OnToggleColorPalette(wxCommandEvent& /*event*/)
{
    wxCommandEvent event(wxEVT_MENU, XRCID("ID_WINDOW_COLORPAL"));
    ProcessWindowEvent(event);
}

void CMainFrame::OnToggleTilePalette(wxCommandEvent& /*event*/)
{
    wxCommandEvent event(wxEVT_MENU, XRCID("ID_WINDOW_TILEPAL"));
    ProcessWindowEvent(event);
}

void CMainFrame::OnTile(wxCommandEvent& event)
{
    Tile(event.GetId() == XRCID("ID_WINDOW_TILE_HORZ") ? wxHORIZONTAL : wxVERTICAL);
}

void CMainFrame::OnUpdateTile(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Enable(GetClientWindow()->GetPageCount() >= 2);
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

void CMainFrame::UpdatePaletteWindow(wxWindow& pWnd, const wxClassInfo** pRtc, BOOL bIsOn)
{
    if (pWnd.GetHandle())       // Handle exists if palette allowed
    {
        wxAuiPaneInfo& pane = auiManager.GetPane(&pWnd);
        BOOL bVisible = pane.IsShown();

        wxView* pView = GetDocumentManager()->GetCurrentView();
        if (!pView)
        {
            if (bVisible)
            {
                pane.Show(false);
                auiMgrScheduleUpdate = true;
            }
        }
        else
        {
            wxASSERT(typeid(*pView) == typeid(wxBitEditView) ||
                    typeid(*pView) == typeid(wxBrdEditView) ||
                    typeid(*pView) == typeid(wxGbxProjView));

            while (*pRtc != NULL)
            {
                if (pView->IsKindOf(*pRtc))
                {
                    if (bIsOn && !bVisible)
                    {
                        pane.Show(true);
                        auiMgrScheduleUpdate = true;
                    }
                    else if (!bIsOn && bVisible)
                    {
                        pane.Show(false);
                        auiMgrScheduleUpdate = true;
                    }
                    return;
                }
                pRtc++;
            }
            if (bVisible)
            {
                pane.Show(false);
                auiMgrScheduleUpdate = true;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////

BOOL CMainFrame::IsQualifyingView(wxWindow& pWnd, const wxClassInfo** pRtc)
{
    if (pWnd.GetHandle())       // Handle exists if palette allowed
    {
        wxView* pView = GetDocumentManager()->GetCurrentView();
        if (pView)
        {
            wxASSERT(typeid(*pView) == typeid(wxBitEditView) ||
                    typeid(*pView) == typeid(wxBrdEditView) ||
                    typeid(*pView) == typeid(wxGbxProjView));

            while (*pRtc != NULL)
            {
                if (pView->IsKindOf(*pRtc))
                    return TRUE;
                pRtc++;
            }
        }
    }
    return FALSE;
}

///////////////////////////////////////////////////////////////////////

void CMainFrame::OnIdle()
{
    if (IsIconized())         // No window palette processing if app minimized
        return;

    UpdatePaletteWindow(*m_wndColorPal, tblColor, m_bColorPalOn);

    if (m_wndColorPal->GetHandle() && m_wndColorPal->IsShownOnScreen())
    {
        wxCommandEvent event(wxEVT_MENU, XRCID("WM_IDLEUPDATECMDUI"));
        m_wndColorPal->ProcessWindowEvent(event);
    }

    if (auiMgrScheduleUpdate)
    {
        auiMgrScheduleUpdate = false;
        auiManager.Update();
    }
}

namespace {
    void OnClosePalette(wxWindow& wxWnd)
    {
        wxCommandEvent event(WM_PALETTE_HIDE_WX);
        // for color palette
        wxWnd.GetEventHandler()->ProcessEventLocally(event);
        // for tile palette
        CB::SendEventToDescendants(wxWnd, event, true);
    }
}

void CMainFrame::OnPaneClose(wxAuiManagerEvent& event)
{
    wxAuiPaneInfo& pane = CheckedDeref(event.GetPane());
    if (pane.name == "CColorPalette" ||
        pane.name == "CTilePalette")
    {
        OnClosePalette(CheckedDeref(pane.window));
    }
    else
    {
        event.Skip();
    }
}

/////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowToolPal(wxCommandEvent& /*event*/)
{
    wxAuiPaneInfo& pane = auiManager.GetPane(m_wndToolPal);
    pane.Show(!pane.IsShown());
    auiManager.Update();
}

void CMainFrame::OnUpdateWindowToolPal(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(m_wndToolPal->IsShownOnScreen());
}

/////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowIToolPal(wxCommandEvent& /*event*/)
{
    wxAuiPaneInfo& pane = auiManager.GetPane(m_wndIToolPal);
    pane.Show(!pane.IsShown());
    auiManager.Update();
}

void CMainFrame::OnUpdateWindowIToolPal(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(m_wndIToolPal->IsShownOnScreen());
}

/////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowColorPal(wxCommandEvent& /*event*/)
{
    m_bColorPalOn = !m_bColorPalOn;
}

void CMainFrame::OnUpdateWindowColorPal(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(m_bColorPalOn);
    pCmdUI.Enable(IsQualifyingView(*m_wndColorPal, tblColor));
}

/////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowTilePalette(wxCommandEvent& /*event*/)
{
    m_bTilePalOn = !m_bTilePalOn;
}

void CMainFrame::OnUpdateWindowTilePalette(wxUpdateUIEvent& pCmdUI)
{
    pCmdUI.Check(m_bTilePalOn);
    pCmdUI.Enable(IsQualifyingView(*m_wndTilePal, tblBrd));
}
