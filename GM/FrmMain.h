// FrmMain.h
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
#ifndef _FRMMAIN_H
#define _FRMMAIN_H

#include "LibMfc.h"

#ifndef     _GMDOC_H
#include    "gmdoc.h"
#endif

#ifndef     _GDITOOLS_H
#include    "gditools.h"
#endif

#ifndef     _PALCOLOR_H
#include    "palcolor.h"
#endif

#ifndef     _FRMDOCKTILE_H
#include    "frmdocktile.h"
#endif

class CMainFrame : public wxDocParentFrameAny<CB::wxAuiMDIParentFrame>
{
public:
    CMainFrame();

// Attributes
public:
    CDocument* GetCurrentDocument();

    CView* GetActiveView() const;

#if 0
    CDockTilePalette& GetDockingTileWindow() { return m_wndTilePal; }
#endif

    BOOL IsTilePaletteOn() { return m_bTilePalOn; }

// Operations
public:
    void SaveProfileSettings();
    void RestoreProfileSettings();

// Implementation
public:
    void OnIdle();
#if 0
    BOOL OnCloseMiniFrame(CPaneFrameWnd* pWnd) override;
    BOOL OnCloseDockingPane(CDockablePane* pWnd) override;
#endif

    virtual ~CMainFrame();
#if 0
#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif
#endif
    void UpdatePaletteWindow(CWnd& pWnd, const CRuntimeClass** pRtc, BOOL IsOn);
    BOOL IsQualifyingView(CWnd& pWnd, const CRuntimeClass** pRtc);

 protected: // control bar embedded members
#if 0
    CMFCMenuBar     m_wndMenuBar;       // Main menu
#endif
    wxAuiToolBar*   m_wndToolBar;       // Main toolbar
    wxAuiToolBar*   m_wndToolPal;       // Button tool palette for board draw
    wxAuiToolBar*   m_wndIToolPal;      // Button tool palette for image edit
#if 0
    CDockColorPalette m_wndColorPal;    // Color tool palette window
    CDockTilePalette m_wndTilePal;      // Container window for tile palette
#endif
    CB::wxStatusBar*  m_wndStatusBar;   // Status bar at window bottom

    BOOL            m_bColorPalOn;
    BOOL            m_bTilePalOn;

// Generated message map functions
protected:
#if 0
    void WinHelp(DWORD_PTR dwData, UINT nCmd) override;
#endif

#if 0
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnClose();
    afx_msg void OnWindowToolPal();
    afx_msg void OnWindowIToolPal();
    afx_msg void OnWindowColorPal();
    afx_msg void OnUpdateWindowToolPal(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWindowIToolPal(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWindowColorPal(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDisable(CCmdUI* pCmdUI);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnHelpIndex();
    afx_msg void OnWindowTilePalette();
    afx_msg void OnUpdateWindowTilePalette(CCmdUI* pCmdUI);
    afx_msg void OnToggleColorPalette();
    afx_msg void OnToggleTilePalette();
#endif
    void OnTile(wxCommandEvent& event);
    void OnUpdateTile(wxUpdateUIEvent& event);
    void OnViewStatusBar(wxCommandEvent& event);
    void OnUpdateViewStatusBar(wxUpdateUIEvent& pCmdUI);

    wxDECLARE_EVENT_TABLE();

private:
    wxAuiManager auiManager;
};

inline CMainFrame* GetMainFrame()
{
    // KLUDGE:  wx may return non-mainframe during shutdown
    wxWindow* w = CB::pGetMainWndWx();
    return w ? dynamic_cast<CMainFrame*>(w) : nullptr;
}

#endif

