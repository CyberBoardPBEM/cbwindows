// DrawObj.h
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

#ifndef _DRAWOBJ_H
#define _DRAWOBJ_H

#ifndef  _FONT_H
#include "Font.h"
#endif

#if !defined(_PIECES_H) && defined(GPLAY)
#include "Pieces.h"
#endif

#if !defined(_MARKS_H) && defined(GPLAY)
#include "Marks.h"
#endif

class CSelection;

#ifdef GPLAY
class CPlayBoardView;
class CGamDoc;
class ObjectID;
#else
class CBrdEditView;
#endif

///////////////////////////////////////////////////////////////////////
// CDrawObj flag. Note, some times these flags are just cargo
// for processing outside the base draw list processing.
// IMPORTANT! Maintain these bit values since they are saved to
// files.

enum DrawObjFlags
{
    dobjFlgLayerMask    = 0x00000007,   // Lower 3 bits are same as OR of TileScale
    dobjFlgDrawPass2    = 0x00000008,   // Used to set what pass an object is drawn in
    dobjFlgLayerNatural = 0x00000010,   // dobjFlgLayerMask is natural scale (bitmaps only)

    dobjFlgLockDown  = 0x00010000   // If set, piece/marker can't be moved
};

///////////////////////////////////////////////////////////////////////

inline DWORD ChangeBits(DWORD dwBase, DWORD dwValue, DWORD dwMask)
{
    return (dwBase & ~dwMask) | (dwValue & dwMask);
}

class CDrawObj
{
public:
    CDrawObj();
    virtual ~CDrawObj() {}
// Attributes
public:
    // IMPORTANT:
    // Don't arbitrarily mess with the values of these since they
    // are stored in files as object identifiers.
    enum CDrawObjType { drawRect = 0, drawEllipse = 1, drawLine = 2,
        drawTile = 3, drawText = 4, drawPolygon = 5, drawBitmap = 6,
        drawPieceObj = 0x80, drawMarkObj = 0x81, drawLineObj = 0x82,
        drawUnknown = 0xFF };

    void SetScaleVisibility(int fTileScale)
        { m_dwDObjFlags = ChangeBits(m_dwDObjFlags, fTileScale, dobjFlgLayerMask); }
    int GetScaleVisibility() { return (int)(GetDObjFlags() & dobjFlgLayerMask); }
    DWORD GetDObjFlags() { return m_dwDObjFlags; }
    void  SetDObjFlags(DWORD dwFlags) { m_dwDObjFlags |= dwFlags; }
    void  ClearDObjFlags(DWORD dwFlags) { m_dwDObjFlags &= ~dwFlags; }
    void  ModifyDObjFlags(DWORD dwFlags, BOOL bState)
        { if (bState) SetDObjFlags(dwFlags); else ClearDObjFlags(dwFlags); }

    virtual CRect& GetRect() { return m_rctExtent; }
    virtual const void SetRect(const CRect& rct) { m_rctExtent = rct; }

