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

////////////////////////////////////////////////////////////////////
// Class variables

CPen*   CDrawObj::c_pPrvPen = NULL;
CBrush* CDrawObj::c_pPrvBrush = NULL;
BOOL    CDrawObj::c_bHitTestDraw = FALSE;

//////////////////////////////////////////////////////////////////
// Class CDrawObj

CDrawObj::CDrawObj()
{
    m_rctExtent.SetRectEmpty();
    m_dwDObjFlags = 0;
    SetScaleVisibility(AllTileScales);
}

BOOL CDrawObj::IsVisible(RECT* pClipRct)
{
    CRect rct = GetEnclosingRect();
    return rct.IntersectRect(&rct, pClipRct);
}

#ifdef GPLAY
void CDrawObj::MoveObject(CPoint ptUpLeft)
{
    m_rctExtent += CPoint(ptUpLeft.x - m_rctExtent.left,
        ptUpLeft.y - m_rctExtent.top);
}
#endif

//DFM991129
void CDrawObj::OffsetObject(CPoint offset)
{
    m_rctExtent += offset;
}
//DFM991129

BOOL CDrawObj::IsExtentOutOfZone(CRect* pRctZone, CPoint& pnt)
{
    CRect rct;
    rct.UnionRect(pRctZone, &m_rctExtent);
    if (rct != *pRctZone)
    {
        pnt.x = pnt.y = 0;
        // Calc offset needed to push the rect onto the pRctZone
        if (m_rctExtent.right > pRctZone->right)
            pnt.x = pRctZone->right - m_rctExtent.right;
        if (m_rctExtent.bottom > pRctZone->bottom)
            pnt.y = pRctZone->bottom - m_rctExtent.bottom;
        return TRUE;
    }
    return FALSE;
}

// Sets up pen and brush for drawing. The previous pen
// and brush are saved in class variables.

void CDrawObj::SetUpDraw(CDC* pDC, CPen* pPen, CBrush* pBrush)
{
    if (c_bHitTestDraw)
        pPen->CreateStockObject(BLACK_PEN);
    else
    {
        if (GetLineColor() != noColor)
        {
            pPen->CreatePen(PS_SOLID, GetLineWidth(), GetLineColor());
        }
        else
            pPen->CreateStockObject(NULL_PEN);
    }

    if (GetFillColor() != noColor)
    {
        if (c_bHitTestDraw)
            pBrush->CreateStockObject(BLACK_BRUSH);
        else
            pBrush->CreateSolidBrush(GetFillColor());
    }
    else
        pBrush->CreateStockObject(NULL_BRUSH);

    c_pPrvPen = pDC->SelectObject(pPen);
    c_pPrvBrush = pDC->SelectObject(pBrush);
}

void CDrawObj::CleanUpDraw(CDC *pDC)
{
    pDC->SelectObject(c_pPrvPen);
    pDC->SelectObject(c_pPrvBrush);
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
    Draw(&dcmem, fullScale);
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
        ar << (short)m_rctExtent.left;
        ar << (short)m_rctExtent.top;
        ar << (short)m_rctExtent.right;
        ar << (short)m_rctExtent.bottom;
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
        ar >> (short)sTmp; m_rctExtent.left = sTmp;
        ar >> (short)sTmp; m_rctExtent.top = sTmp;
        ar >> (short)sTmp; m_rctExtent.right = sTmp;
        ar >> (short)sTmp; m_rctExtent.bottom = sTmp;
    }
}

//DFM19991213
void CDrawObj::CopyAttributes(CDrawObj *source)
{
    m_dwDObjFlags = source->m_dwDObjFlags;
    m_rctExtent   = source->m_rctExtent;
}
//DFM19991213

////////////////////////////////////////////////////////////////////
// CRectObject methods

void CRectObj::Draw(CDC *pDC, TileScale)
{
    CPen   penEdge;
    CBrush brushFill;

    SetUpDraw(pDC, &penEdge, &brushFill);
    pDC->Rectangle(&m_rctExtent);
    CleanUpDraw(pDC);
}

CRect CRectObj::GetEnclosingRect()
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
void CRectObj::ForceIntoZone(CRect* pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
        m_rctExtent += pntOffset;
}

CSelection* CRectObj::CreateSelectProxy(CBrdEditView* pView)
{
    return new CSelRect(pView, this);
}
#endif

//DFM19991210
CDrawObj* CRectObj::Clone()
{
    CRectObj *returnValue = new CRectObj();

    returnValue->CopyAttributes(this);

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
void CRectObj::CopyAttributes (CRectObj *source)
{
    CDrawObj::CopyAttributes (source);

    m_crFill     = source->m_crFill;
    m_crLine     = source->m_crLine;
    m_nLineWidth = source->m_nLineWidth;
}
//DFM19991213

////////////////////////////////////////////////////////////////////
// CEllipse methods

void CEllipse::Draw(CDC *pDC, TileScale)
{
    CPen   penEdge;
    CBrush brushFill;

    SetUpDraw(pDC, &penEdge, &brushFill);
    pDC->Ellipse(&m_rctExtent);
    CleanUpDraw(pDC);
}

#ifndef     GPLAY
CSelection* CEllipse::CreateSelectProxy(CBrdEditView* pView)
{
    return new CSelEllipse(pView, this);
}
#endif

//DFM19991213
CDrawObj* CEllipse::Clone()
{
    CEllipse *returnValue = new CEllipse();

    returnValue->CRectObj::CopyAttributes(this);

    return (returnValue);
}
//DFM19991213

////////////////////////////////////////////////////////////////////
// CPolyObj methods

void CPolyObj::Draw(CDC *pDC, TileScale)
{
    if (m_pPnts == NULL)
        return;
    CPen   penEdge;
    CBrush brushFill;
    SetUpDraw(pDC, &penEdge, &brushFill);

    if (m_crFill == noColor)
        pDC->Polyline(m_pPnts, m_nPnts);
    else
        pDC->Polygon(m_pPnts, m_nPnts);

    CleanUpDraw(pDC);
}

void CPolyObj::AddPoint(CPoint pnt)
{
    POINT* pNew = new POINT[m_nPnts + 1];
    if (m_pPnts != NULL)
    {
        memcpy(pNew, m_pPnts, sizeof(POINT) * m_nPnts);
        delete m_pPnts;
    }
    m_pPnts = pNew;
    m_pPnts[m_nPnts] = pnt;
    m_nPnts++;
    ComputeExtent();
}

void CPolyObj::SetNewPolygon(POINT* pPnts, int nPnts)
{
    if (m_pPnts != NULL)
        delete m_pPnts;
    m_pPnts = new POINT[nPnts];
    m_nPnts = nPnts;
    memcpy(m_pPnts, pPnts, sizeof(POINT) * nPnts);
    ComputeExtent();
}

void CPolyObj::ComputeExtent()
{
    m_rctExtent.SetRectEmpty();
    if (m_pPnts == NULL)
        return;
    int xmin = INT_MAX, xmax = INT_MIN, ymin = INT_MAX, ymax = INT_MIN;
    for (int i = 0; i < m_nPnts; i++)
    {
        xmin = min(xmin, m_pPnts[i].x);
        xmax = max(xmax, m_pPnts[i].x);
        ymin = min(ymin, m_pPnts[i].y);
        ymax = max(ymax, m_pPnts[i].y);
    }
    m_rctExtent.SetRect(xmin, ymin, xmax, ymax);
}

CRect CPolyObj::GetEnclosingRect()
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
void CPolyObj::ForceIntoZone(CRect* pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
    {
        for (int i = 0; i < m_nPnts; i++)
        {
            m_pPnts[i].x += pntOffset.x;
            m_pPnts[i].y += pntOffset.y;
        }
        ComputeExtent();
    }
}
#endif

#ifndef     GPLAY
CSelection* CPolyObj::CreateSelectProxy(CBrdEditView* pView)
{
    return new CSelPoly(pView, this);
}
#endif

void CPolyObj::OffsetObject (CPoint offset)
{
    //  let parent handle its part
    CDrawObj::OffsetObject (offset);

    //  now we handle our part
    for (int i = 0; i < m_nPnts; i++)
    {
        m_pPnts[i].x += offset.x;
        m_pPnts[i].y += offset.y;
    }
}

CDrawObj* CPolyObj::Clone()
{
    CPolyObj* pObj = new CPolyObj();
    pObj->CopyAttributes(this);
    return pObj;
}

void CPolyObj::CopyAttributes(CPolyObj *source)
{
    CDrawObj::CopyAttributes(source);

    m_crFill     = source->m_crFill;
    m_crLine     = source->m_crLine;
    m_nLineWidth = source->m_nLineWidth;
    m_nPnts      = source->m_nPnts;
    m_pPnts      = new POINT[m_nPnts];
    memcpy(m_pPnts, source->m_pPnts, m_nPnts * sizeof(POINT));
}

void CPolyObj::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (DWORD)m_crFill;
        ar << (DWORD)m_crLine;
        ar << (WORD)m_nLineWidth;
        ar << (WORD)m_nPnts;
        if (m_nPnts > 0)
            WriteArchivePoints(ar, m_pPnts, m_nPnts);
    }
    else
    {
        WORD wTmp;
        DWORD dwTmp;
        ar >> dwTmp; m_crFill = (COLORREF)dwTmp;
        ar >> dwTmp; m_crLine = (COLORREF)dwTmp;
        ar >> wTmp;  m_nLineWidth = (UINT)wTmp;
        ar >> wTmp; m_nPnts = (int)wTmp;
        if (m_nPnts > 0)
        {
            m_pPnts = new POINT[m_nPnts];
            ReadArchivePoints(ar, m_pPnts, m_nPnts);
        }
    }
}

//////////////////////////////////////////////////////////////////
// Class CLine

void CLine::Draw(CDC* pDC, TileScale)
{
    CPen pen;
    if (c_bHitTestDraw)
        pen.CreateStockObject(BLACK_PEN);
    else
        pen.CreatePen(PS_SOLID, m_nLineWidth, m_crLine);
    CPen* pPrvPen = pDC->SelectObject(&pen);

    pDC->MoveTo(m_ptBeg);
    pDC->LineTo(m_ptEnd);

    pDC->SelectObject(pPrvPen);
}

void CLine::SetLine(int xBeg, int yBeg, int xEnd, int yEnd)
{
    m_ptBeg.x = xBeg;
    m_ptBeg.y = yBeg;
    m_ptEnd.x = xEnd;
    m_ptEnd.y = yEnd;
    // An enclosing rectangle *must* have some volume...
    int nWidth = m_nLineWidth / 2 ? m_nLineWidth / 2 : 1;
    m_rctExtent.left = min(xBeg, xEnd) - nWidth;
    m_rctExtent.top = min(yBeg, yEnd) - nWidth;
    m_rctExtent.right = max(xBeg, xEnd) + nWidth;
    m_rctExtent.bottom = max(yBeg, yEnd) +nWidth;
}

