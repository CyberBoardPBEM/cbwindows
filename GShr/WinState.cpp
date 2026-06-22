// winstate.cpp - classes used to manage window state.
//
// Copyright (c) 1994-2026 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    "WinState.h"
#include    "Versions.h"
#if !defined(GPLAY)
    #include    "GmDoc.h"
#else
    #include    "GamDoc.h"
#endif

wxDEFINE_EVENT(WM_WINSTATE_WX, WinStateEvent);

///////////////////////////////////////////////////////////////////////////
// Returns FALSE if no frame restoration data is supplied by frames.

BOOL CWinStateManager::GetStateOfOpenDocumentFrames(const Features& ff)
{
    if (!ff.Check(ftrAuiLayout))
    {
        // First entry in the frame list is the main frame...

        OwnerPtr<CWinStateElement> pWse(GetWindowState(dynamic_cast<wxFrame&>(CB::GetMainWndWx())));

        pWse->m_wWinCode = wincodeMainFrame;
        m_pList.push_back(std::move(pWse));

        // Then we need to build a list of MDI frames that are in Z
        // order so we can restore the proper visual order later.

        std::vector<RefPtr<wxFrame>> tblFrame = GetDocumentFrameList();         // Get's unordered list
        ArrangeFrameListInZOrder(tblFrame);     // Order 'em

        // Scan the list in reverse Z order and obtain serialized
        // data needed to later restore the window state.

        for (size_t i = tblFrame.size(); i > size_t(0); i--)
        {
            wxFrame& pWnd = *tblFrame.at(i - size_t(1));
            OwnerPtr<CWinStateElement> pWse(GetWindowState(pWnd));
            pWse->m_wWinCode = wincodeViewFrame;
            OnAnnotateWinStateElement(*pWse, pWnd);
            m_pList.push_back(std::move(pWse));
        }
    }
    else
    {
        wxASSERT(!serializer);
        wxTopLevelWindow& mainfrm = dynamic_cast<wxTopLevelWindow&>(CB::GetMainWndWx());
        wxAuiManager& auiMgr = CheckedDeref(wxAuiManager::GetManager(&mainfrm));
        serializer = NewSerializer(ff, auiMgr, *m_pDoc);
        mainfrm.SaveGeometry(*serializer);
        auiMgr.SaveLayout(*serializer);
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////

void CWinStateManager::RestoreStateOfDocumentFrames()
{
    if (!fileFeatures.Check(ftrAuiLayout))
    {
        CGamDoc::SetLoadingVersionGuard setLoadingVersionGuard(fileVersion);
        // Processes only the main frame and the MDI child frames.
        // All other records are ignored.
        for (CWinStateList::iterator pos = m_pList.begin() ; pos != m_pList.end() ; ++pos)
        {
            CWinStateElement& pWse = **pos;
            if (pWse.m_wWinCode == wincodeMainFrame)
            {
                wxFrame& pWnd = dynamic_cast<wxFrame&>(CB::GetMainWndWx());
                // TODO:  wxWidgets doesn't wrap all of this
                CRect rctVScreen;
                SystemParametersInfo(SPI_GETWORKAREA, 0, (RECT*)rctVScreen, 0);
                int cxVScreen = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                if (cxVScreen > 0)
                {
                    // Multimonitor metrics are are supported
                    int cyVScreen = GetSystemMetrics(SM_CYVIRTUALSCREEN);
                    int xVScreen = GetSystemMetrics(SM_XVIRTUALSCREEN);
                    int yVScreen = GetSystemMetrics(SM_YVIRTUALSCREEN);
                    rctVScreen.SetRect(xVScreen, yVScreen, xVScreen + cxVScreen, yVScreen + cyVScreen);
                }
                else
                {
                    // Otherwise set the virtual screen rect to the primary
                    // desktop dimensions.
                    SystemParametersInfo(SPI_GETWORKAREA, 0, (RECT*)rctVScreen, 0);
                }
                CRect rctDesktop;
                SystemParametersInfo(SPI_GETWORKAREA, 0, (RECT*)rctDesktop, 0);

                // Clone the saved window position for easier calculations
                CRect rctSaved(pWse.m_wndState.rcNormalPosition);

                // Check if the Window's midpoint is visible. If so leave it's position alone.
                if (!rctVScreen.PtInRect(rctSaved.CenterPoint()))
                {
                    // Force the window onto the primary desktop area.
                    pWse.m_wndState.rcNormalPosition = rctDesktop;
                }
                if (pWse.m_wndState.showCmd == SW_SHOWMAXIMIZED)
                {
                    wxASSERT(dynamic_cast<wxTopLevelWindow*>(&pWnd));
                    static_cast<wxTopLevelWindow&>(pWnd).Maximize();
                }
                else
                {
                    wxRect rect = CB::Convert(pWse.m_wndState.rcNormalPosition);
                    pWnd.SetSize(rect.GetSize());
                }
                RestoreWindowState(pWnd, pWse);
            }
            else if (pWse.m_wWinCode == wincodeViewFrame)
            {
                wxFrame& pWnd = OnGetFrameForWinStateElement(pWse);
                pWnd.Raise();
                wxRect rect = CB::Convert(pWse.m_wndState.rcNormalPosition);
                pWnd.SetSize(rect.GetSize());
                RestoreWindowState(pWnd, pWse);
            }
        }
    }
    else
    {
        wxTopLevelWindow& mainfrm = dynamic_cast<wxTopLevelWindow&>(CB::GetMainWndWx());
        wxAuiManager& auiMgr = CheckedDeref(wxAuiManager::GetManager(&mainfrm));
        mainfrm.RestoreToGeometry(CheckedDeref(serializer));
        auiMgr.LoadLayout(*serializer);
    }
}

///////////////////////////////////////////////////////////////////////////

OwnerPtr<CWinStateManager::CWinStateElement> CWinStateManager::GetWindowState(const wxFrame& pWnd)
{
    OwnerPtr<CWinStateElement> pWse = OnCreateWinStateElement();
#if 0
    pWnd.GetWindowPlacement(&pWse->m_wndState);

    TRY
    {
        CMemFile file;
        CArchive ar(&file, CArchive::store);
        BOOL bOK = (BOOL)pWnd.SendMessage(WM_WINSTATE, (WPARAM)&ar, 0);
        ar.Close();
        if (bOK)
        {
            size_t len = value_preserving_cast<size_t>(file.GetLength());
            pWse->m_pWinStateBfr.Reset(file.Detach(), len);
        }
    }
    END_TRY

    return pWse;
#else
    AfxThrowNotSupportedException();
#endif
}

///////////////////////////////////////////////////////////////////////////

BOOL CWinStateManager::RestoreWindowState(wxFrame& pWnd, CWinStateElement& pWse)
{
    if (pWse.m_pWinStateBfr.empty())
        return TRUE;

    BOOL bOK = FALSE;

    try
    {
        CMemFile file(reinterpret_cast<BYTE*>(pWse.m_pWinStateBfr.data()), value_preserving_cast<unsigned>(pWse.m_pWinStateBfr.size()));
        CArchive ar(&file, CArchive::load);
        SetFileFeaturesGuard setFileFeaturesGuard(ar, fileFeatures);
        WinStateEvent event(ar, true);
        bOK = pWnd.ProcessWindowEvent(event) &&
                event.GetResult() &&
                *event.GetResult();
        ar.Close();
        file.Detach();
    }
    catch (...)
    {
    }
    return bOK;
}

/////////////////////////////////////////////////////////////////////////////

std::vector<RefPtr<wxFrame>> CWinStateManager::GetDocumentFrameList()
{
    std::vector<RefPtr<wxFrame>> tblFrames;

#if 0
    POSITION pos = m_pDoc->GetFirstViewPosition();
    while (pos != NULL)
    {
        CView& pView = CheckedDeref(m_pDoc->GetNextView(pos));
        CFrameWnd& pFrame = CheckedDeref(pView.GetParentFrame());
        size_t i;
        for (i = size_t(0); i < tblFrames.size(); i++)
        {
            if (&pFrame == &*tblFrames.at(i))
                break;
        }
        if (i == tblFrames.size())
            tblFrames.push_back(&pFrame);          // Add new frame
    }

    return tblFrames;
#else
    wxASSERT(!"needs testing");
    wxList& views = m_pDoc->GetViews();
    for (auto it = views.begin() ; it != views.end() ; ++it)
    {
        CB::View& pView = dynamic_cast<CB::View&>(CheckedDeref(*it));
        CB::DocChildFrame& pFrame = pView.GetFrame();
        size_t i;
        for (i = size_t(0); i < tblFrames.size(); i++)
        {
            if (&pFrame == &*tblFrames.at(i))
                break;
        }
        if (i == tblFrames.size())
            tblFrames.push_back(&pFrame);          // Add new frame
    }

    return tblFrames;
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CWinStateManager::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (!CB::GetFeatures(ar).Check(ftrAuiLayout))
        {
            if (m_pList.empty())
            {
                ar << (uint32_t)0;
                return;
            }
            if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
            {
                ar << value_preserving_cast<uint32_t>(m_pList.size());
            }
            else
            {
                CB::WriteCount(ar, m_pList.size());
            }
            for (CWinStateList::iterator pos = m_pList.begin() ; pos != m_pList.end() ; ++pos)
            {
                CWinStateElement& pWse = **pos;
                pWse.Serialize(ar);
            }
        }
        else
        {
            CheckedDeref(serializer).Store(ar);
        }
    }
    else
    {
        if (!CB::GetFeatures(ar).Check(ftrAuiLayout))
        {
            wxASSERT(m_pList.empty());
            size_t dwCount;
            if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
            {
                uint32_t temp;
                ar >> temp;
                dwCount = temp;
            }
            else
            {
                dwCount = CB::ReadCount(ar);
            }
            if (dwCount == size_t(0))
                return;
            fileVersion = CB::GetVersion(ar);
            fileFeatures = CB::GetFeatures(ar);
            m_pList.clear();
            while (dwCount--)
            {
                OwnerPtr<CWinStateElement> pWse(OnCreateWinStateElement());
                pWse->Serialize(ar);
                m_pList.push_back(std::move(pWse));
            }
        }
        else
        {
            wxASSERT(!serializer);
            fileVersion = CB::GetVersion(ar);
            fileFeatures = CB::GetFeatures(ar);
            wxTopLevelWindow& mainfrm = dynamic_cast<wxTopLevelWindow&>(CB::GetMainWndWx());
            wxAuiManager& auiMgr = CheckedDeref(wxAuiManager::GetManager(&mainfrm));
            serializer = NewSerializer(fileFeatures, auiMgr, *m_pDoc);
            serializer->Load(ar);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

void CWinStateManager::ArrangeFrameListInZOrder(std::vector<RefPtr<wxFrame>>& tblFrames)
{
#if 0
    std::vector<CFrameWnd*> tblZFrames;

    CMDIFrameWnd* pFrame = (CMDIFrameWnd*)AfxGetMainWnd();
    ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)));

    EnumChildWindows(pFrame->m_hWndMDIClient, EnumFrames, (LPARAM)&tblZFrames);
#else
    wxASSERT(!"needs testing");
    std::vector<wxFrame*> tblZFrames;

    CB::AuiMDIParentFrame& pFrame = dynamic_cast<CB::AuiMDIParentFrame&>(CB::GetMainWndWx());

    {
        wxAuiMDIClientWindow& client = CheckedDeref(pFrame.GetClientWindow());
        std::vector<wxAuiTabCtrl*> groups = client.GetAllTabCtrls();
        for (wxAuiTabCtrl* group : groups)
        {
            std::vector<size_t> indices = client.GetPagesInDisplayOrder(group);
            for (size_t index : indices)
            {
                wxWindow* wnd = client.GetPage(index);
                wxASSERT(dynamic_cast<wxFrame*>(wnd));
                tblZFrames.push_back(static_cast<wxFrame*>(wnd));
            }
        }
    }
#endif

    // Null out any entries that aren't in the caller's table
    for (size_t i = 0; i < tblZFrames.size(); i++)
    {
        size_t j;
        for (j = 0; j < tblFrames.size(); j++)
        {
            if (&*tblFrames.at(j) == tblZFrames.at(i))
                break;
        }
        if (j == tblFrames.size())
            tblZFrames.at(i) = NULL;
    }
    // Now copy the remaining frame pointers into the caller's list
    // in Z order (top to bottom)
    tblFrames.clear();
    tblFrames.reserve(tblZFrames.size());
    for (size_t i = size_t(0); i < tblZFrames.size(); i++)
    {
        if (tblZFrames.at(i) != NULL)
            tblFrames.push_back(tblZFrames.at(i));
    }
}

#if 0
BOOL CALLBACK CWinStateManager::EnumFrames(HWND hWnd, LPARAM dwTblFramePtr)
{
    std::vector<CFrameWnd*>& pTbl = CheckedDeref(reinterpret_cast<std::vector<CFrameWnd*>*>(dwTblFramePtr));
    CWnd* pWnd = CWnd::FromHandlePermanent(hWnd);
    // Only interested in certain kinds of windows.
    if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
        pTbl.push_back(static_cast<CMDIChildWndEx*>(pWnd));
    return TRUE;
}
#endif

///////////////////////////////////////////////////////////////////////////

CWinStateManager::CWinStateElement::CWinStateElement()
{
    m_wWinCode = 0;
    m_wUserCode1 = 0;
    m_boardID = nullBid;
}

void CWinStateManager::CWinStateElement::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_wWinCode;
        ar << m_wUserCode1;
        ar << m_boardID;
        ar << m_wndState;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            ar << value_preserving_cast<uint32_t>(m_pWinStateBfr.size());
        }
        else
        {
            CB::WriteCount(ar, m_pWinStateBfr.size());
        }
        if (!m_pWinStateBfr.empty())
            ar.Write(m_pWinStateBfr.data(), value_preserving_cast<unsigned>(m_pWinStateBfr.size()));
    }
    else
    {
        m_pWinStateBfr.clear();

        ar >> m_wWinCode;
        ar >> m_wUserCode1;
        ar >> m_boardID;
        ar >> m_wndState;
        size_t size;
        if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
        {
            uint32_t dwSize;
            ar >> dwSize;
            size = dwSize;
        }
        else
        {
            size = CB::ReadCount(ar);
        }
        if (size > size_t(0))
        {
            m_pWinStateBfr.resize(size);
            ar.Read(m_pWinStateBfr.data(), value_preserving_cast<unsigned>(m_pWinStateBfr.size()));
        }
    }
}