    virtual CRect GetEnclosingRect() { return m_rctExtent; }
    virtual BOOL HitTest(CPoint pt) { return FALSE; }
#ifdef GPLAY
    virtual ObjectID GetObjectID();
    virtual void MoveObject(CPoint ptUpLeft);
#endif
    virtual void OffsetObject(CPoint offset);
    // ----- //
    virtual BOOL IsVisible(RECT* pClipRct);
    virtual CDrawObjType GetType() const = 0;
    // ----- //
    // Some functions that may or may not mean anything to the
    // underlying object. The object will return TRUE if the
    // property was meaningful.
    virtual BOOL SetForeColor(COLORREF cr) { return FALSE; }
    virtual BOOL SetBackColor(COLORREF cr) { return FALSE; }
    virtual BOOL SetLineWidth(UINT nLineWidth) { return FALSE; }
    virtual BOOL SetFont(FontID fid) { return FALSE; }

// Operations
public:
    virtual void Draw(CDC* pDC, TileScale eScale) = 0;
    // Support required by selection objects
#ifdef GPLAY
    virtual CSelection* CreateSelectProxy(CPlayBoardView* pView) { return NULL;}
#else
    virtual CSelection* CreateSelectProxy(CBrdEditView* pView) { return NULL;}
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(CRect* pRctZone) = 0;
#endif
    virtual CDrawObj* Clone() { return NULL; }
#ifdef GPLAY
    virtual CDrawObj* Clone(CGamDoc* pDoc) { return NULL; }
    virtual BOOL Compare(CDrawObj* pObj) { return FALSE; }
#endif
    // ------- //
    virtual void Serialize(CArchive& ar);

// Implementation - methods
protected:
    BOOL IsExtentOutOfZone(CRect* pRctZone, CPoint& pntOffset);
    // ------- //
    virtual void SetUpDraw(CDC* pDC, CPen* pPen, CBrush* pBrush);
    virtual void CleanUpDraw(CDC *pDC);
    virtual BOOL BitBlockHitTest(CPoint pt);
    virtual UINT GetLineWidth() { return 0; }
    virtual COLORREF GetLineColor() { return noColor; }
    virtual COLORREF GetFillColor() { return noColor; }

    void CopyAttributes (CDrawObj* source);    //DFM19991213

// Implementation - vars
protected:
    DWORD   m_dwDObjFlags;      // OR'ed values from enum DrawObjFlags
    CRect   m_rctExtent;

    // Class variables (may be used to during draw of various offspring
    // class objects)
    static CPen*    c_pPrvPen;
    static CBrush*  c_pPrvBrush;
    static BOOL     c_bHitTestDraw;// TRUE=Draw() is being called for hit test.
};

///////////////////////////////////////////////////////////////////////

#ifdef GPLAY
/* ObjectID is the unique identifier for an object that is
    created by CBPlay during a game.  As such, it is possible
    that two (or more) players could be creating objects in
    parallel.  To give a good probability of these objects
    having distinct values, but not require a public NewID
    server, the main part of the ObjectID is a 16bit random
    number.  Since we're just playing games, not running
    critical infrastructure, this is good enough.

    Currently, the only CBPlay-created objects are markers.
    However, the subtype field permits 13 more additional types
    (subtype 0xF is used up by GameElement's
    GAMEELEM_MARKERID_FLAG, subtype 0x2 is markers, and
    subtype 0x0 is used up as the invalid subtype code).
    Possible future objects:  lines showing moves (subtype 0x3
    already reserved), textual notes, multimedia clips, ... */
class alignas(uint32_t) ObjectID
{
public:
    ObjectID();
    ObjectID(uint16_t i, uint16_t s, CDrawObj::CDrawObjType t);
    explicit ObjectID(PieceID pid);
    explicit ObjectID(DWORD dw);

    bool operator==(const ObjectID& rhs) const
    {
        return reinterpret_cast<const uint32_t&>(*this) == reinterpret_cast<const uint32_t&>(rhs);
    }
    bool operator!=(const ObjectID& rhs) const
    {
        return !operator==(rhs);
    }

private:
    // a random number (seeded by time in seconds)
    uint16_t id;
    /* serial number to avoid equality of two ObjectID values
        created in the same second */
    uint16_t serial : 12;
    // cooked version of CDrawObj::CDrawObjType
    uint16_t subtype : 4;
};

inline CArchive& operator<<(CArchive& ar, const ObjectID& oid)
{
    return ar << reinterpret_cast<const uint32_t&>(oid);
}

inline CArchive& operator>>(CArchive& ar, ObjectID& oid)
{
    return ar >> reinterpret_cast<uint32_t&>(oid);
}
#endif

///////////////////////////////////////////////////////////////////////

class CRectObj : public CDrawObj
{
// Constructors
public:
    CRectObj()
    { m_crLine = RGB(0,0,0); m_crFill = RGB(255,255,255); m_nLineWidth = 0;};

// Attributes
public:
    COLORREF m_crFill;
    COLORREF m_crLine;
    UINT     m_nLineWidth;

