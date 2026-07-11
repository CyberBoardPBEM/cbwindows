// WStateGp.cpp - classes used to manage player program window state.
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
#include    <numeric>
#include    "Gp.h"
#include    "GamDoc.h"
#include    "FrmPbrd.h"
#include    "Board.h"
#include    "PBoard.h"
#include    "VwPbrd.h"
#include    "VwPrjgam.h"
#include    "VwPrjgsn.h"
#include    "WStateGp.h"

enum { gpFrmProject = 0, gpFrmPlayBoard = 1 };

wxFrame& CGpWinStateMgr::OnGetFrameForWinStateElement(const CWinStateElement& pWse)
{
    wxASSERT(pWse.m_wWinCode == wincodeViewFrame);
    CGamDoc& pDoc = GetDocument();

    if (pWse.m_wUserCode1 == gpFrmProject)
    {
        CB::View& view = pDoc.FindProjectOrScenarioView();
        wxFrame& pWnd = view.GetFrame();
        return pWnd;
    }
    else if (pWse.m_wUserCode1 == gpFrmPlayBoard)
    {
        // The second user code is the board's serial number.
        CPlayBoard& pPBoard = CheckedDeref(pDoc.GetPBoardManager().GetPBoardBySerial(pWse.m_boardID));
        CPlayBoardView* pView = pDoc.FindPBoardView(pPBoard);
        if (pView == NULL)
        {
            // No frame open for board. Create it.
            pDoc.CreateNewFrame(
                pPBoard.GetBoard()->GetName(), pPBoard);
            // Try to locate it again
            pView = pDoc.FindPBoardView(pPBoard);
            wxASSERT(pView);
        }
        return static_cast<wxPlayBoardView&>(*pView).GetFrame();
    }
    else
        AfxThrowInvalidArgException();
}

void CGpWinStateMgr::OnAnnotateWinStateElement(CWinStateElement& pWse, const wxFrame& pWnd)
{
#if 0
    if (pWnd.IsKindOf(RUNTIME_CLASS(CProjFrame)))
        pWse.m_wUserCode1 = gpFrmProject;
    else if (pWnd.IsKindOf(RUNTIME_CLASS(CPlayBoardPanelContainer)))
    {
        const CPlayBoardPanelContainer& pPanelContainer = static_cast<const CPlayBoardPanelContainer&>(pWnd);
        const CPlayBoardPanel& pPanel = pPanelContainer.GetChild();
        pWse.m_wUserCode1 = gpFrmPlayBoard;
        pWse.m_boardID = pPanel.m_pPBoard->GetSerialNumber();
    }
#else
    AfxThrowNotSupportedException();
#endif
}

// output direction of Serialize(CArchive&)
void CGpWinStateMgr::GpSerializer::Store(CArchive& ar) const
{
    Serializer::Store(ar);

    ar << tabLayoutInfos << winstates;
}

// input direction of Serialize(CArchive&)
void CGpWinStateMgr::GpSerializer::Load(CArchive& ar)
{
    Serializer::Load(ar);

    wxASSERT(tabLayoutInfos.empty() && winstates.empty());
    ar >> tabLayoutInfos >> winstates;
}

