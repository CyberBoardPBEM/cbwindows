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

#include <list>

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
class ObjectID32;
class ObjectID64;
using ObjectID = std::conditional_t<std::is_same_v<PieceID, PieceID16>, ObjectID32, ObjectID64>;
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
    using OwnerPtr = OwnerPtr<CDrawObj>;

    CDrawObj();
    CDrawObj(const CDrawObj&) = delete;
    CDrawObj& operator=(const CDrawObj&) = delete;
    virtual ~CDrawObj() = default;
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
    int GetScaleVisibility() const { return (int)(GetDObjFlags() & dobjFlgLayerMask); }
    DWORD GetDObjFlags() const { return m_dwDObjFlags; }
    void  SetDObjFlags(DWORD dwFlags) { m_dwDObjFlags |= dwFlags; }
    void  ClearDObjFlags(DWORD dwFlags) { m_dwDObjFlags &= ~dwFlags; }
    void  ModifyDObjFlags(DWORD dwFlags, BOOL bState)
        { if (bState) SetDObjFlags(dwFlags); else ClearDObjFlags(dwFlags); }

    virtual CRect GetRect() const /* override */ = 0;
    virtual void SetRect(const CRect& rct) /* override */ = 0;

    virtual CRect GetEnclosingRect() const /* override */ { return GetRect(); }
    virtual BOOL HitTest(CPoint pt) /* override */ { return FALSE; }
#ifdef GPLAY
    virtual ObjectID GetObjectID() const /* override */;
    virtual void MoveObject(CPoint ptUpLeft) /* override */;
    /* N.B.:  only for use in serializing GeoBoards,
                not general purpose rotate */
    virtual void Rotate(Rotation90 rot) /* override */;
#endif
    virtual void OffsetObject(CPoint offset) /* override */;
    // ----- //
    virtual BOOL IsVisible(const RECT& pClipRct) const /* override */;
    virtual CDrawObjType GetType() const /* override */ = 0;
    // ----- //
    // Some functions that may or may not mean anything to the
    // underlying object. The object will return TRUE if the
    // property was meaningful.
    virtual BOOL SetForeColor(COLORREF cr) /* override */ { return FALSE; }
    virtual BOOL SetBackColor(COLORREF cr) /* override */ { return FALSE; }
    virtual BOOL SetLineWidth(UINT nLineWidth) /* override */ { return FALSE; }
    virtual BOOL SetFont(FontID fid) /* override */ { return FALSE; }

// Operations
public:
    virtual void Draw(CDC& pDC, TileScale eScale) /* override */ = 0;
    // Support required by selection objects
#ifdef GPLAY
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CPlayBoardView& pView) /* override */ { AfxThrowInvalidArgException(); }
#else
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CBrdEditView& pView) /* override */ { AfxThrowInvalidArgException(); }
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(const CRect& pRctZone) /* override */ = 0;
#endif
    virtual OwnerPtr Clone() const /* override */ { AfxThrowInvalidArgException(); }
#ifdef GPLAY
    virtual OwnerPtr Clone(CGamDoc* pDoc) const /* override */ { AfxThrowInvalidArgException(); }
    virtual BOOL Compare(const CDrawObj& pObj) const /* override */ { AfxThrowInvalidArgException(); }
#endif
    // ------- //
    virtual void Serialize(CArchive& ar) /* override */;

// Implementation - methods
protected:
    BOOL IsExtentOutOfZone(const CRect& pRctZone, CPoint& pntOffset) const;
    // ------- //
    virtual void SetUpDraw(CDC& pDC, CPen& pPen, CBrush& pBrush) const /* override */;
    virtual void CleanUpDraw(CDC& pDC) const /* override */;
    virtual BOOL BitBlockHitTest(CPoint pt) /* override */;
    virtual UINT GetLineWidth() const /* override */ { return 0; }
    virtual COLORREF GetLineColor() const /* override */ { return noColor; }
    virtual COLORREF GetFillColor() const /* override */ { return noColor; }

    void CopyAttributes (const CDrawObj& source);    //DFM19991213

