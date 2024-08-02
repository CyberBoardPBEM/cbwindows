// SelObjs.h -- contains class definitions for selection proxies. Used
//  in concert with the selection tool.
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

#ifndef _SELOBJS_H
#define _SELOBJS_H

#include    <functional>

#ifndef     _DRAWOBJ_H
#include    "DrawObj.h"
#endif

///////////////////////////////////////////////////////////////////////

class CBrdEditView;

////////////////////////////////////////////////////////////////////////
// Various tracker hit constants. Each set must be zero based.

const int hitNothing     = -1;

const int hitTopLeft     = 0;       // For CSelRect & CSelGeneric
const int hitTopRight    = 1;
const int hitBottomRight = 2;
const int hitBottomLeft  = 3;
const int hitTop         = 4;
const int hitRight       = 5;
const int hitBottom      = 6;
const int hitLeft        = 7;

const int hitPtA         = 0;       // For CSelLine
const int hitPtB         = 1;

////////////////////////////////////////////////////////////////////////

enum TrackMode { trkCurrent = -1, trkSelected = 0, trkMoving, trkSizing };

class CSelection
{
// Constructors
public:
    CSelection(CBrdEditView& pView, CDrawObj& pObj)
        : m_pView (&pView), m_pObj (&pObj), m_rect (pObj.GetRect())     //DFM19991221
    {}                                                                 //DFM19991221
    virtual ~CSelection() {}

// Attributes
public:
    RefPtr<CDrawObj> m_pObj;           // Associated object that is selected
    CRect     m_rect;           // Enclosing rect for selected object

    virtual HCURSOR GetHandleCursor(int nHandle) const /* override */
        { return AfxGetApp()->LoadStandardCursor(IDC_ARROW); }
    virtual CRect GetRect() const /* override */ { return m_rect; }

// Operations
public:
    virtual int HitTestHandles(CPoint point) const /* override */;
    // ------- //
    virtual void MoveHandle(int m_nHandle, CPoint point) /* override */ {}
    virtual void Offset(CPoint ptDelta) /* override */ { m_rect += ptDelta; }
    // ------- //
    virtual void DrawTracker(wxDC& pDC, TrackMode eMode) const /* override */;
    virtual void InvalidateHandles() /* override */;
    virtual void Invalidate() /* override */;
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE) /* override */ {}
    virtual void Open()  /* override */ {}
    virtual void Close() /* override */ {}

// Miscellaneous Implementer's Overrides
protected:
    virtual wxRect GetHandleRect(int nHandleID) const /* override */;
    virtual wxPoint GetHandleLoc(int nHandleID) const /* override */ = 0;
    virtual int GetHandleCount() const /* override */ = 0;
    virtual void DrawHandles(wxDC& pDC) const /* override */;
    virtual void DrawTrackingImage(CDC& pDC, TrackMode eMode) const /* override */ = 0;

// Implementation
protected:
    RefPtr<CBrdEditView> m_pView;            // Selection's view
    // -- Class level support methods -- //
    static void SetupTrackingDraw(CDC& pDC);
    static void CleanUpTrackingDraw(CDC& pDC);
    // -- Class variables -- //
    static CPen     NEAR c_penDot;
    static int      NEAR c_nPrvROP2;
    static CPen*    NEAR c_pPrvPen;
    static CBrush*  NEAR c_pPrvBrush;
};

/////////////////////////////////////////////////////////////////////
// CSelRect -- CRectObj Selection Proxy

class CSelRect : public CSelection
{
// Constructors
public:
    CSelRect(CBrdEditView& pView, CRectObj& pObj) : CSelection(pView, pObj)
        { m_rect = pObj.GetRect(); }

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID) const override;
    virtual void MoveHandle(int m_nHandle, CPoint point) override;

protected:
    virtual void DrawTrackingImage(CDC& pDC, TrackMode eMode) const override;
    virtual wxPoint GetHandleLoc(int nHandleID) const override;
    virtual int GetHandleCount() const override { return 8; }
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE) override;
};

/////////////////////////////////////////////////////////////////////
// CSelEllipse -- CEllipse Selection Proxy

class CSelEllipse : public CSelRect
{
// Constructors
public:
    CSelEllipse(CBrdEditView& pView, CRectObj& pObj) : CSelRect(pView, pObj) {}

// Overrides
protected:
    virtual void DrawTrackingImage(CDC& pDC, TrackMode eMode) const override;
};

/////////////////////////////////////////////////////////////////////
// CSelLine -- CLine Selection Proxy

class CSelLine : public CSelection
{
// Constructors
public:
    CSelLine(CBrdEditView& pView, CLine& pObj) : CSelection(pView, pObj)
        { pObj.GetLine(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom); }

// Attributes
public:
    virtual CRect GetRect() const override;

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID) const override;
    virtual void MoveHandle(int m_nHandle, CPoint point) override;