///////////////////////////////////////////////////////////////////////////

CWinPlacement::CWinPlacement()
{
    memset(this, 0, sizeof(WINDOWPLACEMENT));
    length = sizeof(WINDOWPLACEMENT);
}

CArchive& AFXAPI operator<<(CArchive& ar, const CWinPlacement& wndPlace)
{
//    ar << (DWORD)wndPlace.length;
    ar << value_preserving_cast<uint32_t>(wndPlace.flags);
    ar << value_preserving_cast<uint32_t>(wndPlace.showCmd);
    ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(wndPlace.ptMinPosition.x));
    ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(wndPlace.ptMinPosition.y));
    ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(wndPlace.ptMaxPosition.x));
    ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(wndPlace.ptMaxPosition.y));
    ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(wndPlace.rcNormalPosition.top));
    ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(wndPlace.rcNormalPosition.bottom));
    ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(wndPlace.rcNormalPosition.left));
    ar << static_cast<uint32_t>(value_preserving_cast<int32_t>(wndPlace.rcNormalPosition.right));

    return ar;
}

CArchive& AFXAPI operator>>(CArchive& ar, CWinPlacement& wndPlace)
{
    uint32_t dwTmp;
//    ar >> dwTmp; wndPlace.length = (UINT)dwTmp;
    ar >> dwTmp; wndPlace.flags = value_preserving_cast<UINT>(dwTmp);
    ar >> dwTmp; wndPlace.showCmd = value_preserving_cast<UINT>(dwTmp);
    ar >> dwTmp; wndPlace.ptMinPosition.x = value_preserving_cast<LONG>(static_cast<int32_t>(dwTmp));
    ar >> dwTmp; wndPlace.ptMinPosition.y = value_preserving_cast<LONG>(static_cast<int32_t>(dwTmp));
    ar >> dwTmp; wndPlace.ptMaxPosition.x = value_preserving_cast<LONG>(static_cast<int32_t>(dwTmp));
    ar >> dwTmp; wndPlace.ptMaxPosition.y = value_preserving_cast<LONG>(static_cast<int32_t>(dwTmp));
    ar >> dwTmp; wndPlace.rcNormalPosition.top = value_preserving_cast<LONG>(static_cast<int32_t>(dwTmp));
    ar >> dwTmp; wndPlace.rcNormalPosition.bottom = value_preserving_cast<LONG>(static_cast<int32_t>(dwTmp));
    ar >> dwTmp; wndPlace.rcNormalPosition.left = value_preserving_cast<LONG>(static_cast<int32_t>(dwTmp));
    ar >> dwTmp; wndPlace.rcNormalPosition.right = value_preserving_cast<LONG>(static_cast<int32_t>(dwTmp));

    return ar;
}