CRect CLine::GetEnclosingRect()
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
void CLine::ForceIntoZone(CRect* pRctZone)
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
CSelection* CLine::CreateSelectProxy(CPlayBoardView* pView)
#else
CSelection* CLine::CreateSelectProxy(CBrdEditView* pView)
#endif
{
    return new CSelLine(pView, this);
}

//DFM19991221
void CLine::OffsetObject(CPoint offset)
{
    // let parent handle its part
    CDrawObj::OffsetObject (offset);

    // now we'll handle our part
    m_ptBeg.Offset (offset.x, offset.y);
    m_ptEnd.Offset (offset.x, offset.y);

}

CDrawObj* CLine::Clone()
{
    CLine* pObj = new CLine;
    pObj->CopyAttributes(this);
    return pObj;
}
//DFM19991213

#ifdef GPLAY
CDrawObj* CLine::Clone(CGamDoc* pDoc)
{
    CLine* pObj = new CLine;
    pObj->CopyAttributes(this);    //DFM19991214
    return pObj;
}

BOOL CLine::Compare(CDrawObj* pObj)
{
    CLine* pLine = (CLine*)pObj;
    if (m_ptBeg != pLine->m_ptBeg)
        return FALSE;
    if (m_ptEnd != pLine->m_ptEnd)
        return FALSE;
    return TRUE;
}
#endif      // GPLAY

//DFM19991214
void CLine::CopyAttributes(CLine *source)
{
    CDrawObj::CopyAttributes(source);

    m_ptBeg = source->m_ptBeg;
    m_ptEnd = source->m_ptEnd;

    m_crLine = source->m_crLine;
    m_nLineWidth = source->m_nLineWidth;
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

void CBitmapImage::Draw(CDC* pDC, TileScale eScale)
{
    g_gt.mTileDC.SelectObject(&m_bitmap);
    SetupPalette(&g_gt.mTileDC);

    CPoint pnt = m_rctExtent.TopLeft();

    if ((GetDObjFlags() & dobjFlgLayerNatural) == 0)
    {
        pDC->SetStretchBltMode(COLORONCOLOR);
        pDC->StretchBlt(pnt.x, pnt.y, m_rctExtent.Width(),  m_rctExtent.Height(), &g_gt.mTileDC,
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

        CSize sizeWorld = pDC->GetWindowExt();
        CSize sizeView = pDC->GetViewportExt();
        CPoint pnt = m_rctExtent.TopLeft();
        ScalePoint(pnt, sizeView, sizeWorld);

        SynchronizeExtentRect(sizeWorld, sizeView); // Only time we can find true scale

        pDC->SaveDC();
        pDC->SetMapMode(MM_TEXT);
        pDC->BitBlt(pnt.x, pnt.y, bmInfo.bmWidth, bmInfo.bmHeight,
                    &g_gt.mTileDC, 0, 0, SRCCOPY);
        pDC->RestoreDC(-1);
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
void CBitmapImage::ForceIntoZone(CRect* pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
        m_rctExtent += pntOffset;
}
#endif

#ifndef GPLAY
CSelection* CBitmapImage::CreateSelectProxy(CBrdEditView* pView)
{
    return new CSelGeneric(pView, this);
}
#endif

//DFM19991221

void CBitmapImage::OffsetObject(CPoint offset)
{
    m_rctExtent += offset;
}

CDrawObj* CBitmapImage::Clone()
{
    CBitmapImage* pObj = new CBitmapImage;
    pObj->CopyAttributes(this);
    return pObj;
}

void CBitmapImage::CopyAttributes(CBitmapImage *source)
{
    CDrawObj::CopyAttributes(source);

    m_eBaseScale = source->m_eBaseScale;
    CDib dib;
    dib.BitmapToDIB(&source->m_bitmap, GetAppPalette());
    m_bitmap.Attach(dib.DIBToBitmap(GetAppPalette())->Detach());
}

void CBitmapImage::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_eBaseScale;

        CDib dib;
        dib.BitmapToDIB(&m_bitmap, GetAppPalette());
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
        if (dib.m_hDib != NULL)
        {
            CBitmap* pBMap = dib.DIBToBitmap(GetAppPalette());
            ASSERT(pBMap != NULL);
            m_bitmap.Attach(pBMap->Detach());
            delete pBMap;
        }
    }
}

//////////////////////////////////////////////////////////////////
// Class CTileImage

void CTileImage::Draw(CDC* pDC, TileScale eScale)
{
    ASSERT(m_pTMgr != NULL);
    CTile tile;
    m_pTMgr->GetTile(m_tid, &tile, eScale);

    CPoint pnt = m_rctExtent.TopLeft();
    if (eScale == halfScale)
    {
        ScalePoint(pnt, pDC->GetViewportExt(), pDC->GetWindowExt());
        pDC->SaveDC();
        pDC->SetMapMode(MM_TEXT);
    }
    tile.TransBlt(pDC, pnt.x, pnt.y);
    if (eScale == halfScale)
        pDC->RestoreDC(-1);
}

void CTileImage::SetTile(int x, int y, TileID tid)
{
    ASSERT(m_pTMgr != NULL);
    CTile tile;
    m_pTMgr->GetTile(tid, &tile);
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
void CTileImage::ForceIntoZone(CRect* pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
        m_rctExtent += pntOffset;
}
#endif

#ifndef GPLAY
CSelection* CTileImage::CreateSelectProxy(CBrdEditView* pView)
{
    return new CSelGeneric(pView, this);
}
#endif

void CTileImage::OffsetObject(CPoint offset)
{
    m_rctExtent += offset;
}

CDrawObj* CTileImage::Clone()
{
    CTileImage* pObj = new CTileImage;
    pObj->CopyAttributes(this);
    return pObj;
}

void CTileImage::CopyAttributes(CTileImage *source)
{
    CDrawObj::CopyAttributes(source);
    m_tid = source->m_tid;
    m_pTMgr = source->m_pTMgr;
}

void CTileImage::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_tid;
    }
    else
    {
        WORD wTmp;
        m_pTMgr = ((CGamDoc*)ar.m_pDocument)->GetTileManager();
        ar >> wTmp; m_tid = (TileID)wTmp;
    }
}

//////////////////////////////////////////////////////////////////
// Class CText

CText::~CText()
{
    CGamDoc::GetFontManager()->DeleteFont(m_fontID);
}

void CText::Draw(CDC* pDC, TileScale eScale)
{
    if (eScale == smallScale && m_rctExtent.Height() < 16)
        return;

    HFONT hFont = CGamDoc::GetFontManager()->GetFontHandle(m_fontID);
    CFont* pPrvFont = pDC->SelectObject(CFont::FromHandle(hFont));
    pDC->SetBkMode(TRANSPARENT);
    COLORREF crPrev = pDC->SetTextColor(m_crText);
    pDC->ExtTextOut(m_rctExtent.left, m_rctExtent.top,
        0, NULL, m_text, m_text.GetLength(), NULL);
    pDC->SetTextColor(crPrev);
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
    pFontMgr->AddFont(fid);             // Incr font usage count

    // Discard previous font ID
    if (m_fontID != 0)
        pFontMgr->DeleteFont(m_fontID);

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
void CText::ForceIntoZone(CRect* pRctZone)
{
    CPoint pntOffset;
    if (IsExtentOutOfZone(pRctZone, pntOffset))
        m_rctExtent += pntOffset;
}
#endif

void CText::OffsetObject(CPoint offset)
{
    m_rctExtent += offset;
}

#ifndef GPLAY
CSelection* CText::CreateSelectProxy(CBrdEditView* pView)
{
    return new CSelGeneric(pView, this);
}
#endif

CDrawObj* CText::Clone()
{
    CText* pObj = new CText;
    pObj->CopyAttributes(this);
    return pObj;
}

void CText::CopyAttributes(CText *source)
{
    CFontTbl* pFontMgr = CGamDoc::GetFontManager();

    CDrawObj::CopyAttributes(source);

    m_nAngle = source->m_nAngle;
    m_crText = source->m_crText;
    m_text = source->m_text;

    if (m_fontID != 0) pFontMgr->DeleteFont(m_fontID);
    m_fontID = source->m_fontID;
    pFontMgr->AddFont(m_fontID);            // make sure font reference count is in sync
}

void CText::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    CFontTbl* pFontMgr = CGamDoc::GetFontManager();
    if (ar.IsStoring())
    {
        ar << (WORD)m_nAngle;
        ar << (DWORD)m_crText;
        ar << m_text;
        pFontMgr->Archive(ar, m_fontID);
    }
    else
    {
        WORD wTmp;
        DWORD dwTmp;
        if (m_fontID != 0) pFontMgr->DeleteFont(m_fontID);
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

static void DrawObjTile(CDC* pDC, CPoint pnt, CTileManager* pTMgr, TileID tid,
    TileScale eScale)
{
    CTile tile;
    pTMgr->GetTile(tid,  &tile, eScale);

    if (eScale == halfScale)
    {
        ScalePoint(pnt, pDC->GetViewportExt(), pDC->GetWindowExt());
        pDC->SaveDC();
        pDC->SetMapMode(MM_TEXT);
    }
    tile.TransBlt(pDC, pnt.x, pnt.y);
    if (eScale == halfScale)
        pDC->RestoreDC(-1);
}

//////////////////////////////////////////////////////////////////
// Class CPieceObj

void CPieceObj::Draw(CDC* pDC, TileScale eScale)
{
    ASSERT(m_pDoc != NULL);
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);

    TileID tid;

    if (!m_pDoc->IsScenario() &&
            pPTbl->IsOwnedButNotByCurrentPlayer(m_pid, m_pDoc))
        tid = pPTbl->GetFrontTileID(m_pid, TRUE);
    else
        tid = pPTbl->GetActiveTileID(m_pid, TRUE);  // Show rotations
    ASSERT(tid != nullTid);

    CPoint pnt = m_rctExtent.TopLeft();
    DrawObjTile(pDC, pnt, pTMgr, tid, eScale);
}

void CPieceObj::SetOwnerMask(DWORD dwMask)
{
    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);
    pPTbl->SetOwnerMask(m_pid, dwMask);
}

BOOL CPieceObj::IsOwned()
{
    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);
    return pPTbl->IsPieceOwned(m_pid);
}

BOOL CPieceObj::IsOwnedBy(DWORD dwMask)
{
    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);
    return pPTbl->IsPieceOwnedBy(m_pid, dwMask);
}