protected:
    virtual void DrawTrackingImage(CDC& pDC, TrackMode eMode) const override;
    virtual wxPoint GetHandleLoc(int nHandleID) const override;
    virtual int GetHandleCount() const override { return 2; }
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE) override;
};

/////////////////////////////////////////////////////////////////////
// CSelPoly -- Polygon Selection Proxy

class CSelPoly : public CSelection
{
// Constructors
public:
    CSelPoly(CBrdEditView& pView, CPolyObj& pObj);
    virtual ~CSelPoly() = default;

// Attributes
public:
    virtual CRect GetRect() const override;
    // -------- //
    std::vector<POINT> m_Pnts;

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID) const override;
    virtual void MoveHandle(int m_nHandle, CPoint point) override;
    virtual void Offset(CPoint ptDelta) override;

protected:
    virtual void DrawTrackingImage(CDC& pDC, TrackMode eMode) const override;
    virtual wxPoint GetHandleLoc(int nHandleID) const override;
    virtual int GetHandleCount() const override
        { return value_preserving_cast<int>(static_cast<const CPolyObj&>(*m_pObj).m_Pnts.size()); }
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE) override;
};

/////////////////////////////////////////////////////////////////////
// CSelGeneric -- Generic Selection Proxy. Used for rectangular
// CDrawObj's that can only be moved and the m_rctExtent is the
// defining perimeter.

class CSelGeneric : public CSelection
{
// Constructors
public:
    CSelGeneric(CBrdEditView& pView, CDrawObj& pObj) : CSelection(pView, pObj)
        { m_rect = pObj.GetRect(); }

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID) const override
        { return AfxGetApp()->LoadStandardCursor(IDC_ARROW); }
    virtual void MoveHandle(int m_nHandle, CPoint point) override {}

protected:
    virtual void DrawTrackingImage(CDC& pDC, TrackMode eMode) const override;
    virtual wxPoint GetHandleLoc(int nHandleID) const override;
    virtual int GetHandleCount() const override { return 4; }
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE) override;
};

/////////////////////////////////////////////////////////////////////

class CSelList : private std::list<OwnerPtr<CSelection>>
{
    using BASE = std::list<OwnerPtr<CSelection>>;
public:
    using BASE::iterator;
    using BASE::begin;
    using BASE::empty;
    using BASE::end;
    using BASE::front;
    using BASE::pop_front;
    using BASE::size;

// Constructors
public:
    CSelList(CBrdEditView& pView) :
        m_pView(&pView),
        m_eTrkMode(trkSelected)
    {
    }
    CSelList(const CSelList&) = delete;
    CSelList& operator=(const CSelList&) = delete;
    ~CSelList() = default;

// Attributes
public:
    BOOL IsMultipleSelects() const { return size() > size_t(1); }
    BOOL IsSingleSelect() const { return size() == size_t(1); }
    BOOL IsAnySelects() const { return !empty(); }
    BOOL IsObjectSelected(const CDrawObj& pObj) const;
    BOOL IsCopyToClipboardPossible() const;

    BOOL IsDObjFlagSetInAllSelectedObjects(DWORD dwFlag) const;
    BOOL IsDObjFlagSetInSomeSelectedObjects(DWORD dwFlag) const;
    // -------- //
    void CopyToClipboard();
    void Open();
    // -------- //
    void SetTrackingMode(TrackMode eTrkMode) { m_eTrkMode = eTrkMode; }
    TrackMode GetTrackingMode() const { return m_eTrkMode; }
    CRect GetEnclosingRect() const { return m_rctEncl; }

// Operations
public:
    CSelection& AddObject(CDrawObj& pObj, BOOL bInvalidate = FALSE);
    void RemoveObject(const CDrawObj& pObj, BOOL bInvalidate = FALSE);
    // -------- //
    void InvalidateListHandles(BOOL bUpdate = FALSE);
    void InvalidateList(BOOL bUpdate = FALSE);
    void PurgeList(BOOL bInvalidate = TRUE);
    // -------- //
    int HitTestHandles(CPoint point) const;
    // -------- //
    void Offset(CPoint ptDelta);
    void MoveHandle(int m_nHandle, CPoint point);

    void SetDObjFlagInAllSelectedObjects(DWORD dwFlag);
    void ClearDObjFlagInAllSelectedObjects(DWORD dwFlag);

    // -------- //
    void OnDraw(wxDC& pDC);  // Called by view OnDraw()
    void DrawTracker(wxDC& pDC, TrackMode eTrkMode = trkCurrent);
    // -------- //
    void UpdateObjects(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE );
    // -------- //
    void ForAllSelections(std::function<void (CDrawObj& pObj)> pFunc);
    // -------- //

// Implementation
protected:
    RefPtr<CBrdEditView> m_pView;          // Selection's view
    TrackMode     m_eTrkMode;       // Current list tracking mode.
    CRect         m_rctEncl;        // Enclosing rect.
    // -------- //
    void CalcEnclosingRect();
};

#endif

