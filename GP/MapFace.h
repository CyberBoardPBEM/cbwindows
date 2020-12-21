// MapFace.h
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

#ifndef _MAPFACE_H
#define _MAPFACE_H

#ifndef     __AFXTEMPL_H__
#include    <afxtempl.h>
#endif

#ifndef     _PIECES_H
#include    "Pieces.h"
#endif

#ifndef     _MARKS_H
#include    "Marks.h"
#endif

//////////////////////////////////////////////////////////////////////
// ElementState's Bit Layout:
//   3         2         1
// 210987654321098765432109876543210
// M----IIIIIIIIIIIIIIII--SAAAAAAAAA
//
// A = Facing Angle in degrees
// S = Side up. 0=top/1=bottom. Markers = 0
// I = Piece or Marker ID code
// M = Marker flag. 1=marker

typedef DWORD ElementState;

const DWORD MARKER_ELEMENT_FLAG = 0x80000000L; // Top set if a marker, else a piece

inline ElementState MakePieceState(PieceID pid, WORD nFacingDegCW, BYTE nSide)
    { return ((DWORD)static_cast<WORD>(pid) << 12) | ((DWORD)nSide << 9) | (nFacingDegCW & 0x1FF); }

inline ElementState MakeMarkerState(MarkID mid, WORD nFacingDegCW)
    { return ((DWORD)static_cast<WORD>(mid) << 12) | (nFacingDegCW & 0x1FF) | MARKER_ELEMENT_FLAG; }

inline int GetElementFacingAngle(ElementState elem)
{
    return (int)(elem & 0x1FF);             // Degree angle is stored in LS nine bits
}

//////////////////////////////////////////////////////////////////////
// This class is used to map rotated tiles related to playing pieces
// and markers to tile IDs.

class CTileFacingMap : public CMap< ElementState, ElementState, TileID, TileID >
{
    CTileManager*   m_pTMgr;
    size_t          m_nTileSet;         // Our private tile set
public:
    CTileFacingMap() { m_pTMgr = NULL; }
    CTileFacingMap(CTileManager* pTileMgr) { m_pTMgr = NULL; SetTileManager(pTileMgr); }

    void    SetTileManager(CTileManager* pTileMgr);

    TileID  GetFacingTileID(ElementState state);
    TileID  CreateFacingTileID(ElementState state, TileID baseTileID);
};

#endif