BOOL CPieceObj::IsOwnedButNotByCurrentPlayer()
{
    ASSERT(m_pDoc != NULL);
    CPieceTable* pPTbl = m_pDoc->GetPieceTable();
    ASSERT(pPTbl != NULL);
    return pPTbl->IsOwnedButNotByCurrentPlayer(m_pid, m_pDoc);
}

void CPieceObj::SetPiece(CRect& rct, PieceID pid)
{
    m_pid = pid;
    m_rctExtent = rct;
    ResyncExtentRect();
}

void CPieceObj::ResyncExtentRect()
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

    CTile tile;
    pTMgr->GetTile(tid, &tile);
    CPoint pnt = m_rctExtent.CenterPoint();
    pnt.x -= tile.GetWidth() / 2;
    pnt.y -= tile.GetHeight() / 2;
    m_rctExtent = CRect(pnt, tile.GetSize());
}

BOOL CPieceObj::HitTest(CPoint pt)
{
    CRect rct = GetEnclosingRect();
    return rct.PtInRect(pt);
}

CSelection* CPieceObj::CreateSelectProxy(CPlayBoardView* pView)
{
    return new CSelGeneric(pView, this);
}

CDrawObj* CPieceObj::Clone(CGamDoc* pDoc)
{
    CPieceObj* pObj = new CPieceObj;
    pObj->m_rctExtent = m_rctExtent;        // From base class
    pObj->m_dwDObjFlags = m_dwDObjFlags;    // From base class
    pObj->m_pDoc = pDoc;
    pObj->m_pid = m_pid;
    return pObj;
}

BOOL CPieceObj::Compare(CDrawObj* pObj)
{
    CPieceObj* pPce = (CPieceObj*)pObj;
    if (m_pid != pPce->m_pid)
        return FALSE;
    if (m_rctExtent != pPce->m_rctExtent)
        return FALSE;
    return TRUE;
}

void CPieceObj::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << (WORD)m_pid;
    }
    else
    {
        WORD wTmp;
        m_pDoc = (CGamDoc*)ar.m_pDocument;
        ar >> wTmp; m_pid = (PieceID)wTmp;
    }
}

//////////////////////////////////////////////////////////////////
// Class CMarkObj

void CMarkObj::Draw(CDC* pDC, TileScale eScale)
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

    MarkDef* pMark = pMMgr->GetMark(m_mid);
    ASSERT(pMark->m_tid != nullTid);

    if (m_nFacingDegCW != 0)
    {
        // Handle rotated markers...
        ElementState state = MakeMarkerState(m_mid, (WORD)m_nFacingDegCW);
        CTileFacingMap* pMapFacing = m_pDoc->GetFacingMap();
        TileID tidFacing = pMapFacing->GetFacingTileID(state);
        if (tidFacing == nullTid)
            tidFacing = pMapFacing->CreateFacingTileID(state, pMark->m_tid);
        return tidFacing;
    }
    else
        return pMark->m_tid;
}