// Called to save information about a single tab control in the given
// notebook.
void CGpWinStateMgr::GpSerializer::SaveNotebookTabControl(const wxAuiTabLayoutInfo& tab)
{
    // TEMP: see https://github.com/wxWidgets/wxWidgets/pull/26679
    wxASSERT((tab.dock_direction != wxAUI_DOCK_CENTRE) ||
                (tab.dock_layer == 0 && tab.dock_row == 0 && tab.dock_pos == 0));
    wxWindow& mgrWnd = CheckedDeref(manager.GetManagedWindow());
    CB::AuiMDIParentFrame& mainfrm = dynamic_cast<CB::AuiMDIParentFrame&>(mgrWnd);
    wxAuiMDIClientWindow& mdiclient = CheckedDeref(mainfrm.GetClientWindow());

    /* wx abbreviates default tab layout, but we deal with
        BoardIDs, so un-abbreviate */
    std::vector<int> pages = tab.pages;
    if (pages.empty())
    {
        pages.resize(mdiclient.GetPageCount());
        std::iota(pages.begin(), pages.end(), 0);
    }

    TabLayoutInfo tabLayoutInfo;
    static_cast<wxAuiDockLayoutInfo&>(tabLayoutInfo) = static_cast<const wxAuiDockLayoutInfo&>(tab);

    // only save pages belonging to doc
    wxList& views = doc.GetViews();
    for (int page : pages)
    {
        wxWindow& tabWindow = CheckedDeref(mdiclient.GetPage(page));
        CB::DocChildFrame& tabFrame = dynamic_cast<CB::DocChildFrame&>(tabWindow);
        wxView& tabView = CheckedDeref(tabFrame.GetView());
        for (auto it = views.begin() ; it != views.end() ; ++it)
        {
            wxView& docView = dynamic_cast<wxView&>(**it);
            if (&docView == &tabView)
            {
                if (dynamic_cast<wxGamProjView*>(&tabView) ||
                    dynamic_cast<wxGsnProjView*>(&tabView))
                {
                    /* there must be exactly one non-board page,
                        and it must be the project/scenario view,
                        so use nullBid to mean project view */
                    tabLayoutInfo.boards.push_back(nullBid);
                    if (page == tab.active)
                    {
                        tabLayoutInfo.active = nullBid;
                    }
                }
                else
                {
                    CPlayBoardPanelView& wnd = dynamic_cast<CPlayBoardPanelView&>(tabView);
                    BoardID id = wnd.GetPanel().m_pPBoard->GetBoard()->GetSerialNumber();
                    tabLayoutInfo.boards.push_back(id);
                    if (page == tab.active)
                    {
                        tabLayoutInfo.active = id;
                    }
                }
                break;
            }
        }
    }

    wxASSERT(tab.pinned.empty());

    /* It's possible that a tab control will only contain pages
        from a different doc.  If so, don't save it. */
    if (!tabLayoutInfo.boards.empty())
    {
        tabLayoutInfos.emplace_back(std::move(tabLayoutInfo));
    }
}

// Called after the last call to SaveNotebook(), does nothing by default.
// We save WinState info
void CGpWinStateMgr::GpSerializer::AfterSaveNotebooks()
{
    Serializer::AfterSaveNotebooks();

    // ignore pages belonging to other docs
    wxList& views = doc.GetViews();
    for (wxObject* o : views)
    {
        CB::View& view = dynamic_cast<CB::View&>(CheckedDeref(o));
        BoardID bid;
        if (dynamic_cast<wxGamProjView*>(&view))
        {
            /* there must be exactly one non-board page,
                and it must be the project view,
                so use nullBid to mean project view */
            bid = nullBid;
        }
        else if (dynamic_cast<CPlayBoardPanelView*>(&view))
        {
            // the view associated with the frame
            CPlayBoardPanelView& wnd = dynamic_cast<CPlayBoardPanelView&>(view);
            bid = wnd.GetPanel().m_pPBoard->GetBoard()->GetSerialNumber();
        }
        else
        {
            // ignore views nested in CPlayBoardPanelView
            continue;
        }
        CB::DocChildFrame& frame = view.GetFrame();
        std::vector<std::byte> winstate;
        {
            bool bOK;
            CMemFile file;
            {
                CArchive ar(&file, CArchive::store);
                CB::SetFeatures(ar, Features(features));
                WinStateEvent event(frame, ar, false);
                bOK = frame.ProcessWindowEvent(event) &&
                        event.GetResult() &&
                        *event.GetResult();
                wxASSERT(bOK ||
                            dynamic_cast<wxGamProjView*>(&view));
            }
            if (bOK)
            {
                file.SeekToBegin();
                void* start;
                void* max;
                UINT len = file.GetBufferPtr(CMemFile::bufferRead, static_cast<UINT>(-1), &start, &max);
                wxASSERT(static_cast<std::byte*>(max) - static_cast<std::byte*>(start) == len);
                winstate.assign(static_cast<std::byte*>(start), static_cast<std::byte*>(max));
            }
        }
        winstates[bid] = std::move(winstate);
    }
}