    virtual CRect GetEnclosingRect();
    virtual enum CDrawObjType GetType() const { return drawRect; }

    void SetRect(RECT* rect) { m_rctExtent = rect; }

    virtual BOOL SetForeColor(COLORREF cr) { m_crLine = cr; return TRUE; }
    virtual BOOL SetBackColor(COLORREF cr) { m_crFill = cr; return TRUE; }
    virtual BOOL SetLineWidth(UINT nLineWidth)
        { m_nLineWidth = nLineWidth; return TRUE; }

// Operations
public:
    virtual void Draw(CDC* pDC, TileScale eScale);

    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt);
#ifndef GPLAY
    virtual CSelection* CreateSelectProxy(CBrdEditView* pView);
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(CRect* pRctZone);
#endif
    // ------- //
    virtual CDrawObj* Clone();                  //DFM19991210

    virtual void Serialize(CArchive& ar);
// Implementation
protected:
    virtual UINT GetLineWidth() { return m_nLineWidth; }
    virtual COLORREF GetLineColor() { return m_crLine; }
    virtual COLORREF GetFillColor() { return m_crFill; }

    void CopyAttributes (CRectObj* source);     //DFM19991213
};

///////////////////////////////////////////////////////////////////////

class CEllipse : public CRectObj
{
// Constructors
public:
    CEllipse() {}

    virtual enum CDrawObjType GetType() const { return drawEllipse; }

// Operations
public:
    virtual void Draw(CDC* pDC, TileScale eScale);
#ifndef GPLAY
    virtual CSelection* CreateSelectProxy(CBrdEditView* pView);
#endif
    virtual CDrawObj* Clone();  //DFM19991210
};

///////////////////////////////////////////////////////////////////////

class CLine : public CDrawObj
{
// Constructors
public:
    CLine() { m_crLine = RGB(0, 0, 0); m_nLineWidth = 0; }
// Attributes
public:
    CPoint  m_ptBeg;
    CPoint  m_ptEnd;

    COLORREF m_crLine;
    UINT     m_nLineWidth;

    void SetLine(int xBeg, int yBeg, int xEnd, int yEnd);
    void GetLine(CRect& rct)
        { rct.left =m_ptBeg.x; rct.top = m_ptBeg.y;
            rct.right = m_ptEnd.x; rct.bottom = m_ptEnd.y; }
    void GetLine(long& xBeg, long& yBeg, long& xEnd, long& yEnd)
        { xBeg = m_ptBeg.x; yBeg = m_ptBeg.y; xEnd = m_ptEnd.x; yEnd = m_ptEnd.y; }
    void GetLine(POINT& ptBeg, POINT& ptEnd)
        { ptBeg = m_ptBeg; ptEnd = m_ptEnd; }

    virtual CRect GetEnclosingRect();
    virtual enum CDrawObjType GetType() const { return drawLine; }

    virtual BOOL SetForeColor(COLORREF cr) { m_crLine = cr; return TRUE; }
    virtual BOOL SetLineWidth(UINT nLineWidth)
        { m_nLineWidth = nLineWidth; return TRUE; }

// Operations
public:
    virtual void Draw(CDC* pDC, TileScale eScale);
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt);
#ifdef GPLAY
    virtual CSelection* CreateSelectProxy(CPlayBoardView* pView);
#else
    virtual CSelection* CreateSelectProxy(CBrdEditView* pView);
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(CRect* pRctZone);
#endif
    virtual void OffsetObject(CPoint offset);       //DFM19991221
    // ------- //
    virtual CDrawObj* Clone(void);                  //DFM19991210
#ifdef GPLAY
    virtual CDrawObj* Clone(CGamDoc* pDoc);
    virtual BOOL Compare(CDrawObj* pObj);
#endif
    virtual void Serialize(CArchive& ar);

protected:
    void CopyAttributes (CLine *source);            //DFM19991214
};

