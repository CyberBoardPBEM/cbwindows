// FrmMain.h : interface of the CMainFrame class
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

#ifndef _FRMMAIN_H_
#define _FRMMAIN_H_

#include "LibMfc.h"

#ifndef _FRMDOCKTRAY_H
#include "FrmDockTray.h"
#endif

#ifndef _FRMDOCKMARK_H
#include "FrmDockMark.h"
#endif

#ifndef _PALREADMSG_H
#include "PalReadMsg.h"
#endif

class CMainFrame : public CMDIFrameWndExCb
{
    DECLARE_DYNAMIC(CMainFrame)
public:
    CMainFrame();
    virtual ~CMainFrame();

// Attributes
public:
    CDocument* GetCurrentDocument();

    CDockMarkPalette* GetDockingMarkerWindow() { return &m_wndMarkPal; }
    CDockTrayPalette* GetDockingTrayAWindow() { return &m_wndTrayPalA; }
    CDockTrayPalette* GetDockingTrayBWindow() { return &m_wndTrayPalB; }

    CReadMsgWnd*      GetMessageWindow() { return &m_wndMessage; }

    CMFCStatusBar* GetStatusBar() { return &m_wndStatusBar; }

    CPalette* GetMasterPalette() { return &m_appPalette; }

// Operations
public:
    void UpdatePaletteWindow(CWnd* pWnd, BOOL bIsOn);
    void ShowPalettePanes(BOOL bShow);
    void OnIdle();

// Implementation
protected:
    CMFCMenuBar   m_wndMenuBar;
    CMFCToolBar   m_wndToolBar;
    CMFCToolBar   m_wndTBarView;
    CMFCToolBar   m_wndTBarPlay;
    CMFCToolBar   m_wndTBarMove;

    CMFCStatusBar m_wndStatusBar;

    CReadMsgWnd   m_wndMessage;

    CDockMarkPalette m_wndMarkPal;
    CDockTrayPalette m_wndTrayPalA;
    CDockTrayPalette m_wndTrayPalB;

    CWindowPos    m_wndPosition;
     //@@@@@@ CMDIWndTab    m_wndMDITabWindow;

    CPalette        m_appPalette;       // Master app GDI palette

// Implementation
public:
    BOOL BuildAppGDIPalette();


    virtual BOOL LoadFrame(UINT nIDResource,
        DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
        CWnd *pParentWnd = NULL, CCreateContext *pContext = NULL);

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
    void WinHelp(DWORD_PTR dwData, UINT nCmd) override;

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnUpdateDisable(CCmdUI* pCmdUI);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnHelpIndex();
    afx_msg void OnClose();
    afx_msg void OnToggleMarkPalette();
    afx_msg void OnToggleTrayPaletteA();
    afx_msg void OnToggleTrayPaletteB();
    afx_msg void OnToggleMessagePalette();
    afx_msg LRESULT OnDDEExecute(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMessageBox(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif // _FRMMAIN_H_
