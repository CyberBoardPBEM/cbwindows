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
#include    "Gp.h"
#include    "GdiTools.h"
#include    "FrmMain.h"
#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndExCb)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndExCb)
    ON_WM_CREATE()
    ON_UPDATE_COMMAND_UI(ID_VIEW_SNAPGRID, OnUpdateDisable)
    ON_WM_HELPINFO()
    ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
    ON_WM_CLOSE()
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_CELLNUM, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_COMPMOVE, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_ACT_ROTATE_0, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_MRKGROUP_FIRST, OnUpdateDisable)
    ON_UPDATE_COMMAND_UI(ID_ACT_TURNOVER, OnUpdateDisable)
    ON_COMMAND(IDW_MARK_PALETTE, OnToggleMarkPalette)
    ON_COMMAND(IDW_TRAY_PALETTEA, OnToggleTrayPaletteA)
    ON_COMMAND(IDW_TRAY_PALETTEB, OnToggleTrayPaletteB)
    ON_COMMAND(IDW_MESSAGE_WINDOW, OnToggleMessagePalette)
    ON_COMMAND(ID_HELP_FINDER, CMDIFrameWndEx::OnHelpFinder)
    ON_COMMAND(ID_HELP, CMDIFrameWndEx::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWndEx::OnContextHelp)
    ON_MESSAGE(WM_MESSAGEBOX, OnMessageBox)
    ON_MESSAGE(WM_DDE_EXECUTE, OnDDEExecute)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// IDs used to initialize control bars

static UINT indicators[] =
{
    ID_SEPARATOR,               // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_COMPMOVE,
    ID_INDICATOR_CELLNUM,
};

static UINT toolbars[] =
{
    IDR_GP_MAINFRAME,
    IDR_TBAR_VIEW,
    IDR_TBAR_MOVE,
    IDR_TBAR_PLAYBACK
};

/////////////////////////////////////////////////////////////////////////////

static char szSectControlBars[] = "ControlBars";

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    // TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

// BUGFIX: Fix problem in MFC7.1 (VS2003)
LRESULT CMainFrame::OnDDEExecute(WPARAM wParam, LPARAM lParam)  //TODO: DLL20200618 REMOVE THIS!!
{
    // unpack the DDE message
    UINT_PTR unused;
    HGLOBAL hData;
    VERIFY(UnpackDDElParam(WM_DDE_EXECUTE, lParam, &unused, (UINT_PTR*)&hData));

    // get the command string
    TCHAR szCommand[_MAX_PATH * 2];
    LPCTSTR lpsz = (LPCTSTR)GlobalLock(hData);
    int commandLength = lstrlen(lpsz);
    if (commandLength >= sizeof(szCommand)/sizeof(TCHAR))
    {
        // The command would be truncated. This could be a security problem
        TRACE0("Warning: Command was ignored because it was too long.\n");
        return 0;
    }
    lstrcpy(szCommand, lpsz);           // THE FIX!
    GlobalUnlock(hData);

    // acknowledge now - before attempting to execute
    ::PostMessage((HWND)wParam, WM_DDE_ACK, (WPARAM)m_hWnd,
        ReuseDDElParam(lParam, WM_DDE_EXECUTE, WM_DDE_ACK,
        (UINT)0x8000, (UINT_PTR)hData));

    // don't execute the command when the window is disabled
    if (!IsWindowEnabled())
    {
        TRACE(traceAppMsg, 0, "Warning: DDE command '%s' ignored because window is disabled.\n",
            szCommand);
        return 0;
    }

    // execute the command
    if (!AfxGetApp()->OnDDECommand(szCommand))
        TRACE(traceAppMsg, 0, "Error: failed to execute DDE command '%s'.\n", szCommand);

    return 0L;
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
        !m_wndToolBar.LoadToolBar(IDR_GP_MAINFRAME))
    {
        TRACE("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if (!m_wndTBarView.CreateEx(this, TBSTYLE_FLAT,
            CBRS_GRIPPER | WS_CHILD | WS_VISIBLE | CBRS_TOP |
            CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
            CRect(0, 0, 0, 0), IDW_TOOLBAR_VIEW) ||
        !m_wndTBarView.LoadToolBar(IDR_TBAR_VIEW))
    {
        TRACE("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if (!m_wndTBarMove.CreateEx(this, TBSTYLE_FLAT,
            CBRS_GRIPPER | WS_CHILD | WS_VISIBLE | CBRS_TOP |
            CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
            CRect(0, 0, 0, 0), IDW_TOOLBAR_MOVE) ||
        !m_wndTBarMove.LoadToolBar(IDR_TBAR_MOVE))
    {
        TRACE("Failed to create toolbar\n");
        return -1;      // fail to create
    }