///////////////////////////////////////////////////////////////////////

class CPolyObj : public CDrawObj
{
// Constructors
public:
    CPolyObj()
    { m_crLine = RGB(0,0,0); m_crFill = noColor; m_nLineWidth = 0;
      m_nPnts = 0; m_pPnts = NULL; }
    ~CPolyObj() { if (m_pPnts != NULL) delete m_pPnts; }

// Attributes
public:
    COLORREF m_crFill;
    COLORREF m_crLine;
    UINT     m_nLineWidth;
    POINT*   m_pPnts;
    int      m_nPnts;

    virtual CRect GetEnclosingRect();
    virtual enum CDrawObjType GetType() const { return drawPolygon; }

    virtual BOOL SetForeColor(COLORREF cr) { m_crLine = cr; return TRUE; }
    virtual BOOL SetBackColor(COLORREF cr) { m_crFill = cr; return TRUE; }
    virtual BOOL SetLineWidth(UINT nLineWidth)
        { m_nLineWidth = nLineWidth; return TRUE; }

// Operations
public:
    void AddPoint(CPoint pnt);
    void SetNewPolygon(POINT* pPnts, int nPnts);

// Overrides
public:
    virtual void Draw(CDC* pDC, TileScale eScale);

    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt);
#ifndef GPLAY
    virtual CSelection* CreateSelectProxy(CBrdEditView* pView);
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(CRect* pRctZone);
#endif
    virtual void OffsetObject(CPoint offset); //DFM19991221
    // ------- //
    virtual CDrawObj* Clone();

    virtual void Serialize(CArchive& ar);
// Implementation
protected:
    void ComputeExtent();

    void CopyAttributes(CPolyObj *source);

    // ------- //
    virtual UINT GetLineWidth() { return m_nLineWidth; }
    virtual COLORREF GetLineColor() { return m_crLine; }
    virtual COLORREF GetFillColor() { return m_crFill; }
};

///////////////////////////////////////////////////////////////////////

class CText : public CDrawObj
{
// Constructors
public:
    CText() { m_crText = RGB(0, 0, 0); m_fontID = 0; m_nAngle = 0;}
    ~CText();
// Attributes
public:
    int         m_nAngle;
    COLORREF    m_crText;
    CString     m_text;
    FontID      m_fontID;

    void SetText(int x, int y, const char* pszText, FontID fntID,
        COLORREF crText = RGB(255,255,255));
    virtual enum CDrawObjType GetType() const { return drawText; }

    virtual BOOL SetForeColor(COLORREF cr) { m_crText = cr; return TRUE; }
    virtual BOOL SetFont(FontID fid);

// Operations
public:
    virtual void Draw(CDC* pDC, TileScale eScale);
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt);
#ifndef GPLAY
    virtual CSelection* CreateSelectProxy(CBrdEditView* pView);
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(CRect* pRctZone);
#endif
    virtual void OffsetObject(CPoint offset);
    // ------- //
    virtual CDrawObj* Clone();

    void CopyAttributes(CText *source);

    virtual void Serialize(CArchive& ar);
};

///////////////////////////////////////////////////////////////////////

class CBitmapImage : public CDrawObj
{
// Constructors
public:
    CBitmapImage() {}
// Attributes
public:
    TileScale       m_eBaseScale;       // Base 1:1 scale. Stretches to other scales
    CBitmap         m_bitmap;

    void SetBitmap(int x, int y, HBITMAP hBMap, TileScale eBaseScale = fullScale);
    virtual enum CDrawObjType GetType() const { return drawBitmap; }

// Operations
public:

    virtual void Draw(CDC* pDC, TileScale eScale);
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt);
#ifndef GPLAY
    virtual CSelection* CreateSelectProxy(CBrdEditView* pView);
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(CRect* pRctZone);
#endif
    virtual void OffsetObject(CPoint offset);
    // ------- //
    virtual CDrawObj* Clone();

    void CopyAttributes(CBitmapImage *source);

    virtual void Serialize(CArchive& ar);

