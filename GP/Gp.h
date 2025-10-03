// Gp.h : main header file for the GP application
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

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include    "resource.h"                // main symbols

/////////////////////////////////////////////////////////////////////////////

#include    "versions.h"
#include    "FrmMain.h"

class CPlayBoard;
class CDrawObj;

/////////////////////////////////////////////////////////////////////////////

#define IDW_TOOLBAR_MAIN        AFX_IDW_TOOLBAR
#define IDW_TOOLBAR_VIEW        59400
#define IDW_TOOLBAR_MOVE        59401
#define IDW_TOOLBAR_PLAY        59402
#define IDW_TRAY_PALETTEA       59403
#define IDW_TRAY_PALETTEB       59404
#define IDW_MARK_PALETTE        59405
#define IDW_MESSAGE_WINDOW      59406

/////////////////////////////////////////////////////////////////////////////

#define WM_ROTATEPIECE_DELTA    (WM_USER + 210) // WPARAM = (int)relative rotation delta
class RotatePieceDeltaEvent : public wxEvent
{
public:
    RotatePieceDeltaEvent(int delta);

    int GetDelta() const { return delta; }

    wxEvent* Clone() const override { return new RotatePieceDeltaEvent(*this); }

private:
    const int delta;
};
wxDECLARE_EVENT(WM_ROTATEPIECE_DELTA_WX, RotatePieceDeltaEvent);
inline RotatePieceDeltaEvent::RotatePieceDeltaEvent(int d) :
    wxEvent(wxID_ANY, WM_ROTATEPIECE_DELTA_WX),
    delta(d)
{
}
typedef void (wxEvtHandler::* RotatePieceDeltaEventFunction)(RotatePieceDeltaEvent&);
#define RotatePieceDeltaEventHandler(func) wxEVENT_HANDLER_CAST(RotatePieceDeltaEventFunction, func)
#define EVT_ROTATEPIECE_DELTA(func) \
    wx__DECLARE_EVT0(WM_ROTATEPIECE_DELTA_WX, RotatePieceDeltaEventHandler(func))

#define WM_CENTERBOARDONPOINT   (WM_USER + 211) // WPARAM = POINT* in board coords
class CenterBoardOnPointEvent : public wxEvent
{
public:
    CenterBoardOnPointEvent(const wxPoint& point);

    const wxPoint& GetPoint() const { return point; }

    wxEvent* Clone() const override { return new CenterBoardOnPointEvent(*this); }

private:
    const wxPoint point;
};
wxDECLARE_EVENT(WM_CENTERBOARDONPOINT_WX, CenterBoardOnPointEvent);
inline CenterBoardOnPointEvent::CenterBoardOnPointEvent(const wxPoint& p) :
    wxEvent(wxID_ANY, WM_CENTERBOARDONPOINT_WX),
    point(p)
{
}
typedef void (wxEvtHandler::* CenterBoardOnPointEventFunction)(CenterBoardOnPointEvent&);
#define CenterBoardOnPointEventHandler(func) wxEVENT_HANDLER_CAST(CenterBoardOnPointEventFunction, func)
#define EVT_CENTERBOARDONPOINT(func) \
    wx__DECLARE_EVT0(WM_CENTERBOARDONPOINT_WX, CenterBoardOnPointEventHandler(func))

#define WM_SHOWPLAYINGBOARD     (WM_USER + 212) // WPARAM = size_t Playing Board Index
class ShowPlayingBoardEvent : public wxEvent
{
public:
    ShowPlayingBoardEvent(size_t playBrdIndex);

    size_t GetPlayingBoardIndex() const { return playBrdIndex; }

    wxEvent* Clone() const override { return new ShowPlayingBoardEvent(*this); }

private:
    const size_t playBrdIndex;
};
wxDECLARE_EVENT(WM_SHOWPLAYINGBOARD_WX, ShowPlayingBoardEvent);
inline ShowPlayingBoardEvent::ShowPlayingBoardEvent(size_t pbi) :
    wxEvent(wxID_ANY, WM_SHOWPLAYINGBOARD_WX),
    playBrdIndex(pbi)
{
}
typedef void (wxEvtHandler::* ShowPlayingBoardEventFunction)(ShowPlayingBoardEvent&);
#define ShowPlayingBoardEventHandler(func) wxEVENT_HANDLER_CAST(ShowPlayingBoardEventFunction, func)
#define EVT_SHOWPLAYINGBOARD(func) \
    wx__DECLARE_EVT0(WM_SHOWPLAYINGBOARD_WX, ShowPlayingBoardEventHandler(func))

#define WM_WINSTATE_RESTORE     (WM_USER + 213) // No Args. Posted to project window
class WinStateRestoreEvent : public wxEvent
{
public:
    WinStateRestoreEvent();