void CMarkObj::ResyncExtentRect()
{
    CTileManager* pTMgr = m_pDoc->GetTileManager();
    ASSERT(pTMgr != NULL);

    TileID tid = GetCurrentTileID();
    ASSERT(tid != nullTid);

    CTile tile;
    pTMgr->GetTile(tid, &tile);
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

CSelection* CMarkObj::CreateSelectProxy(CPlayBoardView* pView)
{
    return new CSelGeneric(pView, this);
}

CDrawObj* CMarkObj::Clone(CGamDoc* pDoc)
{
    CMarkObj* pObj = new CMarkObj;
    pObj->m_rctExtent = m_rctExtent;        // From base class
    pObj->m_dwDObjFlags = m_dwDObjFlags;    // From base class
    pObj->m_pDoc = pDoc;
    pObj->m_mid = m_mid;
    pObj->m_nFacingDegCW = m_nFacingDegCW;
    pObj->m_dwObjectID = m_dwObjectID;
    return pObj;
}

BOOL CMarkObj::Compare(CDrawObj* pObj)
{
    CMarkObj* pMrk = (CMarkObj*)pObj;
    if (m_mid != pMrk->m_mid)
        return FALSE;
    if (m_dwObjectID != pMrk->m_dwObjectID)
        return FALSE;
    if (m_nFacingDegCW != pMrk->m_nFacingDegCW)
        return FALSE;
    if (m_rctExtent != pMrk->m_rctExtent)
        return FALSE;
    return TRUE;
}

void CMarkObj::Serialize(CArchive& ar)
{
    CDrawObj::Serialize(ar);
    if (ar.IsStoring())
    {
        ar << m_dwObjectID;
        ar << (WORD)m_mid;
        ar << (WORD)m_nFacingDegCW;                                 //Ver2.0
    }
    else
    {
        WORD wTmp;
        m_pDoc = ((CGamDoc*)ar.m_pDocument);
        ar >> m_dwObjectID;
        ar >> wTmp; m_mid = (MarkID)wTmp;
        if (CGamDoc::GetLoadingVersion() >= NumVersion(2, 0))       //Ver2.0
        {
            ar >> wTmp;
            m_nFacingDegCW = wTmp;
            if (CGamDoc::GetLoadingVersion() < NumVersion(2, 90))   //Ver2.90
                m_nFacingDegCW *= 5;                                // Convert old value to degrees
        }
        else
            m_nFacingDegCW = 0;
    }
}

//////////////////////////////////////////////////////////////////
// Class CLineObj

CDrawObj* CLineObj::Clone(CGamDoc* pDoc)
{
    CLineObj* pObj = new CLineObj;
    pObj->m_rctExtent = m_rctExtent;        // From base class
    pObj->m_dwDObjFlags = m_dwDObjFlags;    // From base class
    pObj->m_ptBeg = m_ptBeg;                // CLine class
    pObj->m_ptEnd = m_ptEnd;
    pObj->m_crLine = m_crLine;
    pObj->m_nLineWidth = m_nLineWidth;
    pObj->m_dwObjectID = m_dwObjectID;      // From this class
    return pObj;
}

BOOL CLineObj::Compare(CDrawObj* pObj)
{
    CLineObj* pLine = (CLineObj*)pObj;
    if (m_dwObjectID != pLine->m_dwObjectID)
        return FALSE;
    if (m_ptBeg != pLine->m_ptBeg)
        return FALSE;
    if (m_ptEnd != pLine->m_ptEnd)
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

void CDrawList::Draw(CDC* pDC, CRect* pDrawRct, TileScale eScale,
    BOOL bApplyVisibility /* = TRUE */, BOOL bDrawPass2Objects /* = FALSE */,
    BOOL bHideUnlocked /* = FALSE */, BOOL bDrawLockedFirst /* = FALSE */)
{
    POSITION pos;
    // We might need to draw locked objects first so they
    // show up under other object which are unlocked. This is
    // only valid in a non-pass 2 call.
    if (!bDrawPass2Objects && bDrawLockedFirst)
    {
        for (pos = GetHeadPosition(); pos != NULL; )
        {
            CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
            ASSERT(pDObj != NULL);

            // Check if the object should be drawn in this scaling
            if (bApplyVisibility && ((pDObj->GetDObjFlags() & eScale) == 0))
                continue;                   // Doesn't qualify

            ASSERT(!((pDObj->GetDObjFlags() & dobjFlgLockDown) != 0 &&
                (pDObj->GetDObjFlags() & dobjFlgDrawPass2) != 0));// We'll never have a locked pass 2 object

            if ((pDObj->GetDObjFlags() & dobjFlgLockDown) == 0)
                continue;                  // Only process locked objects

            if (pDObj->IsVisible(pDrawRct))
                pDObj->Draw(pDC, eScale);
        }
    }

    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        ASSERT(pDObj != NULL);

        // Check if the object should be drawn in this scaling
        if (bApplyVisibility && ((pDObj->GetDObjFlags() & eScale) == 0))
            continue;                   // Doesn't qualify

        // Check if unlocked objects should be hidden
        if (bHideUnlocked && ((pDObj->GetDObjFlags() & dobjFlgLockDown) == 0))
            continue;                   // Doesn't qualify

        // Only draw objects in their proper passes
        if (bDrawLockedFirst && (pDObj->GetDObjFlags() & dobjFlgLockDown) != 0 ||
            bDrawPass2Objects && (pDObj->GetDObjFlags() & dobjFlgDrawPass2) == 0 ||
           !bDrawPass2Objects && (pDObj->GetDObjFlags() & dobjFlgDrawPass2) != 0)
        {
            continue;                   // Doesn't qualify
        }

        if (pDObj->IsVisible(pDrawRct))
            pDObj->Draw(pDC, eScale);
    }
}

CDrawObj* CDrawList::HitTest(CPoint pt, TileScale eScale,
    BOOL bApplyVisibility /* = TRUE */)
{
    POSITION pos;
    for (pos = GetTailPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetPrev(pos);
        ASSERT(pDObj != NULL);

        if (bApplyVisibility && ((pDObj->GetDObjFlags() & eScale) == 0))
            continue;                   // Doesn't qualify

        if (pDObj->HitTest(pt))
            return pDObj;
    }
    return NULL;
}

void CDrawList::DrillDownHitTest(CPoint point, CPtrList* selLst,
    TileScale eScale, BOOL bApplyVisibility /* = TRUE */)
{
    POSITION pos;
    for (pos = GetTailPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetPrev(pos);
        ASSERT(pDObj != NULL);

        if (bApplyVisibility && ((pDObj->GetDObjFlags() & eScale) == 0))
            continue;                   // Doesn't qualify
        if (pDObj->HitTest(point))
            selLst->AddTail(pDObj);
    }
}

void CDrawList::ArrangeObjectListInDrawOrder(CPtrList* pLst)
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    CPtrList tmpLst;

    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        if (pLst->Find(pDObj) != NULL)
            tmpLst.AddTail(pDObj);
    }
    pLst->RemoveAll();
    pLst->AddTail(&tmpLst);
}

void CDrawList::ArrangeObjectListInVisualOrder(CPtrList* pLst)
{
    // Loop backwards through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    CPtrList tmpLst;

    POSITION pos;
    for (pos = GetTailPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetPrev(pos);
        if (pLst->Find(pDObj) != NULL)
            tmpLst.AddTail(pDObj);
    }
    pLst->RemoveAll();
    pLst->AddTail(&tmpLst);
}

#ifdef GPLAY

void CDrawList::ArrangePieceTableInDrawOrder(CWordArray* pTbl)
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    CWordArray tmpTbl;

    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        if (pDObj->GetType() == CDrawObj::drawPieceObj)
        {
            for (int i = 0; i < pTbl->GetSize(); i++)
            {
                if ((PieceID)pTbl->GetAt(i)== ((CPieceObj*)pDObj)->m_pid)
                {
                    tmpTbl.Add(((CPieceObj*)pDObj)->m_pid);
                    break;
                }
            }
        }
    }
    ASSERT(tmpTbl.GetSize() == pTbl->GetSize());
    pTbl->RemoveAll();
    pTbl->InsertAt(0, &tmpTbl);
}

