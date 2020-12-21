// Gp.h : main header file for the GP application
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

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include    "resource.h"                // main symbols

/////////////////////////////////////////////////////////////////////////////

#include    "versions.h"
#include    "FrmMain.h"

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
#define WM_CENTERBOARDONPOINT   (WM_USER + 211) // WPARAM = POINT* in board coords
#define WM_SHOWPLAYINGBOARD     (WM_USER + 212) // WPARAM = size_t Playing Board Index
#define WM_WINSTATE_RESTORE     (WM_USER + 213) // No Args. Posted to project window
#define WM_SELECT_BOARD_OBJLIST (WM_USER + 214) // WPARAM = CPlayBoard*, LPARAM = CPtrList*

#define WM_MESSAGEBOX           (WM_USER + 215) // WPARAM = Opts. LPARAM = Msg ID or Ptr
enum
{                   // Opts passed in WPARAM
    WMB_MSGID = 0,      // LPARAM is message ID
    WMB_PTR_STATIC = 1, // LPARAM is text pointer. (don't deallocate!)
    WMB_PTR_NEW = 2,    // LPARAM is text pointer allocated with 'new' operator.
    WMB_PTR_CSTRING = 3,// LPARAM is CString object pointer.
};

#define WM_PALETTE_HIDE         (WM_USER + 216)

/////////////////////////////////////////////////////////////////////////////
// Context menu offset definitions.
//
// The values in this enum must be in the same order as the menu resource
// containing the right click popups.

enum { MENU_PV_MOVEMODE = 0, MENU_PV_PLAYMODE, MENU_PV_SCNMODE,
    MENU_PJ_GSN_DEFAULT, MENU_PJ_GAM_DEFAULT, MENU_PV_PIECE_TRAY, MENU_MV_RICHEDIT };

/////////////////////////////////////////////////////////////////////////////
// CGpApp:

class CGpApp : public CWinAppEx
{
public:
    CGpApp();

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
    void DoHelpContext(DWORD dwContextData);
    void DoHelpTopic(LPCTSTR pszTopic);
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
    afx_msg BOOL OnOpenRecentFile(UINT nID);
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

inline CGpApp* GetApp() { return (CGpApp*) AfxGetApp(); }
inline CMainFrame* GetMainFrame() { return (CMainFrame*)(GetApp()->m_pMainWnd); }

/////////////////////////////////////////////////////////////////////////////

