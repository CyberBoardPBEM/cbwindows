// DrawObj.cpp
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

#include    "stdafx.h"
#include    <limits.h>
#ifdef      GPLAY
    #include    "Gp.h"
    #include    "GamDoc.h"
    #include    "GameBox.h"
    #include    "PPieces.h"
    #include    "Marks.h"
#else
    #include    "Gm.h"
    #include    "GmDoc.h"
#endif
#include    "DrawObj.h"
#ifdef      GPLAY
#include    "SelOPlay.h"
#else
#include    "SelObjs.h"
#endif
#include    "Tile.h"
#include    "GdiTools.h"
#include    "GMisc.h"
#include    "CDib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#if defined(GPLAY)
static_assert(sizeof(ObjectID32) == sizeof(uint32_t), "size error");
static_assert(sizeof(ObjectID32) == sizeof(DWORD), "size error");
static_assert(alignof(ObjectID32) == alignof(uint32_t), "align error");
static_assert(sizeof(ObjectID64) == sizeof(uint64_t), "size error");
static_assert(alignof(ObjectID64) == alignof(uint64_t), "align error");
namespace {
    class ObjectIDCheck
    {
    public:
        ObjectIDCheck()
        {
            {
                ObjectID32 test(4, 3, CDrawObj::drawMarkObj);
                ASSERT(reinterpret_cast<uint32_t&>(test) == 0x20030004 || !"non-Microsoft field layout");
            }
            {
                ObjectID32 test(PieceID16(0x1234));
                ASSERT(reinterpret_cast<uint32_t&>(test) == 0x00001234 || !"non-Microsoft field layout");
            }
            {
                ObjectID64 test(4, 3, CDrawObj::drawMarkObj);
                ASSERT(reinterpret_cast<uint64_t&>(test) == 0x2000000000030004 || !"non-Microsoft field layout");
            }
            {
                ObjectID64 test(PieceID32(0x12345678));
                ASSERT(reinterpret_cast<uint64_t&>(test) == 0x0000000012345678 || !"non-Microsoft field layout");
            }
        }
    } objectIDCheck;
}

ObjectID32::ObjectID32(uint16_t i, uint16_t s, CDrawObj::CDrawObjType t) :
    u(i, s, t)
{
    ASSERT((t & 0xFFF0) == 0x0080 || !"unexpected CDrawObjType value");
    ASSERT((t & 0x000F) < 0x000E || !"conflict with GameElement marker tag");
    static_assert(sizeof(int32_t) == sizeof(*this), "need to adjust cast");
    ASSERT(reinterpret_cast<int32_t&>(*this) != INT32_C(-1));
}

ObjectID32::ObjectID32(PieceID16 pid) :
    u(pid)
{
    static_assert(sizeof(int32_t) == sizeof(*this), "need to adjust cast");
    ASSERT(reinterpret_cast<int32_t&>(*this) != INT32_C(-1));
}

#if !defined(NDEBUG)
ObjectID32::ObjectID32(uint32_t dw)
{
    reinterpret_cast<uint32_t&>(*this) = dw;
    static_assert(sizeof(int) == sizeof(*this), "need to adjust cast");
    ASSERT(reinterpret_cast<int&>(*this) != INT32_C(-1));
}
#endif

ObjectID64::ObjectID64(uint16_t i, uint16_t s, CDrawObj::CDrawObjType t) :
    u(i, s, t)
{
    ASSERT((t & 0xFFF0) == 0x0080 || !"unexpected CDrawObjType value");
    ASSERT((t & 0x000F) < 0x000E || !"conflict with GameElement marker tag");
    static_assert(sizeof(int64_t) == sizeof(*this), "need to adjust cast");
    ASSERT(reinterpret_cast<int64_t&>(*this) != INT64_C(-1));
}

ObjectID64::ObjectID64(PieceID32 pid) :
    u(pid)
{
    static_assert(sizeof(int64_t) == sizeof(*this), "need to adjust cast");
    ASSERT(reinterpret_cast<int64_t&>(*this) != INT64_C(-1));
}

#endif

////////////////////////////////////////////////////////////////////
// Class variables

CPen*   CDrawObj::c_pPrvPen = NULL;
CBrush* CDrawObj::c_pPrvBrush = NULL;
BOOL    CDrawObj::c_bHitTestDraw = FALSE;

//////////////////////////////////////////////////////////////////
// Class CDrawObj

CDrawObj::CDrawObj()
{
    m_dwDObjFlags = 0;
    SetScaleVisibility(AllTileScales);
}

BOOL CDrawObj::IsVisible(const RECT& pClipRct) const
{
    CRect rct = GetEnclosingRect();
    return rct.IntersectRect(&rct, &pClipRct);
}

#ifdef GPLAY
ObjectID CDrawObj::GetObjectID() const
{
    return ObjectID();
}

void CDrawObj::MoveObject(CPoint ptUpLeft)
{
    CRect temp = GetRect();
    temp += CPoint(ptUpLeft.x - temp.left,
        ptUpLeft.y - temp.top);
    SetRect(temp);
}

void CDrawObj::Rotate(Rotation90 rot)
{
    CRect temp = GetRect();
    switch (rot)
    {
        case Rotation90::r0:
            ASSERT(!"no-op call");
            break;
        case Rotation90::r90:
            SetRect(CRect(-temp.bottom, temp.left,
                                -temp.top, temp.right));
            break;
        case Rotation90::r180:
            SetRect(CRect(-temp.right, -temp.bottom,
                                -temp.left, -temp.top));
            break;
        case Rotation90::r270:
            SetRect(CRect(temp.top, -temp.right,
                                temp.bottom, -temp.left));
            break;
        default:
            AfxThrowInvalidArgException();
    }
}
#endif

//DFM991129
void CDrawObj::OffsetObject(CPoint offset)
{
    CRect temp = GetRect();
    temp += offset;
    SetRect(temp);
}
//DFM991129

BOOL CDrawObj::IsExtentOutOfZone(const CRect& pRctZone, CPoint& pnt) const
{
    CRect rctExtent = GetRect();
    CRect rct;
    rct.UnionRect(pRctZone, rctExtent);
    if (rct != *pRctZone)
    {
        pnt.x = pnt.y = 0;
        // Calc offset needed to push the rect onto the pRctZone
        if (rctExtent.right > pRctZone.right)
            pnt.x = pRctZone.right - rctExtent.right;
        if (rctExtent.bottom > pRctZone.bottom)
            pnt.y = pRctZone.bottom - rctExtent.bottom;
        return TRUE;
    }
    return FALSE;
}

// Sets up pen and brush for drawing. The previous pen
// and brush are saved in class variables.

void CDrawObj::SetUpDraw(CDC& pDC, CPen& pPen, CBrush& pBrush) const
{
    if (c_bHitTestDraw)
        pPen.CreateStockObject(BLACK_PEN);
    else
    {
        if (GetLineColor() != noColor)
        {
            pPen.CreatePen(PS_SOLID, GetLineWidth(), GetLineColor());
        }
        else
            pPen.CreateStockObject(NULL_PEN);
    }

    if (GetFillColor() != noColor)
    {
        if (c_bHitTestDraw)
            pBrush.CreateStockObject(BLACK_BRUSH);
        else
            pBrush.CreateSolidBrush(GetFillColor());
    }
    else
        pBrush.CreateStockObject(NULL_BRUSH);

    c_pPrvPen = pDC.SelectObject(&pPen);
    c_pPrvBrush = pDC.SelectObject(&pBrush);
}

void CDrawObj::CleanUpDraw(CDC& pDC) const
{
    pDC.SelectObject(c_pPrvPen);
    pDC.SelectObject(c_pPrvBrush);
}

const int hitZone = 5;          // Must be odd and <= 15

static WORD bmapSeed[hitZone] =
    { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

BOOL CDrawObj::BitBlockHitTest(CPoint pt)
{
    CDC dcmem;
    CBitmap bmap;

    dcmem.CreateCompatibleDC(NULL);
    bmap.CreateBitmap(8, 8, 1, 1, NULL);

    CBitmap* pPrvBMap = dcmem.SelectObject(&bmap);
    dcmem.PatBlt(0, 0, 8, 8, WHITENESS);

    dcmem.SetViewportOrg(-(pt.x - hitZone/2), -(pt.y - hitZone/2));

    c_bHitTestDraw = TRUE;
    Draw(dcmem, fullScale);
    c_bHitTestDraw = FALSE;

    BOOL bHit = FALSE;
    dcmem.SetViewportOrg(0, 0);

    for (int x = 0; x < hitZone; x++)
    {
        for (int y = 0; y < hitZone; y++)
        {
            if (dcmem.GetPixel(x, y) == RGB(0,0,0))
            {
                bHit = TRUE;
                goto SCANDONE;
            }
        }
    }
SCANDONE:
    dcmem.SelectObject(pPrvBMap);
    return bHit;
}

void CDrawObj::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << m_dwDObjFlags;            // File Ver 2.0
        CRect temp = GetRect();
        ar << (short)temp.left;
        ar << (short)temp.top;
        ar << (short)temp.right;
        ar << (short)temp.bottom;
    }
    else
    {
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))
            ar >> m_dwDObjFlags;
        else
        {
            WORD wTmp;
            ar >> wTmp;
            m_dwDObjFlags = wTmp;
        }
        short sTmp;
        CRect temp;
        ar >> (short)sTmp; temp.left = sTmp;
        ar >> (short)sTmp; temp.top = sTmp;
        ar >> (short)sTmp; temp.right = sTmp;
        ar >> (short)sTmp; temp.bottom = sTmp;
        SetRect(temp);
    }
}

