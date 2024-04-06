// ColorPal.h : header file
//
// Copyright (c) 1994-2024 By Dale L. Larson & William Su, All Rights Reserved.
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

const COLORREF nullColorRef = 0xFF000000; //(should match noColor in tile.h)

const UINT mouseMask = (MK_CONTROL | MK_SHIFT | MK_LBUTTON | MK_RBUTTON);

const UINT mouseFore = MK_LBUTTON;
const UINT mouseBack1 = (MK_LBUTTON | MK_SHIFT);
const UINT mouseBack2 = MK_RBUTTON;

const UINT mouseSetCustomFromFore = (MK_LBUTTON | MK_CONTROL);
const UINT mouseSetCustomFromBack = (MK_RBUTTON | MK_CONTROL);
const UINT mouseSetCustomFromMix = (MK_LBUTTON | MK_SHIFT);
const UINT mouseSetCustomClear = (MK_LBUTTON | MK_SHIFT | MK_CONTROL);

///////////////////////////////////////////////////////////////////////
// ColorPalette Status Object

class CColorCmdUI : public CCmdUI
{
public:
    // Not used...
    virtual void SetCheck(int nCheck) override {}
    virtual void SetText(LPCTSTR lpszText) override {}
    // Used...
    virtual void Enable(BOOL bOn) override;
    // New for color palette
    virtual void SetColor(COLORREF cr = nullColorRef) /* override */;
    virtual void SetLineWidth(UINT uiLineWidth = 0) /* override */;
    virtual void SetCustomColors(const std::vector<COLORREF>& pCustColors) /* override */;
};

///////////////////////////////////////////////////////////////////////

class CMainFrame;
class CColorPalette;

class CDockColorPalette : public CDockablePane
{
    DECLARE_DYNCREATE(CDockColorPalette);
    // Construction
public:
    CDockColorPalette();

// Operations
public:
    void CalculateMinClientSize(CSize& size);

// Overrides
protected:
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz) override;
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler) override;
    virtual CSize CalcSize(BOOL bVertDock) override
    {
        return CalcSize();
    }
    virtual void GetMinSize(CSize& size) const override
    {
        // Account for size of caption.
        size = CalcSize() + CSize(0, GetCaptionHeight());
    }
    virtual void GetPaneName(CString &strName) const override
    {
        AfxThrowNotSupportedException();
    }

private:
    CSize CalcSize() const;

    OwnerPtr<CColorPalette> m_child;

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};

class CColorPalette : public CWnd
{
public:
    CColorPalette();
    BOOL SetupLineControl();

// Attributes
public:
    CSize GetSize() const
    {
        return m_sizeClient;
    }

// Operations
public:
    void SetIDColor(UINT nID, COLORREF cr);
    void SetLineWidth(UINT nLineWidth);
    void SetCustomColors(const std::vector<COLORREF>& pCustColors);
    void CalculateMinClientSize(CSize& size)
    {
        ComputeLayout();
        size = m_sizeClient;
    }

    // Custom color opaque manipulation methods
    static std::vector<COLORREF> CustomColorsAllocate();
    static void CustomColorsSerialize(CArchive& ar, std::vector<COLORREF>& pCustColors);
    static void CustomColorsClear(std::vector<COLORREF>& pCustColors);

// Implementation
public:

protected:
    CComboBox   m_comboLine;

    CToolTipCtrl m_toolTip;

    CSize       m_sizeClient;

    // Status area vars...

    COLORREF    m_crFore;           // Current colors
    COLORREF    m_crBack;
    COLORREF    m_crTrans;

    CRect       m_rctForeColor;
    CRect       m_rctBackColor;
    CRect       m_rctTrans;         // Rect for transparent color selector
    CRect       m_rctNoColor;       // Rect for no color selector

    // Color picker area vars...

    std::vector<COLORREF> m_pCustColors;

    CRect       m_rctStdColors;
    CRect       m_rctCustColors;
    CRect       m_rctColorMix;
    CRect       m_rctColorBar;
    CRect       m_rctSatValWash;

    BOOL        m_bTrackHue;        // Mouse down in hue picker
    BOOL        m_bTrackSV;         // Mouse down in S/V picker
    BOOL        m_bIgnoreRButtonUp; // avoid two (unrelated) actions on single click

    int         m_nHue;             // Current hue value
    int         m_nVal;             // Current value
    int         m_nSat;             // Current saturation
    COLORREF    m_crCurMix;         // RGB version of above

    OwnerOrNullPtr<CBitmap> m_bmapBar;          // Color bar
    OwnerOrNullPtr<CBitmap> m_bmapWash;         // Color wash bitmap

// Implementation - methods
protected:
    void SetupToolTips(int nMaxWidth);
    void SetupToolTip(const RECT& rct, UINT nID, UINT nFlags = 0);
    void SetupToolTip(const CWnd& pWnd, UINT nID, UINT nFlags = 0);

    void DoPaint(CDC& pDC);
    static void PaintCellGroup(CDC& pDC, const COLORREF* pArray, int xLoc, int yLoc);
    static void PaintCell(CDC& pDC, const CRect& rct, COLORREF cref);
    void PaintSelections(CDC& pDC);

    void UpdateCurrentColors(BOOL bImmediate);
    void UpdateCurrentColorMix(BOOL bUpdate = TRUE);
    BOOL HandleButtonDown(UINT nFlags, CPoint point);

    void ComputeLayout();
    void GenerateSVWash(BOOL bInvalidate = TRUE);

    void MapHSVtoPixelLoc(int& pnHLoc, int& pnSLoc, int& pnVLoc) const;
    BOOL MapMouseLocToH(CPoint pntClient, int& nH, BOOL bCheckValidPoint = TRUE) const;
    BOOL MapMouseLocToSV(CPoint pntClient, int& nS, int& nV, BOOL bCheckValidPoint = TRUE) const;
    static COLORREF* MapMouseToColorCell(COLORREF* pArray, CPoint pntClient, const CRect& rctArray);

    static void NotifyColorChange(UINT nFlags, COLORREF cref);
    static void NotifyCustomColorChange(const std::vector<COLORREF>& pcrCustomColors);

// Overrides
protected:

// Generated message map functions
protected:

    afx_msg void OnPaint();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLineWidthCbnSelchange();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg LRESULT OnPaletteHide(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()
};

#endif