// Called before doing anything else, does nothing by default.
/* For us, store pre-Deserialize state so we can
    maintain it through LoadNotebookTabs() */
void CGpWinStateMgr::GpSerializer::BeforeLoad()
{
    wxWindow& mgrWnd = CheckedDeref(manager.GetManagedWindow());
    CB::AuiMDIParentFrame& mainfrm = dynamic_cast<CB::AuiMDIParentFrame&>(mgrWnd);
    wxAuiNotebook& notebook = CheckedDeref(mainfrm.GetClientWindow());
    /* KLUDGE:  unfortunately, wx creates the new doc's frame
        before starting to read the file, so we need to remove
        the new doc's frame from the pre-Deserialize state */
    size_t selection = value_preserving_cast<size_t>(notebook.GetSelection());
    wxASSERT(selection + size_t(1) == notebook.GetPageCount());
    wxString caption = notebook.GetPageText(selection);
    wxBitmap bitmap = notebook.GetPageBitmap(selection);
    wxWindow& extraFrame = CheckedDeref(notebook.GetPage(selection));
    CB_VERIFY(notebook.RemovePage(selection));

    memSerializer = MakeOwner<MemSerializer>(notebook);

    // and now undo the remove
    notebook.AddPage(&extraFrame, caption, true, bitmap);
    wxASSERT(value_preserving_cast<size_t>(notebook.GetSelection()) == selection);
    wxASSERT(notebook.GetPageCount() == selection + size_t(1));
}

// Load information about all the tab controls in the pane containing
// wxAuiNotebook with the given name.
std::vector<wxAuiTabLayoutInfo> CGpWinStateMgr::GpSerializer::LoadNotebookTabs(const wxString& name)
{
    wxASSERT(name == "mdiclient");
    wxWindow& mgrWnd = CheckedDeref(manager.GetManagedWindow());
    CB::AuiMDIParentFrame& mainfrm = dynamic_cast<CB::AuiMDIParentFrame&>(mgrWnd);
    wxAuiNotebook& notebook = CheckedDeref(mainfrm.GetClientWindow());

    std::vector<wxAuiTabLayoutInfo> retval;

    // retval should start with memSerializer content
    retval = std::move(memSerializer->tabLayoutInfos);

    /* Now add loaded file's windows to matching tab controls.
        (If no matching tab control, create a new one.) */
    for (const TabLayoutInfo& cbTab : tabLayoutInfos)
    {
        wxAuiTabLayoutInfo test1;
        static_cast<wxAuiDockLayoutInfo&>(test1) = static_cast<const wxAuiDockLayoutInfo&>(cbTab);

        // if retval already has a wxTab matching cbTab, use it
        wxAuiTabLayoutInfo* wxTab = nullptr;
        for (wxAuiTabLayoutInfo& test2 : retval)
        {
            if (test1.dock_direction == test2.dock_direction &&
                test1.dock_layer == test2.dock_layer &&
                test1.dock_row == test2.dock_row &&
                test1.dock_pos == test2.dock_pos)
            {
                wxTab = &test2;
                break;
            }
        }
        // if no matching tab control, add to retval
        if (!wxTab)
        {
            retval.push_back(test1);
            wxTab = &retval.back();
            // and finish initializing tab control
            wxTab->active = 0;
        }

        wxASSERT((wxTab->dock_direction != wxAUI_DOCK_CENTRE) ||
                    (wxTab->dock_layer == 0 && wxTab->dock_row == 0 && wxTab->dock_pos == 0));
        // now add the boards to the tab control
        CPBoardManager& boardMgr = doc.GetPBoardManager();
        for (BoardID bid : cbTab.boards)
        {
            CB::View* view;
            // nullBid means project view
            if (bid == nullBid)
            {
                view = &doc.FindProjectOrScenarioView();
            }
            else
            {
                CPlayBoard& brd = CheckedDeref(boardMgr.GetPBoardBySerial(bid));
                wxPlayBoardView& brdView = CheckedDeref(doc.MakeSurePBoardVisible(brd));
                view = &brdView;
            }
            wxWindow& frame = view->GetFrame();
            int page = notebook.FindPage(&frame);

            wxTab->pages.push_back(page);
            if (cbTab.active && *cbTab.active == bid)
            {
                wxTab->active = page;
            }
        }
        // wx treats empty pages as abbreviation for all
        wxASSERT(!wxTab->pages.empty());
    }

    return retval;
}