//DFM19991213
void CDrawObj::CopyAttributes(const CDrawObj& source)
{
    m_dwDObjFlags = source.m_dwDObjFlags;
}
//DFM19991213

#ifdef GPLAY
void ObjectID32::Serialize(CArchive& ar) const
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    switch (u.tag.subtype)
    {
        case stPieceObj:
        case stMarkObj:
        case stMarkerID:
            break;
        case stLineObj:
            ASSERT(!"future feature");
            break;
        default:
            CbThrowBadCastException();
    }

    switch (GetXxxxIDSerializeSize<PieceID>(ar))
    {
        case 2:
            ar << u.buf;
            break;
        case 4:
            ASSERT(!"will probably never be used");
            ar << static_cast<ObjectID64>(*this);
            break;
        default:
            CbThrowBadCastException();
    }
}

void ObjectID32::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }

    switch (GetXxxxIDSerializeSize<PieceID>(ar))
    {
        case 2:
            ar >> u.buf;
            // some old files used different object ID tag values
            if (CB::GetVersion(ar) <= NumVersion(3, 90))
            {
                if (u.tag.subtype == stInvalid)
                {
                    u.tag.subtype = stMarkObj;
                }
            }
            break;
        case 4: {
            ASSERT(!"probably should never be used");
            SerializeBackdoor sb;
            ObjectID64 temp;
            ar >> temp;
            *this = SerializeBackdoorObjectID::Convert(temp);
            break;
        }
        default:
            CbThrowBadCastException();
    }
    switch (u.tag.subtype)
    {
        case stPieceObj:
        case stMarkObj:
        case stMarkerID:
            break;
        case stLineObj:
            ASSERT(!"future feature");
            break;
        default:
            CbThrowBadCastException();
    }
}

ObjectID32::operator ObjectID64() const
{
    switch (u.tag.subtype)
    {
        case stPieceObj:
            return ObjectID64(SerializeBackdoor::Convert(u.pieceObj.pid));
        case stInvalid:
            return ObjectID64();
        case stMarkObj:
            return ObjectID64(u.markObj.id, u.markObj.serial, CDrawObj::drawMarkObj);
        case stLineObj:
            ASSERT(!"future feature");
            // fall through
        default:
            CbThrowBadCastException();
    }
}

void ObjectID64::Serialize(CArchive& ar) const
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    switch (u.tag.subtype)
    {
        case stPieceObj:
        case stMarkObj:
        case stMarkerID:
            break;
        case stLineObj:
            ASSERT(!"future feature");
            break;
        default:
            CbThrowBadCastException();
    }

    switch (GetXxxxIDSerializeSize<PieceID>(ar))
    {
        case 2:
            ar << static_cast<ObjectID32>(*this);
            break;
        case 4:
            ar << u.buf;
            break;
        default:
            CbThrowBadCastException();
    }
}

void ObjectID64::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }

    switch (GetXxxxIDSerializeSize<PieceID>(ar))
    {
        case 2: {
            SerializeBackdoor sb;
            ObjectID32 temp;
            ar >> temp;
            *this = SerializeBackdoorObjectID::Convert(temp);
            break;
        }
        case 4:
            ar >> u.buf;
            break;
        default:
            CbThrowBadCastException();
    }
    switch (u.tag.subtype)
    {
        case stPieceObj:
        case stMarkObj:
        case stMarkerID:
            break;
        case stLineObj:
            ASSERT(!"future feature");
            break;
        default:
            CbThrowBadCastException();
    }
}

ObjectID64::operator ObjectID32() const
{
    switch (u.tag.subtype)
    {
        case stPieceObj:
            return ObjectID32(SerializeBackdoor::Convert(u.pieceObj.pid));
        case stInvalid:
            return ObjectID32();
        case stMarkObj:
            return ObjectID32(u.markObj.id, u.markObj.serial, CDrawObj::drawMarkObj);
        case stLineObj:
            ASSERT(!"future feature");
            // fall through
        default:
            CbThrowBadCastException();
    }
}

ObjectID64 SerializeBackdoorObjectID::Convert(const ObjectID32& oid)
{
    if (!Depth())
    {
        ASSERT(!"only for serialize use");
        AfxThrowNotSupportedException();
    }
    return static_cast<ObjectID64>(oid);
}

ObjectID32 SerializeBackdoorObjectID::Convert(const ObjectID64& oid)
{
    if (!Depth())
    {
        ASSERT(!"only for serialize use");
        AfxThrowNotSupportedException();
    }
    return static_cast<ObjectID32>(oid);
}
#endif

void CDrawObj_SimplRctExtent::CopyAttributes(const CDrawObj_SimplRctExtent& source)
{
    CDrawObj::CopyAttributes(source);
    m_rctExtent = source.m_rctExtent;
}

////////////////////////////////////////////////////////////////////
// CRectObject methods

void CRectObj::Draw(CDC& pDC, TileScale)
{
    CPen   penEdge;
    CBrush brushFill;

    SetUpDraw(pDC, penEdge, brushFill);
    pDC.Rectangle(&m_rctExtent);
    CleanUpDraw(pDC);
}

CRect CRectObj::GetEnclosingRect() const
{
    CRect rct = m_rctExtent;
    rct.InflateRect(m_nLineWidth / 2 + 1, m_nLineWidth / 2 + 1);
    return rct;
}

BOOL CRectObj::HitTest(CPoint pt)
{
    // First check if anywhere near the object.
    CRect rct = GetEnclosingRect();
    if (!rct.PtInRect(pt))
        return FALSE;

    // Now check for actual image hit. (!!!For rects this could actually
    // be qualified without using the bit block method!!!)
    return BitBlockHitTest(pt);
}

#ifndef     GPLAY
void CRectObj::ForceIntoZone(const CRect& pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
        m_rctExtent += pntOffset;
}

OwnerPtr<CSelection> CRectObj::CreateSelectProxy(CBrdEditView& pView)
{
    return MakeOwner<CSelRect>(pView, *this);
}
#endif

//DFM19991210
CDrawObj::OwnerPtr CRectObj::Clone() const
{
    ::OwnerPtr<CRectObj> returnValue = MakeOwner<CRectObj>();

    returnValue->CopyAttributes(*this);

    return (returnValue);
}
//DFM19991210

void CRectObj::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (DWORD)m_crFill;
        ar << (DWORD)m_crLine;
        ar << (WORD)m_nLineWidth;
    }
    else
    {
        WORD wTmp;
        DWORD dwTmp;
        ar >> dwTmp; m_crFill = (COLORREF)dwTmp;
        ar >> dwTmp; m_crLine = (COLORREF)dwTmp;
        ar >> wTmp;  m_nLineWidth = (UINT)wTmp;
    }
}

//DFM19991213
void CRectObj::CopyAttributes (const CRectObj& source)
{
    CDrawObj_SimplRctExtent::CopyAttributes (source);

    m_crFill     = source.m_crFill;
    m_crLine     = source.m_crLine;
    m_nLineWidth = source.m_nLineWidth;
}
//DFM19991213

////////////////////////////////////////////////////////////////////
// CEllipse methods

void CEllipse::Draw(CDC& pDC, TileScale)
{
    CPen   penEdge;
    CBrush brushFill;

    SetUpDraw(pDC, penEdge, brushFill);
    pDC.Ellipse(&m_rctExtent);
    CleanUpDraw(pDC);
}

#ifndef     GPLAY
OwnerPtr<CSelection> CEllipse::CreateSelectProxy(CBrdEditView& pView)
{
    return MakeOwner<CSelEllipse>(pView, *this);
}
#endif

//DFM19991213
CDrawObj::OwnerPtr CEllipse::Clone() const
{
    ::OwnerPtr<CEllipse> returnValue = MakeOwner<CEllipse>();

    returnValue->CRectObj::CopyAttributes(*this);

    return (returnValue);
}
//DFM19991213

////////////////////////////////////////////////////////////////////
// CPolyObj methods

