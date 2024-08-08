// ToolObjs.h
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

#ifndef _TOOLOBJS_H
#define _TOOLOBJS_H

#ifndef     _SELOBJS_H
#include    "SelObjs.h"
#endif

class CBrdEditView;

const UINT timerIDSelectDelay = 666;    // Select timer ID
const UINT timerIDAutoScroll  = 999;    // Autoscroll timer ID
const int  timerSelDelay      = 150;    // Delay until select
const int  timerAutoScroll    = 150;    // Interval of autoscrolls

enum ToolType { ttypeUnknown, ttypeSelect, ttypeText, ttypeRect,
    ttypeEllipse, ttypeLine, ttypeTile, ttypeColorPick, ttypeCellPaint,
    ttypeCellEraser, ttypePaintBack, ttypePolygon };

class CTool
{
// Constructors
public:
    // Various derived tool types.

    CTool(ToolType eType);

// Attributes
public:
    const ToolType m_eToolType;

// Operations
public:
    static CTool& GetTool(ToolType eType);
    // ----------- //
    virtual void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) /* override */;
    virtual void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) /* override */ {}
    virtual void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) /* override */;
    virtual void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) /* override */;
    virtual void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) /* override */ {}
    virtual wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const /* override */
        { return wxNullCursor; }

// Implementation
private:
    // -- Class variables -- //
    static std::vector<CTool*> c_toolLib;
protected:
    // Drag related vars....
    static wxPoint c_ptDown;         // Document coords.
    static wxPoint c_ptLast;
};

////////////////////////////////////////////////////////////////////////
// CSelect - Object Selection/Manipulation tool

class CSelectTool : public CTool
{
// Constructors
public:
    CSelectTool() : CTool(ttypeSelect) { m_nTimerID = uintptr_t(0); }

// Attributes
public:
    enum SelectMode { smodeNormal, smodeSizing, smodeNet, smodeMove,
        smodeGrpMove };
    // ------ //
    SelectMode  m_eSelMode;
    int         m_nHandleID;

// Operations
public:
    void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) override;
    void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) override;
    wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const override;

// Implementation
public:
    uintptr_t m_nTimerID;
    wxRect  m_rectMultiBorder;
    // ------- //
    BOOL ProcessAutoScroll(CBrdEditView& pView);
    void DrawSelectionRect(wxDC& pDC, const wxRect& pRct) const;
    void DrawNetRect(wxDC& pDC, CBrdEditView& pView) const;
    BOOL AdjustPoint(const CBrdEditView& pView, wxPoint& point) const;
    void MoveSelections(CSelList& pSLst, wxPoint point);
    void StartDragTimer(CBrdEditView& pView);
    void KillDragTimer(CBrdEditView& pView);
    void StartScrollTimer(CBrdEditView& pView);
    void KillScrollTimer(CBrdEditView& pView);
    // ------- //
    void StartSizingOperation(CBrdEditView& pView, int nMods,
        wxPoint point, int nHandleID = -1);
};

////////////////////////////////////////////////////////////////////////
// CShapeTool - tool used a basis of rectangular related shapes

class CShapeTool : public CTool
{
// Constructors
public:
    CShapeTool(ToolType eType) : CTool(eType) {}

// Operations
public:
    void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) override;
    void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) override;
    wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const override;

// Implementation
public:
    virtual CDrawObj::OwnerPtr CreateDrawObj(const CBrdEditView& pView, wxPoint point,
        int& nHandle) const /* override */ = 0;
    virtual BOOL IsEmptyObject() const /* override */ = 0;
    // --------- //
    /* NOTE:  CShapeTool owns m_pObj, but CPolyTool passes
        ownership of m_pObj to CBrdEditView immediately */
    OwnerOrNullPtr<CDrawObj> m_pObj;
};

////////////////////////////////////////////////////////////////////////
// CRectTool - tool used to create rectangles.

class CRectTool : public CShapeTool
{
// Constructors
public:
    CRectTool() : CShapeTool(ttypeRect) { }

// Implementation
public:
    CDrawObj::OwnerPtr CreateDrawObj(const CBrdEditView& pView, wxPoint point,
        int& nHandle) const override;
    BOOL IsEmptyObject() const override;
};

////////////////////////////////////////////////////////////////////////
// CEllipseTool - tool used to create rectangles.

class CEllipseTool : public CShapeTool
{
// Constructors
public:
    CEllipseTool() : CShapeTool(ttypeEllipse) { }

// Implementation
public:
    CDrawObj::OwnerPtr CreateDrawObj(const CBrdEditView& pView, wxPoint point,
        int& nHandle) const override;
    BOOL IsEmptyObject() const override;
};

////////////////////////////////////////////////////////////////////////
// CLineTool - tool used to create lines

class CLineTool : public CShapeTool
{
// Constructors
public:
    CLineTool() : CShapeTool(ttypeLine) { }

// Implementation
public:
    CDrawObj::OwnerPtr CreateDrawObj(const CBrdEditView& pView, wxPoint point,
        int& nHandle) const override;
    BOOL IsEmptyObject() const override;
};