// TODO:  use CMFCDropDownToolbarButton or CMFCToolBarMenuButton?
#if 1
    // KLUDGE:  CMFCDropDownToolBar doesn't add images
    if (!CMFCToolBar::AddToolBarForImageCollection(IDR_TBMENU))
    {
        TRACE("Failed to load menu images\n");
        return -1;      // fail to create
    }

    if (!m_flipToolbar.Create(this,
                                CBRS_GRIPPER | WS_CHILD | CBRS_TOP |
                                CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
                                IDR_TBMENU) ||
        !m_flipToolbar.LoadToolBar(IDR_TBMENU) ||
        !m_flipToolbar.GetSafeHwnd())
    {
        TRACE("Failed to create flip toolbar\n");
        return -1;      // fail to create
    }
    if (!m_wndTBarMove.ReplaceButton(ID_ACT_TURNOVER, CMFCDropDownToolbarButton(_T("Lorem Ipsum"), &m_flipToolbar)))
    {
        TRACE("Failed to replace flip button\n");
        return -1;      // fail to create
    }
#else
    if (!CMFCToolBar::AddToolBarForImageCollection(IDR_TBMENU))
    {
        TRACE("Failed to load menu images\n");
        return -1;      // fail to create
    }

    CMenu bar;
    bool rc = bar.LoadMenu(IDR_MENU_PLAYER_POPUPS);
    if (!rc)
    {
        TRACE("Failed to load popup menu\n");
        return -1;      // fail to create
    }
    CMenu& popup = CheckedDeref(bar.GetSubMenu(MENU_ACT_TURNOVER));
    if (!popup.m_hMenu)
    {
        TRACE("Failed to find flip menu\n");
        return -1;      // fail to create
    }
    if (!m_wndTBarMove.ReplaceButton(ID_ACT_TURNOVER, CMFCToolBarMenuButton(ID_ACT_TURNOVER, popup.m_hMenu, -1)))
    {
        TRACE("Failed to replace flip button\n");
        return -1;      // fail to create
    }
