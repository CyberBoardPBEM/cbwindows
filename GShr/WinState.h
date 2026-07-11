// WinState.h - classes used to manage window state.
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

#ifndef _WINSTATE_H
#define _WINSTATE_H

#include "Board.h"
#include "Versions.h"

///////////////////////////////////////////////////////////////////////

// This message is sent to each frame than holds document views.
// The frame should serialize all the information required to restore
// the frame's *contents* to it's current visual state.

#define WM_WINSTATE   (WM_USER + 300)  // WPARAM = CArchive*, LPARAM = 0 if save, 1 if restore
class WinStateEvent : public wxEvent
{
public:
    WinStateEvent(wxWindow& dest, CArchive& ar, bool restore);

    /* KLUDGE:  unlike MFC, wxSplitterWindow doesn't support 2x2,
                so sometimes need to consume excess window state
                w/o applying it */
    bool GetIgnore() const { return ignore; }
    class SetIgnore
    {
    public:
        SetIgnore(WinStateEvent& event) :
            ignore(event.ignore)
        {
            ignore = true;
        }
        ~SetIgnore()
        {
            ignore = false;
        }
    private:
        bool& ignore;
    };

    CArchive& GetArchive() const { return ar; }

    const std::optional<bool>& GetResult() const { return result; }
    void SetResult(bool b) { result = b; }

    wxEvent* Clone() const override { return new WinStateEvent(*this); }

private:
    CArchive& ar;
    bool ignore = false;
    std::optional<bool> result = std::nullopt;
};
wxDECLARE_EVENT(WM_WINSTATE_WX, WinStateEvent);
inline WinStateEvent::WinStateEvent(wxWindow& dest, CArchive& a, bool restore) :
    wxEvent(wxID_ANY, WM_WINSTATE_WX),
    ar(a)
{
    WXUNUSED_UNLESS_DEBUG(restore);
    wxASSERT(bool(ar.IsLoading()) == restore);
    SetEventObject(&dest);
}
typedef void (wxEvtHandler::* WinStateEventFunction)(WinStateEvent&);
#define WinStateEventHandler(func) wxEVENT_HANDLER_CAST(WinStateEventFunction, func)
#define EVT_WINSTATE(func) \
    wx__DECLARE_EVT0(WM_WINSTATE_WX, WinStateEventHandler(func))

///////////////////////////////////////////////////////////////////////
// Helper class for working with the Windows WINDOWPLACEMENT structure

struct CWinPlacement : public WINDOWPLACEMENT
{
    CWinPlacement();

    friend CArchive& AFXAPI operator<<(CArchive& ar, const CWinPlacement& wndPlace);
    friend CArchive& AFXAPI operator>>(CArchive& ar, CWinPlacement& wndPlace);
};

///////////////////////////////////////////////////////////////////////

class CWinStateManager
{
public:
    CWinStateManager(CGamDoc& pDoc) : m_pDoc(&pDoc) {}

    virtual ~CWinStateManager() = default;

public:
    BOOL GetStateOfOpenDocumentFrames(const Features& fileFeatures);
    void RestoreStateOfDocumentFrames();

    void Serialize(CArchive& ar);

    enum { wincodeUnknown = 0, wincodeMainFrame = 1, wincodeViewFrame = 2, wincodeToolPal = 3 };

protected:
    // non-ftrAuilayout
    struct CWinStateElement
    {
        uint16_t m_wWinCode;           // Generic type of window
        uint16_t m_wUserCode1;         // Used by subclass to refine WinCode
        BoardID m_boardID;          // Used by subclass to refine WinCode
        CWinPlacement m_wndState;   // Window placement information

        std::vector<std::byte> m_pWinStateBfr;       // Serialized window data

        CWinStateElement();
        virtual ~CWinStateElement() = default;
        void Serialize(CArchive& ar);
    };

protected:
    OwnerPtr<CWinStateElement> GetWindowState(const wxFrame& pWnd);
    BOOL RestoreWindowState(wxFrame& pWnd, CWinStateElement& pWse);
    std::vector<RefPtr<wxFrame>> GetDocumentFrameList();

    void ArrangeFrameListInZOrder(std::vector<RefPtr<wxFrame>>& tblFrames);
#if 0
    static BOOL CALLBACK EnumFrames(HWND hWnd, LPARAM dwTblFramePtr);
#endif

    // Required override used to locate or, if necessary, recreate frames.
    // This is called when window states are being restored.
    virtual wxFrame& OnGetFrameForWinStateElement(const CWinStateElement& pWse) /* override */ = 0;

    // Allow subclass to create specialized version of CWinStateElement
    virtual OwnerPtr<CWinStateElement> OnCreateWinStateElement() /* override */ { return MakeOwner<CWinStateElement>(); }

    // Allow subclass to add more information.
    virtual void OnAnnotateWinStateElement(CWinStateElement& pWse, const wxFrame& pWnd) /* override */ {}

protected:
    RefPtr<CGamDoc> m_pDoc;
    // for RestoreStateOfDocumentFrames, which is after ar is destroyed
    int fileVersion = INT_MAX;
    Features fileFeatures;

    // non-ftrAuilayout
    typedef std::list<OwnerPtr<CWinStateElement>> CWinStateList;
    CWinStateList m_pList;            // Win state element list

    // ftrAuilayout
    class Serializer : public wxTopLevelWindow::GeometryStore,
                        public wxAuiSerializer,
                        public wxAuiDeserializer
    {
    public:
        Serializer(const Features& ftrs, wxAuiManager& mgr, CGamDoc& d);

        virtual void Store(CArchive& ar) const;
        virtual void Load(CArchive& ar);

        // GeometryStore
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
        bool SaveValue(const wxString& name, int value) override;

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
        bool RestoreValue(const wxString& name, int* value) const override;

        // AuiSerializer
        // Called before starting to save information about the panes, does nothing
        // by default.
        void BeforeSavePanes() override;
        // Save information about the given pane.
        void SavePane(const wxAuiPaneLayoutInfo& pane) override;

        // Called before starting to save information about the tabs in the
        // notebook in the AUI pane with the given name.
        void BeforeSaveNotebook(const wxString& name) override;

        // AuiDeserializer
        // Load information about all the panes previously saved with SavePane().
        std::vector<wxAuiPaneLayoutInfo> LoadPanes() override;

    protected:
        const Features& features;
        wxAuiManager& manager;
        CGamDoc& doc;

        std::map<CB::string, int32_t> geometry;

        std::vector<wxAuiPaneLayoutInfo> paneLayoutInfos;
    };
    virtual OwnerPtr<Serializer> NewSerializer(const Features& ftrs, wxAuiManager& mgr, CGamDoc& d) = 0;

private:
    OwnerOrNullPtr<Serializer> serializer;
};

CArchive& operator<<(CArchive& ar, const wxAuiDockLayoutInfo& dock);
CArchive& operator>>(CArchive& ar, wxAuiDockLayoutInfo& dock);

CArchive& operator<<(CArchive& ar, const wxAuiPaneLayoutInfo& dock);
/* KLUDGE:  wxAuiPaneLayoutInfo has no default ctor
CArchive& operator>>(CArchive& ar, wxAuiPaneLayoutInfo& dock);
*/
wxAuiPaneLayoutInfo ReadwxAuiPaneLayoutInfo(CArchive& ar);

#endif

