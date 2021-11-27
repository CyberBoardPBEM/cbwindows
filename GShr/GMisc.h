// GMisc.h
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

#ifndef _GMISC_H
#define _GMISC_H

#include <vector>

#include "MD5.h"

// *** CALCLIB.CPP *** //

void Compute16ByteHash(LPBYTE pDataToHash, int nDataLen, LPBYTE p16ByteHashCode);

void MD5Calc(MD5_CTX* ctx, BYTE* pMsg, int nMsgLen);

#ifdef GPLAY

int CalcRandomNumberUsingSeed(int nLow, UINT nRange, UINT nSeed,
    UINT* pnNextSeed = NULL);
int CalcRandomNumber(int nLow, UINT nRange,
    UINT* pnNextSeed = NULL);

void CalcRandomIndexVector(int nNumIndices, int nRange, UINT nSeed, int* pnIndices,
    UINT* pnNextSeed = NULL);

std::vector<int> AllocateAndCalcRandomIndexVector(int nNumIndices, int nRange, UINT nSeed,
    UINT* pnNextSeed = NULL);

#endif

inline int MidPnt(int a, int b) { return a+(b-a)/2; }
CPoint GetMidRect(const CRect& rct);
CPoint RotatePointAroundPoint(CPoint pntOrigin, CPoint pntXlate, int nAngleDeg);

int GridizeClosest1000(int nVal, int nMultiple, int nOffset);
int GridizeClosest(int nVal, int nMultiple, int nOffset);
BOOL StrDecimalChecked(const char **psp, int *pnVal, int *pnScale);
size_t CalcAllocSize(size_t uiDesired, size_t uiBaseSize, size_t uiExtendSize);
void ScalePoint(POINT& pnt, const CSize& numSize, const CSize& denSize);
void ScaleRect(RECT& rct, const CSize& numSize, const CSize& denSize);
int NumInWordArray(CWordArray& array, int val);
WORD GetTimeBasedRandomNumber(BOOL bZeroAllowed = TRUE);
WORD GetStringBasedRandomNumber(LPCSTR str);
DWORD GetStringHash(LPCSTR str);
CWnd* GetWindowFromPoint(CPoint point);
void PushRectOntoScreen(RECT& rct);
int Sin10K(int angle);
int Cos10K(int angle);

// *** ARCLIB.CPP *** //

// Convert from pre-3.10 ownership serialized owner masks to 26 player masks
inline DWORD UPGRADE_OWNER_MASK(WORD wOldMask)
{
    DWORD dwNewMask = wOldMask & 0x8000 ? 0x80000000: 0;    // IMPORTANT: Must be in sync with player.h defs.
    wOldMask &= 0xFF;
    dwNewMask |= wOldMask == 0xFF ?  0x03FFFFFF : wOldMask;
    return dwNewMask;
}

void WriteArchivePoints(CArchive& ar, const POINT* pPnts, size_t nPnts);
void ReadArchivePoints(CArchive& ar, POINT* pPnts, size_t nPnts);

#endif

