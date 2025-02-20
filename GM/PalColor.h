// ColorPal.h : header file
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

#ifndef _PALCOLOR_H
#define _PALCOLOR_H

inline bool mouseFore(const wxMouseEvent& event)
{
    return event.GetButton() == wxMOUSE_BTN_LEFT &&
            event.GetModifiers() == wxMOD_NONE;
}
inline bool mouseBack1(const wxMouseEvent& event)
{
    return event.GetButton() == wxMOUSE_BTN_LEFT &&
            event.GetModifiers() == wxMOD_SHIFT;
}
inline bool mouseBack2(const wxMouseEvent& event)
{
    return event.GetButton() == wxMOUSE_BTN_RIGHT &&
            event.GetModifiers() == wxMOD_NONE;
}

inline bool mouseSetCustomFromFore(const wxMouseEvent& event)
{
    return event.GetButton() == wxMOUSE_BTN_LEFT &&
            event.GetModifiers() == wxMOD_CONTROL;
}
inline bool mouseSetCustomFromBack(const wxMouseEvent& event)
{
    return event.GetButton() == wxMOUSE_BTN_RIGHT &&
        event.GetModifiers() == wxMOD_CONTROL;
}
inline bool mouseSetCustomFromMix(const wxMouseEvent& event)
{
    return event.GetButton() == wxMOUSE_BTN_LEFT &&
            event.GetModifiers() == wxMOD_SHIFT;
}
inline bool mouseSetCustomClear(const wxMouseEvent& event)
{
    return event.GetButton() == wxMOUSE_BTN_LEFT &&
            event.GetModifiers() == (wxMOD_SHIFT | wxMOD_CONTROL);
}

///////////////////////////////////////////////////////////////////////
// ColorPalette Status Object

class CColorPalette;

class CColorCmdUI : public wxCommandEvent
{
public:
    CColorCmdUI(CColorPalette& colorPalette);

    // Used...
    void Enable(BOOL bOn);
    // New for color palette
    void SetColor(wxColour cr = wxNullColour);
    void SetLineWidth(UINT uiLineWidth = 0);
    void SetCustomColors(const std::vector<wxColour>& pCustColors);

    void DoUpdate();

private:
    RefPtr<CColorPalette> colorPalette;
    bool m_bEnableChanged;
};
wxDECLARE_EVENT(cbEVT_UPDATE_UI_COLOR, CColorCmdUI);
typedef void (wxEvtHandler::* CColorCmdUIFunction)(CColorCmdUI&);
#define CColorCmdUIHandler(func) wxEVENT_HANDLER_CAST(CColorCmdUIFunction, func)
#define EVT_UPDATE_UI_COLOR(id, func) \
    wx__DECLARE_EVT1(cbEVT_UPDATE_UI_COLOR, id, CColorCmdUIHandler(func))

///////////////////////////////////////////////////////////////////////

class CMainFrame;

class CColorPalette : public wxPanel
{
public:
    CColorPalette();
    BOOL SetupLineControl();

// Attributes
protected:
    wxSize DoGetBestClientSize() const override
    {
        return m_sizeClient;
    }

// Operations
public:
    void SetIDColor(int nID, wxColour cr);
    void SetLineWidth(UINT nLineWidth);
    void SetCustomColors(const std::vector<wxColour>& pCustColors);
    void CalculateMinClientSize(wxSize& size)
    {
        ComputeLayout();
        size = m_sizeClient;
    }

    // Custom color opaque manipulation methods
    static std::vector<wxColour> CustomColorsAllocate();
    static void CustomColorsSerialize(CArchive& ar, std::vector<wxColour>& pCustColors);
    static void CustomColorsClear(std::vector<wxColour>& pCustColors);

// Implementation
public:

protected:
    // wx owns m_comboLine
    RefPtr<wxChoice> m_comboLine;

    CB::ToolTip m_toolTip;

    wxSize       m_sizeClient;

    // Status area vars...

    wxColour    m_crFore;           // Current colors
    wxColour    m_crBack;
    wxColour    m_crTrans;

    wxRect      m_rctForeColor;
    wxRect      m_rctBackColor;
    wxRect      m_rctTrans;         // Rect for transparent color selector
    wxRect      m_rctNoColor;       // Rect for no color selector

    // Color picker area vars...

    std::vector<wxColour> m_pCustColors;

    wxRect      m_rctStdColors;
    wxRect      m_rctCustColors;
    wxRect      m_rctColorMix;
    wxRect      m_rctColorBar;
    wxRect      m_rctSatValWash;

    BOOL        m_bTrackHue;        // Mouse down in hue picker
    BOOL        m_bTrackSV;         // Mouse down in S/V picker
    BOOL        m_bIgnoreRButtonUp; // avoid two (unrelated) actions on single click

    int         m_nHue;             // Current hue value
    int         m_nVal;             // Current value
    int         m_nSat;             // Current saturation
    wxColour    m_crCurMix;         // RGB version of above

    OwnerOrNullPtr<wxBitmap> m_bmapBar;          // Color bar
    OwnerOrNullPtr<wxBitmap> m_bmapWash;         // Color wash bitmap

// Implementation - methods
protected:
    void SetupToolTips(int nMaxWidth);
    void SetupToolTip(const wxRect& rct, wxString tip, CB::ToolTip::Flags flags = CB::ToolTip::NONE);
    void SetupToolTip(wxWindow& pWnd, wxString tip, CB::ToolTip::Flags flags = CB::ToolTip::NONE);

    void DoPaint(wxDC& pDC);
    void PaintCellGroup(wxDC& pDC, const wxColour* pArray, int xLoc, int yLoc);
    void PaintCell(wxDC& pDC, const wxRect& rct, wxColour cref);
    void PaintSelections(wxDC& pDC);

    void UpdateCurrentColors(BOOL bImmediate);
    void UpdateCurrentColorMix(BOOL bUpdate = TRUE);
    BOOL HandleButtonDown(const wxMouseEvent& event);

    void ComputeLayout();
    void GenerateSVWash(BOOL bInvalidate = TRUE);

    void MapHSVtoPixelLoc(int& pnHLoc, int& pnSLoc, int& pnVLoc) const;
    BOOL MapMouseLocToH(wxPoint pntClient, int& nH, BOOL bCheckValidPoint = TRUE) const;
    BOOL MapMouseLocToSV(wxPoint pntClient, int& nS, int& nV, BOOL bCheckValidPoint = TRUE) const;
    wxColour* MapMouseToColorCell(wxColour* pArray, wxPoint pntClient, const wxRect& rctArray);

    static void NotifyColorChange(const wxMouseEvent& event, wxColour cref);
    static void NotifyCustomColorChange(const std::vector<wxColour>& pcrCustomColors);

// Overrides
protected:

// Generated message map functions
protected:

    void OnPaint(wxPaintEvent& event);
    void OnCreate(wxWindowCreateEvent& event);
    void OnLButtonDown(wxMouseEvent& event);
    void OnRButtonDown(wxMouseEvent& event);
    void OnUpdateCmdUI(wxUpdateUIEvent& pCmdUI);
    void OnLineWidthCbnSelchange(wxCommandEvent& event);
#if 0
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
#endif
    void OnMouseMove(wxMouseEvent& event);
    void OnLButtonUp(wxMouseEvent& event);
    void OnRButtonUp(wxMouseEvent& event);
    void OnLButtonDblClk(wxMouseEvent& event);
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
    void OnPaletteHide(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif

