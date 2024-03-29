// Rotate.cpp
//
// Copyright (c) 1994-2023 By Dale L. Larson & William Su, All Rights Reserved.
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
#include    <math.h>
#include    <limits.h>
#include    "GdiTools.h"
#include    "GMisc.h"
#include    "CDib.h"

/////////////////////////////////////////////////////////////////////

const   int numPnts = 4;            // Only rects in rot code

/////////////////////////////////////////////////////////////////////

struct Stepper
{
    int     m_nVal;
    int     m_nRem;
    BOOL    m_bNeg;
    div_t   m_step;
    int     m_nSteps;

    Stepper() {}
    Stepper(int nStart, int nEnd, int nSteps);
    void Setup(int nStart, int nEnd, int nSteps);
    void Step();
    int RoundedVal();
    operator int () { return m_nVal; }
    void operator ++(int) { Step(); }
};

inline Stepper::Stepper(int nStart, int nEnd, int nSteps)
    { Setup(nStart, nEnd, nSteps); }

void Stepper::Setup(int nStart, int nEnd, int nSteps)
{
    m_nSteps = nSteps;
    m_nVal = nStart;
    m_nRem = 0;
    int nDiff = nEnd - nStart;
    if ((m_bNeg = nDiff < 0) != false)
        nDiff = -nDiff;
    m_step = nSteps > 0 ? div(nDiff, nSteps) : div(0,1);
}

void Stepper::Step()
{
    m_nVal += m_bNeg ? -m_step.quot : m_step.quot;
    m_nRem += m_step.rem;
    if (m_nRem >= m_nSteps)
    {
        m_nVal += m_bNeg ? -1 : 1;
        m_nRem -= m_nSteps;
    }
}

inline int Stepper::RoundedVal()
{
    if (m_nSteps > 0 && (m_nRem << 1) >= m_nSteps)
        return m_bNeg ? m_nVal - 1 : m_nVal + 1;
    else
        return m_nVal;
}

/////////////////////////////////////////////////////////////////////

struct CDib::ImgEdge
{
    POINT*      m_pSPnts;               // Source rect corner points
    POINT*      m_pDPnts;               // Rotated rect corner points
    int         m_nCurPnt;              // Current 'far' point index
    int         m_nBotPnt;              // Point at bottom of dest image
    int         m_nPntIncr;             // Point index increment (-1, 1)
    int         m_nLines;               // Number of scanlines left on edge

    Stepper     m_srcX;
    Stepper     m_srcY;
    Stepper     m_dstX;

    // ------- //
    ImgEdge(BOOL bLeftSide, POINT* pSrcPnts, POINT* pDstPnts,
        int nTopPnt, int nBotPnt);

    BOOL SetupNextEdge();
    BOOL NextScanLine();
};

/////////////////////////////////////////////////////////////////////

static CSize CalcRotatedRect(CSize size, int angle, POINT* pSPnts, POINT* pDPnts);
static void RotatePoint(POINT& pt, int nSin, int nCos);
namespace {
    void RotatePoints(POINT* pPnts, int nPnts, int nDegrees);
}

/////////////////////////////////////////////////////////////////////

    // angle is clockwise
    CDib CDib::RotateFast(int angle) const
    {
        CDib retval;
        switch (angle)
        {
            case 90:
                retval.m_wximg = m_wximg.Rotate90(true);
                break;
            case 180:
                retval.m_wximg = m_wximg.Rotate180();
                break;
            case 270:
                retval.m_wximg = m_wximg.Rotate90(false);
                break;
            default:
                AfxThrowInvalidArgException();
        }
        return retval;
    }

