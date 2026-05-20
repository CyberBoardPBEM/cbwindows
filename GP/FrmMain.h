// FrmMain.h : interface of the CMainFrame class
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

#ifndef _FRMMAIN_H_
#define _FRMMAIN_H_

#include "LibMfc.h"

class CDockTrayPalette;
class CDockMarkPalette;
class CReadMsgWnd;
class CReadMsgWndContainer;

class CMainFrame : public wxDocParentFrameAny<CB::AuiMDIParentFrame>,
                    public CB::FreezeUntilIdleMixin
{
public:
    CMainFrame();
    ~CMainFrame() override;

// Attributes
public:
    CDocument* GetCurrentDocument();

#if 0
    CDockMarkPalette& GetDockingMarkerWindow() { return *m_wndMarkPal; }
    CDockTrayPalette& GetDockingTrayAWindow() { return *m_wndTrayPalA; }
    CDockTrayPalette& GetDockingTrayBWindow() { return *m_wndTrayPalB; }

    CReadMsgWnd&      GetMessageWindow();
#endif

    // unfortunately, wx isn't const correct here
    CB::StatusBar* GetStatusBar() const override { return const_cast<CB::StatusBar*>(&*m_wndStatusBar); }

// Operations
public:
#if 0
    void UpdatePaletteWindow(CWnd& pWnd, BOOL bIsOn);
    void ShowPalettePanes(BOOL bShow);
#endif
    void OnIdle();
#if 0
    BOOL OnCloseMiniFrame(CPaneFrameWnd* pWnd) override;
    BOOL OnCloseDockingPane(CDockablePane* pWnd) override;
#endif

// Implementation
protected:
#if 0
    CMFCMenuBar   m_wndMenuBar;
#endif
    CB::propagate_const<wxAuiToolBar*> m_wndToolBar;       // Main toolbar
    CB::propagate_const<wxAuiToolBar*> m_wndTBarView;
    CB::propagate_const<wxAuiToolBar*> m_wndTBarPlay;
    CB::propagate_const<CB::AuiToolBar*> m_wndTBarMove;

    CB::propagate_const<CB::StatusBar*> m_wndStatusBar = nullptr;

#if 0
    OwnerPtr<CReadMsgWndContainer> m_wndMessage;

    OwnerPtr<CDockMarkPalette> m_wndMarkPal;
    OwnerPtr<CDockTrayPalette> m_wndTrayPalA;
    OwnerPtr<CDockTrayPalette> m_wndTrayPalB;

    CWindowPos    m_wndPosition;
#endif
     //@@@@@@ CMDIWndTab    m_wndMDITabWindow;

// Implementation
public:
#if 0
    BOOL LoadFrame(UINT nIDResource,
        DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
        CWnd *pParentWnd = NULL, CCreateContext *pContext = NULL) override;
#endif

#if 0
#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif
#endif

// Generated message map functions
protected:
#if 0
    void WinHelp(DWORD_PTR dwData, UINT nCmd) override;
#endif

#if 0
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
#endif
    void OnUpdateEnable(wxUpdateUIEvent& pCmdUI);
    void OnTile(wxCommandEvent& event);
    void OnUpdateTile(wxUpdateUIEvent& pCmdUI);
    void OnViewStatusBar(wxCommandEvent& event);
    void OnUpdateViewStatusBar(wxUpdateUIEvent& pCmdUI);
    void OnUpdateDisable(wxUpdateUIEvent& pCmdUI);

    wxDECLARE_EVENT_TABLE();

private:
    typedef wxDocParentFrameAny<CB::AuiMDIParentFrame> BASE;

    wxAuiManager auiManager;
#if 0
    CMFCDropDownToolBar m_flipToolbar;
#endif
};

inline CMainFrame* GetMainFrame()
{
    // KLUDGE:  wx may return non-mainframe during shutdown
    wxWindow* w = CB::pGetMainWndWx();
    return w ? dynamic_cast<CMainFrame*>(w) : nullptr;
}

/////////////////////////////////////////////////////////////////////////////
#endif // _FRMMAIN_H_