// Implementation - vars
protected:
    DWORD   m_dwDObjFlags;      // OR'ed values from enum DrawObjFlags

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

    N.B.:  ObjectID is sometimes reinterpreted as GameElement

    Currently, the only CBPlay-created objects are markers.
    However, the subtype field permits 12 more additional types
    (subtype 0xF is used up by GameElement's
    GAMEELEM_MARKERID_FLAG, subtype 0x0 is pieceObjs,
    subtype 0x2 is markerOjbs, and subtype 0x1 is used up as the
    invalid subtype code).
    Possible future objects:  lines showing moves (subtype 0x3
    already reserved), textual notes, multimedia clips, ... */
/* WARNING:  Strictly speaking, I believe this code is depending
        on undefined behavior (it reads from elements of the
        union other than the one that was most recently
        written). However, I think the code is a lot more
        readable this way rather than doing this work using bit
        twiddling within an uintXX_t, which would be the
        well-defined approach.  */
class alignas(uint32_t) ObjectID32
{
public:
    typedef uint32_t UNDERLYING_TYPE;

    /* N.B.:  CB3.1 always seemed to init ObjectID so this is
                the invalid value, not uninitialized data */
    constexpr ObjectID32() = default;
    ObjectID32(uint16_t i, uint16_t s, CDrawObj::CDrawObjType t);
    explicit ObjectID32(PieceID16 pid);
#if !defined(NDEBUG)
    [[deprecated("only for GetObjectIDFromElementLegacyCheck()")]] explicit ObjectID32(UNDERLYING_TYPE dw);
#endif
    ObjectID32(const ObjectID32&) = default;
    ObjectID32& operator=(const ObjectID32&) = default;
    ~ObjectID32() = default;

    bool operator==(const ObjectID32& rhs) const
    {
        if (u.tag.subtype != rhs.u.tag.subtype)
        {
            return false;
        }

        switch (u.tag.subtype)
        {
            case stPieceObj:
                return u.pieceObj.pid == rhs.u.pieceObj.pid;
            case stInvalid:
                return true;
            case stMarkObj:
                return u.markObj.id == rhs.u.markObj.id &&
                        u.markObj.serial == rhs.u.markObj.serial;
            case stMarkerID:
                ASSERT(!"conflicts with GameElement(MarkID)");
                // fall through
            default:
                CbThrowBadCastException();
        }
    }
    bool operator!=(const ObjectID32& rhs) const
    {
        return !operator==(rhs);
    }

    void Serialize(CArchive& ar) const;
    void Serialize(CArchive& ar);

private:
    /* N.B.:  currently only 4 bits (values 0 - 15) available!
                but, declare underlying type uint16_t to
                let it fit in the bit fields better */
    enum Subtype : uint16_t {
        stPieceObj = 0,
        stInvalid = 1,
        stMarkObj = 2,
        stLineObj = 3,
        stMarkerID = 0xf,
    };
    union U {
        struct Tag
        {
            uint16_t : 16;
            uint16_t : 12;
            Subtype subtype : 4;
        private:
            constexpr Tag() = default;
        } tag;
        struct MarkObj
        {
            // a random number (seeded by time in seconds)
            uint16_t id;
            /* serial number to avoid equality of two ObjectID values
                created in the same second */
            uint16_t serial : 12;
            Subtype subtype : 4;

            constexpr MarkObj(uint16_t i, uint16_t s, CDrawObj::CDrawObjType t) :
                id(i),
                serial(s),
                subtype((ASSERT(t == CDrawObj::drawMarkObj), stMarkObj))
            {
            }
        } markObj;
        struct PieceObj
        {
            PieceID16 pid;
            uint16_t pad : 12;
            Subtype subtype : 4;

            constexpr PieceObj(PieceID16 p) :
                pid(p),
                pad(0),
                subtype(stPieceObj)
            {
            }
        } pieceObj;
        struct Invalid
        {
            uint16_t pad1 : 16;
            uint16_t pad2 : 12;
            Subtype subtype : 4;

            constexpr Invalid() :
                pad1(0),     // GameElement32 Hash() won't work properly if bits uninitialized
                pad2(0),
                subtype(stInvalid)
            {
            }
        } invalid;
        UNDERLYING_TYPE buf;