// CB4
CWinStateManager::Serializer::Serializer(const Features& ftrs, wxAuiManager& mgr, CGamDoc& d) :
    wxAuiDeserializer(mgr),
    features(ftrs),
    manager(mgr),
    doc(d)
{
}

void CWinStateManager::Serializer::Store(CArchive& ar) const
{
    wxASSERT(!geometry.empty());
    ar << geometry;

    wxASSERT(!paneLayoutInfos.empty());
    ar << paneLayoutInfos;
}

void CWinStateManager::Serializer::Load(CArchive& ar)
{
    wxASSERT(geometry.empty());
    ar >> geometry;

    wxASSERT(paneLayoutInfos.empty());
    size_t size = CB::ReadCount(ar);
    paneLayoutInfos.reserve(size);
    for (size_t i = size_t(0) ; i < size ; ++i)
    {
        paneLayoutInfos.emplace_back(ReadwxAuiPaneLayoutInfo(ar));
    }
}

/**
    Save a single field with the given value.

    Note that if this function returns @false, SaveGeometry() supposes
    that saving the geometry failed and returns @false itself, without
    even trying to save anything else.

    @param name uniquely identifies the field but is otherwise
        arbitrary.
    @param value value of the field (can be positive or negative, i.e.
        it can't be assumed that a value like -1 is invalid).

    @return @true if the field was saved or @false if saving it failed,
        resulting in wxTopLevelWindow::SaveGeometry() failure.
    */
