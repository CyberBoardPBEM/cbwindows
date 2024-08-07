// Gm.h : main header file for the GM application
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

#include "resource.h"           // main symbols

/////////////////////////////////////////////////////////////////////////////

#include "versions.h"

/////////////////////////////////////////////////////////////////////////////

#define     IDW_TOOLBAR_MAIN         AFX_IDW_TOOLBAR
#define     IDW_TOOLBAR_IMAGE_DRAW   59400
#define     IDW_TOOLBAR_BOARD_DRAW   59401
#define     IDW_COLOR_PALETTE        59402
#define     IDW_TILE_PALETTE         59403

/////////////////////////////////////////////////////////////////////////////
// Our clipboard formats

extern const wxDataFormat CF_TILEIMAGES;      // Dword count, dword small colorref, full Dib,
                                              // half Dib, repeat...
extern const wxDataFormat CF_TIDLIST;         // dword count followed array of DWORDS.
extern const wxDataFormat CF_GBOXID;          // dword gameboxid, dword cur process id

/////////////////////////////////////////////////////////////////////////////

#define WM_SETCOLOR         (WM_USER + 666)     // See COLORPAL.H
class SetColorEvent : public wxEvent
{
public:
    SetColorEvent(int subId, wxColour color);

    int GetSubId() const { return subId; }
    wxColour GetColor() const { return color; }

    wxEvent* Clone() const override { return new SetColorEvent(*this); }

private:
    const int subId;
    const wxColour color;
};
wxDECLARE_EVENT(WM_SETCOLOR_WX, SetColorEvent);
inline SetColorEvent::SetColorEvent(int s, wxColour c) :
    wxEvent(wxID_ANY, WM_SETCOLOR_WX),
    subId(s),
    color(c)
{
}
typedef void (wxEvtHandler::* SetColorEventFunction)(SetColorEvent&);
#define SetColorEventHandler(func) wxEVENT_HANDLER_CAST(SetColorEventFunction, func)
#define EVT_SETCOLOR(func) \
    wx__DECLARE_EVT0(WM_SETCOLOR_WX, SetColorEventHandler(func))

#define WM_SETLINEWIDTH     (WM_USER + 667)     // See COLORPAL.H
class SetLineWidthEvent : public wxEvent
{
public:
    SetLineWidthEvent(int width);

    int GetWidth() const { return width; }

    wxEvent* Clone() const override { return new SetLineWidthEvent(*this); }

private:
    const int width;
};
wxDECLARE_EVENT(WM_SETLINEWIDTH_WX, SetLineWidthEvent);
inline SetLineWidthEvent::SetLineWidthEvent(int w) :
    wxEvent(wxID_ANY, WM_SETLINEWIDTH_WX),
    width(w)
{
}
typedef void (wxEvtHandler::* SetLineWidthEventFunction)(SetLineWidthEvent&);
#define SetLineWidthEventHandler(func) wxEVENT_HANDLER_CAST(SetLineWidthEventFunction, func)
#define EVT_SETLINEWIDTH(func) \
    wx__DECLARE_EVT0(WM_SETLINEWIDTH_WX, SetLineWidthEventHandler(func))

#define WM_SETCUSTOMCOLOR   (WM_USER + 668)     // See COLORPAL.H
class SetCustomColorEvent : public wxEvent
{
public:
    SetCustomColorEvent(const std::vector<wxColour>& colors);

    const std::vector<wxColour>& GetColors() const { return colors; }

    wxEvent* Clone() const override { return new SetCustomColorEvent(*this); }

private:
    std::vector<wxColour> colors;
};
wxDECLARE_EVENT(WM_SETCUSTOMCOLOR_WX, SetCustomColorEvent);
inline SetCustomColorEvent::SetCustomColorEvent(const std::vector<wxColour>& c) :
    wxEvent(wxID_ANY, WM_SETCUSTOMCOLOR_WX),
    colors(c)
{
}
typedef void (wxEvtHandler::* SetCustomColorEventFunction)(SetCustomColorEvent&);
#define SetCustomColorEventHandler(func) wxEVENT_HANDLER_CAST(SetCustomColorEventFunction, func)
#define EVT_SETCUSTOMCOLOR(func) \
    wx__DECLARE_EVT0(WM_SETCUSTOMCOLOR_WX, SetCustomColorEventHandler(func))

#define WM_PALETTE_HIDE     (WM_USER + 669)
wxDECLARE_EVENT(WM_PALETTE_HIDE_WX, wxCommandEvent);

/////////////////////////////////////////////////////////////////////////////
// Context menu offset definitions.
//
// The values in this enum must be in the same order as the menu resource
// containing the right click popups.

enum { MENU_BV_DRAWING = 0, MENU_IV_BITEDIT, MENU_PJ_DEFAULT,
    MENU_PJ_TILELIST, MENU_PJ_PIECELIST, MENU_PJ_MARKERLIST };

/////////////////////////////////////////////////////////////////////////////
// CGmApp:
// See gm.cpp for the implementation of this class
//

class CGmApp : public CWinAppEx
{
public:
    static CGmApp& Get();
private:
    class CwxGmApp;
    friend wxMFCApp<CGmApp>;
    CGmApp();
public:

// Atrributes
public:
    DWORD   m_dwHtmlHelpCookie;
    BOOL    m_bDisableHtmlHelp;
    HANDLE  m_hHtmlProcessHandle;
    UINT    m_nAppLook;

    CMultiDocTemplate* m_pMapViewTmpl;
    CMultiDocTemplate* m_pTileEditTmpl;

// Operations
public:
    void DoHelpShellLaunch();
    void DoHelpContents();
    void DoHelpContext(uintptr_t dwContextData);
    void DoHelpTopic(const CB::string& pszTopic);
    BOOL DoHelpTipHelp(HELPINFO* pHelpInfo, DWORD* dwIDArray);
    void DoHelpWhatIsHelp(CWnd* pWndCtrl, DWORD* dwIDArray);

    BOOL DispatchMessages();
    void UpdateAllViewsForAllDocuments(CView* pSender, LPARAM lHint = 0,
        CObject* pHint = NULL);

// Overrides
    virtual BOOL InitInstance();
    virtual int ExitInstance();
    virtual BOOL OnIdle(LONG lCount);

// Implementation

    //{{AFX_MSG(CGmApp)
    afx_msg void OnAppAbout();
    afx_msg void OnHelpWebsite();
    afx_msg void OnHelpReleases();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

class CMainFrame;

inline CGmApp* GetApp() { return (CGmApp*)AfxGetApp(); }
inline CMainFrame* GetMainFrame() { return (CMainFrame*)(GetApp()->m_pMainWnd); }

/////////////////////////////////////////////////////////////////////////////