#endif

    if (!m_wndTBarPlay.CreateEx(this, TBSTYLE_FLAT,
            CBRS_GRIPPER | WS_CHILD | WS_VISIBLE | CBRS_TOP |
            CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
            CRect(0, 0, 0, 0), IDW_TOOLBAR_PLAY) ||
        !m_wndTBarPlay.LoadToolBar(IDR_TBAR_PLAYBACK))
    {
        TRACE("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }
    m_wndStatusBar.SetPaneStyle(0, SBPS_STRETCH);

    CString str;
    str.LoadString(IDS_TRAYA_TITLE);
    if (!m_wndTrayPalA.Create((LPCTSTR)str, this, CRect(0, 0, 200, 1000), TRUE, IDW_TRAY_PALETTEA,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create tray A palette dock window\n");
        return -1;      // fail to create
    }

    str.LoadString(IDS_TRAYB_TITLE);
    if (!m_wndTrayPalB.Create((LPCTSTR)str, this, CRect(0, 0, 200, 1000), TRUE, IDW_TRAY_PALETTEB,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create tray B palette dock window\n");
        return -1;      // fail to create
    }

    str.LoadString(IDS_PAL_MARKERS);
    if (!m_wndMarkPal.Create((LPCTSTR)str, this, CRect(0, 0, 200, 1000), TRUE, IDW_MARK_PALETTE,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create marker palette dock window\n");
        return -1;      // fail to create
    }

    str.LoadString(IDS_MESSAGE_WND);
    if (!m_wndMessage.Create((LPCTSTR)str, this, CRect(0, 0, 300, 80), TRUE, IDW_MESSAGE_WINDOW,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
    {
        TRACE0("Failed to create message window\n");
        return -1;      // fail to create
    }

    CString strBarName;
    strBarName.LoadString(IDS_BARNAME_STANDARD);
    m_wndToolBar.SetWindowText(strBarName);

    strBarName.LoadString(IDS_BARNAME_VIEW);
    m_wndTBarView.SetWindowText(strBarName);

    strBarName.LoadString(IDS_BARNAME_MOVE);
    m_wndTBarMove.SetWindowText(strBarName);

    strBarName.LoadString(IDS_BARNAME_PLAYBACK);
    m_wndTBarPlay.SetWindowText(strBarName);

    m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    m_wndTBarView.EnableDocking(CBRS_ALIGN_ANY);
    m_wndTBarMove.EnableDocking(CBRS_ALIGN_ANY);
    m_wndTBarPlay.EnableDocking(CBRS_ALIGN_ANY);
    m_wndTrayPalA.EnableDocking(CBRS_ALIGN_ANY);
    m_wndTrayPalB.EnableDocking(CBRS_ALIGN_ANY);
    m_wndMarkPal.EnableDocking(CBRS_ALIGN_ANY);
    m_wndMessage.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);

    DockPane(&m_wndMenuBar);

    // Setup topmost default tool bar layout.
    DockPane(&m_wndTBarPlay);
    DockPaneLeftOf(&m_wndTBarMove, &m_wndTBarPlay);
    DockPaneLeftOf(&m_wndTBarView, &m_wndTBarMove);
    DockPaneLeftOf(&m_wndToolBar, &m_wndTBarView);

    DockPane(&m_wndTrayPalA, AFX_IDW_DOCKBAR_RIGHT);
    m_wndTrayPalB.DockToWindow(&m_wndTrayPalA, CBRS_ALIGN_BOTTOM);
    m_wndMarkPal.DockToWindow(&m_wndTrayPalB, CBRS_ALIGN_BOTTOM);

    DockPane(&m_wndMessage, AFX_IDW_DOCKBAR_BOTTOM);

    //@@@@@ LoadBarState(szSectControlBars);
    //@@@@@ m_wndMDITabWindow.Install(this);

    return 0;
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
    CWnd *pParentWnd, CCreateContext *pContext)
{
    if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
        return FALSE;
    return TRUE;
}

void CMainFrame::OnClose()
{
    SaveBarState(szSectControlBars);

    // Save frame window size and position.
//@@@@@@    m_wndPosition.SaveWindowPos(this);

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

///////////////////////////////////////////////////////////////////////

void CMainFrame::OnIdle()
{
    if (GetCurrentDocument() == NULL)
    {
    	ShowPalettePanes(FALSE);
    }
}

void CMainFrame::ShowPalettePanes(BOOL bShow)
{
    ShowPane(&m_wndTrayPalA, bShow, FALSE, TRUE);
    ShowPane(&m_wndTrayPalB, bShow, FALSE, TRUE);
    ShowPane(&m_wndMarkPal, bShow, FALSE, TRUE);
    ShowPane(&m_wndMessage, bShow, FALSE, TRUE);
}

///////////////////////////////////////////////////////////////////////

void CMainFrame::UpdatePaletteWindow(CWnd* pWnd, BOOL bIsOn)
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
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void GenerateColorWash(LPPALETTEENTRY pPE);   // See COLOR.CPP //

BOOL CMainFrame::BuildAppGDIPalette()
{
    ClearSystemPalette();

    uint16_t nPalSize = uint16_t(256);
    std::vector<char> v((sizeof(LOGPALETTE) +
        value_preserving_cast<size_t>(nPalSize * sizeof(PALETTEENTRY))));
    LPLOGPALETTE pLP = reinterpret_cast<LPLOGPALETTE>(v.data());
    // Start with speedy identity pal
    SetupIdentityPalette(nPalSize, pLP);
    GenerateColorWash(&pLP->palPalEntry[10]);

    m_appPalette.DeleteObject();
    m_appPalette.CreatePalette(pLP);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnToggleMarkPalette()
{
    SendMessage(WM_COMMAND, ID_VIEW_MARKERPAL);
}

void CMainFrame::OnToggleTrayPaletteA()
{
    SendMessage(WM_COMMAND, ID_VIEW_TRAYA);
}

void CMainFrame::OnToggleTrayPaletteB()
{
    SendMessage(WM_COMMAND, ID_VIEW_TRAYB);
}

void CMainFrame::OnToggleMessagePalette()
{
    SendMessage(WM_COMMAND, ID_PBCK_READMESSAGE);
}

// Default handler for toolbar buttons. Disables and unchecks
// the button.
void CMainFrame::OnUpdateDisable(CCmdUI* pCmdUI)
{
    if (pCmdUI->m_pSubMenu != NULL)
    {
        // Need to handle menu that the submenu is connected to.
        pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex,
            MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
    }

    bool clearCheck = true;
    if (pCmdUI->m_pOther &&
        pCmdUI->m_pOther->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
    {
        CMFCToolBar* pToolBar = static_cast<CMFCToolBar*>(pCmdUI->m_pOther);
        clearCheck = bool(pToolBar->GetButtonStyle(pCmdUI->m_nIndex) & /*TBBS_CHECKBOX*/TBSTYLE_CHECK);
    }
    if (clearCheck)
    {
        pCmdUI->SetCheck(0);
    }

    pCmdUI->Enable(0);
}

// Handles deferred (via PostMessage) messages that need to be shown
// to the user. See GP.H for message details.
LRESULT CMainFrame::OnMessageBox(WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
        case WMB_MSGID:
            AfxMessageBox((UINT)lParam, MB_OK);
            break;
        case WMB_PTR_STATIC:
            AfxMessageBox((LPCTSTR)lParam, MB_OK);
            break;
        case WMB_PTR_NEW:
            AfxMessageBox((LPCTSTR)lParam, MB_OK);
            delete (char*)lParam;
            break;
        case WMB_PTR_CSTRING:
            AfxMessageBox(*((CString*)lParam), MB_OK);
            delete (CString*)lParam;
            break;
        default:
            ASSERT(FALSE);      // SHOULD NEVER HAPPEN!
    }
    return (LRESULT)0;
}

