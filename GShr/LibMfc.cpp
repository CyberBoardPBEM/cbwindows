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
    uintptr_t* pdwPidOrHandle = reinterpret_cast<uintptr_t*>(lParam);
    DWORD dwPid;
    GetWindowThreadProcessId(hwnd, &dwPid);

    if (dwPid == *pdwPidOrHandle)
    {
        *pdwPidOrHandle = reinterpret_cast<uintptr_t>(hwnd);
        return FALSE;
    }
    return TRUE;
}

HWND FindWindowForProcessID(DWORD dwProcessID)
{
    uintptr_t dwProcIDorHandle = dwProcessID;
    if (EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&dwProcIDorHandle)) != 0)
        return NULL;                // Didn't find one
    return reinterpret_cast<HWND>(dwProcIDorHandle);
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
        intptr_t nMenuEntries = pTblSelections != NULL ? pTblSelections->GetSize() :
            tblNames.GetSize();
        for (intptr_t i = 0; i < nMenuEntries; i++)
        {
            intptr_t nNameIdx = pTblSelections != NULL ? value_preserving_cast<intptr_t>(pTblSelections->GetAt(i)) : i;
            ASSERT(nNameIdx < tblNames.GetSize());
            // Break the menu every 'nBreaksAt' lines since Windows
            // won't automatically break the menu if it is
            // too tall.
            UINT nFlags = value_preserving_cast<UINT>(MF_ENABLED | MF_STRING |
                (i % value_preserving_cast<intptr_t>(nBreaksAt) == 0 && i != 0 ? MF_MENUBARBREAK : 0));
            VERIFY(menu.AppendMenu(nFlags, nBaseID + value_preserving_cast<uintptr_t>(i), tblNames.GetAt(nNameIdx)));
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
        pWnd->SendMessage(nCmd, MAKELONG(uint16_t(nSBCode), uint16_t(0)));
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

CB::string CB::string::LoadString(UINT nID)
{
    CString temp;
    if (!temp.LoadString(nID))
    {
        ASSERT(!"unknown id");
        AfxThrowResourceException();
    }
    return temp;
}

CB::string CB::string::LoadString(int nID)
{
    return LoadString(value_preserving_cast<UINT>(nID));
}

CB::string CB::string::GetModuleFileName(HMODULE hModule)
{
    CString temp;
    for ( ; ; )
    {
        CB::string::value_type* ptr = temp.GetBufferSetLength(temp.GetLength() + MAX_PATH);
        DWORD rc = ::GetModuleFileName(hModule, ptr, temp.GetLength());
        if (!rc)
        {
            AfxThrowResourceException();
        }
        else if (rc < value_preserving_cast<DWORD>(temp.GetLength()))
        {
            return ptr;
        }
    }
}

CB::string CB::string::GetWindowText(const CWnd& wnd)
{
    CString temp;
    wnd.GetWindowText(temp);
    return temp;
}

CB::string CB::string::GetText(const CListBox& wnd, int nIndex)
{
    CString temp;
    wnd.GetText(nIndex, temp);
    return temp;
}

CB::string CB::string::GetLBText(const CComboBox& wnd, int nIndex)
{
    CString temp;
    wnd.GetLBText(nIndex, temp);
    return temp;
}

CB::string CB::string::GetMenuString(const CMenu& menu, UINT nIDItem, UINT nFlags)
{
    CString temp;
    if (!menu.GetMenuString(nIDItem, temp, nFlags))
    {
        AfxThrowResourceException();
    }
    return temp;
}

CB::string CB::string::GetDocString(const CDocTemplate& docTempl, CDocTemplate::DocStringIndex index)
{
    CString temp;
    if (!docTempl.GetDocString(temp, index))
    {
        AfxThrowResourceException();
    }
    return temp;
}

std::unique_ptr<CB::string> CB::string::DoPromptFileName(CWinApp& app, UINT nIDSTitle,
    DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate)
{
    CString temp;
    if (!app.DoPromptFileName(temp, nIDSTitle, lFlags, bOpenFileDialog, pTemplate))
    {
        return nullptr;
    }
    return std::make_unique<CB::string>(temp);
}

CB::string& CB::string::operator=(const string& other)
{
    wide.reset();
    cp1252 = other.cp1252;
    return *this;
}

CB::string::~string()
{
    static size_t strs;
    static size_t wstrs;
    static class Report
    {
    public:
        ~Report()
        {
            TRACE("strs %zu, wstrs %zu\n", strs, wstrs);
        }
    } report;
    ++strs;
    if (wide)
    {
        ++wstrs;
    }
}

CB::string::string(std::wstring_view s)
{
    // TODO:  utf-8 conversion
    cp1252.resize(s.size());
    for (size_t i = size_t(0) ; i < cp1252.size() ; ++i)
    {
        cp1252[i] = static_cast<char>(value_preserving_cast<uint8_t>(s[i]));
    }
}

const std::wstring& CB::string::std_wstr() const
{
    if (!wide)
    {
        // TODO:  utf-8 conversion
        wide.reset(new std::wstring);
        wide->resize(cp1252.size());
        for (size_t i = size_t(0) ; i < cp1252.size() ; ++i)
        {
            (*wide)[i] = static_cast<uint8_t>(cp1252[i]);
        }
    }
    return *wide;
}

void DDX_Check(CDataExchange* pDX, int nIDC, bool& value)
{
    BOOL temp = value;
    DDX_Check(pDX, nIDC, temp);
    value = temp;
}

void DDX_Text(CDataExchange* pDX, int nIDC, std::string& value)
{
    CString temp = value.c_str();
    DDX_Text(pDX, nIDC, temp);
    value = temp;
}

void DDV_MaxChars(CDataExchange* pDX, std::string const& value, int nChars)
{
    CString temp = value.c_str();
    DDV_MaxChars(pDX, temp, nChars);
}

void AfxFormatString1(std::string& rString, UINT nIDS, LPCTSTR lpsz1)
{
    CString temp;
    AfxFormatString1(temp, nIDS, lpsz1);
    rString = temp;
}

// CB currently only uses cp1252 strings
/* CB::string will be replacing CString,
    so use (ANSI) CString file format */
namespace {
    void WriteStringLength(CArchive& ar, size_t size)
    {
        if (size < size_t(0xff))
        {
            ar << value_preserving_cast<uint8_t>(size);
            return;
        }
        ar << uint8_t(0xff);
        if (size < size_t(0xffff))
        {
            ar << value_preserving_cast<uint16_t>(size);
            return;
        }
        ASSERT(!"untested code");
        ar << uint8_t(0xff);
        ar << uint16_t(0xffff);
        if (size < size_t(0xffffffff))
        {
            ar << value_preserving_cast<uint32_t>(size);
            return;
        }
        ASSERT(!"untested code");
        ar << uint8_t(0xff);
        ar << uint16_t(0xffff);
        ar << uint32_t(0xffffffff);
        ar << uint64_t(size);
    }

    size_t ReadStringLength(CArchive& ar)
    {
        {
            uint8_t len8;
            ar >> len8;
            if (len8 < 0xff)
            {
                return len8;
            }
        }
        {
            uint16_t len16;
            ar >> len16;
            if (len16 < 0xffff)
            {
                return len16;
            }
        }
        ASSERT(!"untested code");
        {
            uint32_t len32;
            ar >> len32;
            if (len32 < 0xffffffff)
            {
                return len32;
            }
        }
        ASSERT(!"untested code");
        {
            uint64_t len64;
            ar >> len64;
            if (len64 > SIZE_T_MAX)
            {
                AfxThrowArchiveException(CArchiveException::genericException);
            }
            return value_preserving_cast<size_t>(len64);
        }
    }
}

void CB::string::Serialize(CArchive& ar) const
{
#if 0
    CString cs = cp1252.c_str();
    ar << cs;
#else
    WriteStringLength(ar, cp1252.size());
    ar.Write(cp1252.c_str(), value_preserving_cast<UINT>(cp1252.size()));
#endif
}

void CB::string::Serialize(CArchive& ar)
{
    wide.reset();
#if 0
    CString cs;
    ar >> cs;
    cp1252 = cs;
#else
    size_t size = ReadStringLength(ar);
    if (size > cp1252.max_size())
    {
        AfxThrowArchiveException(CArchiveException::genericException);
    }
    cp1252.resize(size);
    UINT rc = ar.Read(&cp1252[0], value_preserving_cast<UINT>(size));
    if (rc != size)
    {
        AfxThrowArchiveException(CArchiveException::genericException);
    }
#endif
}
