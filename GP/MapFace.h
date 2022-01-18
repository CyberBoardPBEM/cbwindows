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

class alignas(uint64_t) ElementState
{
public:
    // uninitialized data
    ElementState() = default;
    ElementState(PieceID p, uint16_t nFacingDegCW, uint8_t nSide) :
        marker(0),
        side(nSide),
        angle(nFacingDegCW),
        pad(0),
        pid(p)
    {
        if (nSide >= uint8_t(128) || nFacingDegCW >= uint16_t(360))
        {
            ASSERT(!"side or angle out of range");
            AfxThrowInvalidArgException();
        }
    }
    ElementState(MarkID m, uint16_t nFacingDegCW) :
        marker(1),
        angle(nFacingDegCW),
        side(0),
        pad(0),
        mid(m)
    {
        if (nFacingDegCW >= uint16_t(360))
        {
            ASSERT(!"=angle out of range");
            AfxThrowInvalidArgException();
        }
    }
    ElementState(const ElementState&) = default;
    ElementState& operator=(const ElementState&) = default;
    ~ElementState() = default;

    bool operator==(const ElementState& rhs) const
    {
        if (marker)
        {
            return rhs.marker &&
                    angle == rhs.angle &&
                    mid == rhs.mid;
        }
        else
        {
            return !rhs.marker &&
                    angle == rhs.angle && side == rhs.side &&
                    pid == rhs.pid;
        }
    }

    uint16_t GetFacing() const
    {
        return value_preserving_cast<uint16_t>(angle);
    }

    unsigned int Hash() const
    {
        return HashKey(reinterpret_cast<const uint64_t&>(*this));
    }

private:
    uint32_t marker : 1;
    uint32_t angle : 9;
    // allow for 100 sides for future development
    uint32_t side : 7;
    uint32_t pad : 15;
    union {
        PieceID pid;
        MarkID mid;
    };
};
static_assert(sizeof(ElementState) == sizeof(uint64_t) &&
                alignof(ElementState) == alignof(uint64_t), "ElementState does not match uint64_t");
template<>
inline unsigned int HashKey(ElementState key)
{
    return key.Hash();
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