void CPolyObj::Draw(CDC& pDC, TileScale)
{
    if (m_Pnts.empty())
        return;
    CPen   penEdge;
    CBrush brushFill;
    SetUpDraw(pDC, penEdge, brushFill);

    if (m_crFill == noColor)
        pDC.Polyline(m_Pnts.data(), value_preserving_cast<int>(m_Pnts.size()));
    else
        pDC.Polygon(m_Pnts.data(), value_preserving_cast<int>(m_Pnts.size()));

    CleanUpDraw(pDC);
}

void CPolyObj::AddPoint(CPoint pnt)
{
    m_Pnts.push_back(pnt);
    ComputeExtent();
}

void CPolyObj::SetNewPolygon(const std::vector<POINT>& pnts)
{
    m_Pnts = pnts;
    ComputeExtent();
}

void CPolyObj::ComputeExtent()
{
    m_rctExtent.SetRectEmpty();
    if (m_Pnts.empty())
        return;
    int xmin = INT_MAX, xmax = INT_MIN, ymin = INT_MAX, ymax = INT_MIN;
    for (size_t i = size_t(0) ; i < m_Pnts.size() ; ++i)
    {
        xmin = CB::min(xmin, m_Pnts[i].x);
        xmax = CB::max(xmax, m_Pnts[i].x);
        ymin = CB::min(ymin, m_Pnts[i].y);
        ymax = CB::max(ymax, m_Pnts[i].y);
    }
    m_rctExtent.SetRect(xmin, ymin, xmax, ymax);
}

CRect CPolyObj::GetEnclosingRect() const
{
    CRect rct = m_rctExtent;
    rct.InflateRect(m_nLineWidth + 1, m_nLineWidth + 1);
    return rct;
}

BOOL CPolyObj::HitTest(CPoint pt)
{
    // First check if anywhere near the object.
    CRect rct = GetEnclosingRect();
    if (!rct.PtInRect(pt))
        return FALSE;

    // Now check for actual image hit.
    return BitBlockHitTest(pt);
}

#ifndef     GPLAY
void CPolyObj::ForceIntoZone(const CRect& pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
    {
        for (size_t i = size_t(0) ; i < m_Pnts.size() ; ++i)
        {
            m_Pnts[i].x += pntOffset.x;
            m_Pnts[i].y += pntOffset.y;
        }
        ComputeExtent();
    }
}
#endif

#ifndef     GPLAY
OwnerPtr<CSelection> CPolyObj::CreateSelectProxy(CBrdEditView& pView)
{
    return MakeOwner<CSelPoly>(pView, *this);
}
#else
void CPolyObj::Rotate(Rotation90 rot)
{
    //  let parent handle its part
    CDrawObj::Rotate(rot);

    switch (rot)
    {
        case Rotation90::r0:
            ASSERT(!"no-op call");
            break;
        case Rotation90::r90:
            for (POINT& pnt : m_Pnts)
            {
                long temp = pnt.y;
                pnt.y = pnt.x;
                pnt.x = -temp;
            }
            break;
        case Rotation90::r180:
            for (POINT& pnt : m_Pnts)
            {
                pnt.x = -pnt.x;
                pnt.y = -pnt.y;
            }
            break;
        case Rotation90::r270:
            for (POINT& pnt : m_Pnts)
            {
                long temp = pnt.y;
                pnt.y = -pnt.x;
                pnt.x = temp;
            }
            break;
        default:
            AfxThrowInvalidArgException();
    }
}
#endif

void CPolyObj::OffsetObject (CPoint offset)
{
    //  let parent handle its part
    CDrawObj::OffsetObject (offset);

    //  now we handle our part
    for (size_t i = size_t(0) ; i < m_Pnts.size() ; ++i)
    {
        m_Pnts[i].x += offset.x;
        m_Pnts[i].y += offset.y;
    }
}

CDrawObj::OwnerPtr CPolyObj::Clone() const
{
    ::OwnerPtr<CPolyObj> pObj = MakeOwner<CPolyObj>();
    pObj->CopyAttributes(*this);
    return pObj;
}

void CPolyObj::CopyAttributes(const CPolyObj& source)
{
    CDrawObj_SimplRctExtent::CopyAttributes(source);

    m_crFill     = source.m_crFill;
    m_crLine     = source.m_crLine;
    m_nLineWidth = source.m_nLineWidth;
    m_Pnts       = source.m_Pnts;
}

void CPolyObj::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (DWORD)m_crFill;
        ar << (DWORD)m_crLine;
        ar << (WORD)m_nLineWidth;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<WORD>(m_Pnts.size());
        }
        else
        {
            CB::WriteCount(ar, m_Pnts.size());
        }
        if (!m_Pnts.empty())
            WriteArchivePoints(ar, m_Pnts.data(), m_Pnts.size());
    }
    else
    {
        WORD wTmp;
        DWORD dwTmp;
        ar >> dwTmp; m_crFill = (COLORREF)dwTmp;
        ar >> dwTmp; m_crLine = (COLORREF)dwTmp;
        ar >> wTmp;  m_nLineWidth = (UINT)wTmp;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar >> wTmp; m_Pnts.resize(wTmp);
        }
        else
        {
            m_Pnts.resize(CB::ReadCount(ar));
        }
        if (!m_Pnts.empty())
        {
            ReadArchivePoints(ar, m_Pnts.data(), m_Pnts.size());
        }
    }
}

//////////////////////////////////////////////////////////////////
// Class CLine

void CLine::Draw(CDC& pDC, TileScale)
{
    CPen pen;
    if (c_bHitTestDraw)
        pen.CreateStockObject(BLACK_PEN);
    else
        pen.CreatePen(PS_SOLID, m_nLineWidth, m_crLine);
    CPen* pPrvPen = pDC.SelectObject(&pen);

    pDC.MoveTo(m_ptBeg);
    pDC.LineTo(m_ptEnd);

    pDC.SelectObject(pPrvPen);
}

void CLine::SetLine(int xBeg, int yBeg, int xEnd, int yEnd)
{
    m_ptBeg.x = xBeg;
    m_ptBeg.y = yBeg;
    m_ptEnd.x = xEnd;
    m_ptEnd.y = yEnd;
    // An enclosing rectangle *must* have some volume...
    int nWidth = m_nLineWidth / 2 ? m_nLineWidth / 2 : 1;
    m_rctExtent.left = CB::min(xBeg, xEnd) - nWidth;
    m_rctExtent.top = CB::min(yBeg, yEnd) - nWidth;
    m_rctExtent.right = CB::max(xBeg, xEnd) + nWidth;
    m_rctExtent.bottom = CB::max(yBeg, yEnd) +nWidth;
}

CRect CLine::GetEnclosingRect() const
{
    CRect rct = m_rctExtent;
    rct.NormalizeRect();
    return rct;
}

BOOL CLine::HitTest(CPoint pt)
{
    // First check if anywhere near the object.
    CRect rct = GetEnclosingRect();
    if (!rct.PtInRect(pt))
        return FALSE;

    // Now check for actual image hit.
    return BitBlockHitTest(pt);
}

#ifndef     GPLAY
void CLine::ForceIntoZone(const CRect& pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
    {
        SetLine(m_ptBeg.x + pntOffset.x, m_ptBeg.y + pntOffset.y,
            m_ptEnd.x + pntOffset.x, m_ptEnd.y + pntOffset.y);
    }
}
#endif

#ifdef GPLAY
OwnerPtr<CSelection> CLine::CreateSelectProxy(CPlayBoardView& pView)
#else
OwnerPtr<CSelection> CLine::CreateSelectProxy(CBrdEditView& pView)
#endif
{
    return MakeOwner<CSelLine>(pView, *this);
}

#ifdef GPLAY
void CLine::Rotate(Rotation90 rot)
{
    //  let parent handle its part
    CDrawObj::Rotate(rot);

    switch (rot)
    {
        case Rotation90::r0:
            ASSERT(!"no-op call");
            break;
        case Rotation90::r90:
        {
            long temp = m_ptBeg.y;
            m_ptBeg.y = m_ptBeg.x;
            m_ptBeg.x = -temp;
            temp = m_ptEnd.y;
            m_ptEnd.y = m_ptEnd.x;
            m_ptEnd.x = -temp;
            break;
        }
        case Rotation90::r180:
            m_ptBeg.x = -m_ptBeg.x;
            m_ptBeg.y = -m_ptBeg.y;
            m_ptEnd.x = -m_ptEnd.x;
            m_ptEnd.y = -m_ptEnd.y;
            break;
        case Rotation90::r270:
        {
            long temp = m_ptBeg.y;
            m_ptBeg.y = -m_ptBeg.x;
            m_ptBeg.x = temp;
            temp = m_ptEnd.y;
            m_ptEnd.y = -m_ptEnd.x;
            m_ptEnd.x = temp;
            break;
        }
        default:
            AfxThrowInvalidArgException();
    }
}
#endif

