// ToolPlay.h
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
    virtual void OnLButtonDown(CPlayBoardView* pView, UINT nFlags, CPoint point);
    virtual void OnLButtonDblClk(CPlayBoardView* pView, UINT nFlags, CPoint point) {}
    virtual bool OnLButtonUp(CPlayBoardView* pView, UINT nFlags, CPoint point) /* override */;
    virtual void OnMouseMove(CPlayBoardView* pView, UINT nFlags, CPoint point);
    virtual void OnTimer(CPlayBoardView* pView, uintptr_t nIDEvent) /*override*/ {}
    virtual BOOL OnSetCursor(CPlayBoardView* pView, UINT nHitTest)
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
    virtual void OnLButtonDown(CPlayBoardView* pView, UINT nFlags, CPoint point);
    virtual void OnLButtonDblClk(CPlayBoardView* pView, UINT nFlags, CPoint point);
    virtual bool OnLButtonUp(CPlayBoardView* pView, UINT nFlags, CPoint point) override;
    virtual void OnMouseMove(CPlayBoardView* pView, UINT nFlags, CPoint point);
    virtual void OnTimer(CPlayBoardView* pView, uintptr_t nIDEvent) override;
    virtual BOOL OnSetCursor(CPlayBoardView* pView, UINT nHitTest);

// Implementation
public:
    uintptr_t m_nTimerID;
    CRect   m_rectMultiBorder;
    // ------- //
    BOOL ProcessAutoScroll(CPlayBoardView* pView);
    void DrawSelectionRect(CDC* pDC, CRect* pRct);
    void DrawNetRect(CDC* pDC, CPlayBoardView* pView);
    BOOL AdjustPoint(CPlayBoardView* pView, CPoint& point);
    void MoveSelections(CSelList *pSLst, CPoint point);
    void StartDragTimer(CPlayBoardView* pView);
    void KillDragTimer(CPlayBoardView* pView);
    void StartScrollTimer(CPlayBoardView* pView);
    void KillScrollTimer(CPlayBoardView* pView);
    // ------- //
    void DoDragDropStart(CPlayBoardView* pView);
    void DoDragDrop(CPlayBoardView* pView, CPoint pntClient);
    bool DoDragDropEnd(CPlayBoardView* pView, CPoint pntClient);
    // ------- //
    void StartSizingOperation(CPlayBoardView* pView, UINT nFlags,
        CPoint point, int nHandleID = -1);
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
    virtual void OnLButtonDown(CPlayBoardView* pView, UINT nFlags, CPoint point);
    virtual void OnLButtonDblClk(CPlayBoardView* pView, UINT nFlags, CPoint point) {}
    virtual bool OnLButtonUp(CPlayBoardView* pView, UINT nFlags, CPoint point) override;
    virtual void OnMouseMove(CPlayBoardView* pView, UINT nFlags, CPoint point);
    virtual void OnTimer(CPlayBoardView* pView, uintptr_t nIDEvent) override;
    virtual BOOL OnSetCursor(CPlayBoardView* pView, UINT nHitTest);

// Implementation
public:
    virtual CDrawObj* CreateDrawObj(CPlayBoardView* pView, CPoint point,
        int& nHandle) = 0;
    virtual BOOL IsEmptyObject() = 0;
    // --------- //
    CDrawObj*   m_pObj;
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
    virtual CDrawObj* CreateDrawObj(CPlayBoardView* pView, CPoint point,
        int& nHandle);
    virtual BOOL IsEmptyObject();
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
    virtual void OnLButtonDown(CPlayBoardView* pView, UINT nFlags, CPoint point);
    virtual void OnLButtonDblClk(CPlayBoardView* pView, UINT nFlags, CPoint point) {}
    virtual bool OnLButtonUp(CPlayBoardView* pView, UINT nFlags, CPoint point) override { return true; }
    virtual void OnMouseMove(CPlayBoardView* pView, UINT nFlags, CPoint point) {}
    virtual void OnTimer(CPlayBoardView* pView, uintptr_t nIDEvent) override {}
    virtual BOOL OnSetCursor(CPlayBoardView* pView, UINT nHitTest);

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
    virtual void OnLButtonDown(CPlayBoardView* pView, UINT nFlags, CPoint point);
    virtual void OnLButtonDblClk(CPlayBoardView* pView, UINT nFlags, CPoint point) {}
    virtual bool OnLButtonUp(CPlayBoardView* pView, UINT nFlags, CPoint point) override { return true; }
    virtual void OnMouseMove(CPlayBoardView* pView, UINT nFlags, CPoint point) {}
    virtual void OnTimer(CPlayBoardView* pView, uintptr_t nIDEvent) override {}
    virtual BOOL OnSetCursor(CPlayBoardView* pView, UINT nHitTest);

// Implementation
public:
};

#endif