void CDrawList::ArrangePieceTableInVisualOrder(CWordArray* pTbl)
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    CWordArray tmpTbl;

    POSITION pos;
    for (pos = GetTailPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetPrev(pos);
        if (pDObj->GetType() == CDrawObj::drawPieceObj)
        {
            for (int i = 0; i < pTbl->GetSize(); i++)
            {
                if ((PieceID)pTbl->GetAt(i)== ((CPieceObj*)pDObj)->m_pid)
                {
                    tmpTbl.Add(((CPieceObj*)pDObj)->m_pid);
                    break;
                }
            }
        }
    }
    ASSERT(tmpTbl.GetSize() == pTbl->GetSize());
    pTbl->RemoveAll();
    pTbl->InsertAt(0, &tmpTbl);
}

void CDrawList::ArrangeObjectPtrTableInDrawOrder(CPtrArray* pTbl)
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    CPtrArray tmpTbl;

    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        for (int i = 0; i < pTbl->GetSize(); i++)
        {
            if (pTbl->GetAt(i) == pDObj)
            {
                tmpTbl.Add(pDObj);
                break;
            }
        }
    }
    ASSERT(tmpTbl.GetSize() == pTbl->GetSize());
    pTbl->RemoveAll();
    pTbl->Copy(tmpTbl);
}

void CDrawList::ArrangeObjectPtrTableInVisualOrder(CPtrArray* pTbl)
{
    // Loop through the drawing list looking for objects that are
    // selected. Add them to a local list. When done, purge the caller's
    // list and transfer temp list to the callers list.
    CPtrArray tmpTbl;

    POSITION pos;
    for (pos = GetTailPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetPrev(pos);
        for (int i = 0; i < pTbl->GetSize(); i++)
        {
            if (pTbl->GetAt(i) == pDObj)
            {
                tmpTbl.Add(pDObj);
                break;
            }
        }
    }
    ASSERT(tmpTbl.GetSize() == pTbl->GetSize());
    pTbl->RemoveAll();
    pTbl->Copy(tmpTbl);
}

#endif // GPLAY

void CDrawList::RemoveObjectsInList(CPtrList* pLst)
{
    POSITION pos;
    for (pos = pLst->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)pLst->GetNext(pos);
        POSITION pos = Find(pDObj);
        if (pos != NULL)
            RemoveAt(pos);
    }
}

void CDrawList::Flush(void)
{
    while (!IsEmpty())
        delete (CDrawObj*)RemoveHead();
}

void CDrawList::RemoveObject(CDrawObj* pDrawObj)
{
    POSITION pos = Find(pDrawObj);
    if (pos != NULL)
        RemoveAt(pos);
}

#ifdef GPLAY

void CDrawList::SetOwnerMasks(DWORD dwOwnerMask)
{
    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CPieceObj* pObj = (CPieceObj*)GetNext(pos);
        ASSERT(pObj != NULL);
        if (pObj->GetType() == CDrawObj::drawPieceObj)
            pObj->SetOwnerMask(dwOwnerMask);
    }
}

CPieceObj* CDrawList::FindPieceID(PieceID pid)
{
    CPieceObj* pObj = (CPieceObj*)FindObjectID((DWORD)pid);
    if (pObj != NULL)
    {
        ASSERT(pObj->GetType() == CDrawObj::drawPieceObj);
        return (CPieceObj*)pObj;
    }
    return NULL;
}

CDrawObj* CDrawList::FindObjectID(DWORD oid)
{
    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        ASSERT(pDObj != NULL);
        if (pDObj->GetObjectID() == oid)
             return pDObj;
    }
    return NULL;
}

void CDrawList::GetPieceObjectPtrList(CPtrList* pLst)
{
    pLst->RemoveAll();
    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        ASSERT(pDObj != NULL);
        if (pDObj->GetType() == CDrawObj::drawPieceObj)
            pLst->AddTail(pDObj);
    }
}

void CDrawList::GetPieceIDTable(CWordArray *pTbl)
{
    pTbl->RemoveAll();
    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        ASSERT(pDObj != NULL);
        if (pDObj->GetType() == CDrawObj::drawPieceObj)
            pTbl->Add(((CPieceObj*)pDObj)->m_pid);
    }
}

void CDrawList::GetObjectListFromPieceIDTable(CWordArray* pTbl, CPtrList* pLst)
{
    pLst->RemoveAll();
    for (int i = 0; i < pTbl->GetSize(); i++)
    {
        CPieceObj* pObj = FindPieceID((PieceID)pTbl->GetAt(i));
        ASSERT(pObj != NULL);
        if (pObj)
            pLst->AddTail(pObj);
    }
}

void CDrawList::GetPieceIDTableFromObjList(CPtrList* pLst, CWordArray* pTbl,
    BOOL bDeleteObjs /* = FALSE*/)
{
    pTbl->RemoveAll();
    POSITION pos;
    for (pos = pLst->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)pLst->GetNext(pos);
        ASSERT(pDObj != NULL);
        if (pDObj->GetType() == CDrawObj::drawPieceObj)
            pTbl->Add(((CPieceObj*)pDObj)->m_pid);
        if (bDeleteObjs) delete pDObj;
    }
}

