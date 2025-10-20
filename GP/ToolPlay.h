// ToolPlay.h
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

#ifndef _TOOLPLAY_H
#define _TOOLPLAY_H

#ifndef     _SELOPLAY_H
#include    "SelOPlay.h"
#endif

class CPlayBoardView;

const UINT timerIDSelectDelay = 666;    // Select timer ID
const UINT timerIDAutoScroll  = 999;    // Autoscroll timer ID
const int  timerSelDelay      = 250;    // Delay until select
const int  timerAutoScroll    = 80;     // Interval of autoscrolls

enum PToolType { ptypeUnknown, ptypeSelect, ptypeLine, ptypeTextBox,
    ptypeMPlot };

class CPlayTool
{
// Constructors
public:
    // Various derived tool types.

    CPlayTool(PToolType eType);

// Attributes
public:
    const PToolType m_eToolType;

// Operations
public:
    static CPlayTool& GetTool(PToolType eType);
    // ----------- //
    virtual void OnLButtonDown(CPlayBoardView& pView, UINT nFlags, CPoint point) /* override */;
    virtual void OnLButtonDblClk(CPlayBoardView& /*pView*/, UINT /*nFlags*/, CPoint /*point*/) /* override */ {}
    virtual bool OnLButtonUp(CPlayBoardView& pView, UINT nFlags, CPoint point) /* override */;
    virtual void OnMouseMove(CPlayBoardView& pView, UINT nFlags, CPoint point) /* override */;
    virtual void OnTimer(CPlayBoardView& /*pView*/, uintptr_t /*nIDEvent*/) /*override*/ {}
    virtual BOOL OnSetCursor(const CPlayBoardView& /*pView*/, UINT /*nHitTest*/) const /*override*/
        { return FALSE; }

// Implementation
private:
    // -- Class variables -- //
    static std::vector<CPlayTool*> c_toolLib;
protected:
    // Drag related vars....
    static CPoint c_ptDown;         // Document coords.
    static CPoint c_ptLast;
};

////////////////////////////////////////////////////////////////////////
// CPSelectTool - Object Selection/Manipulation tool

class CPSelectTool : public CPlayTool
{
// Constructors
public:
    CPSelectTool() : CPlayTool(ptypeSelect) { m_nTimerID = uintptr_t(0); }

// Attributes
public:
    enum SelectMode { smodeNormal, smodeSizing, smodeNet, smodeMove,
        smodeGrpMove };
    // ------ //
    SelectMode  m_eSelMode;
    int         m_nHandleID;
    // Inter-view Drag/Drop processing support
    DragInfo    m_di;
    HWND        m_hLastWnd;

// Operations
public:
    void OnLButtonDown(CPlayBoardView& pView, UINT nFlags, CPoint point) override;
    void OnLButtonDblClk(CPlayBoardView& pView, UINT nFlags, CPoint point) override;
    bool OnLButtonUp(CPlayBoardView& pView, UINT nFlags, CPoint point) override;
    void OnMouseMove(CPlayBoardView& pView, UINT nFlags, CPoint point) override;
    void OnTimer(CPlayBoardView& pView, uintptr_t nIDEvent) override;
    BOOL OnSetCursor(const CPlayBoardView& pView, UINT nHitTest) const override;

// Implementation
public:
    uintptr_t m_nTimerID;
    CRect   m_rectMultiBorder;
    // ------- //
    BOOL ProcessAutoScroll(CPlayBoardView& pView);
    void DrawSelectionRect(wxDC& pDC, const wxRect& pRct) const;
    void DrawNetRect(wxDC& pDC, const CPlayBoardView& pView) const;
    [[nodiscard]] CPoint AdjustPoint(const CPlayBoardView& pView, CPoint point) const;
    void MoveSelections(CSelList &pSLst, const CPoint& point);
    void StartDragTimer(CPlayBoardView& pView);
    void KillDragTimer(CPlayBoardView& pView);
    void StartScrollTimer(CPlayBoardView& pView);
    void KillScrollTimer(CPlayBoardView& pView);
    // ------- //
    void DoDragDropStart(CPlayBoardView& pView);
    void DoDragDrop(CPlayBoardView& pView, const CPoint& pntClient);
    bool DoDragDropEnd(CPlayBoardView& pView, const CPoint& pntClient);
    // ------- //
    void StartSizingOperation(CPlayBoardView& pView, UINT nFlags,
        const CPoint& point, int nHandleID = -1);
};

