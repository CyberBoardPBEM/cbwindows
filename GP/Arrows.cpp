// Arrows.cpp
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
#error dead code
#include    "GMisc.h"

const int xArrowHeadSize = 4;
const int yArrowHeadSize = 4;

// If a point vector isn't supplied, one is allocated for you.

POINT* CalcRectBoundryArrow(CRect* pRctBound, int nDegrees,
    POINT* p4Pts /* = NULL */)
{
    if (p4Pts == NULL)
        p4Pts = new POINT[4];
    // Calculate size of arrow...
    int xHalf = pRctBound->Width() / 2;
    int yHalf = pRctBound->Height() / 2;
    int r1 = (int)(((long)yHalf * sin10K(nDegrees)) / 10000);
    int r2 = (int)(((long)xHalf * cos10K(nDegrees)) / 10000);
    int r = min(r1, r2);
    // Build arrow along x axis (0 degrees) at point (0, 0)
    p4Pts[0].x = p4Pts[0].y = 0;
    p4Pts[1].x = r;
    p4Pts[1].y = 0;
    p4Pts[2].x = r - xArrowHeadSize;
    p4Pts[2].y = yArrowHeadSize;
    p4Pts[3].x = r - xArrowHeadSize;
    p4Pts[3].y = -yArrowHeadSize;
    // Now rotate and translate to final position.
    RotatePoints(p4Pts, 4, nDegrees);
    OffsetPoints(p4Pts, 4, pRctBound.left + xHalf, pRctBound.top + yHalf);

    return p4Pts;
}

// If the crArrow argument isn't supplied, the pen is assumed to be selected.
void DrawArrow(CDC* pDC, POINT* p4Pts, COLORREF crArrow /* = noColor*/)
{
    if (crArrow != noColor)
    {
        CPen pnArrow(crArrow);
        CPen* pnPrv = pDC->SelectObject(&pnArrow);
        pDC->PolyLine(p4Pts, 3);
        pDC->MoveTo(p4Pts[1]);
        pDC->LineTo(p4Pts[3]);
        pDC->SelectObject(pnPrv);
    }
    else
    {
        pDC->PolyLine(p4Pts, 3);
        pDC->MoveTo(p4Pts[1]);
        pDC->LineTo(p4Pts[3]);
    }
}