BOOL CDrawList::HasMarker()
{
    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        ASSERT(pDObj != NULL);
        if (pDObj->GetType() == CDrawObj::drawMarkObj)
            return TRUE;
    }
    return FALSE;
}

#endif // GPLAY

#ifndef GPLAY

BOOL CDrawList::PurgeMissingTileIDs(CTileManager* pTMgr)
{
    POSITION pos1, pos2;
    BOOL bPurged = FALSE;
    for (pos1 = GetHeadPosition(); (pos2 = pos1) != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos1);
        ASSERT(pDObj != NULL);
        if (pDObj->GetType() == CDrawObj::drawTile)
        {
            if (!pTMgr->IsTileIDValid(((CTileImage*)pDObj)->m_tid))
            {
                GetAt(pos2);
                RemoveAt(pos2);
                delete pDObj;
                bPurged = TRUE;
            }
        }
    }
    return bPurged;
}

BOOL CDrawList::IsTileInUse(TileID tid)
{
    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        ASSERT(pDObj != NULL);
        if (pDObj->GetType() == CDrawObj::drawTile)
        {
            if (((CTileImage*)pDObj)->m_tid == tid)
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

void CDrawList::ForceIntoZone(CRect* pRctZone)
{
    POSITION pos;
    for (pos = GetTailPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetPrev(pos);
        ASSERT(pDObj != NULL);
        pDObj->ForceIntoZone(pRctZone);
    }
}

#endif  // !GPLAY

#ifdef GPLAY
CDrawList* CDrawList::Clone(CGamDoc* pDoc)
{
    CDrawList* pLst = new CDrawList;
    POSITION pos;
    for (pos = GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
        pLst->AddTail(pDObj->Clone(pDoc));
    }
    return pLst;
}

void CDrawList::Restore(CGamDoc* pDoc, CDrawList* pLst)
{
    Flush();
    POSITION pos;
    for (pos = pLst->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)pLst->GetNext(pos);
        AddTail(pDObj->Clone(pDoc));    // Clone it back
    }
}

BOOL CDrawList::Compare(CDrawList* pLst)
{
    if (GetCount() != pLst->GetCount())
        return FALSE;

    POSITION pos1 = GetHeadPosition();
    POSITION pos2 = pLst->GetHeadPosition();

    while (pos1 != NULL && pos2 != NULL)
    {
        CDrawObj* pDObj1 = (CDrawObj*)GetNext(pos1);
        CDrawObj* pDObj2 = (CDrawObj*)pLst->GetNext(pos2);
        if (pDObj1->GetType() != pDObj2->GetType())
            return FALSE;
        if (!pDObj1->Compare(pDObj2))
            return FALSE;
    }
    return TRUE;
}

void CDrawList::AppendWithOffset(CDrawList* pSourceLst, CPoint pntOffet)
{
    POSITION pos;
    for (pos = pSourceLst->GetHeadPosition(); pos != NULL; )
    {
        CDrawObj* pDObj = (CDrawObj*)pSourceLst->GetNext(pos);
        CDrawObj* pObjClone = pDObj->Clone();
        pObjClone->OffsetObject(pntOffet);
        AddTail(pObjClone);
    }
}

#endif // GPLAY

void CDrawList::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        ar << (WORD)GetCount();
#ifdef _DEBUG
        int nObjects = GetCount();
#endif
        POSITION pos;
        for (pos = GetHeadPosition(); pos != NULL; )
        {
            CDrawObj* pDObj = (CDrawObj*)GetNext(pos);
            ASSERT(pDObj != NULL);
            ASSERT(pDObj->GetType() < CDrawObj::drawUnknown);
            ar << (WORD)pDObj->GetType();
            pDObj->Serialize(ar);
#ifdef _DEBUG
            nObjects--;
#endif
        }
        ASSERT(nObjects == 0);
    }
    else
    {
        Flush();
        WORD wCount;
        ar >> wCount;
        for (WORD i = 0; i < wCount; i++)
        {
            CDrawObj* pDObj = NULL;
            WORD wType;
            ar >> wType;
            switch ((CDrawObj::CDrawObjType)wType)
            {
                case CDrawObj::drawRect:
                    pDObj = new CRectObj;
                    break;
                case CDrawObj::drawEllipse:
                    pDObj = new CEllipse;
                    break;
                case CDrawObj::drawLine:
                    pDObj = new CLine;
                    break;
                case CDrawObj::drawTile:
                    pDObj = new CTileImage;
                    break;
                case CDrawObj::drawText:
                    pDObj = new CText;
                    break;
                case CDrawObj::drawPolygon:
                    pDObj = new CPolyObj;
                    break;
                case CDrawObj::drawBitmap:
                    pDObj = new CBitmapImage;
                    break;
#ifdef GPLAY
                case CDrawObj::drawPieceObj:
                    pDObj = new CPieceObj;
                    break;
                case CDrawObj::drawMarkObj:
                    pDObj = new CMarkObj;
                    break;
                case CLineObj::drawLineObj:
                    pDObj = new CLineObj;
                    break;
#endif
                default:
                    ASSERT(FALSE);
                    AfxThrowArchiveException(CArchiveException::badClass);
            }
            ASSERT(pDObj != NULL);
            pDObj->Serialize(ar);
            AddTail(pDObj);
        }
    }
}