        constexpr U() : invalid() {}
        U(uint16_t i, uint16_t s, CDrawObj::CDrawObjType t) :
            markObj(i, s, t)
        {
        }
        U(PieceID16 p) :
            pieceObj(p)
        {
        }
        U(const U&) = default;
        U& operator=(const U&) = default;
        ~U() = default;
    } u;

    // Serialize conversion helpers
    /* use ObjectID32::operator ObjectID64() const instead of
        ObjectID64::ObjectID64(const ObjectID32&) to preserve
        privacy of ObjectID32's content */
    explicit operator ObjectID64() const;
    friend class SerializeBackdoorObjectID;

    friend std::formatter<ObjectID32, char>;
    friend std::formatter<ObjectID32, wchar_t>;
};

template<typename CharT>
struct std::formatter<ObjectID32, CharT> : private std::formatter<ObjectID32::UNDERLYING_TYPE, CharT>
{
private:
    using BASE = formatter<ObjectID32::UNDERLYING_TYPE, CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(ObjectID32 oid, FormatContext& ctx)
    {
        return BASE::format(oid.u.buf, ctx);
    }
};

inline CArchive& operator<<(CArchive& ar, const ObjectID32& oid)
{
    oid.Serialize(ar);
    return ar;
}

inline CArchive& operator>>(CArchive& ar, ObjectID32& oid)
{
    oid.Serialize(ar);
    return ar;
}

class alignas(uint64_t) ObjectID64
{
public:
    typedef uint64_t UNDERLYING_TYPE;

    constexpr ObjectID64() = default;
    ObjectID64(uint16_t i, uint16_t s, CDrawObj::CDrawObjType t);
    explicit ObjectID64(PieceID32 pid);
    ObjectID64(const ObjectID64&) = default;
    ObjectID64& operator=(const ObjectID64&) = default;
    ~ObjectID64() = default;

    bool operator==(const ObjectID64& rhs) const
    {
        if (u.tag.subtype != rhs.u.tag.subtype)
        {
            return false;
        }

        switch (u.tag.subtype)
        {
            case stPieceObj:
                return u.pieceObj.pid == rhs.u.pieceObj.pid;
            case stInvalid:
                return true;
            case stMarkObj:
                return u.markObj.id == rhs.u.markObj.id &&
                    u.markObj.serial == rhs.u.markObj.serial;
            case stMarkerID:
                ASSERT(!"conflicts with GameElement(MarkID)");
                // fall through
            default:
                CbThrowBadCastException();
        }
    }
    bool operator!=(const ObjectID64& rhs) const
    {
        return !operator==(rhs);
    }

    void Serialize(CArchive& ar) const;
    void Serialize(CArchive& ar);

private:
    /* N.B.:  currently only 4 bits (values 0 - 15) available!
                but, declare underlying type uint16_t to
                let it fit in the bit fields better */
    enum Subtype : uint16_t {
        stPieceObj = 0,
        stInvalid = 1,
        stMarkObj = 2,
        stLineObj = 3,
        stMarkerID = 0xf,
    };
    union U {
        struct Tag
        {
            uint32_t : 32;
            uint16_t : 16;
            uint16_t : 12;
            Subtype subtype : 4;
        private:
            constexpr Tag() = default;
        } tag;
        struct MarkObj
        {
            // TODO:  since we have more space, more random bits?
            // a random number (seeded by time in seconds)
            uint16_t id;
            /* serial number to avoid equality of two ObjectID values
                created in the same second */
            uint16_t serial : 12;
            uint16_t pad1 : 4;
            uint16_t pad2 : 16;
            uint16_t pad3 : 12;
            Subtype subtype : 4;

            constexpr MarkObj(uint16_t i, uint16_t s, CDrawObj::CDrawObjType t) :
                id(i),
                serial(s),
                pad1(0),
                pad2(0),
                pad3(0),
                subtype((ASSERT(t == CDrawObj::drawMarkObj), stMarkObj))
            {
            }
        } markObj;
        struct PieceObj
        {
            PieceID32 pid;
            uint16_t pad1 : 16;
            uint16_t pad2 : 12;
            Subtype subtype : 4;

