// MapStrng.h
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

#ifndef _MAPSTRNG_H
#define _MAPSTRNG_H

#ifndef     __AFXTEMPL_H__
#include    <afxtempl.h>
#endif

#include    "Pieces.h"
#include    "Marks.h"

#if defined(GPLAY)
#include    "DrawObj.h"
#endif

//////////////////////////////////////////////////////////////////////

typedef DWORD GameElement;

const DWORD GAMEELEM_MARKERID_FLAG = 0xF0000000L; // Top 4 bits set if a marker, else a piece
const DWORD GAMEELEM_OBJECTID_MASK = 0xFFFE0000L; // else...top 15 bits nonzero if objectID
                                                  // otherwise it's a piece ID with side code

inline GameElement MakePieceElement(PieceID pid, int nSide = 0)
    { return (GameElement)(pid | (DWORD)nSide << 16); }

inline GameElement MakeMarkerElement(MarkID mid)
    { return (GameElement)((DWORD)mid | GAMEELEM_MARKERID_FLAG); }

#if defined(GPLAY)
inline GameElement MakeObjectIDElement(ObjectID dwObjectID)
    { return (GameElement)reinterpret_cast<uint32_t&>(dwObjectID); }
#endif

inline BOOL IsGameElementAPiece(GameElement elem)
    { return !(BOOL)(elem & GAMEELEM_MARKERID_FLAG); }

inline BOOL IsGameElementAMarker(GameElement elem)
    { return (BOOL)((elem & GAMEELEM_OBJECTID_MASK) == GAMEELEM_MARKERID_FLAG); }

inline BOOL IsGameElementAnObjectID(GameElement elem)
    {
        DWORD dwVal = elem & GAMEELEM_OBJECTID_MASK;
        return (BOOL)(dwVal != 0 && dwVal != GAMEELEM_MARKERID_FLAG);
    }

inline PieceID GetPieceIDFromElement(GameElement elem)
    { return (PieceID)(elem & 0xFFFF); }

#if defined(GPLAY)
inline ObjectID GetObjectIDFromElement(GameElement elem)
    { return static_cast<ObjectID>(elem); }
#endif


//////////////////////////////////////////////////////////////////////
// This class is used to map rotated tiles related to playing pieces
// and markers to tile IDs.

class CGameElementStringMap : public CMap< GameElement, GameElement, CString, CString& >
{
public:
    CGameElementStringMap() {}

    void Serialize(CArchive& ar);
    void Clone(CGameElementStringMap* pMapToCopy);
    BOOL Compare(CGameElementStringMap* pMapToCompare);
};

#endif