//DFM19991221
void CLine::OffsetObject(CPoint offset)
{
    // let parent handle its part
    CDrawObj::OffsetObject (offset);

    // now we'll handle our part
    m_ptBeg.Offset (offset.x, offset.y);
    m_ptEnd.Offset (offset.x, offset.y);

}

CDrawObj::OwnerPtr CLine::Clone() const
{
    ::OwnerPtr<CLine> pObj = MakeOwner<CLine>();
    pObj->CopyAttributes(*this);
    return pObj;
}
//DFM19991213

#ifdef GPLAY
CDrawObj::OwnerPtr CLine::Clone(CGamDoc* pDoc) const
{
    ::OwnerPtr<CLine> pObj = MakeOwner<CLine>();
    pObj->CopyAttributes(*this);    //DFM19991214
    return pObj;
}

BOOL CLine::Compare(const CDrawObj& pObj) const
{
    if (pObj.GetType() != GetType())
    {
        AfxThrowInvalidArgException();
    }
    const CLine& pLine = static_cast<const CLine&>(pObj);
    if (m_ptBeg != pLine.m_ptBeg)
        return FALSE;
    if (m_ptEnd != pLine.m_ptEnd)
        return FALSE;
    return TRUE;
}
#endif      // GPLAY

//DFM19991214
void CLine::CopyAttributes(const CLine& source)
{
    CDrawObj_SimplRctExtent::CopyAttributes(source);

    m_ptBeg = source.m_ptBeg;
    m_ptEnd = source.m_ptEnd;

    m_crLine = source.m_crLine;
    m_nLineWidth = source.m_nLineWidth;
}
//DFM19991214

void CLine::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (short)m_ptBeg.x;
        ar << (short)m_ptBeg.y;
        ar << (short)m_ptEnd.x;
        ar << (short)m_ptEnd.y;

        ar << (DWORD)m_crLine;
        ar << (WORD)m_nLineWidth;
    }
    else
    {
        WORD wTmp;
        DWORD dwTmp;
        short sTmp;
        ar >> sTmp; m_ptBeg.x = sTmp;
        ar >> sTmp; m_ptBeg.y = sTmp;
        ar >> sTmp; m_ptEnd.x = sTmp;
        ar >> sTmp; m_ptEnd.y = sTmp;

        ar >> dwTmp; m_crLine = (COLORREF)dwTmp;
        ar >> wTmp;  m_nLineWidth = (UINT)wTmp;
        // Make sure extent rect is properly set.
        SetLine(m_ptBeg.x, m_ptBeg.y, m_ptEnd.x, m_ptEnd.y);
    }
}

//////////////////////////////////////////////////////////////////
// Class CBitmapImage

void CBitmapImage::Draw(CDC& pDC, TileScale eScale)
{
    g_gt.mTileDC.SelectObject(&m_bitmap);
    SetupPalette(g_gt.mTileDC);

    CPoint pnt = m_rctExtent.TopLeft();

    if ((GetDObjFlags() & dobjFlgLayerNatural) == 0)
    {
        pDC.SetStretchBltMode(COLORONCOLOR);
        pDC.StretchBlt(pnt.x, pnt.y, m_rctExtent.Width(),  m_rctExtent.Height(), &g_gt.mTileDC,
            0, 0, m_rctExtent.Width(), m_rctExtent.Height(), SRCCOPY);

        g_gt.SelectSafeObjectsForTileDC();
    }
    else
    {
        ASSERT(GetScaleVisibility() ==  fullScale || // there must only be one scale set
            GetScaleVisibility() == halfScale || GetScaleVisibility() == smallScale);

        // Only draw the bitmap at the proper board scaling.
        if (GetScaleVisibility() != eScale)
            return;
        BITMAP bmInfo;
        m_bitmap.GetObject(sizeof(bmInfo), &bmInfo);

        CSize sizeWorld = pDC.GetWindowExt();
        CSize sizeView = pDC.GetViewportExt();
        CPoint pnt = m_rctExtent.TopLeft();
        ScalePoint(pnt, sizeView, sizeWorld);

        SynchronizeExtentRect(sizeWorld, sizeView); // Only time we can find true scale

        pDC.SaveDC();
        pDC.SetMapMode(MM_TEXT);
        pDC.BitBlt(pnt.x, pnt.y, bmInfo.bmWidth, bmInfo.bmHeight,
                    &g_gt.mTileDC, 0, 0, SRCCOPY);
        pDC.RestoreDC(-1);
    }
}

void CBitmapImage::SetBitmap(int x, int y, HBITMAP hBMap,
    TileScale eBaseScale /* = fullScale */)
{
    m_bitmap.Attach(hBMap);
    m_eBaseScale = eBaseScale;

    BITMAP bmInfo;
    m_bitmap.GetObject(sizeof(bmInfo), &bmInfo);

    m_rctExtent.left = x;
    m_rctExtent.top = y;
    m_rctExtent.right += bmInfo.bmWidth;
    m_rctExtent.bottom += bmInfo.bmHeight;
}

BOOL CBitmapImage::HitTest(CPoint pt)
{
    CRect rct = GetEnclosingRect();
    return rct.PtInRect(pt);
}

// The top and left locations are assumed to be correct.
void CBitmapImage::SynchronizeExtentRect(CSize sizeWorld, CSize sizeView)
{
    ASSERT((GetDObjFlags() & dobjFlgLayerNatural) != 0);// Only should be called in this case
    ASSERT(m_bitmap.GetSafeHandle() != NULL);
    if (m_bitmap.GetSafeHandle() == NULL)
        return;

    BITMAP bmInfo;
    m_bitmap.GetObject(sizeof(bmInfo), &bmInfo);
    // Might compensate the extend rect for objects with natural
    // sizes in scalings smaller that full scale.
    m_rctExtent.right = m_rctExtent.left;
    m_rctExtent.bottom = m_rctExtent.top;
    m_rctExtent.right += (sizeWorld.cx * bmInfo.bmWidth) / sizeView.cx;
    m_rctExtent.bottom += (sizeWorld.cy * bmInfo.bmHeight) / sizeView.cy;
}

#ifndef     GPLAY
void CBitmapImage::ForceIntoZone(const CRect& pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
        m_rctExtent += pntOffset;
}
#endif

#ifndef GPLAY
OwnerPtr<CSelection> CBitmapImage::CreateSelectProxy(CBrdEditView& pView)
{
    return MakeOwner<CSelGeneric>(pView, *this);
}
#else
void CBitmapImage::Rotate(Rotation90 rot)
{
    //  let parent handle its part
    CDrawObj::Rotate(rot);

    switch (rot)
    {
        case Rotation90::r0:
            ASSERT(!"no-op call");
            break;
        case Rotation90::r90:
        case Rotation90::r180:
        case Rotation90::r270:
        {
            ::OwnerPtr<CBitmap> bmpRot = ::Rotate(m_bitmap, rot);
            m_bitmap.DeleteObject();
            m_bitmap.Attach(bmpRot->Detach());
            break;
        }
        default:
            AfxThrowInvalidArgException();
    }
}
#endif

//DFM19991221

CDrawObj::OwnerPtr CBitmapImage::Clone() const
{
    ::OwnerPtr<CBitmapImage> pObj = MakeOwner<CBitmapImage>();
    pObj->CopyAttributes(*this);
    return pObj;
}

void CBitmapImage::CopyAttributes(const CBitmapImage& source)
{
    CDrawObj_SimplRctExtent::CopyAttributes(source);

    m_eBaseScale = source.m_eBaseScale;
    CDib dib(source.m_bitmap, GetAppPalette());
    m_bitmap.Attach(dib.DIBToBitmap(GetAppPalette())->Detach());
}

void CBitmapImage::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_eBaseScale;

        CDib dib(m_bitmap, GetAppPalette());
#ifndef GPLAY
        dib.SetCompressLevel(((CGamDoc*)ar.m_pDocument)->GetCompressLevel());
#endif
        ar << dib;
    }
    else
    {
        WORD wTmp;
        ar >> wTmp; m_eBaseScale = (TileScale)wTmp;

        CDib dib;
        ar >> dib;
        if (dib)
        {
            ::OwnerPtr<CBitmap> pBMap = dib.DIBToBitmap(GetAppPalette());
            m_bitmap.Attach(pBMap->Detach());
        }
    }
}

//////////////////////////////////////////////////////////////////
// Class CTileImage

void CTileImage::Draw(CDC& pDC, TileScale eScale)
{
    ASSERT(m_pTMgr != NULL);
    CTile tile = m_pTMgr->GetTile(m_tid, eScale);

    CPoint pnt = m_rctExtent.TopLeft();
    if (eScale == halfScale)
    {
        ScalePoint(pnt, pDC.GetViewportExt(), pDC.GetWindowExt());
        pDC.SaveDC();
        pDC.SetMapMode(MM_TEXT);
    }
    tile.TransBlt(pDC, pnt.x, pnt.y);
    if (eScale == halfScale)
        pDC.RestoreDC(-1);
}