            constexpr PieceObj(PieceID32 p) :
                pid(p),
                pad1(0),
                pad2(0),
                subtype(stPieceObj)
            {
            }
        } pieceObj;
        struct Invalid
        {
            uint32_t pad1 : 32;
            uint16_t pad2 : 16;
            uint16_t pad3 : 12;
            Subtype subtype : 4;

            constexpr Invalid() :
                pad1(0),     // GameElement64 Hash() won't work properly if bits uninitialized
                pad2(0),
                pad3(0),
                subtype(stInvalid)
            {
            }
        } invalid;
        UNDERLYING_TYPE buf;

        constexpr U() : invalid() {}
        U(uint16_t i, uint16_t s, CDrawObj::CDrawObjType t) :
            markObj(i, s, t)
        {
        }
        U(PieceID32 p) :
            pieceObj(p)
        {
        }
        U(const U&) = default;
        U& operator=(const U&) = default;
        ~U() = default;
    } u;

    // Serialize conversion helpers
    explicit operator ObjectID32() const;
    friend class SerializeBackdoorObjectID;

    friend std::formatter<ObjectID64, char>;
    friend std::formatter<ObjectID64, wchar_t>;
};

template<typename CharT>
struct std::formatter<ObjectID64, CharT>
{
public:
    template<typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
        typename ParseContext::iterator begin = ctx.begin();
        typename ParseContext::iterator retval = fmtUL.parse(ctx);
        ASSERT(ctx.begin() == begin);
        VERIFY(fmtPID.parse(ctx) == retval);
        return retval;
    }

    template<typename FormatContext>
    FormatContext::iterator format(ObjectID64 oid, FormatContext& ctx)
    {
        switch (oid.u.tag.subtype)
        {
            case ObjectID64::stPieceObj:
                std::format_to(ctx.out(), "ObjectID64(pid:");
                fmtPID.format(oid.u.pieceObj.pid, ctx);
                return std::format_to(ctx.out(), ")");
            case ObjectID64::stInvalid:
                return std::format_to(ctx.out(), "ObjectID64(invalid)");
            case ObjectID64::stMarkObj:
                std::format_to(ctx.out(), "ObjectID64(markobj(id:");
                fmtUL.format(oid.u.markObj.id, ctx);
                std::format_to(ctx.out(), ", serial:");
                fmtUL.format(oid.u.markObj.serial, ctx);
                return std::format_to(ctx.out(), "))");
            case ObjectID64::stLineObj:
                // not implemented
                [[fallthrough]];
            case ObjectID64::stMarkerID:
                // GameElement64
                [[fallthrough]];
            default:
                ASSERT(!"illegal ObjectID subtype");
                std::format_to(ctx.out(), "ObjectID64(unknown:");
                fmtUL.format(oid.u.buf, ctx);
                return std::format_to(ctx.out(), ")");
        }
    }

private:
    formatter<ObjectID64::UNDERLYING_TYPE, CharT> fmtUL;
    formatter<PieceID32, CharT> fmtPID;
};

inline CArchive& operator<<(CArchive& ar, const ObjectID64& oid)
{
    oid.Serialize(ar);
    return ar;
}

inline CArchive& operator>>(CArchive& ar, ObjectID64& oid)
{
    oid.Serialize(ar);
    return ar;
}

class SerializeBackdoorObjectID : public SerializeBackdoor
{
public:
    using SerializeBackdoor::Convert;
    static ObjectID64 Convert(const ObjectID32& oid);
    static ObjectID32 Convert(const ObjectID64& oid);
};
#endif

///////////////////////////////////////////////////////////////////////

class CDrawObj_SimplRctExtent : public CDrawObj
{
public:
    CDrawObj_SimplRctExtent() { m_rctExtent.SetRectEmpty(); }
    CRect GetRect() const override { return m_rctExtent; }
    void SetRect(const CRect& rct) override { m_rctExtent = rct; }

protected:
    void CopyAttributes(const CDrawObj_SimplRctExtent& source);

    CRect m_rctExtent;
};