bool CWinStateManager::Serializer::SaveValue(const wxString& name, int value)
{
    geometry[name] = value_preserving_cast<int32_t>(value);
    return true;
}

/**
    Try to restore a single field.

    Unlike for SaveValue(), returning @false from this function may
    indicate that the value simply wasn't present and doesn't prevent
    RestoreToGeometry() from continuing with trying to restore the
    other values.

    @param name uniquely identifies the field
    @param value non-null pointer to the value to be filled by this
        function

    @return @true if the value was retrieved or @false if it wasn't
        found or an error occurred.
    */
bool CWinStateManager::Serializer::RestoreValue(const wxString& name, int* value) const
{
    auto it = geometry.find(name);
    if (it != geometry.end())
    {
        CheckedDeref(value) = value_preserving_cast<int>(it->second);
        return true;
    }
    return false;
}

// Called before starting to save information about the panes, does nothing
// by default.
void CWinStateManager::Serializer::BeforeSavePanes()
{
    wxASSERT(paneLayoutInfos.empty());
}

// Save information about the given pane.
void CWinStateManager::Serializer::SavePane(const wxAuiPaneLayoutInfo& pane)
{
    paneLayoutInfos.emplace_back(pane);
}

// Called before starting to save information about the tabs in the
// notebook in the AUI pane with the given name.
void CWinStateManager::Serializer::BeforeSaveNotebook(const wxString& name)
{
    wxASSERT(name == "mdiclient");
}