void CTileImage::SetTile(int x, int y, TileID tid)
{
    ASSERT(m_pTMgr != NULL);
    CTile tile = m_pTMgr->GetTile(tid);
    m_tid = tid;
    m_rctExtent.left = m_rctExtent.right = x;
    m_rctExtent.top = m_rctExtent.bottom = y;
    m_rctExtent.right += tile.GetWidth();
    m_rctExtent.bottom += tile.GetHeight();
}

BOOL CTileImage::HitTest(CPoint pt)
{
    CRect rct = GetEnclosingRect();
    return rct.PtInRect(pt);
}

#ifndef     GPLAY
void CTileImage::ForceIntoZone(const CRect& pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
        m_rctExtent += pntOffset;
}
#endif

#ifndef GPLAY
OwnerPtr<CSelection> CTileImage::CreateSelectProxy(CBrdEditView& pView)
{
    return MakeOwner<CSelGeneric>(pView, *this);
}
#else
void CTileImage::Rotate(Rotation90 rot)
{
    //  let parent handle its part
    CDrawObj::Rotate(rot);

    switch (rot)
    {
        case Rotation90::r0:
            ASSERT(!"no-op call");
            break;
        case Rotation90::r90:
        case Rotation90::r180:
        case Rotation90::r270:
            // need to replace tile
            m_tid = m_pTMgr->Get(m_tid, rot);
            break;
        default:
            AfxThrowInvalidArgException();
    }
}
#endif

CDrawObj::OwnerPtr CTileImage::Clone() const
{
    ::OwnerPtr<CTileImage> pObj = MakeOwner<CTileImage>();
    pObj->CopyAttributes(*this);
    return pObj;
}

void CTileImage::CopyAttributes(const CTileImage& source)
{
    CDrawObj_SimplRctExtent::CopyAttributes(source);
    m_tid = source.m_tid;
    // cloning isn't really violation of source's const
    m_pTMgr = const_cast<CTileManager*>(&*source.m_pTMgr);
}

void CTileImage::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_tid;
    }
    else
    {
        m_pTMgr = ((CGamDoc*)ar.m_pDocument)->GetTileManager();
        ar >> m_tid;
    }
}

//////////////////////////////////////////////////////////////////
// Class CText

CText::~CText()
{
}

void CText::Draw(CDC& pDC, TileScale eScale)
{
    if (eScale == smallScale && m_rctExtent.Height() < 16)
        return;

    HFONT hFont = CGamDoc::GetFontManager()->GetFontHandle(m_fontID, m_geoRot);
    CFont* pPrvFont = pDC.SelectObject(CFont::FromHandle(hFont));
    pDC.SetBkMode(TRANSPARENT);
    COLORREF crPrev = pDC.SetTextColor(m_crText);
    CPoint topLeft = m_rctExtent.TopLeft() + m_geoOffset;
    pDC.ExtTextOut(topLeft.x, topLeft.y,
        0, NULL, m_text, m_text.GetLength(), NULL);
    pDC.SetTextColor(crPrev);
}

void CText::SetText(int x, int y, const char* pszText, FontID fntID,
    COLORREF crText)
{
    m_text = pszText;
    m_crText = crText;
    m_rctExtent.left = x;
    m_rctExtent.top = y;

    SetFont(fntID);
}

BOOL CText::SetFont(FontID fid)
{
    CFontTbl* pFontMgr = CGamDoc::GetFontManager();

    m_fontID = fid;

    HFONT hFont = pFontMgr->GetFontHandle(m_fontID);
    CFont* pPrvFont = g_gt.mDC1.SelectObject(CFont::FromHandle(hFont));

    // Compute (new) dimensions
    CSize sizeTxt = g_gt.mDC1.GetTextExtent(m_text, m_text.GetLength());
    m_rctExtent.right = m_rctExtent.left + sizeTxt.cx;
    m_rctExtent.bottom = m_rctExtent.top + sizeTxt.cy;
    g_gt.mDC1.SelectObject(pPrvFont);

    return TRUE;
}

BOOL CText::HitTest(CPoint pt)
{
    // First check if anywhere near the object.
    CRect rct = GetEnclosingRect();
    return rct.PtInRect(pt);
}

#ifndef     GPLAY
void CText::ForceIntoZone(const CRect& pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
        m_rctExtent += pntOffset;
}
#endif

#ifndef GPLAY
OwnerPtr<CSelection> CText::CreateSelectProxy(CBrdEditView& pView)
{
    return MakeOwner<CSelGeneric>(pView, *this);
}
#else
void CText::Rotate(Rotation90 rot)
{
    // let parent handle its part
    CDrawObj::Rotate(rot);

    switch (rot)
    {
        case Rotation90::r0:
            ASSERT(!"no-op call");
            m_geoRot = 0;
            m_geoOffset = CSize(0, 0);
            break;
        case Rotation90::r90:
            m_geoRot = 90;
            m_geoOffset = CSize(m_rctExtent.right - m_rctExtent.left, 0);
            break;
        case Rotation90::r180:
            m_geoRot = 180;
            m_geoOffset = m_rctExtent.BottomRight() - m_rctExtent.TopLeft();
            break;
        case Rotation90::r270:
            m_geoRot = 270;
            m_geoOffset = CSize(0, m_rctExtent.bottom - m_rctExtent.top);
            break;
        default:
            AfxThrowInvalidArgException();
    }
}
#endif

CDrawObj::OwnerPtr CText::Clone() const
{
    ::OwnerPtr<CText> pObj = MakeOwner<CText>();
    pObj->CopyAttributes(*this);
    return pObj;
}

void CText::CopyAttributes(const CText& source)
{
    CFontTbl* pFontMgr = CGamDoc::GetFontManager();

    CDrawObj_SimplRctExtent::CopyAttributes(source);

    m_nAngle = source.m_nAngle;
    m_crText = source.m_crText;
    m_text = source.m_text;

    m_fontID = source.m_fontID;

    m_geoRot = source.m_geoRot;
    m_geoOffset = source.m_geoOffset;
}

void CText::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    CFontTbl* pFontMgr = CGamDoc::GetFontManager();
    if (ar.IsStoring())
    {
        if (m_geoRot)
        {
            ASSERT(!"serializing rotated board not supported");
            AfxThrowNotSupportedException();
        }
        ar << (WORD)m_nAngle;
        ar << (DWORD)m_crText;
        ar << m_text;
        pFontMgr->Archive(ar, m_fontID);
    }
    else
    {
        WORD wTmp;
        DWORD dwTmp;
        m_fontID = 0;
        ar >> wTmp; m_nAngle = (int)wTmp;
        ar >> dwTmp; m_crText = (COLORREF)dwTmp;
        ar >> m_text;
        pFontMgr->Archive(ar, m_fontID);
    }
}

//////////////////////////////////////////////////////////////////
//   THE FOLLOWING OBJECTS ARE ONLY USED IN THE PLAYER MODULE   //
//////////////////////////////////////////////////////////////////

#ifdef GPLAY

// Tile drawing helper func...

static void DrawObjTile(CDC& pDC, CPoint pnt, CTileManager* pTMgr, TileID tid,
    TileScale eScale)
{
    CTile tile = pTMgr->GetTile(tid, eScale);

    if (eScale == halfScale)
    {
        ScalePoint(pnt, pDC.GetViewportExt(), pDC.GetWindowExt());
        pDC.SaveDC();
        pDC.SetMapMode(MM_TEXT);
    }
    tile.TransBlt(pDC, pnt.x, pnt.y);
    if (eScale == halfScale)
        pDC.RestoreDC(-1);
}

//////////////////////////////////////////////////////////////////
// Class CPieceObj

CRect CPieceObj::GetRect() const
{
    CSize size = GetSize();
    return CRect(CPoint(m_center.x - size.cx/2,
                        m_center.y - size.cy/2),
                size);
}

void CPieceObj::SetRect(const CRect& rct)
{
    m_center = rct.CenterPoint();
}

void CPieceObj::Draw(CDC& pDC, TileScale eScale)
{
    ASSERT(m_pDoc != NULL);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);

    TileID tid;

    if (!m_pDoc->IsScenario() &&
            pPTbl->IsOwnedButNotByCurrentPlayer(m_pid, *m_pDoc))
        tid = pPTbl->GetFrontTileID(m_pid, TRUE);
    else
        tid = pPTbl->GetActiveTileID(m_pid, TRUE);  // Show rotations
    ASSERT(tid != nullTid);

    CPoint pnt = GetRect().TopLeft();
    DrawObjTile(pDC, pnt, pTMgr, tid, eScale);
}

void CPieceObj::SetOwnerMask(DWORD dwMask)
{
    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);
    pPTbl->SetOwnerMask(m_pid, dwMask);
}