// angle is clockwise
CDib CDib::Rotate(int angle, COLORREF crTrans) const
{
DBGREL_CPP20_TRACE("{}({} x {}, {}deg)\n", __func__, Width(), Height(), angle);
    ASSERT(0 <= angle && angle < 360);
    ASSERT(angle != 0 || !"unnecessary call");
    if (angle % 90 == 0)
    {
        return RotateFast(angle);
    }
    POINT   pntSrc[numPnts];
    POINT   pntDst[numPnts];

    // There was a long standing bug in the DIB XY addressing (through V2.x)
    // that caused the rotation to be computed opposite of it's actual
    // angle. Since the addressing is fixed we need to reverse the
    // angle to maintain backward compatibility with existing games.
    angle = 360 - angle;

    CSize sizeSrc(Width(), Height());
    CSize sizeDst = CalcRotatedRect(sizeSrc, angle, pntSrc, pntDst);
    CDib pDDib = CreateTransparentColorDIB(sizeDst, crTrans);

    // Find top and bottom point indexes
    int nTopPnt = 0;
    int nBotPnt = 0;
    for (int i = 0; i < numPnts; i++)
    {
        if (pntDst[i].y == 0)
            nTopPnt = i;
        if (pntDst[i].y == sizeDst.cy - 1)
            nBotPnt = i;
    }

    ImgEdge lftEdge(TRUE,  pntSrc, pntDst, nTopPnt, nBotPnt);
    lftEdge.SetupNextEdge();
    ImgEdge rgtEdge(FALSE, pntSrc, pntDst, nTopPnt, nBotPnt);
    rgtEdge.SetupNextEdge();

    int yCur = 0;

    while (1)
    {
        DrawScanLine(lftEdge, rgtEdge, yCur, pDDib);
        if (!lftEdge.NextScanLine())
            break;
        if (!rgtEdge.NextScanLine())
            break;
        yCur++;
    }
    return pDDib;
}

/////////////////////////////////////////////////////////////////////

void CDib::DrawScanLine(ImgEdge& lftEdge, ImgEdge& rgtEdge, int dstY,
    CDib& pDDib) const
{
    wxImagePixelData srcData(const_cast<wxImage&>(m_wximg));
    ASSERT(srcData);
    wxImagePixelData::Iterator src(srcData);
    int dstX = lftEdge.m_dstX;
    int dstXMax = rgtEdge.m_dstX;
    wxImagePixelData destData(pDDib.m_wximg);
    ASSERT(destData);
    wxImagePixelData::Iterator dest(destData);
    dest.MoveTo(destData, dstX, dstY);

    int dstWd = dstXMax - dstX;

    Stepper srcX(lftEdge.m_srcX, rgtEdge.m_srcX, dstWd);
    Stepper srcY(lftEdge.m_srcY, rgtEdge.m_srcY, dstWd);

    //  TRACE2("For Y = %d, Dest Width = %d\n", dstY, dstWd + 1);
    for (; dstX <= dstXMax; dstX++)
    {
        src.MoveTo(srcData, srcX.RoundedVal(),
            srcY.RoundedVal());

        dest.Red() = src.Red();
        dest.Green() = src.Green();
        dest.Blue() = src.Blue();
        ++dest;
        // TEST CODE:
        //        char str[256];
        //        sprintf(str, "dX=%d, dY=%d : sX=%d, sY=%d\n",
        //          dstX, dstY,
        //          srcX.RoundedVal(), srcY.RoundedVal());
        //        TRACE(str);
        srcX++;
        srcY++;
    }
}

/////////////////////////////////////////////////////////////////////

CDib CDib::CreateTransparentColorDIB(CSize size, COLORREF crTrans)
{
    CDib pDib(size.cx, size.cy);
    pDib.Fill(crTrans);

    return pDib;
}

/////////////////////////////////////////////////////////////////////

static void RotateRectVector(POINT* pPnts, int angle)
{
    RotatePoints(pPnts, 4, angle);
}

/////////////////////////////////////////////////////////////////////

static void LoadRectVector(CSize size, POINT* pPnts)
{
    pPnts[0].x = pPnts[3].x = 0;
    pPnts[1].x = pPnts[2].x = size.cx - 1;
    pPnts[0].y = pPnts[1].y = 0;
    pPnts[2].y = pPnts[3].y = size.cy - 1;
}

/////////////////////////////////////////////////////////////////////

static void OffsetRectVector(POINT* pPnts, int xOff, int yOff)
{
    for (int i = 0; i < numPnts; i++)
    {
        pPnts[i].x += xOff;
        pPnts[i].y += yOff;
    }
}

/////////////////////////////////////////////////////////////////////

static void CopyRectVector(POINT* pSrcPnts, POINT* pDstPnts)
{
    for (int i = 0; i < numPnts; i++)
        pDstPnts[i] = pSrcPnts[i];
}

/////////////////////////////////////////////////////////////////////

