// LibMfc.cpp - Miscellaneous MFC Support Functions
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
#include    <WTYPES.H>

#include    "LibMfc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    LPDWORD pdwPidOrHandle = (LPDWORD)lParam;
    DWORD dwPid;
    GetWindowThreadProcessId(hwnd, &dwPid);

    if (dwPid == *pdwPidOrHandle)
    {
        *pdwPidOrHandle = (DWORD)hwnd;
        return FALSE;
    }
    return TRUE;
}

HWND FindWindowForProcessID(DWORD dwProcessID)
{
    DWORD dwProcIDorHandle = dwProcessID;
    if (EnumWindows(EnumWindowsProc, (LPARAM)&dwProcIDorHandle) != 0)
        return NULL;                // Didn't find one
    return (HWND)dwProcIDorHandle;
}

// Returns TRUE if succeeded
BOOL FindWindowForProcessIDAndBringToFront(DWORD dwProcessID)
{
    HWND hWnd = FindWindowForProcessID(dwProcessID);
    if (hWnd == NULL)
        return FALSE;
    BringWindowToTop(hWnd);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL AppendStringToEditBox(CEdit& edit, CString strAppend,
    BOOL bEnsureNewline /* = FALSE */)
{
    if (bEnsureNewline)
    {
        CString str;
        edit.GetWindowText(str);
        if (str != "" && str.GetAt(str.GetLength() - 1) != '\n')
            AppendStringToEditBox(edit, "\r\n", FALSE);
    }
    int nLen = edit.GetWindowTextLength();
    edit.SetSel(nLen, nLen);
    edit.ReplaceSel(strAppend);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL GetMaximumTextExtent(HDC hDC, LPCTSTR pszStr, int nStringLen, int nMaxWidth,
    int* pnFit)
{
    int  nStrLen = nStringLen == -1 ? lstrlen(pszStr) : nStringLen;
    SIZE size;

    // Determine the largest string that can fit in the cell...
    if (GetTextExtentExPoint(hDC, pszStr, nStrLen, nMaxWidth,
            pnFit, NULL, &size))
        return TRUE;

    // If it failed (probably due to running on Win32s) do it the hard way!
    //
    // First try entire string extent to save time.
    if (!GetTextExtentPoint(hDC, pszStr, nStrLen, &size))
        return FALSE;

    if (size.cx <= nMaxWidth)
    {
        *pnFit = nStrLen;
        return TRUE;
    }
    // Use brute force approach. Perhaps this can be made more clever when
    // the product ship data isn't looming.
    int nChar;
    for (nChar = 0; nChar < nStrLen; nChar++)
    {
        if (!GetTextExtentPoint(hDC, pszStr, nChar + 1, &size))
            return FALSE;
        if (size.cx > nMaxWidth)
            break;
    }
    ASSERT(nChar < nStrLen);        // Should always bail out before end string!
    *pnFit = nChar;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

CDisableMainWindow::CDisableMainWindow(BOOL bDisable /* = TRUE */)
{
    DisableMainWindow(bDisable);
}

CDisableMainWindow::~CDisableMainWindow()
{
    DisableMainWindow(FALSE);
}

void CDisableMainWindow::DisableMainWindow(BOOL bDisable /* = TRUE */)
{
    ASSERT(AfxGetApp() != NULL);
    ASSERT(AfxGetApp()->m_pMainWnd != NULL);

    AfxGetApp()->m_pMainWnd->EnableWindow(!bDisable);
}

///////////////////////////////////////////////////////////////////////
// This routine will simply create a unique temporary filename.

void GetTemporaryFileName(LPCTSTR lpPrefixString, CString& strTempName)
{
    TCHAR szTempName[_MAX_PATH];
    TCHAR szPath[_MAX_PATH];

    // Save to temporary path
    VERIFY(GetTempPath(sizeof(szPath)/sizeof(TCHAR), szPath) != 0);
    VERIFY(GetTempFileName(szPath, lpPrefixString, 0, szTempName) != 0);
    strTempName = szTempName;
}

///////////////////////////////////////////////////////////////////////
// TruncatedAnsiStringWithEllipses() - This function shortens a string
// to fit within a target width. The shortened string has "..." at the
// end of it.

void TruncatedAnsiStringWithEllipses(CDC* pRefDC, int nTargWidth, CString& str)
{
    CString strWrk = "..." + str;       // Ellipse is prefix for size calc
    int nFitWidth;
    VERIFY(GetMaximumTextExtent(pRefDC->m_hAttribDC, strWrk, strWrk.GetLength(),
        nTargWidth, &nFitWidth));
    ASSERT(nFitWidth > 0);
    str = str.Left(nFitWidth - 3) + "...";
}

///////////////////////////////////////////////////////////////////////
// This routine will scan the supplied menu for an entry hooked to
// a submenu. It then checks to see if the ID of the first item in
// submenu matches the specified ID. If it does, the submenu's index is
// returned. Otherwise -1 is returned.

UINT LocateSubMenuIndexOfMenuHavingStartingID(CMenu* pMenu, UINT nID)
{
    for (UINT nIndex = 0;
        pMenu->GetMenuState(nIndex, MF_BYPOSITION) != -1;
        nIndex++)
    {
        CMenu* pSubMenu = pMenu->GetSubMenu(nIndex);
        // Support two levels of checking.
        if (pSubMenu != NULL)
        {
            if (pSubMenu->GetMenuItemID(0) == nID)
                return nIndex;
            // Try the next level...
            pSubMenu = pSubMenu->GetSubMenu(0);
            if (pSubMenu != NULL && pSubMenu->GetMenuItemID(0) == nID)
                return nIndex;
        }
//      if (pSubMenu != NULL && pSubMenu->GetMenuItemID(0) == nID)
//          return nIndex;
    }
    return (UINT)-1;
}

///////////////////////////////////////////////////////////////////////
// This routine appends a list of strings to the supplied menu. The first
// string is assigned ID 'nBaseID'. The second ID is 'nBaseID' + 1 and so
// on. You can optionally provide a UINT array which specifies which
// indexes of the string array you wish to have added to the menu (i.e.,
// it allows you to specify a subset of the string array. Finally, since
// the menu could be quite large you can specify how many items are stacked
// vertically before a menu break is forced. The default break value
// is 20 menu items.

void CreateSequentialSubMenuIDs(CMenu& menu, UINT nBaseID, CStringArray& tblNames,
    CUIntArray* pTblSelections /* = NULL */, UINT nBreaksAt /* = 20 */)
{
    if (tblNames.GetSize() > 0)
    {
        int nMenuEntries = pTblSelections != NULL ? pTblSelections->GetSize() :
            tblNames.GetSize();
        for (int i = 0; i < nMenuEntries; i++)
        {
            int nNameIdx = pTblSelections != NULL ? pTblSelections->GetAt(i) : i;
            ASSERT(nNameIdx < tblNames.GetSize());
            // Break the menu every 'nBreaksAt' lines since Windows
            // won't automatically break the menu if it is
            // too tall.
            UINT nFlags = MF_ENABLED | MF_STRING |
                (i % nBreaksAt == 0 && i != 0 ? MF_MENUBARBREAK : 0);
            VERIFY(menu.AppendMenu(nFlags, nBaseID + i, tblNames.GetAt(nNameIdx)));
        }
    }
}

///////////////////////////////////////////////////////////////////////
// This routine maps key codes to scrollbar messages. The scroll
// message is sent to the supplied window. Returns FALSE if
// no key mapping was found.

BOOL TranslateKeyToScrollBarMessage(CWnd* pWnd, UINT nChar)
{
    UINT nCmd = WM_VSCROLL;
    UINT nSBCode = (UINT)-1;
    BOOL bControl = GetKeyState(VK_CONTROL) < 0;
    switch(nChar)
    {
        case VK_UP:    nSBCode = SB_LINEUP; break;
        case VK_DOWN:  nSBCode = SB_LINEDOWN; break;
        case VK_LEFT:  nSBCode = SB_LINELEFT;  nCmd = WM_HSCROLL; break;
        case VK_RIGHT: nSBCode = SB_LINERIGHT; nCmd = WM_HSCROLL; break;
        case VK_PRIOR:
            nSBCode = SB_PAGEUP; if (bControl) nCmd = WM_HSCROLL; break;
        case VK_NEXT:
            nSBCode = SB_PAGEDOWN; if (bControl) nCmd = WM_HSCROLL; break;
        case VK_HOME:
            nSBCode = SB_TOP; if (bControl) nCmd = WM_HSCROLL; break;
        case VK_END:
            nSBCode = SB_BOTTOM; if (bControl) nCmd = WM_HSCROLL; break;
        default: break;
    }
    if (nSBCode != (UINT)-1)
    {
        pWnd->SendMessage(nCmd, MAKELONG(nSBCode, 0));
        return TRUE;
    }
    return FALSE;
}

namespace {
    class CPaneContainerManagerCb : public CPaneContainerManager
    {
        DECLARE_DYNCREATE(CPaneContainerManagerCb)
    public:
        virtual BOOL Create(CWnd* pParentWnd, CPaneDivider* pDefaultSlider, CRuntimeClass* pContainerRTC = NULL);
        void AddHiddenPanesToList(CObList* plstControlBars, CObList* plstSliders);
    };

    class CPaneContainerCb : public CPaneContainer
    {
        DECLARE_DYNCREATE(CPaneContainerCb)
    public:
        virtual void Resize(CRect rect, HDWP& hdwp, BOOL bRedraw = FALSE);
    };

    class CPaneDividerCb : public CPaneDivider
    {
        DECLARE_DYNCREATE(CPaneDividerCb)
    public:
        virtual void OnShowPane(CDockablePane* pBar, BOOL bShow);
        void GetHiddenPanes(CObList& lstBars);
    };

    IMPLEMENT_DYNCREATE(CPaneContainerManagerCb, CPaneContainerManager)

    BOOL CPaneContainerManagerCb::Create(CWnd* pParentWnd, CPaneDivider* pDefaultSlider, CRuntimeClass* pContainerRTC /*= NULL*/)
    {
        ASSERT(!pContainerRTC);
        return CPaneContainerManager::Create(pParentWnd, pDefaultSlider, RUNTIME_CLASS(CPaneContainerCb));
    }

    // based on CPaneContainerManager::AddPanesToList
    void CPaneContainerManagerCb::AddHiddenPanesToList(CObList* plstControlBars, CObList* plstSliders)
    {
        ASSERT_VALID(this);
        if (plstControlBars != NULL)
        {
            for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
            {
                CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, m_lstControlBars.GetNext(pos));
                ASSERT_VALID(pWnd);

                if (!(pWnd->GetStyle() & WS_VISIBLE))
                {
                    plstControlBars->AddTail(pWnd);
                }
            }
        }

        if (plstSliders != NULL)
        {
            for (POSITION pos = m_lstSliders.GetHeadPosition(); pos != NULL;)
            {
                CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, m_lstSliders.GetNext(pos));
                ASSERT_VALID(pWnd);

                if (!(pWnd->GetStyle() & WS_VISIBLE))
                {
                    plstSliders->AddTail(pWnd);
                }
            }
        }
    }

    IMPLEMENT_DYNCREATE(CPaneContainerCb, CPaneContainer)

    void CPaneContainerCb::Resize(CRect rect, HDWP& hdwp, BOOL bRedraw /*= FALSE*/)
    {
        CPaneContainer::Resize(rect, hdwp, bRedraw);

        /* if hidden panes larger than rect, shrink them to fit rect
            (fix issue #16) */
        CPaneContainerManagerCb* mgr = DYNAMIC_DOWNCAST(CPaneContainerManagerCb, m_pContainerManager);
        ASSERT(mgr);
        CWnd* frame = mgr->GetDockSiteFrameWnd();
        // rect is client coords, but client of what?
        CRect screenRect = rect;
        // best guess:  client of docksite
        frame->ClientToScreen(screenRect);
        CObList panes;
        mgr->AddHiddenPanesToList(&panes, NULL);
        for (POSITION pos = panes.GetHeadPosition() ; pos ; )
        {
            CDockablePane* pane = DYNAMIC_DOWNCAST(CDockablePane, panes.GetNext(pos));
            ASSERT(pane && !pane->IsVisible());
            CRect paneRect;
            pane->GetWindowRect(paneRect);
            CRect newRect;
            newRect.SetRectEmpty();
            if (paneRect.Width() > rect.Width())
            {
                if (newRect.IsRectEmpty())
                {
                    newRect = paneRect;
                }
                newRect.left = screenRect.left;
                newRect.right = screenRect.right;
            }
            if (paneRect.Height() > rect.Height())
            {
                if (newRect.IsRectEmpty())
                {
                    newRect = paneRect;
                }
                newRect.top = screenRect.top;
                newRect.bottom = screenRect.bottom;
            }
            if (!newRect.IsRectEmpty())
            {
                pane->GetParent()->ScreenToClient(newRect);
                pane->MoveWindow(newRect, FALSE, hdwp);
            }
        }
    }

    IMPLEMENT_DYNCREATE(CPaneDividerCb, CPaneDivider)

    void CPaneDividerCb::OnShowPane(CDockablePane* pBar, BOOL bShow)
    {
        if (bShow)
        {
            // ensure pBar is at least as big as its minimum size
            CSize minSize;
            pBar->GetMinSize(minSize);
            CRect paneRect;
            pBar->GetWindowRect(paneRect);
            bool update = false;
            if (paneRect.Width() < minSize.cx)
            {
                paneRect.right = paneRect.left + minSize.cx;
                update = true;
            }
            if (paneRect.Height() < minSize.cy)
            {
                paneRect.bottom = paneRect.top + minSize.cy;
                update = true;
            }
            if (update)
            {
                pBar->GetParent()->ScreenToClient(paneRect);
                pBar->MoveWindow(paneRect, FALSE);
            }
        }
        CPaneDivider::OnShowPane(pBar, bShow);
    }

    void CPaneDividerCb::GetHiddenPanes(CObList& lstBars)
    {
        if (m_pContainerManager != NULL)
        {
            (DYNAMIC_DOWNCAST(CPaneContainerManagerCb, m_pContainerManager))->AddHiddenPanesToList(&lstBars, NULL);
        }
    }
}

IMPLEMENT_DYNAMIC(CMDIFrameWndExCb, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMDIFrameWndExCb, CMDIFrameWndEx)
    ON_UPDATE_COMMAND_UI(ID_WINDOW_TILE_HORZ, OnUpdateWindowTile)
    ON_UPDATE_COMMAND_UI(ID_WINDOW_TILE_VERT, OnUpdateWindowTile)
    ON_COMMAND_EX(ID_WINDOW_TILE_HORZ, OnWindowTile)
    ON_COMMAND_EX(ID_WINDOW_TILE_VERT, OnWindowTile)
END_MESSAGE_MAP()

CMDIFrameWndExCb::CMDIFrameWndExCb()
{
    CPaneDivider::m_pContainerManagerRTC = RUNTIME_CLASS(CPaneContainerManagerCb);
    CPaneDivider::m_pSliderRTC = RUNTIME_CLASS(CPaneDividerCb);
}

// KLUDGE:  dirty trick to get access to CMDIClientAreaWnd members
class CMDIFrameWndExCb::CMDIClientAreaWndCb : public CMDIClientAreaWnd
{
public:
    void OnUpdateWindowTile(CCmdUI* pCmdUI)
    {
        BOOL vert = pCmdUI->m_nID == ID_WINDOW_TILE_VERT;
        // allow switching between horz and vert split
        if (!vert && m_groupAlignment == GROUP_VERT_ALIGN ||
            vert && m_groupAlignment == GROUP_HORZ_ALIGN) {
            pCmdUI->Enable(TRUE);
            return;
        }

        // if active tab is in a multiple-tab group, allow split
        CMFCTabCtrl* activeTabWnd = FindActiveTabWnd();
        if (!activeTabWnd)
        {
            pCmdUI->Enable(FALSE);
            return;
        }
        pCmdUI->Enable(activeTabWnd->GetTabsNum() >= 2);
    }

    BOOL OnWindowTile(UINT nID)
    {
        BOOL vert = nID == ID_WINDOW_TILE_VERT;

        // no reason to show intermediate window changes
        SetRedraw(FALSE);

        // see comment in CMDIClientAreaWnd::MDITabNewGroup
        if (vert && m_groupAlignment == GROUP_HORZ_ALIGN ||
            !vert && m_groupAlignment == GROUP_VERT_ALIGN)
        {
            Unsplit();
        }

        MDITabNewGroup(vert);

        // show final state
        SetRedraw(TRUE);
        UpdateTabs(TRUE);
        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

        return TRUE;
    }

protected:
    void Unsplit()
    {
        CMFCTabCtrl* activeTabWnd = FindActiveTabWnd();
        CWnd* activeWnd = activeTabWnd ? activeTabWnd->GetActiveWnd() : NULL;

        CMFCTabCtrl* pFirstTabWnd = NULL;
        for (POSITION pos = m_lstTabbedGroups.GetHeadPosition() ; pos != NULL ; )
        {
            CMFCTabCtrl* pNextTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
            ASSERT_VALID(pNextTabWnd);
            if (!pFirstTabWnd)
            {
                pFirstTabWnd = pNextTabWnd;
            }
            else
            {
                int tabs = pNextTabWnd->GetTabsNum();
                for (int i = 0; i < tabs; ++i)
                {
                    MoveWindowToTabGroup(pNextTabWnd, pFirstTabWnd, 0);
                }
            }
        }

        if (activeWnd)
        {
            SetActiveTab(activeWnd->GetSafeHwnd());
        }
    }
};

void CMDIFrameWndExCb::OnUpdateWindowTile(CCmdUI* pCmdUI)
{
    GetMDIClient().OnUpdateWindowTile(pCmdUI);
}

BOOL CMDIFrameWndExCb::OnWindowTile(UINT nID)
{
    return GetMDIClient().OnWindowTile(nID);
}

const CMDIFrameWndExCb::CMDIClientAreaWndCb& CMDIFrameWndExCb::GetMDIClient() const
{
    return const_cast<CMDIFrameWndExCb*>(this)->GetMDIClient();
}

CMDIFrameWndExCb::CMDIClientAreaWndCb& CMDIFrameWndExCb::GetMDIClient()
{
    return static_cast<CMDIClientAreaWndCb&>(m_wndClientArea);
}