class CRectObj : public CDrawObj_SimplRctExtent
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

    virtual CRect GetEnclosingRect() const override;
    virtual enum CDrawObjType GetType() const override { return drawRect; }

    virtual BOOL SetForeColor(COLORREF cr) override { m_crLine = cr; return TRUE; }
    virtual BOOL SetBackColor(COLORREF cr) override { m_crFill = cr; return TRUE; }
    virtual BOOL SetLineWidth(UINT nLineWidth) override
        { m_nLineWidth = nLineWidth; return TRUE; }

// Operations
public:
    virtual void Draw(CDC& pDC, TileScale eScale) override;

    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt) override;
#ifndef GPLAY
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CBrdEditView& pView) override;
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(const CRect& pRctZone) override;
#endif
    // ------- //
    virtual OwnerPtr Clone() const override;                  //DFM19991210
#ifdef GPLAY
    virtual OwnerPtr Clone(CGamDoc* /*pDoc*/) const override
    {
        return Clone();
    }
#endif

    virtual void Serialize(CArchive& ar) override;
// Implementation
protected:
    virtual UINT GetLineWidth() const override { return m_nLineWidth; }
    virtual COLORREF GetLineColor() const override { return m_crLine; }
    virtual COLORREF GetFillColor() const override { return m_crFill; }

    void CopyAttributes (const CRectObj& source);     //DFM19991213
};

///////////////////////////////////////////////////////////////////////

class CEllipse : public CRectObj
{
// Constructors
public:
    CEllipse() {}

    virtual enum CDrawObjType GetType() const override { return drawEllipse; }

// Operations
public:
    virtual void Draw(CDC& pDC, TileScale eScale) override;
#ifndef GPLAY
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CBrdEditView& pView) override;
#endif
    virtual OwnerPtr Clone() const override;  //DFM19991210
};

///////////////////////////////////////////////////////////////////////

class CLine : public CDrawObj_SimplRctExtent
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

    virtual CRect GetEnclosingRect() const override;
    virtual enum CDrawObjType GetType() const override { return drawLine; }

    virtual BOOL SetForeColor(COLORREF cr) override { m_crLine = cr; return TRUE; }
    virtual BOOL SetLineWidth(UINT nLineWidth) override
        { m_nLineWidth = nLineWidth; return TRUE; }

// Operations
public:
    virtual void Draw(CDC& pDC, TileScale eScale) override;
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt) override;
#ifdef GPLAY
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CPlayBoardView& pView) override;
#else
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CBrdEditView& pView) override;
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(const CRect& pRctZone) override;
#else
    virtual void Rotate(Rotation90 rot) override;
#endif
    virtual void OffsetObject(CPoint offset) override;       //DFM19991221
    // ------- //
    virtual OwnerPtr Clone(void) const override;                  //DFM19991210
#ifdef GPLAY
    virtual OwnerPtr Clone(CGamDoc* pDoc) const override;
    virtual BOOL Compare(const CDrawObj& pObj) const override;
#endif
    virtual void Serialize(CArchive& ar) override;

protected:
    void CopyAttributes (const CLine& source);            //DFM19991214
};

///////////////////////////////////////////////////////////////////////

class CPolyObj : public CDrawObj_SimplRctExtent
{
// Constructors
public:
    CPolyObj()
    { m_crLine = RGB(0,0,0); m_crFill = noColor; m_nLineWidth = 0; }
    ~CPolyObj() = default;

// Attributes
public:
    COLORREF m_crFill;
    COLORREF m_crLine;
    UINT     m_nLineWidth;
    std::vector<POINT> m_Pnts;

    virtual CRect GetEnclosingRect() const override;
    virtual enum CDrawObjType GetType() const override { return drawPolygon; }

    virtual BOOL SetForeColor(COLORREF cr) override { m_crLine = cr; return TRUE; }
    virtual BOOL SetBackColor(COLORREF cr) override { m_crFill = cr; return TRUE; }
    virtual BOOL SetLineWidth(UINT nLineWidth) override
        { m_nLineWidth = nLineWidth; return TRUE; }

// Operations
public:
    void AddPoint(CPoint pnt);
    void SetNewPolygon(const std::vector<POINT>& pnts);

// Overrides
public:
    virtual void Draw(CDC& pDC, TileScale eScale) override;

    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt) override;
