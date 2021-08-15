// SelOPlay.h -- contains class definitions for selection proxies. Used
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

#ifndef _SELOPLAY_H
#define _SELOPLAY_H

#ifndef     _DRAWOBJ_H
#include    "DrawObj.h"
#endif

///////////////////////////////////////////////////////////////////////

class CPlayBoardView;

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
// Orders (by x and y) list of selection handles. Even if there is
// more that one handle at a particular (x,y) there is only one in
// this list. This is done to prevent an even number of XOR's
// essentially erasing a grab handle.

class CHandleList : public CList< POINT, POINT& >
{
// Methods
public:
    void AddHandle(POINT pntNew);
};

////////////////////////////////////////////////////////////////////////

enum TrackMode { trkCurrent = -1, trkSelected = 0, trkMoving, trkSizing };

class CSelection
{
// Constructors
public:
    CSelection(CPlayBoardView& pView, CDrawObj& pObj) :
        m_pView(&pView), m_pObj(&pObj)
    {}
    virtual ~CSelection() = default;

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
    virtual void DrawTracker(CDC& pDC, TrackMode eMode) const /* override */;
    virtual void InvalidateHandles() /* override */;
    virtual void Invalidate() /* override */;
    virtual void UpdateObject(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE) /* override */ {}
    virtual void Open() /* override */;
    virtual void Close() /* override */ {}
    virtual void AddHandles(CHandleList& listHandles) /* override */ = 0;

// Miscellaneous Implementer's Overrides
protected:
    virtual CRect GetHandleRect(int nHandleID) const /* override */;
    virtual CPoint GetHandleLoc(int nHandleID) const /* override */ = 0;
    virtual int GetHandleCount() const /* override */ = 0;
    virtual void DrawHandles(CDC& pDC) const /* override */;
    virtual void DrawTrackingImage(CDC& pDC, TrackMode eMode) const /* override */ = 0;

// Implementation
protected:
    RefPtr<CPlayBoardView> m_pView;            // Selection's view
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
// CSelLine -- CLine Selection Proxy

class CSelLine : public CSelection
{
// Constructors
public:
    CSelLine(CPlayBoardView& pView, CLine& pObj) : CSelection(pView, pObj)
        { pObj.GetLine(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom); }

// Attributes
public:
    virtual CRect GetRect() const override;

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID) const override;
    virtual void MoveHandle(int m_nHandle, CPoint point) override;

protected:
    virtual void AddHandles(CHandleList& listHandles) override;
    virtual void DrawTrackingImage(CDC& pDC, TrackMode eMode) const override;
    virtual CPoint GetHandleLoc(int nHandleID) const override;
    virtual int GetHandleCount() const override { return 2; }
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
    CSelGeneric(CPlayBoardView& pView, CDrawObj& pObj) : CSelection(pView, pObj)
        { m_rect = pObj.GetRect(); }

// Overrides
public:
    virtual HCURSOR GetHandleCursor(int nHandleID) const override
        { return AfxGetApp()->LoadStandardCursor(IDC_ARROW); }
    virtual void MoveHandle(int m_nHandle, CPoint point) override {}

protected:
    virtual void AddHandles(CHandleList& listHandles) override;
    virtual void DrawTrackingImage(CDC& pDC, TrackMode eMode) const override;
    virtual CPoint GetHandleLoc(int nHandleID) const override;
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

// Constructors
public:
    CSelList(CPlayBoardView& pView) :
        m_pView(&pView),
        m_eTrkMode(trkSelected),
        m_pobjSnapReference(NULL)
    {
    }
    CSelList(const CSelList&) = delete;
    CSelList& operator=(const CSelList&) = delete;
    ~CSelList() { PurgeList(FALSE); }

// Attributes
public:
    BOOL IsMultipleSelects() const { return size() > size_t(1); }
    BOOL IsSingleSelect() const { return size() == size_t(1); }
    BOOL IsAnySelects() const { return !empty(); }
    BOOL IsObjectSelected(const CDrawObj& pObj) const;
    BOOL HasPieces() const;
    BOOL HasOwnedPieces() const;
    BOOL HasNonOwnedPieces() const;
    BOOL HasOwnedPiecesNotMatching(DWORD dwOwnerMask) const;
    BOOL HasMarkers() const;
    BOOL Has2SidedPieces() const;
    BOOL IsMarkerSelected() const;
    // -------- //
    void SetMouseOffset(CSize size) { m_sizeMseOffset = size; }
    CSize GetMouseOffset() const { return m_sizeMseOffset; }
    // -------- //
    void SetSnapReferenceObject(CDrawObj* pObj);
    CRect GetSnapReferenceRect() const;
    // -------- //
    CPlayBoardView& GetView() { return *m_pView; }
    void SetTrackingMode(TrackMode eTrkMode) { m_eTrkMode = eTrkMode; }
    TrackMode GetTrackingMode() const { return m_eTrkMode; }
    CRect GetEnclosingRect() const { return m_rctEncl; }
    CRect GetPiecesEnclosingRect(BOOL bIncludeMarkers = TRUE) const;

// Operations
public:
    CSelection& AddObject(CDrawObj& pObj, BOOL bInvalidate = FALSE);
    void RemoveObject(const CDrawObj& pObj, BOOL bInvalidate = FALSE);
    const CSelection* FindObject(const CDrawObj& pObj) const;
    // -------- //
    enum ObjTypes { otInvalid, otPieces, otPiecesMarks, otAll };
    void LoadTableWithObjectPtrs(std::vector<CB::not_null<CDrawObj*>>& pTbl, ObjTypes objTypes,
        BOOL bVisualOrder);
    void LoadTableWithPieceIDs(std::vector<PieceID>& pTbl, BOOL bVisualOrder = TRUE);
    // -------- //
    enum LoadFilter { LF_NOTOWNED, LF_OWNED, LF_BOTH };
    void LoadTableWithOwnerStatePieceIDs(std::vector<PieceID>& pTbl, LoadFilter eWantOwned, BOOL bVisualOrder = TRUE);
    // -------- //
    void InvalidateListHandles(BOOL bUpdate = FALSE);
    void InvalidateList(BOOL bUpdate = FALSE);
    void PurgeList(BOOL bInvalidate = TRUE);
    // -------- //
    void CountDObjFlags(DWORD dwFlagBits, int& nSet, int& nCleared) const;
    void DeselectIfDObjFlagsSet(DWORD dwFlagBits);
    // -------- //
    int  HitTestHandles(CPoint point) const;
    // -------- //
    void Offset(CPoint ptDelta);
    void MoveHandle(int m_nHandle, CPoint point);
    // -------- //
    void OnDraw(CDC& pDC);  // Called by view OnDraw()
    void DrawTracker(CDC& pDC, TrackMode eTrkMode = trkCurrent);
    // -------- //
    void UpdateObjects(BOOL bInvalidate = TRUE,
        BOOL bUpdateObjectExtent = TRUE );
    // -------- //
    void ForAllSelections(void (*pFunc)(CDrawObj& pObj, DWORD dwUser),
        DWORD dwUserVal);

    void Open();

// Implementation - methods
protected:
    void CalcEnclosingRect();

    void RegenerateHandleList();

// Implementation - vars
protected:
    RefPtr<CPlayBoardView> m_pView;        // Selection's view
    TrackMode       m_eTrkMode;     // Current list tracking mode.
    CRect           m_rctEncl;      // Enclosing rect.
    CSize           m_sizeMseOffset;// Mouse offset from upper left m_rctEncl
    CDrawObj*       m_pobjSnapReference;// Reference for snap grid processing

    // List of uniqeue handle coordinates. No coordinate is duplicated.
    // It is only used to draw handles.
    CHandleList     m_listHandles;
};

#endif

