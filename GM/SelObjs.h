// SelObjs.h -- contains class definitions for selection proxies. Used
//  in concert with the selection tool.
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

#ifndef _SELOBJS_H
#define _SELOBJS_H

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
    CSelection(CBrdEditView* pView, CDrawObj* pObj)
        : m_pView (pView), m_pObj (pObj), m_rect (pObj->GetRect())     //DFM19991221
    {}                                                                 //DFM19991221
    virtual ~CSelection() {}

// Attributes
public:
    CDrawObj* m_pObj;           // Associated object that is selected
    CRect     m_rect;           // Enclosing rect for selected object

    virtual HCURSOR GetHandleCursor(int nHandle)
        { return AfxGetApp()->LoadStandardCursor(IDC_ARROW); }
    virtual CRect GetRect() { return m_rect; }

// Operations
public:
    virtual int  HitTestHandles(CPoint point);
    // ------- //
    virtual void MoveHandle(int m_nHandle, CPoint point) {}
    virtual void Offset(CPoint ptDelta) { m_rect += ptDelta; }
    // ------- //
    virtual void DrawTracker(CDC *pDC, TrackMode eMode);
    virtual void InvalidateHandles();
    virtual void Invalidate();
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE) {}
    virtual void Open()  {}
    virtual void Close() {}

// Miscellaneous Implementer's Overrides
protected:
    virtual CRect GetHandleRect(int nHandleID);
    virtual CPoint GetHandleLoc(int nHandleID) = 0;
    virtual int GetHandleCount() = 0;
    virtual void DrawHandles(CDC* pDC);
    virtual void DrawTrackingImage(CDC* pDC, TrackMode eMode) = 0;

// Implementation
protected:
    CBrdEditView*   m_pView;            // Selection's view
    // -- Class level support methods -- //
    static void SetupTrackingDraw(CDC* pDC);
    static void CleanUpTrackingDraw(CDC* pDC);
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
    CSelRect(CBrdEditView* pView, CRectObj* pObj) : CSelection(pView, pObj)
        { m_rect = pObj->GetRect(); }

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID);
    virtual void MoveHandle(int m_nHandle, CPoint point);

protected:
    virtual void DrawTrackingImage(CDC* pDC, TrackMode eMode);
    virtual CPoint GetHandleLoc(int nHandleID);
    virtual int GetHandleCount() { return 8; }
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE);
};

/////////////////////////////////////////////////////////////////////
// CSelEllipse -- CEllipse Selection Proxy

class CSelEllipse : public CSelRect
{
// Constructors
public:
    CSelEllipse(CBrdEditView* pView, CRectObj* pObj) : CSelRect(pView, pObj) {}

// Overrides
protected:
    virtual void DrawTrackingImage(CDC* pDC, TrackMode eMode);
};

/////////////////////////////////////////////////////////////////////
// CSelLine -- CLine Selection Proxy

class CSelLine : public CSelection
{
// Constructors
public:
    CSelLine(CBrdEditView* pView, CLine* pObj) : CSelection(pView, pObj)
        { pObj->GetLine(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom); }

// Attributes
public:
    virtual CRect GetRect();

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID);
    virtual void MoveHandle(int m_nHandle, CPoint point);

protected:
    virtual void DrawTrackingImage(CDC* pDC, TrackMode eMode);
    virtual CPoint GetHandleLoc(int nHandleID);
    virtual int GetHandleCount() { return 2; }
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE);
};

/////////////////////////////////////////////////////////////////////
// CSelPoly -- Polygon Selection Proxy

class CSelPoly : public CSelection
{
// Constructors
public:
    CSelPoly(CBrdEditView* pView, CPolyObj* pObj);
    virtual ~CSelPoly() { if (m_pPnts) delete m_pPnts; }

// Attributes
public:
    virtual CRect GetRect();
    // -------- //
    POINT*   m_pPnts;
    int      m_nPnts;

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID);
    virtual void MoveHandle(int m_nHandle, CPoint point);
    virtual void Offset(CPoint ptDelta);