static void FindRectVectorMinMax(POINT* pPnts, CPoint& pntMin, CPoint& pntMax)
{
    pntMin.x = INT_MAX;
    pntMax.x = INT_MIN;
    pntMin.y = INT_MAX;
    pntMax.y = INT_MIN;
    for (int i = 0; i < numPnts; i++)
    {
        pntMin.x = CB::min(pntMin.x, pPnts[i].x);
        pntMin.y = CB::min(pntMin.y, pPnts[i].y);
        pntMax.x = CB::max(pntMax.x, pPnts[i].x);
        pntMax.y = CB::max(pntMax.y, pPnts[i].y);
    }
}

/////////////////////////////////////////////////////////////////////

static CSize CalcRotatedRect(CSize size, int angle, POINT* pSPnts, POINT* pDPnts)
{
    LoadRectVector(size, pSPnts);
    CopyRectVector(pSPnts, pDPnts);
    int xHalf = size.cx / 2;
    int yHalf = size.cy / 2;
    OffsetRectVector(pDPnts, -xHalf, -yHalf);
    RotateRectVector(pDPnts, angle);

    CPoint pntMin;
    CPoint pntMax;
    FindRectVectorMinMax(pDPnts, pntMin, pntMax);

    // Shove points back into the +x,+y quadrant
    OffsetRectVector(pDPnts, -pntMin.x, -pntMin.y);

    return CSize(pntMax.x - pntMin.x + 1, pntMax.y - pntMin.y + 1);
}

/////////////////////////////////////////////////////////////////////

CDib::ImgEdge::ImgEdge(BOOL bLeftSide, POINT* pSrcPnts, POINT* pDstPnts,
    int nTopPnt, int nBotPnt)
{
    m_pDPnts = pDstPnts;
    m_pSPnts = pSrcPnts;
    m_nCurPnt = nTopPnt;
    m_nBotPnt = nBotPnt;
    m_nPntIncr = bLeftSide ? -1 : 1;
}

/////////////////////////////////////////////////////////////////////

BOOL CDib::ImgEdge::SetupNextEdge()
{
    int nBasePnt = m_nCurPnt;       // Seed the line's base point.
    while (1)
    {
        if (nBasePnt == m_nBotPnt)
            return FALSE;               // All done.
        int nNextPnt = nBasePnt + m_nPntIncr;
        if (nNextPnt >= numPnts)
            nNextPnt = 0;
        else if (nNextPnt < 0)
            nNextPnt = numPnts - 1;
        m_nLines = m_pDPnts[nNextPnt].y - m_pDPnts[nBasePnt].y;
        if (m_nLines > 0)       // Filters out slopes of zero
        {
            m_nCurPnt = nNextPnt;

            m_srcX.Setup(m_pSPnts[nBasePnt].x, m_pSPnts[nNextPnt].x, m_nLines);
            m_srcY.Setup(m_pSPnts[nBasePnt].y, m_pSPnts[nNextPnt].y, m_nLines);

            m_dstX.Setup(m_pDPnts[nBasePnt].x, m_pDPnts[nNextPnt].x, m_nLines);

            // Make sure only last segment does last line.
            if (m_pDPnts[m_nCurPnt].y == m_pDPnts[m_nBotPnt].y)
                m_nLines++;

            return TRUE;
        }
        nBasePnt = nNextPnt;
    }
}

/////////////////////////////////////////////////////////////////////

BOOL CDib::ImgEdge::NextScanLine()
{
    if (--m_nLines == 0)
        return SetupNextEdge();

    m_srcX++;
    m_srcY++;

    m_dstX++;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

namespace {
void RotatePoints(POINT* pPnts, int nPnts, int nDegrees)
{
    // Do it cheap and EZ at first....
    // Generate scaled integers (sin and cos: 0 <= sin(a) <= 1).
    // Scale by 1000.
    int nSin = Sin10K(nDegrees);
    int nCos = Cos10K(nDegrees);
    for (int i = 0; i < nPnts; i++)
        RotatePoint(pPnts[i], nSin, nCos);
}
}

/////////////////////////////////////////////////////////////////////

// Note: Cosine and Sine are scaled by 10000 to make them integers.
// Rotation is Counter Clockwise rather than the CW convention.

static void RotatePoint(POINT& pt, int nSin, int nCos)
{
    int x = pt.x;
    int y = pt.y;
    pt.x = (int)((x * (long)nCos + y * (long)nSin) / 10000);
    pt.y = (int)((-x * (long)nSin + y * (long)nCos) / 10000);
}