#ifndef GPLAY
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CBrdEditView& pView) override;
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(const CRect& pRctZone) override;
#else
    virtual void Rotate(Rotation90 rot) override;
#endif
    virtual void OffsetObject(CPoint offset) override; //DFM19991221
    // ------- //
    virtual OwnerPtr Clone() const override;

    virtual void Serialize(CArchive& ar) override;
// Implementation
protected:
    void ComputeExtent();

    void CopyAttributes(const CPolyObj& source);

    // ------- //
    virtual UINT GetLineWidth() const override { return m_nLineWidth; }
    virtual COLORREF GetLineColor() const override { return m_crLine; }
    virtual COLORREF GetFillColor() const override { return m_crFill; }
};

///////////////////////////////////////////////////////////////////////

class CText : public CDrawObj_SimplRctExtent
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
    virtual enum CDrawObjType GetType() const override { return drawText; }

    virtual BOOL SetForeColor(COLORREF cr) override { m_crText = cr; return TRUE; }
    virtual BOOL SetFont(FontID fid) override;

// Operations
public:
    virtual void Draw(CDC& pDC, TileScale eScale) override;
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt) override;
#ifndef GPLAY
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CBrdEditView& pView) override;
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(const CRect& pRctZone) override;
#else
    virtual void Rotate(Rotation90 rot) override;
#endif
    // ------- //
    virtual OwnerPtr Clone() const override;

    void CopyAttributes(const CText& source);

    virtual void Serialize(CArchive& ar) override;

private:
    int m_geoRot = 0;
    CSize m_geoOffset = CSize(0, 0);
};

///////////////////////////////////////////////////////////////////////

class CBitmapImage : public CDrawObj_SimplRctExtent
{
// Constructors
public:
    CBitmapImage() {}
// Attributes
public:
    TileScale       m_eBaseScale;       // Base 1:1 scale. Stretches to other scales
    CBitmap         m_bitmap;

    void SetBitmap(int x, int y, HBITMAP hBMap, TileScale eBaseScale = fullScale);
    virtual enum CDrawObjType GetType() const override { return drawBitmap; }

// Operations
public:

    virtual void Draw(CDC& pDC, TileScale eScale) override;
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt) override;
#ifndef GPLAY
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CBrdEditView& pView) override;
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(const CRect& pRctZone) override;
#else
    virtual void Rotate(Rotation90 rot) override;
#endif
    // ------- //
    virtual OwnerPtr Clone() const override;

    void CopyAttributes(const CBitmapImage& source);

    virtual void Serialize(CArchive& ar) override;

protected:
    void SynchronizeExtentRect(CSize sizeWorld, CSize sizeView);
};

///////////////////////////////////////////////////////////////////////

class CTileImage : public CDrawObj_SimplRctExtent
{
// Constructors
public:
    CTileImage() : m_tid(nullTid) { m_pTMgr = NULL; }
    CTileImage(CTileManager* pTMgr) : m_tid(nullTid) { m_pTMgr = pTMgr; }
// Attributes
public:
    TileID          m_tid;
    CB::propagate_const<CTileManager*>   m_pTMgr;

    void SetTile(int x, int y, TileID tid);
    virtual enum CDrawObjType GetType() const override { return drawTile; }
// Operations
public:
    virtual void Draw(CDC& pDC, TileScale eScale) override;
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt) override;
#ifndef GPLAY
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CBrdEditView& pView) override;
#endif
    // ------- //
#ifndef GPLAY
    virtual void ForceIntoZone(const CRect& pRctZone) override;
#else
    virtual void Rotate(Rotation90 rot) override;
#endif
    // ------- //
    virtual OwnerPtr Clone() const override;

    void CopyAttributes(const CTileImage& source);

    virtual void Serialize(CArchive& ar) override;
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

    CRect GetRect() const override;
    void SetRect(const CRect& rct) override;

