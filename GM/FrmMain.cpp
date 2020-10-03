// FrmMain.cpp : implementation of the CMainFrame class
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

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndExCb)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndExCb)
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_PALETTECHANGED()
    ON_WM_QUERYNEWPALETTE()
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
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
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

static CRuntimeClass *tblColor[] = { RUNTIME_CLASS(CBrdEditView),
    RUNTIME_CLASS(CBitEditView), RUNTIME_CLASS(CTileSelView), NULL };

static CRuntimeClass *tblBit[] = { RUNTIME_CLASS(CBitEditView),
    RUNTIME_CLASS(CTileSelView), NULL };

static CRuntimeClass *tblBrd[] = { RUNTIME_CLASS(CBrdEditView), NULL };

///////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    m_bColorPalOn = TRUE;
    m_bTilePalOn = TRUE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Build a universal GDI color palette for the app...
    if (!BuildAppGDIPalette())
    {
        TRACE0("Failed to create application wide GDI palette.\n");
        return -1;      // fail to create
    }

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

    CSize colorPalSize;
    m_wndColorPal.CalculateMinClientSize(colorPalSize);
    m_wndColorPal.SetMinSize(colorPalSize);
    if (!m_wndColorPal.Create("Colors", this, colorPalSize, TRUE, IDW_COLOR_PALETTE,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create color palette window\n");
        return -1;      // fail to create
    }

    if (!m_wndTilePal.Create("Tiles", this, CSize(colorPalSize.cx, 800), TRUE, IDW_TILE_PALETTE,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create tile palette dock window\n");
        return -1;      // fail to create
    }

    m_wndToolBar.SetWindowText("Standard");
    m_wndToolPal.SetWindowText("Board Tools");
    m_wndIToolPal.SetWindowText("Tile Tools");

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

void CMainFrame::WinHelp(DWORD dwData, UINT nCmd)
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

///////////////////////////////////////////////////////////////////////

CDocument* CMainFrame::GetCurrentDocument()
{
    CMDIChildWndEx* pMDIChild = (CMDIChildWndEx*)MDIGetActive();
    if (pMDIChild != NULL)
    {
        CView *pView = pMDIChild->GetActiveView();
        ASSERT(pView != NULL);
        ASSERT(pView->IsKindOf(RUNTIME_CLASS(CView)));
        return pView->GetDocument();
    }
    return NULL;
}

CView* CMainFrame::GetActiveView() const
{
    CMDIChildWndEx* pMDIChild = (CMDIChildWndEx*)MDIGetActive();
    if (pMDIChild != NULL)
        return pMDIChild->GetActiveView();
    return NULL;
}

///////////////////////////////////////////////////////////////////////

static char szSectSettings[] = "Settings";
static char szSectControlBars[] = "ControlBars";
static char szEntryToolPal[] = "ToolPal";
static char szEntryIToolPal[] = "ImageToolPal";
static char szEntryColorPal[] = "ColorPal";
static char szEntryTilePal[] = "TilePal";
static char szEntryToolBar[] = "ToolBar";
static char szEntryStatusBar[] = "StatusBar";

void CMainFrame::SaveProfileSettings()
{
    SaveBarState(szSectControlBars);

    GetApp()->WriteProfileInt(szSectSettings, szEntryColorPal, m_bColorPalOn);
    GetApp()->WriteProfileInt(szSectSettings, szEntryTilePal, m_bTilePalOn);

    GetApp()->WriteProfileInt(szSectSettings, szEntryStatusBar,
        (m_wndStatusBar.GetStyle() & WS_VISIBLE) ? 1 : 0);
}

void CMainFrame::RestoreProfileSettings()
{
    // Note: I only send a message to turn off the tool and status bars
    // since MFC sets them on by default.
    if (!GetApp()->GetProfileInt(szSectSettings, szEntryStatusBar, 1))
        SendMessage(WM_COMMAND, ID_VIEW_STATUS_BAR, 0L);

    m_bColorPalOn = GetApp()->GetProfileInt(szSectSettings, szEntryColorPal, TRUE);
    m_bTilePalOn = GetApp()->GetProfileInt(szSectSettings, szEntryTilePal, TRUE);
}

///////////////////////////////////////////////////////////////////////
// Default attribute handlers. If no views respond to certain update
// commands, the main frame window then gets a whack at it. We (what
// do you mean 'we'? Is there anyone else here?) simply disable
// the 'items'.

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

///////////////////////////////////////////////////////////////////////

void GenerateColorWash(LPPALETTEENTRY pPE);   // See COLOR.CPP //

BOOL CMainFrame::BuildAppGDIPalette()
{
    ClearSystemPalette();

    int nPalSize = 256;
    LPLOGPALETTE pLP = (LPLOGPALETTE) new char[sizeof(LOGPALETTE) +
        nPalSize * sizeof(PALETTEENTRY)];
    SetupIdentityPalette(nPalSize, pLP);    // Start with speedy identity pal
    GenerateColorWash(&pLP->palPalEntry[10]);

    m_appPalette.DeleteObject();
    VERIFY(m_appPalette.CreatePalette(pLP));
    delete pLP;

    return TRUE;
}

///////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

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

void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{
    if (pFocusWnd != this)
        OnQueryNewPalette();
}

BOOL CMainFrame::OnQueryNewPalette()
{
    if (m_appPalette.m_hObject == NULL)
        return CMDIFrameWndEx::OnQueryNewPalette();

    CDC* pDC = GetDC();
    ASSERT(pDC != NULL);

    CPalette* prvPal = pDC->SelectPalette(&m_appPalette, FALSE);
    UINT nChange = pDC->RealizePalette();
    pDC->SelectPalette(prvPal, TRUE);
    pDC->RealizePalette();

    ReleaseDC(pDC);

    // Colors changed...update views
    if (nChange)
        GetApp()->UpdateAllViewsForAllDocuments(NULL);

    return TRUE;
}

///////////////////////////////////////////////////////////////////////
// pRtc points to a NULL terminated list of CRuntimeClass pointers
// specifying qualifying active views for the specified palette.

void CMainFrame::UpdatePaletteWindow(CWnd* pWnd, CRuntimeClass** pRtc, BOOL bIsOn)
{
    if (pWnd->m_hWnd != NULL)       // Handle exists if palette allowed
    {
        BOOL bIsControlBar = pWnd->IsKindOf(RUNTIME_CLASS(CBasePane));
        BOOL bVisible = ((pWnd->GetStyle() & WS_VISIBLE) != 0);

        CMDIChildWndEx* pMDIChild = (CMDIChildWndEx*)MDIGetActive();
        if (pMDIChild == NULL || pMDIChild->IsIconic())
        {
            if (!bIsControlBar && bVisible)
                pWnd->ShowWindow(SW_HIDE);
            else if (bIsControlBar && bVisible)
                ShowPane((CBasePane*)pWnd, FALSE, FALSE, FALSE);
        }
        else
        {
            CView *pView = pMDIChild->GetActiveView();
            ASSERT(pView != NULL);

            while (*pRtc != NULL)
            {
                if (pView->IsKindOf(*pRtc))
                {
                    if (bIsOn && !bVisible)
                    {
                        if (!bIsControlBar)
                            pWnd->ShowWindow(SW_SHOW);
                        else
                            ShowPane((CBasePane*)pWnd, TRUE, FALSE, FALSE);
                    }
                    else if (!bIsOn && bVisible)
                    {
                        if (!bIsControlBar)
                            pWnd->ShowWindow(SW_HIDE);
                        else
                            ShowPane((CBasePane*)pWnd, FALSE, FALSE, FALSE);
                    }
                    return;
                }
                pRtc++;
            }
            if (!bIsControlBar && bVisible)
                pWnd->ShowWindow(SW_HIDE);
            else if (bIsControlBar && bVisible)
                ShowPane((CBasePane*)pWnd, FALSE, FALSE, FALSE);
        }
    }
}

///////////////////////////////////////////////////////////////////////

BOOL CMainFrame::IsQualifyingView(CWnd* pWnd, CRuntimeClass** pRtc)
{
    if (pWnd->m_hWnd != NULL)       // Handle exists if palette allowed
    {
        CMDIChildWndEx* pMDIChild = (CMDIChildWndEx*)MDIGetActive();
        if (pMDIChild != NULL)
        {
            CView *pView = pMDIChild->GetActiveView();
            ASSERT(pView != NULL);

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
    if (IsIconic())         // No window palette processing if app minimized
        return;

    UpdatePaletteWindow(&m_wndColorPal, tblColor, m_bColorPalOn);

    if (GetCurrentDocument() == NULL)
        ShowPane(&m_wndTilePal, FALSE, FALSE, TRUE);

    if (m_wndColorPal.m_hWnd != NULL && m_wndColorPal.IsWindowVisible())
        m_wndColorPal.SendMessage(WM_IDLEUPDATECMDUI, (WPARAM)TRUE);
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
    pCmdUI->Enable(IsQualifyingView(&m_wndColorPal, tblColor));
}

/////////////////////////////////////////////////////////////////

void CMainFrame::OnWindowTilePalette()
{
    m_bTilePalOn = !m_bTilePalOn;
}

void CMainFrame::OnUpdateWindowTilePalette(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bTilePalOn);
    pCmdUI->Enable(IsQualifyingView(&m_wndTilePal, tblBrd));
}