BOOL CPieceObj::IsOwned() const
{
    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);
    return pPTbl->IsPieceOwned(m_pid);
}

BOOL CPieceObj::IsOwnedBy(DWORD dwMask) const
{
    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);
    return pPTbl->IsPieceOwnedBy(m_pid, dwMask);
}

BOOL CPieceObj::IsOwnedButNotByCurrentPlayer() const
{
    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);
    return pPTbl->IsOwnedButNotByCurrentPlayer(m_pid, *m_pDoc);
}

void CPieceObj::SetPiece(const CRect& rct, PieceID pid)
{
    m_pid = pid;
    SetRect(rct);
}

CSize CPieceObj::GetSize() const
{
    ASSERT(m_pDoc != NULL);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);

    TileID tid;

    if (pPTbl->IsPieceOwned(m_pid) &&
            !pPTbl->IsPieceOwnedBy(m_pid, m_pDoc->GetCurrentPlayerMask()))
        tid = pPTbl->GetFrontTileID(m_pid, TRUE);
    else
        tid = pPTbl->GetActiveTileID(m_pid, TRUE);
    ASSERT(tid != nullTid);

    CTile tile = pTMgr->GetTile(tid);
    return tile.GetSize();
}

BOOL CPieceObj::HitTest(CPoint pt)
{
    CRect rct = GetEnclosingRect();
    return rct.PtInRect(pt);
}

OwnerPtr<CSelection> CPieceObj::CreateSelectProxy(CPlayBoardView& pView)
{
    return MakeOwner<CSelGeneric>(pView, *this);
}

CDrawObj::OwnerPtr CPieceObj::Clone(CGamDoc* pDoc) const
{
    ::OwnerPtr<CPieceObj> pObj = MakeOwner<CPieceObj>();
    pObj->m_center = m_center;
    pObj->m_dwDObjFlags = m_dwDObjFlags;    // From base class
    pObj->m_pDoc = pDoc;
    pObj->m_pid = m_pid;
    return pObj;
}

BOOL CPieceObj::Compare(const CDrawObj& pObj) const
{
    if (pObj.GetType() != GetType())
    {
        AfxThrowInvalidArgException();
    }
    const CPieceObj& pPce = static_cast<const CPieceObj&>(pObj);
    if (m_pid != pPce.m_pid)
        return FALSE;
    if (m_center != pPce.m_center)
        return FALSE;
    return TRUE;
}

void CPieceObj::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_pid;
    }
    else
    {
        m_pDoc = (CGamDoc*)ar.m_pDocument;
        ar >> m_pid;
    }
}

//////////////////////////////////////////////////////////////////
// Class CMarkObj

void CMarkObj::Draw(CDC& pDC, TileScale eScale)
{
    ASSERT(m_pDoc != NULL);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    TileID tid = GetCurrentTileID();
    CPoint pnt = m_rctExtent.TopLeft();

    DrawObjTile(pDC, pnt, pTMgr, tid, eScale);
}

void CMarkObj::SetMark(CRect& rct, MarkID mid)
{
    m_mid = mid;
    m_rctExtent = rct;
}

TileID CMarkObj::GetCurrentTileID()
{
    ASSERT(m_pDoc != NULL);
    CMarkManager* pMMgr = m_pDoc->GetMarkManager();
    ASSERT(pMMgr != NULL);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    MarkDef& pMark = pMMgr->GetMark(m_mid);
    ASSERT(pMark.m_tid != nullTid);

    if (m_nFacingDegCW != 0)
    {
        // Handle rotated markers...
        ElementState state(m_mid, m_nFacingDegCW);
        CTileFacingMap* pMapFacing = m_pDoc->GetFacingMap();
        TileID tidFacing = pMapFacing->GetFacingTileID(state);
        if (tidFacing == nullTid)
            tidFacing = pMapFacing->CreateFacingTileID(state, pMark.m_tid);
        return tidFacing;
    }
    else
        return pMark.m_tid;
}

void CMarkObj::ResyncExtentRect()
{
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    TileID tid = GetCurrentTileID();
    ASSERT(tid != nullTid);

    CTile tile = pTMgr->GetTile(tid);
    CPoint pnt = m_rctExtent.CenterPoint();
    pnt.x -= tile.GetWidth() / 2;
    pnt.y -= tile.GetHeight() / 2;
    m_rctExtent = CRect(pnt, tile.GetSize());
}

BOOL CMarkObj::HitTest(CPoint pt)
{
    CRect rct = GetEnclosingRect();
    return rct.PtInRect(pt);
}

OwnerPtr<CSelection> CMarkObj::CreateSelectProxy(CPlayBoardView& pView)
{
    return MakeOwner<CSelGeneric>(pView, *this);
}

CDrawObj::OwnerPtr CMarkObj::Clone(CGamDoc* pDoc) const
{
    ::OwnerPtr<CMarkObj> pObj = MakeOwner<CMarkObj>();
    pObj->m_rctExtent = m_rctExtent;        // From base class
    pObj->m_dwDObjFlags = m_dwDObjFlags;    // From base class
    pObj->m_pDoc = pDoc;
    pObj->m_mid = m_mid;
    pObj->m_nFacingDegCW = m_nFacingDegCW;
    pObj->m_dwObjectID = m_dwObjectID;
    return pObj;
}

BOOL CMarkObj::Compare(const CDrawObj& pObj) const
{
    if (pObj.GetType() != GetType())
    {
        AfxThrowInvalidArgException();
    }
    const CMarkObj& pMrk = static_cast<const CMarkObj&>(pObj);
    if (m_mid != pMrk.m_mid)
        return FALSE;
    if (m_dwObjectID != pMrk.m_dwObjectID)
        return FALSE;
    if (m_nFacingDegCW != pMrk.m_nFacingDegCW)
        return FALSE;
    if (m_rctExtent != pMrk.m_rctExtent)
        return FALSE;
    return TRUE;
}

void CMarkObj::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_dwObjectID;
        ar << m_mid;
        ar << m_nFacingDegCW;                                 //Ver2.0
    }
    else
    {
        m_pDoc = ((CGamDoc*)ar.m_pDocument);
        ar >> m_dwObjectID;
        ar >> m_mid;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))       //Ver2.0
        {
            ar >> m_nFacingDegCW;
            if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   //Ver2.90
                m_nFacingDegCW *= uint16_t(5);                                // Convert old value to degrees
        }
        else
            m_nFacingDegCW = uint16_t(0);
    }
}

//////////////////////////////////////////////////////////////////
// Class CLineObj

CDrawObj::OwnerPtr CLineObj::Clone(CGamDoc* pDoc) const
{
    ::OwnerPtr<CLineObj> pObj = MakeOwner<CLineObj>();
    pObj->m_rctExtent = m_rctExtent;        // From base class
    pObj->m_dwDObjFlags = m_dwDObjFlags;    // From base class
    pObj->m_ptBeg = m_ptBeg;                // CLine class
    pObj->m_ptEnd = m_ptEnd;
    pObj->m_crLine = m_crLine;
    pObj->m_nLineWidth = m_nLineWidth;
    pObj->m_dwObjectID = m_dwObjectID;      // From this class
    return pObj;
}

BOOL CLineObj::Compare(const CDrawObj& pObj) const
{
    if (pObj.GetType() != GetType())
    {
        AfxThrowInvalidArgException();
    }
    const CLineObj& pLine = static_cast<const CLineObj&>(pObj);
    if (m_dwObjectID != pLine.m_dwObjectID)
        return FALSE;
    if (m_ptBeg != pLine.m_ptBeg)
        return FALSE;
    if (m_ptEnd != pLine.m_ptEnd)
        return FALSE;
    return TRUE;
}

void CLineObj::Serialize(CArchive& ar)
{
    CLine::Serialize(ar);
    if (ar.IsStoring())
        ar << m_dwObjectID;
    else
        ar >> m_dwObjectID;
}

#endif  // GPLAY

//////////////////////////////////////////////////////////////////
//                     CDrawList Processing                     //
//////////////////////////////////////////////////////////////////

// pDc - The device context to draw on.
// pDrawRct - Only objects within this rectangle will be rendered.
// eScale - Determines what scale of board image should be rendered.
// bApplyVisibility - Honors the object's visibility mask. That is,
//      an object will only be rendered if it is allowed at a
//      particular TileScale.
// bDrawPass2Objects - Some objects are tagged to only be rendered
//      in a so-called pass 2. This flag is used to determine if
//      the current call is to only render pass 2 objects.
// bHideUnlocked - If this is set if all unlocked objects are to
//      be hidden. The user might do this to see the map's terrain.