protected:
    void SynchronizeExtentRect(CSize sizeWorld, CSize sizeView);
};

///////////////////////////////////////////////////////////////////////

class CTileImage : public CDrawObj
{
// Constructors
public:
    CTileImage() : m_tid(nullTid) { m_pTMgr = NULL; }
    CTileImage(CTileManager* pTMgr) : m_tid(nullTid) { m_pTMgr = pTMgr; }
// Attributes
public:
    TileID          m_tid;
    CTileManager*   m_pTMgr;

    void SetTile(int x, int y, TileID tid);
    virtual enum CDrawObjType GetType() const { return drawTile; }
// Operations
public:
    virtual void Draw(CDC* pDC, TileScale eScale);
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt);
#ifndef GPLAY
    virtual CSelection* CreateSelectProxy(CBrdEditView* pView);
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(CRect* pRctZone);
#endif
    virtual void OffsetObject(CPoint offset);
    // ------- //
    virtual CDrawObj* Clone();

    void CopyAttributes(CTileImage *source);

    virtual void Serialize(CArchive& ar);
};

/////////////////////////////////////////////////////////////////////////
//    The following classes are only used in the Game Player module    //
/////////////////////////////////////////////////////////////////////////

#ifdef GPLAY

class CPieceTable;

class CPieceObj : public CDrawObj
{
// Constructors
public:
    CPieceObj() { m_pid = nullPid; m_pDoc = NULL; }
    CPieceObj(CGamDoc* pDoc) { m_pid = nullPid; m_pDoc = pDoc; }

// Attributes
public:
    PieceID         m_pid;
    CGamDoc*        m_pDoc;


    void SetPiece(CRect& rct, PieceID pid);

    BOOL IsOwned();
    BOOL IsOwnedBy(DWORD dwMask);
    BOOL IsOwnedButNotByCurrentPlayer();
    void SetOwnerMask(DWORD dwMask);

    virtual enum CDrawObjType GetType() const { return drawPieceObj; }
    virtual ObjectID GetObjectID() { return static_cast<ObjectID>(m_pid); }

// Operations
public:
    void ResyncExtentRect();

    virtual void Draw(CDC* pDC, TileScale eScale);
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt);
    virtual CSelection* CreateSelectProxy(CPlayBoardView* pView);
    // ------- //
    virtual CDrawObj* Clone(CGamDoc* pDoc);
    virtual BOOL Compare(CDrawObj* pObj);
    virtual void Serialize(CArchive& ar);
};

///////////////////////////////////////////////////////////////////////

class CLineObj : public CLine
{
// Constructors
public:
    CLineObj() {}
// Other...
protected:
    virtual enum CDrawObjType GetType() const { return drawLineObj; }

    ObjectID    m_dwObjectID;
public:
    virtual ObjectID GetObjectID() { return m_dwObjectID; }
    void    SetObjectID(ObjectID dwID) { m_dwObjectID = dwID; }
    // ------ //
    virtual CDrawObj* Clone(CGamDoc* pDoc);
    virtual BOOL Compare(CDrawObj* pObj);
    virtual void Serialize(CArchive& ar);
};

///////////////////////////////////////////////////////////////////////

class CMarkObj : public CDrawObj
{
// Constructors
public:
    CMarkObj() { m_mid = nullMid; m_pDoc = NULL; m_nFacingDegCW = 0; }
    CMarkObj(CGamDoc* pDoc)
        { m_mid = nullMid; m_pDoc = pDoc; m_nFacingDegCW = 0; }

// Attributes
public:
    MarkID          m_mid;
    CGamDoc*        m_pDoc;

    void SetMark(CRect& rct, MarkID mid);

    void SetFacing(int nFacingDegCW) { m_nFacingDegCW = nFacingDegCW; }
    int  GetFacing() { return m_nFacingDegCW; }

    void    SetObjectID(ObjectID dwID) { m_dwObjectID = dwID; }
    virtual ObjectID GetObjectID() { return m_dwObjectID; }