// Attributes
public:
    PieceID         m_pid;
    CGamDoc*        m_pDoc;


    void SetPiece(const CRect& rct, PieceID pid);

    BOOL IsOwned() const;
    BOOL IsOwnedBy(DWORD dwMask) const;
    BOOL IsOwnedButNotByCurrentPlayer() const;
    void SetOwnerMask(DWORD dwMask);

    virtual enum CDrawObjType GetType() const override { return drawPieceObj; }
    virtual ObjectID GetObjectID() const override { return static_cast<ObjectID>(m_pid); }

// Operations
private:
    CSize GetSize() const;

public:
    virtual void Draw(CDC& pDC, TileScale eScale) override;
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt) override;
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CPlayBoardView& pView) override;
    // ------- //
    virtual OwnerPtr Clone(CGamDoc* pDoc) const override;
    virtual BOOL Compare(const CDrawObj& pObj) const override;
    virtual void Serialize(CArchive& ar) override;

private:
    CPoint m_center;
};

///////////////////////////////////////////////////////////////////////

class CLineObj : public CLine
{
// Constructors
public:
    CLineObj() {}
// Other...
protected:
    virtual enum CDrawObjType GetType() const override { return drawLineObj; }

    ObjectID    m_dwObjectID;
public:
    virtual ObjectID GetObjectID() const override { return m_dwObjectID; }
    void    SetObjectID(ObjectID dwID) { m_dwObjectID = dwID; }
    // ------ //
    virtual OwnerPtr Clone(CGamDoc* pDoc) const override;
    virtual BOOL Compare(const CDrawObj& pObj) const override;
    virtual void Serialize(CArchive& ar) override;
};

///////////////////////////////////////////////////////////////////////

class CMarkObj : public CDrawObj_SimplRctExtent
{
// Constructors
public:
    CMarkObj() { m_mid = nullMid; m_pDoc = NULL; m_nFacingDegCW = uint16_t(0); }
    CMarkObj(CGamDoc* pDoc)
        { m_mid = nullMid; m_pDoc = pDoc; m_nFacingDegCW = uint16_t(0); }

// Attributes
public:
    MarkID          m_mid;
    CGamDoc*        m_pDoc;

    void SetMark(CRect& rct, MarkID mid);

    void SetFacing(uint16_t nFacingDegCW) { m_nFacingDegCW = nFacingDegCW; }
    uint16_t GetFacing() { return m_nFacingDegCW; }

    void    SetObjectID(ObjectID dwID) { m_dwObjectID = dwID; }
    virtual ObjectID GetObjectID() const override { return m_dwObjectID; }

    void ResyncExtentRect();

    virtual enum CDrawObjType GetType() const override { return drawMarkObj; }
protected:
    ObjectID m_dwObjectID;
    uint16_t m_nFacingDegCW;           // Rotation of marker (degrees)

// Operations
public:
    TileID GetCurrentTileID();
    // ------- //
    virtual void Draw(CDC& pDC, TileScale eScale) override;
    // Support required by selection processing.
    virtual BOOL HitTest(CPoint pt) override;
    virtual ::OwnerPtr<CSelection> CreateSelectProxy(CPlayBoardView& pView) override;
    // ------- //
    virtual OwnerPtr Clone(CGamDoc* pDoc) const override;
    virtual BOOL Compare(const CDrawObj& pObj) const override;
    virtual void Serialize(CArchive& ar) override;
};

#endif  // GPLAY

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// The DrawList manager class

/* WARNING:  Ownership of objects in CDrawList is hard to
    determine.  CB 3.1 code deletes objects in this list when
    list is destroyed, which implies list owns its content, but
    RemoveObject* doesn't delete object, so in that case there
    must be another owner somewhere.  CB 3.5 doesn't do anything
    to clarify this. */
class CDrawList : private std::list<CDrawObj::OwnerPtr>
{
    using BASE = std::list<CDrawObj::OwnerPtr>;
    friend class CGamDoc;
public:
    /* N.B.:  as in CB 3.1, begin/end are in terms of underlying
                list, but Front/Back are in terms of display,
                which is in opposite order of
                begin/end/front/back */
    using BASE::const_iterator;
    using BASE::iterator;
    using BASE::begin;
    using BASE::clear;
    using BASE::end;