void CDrawList::Draw(CDC& pDC, const CRect& pDrawRct, TileScale eScale,
    BOOL bApplyVisibility /* = TRUE */, BOOL bDrawPass2Objects /* = FALSE */,
    BOOL bHideUnlocked /* = FALSE */, BOOL bDrawLockedFirst /* = FALSE */)
{
    // We might need to draw locked objects first so they
    // show up under other object which are unlocked. This is
    // only valid in a non-pass 2 call.
    if (!bDrawPass2Objects && bDrawLockedFirst)
    {
        for (iterator pos = begin(); pos != end(); ++pos)
        {
            CDrawObj& pDObj = **pos;

            // Check if the object should be drawn in this scaling
            if (bApplyVisibility && ((pDObj.GetDObjFlags() & eScale) == 0))
                continue;                   // Doesn't qualify

            ASSERT(!((pDObj.GetDObjFlags() & dobjFlgLockDown) != 0 &&
                (pDObj.GetDObjFlags() & dobjFlgDrawPass2) != 0));// We'll never have a locked pass 2 object

            if ((pDObj.GetDObjFlags() & dobjFlgLockDown) == 0)
                continue;                  // Only process locked objects

            if (pDObj.IsVisible(pDrawRct))
                pDObj.Draw(pDC, eScale);
        }
    }

    for (iterator pos = begin(); pos != end(); ++pos)
    {
        CDrawObj& pDObj = **pos;

        // Check if the object should be drawn in this scaling
        if (bApplyVisibility && ((pDObj.GetDObjFlags() & eScale) == 0))
            continue;                   // Doesn't qualify

        // Check if unlocked objects should be hidden
        if (bHideUnlocked && ((pDObj.GetDObjFlags() & dobjFlgLockDown) == 0))
            continue;                   // Doesn't qualify

        // Only draw objects in their proper passes
        if (bDrawLockedFirst && (pDObj.GetDObjFlags() & dobjFlgLockDown) != 0 ||
            bDrawPass2Objects && (pDObj.GetDObjFlags() & dobjFlgDrawPass2) == 0 ||
           !bDrawPass2Objects && (pDObj.GetDObjFlags() & dobjFlgDrawPass2) != 0)
        {
            continue;                   // Doesn't qualify
        }

        if (pDObj.IsVisible(pDrawRct))
            pDObj.Draw(pDC, eScale);
    }
}

CDrawObj* CDrawList::HitTest(CPoint pt, TileScale eScale,
    BOOL bApplyVisibility /* = TRUE */)
{
    for (reverse_iterator pos = rbegin(); pos != rend(); ++pos)
    {
        CDrawObj& pDObj = **pos;

        if (bApplyVisibility && ((pDObj.GetDObjFlags() & eScale) == 0))
            continue;                   // Doesn't qualify

        if (pDObj.HitTest(pt))
            return &pDObj;
    }
    return NULL;
}

void CDrawList::DrillDownHitTest(CPoint point, std::vector<CB::not_null<CDrawObj*>>& selLst,
    TileScale eScale, BOOL bApplyVisibility /* = TRUE */)
{
    for (reverse_iterator pos = rbegin(); pos != rend(); ++pos)
    {
        CDrawObj& pDObj = **pos;

        if (bApplyVisibility && ((pDObj.GetDObjFlags() & eScale) == 0))
            continue;                   // Doesn't qualify
        if (pDObj.HitTest(point))
            selLst.push_back(&pDObj);
    }
}

void CDrawList::ArrangeObjectListInDrawOrder(std::vector<CB::not_null<CDrawObj*>>& pLst)
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    std::vector<CB::not_null<CDrawObj*>> tmpLst;

    for (iterator pos = begin() ; pos != end() ; ++pos)
    {
        CDrawObj& pDObj = **pos;
        if (std::find(pLst.begin(), pLst.end(), &pDObj) != pLst.end())
            tmpLst.push_back(&pDObj);
    }
    pLst = std::move(tmpLst);
}

void CDrawList::ArrangeObjectListInVisualOrder(std::vector<CB::not_null<CDrawObj*>>& pLst)
{
    // Loop backwards through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    std::vector<CB::not_null<CDrawObj*>> tmpLst;

    for (reverse_iterator pos = rbegin() ; pos != rend() ; ++pos)
    {
        CDrawObj& pDObj = **pos;
        if (std::find(pLst.begin(), pLst.end(), &pDObj) != pLst.end())
            tmpLst.push_back(&pDObj);
    }
    pLst = std::move(tmpLst);
}

#ifdef GPLAY

void CDrawList::ArrangePieceTableInDrawOrder(std::vector<PieceID>& pTbl) const
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    std::vector<PieceID> tmpTbl;
    tmpTbl.reserve(pTbl.size());

    for (const_iterator pos = begin(); pos != end(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        if (pDObj.GetType() == CDrawObj::drawPieceObj)
        {
            for (size_t i = 0; i < pTbl.size(); i++)
            {
                if (pTbl.at(i)== static_cast<const CPieceObj&>(pDObj).m_pid)
                {
                    tmpTbl.push_back(static_cast<const CPieceObj&>(pDObj).m_pid);
                    break;
                }
            }
        }
    }
    ASSERT(tmpTbl.size() == pTbl.size());
    pTbl = std::move(tmpTbl);
}

void CDrawList::ArrangePieceTableInVisualOrder(std::vector<PieceID>& pTbl) const
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    std::vector<PieceID> tmpTbl;
    tmpTbl.reserve(pTbl.size());

    for (const_reverse_iterator pos = rbegin(); pos != rend(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        if (pDObj.GetType() == CDrawObj::drawPieceObj)
        {
            for (size_t i = 0; i < pTbl.size(); i++)
            {
                if (pTbl.at(i)== static_cast<const CPieceObj&>(pDObj).m_pid)
                {
                    tmpTbl.push_back(static_cast<const CPieceObj&>(pDObj).m_pid);
                    break;
                }
            }
        }
    }
    ASSERT(tmpTbl.size() == pTbl.size());
    pTbl = std::move(tmpTbl);
}

void CDrawList::ArrangeObjectPtrTableInDrawOrder(std::vector<CB::not_null<CDrawObj*>>& pTbl) const
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    std::vector<CB::not_null<CDrawObj*>> tmpTbl;
    tmpTbl.reserve(pTbl.size());

    for (const_iterator pos = begin(); pos != end(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        for (size_t i = 0; i < pTbl.size(); i++)
        {
            if (pTbl.at(i) == &pDObj)
            {
                tmpTbl.push_back(pTbl.at(i));
                break;
            }
        }
    }
    ASSERT(tmpTbl.size() == pTbl.size());
    pTbl.clear();
    pTbl = std::move(tmpTbl);
}

void CDrawList::ArrangeObjectPtrTableInVisualOrder(std::vector<CB::not_null<CDrawObj*>>& pTbl) const
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    std::vector<CB::not_null<CDrawObj*>> tmpTbl;
    tmpTbl.reserve(pTbl.size());

    for (const_reverse_iterator pos = rbegin(); pos != rend(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        for (size_t i = 0; i < pTbl.size(); i++)
        {
            if (pTbl.at(i) == &pDObj)
            {
                tmpTbl.push_back(pTbl.at(i));
                break;
            }
        }
    }
    ASSERT(tmpTbl.size() == pTbl.size());
    pTbl.clear();
    pTbl = std::move(tmpTbl);
}

#endif // GPLAY

CDrawList::const_iterator CDrawList::Find(const CDrawObj& drawObj) const
{
    return std::find_if(begin(), end(), [&drawObj](const CDrawObj::OwnerPtr& p) { return p == &drawObj; });
}

CDrawList::iterator CDrawList::Find(const CDrawObj& drawObj)
{
    return std::find_if(begin(), end(), [&drawObj](const CDrawObj::OwnerPtr& p) { return p == &drawObj; });
}

// NOTE:  RemoveObject* do not erase
void CDrawList::RemoveObjectsInList(const std::vector<CB::not_null<CDrawObj*>>& pLst)
{
    for (size_t i = size_t(0) ; i < pLst.size() ; ++i)
    {
        CDrawObj& pDObj = *pLst[i];
        RemoveObject(pDObj);
    }
}

void CDrawList::RemoveObject(const CDrawObj& pDrawObj)
{
    iterator pos = Find(pDrawObj);
    if (pos != end())
    {
        /* NOTE:  RemoveObject* do not destroy obj, so we must
                    call unique_ptr::release.  This violates
                    not_null, but we'll live with it since we
                    clean up by destroying  the invalid
                    not_null. */
        CB::propagate_const<std::unique_ptr<CDrawObj>> p = CB::get_underlying(std::move(*pos));
        CB::get_underlying(std::move(p)).release();
        erase(pos);
    }
}

#ifdef GPLAY

void CDrawList::SetOwnerMasks(DWORD dwOwnerMask)
{
    for (iterator pos = begin(); pos != end(); ++pos)
    {
        CDrawObj& pObj = **pos;
        if (pObj.GetType() == CDrawObj::drawPieceObj)
        {
            CPieceObj& pPObj = static_cast<CPieceObj&>(pObj);
            pPObj.SetOwnerMask(dwOwnerMask);
        }
    }
}