////////////////////////////////////////////////////////////////////////
// CShapeTool - tool used a basis of rectangular related shapes

class CPShapeTool : public CPlayTool
{
// Constructors
public:
    CPShapeTool(PToolType eType) : CPlayTool(eType) {}

// Operations
public:
    void OnLButtonDown(CPlayBoardView& pView, UINT nFlags, CPoint point) override;
    void OnLButtonDblClk(CPlayBoardView& /*pView*/, UINT /*nFlags*/, CPoint /*point*/) override {}
    bool OnLButtonUp(CPlayBoardView& pView, UINT nFlags, CPoint point) override;
    void OnMouseMove(CPlayBoardView& pView, UINT nFlags, CPoint point) override;
    void OnTimer(CPlayBoardView& pView, uintptr_t nIDEvent) override;
    BOOL OnSetCursor(const CPlayBoardView& pView, UINT nHitTest) const override;

// Implementation
public:
    virtual OwnerPtr<CDrawObj> CreateDrawObj(CPlayBoardView& pView, const CPoint& point,
        int& nHandle) const = 0;
    virtual BOOL IsEmptyObject() const = 0;
    // --------- //
    OwnerOrNullPtr<CDrawObj> m_pObj;
};

////////////////////////////////////////////////////////////////////////
// CPLineTool - tool used to create lines

class CPLineTool : public CPShapeTool
{
// Constructors
public:
    CPLineTool() : CPShapeTool(ptypeLine) { }

// Implementation
public:
    OwnerPtr<CDrawObj> CreateDrawObj(CPlayBoardView& pView, const CPoint& point,
        int& nHandle) const override;
    BOOL IsEmptyObject() const override;
};

////////////////////////////////////////////////////////////////////////
// CTextBoxTool - Text box drawing object tool

class CPTextBoxTool : public CPlayTool
{
// Constructors
public:
    CPTextBoxTool() : CPlayTool(ptypeTextBox) { }

// Operations
public:
    void OnLButtonDown(CPlayBoardView& pView, UINT nFlags, CPoint point) override;
    void OnLButtonDblClk(CPlayBoardView& /*pView*/, UINT /*nFlags*/, CPoint /*point*/) override {}
    bool OnLButtonUp(CPlayBoardView& /*pView*/, UINT /*nFlags*/, CPoint /*point*/) override { return true; }
    void OnMouseMove(CPlayBoardView& /*pView*/, UINT /*nFlags*/, CPoint /*point*/) override {}
    void OnTimer(CPlayBoardView& /*pView*/, uintptr_t /*nIDEvent*/) override {}
    BOOL OnSetCursor(const CPlayBoardView& pView, UINT nHitTest) const override;

// Implementation
public:
};

/////////////////////////////////////////////////////////////////////////////

// CPPlotTool - move plot tool

class CPPlotTool : public CPlayTool
{
// Constructors
public:
    CPPlotTool() : CPlayTool(ptypeMPlot) { }

// Operations
public:
    void OnLButtonDown(CPlayBoardView& pView, UINT nFlags, CPoint point) override;
    void OnLButtonDblClk(CPlayBoardView& /*pView*/, UINT /*nFlags*/, CPoint /*point*/) override {}
    bool OnLButtonUp(CPlayBoardView& /*pView*/, UINT /*nFlags*/, CPoint /*point*/) override { return true; }
    void OnMouseMove(CPlayBoardView& /*pView*/, UINT /*nFlags*/, CPoint /*point*/) override {}
    void OnTimer(CPlayBoardView& /*pView*/, uintptr_t /*nIDEvent*/) override {}
    BOOL OnSetCursor(const CPlayBoardView& pView, UINT nHitTest) const override;

// Implementation
public:
};

#endif


