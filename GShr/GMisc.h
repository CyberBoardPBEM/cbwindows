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

std::array<std::byte, 16> Compute16ByteHash(const void* pDataToHash, size_t nDataLen);

template<size_t N>
std::array<std::byte, N> Compute16ByteHash(const void* pDataToHash, size_t nDataLen)
{
    std::array<std::byte, 16> temp = Compute16ByteHash(pDataToHash, nDataLen);
    static_assert(N >= 16);
    std::array<std::byte, N> retval;
    static_assert(std::is_trivially_copyable_v<std::byte>);
    memcpy(retval.data(), temp.data(), temp.size()*sizeof(std::byte));
    return retval;
}

void MD5Calc(MD5_CTX* ctx, const void* pMsg, size_t nMsgLen);

#ifdef GPLAY

int32_t CalcRandomNumberUsingSeed(int32_t nLow, uint32_t nRange, uint32_t nSeed,
    uint32_t* pnNextSeed = NULL);
int32_t CalcRandomNumber(int32_t nLow, uint32_t nRange,
    uint32_t* pnNextSeed = NULL);

void CalcRandomIndexVector(std::vector<size_t>& pnIndices, size_t nRange, uint32_t nSeed,
    uint32_t* pnNextSeed = NULL);

std::vector<size_t> AllocateAndCalcRandomIndexVector(size_t nNumIndices, size_t nRange, uint32_t nSeed,
    uint32_t* pnNextSeed = NULL);

#endif

inline int MidPnt(int a, int b) { return a+(b-a)/2; }
CPoint GetMidRect(const CRect& rct);
CPoint RotatePointAroundPoint(CPoint pntOrigin, CPoint pntXlate, int nAngleDeg);

int GridizeClosest1000(int nVal, int nMultiple, int nOffset);
int GridizeClosest(int nVal, int nMultiple, int nOffset);
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
inline uint32_t UPGRADE_OWNER_MASK(uint16_t wOldMask)
{
    uint32_t dwNewMask = wOldMask & 0x8000 ? 0x80000000: 0;    // IMPORTANT: Must be in sync with player.h defs.
    wOldMask &= uint16_t(0xFF);
    dwNewMask |= wOldMask == 0xFF ? uint32_t(0x03FFFFFF) : wOldMask;
    return dwNewMask;
}

void WriteArchivePoints(CArchive& ar, const POINT* pPnts, size_t nPnts);
void ReadArchivePoints(CArchive& ar, POINT* pPnts, size_t nPnts);

#endif