const CPieceObj* CDrawList::FindPieceID(PieceID pid) const
{
    const CDrawObj* pObj = FindObjectID(static_cast<ObjectID>(pid));
    if (pObj != NULL)
    {
        ASSERT(pObj->GetType() == CDrawObj::drawPieceObj);
        return static_cast<const CPieceObj*>(pObj);
    }
    return NULL;
}

const CDrawObj* CDrawList::FindObjectID(ObjectID oid) const
{
    for (const_iterator pos = begin(); pos != end(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        if (pDObj.GetObjectID() == oid)
             return &pDObj;
    }
    return NULL;
}

void CDrawList::GetPieceObjectPtrList(std::vector<CB::not_null<CPieceObj*>>& pLst)
{
    pLst.clear();
    for (iterator pos = begin(); pos != end(); ++pos)
    {
        CDrawObj& pDObj = **pos;
        if (pDObj.GetType() == CDrawObj::drawPieceObj)
            pLst.push_back(&static_cast<CPieceObj&>(pDObj));
    }
}

void CDrawList::GetPieceIDTable(std::vector<PieceID>& pTbl) const
{
    pTbl.clear();
    for (const_iterator pos = begin(); pos != end(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        if (pDObj.GetType() == CDrawObj::drawPieceObj)
            pTbl.push_back(static_cast<const CPieceObj&>(pDObj).m_pid);
    }
}

void CDrawList::GetObjectListFromPieceIDTable(const std::vector<PieceID>& pTbl, std::vector<CB::not_null<CPieceObj*>>& pLst)
{
    pLst.clear();
    pLst.reserve(pTbl.size());
    for (size_t i = size_t(0); i < pTbl.size(); i++)
    {
        CPieceObj* pObj = FindPieceID(pTbl.at(i));
        ASSERT(pObj != NULL);
        if (pObj)
            pLst.push_back(pObj);
    }
}

void CDrawList::GetPieceIDTableFromObjList(const std::vector<CB::not_null<CDrawObj*>>& pLst, std::vector<PieceID>& pTbl,
    BOOL bDeleteObjs /* = FALSE*/)
{
    pTbl.clear();
    for (size_t i = size_t(0) ; i < pLst.size() ; ++i)
    {
        CDrawObj& pDObj = *pLst[i];
        if (pDObj.GetType() == CDrawObj::drawPieceObj)
            pTbl.push_back(static_cast<CPieceObj&>(pDObj).m_pid);
        if (bDeleteObjs) delete &pDObj;
    }
}

BOOL CDrawList::HasMarker() const
{
    for (const_iterator pos = begin(); pos != end(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        if (pDObj.GetType() == CDrawObj::drawMarkObj)
            return TRUE;
    }
    return FALSE;
}

#endif // GPLAY

#ifndef GPLAY

BOOL CDrawList::PurgeMissingTileIDs(CTileManager* pTMgr)
{
    BOOL bPurged = FALSE;
    /* NOTE:  extra complexity due to
            erase invalidating all iterators to erased object */
    for (iterator pos1 = begin(), pos2; (pos2 = pos1) != end(); )
    {
        CDrawObj& pDObj = **pos1++;
        if (pDObj.GetType() == CDrawObj::drawTile)
        {
            if (!pTMgr->IsTileIDValid(static_cast<CTileImage&>(pDObj).m_tid))
            {
                erase(pos2);
                bPurged = TRUE;
            }
        }
    }
    return bPurged;
}

BOOL CDrawList::IsTileInUse(TileID tid) const
{
    for (const_iterator pos = begin(); pos != end(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        if (pDObj.GetType() == CDrawObj::drawTile)
        {
            if (static_cast<const CTileImage&>(pDObj).m_tid == tid)
                return TRUE;
        }
    }
    return FALSE;
#ifdef WANT_ODD_WAY_OF_DOING_THINGS
    POSITION pos1, pos2;
    for (pos1 = GetHeadPosition(); (pos2 = pos1) != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos1);
        ASSERT(pDObj != NULL);
        if (pDObj->GetType() == CDrawObj::drawTile)
        {
            if (((CTileImage*)pDObj)->m_tid == tid)
                return TRUE;
        }
    }
    return FALSE;
#endif
}

void CDrawList::ForceIntoZone(const CRect& pRctZone)
{
    for (reverse_iterator pos = rbegin(); pos != rend(); ++pos)
    {
        CDrawObj& pDObj = **pos;
        pDObj.ForceIntoZone(pRctZone);
    }
}

#endif  // !GPLAY

#ifdef GPLAY
CDrawList CDrawList::Clone(CGamDoc* pDoc) const
{
    CDrawList pLst;
    for (const_iterator pos = begin(); pos != end(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        pLst.push_back(pDObj.Clone(pDoc));
    }
    return pLst;
}

void CDrawList::Restore(CGamDoc* pDoc, const CDrawList& pLst)
{
    clear();
    for (const_iterator pos = pLst.begin(); pos != pLst.end(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        push_back(pDObj.Clone(pDoc));    // Clone it back
    }
}

BOOL CDrawList::Compare(const CDrawList& pLst) const
{
    if (size() != pLst.size())
        return FALSE;

    const_iterator pos1 = begin();
    const_iterator pos2 = pLst.begin();

    for ( ; pos1 != end() && pos2 != pLst.end() ; ++pos1, ++pos2)
    {
        const CDrawObj& pDObj1 = **pos1;
        const CDrawObj& pDObj2 = **pos2;
        if (pDObj1.GetType() != pDObj2.GetType())
            return FALSE;
        if (!pDObj1.Compare(pDObj2))
            return FALSE;
    }
    ASSERT(pos1 == end() && pos2 == pLst.end());
    return TRUE;
}

void CDrawList::AppendWithRotOffset(const CDrawList& pSourceLst, Rotation90 rot, CPoint pntOffet)
{
    for (const_iterator pos = pSourceLst.begin(); pos != pSourceLst.end(); ++pos)
    {
        const CDrawObj& pDObj = **pos;
        CDrawObj::OwnerPtr pObjClone = pDObj.Clone();
        // order matters:  rotate, then offset
        if (rot != Rotation90::r0)
        {
            pObjClone->Rotate(rot);
        }
        pObjClone->OffsetObject(pntOffet);
        push_back(std::move(pObjClone));
    }
}

#endif // GPLAY

void CDrawList::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            ar << value_preserving_cast<WORD>(size());
        }
        else
        {
            CB::WriteCount(ar, size());
        }
#ifdef _DEBUG
        size_t nObjects = size();
#endif
        for (iterator pos = begin(); pos != end(); ++pos)
        {
            CDrawObj& pDObj = **pos;
            ASSERT(pDObj.GetType() < CDrawObj::drawUnknown);
            ar << static_cast<WORD>(pDObj.GetType());
            pDObj.Serialize(ar);
#ifdef _DEBUG
            nObjects--;
#endif
        }
        ASSERT(nObjects == size_t(0));
    }
    else
    {
        clear();
        size_t count;
        if (CB::GetVersion(ar) <= NumVersion(3, 90))
        {
            WORD wCount;
            ar >> wCount;
            count = wCount;
        }
        else
        {
            count = CB::ReadCount(ar);
        }
        for (size_t i = size_t(0); i < count; ++i)
        {
            OwnerOrNullPtr<CDrawObj> pDObj;
            WORD wType;
            ar >> wType;
            switch (static_cast<CDrawObj::CDrawObjType>(wType))
            {
                case CDrawObj::drawRect:
                    pDObj = MakeOwner<CRectObj>();
                    break;
                case CDrawObj::drawEllipse:
                    pDObj = MakeOwner<CEllipse>();
                    break;
                case CDrawObj::drawLine:
                    pDObj = MakeOwner<CLine>();
                    break;
                case CDrawObj::drawTile:
                    pDObj = MakeOwner<CTileImage>();
                    break;
                case CDrawObj::drawText:
                    pDObj = MakeOwner<CText>();
                    break;
                case CDrawObj::drawPolygon:
                    pDObj = MakeOwner<CPolyObj>();
                    break;
                case CDrawObj::drawBitmap:
                    pDObj = MakeOwner<CBitmapImage>();
                    break;
#ifdef GPLAY
                case CDrawObj::drawPieceObj:
                    pDObj = MakeOwner<CPieceObj>();
                    break;
                case CDrawObj::drawMarkObj:
                    pDObj = MakeOwner<CMarkObj>();
                    break;
                case CLineObj::drawLineObj:
                    pDObj = MakeOwner<CLineObj>();
                    break;
#endif
                default:
                    ASSERT(FALSE);
                    AfxThrowArchiveException(CArchiveException::badClass);
            }
            pDObj->Serialize(ar);
            push_back(std::move(pDObj));
        }
    }
}


