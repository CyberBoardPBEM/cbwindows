// WStateGp.h - classes used to manage player program window state.
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

#ifndef _WSTATEGP_H
#define _WSTATEGP_H

#ifndef _WINSTATE_H
#include "WinState.h"
#endif

class CGpWinStateMgr : public CWinStateManager
{
public:
    using CWinStateManager::CWinStateManager;

protected:
    CGamDoc& GetDocument() { return *m_pDoc; }

    wxFrame& OnGetFrameForWinStateElement(const CWinStateElement& pWse) override;
    void OnAnnotateWinStateElement(CWinStateElement& pState, const wxFrame& pWnd) override;

    OwnerPtr<Serializer> NewSerializer(const Features& ftrs, wxAuiManager& mgr, CGamDoc& d) override
    {
        return MakeOwner<GpSerializer>(ftrs, mgr, d);
    }

private:
    class GpSerializer : public Serializer
    {
    public:
        using Serializer::Serializer;

        // output direction of Serialize(CArchive&)
        void Store(CArchive& ar) const override;
        // input direction of Serialize(CArchive&)
        void Load(CArchive& ar) override;

        // AuiSerializer
        // Called to save information about a single tab control in the given
        // notebook.
        void SaveNotebookTabControl(const wxAuiTabLayoutInfo& tab) override;
        // Called after the last call to SaveNotebook(), does nothing by default.
        // We save WinState info
        void AfterSaveNotebooks() override;

        // AuiDeserializer
        // Called before doing anything else, does nothing by default.
        /* For us, store pre-Deserialize state so we can
            maintain it through LoadNotebookTabs() */
        void BeforeLoad() override;
        // Load information about all the tab controls in the pane containing
        // wxAuiNotebook with the given name.
        std::vector<wxAuiTabLayoutInfo> LoadNotebookTabs(const wxString& name) override;
        // If any pages haven't been assigned to any tab control after restoring
        // the pages order, they are passed to this function to determine what to
        // do with them.
        //
        // By default, it returns true without modifying the output arguments,
        // which results in the page being appended to the main tab control. It may
        // also modify tabCtrl and tabIndex arguments to modify where the page
        // should appear or return false to remove the page from the notebook
        // completely.
        bool HandleOrphanedPage(wxAuiNotebook& book,
                                   int page,
                                   wxAuiTabCtrl** tabCtrl,
                                   int* tabIndex) override;
        // Called after restoring everything, calls Update() on the manager by
        // default.
        // We restore WinState info
        /* Restore WinState info after wxAuiManager::Update()
            because some WinState data depends on window size */
        void AfterLoad() override;

    private:
        /* we can't serialize wxAuiTabLayoutInfo since page
            indices have no meaning outside current process */
        struct TabLayoutInfo : public wxAuiDockLayoutInfo
        {
            /* there must be exactly one non-board page,
                and it must be the project view,
                so use nullBid to mean project view */
            std::vector<BoardID> boards;
            // nullopt means no active view
            // nullBid means project view active
            std::optional<BoardID> active;

            /* inline because an implementation in the global
                namespace can't refer to private class */
            friend CArchive& operator<<(CArchive& ar, const TabLayoutInfo& info)
            {
                info.Store(ar);
                return ar;
            }
            friend CArchive& operator>>(CArchive& ar, TabLayoutInfo& info)
            {
                info.Load(ar);
                return ar;
            }

        private:
            // impl <<
            void Store(CArchive& ar) const;
            // impl >>
            void Load(CArchive& ar);
        };
        std::vector<TabLayoutInfo> tabLayoutInfos;
        class less
        {
        public:
            bool operator()(BoardID left, BoardID right) const
            {
                return static_cast<BoardID::UNDERLYING_TYPE>(left) < static_cast<BoardID::UNDERLYING_TYPE>(right);
            }
        };
        std::map<BoardID, std::vector<std::byte>, less> winstates;

        /* wx expects LoadNotebookTabs() to report all pages,
            not just new ones, so we need BeforeLoad() to
            preserve the pre-existing page layout.  This
            preservation must save all tabs, not just one doc's,
            and never goes outside process, so it's too different
            from CWinStateManager::Serializer to bother sharing
            implementation */
        struct MemSerializer : public wxAuiBookSerializer
        {
            MemSerializer(wxAuiNotebook& book);
            // Called before starting to save information about the tabs in the
            // notebook in the AUI pane with the given name.
            void BeforeSaveNotebook(const wxString& /*name*/) override {}
            // Called to save information about a single tab control in the given
            // notebook.
            void SaveNotebookTabControl(const wxAuiTabLayoutInfo& tab) override;

            wxAuiNotebook& book;
            std::vector<wxAuiTabLayoutInfo> tabLayoutInfos;
        };
        OwnerOrNullPtr<MemSerializer> memSerializer;
    };
};

#endif