    void ResyncExtentRect();

    virtual enum CDrawObjType GetType() const { return drawMarkObj; }
protected:
    ObjectID m_dwObjectID;
    int      m_nFacingDegCW;           // Rotation of marker (degrees)

// Operations
public:
    TileID GetCurrentTileID();
    // ------- //
    virtual void Draw(CDC* pDC, TileScale eScale);
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt);
    virtual CSelection* CreateSelectProxy(CPlayBoardView* pView);
    // ------- //
    virtual CDrawObj* Clone(CGamDoc* pDoc);
    virtual BOOL Compare(CDrawObj* pObj);
    virtual void Serialize(CArchive& ar);
};

#endif  // GPLAY

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// The DrawList manager class

class CDrawList : public CPtrList
{
    friend class CGamDoc;
public:
    ~CDrawList() { Flush(); }
public:
    void AddObject(CDrawObj* pDrawObj) { AddToFront(pDrawObj); }
    void RemoveObject(CDrawObj* pDrawObj);
    void RemoveObjectsInList(CPtrList* pLst);
    void AddToEnd(CDrawObj* pDrawObj) { AddHead(pDrawObj); }
    void AddToBack(CDrawObj* pDrawObj) { AddHead(pDrawObj); }
    void AddToFront(CDrawObj* pDrawObj) { AddTail(pDrawObj); }
    void Flush();
    void Draw(CDC* pDC, CRect* pDrawRct, TileScale eScale,
        BOOL bApplyVisibility = TRUE, BOOL bDrawPass2Objects = FALSE,
        BOOL bHideUnlocked = FALSE, BOOL bDrawLockedFirst = FALSE);
    CDrawObj* HitTest(CPoint pt, TileScale eScale = (TileScale)AllTileScales,
        BOOL bApplyVisibility = TRUE);
    void DrillDownHitTest(CPoint point, CPtrList* selLst,
        TileScale eScale = (TileScale)AllTileScales, BOOL bApplyVisibility = TRUE);
#ifdef GPLAY
    void ArrangePieceTableInDrawOrder(std::vector<PieceID>& pTbl);
    void ArrangePieceTableInVisualOrder(std::vector<PieceID>& pTbl);
#endif
    void ArrangeObjectListInDrawOrder(CPtrList* pLst);
    void ArrangeObjectListInVisualOrder(CPtrList* pLst);
    void ArrangeObjectPtrTableInDrawOrder(std::vector<CDrawObj*>& pTbl);
    void ArrangeObjectPtrTableInVisualOrder(std::vector<CDrawObj*>& pTbl);
#ifdef GPLAY
    void SetOwnerMasks(DWORD dwOwnerMask);

    CPieceObj* FindPieceID(PieceID pid);
    CDrawObj* FindObjectID(ObjectID oid);
    BOOL HasObject(CDrawObj* pObj) { return Find(pObj) != NULL; }
    BOOL HasMarker();
    void GetPieceObjectPtrList(CPtrList* pLst);
    void GetPieceIDTable(std::vector<PieceID>& pTbl);
    void GetObjectListFromPieceIDTable(const std::vector<PieceID>& pTbl, CPtrList* pLst);
    static void GetPieceIDTableFromObjList(CPtrList* pLst, std::vector<PieceID>& pTbl,
        BOOL bDeleteObjs = FALSE);
    // ------- //
    CDrawList* Clone(CGamDoc* pDoc);
    void Restore(CGamDoc* pDoc, CDrawList* pLst);
    BOOL Compare(CDrawList* pLst);
    void AppendWithOffset(CDrawList* pSourceLst, CPoint pntOffet);
#else
    BOOL PurgeMissingTileIDs(CTileManager* pTMgr);
    BOOL IsTileInUse(TileID tid);
    // ------- //
    void ForceIntoZone(CRect* pRctZone);
#endif
    // -------- //
    void Serialize(CArchive& ar);
};

#endif

