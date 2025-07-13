// LibMfc.cpp - Miscellaneous MFC Support Functions
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
#include    <WTYPES.H>

#include    <map>

#include    "LibMfc.h"

#if defined(GPLAY)
    #include "../GP/Resource.h"
#else
    #include "../GM/Resource.h"
    #include "../GM/PalColor.h"
#endif

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

BOOL AppendStringToEditBox(wxTextCtrl& edit, const CB::string& strAppend,
    BOOL bEnsureNewline /* = FALSE */)
{
    if (bEnsureNewline)
    {
        CB::string str = edit.GetValue();
        if (!str.empty() && str[str.a_size() - size_t(1)] != '\n')
            AppendStringToEditBox(edit, L"\n", FALSE);
    }
    edit.AppendText(strAppend);
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

void CreateSequentialSubMenuIDs(CMenu& menu, UINT nBaseID, const std::vector<CB::string>& tblNames,
    const std::vector<size_t>* pTblSelections /* = NULL */, UINT nBreaksAt /* = 20 */)
{
    if (!tblNames.empty())
    {
        size_t nMenuEntries = pTblSelections != NULL ? pTblSelections->size() :
            tblNames.size();
        for (size_t i = size_t(0) ; i < nMenuEntries ; ++i)
        {
            size_t nNameIdx = pTblSelections != NULL ? (*pTblSelections)[i] : i;
            ASSERT(nNameIdx < tblNames.size());
            // Break the menu every 'nBreaksAt' lines since Windows
            // won't automatically break the menu if it is
            // too tall.
            UINT nFlags = value_preserving_cast<UINT>(MF_ENABLED | MF_STRING |
                (i % nBreaksAt == size_t(0) && i != size_t(0) ? MF_MENUBARBREAK : 0));
            VERIFY(menu.AppendMenu(nFlags, nBaseID + i, tblNames[nNameIdx]));
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
    std::vector<CB::string::value_type> temp;
    for ( ; ; )
    {
        temp.resize(temp.size() + size_t(MAX_PATH));
        CB::string::value_type* ptr = temp.data();
        DWORD rc = ::GetModuleFileName(hModule, ptr, value_preserving_cast<DWORD>(temp.size()));
        if (!rc)
        {
            AfxThrowResourceException();
        }
        else if (rc < value_preserving_cast<DWORD>(temp.size()))
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
    wxstr.reset();
    stdwide.reset();
    cp1252 = other.cp1252;
    return *this;
}

CB::string::~string()
{
    static size_t strs;
    static size_t wxstrs;
    static size_t stdwides;
    static class Report
    {
    public:
        ~Report()
        {
            /* WARNING:  don't use CPP20_TRACE here (it creates
                more CB::string instances) */
            TRACE("strs %zu, wxstrs %zu, stdwides %zu\n", strs, wxstrs, stdwides);
        }
    } report;
    ++strs;
    if (wxstr)
    {
        ++wxstrs;
    }
    if (stdwide)
    {
        ++stdwides;
    }
}

CB::string::string(const wxString& s)
{
    wxString temp = wxTextBuffer::Translate(s, wxTextBuffer::typeDefault);
    *this = temp.ToStdWstring();
}

CB::string::string(std::wstring_view s)
{
    wxCharBuffer buf = wxConvLibc.cWC2MB(s.data(), s.size(), nullptr);
    if (!s.empty() && buf.length() == 0)
    {
        AfxThrowInvalidArgException();
    }
    cp1252 = std::string(buf, buf.length());
}

const wxString& CB::string::wx_str() const
{
    if (!wxstr)
    {
        wxstr.reset(new wxString(cp1252.data(), cp1252.size()));
        *wxstr = wxTextBuffer::Translate(*wxstr, wxTextFileType_Unix);
    }
    return *wxstr;
}

const std::wstring& CB::string::std_wstr() const
{
    if (!stdwide)
    {
        wxString temp = wx_str();
        temp = wxTextBuffer::Translate(temp, wxTextBuffer::typeDefault);
        stdwide.reset(new std::wstring(temp.ToStdWstring()));
    }
    return *stdwide;
}

void DDX_Check(CDataExchange* pDX, int nIDC, bool& value)
{
    BOOL temp = value;
    DDX_Check(pDX, nIDC, temp);
    value = temp;
}

void DDX_Text(CDataExchange* pDX, int nIDC, CB::string& value)
{
    CString temp = value;
    DDX_Text(pDX, nIDC, temp);
    value = temp;
}

void DDV_MaxChars(CDataExchange* pDX, const CB::string& value, int nChars)
{
    CString temp = value;
    DDV_MaxChars(pDX, temp, nChars);
}

CB::string AfxFormatString1(UINT nIDS, const CB::string& lpsz1)
{
    CString temp;
    AfxFormatString1(temp, nIDS, lpsz1);
    return temp;
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
    wxstr.reset();
    stdwide.reset();
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

CB::wxNativeContainerWindowMixin::operator const wxNativeContainerWindow*() const
{
    if (!mfcWnd->m_hWnd)
    {
        /* N.B.:  wx deletes this when HWND is destroyed,
                    BUT does not tell us! */
        wxASSERT(wxWnd || !"do not call before creating HWND");
        wxWnd = nullptr;
    }
    else if (!wxWnd)
    {
        wxWnd = new wxNativeContainerWindow(mfcWnd->m_hWnd);

        // N.B.:  this is a dirty hack
        wxNativeContainerWindowMixin* ncThis = const_cast<wxNativeContainerWindowMixin*>(this);
        wxWindow& rThis = const_cast<wxNativeContainerWindowMixin&>(*ncThis);

        // fill in wx children list
        for (CWnd* mfcChild = mfcWnd->GetWindow(GW_CHILD) ;
            mfcChild ;
            mfcChild = mfcChild->GetWindow(GW_HWNDNEXT))
        {
            wxWindow* wxChild = CB::GetWxWindow(*mfcChild);
            if (wxChild)
            {
                rThis.AddChild(wxChild);
                wxASSERT(wxChild->GetParent() == &rThis);
            }
        }

        /* in some cases, wx Layout() won't do anything,
            so we may need to resize child ourselves */
        rThis.Bind(wxEVT_SIZE,
                    [ncThis](wxSizeEvent& event)
                    {
                        if (!ncThis->mfcWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndExCb)))
                        {
                            wxWindowList& children = ncThis->wxWnd->GetChildren();
                            if (children.size() == size_t(1) &&
                                children.front()->IsTopLevel())
                            {
                                wxSize size = ncThis->wxWnd->GetClientSize();
                                wxWindow& child = CheckedDeref(children.front());
                                child.SetSize(0, 0, size.x, size.y);
                            }
                        }
                        else
                        {
                            /* wx args don't match WM_SIZE, but it looks
                                like specific args aren't critical */
                            CMDIFrameWndExCb& mfcMdiWnd = static_cast<CMDIFrameWndExCb&>(*ncThis->mfcWnd);
                            mfcMdiWnd.OnSize(0, 0, 0);
                        }
                        event.Skip();
                    }
        );
    }
    return &*wxWnd;
}

/* if mfcWnd has
    wxNativeContainerWindowMixin, return it */
wxWindow* CB::GetWxWindow(CWnd& mfcWnd)
{
    CB::wxNativeContainerWindowMixin* mixin = dynamic_cast<CB::wxNativeContainerWindowMixin*>(&mfcWnd);
    return mixin ? *mixin : nullptr;
}

/* if mfcWnd or one of its descendants has
    wxNativeContainerWindowMixin, return it */
wxWindow* CB::FindWxWindow(CWnd& mfcWnd)
{
    wxWindow* wxWnd = GetWxWindow(mfcWnd);
    if (wxWnd)
    {
        return wxWnd;
    }
    for (CWnd* child = mfcWnd.GetWindow(GW_CHILD) ;
        child ;
        child = child->GetWindow(GW_HWNDNEXT))
    {
        wxWindow* test = FindWxWindow(*child);
        if (test)
        {
            if (!wxWnd)
            {
                wxWnd = test;
            }
            else
            {
                wxASSERT(!"conflicting wxWindow conversion");
                return nullptr;
            }
        }
    }
    return wxWnd;
}

// emulate CWnd::SendMessageToDescendants()
void CB::SendEventToDescendants(wxWindow& wnd, wxEvent& event, bool deep /*= true*/)
{
    wxWindowList& children = wnd.GetChildren();
    for (wxWindowList::iterator i = children.begin() ; i != children.end() ; ++i)
    {
        (*i)->GetEventHandler()->ProcessEventLocally(event);
        if (deep)
        {
            SendEventToDescendants(**i, event, true);
        }
    }
}

#if 0
const CWnd* CB::ToCWnd(const wxWindow& w)
{
    const wxNativeContainerWindow* ncw = dynamic_cast<const wxNativeContainerWindow*>(&w);
    if (!ncw)
    {
        return nullptr;
    }
    WXHWND wxhwnd = ncw->GetHWND();
    HWND hwnd = reinterpret_cast<HWND>(wxhwnd);
    return CWnd::FromHandlePermanent(hwnd);
}


// MFC if possible, wx otherwise
const std::type_info& CB::GetPublicTypeid(const wxWindow& w)
{
    const CWnd* mfcWnd = ToCWnd(w);
    return mfcWnd ? typeid(*mfcWnd) : typeid(w);
}
#endif

void CB::wxView::OnDraw(wxDC * dc)
{
    CPP20_TRACE("{}({})\n", __func__, *this);
    wxASSERT(!"not impl");
}

// for CB, forward events to window here
bool CB::wxView::TryBefore(wxEvent& event)
{
    /* wx expects TryBefore() to be checked before the event
        handlers specific to the current class, and I think
        the wxWindow handlers are equivalent to what wx would
        expect to be the wxView handlers, so check them after
        base class TryBefore() */
    if (::wxView::TryBefore(event))
    {
        return true;
    }

    /* KLUDGE:  there is time during doc setup where
                wxGbxProjView exists, but can't create
                corresponding window because the doc
                isn't ready yet */
    wxWindowList& children = GetFrame()->GetChildren();
    if (children.empty())
    {
        return false;
    }

    /* only process wxFocusEvent for setting focus to
        view's frame or descendants */
    if (event.GetEventType() == wxEVT_KILL_FOCUS)
    {
        return false;
    }
    else
    {
        wxFocusEvent* fe = dynamic_cast<wxFocusEvent*>(&event);
        if (fe)
        {
            wxASSERT(fe->GetEventType() == wxEVT_SET_FOCUS &&
                fe->GetEventObject() &&
                dynamic_cast<wxWindow*>(fe->GetEventObject()));
            if (!GetFrame()->IsDescendant(static_cast<wxWindow*>(fe->GetEventObject())))
            {
                return false;
            }
        }
    }

    // only process wxChildFocusEvent for frame's (strict) descendants
    wxChildFocusEvent* cfe = dynamic_cast<wxChildFocusEvent*>(&event);
    if (cfe)
    {
        if (GetFrame() == cfe->GetWindow() ||
            !GetFrame()->IsDescendant(cfe->GetWindow()))
        {
            return false;
        }
    }

    if (dynamic_cast<wxWindowCreateEvent*>(&event))
    {
        /* KLUDGE:  passing this to window causes
            problems because vwbitedit isn't ready,
            and I don't know of a need for this,
            so drop it */
        return false;
    }

    // view shouldn't process a different window's mouse events
    if (dynamic_cast<wxMouseEvent*>(&event) &&
        event.GetEventObject() != &GetWindow())
    {
        return false;
    }

    return GetWindow().ProcessWindowEventLocally(event);
}

void CB::wxView::FileHistoryAddMenu()
{
    wxFrame& frame = dynamic_cast<wxFrame&>(CheckedDeref(GetFrame()));
    wxMenuBar& menubar = CheckedDeref(frame.GetMenuBar());
    wxMenu& menuFile = CheckedDeref(menubar.GetMenu(size_t(0)));
    wxDocManager& docMgr = CheckedDeref(wxDocManager::GetDocumentManager());
    docMgr.FileHistoryUseMenu(&menuFile);
    docMgr.FileHistoryAddFilesToMenu(&menuFile);
    /*KLUDGE:  MRU menu items don't always respect
                wxFileHistoryMenuPathStyle without this */
    wxFileHistory& fileHist = CheckedDeref(docMgr.GetFileHistory());
    wxFileHistoryMenuPathStyle style = fileHist.GetMenuPathStyle();
    wxFileHistoryMenuPathStyle tempStyle = style == wxFH_PATH_SHOW_IF_DIFFERENT ?
                                                wxFH_PATH_SHOW_NEVER
                                            :
                                                wxFH_PATH_SHOW_IF_DIFFERENT;
    fileHist.SetMenuPathStyle(tempStyle);
    fileHist.SetMenuPathStyle(style);
}

void CB::wxView::FileHistoryRemoveMenu()
{
    wxFrame& frame = dynamic_cast<wxFrame&>(CheckedDeref(GetFrame()));
    wxMenuBar& menubar = CheckedDeref(frame.GetMenuBar());
    wxMenu& menuFile = CheckedDeref(menubar.GetMenu(size_t(0)));
    wxDocManager& docMgr = CheckedDeref(wxDocManager::GetDocumentManager());
    docMgr.FileHistoryRemoveMenu(&menuFile);
}

CB::ToolTip::~ToolTip()
{
    Enable(false);

    // can't use ++it because Delete invalidates it
    for (auto it = toolInfos.begin() ;
        it != toolInfos.end() ;
        it = toolInfos.begin())
    {
        if (it->rect)
        {
            Delete(it->wnd, *it->rect);
        }
        else
        {
            Delete(it->wnd);
        }
    }
}

void CB::ToolTip::Enable(bool b)
{
    enabled = b;
    if (enabled)
    {
        if (!toolInfos.empty())
        {
            wxWindow& wnd = toolInfos.front().wnd.get();
            if (delayMs < 0)
            {
                delayMs = wxSystemSettingsNative::GetMetric(wxSYS_DCLICK_MSEC, &wnd);
            }
            if (reshowMs < 0)
            {
                reshowMs = wxSystemSettingsNative::GetMetric(wxSYS_DCLICK_MSEC, &wnd) / 5;
            }
            if (autopopMs < 0)
            {
                autopopMs = wxSystemSettingsNative::GetMetric(wxSYS_DCLICK_MSEC, &wnd) * 10;
            }
            /* TODO:  do we need to synthesize a mouse move in
                        case mouse is already in a tool? */
        }
    }
    else
    {
        CloseTipWindow();
        state = sNoTool;
        Stop();
    }
}

// set maximum width for the new tooltips: -1 disables wrapping
void CB::ToolTip::SetMaxWidth(int width)
{
    maxWidth = width;
}

void CB::ToolTip::Add(wxWindow& wnd, std::optional<wxRect>&& rect,
                            wxString&& tip, Flags flags)
{
    wxASSERT(!(flags & TRACK) || !"TRACK not implemented");
    // center shouldn't move, track should move
    wxASSERT((flags & (CENTER | TRACK)) != (CENTER | TRACK));

    auto it = std::find_if(toolInfos.begin(), toolInfos.end(),
                            [&wnd](const ToolInfo& ti)
                            {
                                return &ti.wnd.get() == &wnd;
                            });
    wxASSERT(it == toolInfos.end() ||
                rect && it->rect ||
                "full window only permitted when using one tool");

    // if first tool on this wnd, bind move handler
    if (it == toolInfos.end())
    {
        wnd.Bind(wxEVT_MOTION, &ToolTip::OnMouseMove, this);
    }
    toolInfos.emplace_back(wnd, std::move(rect), std::move(tip), flags);
    Enable(enabled);
}

void CB::ToolTip::Delete(wxWindow& wnd, std::optional<wxRect> rect)
{
    auto it = std::find_if(toolInfos.begin(), toolInfos.end(),
                            [&wnd, &rect](const ToolInfo& ti)
                            {
                                return &ti.wnd.get() == &wnd &&
                                        ti.rect == rect;
                            });
    wxASSERT(it != toolInfos.end());

    if (it == tipTool)
    {
        CloseTipWindow();
    }
    toolInfos.erase(it);

    // if no more tools on this wnd, unbind move handler
    it = std::find_if(toolInfos.begin(), toolInfos.end(),
                        [&wnd](const ToolInfo& ti)
                        {
                            return &ti.wnd.get() == &wnd;
                        });
    if (it == toolInfos.end())
    {
        wnd.Unbind(wxEVT_MOTION, &ToolTip::OnMouseMove, this);
    }
}

// wxTimer
void CB::ToolTip::Notify()
{
    switch (state)
    {
        case sDisplayWait:
        {
            wxASSERT(tipTool != toolInfos.end());
            wxRect screenRect;
            if (tipTool->rect)
            {
                screenRect = wxRect(tipTool->wnd.get().ClientToScreen(tipTool->rect->GetTopLeft()),
                    tipTool->rect->GetSize());
            }
            else
            {
                screenRect = tipTool->wnd.get().GetScreenRect();
            }
            tipWindow = new wxTipWindow(&tipTool->wnd.get(),
                tipTool->tip, maxWidth,
                &tipWindow,
                &screenRect);
            if (tipTool->flags & CENTER)
            {
                wxCoord center = screenRect.GetLeft() + screenRect.GetWidth() / 2;
                wxCoord left = center - tipWindow->GetRect().GetWidth() / 2;
                tipWindow->Move(left, screenRect.GetBottom() + 1);
            }
            Start(autopopMs);
            state = sDisplay;
            break;
        }
        case sDisplay:
        {
            // autopop
            // don't re-show tooltip until it's the new tip
            auto temp = tipTool;
            CloseTipWindow();
            tipTool = temp;
            Stop();
            break;
        }
        default:
            wxASSERT(!"why is timer running?");
    }
}

void CB::ToolTip::OnMouseMove(wxMouseEvent& event)
{
    event.Skip();
    if (!enabled)
    {
        return;
    }

    wxASSERT(wxDynamicCast(event.GetEventObject(), wxWindow));
    wxWindow& wnd = *static_cast<wxWindow*>(event.GetEventObject());
    const wxPoint& pt = event.GetPosition();

    auto it = std::find_if(toolInfos.begin(), toolInfos.end(),
                            [&wnd, &pt](const ToolInfo& ti) {
                                return &ti.wnd.get() == &wnd &&
                                        (!ti.rect || ti.rect->Contains(pt));
                            });

    if (it != toolInfos.end())
    {
        switch (state)
        {
            case sNoTool:
                wxASSERT(tipTool == toolInfos.end());
                if (StartOnce(delayMs))
                {
                    state = sDisplayWait;
                }
                else
                {
                    wxASSERT(!"start timer error");
                }
                break;
            case sDisplayWait:
                if (it != tipTool)
                {
                    // restart delay
                    if (!StartOnce(delayMs))
                    {
                        wxASSERT(!"start timer error");
                    }
                }
                // else do nothing
                break;
            case sDisplay:
                if (it != tipTool)
                {
                    CloseTipWindow();
                    if (StartOnce(reshowMs))
                    {
                        state = sDisplayWait;
                    }
                    else
                    {
                        wxASSERT(!"start timer error");
                        state = sDisplay;
                    }
                }
                // else do nothing
                break;
        }
    }
    else
    {
        CloseTipWindow();
        state = sNoTool;
        Stop();
    }
    tipTool = it;
}

void CB::ToolTip::CloseTipWindow()
{
    if (tipWindow)
    {
        tipWindow->Close();
        wxASSERT(!tipWindow);
    }
    tipTool = toolInfos.end();
}

CB::ToolTip::ToolInfo::ToolInfo(wxWindow& w, std::optional<wxRect>&& r, wxString&& t, Flags f) :
    wnd(w),
    rect(std::move(r)),
    tip(std::move(t)),
    flags(f)
{
}

void CB::InflateAndNormalize(wxRect& rect, int dx, int dy)
{
    if (dx < 0 && abs(2*dx) > rect.GetWidth())
    {
        wxASSERT(!"needs testing");
        /* the sides will move so far that
            left and right will swap
        newleft = oldright + dx
                === oldleft + width + dx
                === oldleft + -1*-width + -1*-dx
                === oldleft + -1*(-width + -dx)
                === oldleft - (-width + -dx)
                === oldleft - (-width + abs(dx))
                === oldleft - (abs(dx) - width)
        newright = oldleft - dx
                === oldright - width - dx
                === oldright + (- width - dx)
                === oldright + (-width + -dx)
                === oldright + (-width + abs(dx))
                === oldright + (abs(dx) - width)
        */
        dx = abs(dx) - rect.GetWidth();
    }
    if (dy < 0 && abs(2*dy) > rect.GetHeight())
    {
        wxASSERT(!"needs testing");
        dy = abs(dy) - rect.GetHeight();
    }
    rect.Inflate(dx, dy);
}

#if 0
namespace CB
{
    int ToWxID(int id)
    {
        switch (id)
        {
            case ID_EDIT_CLEAR:
                return wxID_CLEAR;
            case ID_EDIT_COPY:
                return wxID_COPY;
            case ID_EDIT_PASTE:
                return wxID_PASTE;
            case ID_EDIT_UNDO:
                return wxID_UNDO;
            default:
#if defined(GPLAY)
                // TODO:
                return wxID_NONE;
#else
                switch (id)
                {
                    case ID_BUGFIX_DUMPBADTILES:
                        return XRCID("ID_BUGFIX_DUMPBADTILES");
                    case ID_COLOR_FOREGROUND:
                        return XRCID("ID_COLOR_FOREGROUND");
                    case ID_COLOR_BACKGROUND:
                        return XRCID("ID_COLOR_BACKGROUND");
                    case ID_COLOR_TRANSPARENT:
                        return XRCID("ID_COLOR_TRANSPARENT");
                    case ID_COLOR_CUSTOM:
                        return XRCID("ID_COLOR_CUSTOM");
                    case ID_DUMP_TILEDATA:
                        return XRCID("ID_DUMP_TILEDATA");
                    case ID_DWG_DRAWABOVEGRID:
                        return XRCID("ID_DWG_DRAWABOVEGRID");
                    case ID_DWG_FONT:
                        return XRCID("ID_DWG_FONT");
                    case ID_DWG_TOBACK:
                        return XRCID("ID_DWG_TOBACK");
                    case ID_DWG_TOFRONT:
                        return XRCID("ID_DWG_TOFRONT");
                    case ID_EDIT_CREATEBOARD:
                        return XRCID("ID_EDIT_CREATEBOARD");
                    case ID_EDIT_CREATEMARKGROUP:
                        return XRCID("ID_EDIT_CREATEMARKGROUP");
                    case ID_EDIT_CREATEPIECEGROUP:
                        return XRCID("ID_EDIT_CREATEPIECEGROUP");
                    case ID_EDIT_CREATETILEGROUP:
                        return XRCID("ID_EDIT_CREATETILEGROUP");
                    case ID_EDIT_GBOXPROPERTIES:
                        return XRCID("ID_EDIT_GBOXPROPERTIES");
                    case ID_EDIT_LAYER_BASE:
                        return XRCID("ID_EDIT_LAYER_BASE");
                    case ID_EDIT_LAYER_TILE:
                        return XRCID("ID_EDIT_LAYER_TILE");
                    case ID_EDIT_LAYER_TOP:
                        return XRCID("ID_EDIT_LAYER_TOP");
                    case ID_EDIT_MOVE:
                        return XRCID("ID_EDIT_MOVE");
                    case ID_EDIT_PASTEBITMAPFROMFILE:
                        return XRCID("ID_EDIT_PASTEBITMAPFROMFILE");
                    case ID_EXPORT_GAMEBOX:
                        return XRCID("ID_EXPORT_GAMEBOX");
                    case ID_IMAGE_BOARDMASK:
                        return XRCID("ID_IMAGE_BOARDMASK");
                    case ID_IMAGE_GRIDLINES:
                        return XRCID("ID_IMAGE_GRIDLINES");
                    case ID_INDICATOR_CELLNUM:
                        return XRCID("ID_INDICATOR_CELLNUM");
                    case ID_ITOOL_PENCIL:
                        return XRCID("ID_ITOOL_PENCIL");
                    case ID_ITOOL_SELECT:
                        return XRCID("ID_ITOOL_SELECT");
                    case ID_ITOOL_BRUSH:
                        return XRCID("ID_ITOOL_BRUSH");
                    case ID_ITOOL_FILL:
                        return XRCID("ID_ITOOL_FILL");
                    case ID_ITOOL_TEXT:
                        return XRCID("ID_ITOOL_TEXT");
                    case ID_ITOOL_LINE:
                        return XRCID("ID_ITOOL_LINE");
                    case ID_ITOOL_RECT:
                        return XRCID("ID_ITOOL_RECT");
                    case ID_ITOOL_OVAL:
                        return XRCID("ID_ITOOL_OVAL");
                    case ID_ITOOL_FILLRECT:
                        return XRCID("ID_ITOOL_FILLRECT");
                    case ID_ITOOL_FILLOVAL:
                        return XRCID("ID_ITOOL_FILLOVAL");
                    case ID_ITOOL_DROPPER:
                        return XRCID("ID_ITOOL_DROPPER");
                    case ID_ITOOL_COLORCHANGE:
                        return XRCID("ID_ITOOL_COLORCHANGE");
                    case ID_LINE_WIDTH:
                        return XRCID("ID_LINE_WIDTH");
                    case ID_PROJECT_CHGID:
                        return XRCID("ID_PROJECT_CHGID");
                    case ID_PROJECT_LOADTILEFILE:
                        return XRCID("ID_PROJECT_LOADTILEFILE");
                    case ID_PROJECT_SAVETILEFILE:
                        return XRCID("ID_PROJECT_SAVETILEFILE");
                    case ID_STICKY_DRAWTOOLS:
                        return XRCID("ID_STICKY_DRAWTOOLS");
                    case ID_TOOL_ARROW:
                        return XRCID("ID_TOOL_ARROW");
                    case ID_TOOL_DROPPER:
                        return XRCID("ID_TOOL_DROPPER");
                    case ID_TOOL_ERASER:
                        return XRCID("ID_TOOL_ERASER");
                    case ID_TOOL_FILL:
                        return XRCID("ID_TOOL_FILL");
                    case ID_TOOL_LINE:
                        return XRCID("ID_TOOL_LINE");
                    case ID_TOOL_OVAL:
                        return XRCID("ID_TOOL_OVAL");
                    case ID_TOOL_POLYGON:
                        return XRCID("ID_TOOL_POLYGON");
                    case ID_TOOL_RECT:
                        return XRCID("ID_TOOL_RECT");
                    case ID_TOOL_SETVISIBLESCALE:
                        return XRCID("ID_TOOL_SETVISIBLESCALE");
                    case ID_TOOL_SUSPENDSCALEVISIBILITY:
                        return XRCID("ID_TOOL_SUSPENDSCALEVISIBILITY");
                    case ID_TOOL_TEXT:
                        return XRCID("ID_TOOL_TEXT");
                    case ID_TOOL_TILE:
                        return XRCID("ID_TOOL_TILE");
                    case ID_TOOLS_BRDPROPS:
                        return XRCID("ID_TOOLS_BRDPROPS");
                    case ID_TOOLS_BRDSNAPGRID:
                        return XRCID("ID_TOOLS_BRDSNAPGRID");
                    case ID_VIEW_FULLSCALE:
                        return XRCID("ID_VIEW_FULLSCALE");
                    case ID_VIEW_GRIDLINES:
                        return XRCID("ID_VIEW_GRIDLINES");
                    case ID_VIEW_HALFSCALE:
                        return XRCID("ID_VIEW_HALFSCALE");
                    case ID_VIEW_SMALLSCALE:
                        return XRCID("ID_VIEW_SMALLSCALE");
                    case ID_VIEW_TOGGLE_SCALE:
                        return XRCID("ID_VIEW_TOGGLE_SCALE");
                    case ID_VIEW_ZOOMIN:
                        return wxID_ZOOM_IN;
                    case ID_VIEW_ZOOMOUT:
                        return wxID_ZOOM_OUT;
                    default:
                        return wxID_NONE;
                }
#endif
        }
    }
}

BOOL CB::RelayOnCmdMsg(wxEvtHandler& dest,
                        UINT nID, int nCode, void* pExtra,
                        AFX_CMDHANDLERINFO* pHandlerInfo)
{
    switch (nCode)
    {
        case CN_COMMAND:
        {
            // MFC uses this as part of its CN_UPDATE_COMMAND_UI
            if (pHandlerInfo)
            {
                /* MFC checks whether handler exists, but that's
                not possible in wx
                (see https://groups.google.com/g/wx-dev/c/DD9CYFAjbdM/m/ndNP3TZXBgAJ),
                so approximate this way */
                class CDummyCmdUI : public CCmdUI
                {
                    void SetCheck(int nCheck = 1) override {}   // 0, 1 or 2 (indeterminate)
                };
                CDummyCmdUI dummy;
                return RelayOnCmdMsg(dest, nID, CN_UPDATE_COMMAND_UI, &dummy, nullptr);
            }
            wxCommandEvent event(wxEVT_MENU, CB::ToWxID(nID));
            return dest.ProcessEvent(event);
        }
        case CN_UPDATE_COMMAND_UI:
        {
            CCmdUI& pCmdUI = CheckedDeref(static_cast<CCmdUI*>(pExtra));
            wxUpdateUIEvent event(CB::ToWxID(nID));
#if !defined(GPLAY)
            CColorCmdUI* colorCmdUI = dynamic_cast<CColorCmdUI*>(&pCmdUI);
            if (colorCmdUI)
            {
                event.SetClientData(colorCmdUI);
            }
#endif
            bool retval = dest.ProcessEvent(event);
            if (retval)
            {
                /* enable the menu, even if the item isn't
                    (this handles the image tool menu when
                    the small bmp is selected) */
                if (pCmdUI.m_pSubMenu != NULL)
                {
                    pCmdUI.m_pMenu->EnableMenuItem(pCmdUI.m_nIndex,
                        MF_BYPOSITION | MF_ENABLED);
                }
                if (event.GetSetEnabled())
                {
                    pCmdUI.Enable(event.GetEnabled());
                }
                if (event.GetSetChecked())
                {
                    pCmdUI.SetCheck(event.GetChecked());
                }
                if (event.GetSetText())
                {
                    pCmdUI.SetText(event.GetText());
                }
            }
            return retval;
        }
        default:
            return false;
    }
}
#endif

namespace CB
{
    int ToMFCID(int wxId)
    {
        static const std::map<int, int> ids = {
#if !defined(GPLAY)
            { XRCID("ID_IMAGE_DISCARD"), ID_IMAGE_DISCARD },
            { XRCID("ID_IMAGE_UPDATE"), ID_IMAGE_UPDATE },
            { XRCID("ID_STICKY_DRAWTOOLS"), ID_STICKY_DRAWTOOLS },
            { XRCID("ID_TOOLS_RESIZETILE"), ID_TOOLS_RESIZETILE },
            { XRCID("ID_TOOLS_ROT90"), ID_TOOLS_ROT90 },
            { XRCID("ID_TOOLS_ROT180"), ID_TOOLS_ROT180 },
            { XRCID("ID_TOOLS_ROT270"), ID_TOOLS_ROT270 },
#else
#endif
        };
        auto it = ids.find(wxId);
        if (it != ids.end())
        {
            return it->second;
        }
        else
        {
            return 0;
        }
    }
}

#if 0
bool CB::RelayProcessEvent(CCmdTarget& dest,
                            wxEvent& event)
{
    if (event.GetEventType() == wxEVT_MENU)
    {
        wxCommandEvent& cmdEvent = dynamic_cast<wxCommandEvent&>(event);
        if (int mfcId = ToMFCID(cmdEvent.GetId()))
        {
            return dest.OnCmdMsg(mfcId, CN_COMMAND, nullptr, nullptr);
        }
    }
    else if (event.GetEventType() == wxEVT_UPDATE_UI)
    {
        wxUpdateUIEvent& uuiEvent = dynamic_cast<wxUpdateUIEvent&>(event);
        if (int mfcId = ToMFCID(uuiEvent.GetId()))
        {
            class Adapter : public CCmdUI
            {
            public:
                Adapter(wxUpdateUIEvent& e) : wxUUIEvent(e) {}
                void Enable(BOOL bOn = TRUE) override
                {
                    wxUUIEvent.Enable(bOn);
                }
                void SetCheck(int nCheck = 1) override   // 0, 1 or 2 (indeterminate)
                {
                    switch (nCheck)
                    {
                        case 0:
                            wxUUIEvent.Check(false);
                            break;
                        case 1:
                            wxUUIEvent.Check(true);
                            break;
                        case 2:
                            wxASSERT(!"untested code");
                            wxUUIEvent.Set3StateValue(wxCHK_UNDETERMINED);
                            break;
                        default:
                            wxASSERT(!"unknown value");
                    }
                }
                void SetRadio(BOOL bOn = TRUE) override
                {
                    wxASSERT(!"not implemented");
                }
                void SetText(LPCTSTR lpszText) override
                {
                    wxUUIEvent.SetText(lpszText);
                }
            private:
                wxUpdateUIEvent& wxUUIEvent;
            };
            Adapter cmdui(uuiEvent);
            if (dest.OnCmdMsg(mfcId, CN_UPDATE_COMMAND_UI, &cmdui, nullptr))
            {
                return true;
            }
        }
    }

    return false;
}
#endif

int CB::GetMouseButtons(const wxMouseState& event)
{
    int retval = 0;
    retval |= event.LeftIsDown() ? wxMOUSE_BTN_LEFT : 0;
    retval |= event.MiddleIsDown() ? wxMOUSE_BTN_MIDDLE : 0;
    retval |= event.RightIsDown() ? wxMOUSE_BTN_RIGHT : 0;
    retval |= event.Aux1IsDown() ? wxMOUSE_BTN_AUX1 : 0;
    retval |= event.Aux2IsDown() ? wxMOUSE_BTN_AUX2 : 0;
    return retval;
}

wxDocTemplate& CB::FindDocTemplateByView(const wxClassInfo& classInfo)
{
    wxDocManager& docMgr = CheckedDeref(wxDocManager::GetDocumentManager());
    wxList& templates = docMgr.GetTemplates();
    for (auto it = templates.begin() ; it != templates.end() ; ++it)
    {
        wxDocTemplate& templ = dynamic_cast<wxDocTemplate&>(**it);
        if (templ.GetViewClassInfo() == &classInfo)
        {
            return templ;
        }
    }

    AfxThrowInvalidArgException();
}

#if !defined(GPLAY)
wxAuiToolBar& CB::CreateToolbar(wxWindow& parent, const ToolArgs(&toolArgs)[], size_t count, unsigned bmapID)
{
    wxBitmap tools(std::format("#{}", bmapID),
                    wxBITMAP_TYPE_BMP_RESOURCE);
    wxAuiToolBar& toolbar = *new wxAuiToolBar(&parent);
    int separators = 0;
    for (int i = 0 ; i < value_preserving_cast<int>(count) ; ++i)
    {
        int id = toolArgs[i].xrcId;
        if (id != wxID_SEPARATOR)
        {
            CB::string str = CB::string::LoadString(toolArgs[i].stringId);
            std::vector<wxString> tokens;
            wxStringTokenizer tokenizer(str, "\n");
            while (tokenizer.HasMoreTokens())
            {
                tokens.push_back(tokenizer.GetNextToken());
            }
            if (tokens.size() == size_t(1))
            {
                tokens.push_back(wxEmptyString);
            }
            wxASSERT(tokens.size() == size_t(2));
            wxBitmap tool = tools.GetSubBitmap(wxRect(wxPoint(16*(i - separators), 0),
                                                wxSize(16, 16)));
            toolbar.AddTool(id,
                            wxEmptyString,
                            tool,
                            wxNullBitmap,
                            toolArgs[i].kind,
                            tokens.back(),
                            tokens.front(),
                            nullptr);
        }
        else
        {
            ++separators;
            toolbar.AddSeparator();
        }
    }
    wxASSERT(tools.GetSize() == wxSize(
        value_preserving_cast<int>(size_t(16) * (count - value_preserving_cast<size_t>(separators))), 16));
    toolbar.Realize();
    return toolbar;
}
#endif

wxBEGIN_EVENT_TABLE(CB::wxStatusBar, ::wxStatusBar)
    EVT_IDLE(OnIdle)
    EVT_UPDATE_UI(wxID_SEPARATOR, OnUpdateUI)
    EVT_UPDATE_UI(ID_INDICATOR_CAPS, OnUpdateUI)
    EVT_UPDATE_UI(ID_INDICATOR_NUM, OnUpdateUI)
wxEND_EVENT_TABLE()

void CB::wxStatusBar::SetIndicators(
                                const int (&ids)[],
                                size_t count)
{
    wxASSERT(value_preserving_cast<size_t>(GetFieldsCount()) == count);
    indicators.resize(count);

    wxInfoDC dc(this);
    std::vector<int> widths(count);
    for (size_t i = size_t(0) ; i < count ; ++i)
    {
        indicators[i] = ids[value_preserving_cast<ptrdiff_t>(i)];
        if (indicators[i] != wxID_SEPARATOR)
        {
            CB::string s = CB::string::LoadString(indicators[i]);
            widths[i] = dc.GetTextExtent(s).x;
            SetStatusText(s, value_preserving_cast<int>(i));
        }
        else
        {
            widths[i] = -1;
        }
    }
    SetStatusWidths(value_preserving_cast<int>(widths.size()), widths.data());
}

void CB::wxStatusBar::OnIdle(wxIdleEvent& WXUNUSED(event))
{
    // represent disabled fields as ""
    // use the status field stacks to remember enabled value
    for (int i = 0 ; i < GetFieldsCount() ; ++i)
    {
        wxUpdateUIEvent uievent(indicators[value_preserving_cast<size_t>(i)]);
        bool rc = ProcessWindowEvent(uievent);
        wxASSERT(rc == !uievent.GetSkipped());
        if (rc)
        {
            bool wasEnabled = !GetStatusText(i).empty();
            if (uievent.GetSetText())
            {
                // reserve "" for disabled fields
                wxString s = uievent.GetText();
                if (s.empty())
                {
                    s = wxString("\0", 1);
                }
                SetStatusText(s, i);
            }
            if (uievent.GetSetEnabled())
            {
                if (wasEnabled && !uievent.GetEnabled())
                {
                    PushStatusText(wxEmptyString, i);
                }
                else if (!wasEnabled && uievent.GetEnabled())
                {
                    PopStatusText(i);
                }
            }
        }
    }
}

void CB::wxStatusBar::OnUpdateUI(wxUpdateUIEvent& event)
{
    if (event.GetId() == ID_INDICATOR_CAPS)
    {
        event.Enable(wxGetKeyState(WXK_CAPITAL));
    }
    else if (event.GetId() == ID_INDICATOR_NUM)
    {
        event.Enable(wxGetKeyState(WXK_NUMLOCK));
    }
    else
    {
        event.Skip();
    }
}

CB::wxStatusBar& CB::wxAuiMDIParentFrame::CreateStatusBar(
                                const int (&ids)[],
                                size_t count)
{
    ::wxStatusBar* psb = ::wxAuiMDIParentFrame::CreateStatusBar(value_preserving_cast<int>(count));
    ::wxStatusBar& sb = CheckedDeref(psb);
    CB::wxStatusBar& retval = dynamic_cast<CB::wxStatusBar&>(sb);
    retval.SetIndicators(ids, count);
    return retval;
}

CB::wxStatusBar* CB::wxAuiMDIParentFrame::OnCreateStatusBar(int number,
                                            long style,
                                            wxWindowID id,
                                            const wxString& name)
{
    CB::wxStatusBar* statusBar = new CB::wxStatusBar(this, id, style, "CB::" + name);

    statusBar->SetFieldsCount(number);

    return statusBar;
}