    wxEvent* Clone() const override { return new WinStateRestoreEvent(*this); }
};
wxDECLARE_EVENT(WM_WINSTATE_RESTORE_WX, WinStateRestoreEvent);
inline WinStateRestoreEvent::WinStateRestoreEvent() :
    wxEvent(wxID_ANY, WM_WINSTATE_RESTORE_WX)
{
}
typedef void (wxEvtHandler::* WinStateRestoreEventFunction)(WinStateRestoreEvent&);
#define WinStateRestoreEventHandler(func) wxEVENT_HANDLER_CAST(WinStateRestoreEventFunction, func)
#define EVT_WINSTATE_RESTORE(func) \
    wx__DECLARE_EVT0(WM_WINSTATE_RESTORE_WX, WinStateRestoreEventHandler(func))

#define WM_SELECT_BOARD_OBJLIST (WM_USER + 214) // WPARAM = CPlayBoard*, LPARAM = const std::vector<CB::not_null<CDrawObj*>>*
class SelectBoardObjListEvent : public wxEvent
{
public:
    SelectBoardObjListEvent(const CPlayBoard& b,
                            const std::vector<CB::not_null<CDrawObj*>>& l);

    const CPlayBoard& GetBoard() const { return board; }
    const std::vector<CB::not_null<CDrawObj*>>& GetObjList() const
    {
        return objList;
    }

    wxEvent* Clone() const override { return new SelectBoardObjListEvent(*this); }

private:
    const CPlayBoard& board;
    const std::vector<CB::not_null<CDrawObj*>>& objList;
};
wxDECLARE_EVENT(WM_SELECT_BOARD_OBJLIST_WX, SelectBoardObjListEvent);
inline SelectBoardObjListEvent::SelectBoardObjListEvent(const CPlayBoard& b,
                            const std::vector<CB::not_null<CDrawObj*>>& l) :
    wxEvent(wxID_ANY, WM_SELECT_BOARD_OBJLIST_WX),
    board(b),
    objList(l)
{
}
typedef void (wxEvtHandler::* SelectBoardObjListEventFunction)(SelectBoardObjListEvent&);
#define SelectBoardObjListEventHandler(func) wxEVENT_HANDLER_CAST(SelectBoardObjListEventFunction, func)
#define EVT_SELECT_BOARD_OBJLIST(func) \
    wx__DECLARE_EVT0(WM_SELECT_BOARD_OBJLIST_WX, SelectBoardObjListEventHandler(func))

#define WM_MESSAGEBOX           (WM_USER + 215) // WPARAM = Opts. LPARAM = Msg ID or Ptr
enum
{                   // Opts passed in WPARAM
    WMB_MSGID = 0,      // LPARAM is message ID
    WMB_PTR_STATIC = 1, // LPARAM is text pointer. (don't deallocate!)
    WMB_PTR_NEW = 2,    // LPARAM is text pointer allocated with 'new' operator.
    WMB_PTR_CBSTRING = 3,// LPARAM is CB::string object pointer.
};

#define WM_PALETTE_HIDE         (WM_USER + 216)

/////////////////////////////////////////////////////////////////////////////
// Context menu offset definitions.
//
// The values in this enum must be in the same order as the menu resource
// containing the right click popups.

enum { MENU_PV_MOVEMODE = 0, MENU_PV_PLAYMODE, MENU_PV_SCNMODE,
    MENU_PJ_GSN_DEFAULT, MENU_PJ_GAM_DEFAULT, MENU_PV_PIECE_TRAY, MENU_MV_RICHEDIT,
    MENU_ACT_TURNOVER, MENU_PV_SELCT_BOX, };

/////////////////////////////////////////////////////////////////////////////
// CGpApp:

class CGpApp : public CWinAppEx
{
public:
    static CGpApp& Get();
private:
    class CwxGpApp;
    friend wxMFCApp<CGpApp>;
    CGpApp();
public:

// Operations
public:

// Attributes
public:
    CDocument* GetCurrentDocument();

    CMultiDocTemplate* m_pBrdViewTmpl;

    CMultiDocTemplate* GetScnenarioDocTemplate()
        { return m_pScnDocTemplate; }

    DWORD   m_dwHtmlHelpCookie;
    BOOL    m_bDisableHtmlHelp;
    HANDLE  m_hHtmlProcessHandle;

// Methods
public:
    void DoHelpShellLaunch();
    void DoHelpContents();
    void DoHelpContext(uintptr_t dwContextData);
    void DoHelpTopic(const CB::string& pszTopic);
    BOOL DoHelpTipHelp(HELPINFO* pHelpInfo, DWORD* dwIDArray);
    void DoHelpWhatIsHelp(CWnd* pWndCtrl, DWORD* dwIDArray);

    BOOL DispatchMessages();
    void Delay(int nMilliSecs, BOOL* pbBailOut = NULL);

// Overrides
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual BOOL OnIdle(LONG lCount);

    virtual BOOL PreTranslateMessage(MSG *pMsg);

// Implementation
    CMultiDocTemplate* m_pScnDocTemplate;

    afx_msg void OnAppAbout();
    afx_msg void OnHelpWebsite();
    afx_msg void OnHelpReleases();
    afx_msg BOOL OnOpenRecentFile(UINT nID);
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

inline CGpApp* GetApp() { return (CGpApp*) AfxGetApp(); }
inline CMainFrame* GetMainFrame() { return (CMainFrame*)(GetApp()->m_pMainWnd); }

/////////////////////////////////////////////////////////////////////////////