// If any pages haven't been assigned to any tab control after restoring
// the pages order, they are passed to this function to determine what to
// do with them.
//
// By default, it returns true without modifying the output arguments,
// which results in the page being appended to the main tab control. It may
// also modify tabCtrl and tabIndex arguments to modify where the page
// should appear or return false to remove the page from the notebook
// completely.
bool CGpWinStateMgr::GpSerializer::HandleOrphanedPage(wxAuiNotebook& book,
                            int page,
                            wxAuiTabCtrl** tabCtrl,
                            int* tabIndex)
{
    wxASSERT(!"should not have orphaned pages");
    return Serializer::HandleOrphanedPage(book, page, tabCtrl, tabIndex);
}

// Called after restoring everything, calls Update() on the manager by
// default.
// We restore WinState info
/* Restore WinState info after wxAuiManager::Update()
    because some WinState data depends on window size */
void CGpWinStateMgr::GpSerializer::AfterLoad()
{
    Serializer::AfterLoad();

    for (const auto& pair : winstates)
    {
        BoardID bid = pair.first;
        const std::vector<std::byte>& winstate = pair.second;
        CB::View* view;
        if (bid == nullBid)
        {
            view = &doc.FindProjectOrScenarioView();
        }
        else
        {
            CPlayBoard& pbrd = CheckedDeref(doc.GetPBoardManager().
                                                    GetPBoardBySerial(bid));
            CPlayBoardView& wnd = CheckedDeref(doc.FindPBoardView(pbrd));
            view = &static_cast<CB::View&>(wnd);
        }
        wxWindow& frame = view->GetFrame();
        // const_cast<> safe since file is only read
        CMemFile file(const_cast<BYTE*>(reinterpret_cast<const BYTE*>(winstate.data())), value_preserving_cast<unsigned>(winstate.size()));
        CArchive ar(&file, CArchive::load);
        CB::SetFeatures(ar, Features(features));
        WinStateEvent event(frame, ar, true);
        bool bOK = frame.ProcessWindowEvent(event) &&
                    event.GetResult() &&
                    *event.GetResult();
        wxASSERT(bOK ||
                    bid == nullBid);
    }
}

void CGpWinStateMgr::GpSerializer::TabLayoutInfo::Store(CArchive& ar) const
{
    ar  << static_cast<const wxAuiDockLayoutInfo&>(*this)
        << boards
        << active;
}

void CGpWinStateMgr::GpSerializer::TabLayoutInfo::Load(CArchive& ar)
{
    ar  >> static_cast<wxAuiDockLayoutInfo&>(*this)
        >> boards
        >> active;
}

CGpWinStateMgr::GpSerializer::MemSerializer::MemSerializer(wxAuiNotebook& b) :
    book(b)
{
    book.SaveLayout("MemSerializer", *this);
}

// Called to save information about a single tab control in the given
// notebook.
void CGpWinStateMgr::GpSerializer::MemSerializer::SaveNotebookTabControl(const wxAuiTabLayoutInfo& tab)
{
    // TEMP: see https://github.com/wxWidgets/wxWidgets/pull/26679
    wxASSERT((tab.dock_direction != wxAUI_DOCK_CENTRE) ||
                (tab.dock_layer == 0 && tab.dock_row == 0 && tab.dock_pos == 0));
    tabLayoutInfos.emplace_back(tab);
    // wx abbreviates default tab layout, so un-abbreviate
    std::vector<int>& pages = tabLayoutInfos.back().pages;
    if (pages.empty())
    {
        pages.resize(book.GetPageCount());
        std::iota(pages.begin(), pages.end(), 0);
    }
}