////////////////////////////////////////////////////////////////////////
// CPolyTool - tool used to create polygons and polylines

class CPolyTool : public CTool
{
// Constructors
public:
    CPolyTool() : CTool(ttypePolygon) {}

// Operations
public:
    void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) override;
    void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) override {}
    wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const override;

    // --------- //
    void RemoveRubberBand(CBrdEditView& pView);
    void FinalizePolygon(CBrdEditView& pView, BOOL bForceDestroy = FALSE);

// Implementation
public:
    void DrawRubberLine(wxDC& pDC);
    // --------- //
    /* NOTE:  CShapeTool owns m_pObj, but CPolyTool passes
        ownership of m_pObj to CBrdEditView immediately */
    CPolyObj*   m_pObj;
};

////////////////////////////////////////////////////////////////////////
// CTextTool - Text drawing object tool

class CTextTool : public CTool
{
// Constructors
public:
    CTextTool() : CTool(ttypeText) { }

// Operations
public:
    void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) override {}
    void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) override {}
    wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CColorPickupTool - tool used to extract colors from view.

class CColorPickupTool : public CTool
{
// Constructors
public:
    CColorPickupTool() : CTool(ttypeColorPick) { }

// Operations
public:
    void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) override {}
    void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) override {}
    wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CTileTool - tile tool.

class CTileTool : public CTool
{
// Constructors
public:
    CTileTool() : CTool(ttypeTile) { }

// Operations
public:
    void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) override;
    void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) override {}
    wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CCellPaintTool - Painting tool for cell grid.

class CCellPaintTool : public CTool
{
// Constructors
public:
    CCellPaintTool() : CTool(ttypeCellPaint) { }

// Operations
public:
    void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) override;
    void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) override {}
    wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CPaintTool - Painting tool for cell grid.

class CPaintTool : public CTool
{
// Constructors
public:
    CPaintTool() : CTool(ttypePaintBack) { }

// Operations
public:
    void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) override {}
    void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) override {}
    wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// CCellEraserTool - Erasing tool for cell grid.

class CCellEraserTool : public CTool
{
// Constructors
public:
    CCellEraserTool() : CTool(ttypeCellEraser) { }

// Operations
public:
    void OnLButtonDown(CBrdEditView& pView, int nMods, wxPoint point) override;
    void OnLButtonDblClk(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnLButtonUp(CBrdEditView& pView, int nMods, wxPoint point) override {}
    void OnMouseMove(CBrdEditView& pView, int nMods, int nButs, wxPoint point) override;
    void OnTimer(CBrdEditView& pView, uintptr_t nIDEvent) override {}
    wxCursor OnSetCursor(const CBrdEditView& pView, wxPoint point) const override;

// Implementation
public:
};

////////////////////////////////////////////////////////////////////////
// ::::RAMBLINGS AND ISSUES::::
// How do we identify drag bounds from view/document.
////////////////////////////////////////////////////////////////////////
// Methods provided by the CView...
//  CSelList* CXxxView::GetSelectList();
//  CXxxObject* CXxxView::ObjectHitTest(CPoint point);
//  void CXxxView::WorkspaceToClient(CRect& rct);
//  void CXxxView::WorkspaceToClient(CPoint& point);
//  void CXxxView::ClientToWorkspace(CRect& rct);
//  void CXxxView::ClientToWorkspace(CPoint& point);
//  void CXxxView::SelectWithinRect(CRect* pRect);
//  void CXxxView::InvalidateWorkspaceRect(const CRect* pRect, BOOL bErase);
//  ----
// Methods provided by the selection list.
//  int CSelList::GetCount();
//  CSelection* CSelList::AddObject(CXxxObject* pXxx, BOOL bInvalidate = FALSE);
//  CSelection* CSelList::GetHead()
//  void CSelList::RemoveObject(CXxxObject* pXxx, BOOL bInvalidate = FALSE);
//  void CSelList::InvalidateList(BOOL bUpdate = FALSE);
//  void CSelList::PurgeList(BOOL bInvalidate = FALSE);// (Handles empty case.)
//  void CSelList::DrawTracker(CDC *pDC, TrackMode eMode);
//  void CSelList::UpdateDocObjects();
//  ----
// Methods provided by the selection object.
//  void CSelection::DrawTracker(CDC *pDC, TrackMode eMode);
//  int CSelection::HitTestHandles(CPoint pnt); // -1 means no handle hit
//  HCURSOR CSelection::GetHandlesCursor(UINT nHandle);
//  void CSelection::UpdateDocObjects();
//  void CSelection::MoveHandle(UINT m_nHandle, CPoint point);
//  void CSelection::OnOpen();
//  ----
// Support required from displayed document items:
//  CSelection* CXxxObject::CreateSelectProxy();
//

#endif