protected:
    virtual void DrawTrackingImage(CDC* pDC, TrackMode eMode);
    virtual CPoint GetHandleLoc(int nHandleID);
    virtual int GetHandleCount()
        { return ((CPolyObj*)m_pObj)->m_nPnts; }
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE);
};

/////////////////////////////////////////////////////////////////////
// CSelGeneric -- Generic Selection Proxy. Used for rectangular
// CDrawObj's that can only be moved and the m_rctExtent is the
// defining perimeter.

class CSelGeneric : public CSelection
{
// Constructors
public:
    CSelGeneric(CBrdEditView* pView, CDrawObj* pObj) : CSelection(pView, pObj)
        { m_rect = pObj->GetRect(); }

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID)
        { return AfxGetApp()->LoadStandardCursor(IDC_ARROW); }
    virtual void MoveHandle(int m_nHandle, CPoint point) {}

protected:
    virtual void DrawTrackingImage(CDC* pDC, TrackMode eMode);
    virtual CPoint GetHandleLoc(int nHandleID);
    virtual int GetHandleCount() { return 4; }
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE);
};

/////////////////////////////////////////////////////////////////////

class CSelList : public CPtrList
{
// Constructors
public:
    CSelList(CBrdEditView* pView) { m_pView = pView; }
    CSelList() { m_pView = NULL; m_eTrkMode = trkSelected; }
    ~CSelList() { PurgeList(FALSE); }

// Attributes
public:
    BOOL IsMultipleSelects() const { return GetCount() > 1; }
    BOOL IsSingleSelect() const { return GetCount() == 1; }
    BOOL IsAnySelects() const { return GetCount() > 0; }
    BOOL IsObjectSelected(CDrawObj* pObj) const;
    BOOL IsCopyToClipboardPossible() const;

    BOOL IsDObjFlagSetInAllSelectedObjects(DWORD dwFlag) const;
    BOOL IsDObjFlagSetInSomeSelectedObjects(DWORD dwFlag) const;
    // -------- //
    void CopyToClipboard();
    void Open();
    // -------- //
    void SetView(CBrdEditView* pView) { m_pView = pView; }
    void SetTrackingMode(TrackMode eTrkMode) { m_eTrkMode = eTrkMode; }
    TrackMode GetTrackingMode() const { return m_eTrkMode; }
    CRect GetEnclosingRect() const { return m_rctEncl; }

// Operations
public:
    CSelection* GetHead() { return (CSelection*)CPtrList::GetHead(); }
    CSelection* AddObject(CDrawObj* pObj, BOOL bInvalidate = FALSE);
    void RemoveObject(CDrawObj* pObj, BOOL bInvalidate = FALSE);
    // -------- //
    void InvalidateListHandles(BOOL bUpdate = FALSE);
    void InvalidateList(BOOL bUpdate = FALSE);
    void PurgeList(BOOL bInvalidate = TRUE);
    // -------- //
    int  HitTestHandles(CPoint point);
    // -------- //
    void Offset(CPoint ptDelta);
    void MoveHandle(UINT m_nHandle, CPoint point);

    void SetDObjFlagInAllSelectedObjects(DWORD dwFlag);
    void ClearDObjFlagInAllSelectedObjects(DWORD dwFlag);

    // -------- //
    void OnDraw(CDC *pDC);  // Called by view OnDraw()
    void DrawTracker(CDC *pDC, TrackMode eTrkMode = trkCurrent);
    // -------- //
    void UpdateObjects(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE );
    // -------- //
    void ForAllSelections(void (*pFunc)(CDrawObj* pObj, DWORD dwUser),
        DWORD dwUserVal);
    // -------- //

// Implementation
protected:
    CBrdEditView* m_pView;          // Selection's view
    TrackMode     m_eTrkMode;       // Current list tracking mode.
    CRect         m_rctEncl;        // Enclosing rect.
    // -------- //
    void CalcEnclosingRect();
};

#endif