// Load information about all the panes previously saved with SavePane().
std::vector<wxAuiPaneLayoutInfo> CWinStateManager::Serializer::LoadPanes()
{
    wxASSERT(!paneLayoutInfos.empty());
    std::vector<wxAuiPaneLayoutInfo> retval = std::move(paneLayoutInfos);
    return retval;
}

CArchive& operator<<(CArchive& ar, const wxAuiDockLayoutInfo& dock)
{
    return ar   << value_preserving_cast<int32_t>(dock.dock_direction)
                << value_preserving_cast<int32_t>(dock.dock_layer)
                << value_preserving_cast<int32_t>(dock.dock_row)
                << value_preserving_cast<int32_t>(dock.dock_pos)
                << value_preserving_cast<int32_t>(dock.dock_proportion)
                << value_preserving_cast<int32_t>(dock.dock_size);
}

CArchive& operator>>(CArchive& ar, wxAuiDockLayoutInfo& dock)
{
    int32_t temp;
    ar >> temp;
    dock.dock_direction = value_preserving_cast<int>(temp);
    ar >> temp;
    dock.dock_layer = value_preserving_cast<int>(temp);
    ar >> temp;
    dock.dock_row = value_preserving_cast<int>(temp);
    ar >> temp;
    dock.dock_pos = value_preserving_cast<int>(temp);
    ar >> temp;
    dock.dock_proportion = value_preserving_cast<int>(temp);
    ar >> temp;
    dock.dock_size = value_preserving_cast<int>(temp);
    return ar;
}

CArchive& operator<<(CArchive& ar, const wxAuiPaneLayoutInfo& dock)
{
    return ar   << static_cast<const wxAuiDockLayoutInfo&>(dock)
                << CB::string(dock.name)
                << dock.floating_pos
                << dock.floating_size
                << dock.floating_client_size
                << dock.is_maximized
                << dock.is_hidden;
}

/* KLUDGE:  wxAuiPaneLayoutInfo has no default ctor
CArchive& operator>>(CArchive& ar, wxAuiPaneLayoutInfo& dock);
*/
wxAuiPaneLayoutInfo ReadwxAuiPaneLayoutInfo(CArchive& ar)
{
    wxAuiDockLayoutInfo temp1;
    ar >> temp1;
    CB::string temp2;
    ar >> temp2;
    wxAuiPaneLayoutInfo retval(temp2);
    static_cast<wxAuiDockLayoutInfo&>(retval) = temp1;
    ar  >> retval.floating_pos
        >> retval.floating_size
        >> retval.floating_client_size
        >> retval.is_maximized
        >> retval.is_hidden;
    return retval;
}