    CDrawList() = default;
    CDrawList(const CDrawList&) = delete;
    CDrawList& operator=(const CDrawList&) = delete;
    CDrawList(CDrawList&&) = default;
    CDrawList& operator=(CDrawList&&) = default;
    ~CDrawList() = default;
public:
    const_iterator Find(const CDrawObj& drawObj) const;
    iterator Find(const CDrawObj& drawObj);
    // NOTE:  See WARNING: above
    void RemoveObject(const CDrawObj& pDrawObj);
    void RemoveObjectsInList(const std::vector<CB::not_null<CDrawObj*>>& pLst);
    void AddToBack(CDrawObj::OwnerPtr pDrawObj) { push_front(std::move(pDrawObj)); }
    void AddToFront(CDrawObj::OwnerPtr pDrawObj) { push_back(std::move(pDrawObj)); }
    CDrawObj& Front() { return *back(); }
    CDrawObj& Back() { return *front(); }
    void Draw(CDC& pDC, const CRect& pDrawRct, TileScale eScale,
        BOOL bApplyVisibility = TRUE, BOOL bDrawPass2Objects = FALSE,
        BOOL bHideUnlocked = FALSE, BOOL bDrawLockedFirst = FALSE);
    CDrawObj* HitTest(CPoint pt, TileScale eScale = (TileScale)AllTileScales,
        BOOL bApplyVisibility = TRUE);
    void DrillDownHitTest(CPoint point, std::vector<CB::not_null<CDrawObj*>>& selLst,
        TileScale eScale = (TileScale)AllTileScales, BOOL bApplyVisibility = TRUE);
#ifdef GPLAY
    void ArrangePieceTableInDrawOrder(std::vector<PieceID>& pTbl) const;
    void ArrangePieceTableInVisualOrder(std::vector<PieceID>& pTbl) const;
#endif
    void ArrangeObjectListInDrawOrder(std::vector<CB::not_null<CDrawObj*>>& pLst);
    void ArrangeObjectListInVisualOrder(std::vector<CB::not_null<CDrawObj*>>& pLst);
    void ArrangeObjectPtrTableInDrawOrder(std::vector<CB::not_null<CDrawObj*>>& pTbl) const;
    void ArrangeObjectPtrTableInVisualOrder(std::vector<CB::not_null<CDrawObj*>>& pTbl) const;
#ifdef GPLAY
    void SetOwnerMasks(DWORD dwOwnerMask);

    const CPieceObj* FindPieceID(PieceID pid) const;
    CPieceObj* FindPieceID(PieceID pid)
    {
        return const_cast<CPieceObj*>(std::as_const(*this).FindPieceID(pid));
    }
    const CDrawObj* FindObjectID(ObjectID oid) const;
    CDrawObj* FindObjectID(ObjectID oid)
    {
        return const_cast<CDrawObj*>(std::as_const(*this).FindObjectID(oid));
    }
    BOOL HasObject(const CDrawObj& pObj) const { return Find(pObj) != end(); }
    BOOL HasMarker() const;
    void GetPieceObjectPtrList(std::vector<CB::not_null<CPieceObj*>>& pLst);
    void GetPieceIDTable(std::vector<PieceID>& pTbl) const;
    void GetObjectListFromPieceIDTable(const std::vector<PieceID>& pTbl, std::vector<CB::not_null<CPieceObj*>>& pLst);
    static void GetPieceIDTableFromObjList(const std::vector<CB::not_null<CDrawObj*>>& pLst, std::vector<PieceID>& pTbl,
        BOOL bDeleteObjs = FALSE);
    // ------- //
    CDrawList Clone(CGamDoc* pDoc) const;
    void Restore(CGamDoc* pDoc, const CDrawList& pLst);
    BOOL Compare(const CDrawList& pLst) const;
    void AppendWithRotOffset(const CDrawList& pSourceLst, Rotation90 rot, CPoint pntOffet);
#else
    BOOL PurgeMissingTileIDs(CTileManager* pTMgr);
    BOOL IsTileInUse(TileID tid) const;
    // ------- //
    void ForceIntoZone(const CRect& pRctZone);
#endif
    // -------- //
    void Serialize(CArchive& ar);
};

#endif

