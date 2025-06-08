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
    wxDocument* GetCurrentDocument();

    wxView* GetActiveView() const;

    CDockTilePalette& GetDockingTileWindow() { return CheckedDeref(m_wndTilePal); }

    BOOL IsTilePaletteOn() { return m_bTilePalOn; }

// Operations
public:
    void SaveProfileSettings();
    void RestoreProfileSettings();
    void AuiScheduleUpdate() { auiMgrScheduleUpdate = true; }

// Implementation
public:
    void OnIdle();
    void OnPaneClose(wxAuiManagerEvent& event);

    virtual ~CMainFrame();
#if 0
#ifdef _DEBUG
    void AssertValid() const override;
    void Dump(CDumpContext& dc) const override;
#endif
#endif
    void UpdatePaletteWindow(wxWindow& pWnd, const wxClassInfo** pRtc, BOOL IsOn);
    BOOL IsQualifyingView(wxWindow& pWnd, const wxClassInfo** pRtc);

 protected: // control bar embedded members
#if 0
    CMFCMenuBar     m_wndMenuBar;       // Main menu
#endif
    wxAuiToolBar*   m_wndToolBar;       // Main toolbar
    wxAuiToolBar*   m_wndToolPal;       // Button tool palette for board draw
    wxAuiToolBar*   m_wndIToolPal;      // Button tool palette for image edit
    CColorPalette*  m_wndColorPal;      // Color tool palette window
    CDockTilePalette* m_wndTilePal;     // Container window for tile palette
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
#endif
    void OnWindowToolPal(wxCommandEvent& event);
    void OnWindowIToolPal(wxCommandEvent& event);
    void OnWindowColorPal(wxCommandEvent& event);
    void OnUpdateWindowToolPal(wxUpdateUIEvent& pCmdUI);
    void OnUpdateWindowIToolPal(wxUpdateUIEvent& pCmdUI);
    void OnUpdateWindowColorPal(wxUpdateUIEvent& pCmdUI);
    void OnUpdateEnable(wxUpdateUIEvent& pCmdUI);
    void OnUpdateDisable(wxUpdateUIEvent& pCmdUI);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnHelpIndex();
#endif
    void OnWindowTilePalette(wxCommandEvent& event);
    void OnUpdateWindowTilePalette(wxUpdateUIEvent& pCmdUI);
    void OnToggleColorPalette(wxCommandEvent& event);
    void OnToggleTilePalette(wxCommandEvent& event);
    void OnTile(wxCommandEvent& event);
    void OnUpdateTile(wxUpdateUIEvent& pCmdUI);
    void OnViewStatusBar(wxCommandEvent& event);
    void OnUpdateViewStatusBar(wxUpdateUIEvent& pCmdUI);

    wxDECLARE_EVENT_TABLE();

private:
    wxAuiManager auiManager;
    /* performing all idle updates as a single transaction
        improves AUI's sizing choices */
    bool auiMgrScheduleUpdate = false;
};

inline CMainFrame* GetMainFrame()
{
    // KLUDGE:  wx may return non-mainframe during shutdown
    wxWindow* w = CB::pGetMainWndWx();
    return w ? dynamic_cast<CMainFrame*>(w) : nullptr;
}

#endif

