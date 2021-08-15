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

#if !defined(NDEBUG)
typedef DWORD GameElementLegacyCheck;
#endif
// N.B.:  ObjectID is sometimes reinterpreted as GameElement
/* WARNING:  Strictly speaking, I believe this code is depending
        on undefined behavior (it reads from elements of the
        union other than the one that was most recently
        written). However, I think the code is a lot more
        readable this way rather than doing this work using bit
        twiddling within an uintXX_t, which would be the
        well-defined approach.  */
class alignas(uint32_t) GameElement
{
public:
    // uninitialized data
    GameElement() = default;
    GameElement(PieceID pid, int nSide = 0)
        {
            new (&u.pieceElement) U::PieceElement(pid, nSide);
            ASSERT(IsAPiece() && !IsAMarker() && !IsAnObject());
            static_assert(sizeof(int) == sizeof(*this), "need to adjust cast");
            ASSERT(reinterpret_cast<int&>(*this) != -1);
        }
    GameElement(MarkID mid)
        {
            new (&u.markerElement) U::MarkerElement(mid);
            ASSERT(!IsAPiece() && IsAMarker() && !IsAnObject());
            static_assert(sizeof(int) == sizeof(*this), "need to adjust cast");
            ASSERT(reinterpret_cast<int&>(*this) != -1);
        }
#if defined(GPLAY)
    GameElement(ObjectID oid)
        {
            new (&u.objectElement) U::ObjectElement(oid);
            ASSERT(!IsAPiece() && !IsAMarker() && IsAnObject());
            static_assert(sizeof(int) == sizeof(*this), "need to adjust cast");
            ASSERT(reinterpret_cast<int&>(*this) != -1);
        }
#endif
    GameElement(const GameElement&) = default;
    GameElement& operator=(const GameElement&) = default;
    ~GameElement() = default;

    bool operator==(const GameElement& rhs) const
    {
        if (u.tag.tag != rhs.u.tag.tag) {
            return false;
        }
        else if (u.buf == GameElement(Invalid_t()).u.buf &&
                rhs.u.buf == GameElement(Invalid_t()).u.buf)
        {
            return true;
        }

        switch (u.tag.tag)
        {
            case PIECE:
                return u.pieceElement.pid == rhs.u.pieceElement.pid &&
                        u.pieceElement.nSide == rhs.u.pieceElement.nSide;
            case MARKER:
                return u.markerElement.mid == rhs.u.markerElement.mid;
            default:
#if defined(GPLAY)
                return u.objectElement.oid == rhs.u.objectElement.oid;
#else
                CbThrowBadCastException();
#endif
        }
    }
    bool operator!=(const GameElement& rhs) const
    {
        return !(*this == rhs);
    }

    bool IsAPiece() const
    {
        return u.tag.tag == PIECE;
    }
    bool IsAMarker() const
    {
        return u.tag.tag == MARKER;
    }
    bool IsAnObject() const
    {
        return u.tag.tag != PIECE &&
                u.tag.tag != MARKER &&
                *this != GameElement(Invalid_t());
    }

    explicit operator PieceID() const
    {
        if (!IsAPiece()) {
            CbThrowBadCastException();
        }
        return u.pieceElement.pid;
    }

#if defined(GPLAY)
    explicit operator ObjectID() const
    {
        if (!IsAnObject()) {
            CbThrowBadCastException();
        }
        return u.objectElement.oid;
    }
#endif

    void Serialize(CArchive& ar) const
    {
        static_assert(sizeof(std::remove_reference_t<decltype(*this)>) == sizeof(u.buf), "size mismatch");
        static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(decltype(u.buf)), "align mismatch");
        static_assert(std::is_trivially_copyable_v<std::remove_reference_t<decltype(*this)>>, "needs more complex serialize");
        if (!ar.IsStoring())
        {
            AfxThrowArchiveException(CArchiveException::readOnly);
        }
        ar << u.buf;
    }

    void Serialize(CArchive& ar)
    {
        static_assert(sizeof(std::remove_reference_t<decltype(*this)>) == sizeof(u.buf), "size mismatch");
        static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(decltype(u.buf)), "align mismatch");
        static_assert(std::is_trivially_copyable_v<std::remove_reference_t<decltype(*this)>>, "needs more complex serialize");
        if (ar.IsStoring())
        {
            AfxThrowArchiveException(CArchiveException::readOnly);
        }
        ar >> u.buf;
    }

    UINT Hash() const
    {
        return HashKey(u.buf);
    }

private:
    // Top 4 bits set if a marker, else a piece
    // else...top 15 bits nonzero if objectID
    // otherwise it's a piece ID with side code
    enum {
        PIECE = 0,
        MARKER = 0x7800,
    };
    // Invalid_v<GameElement> helper
    struct Invalid_t {};

    /* constexpr GameElement --> constexpr union
        constexpr union --> union can't be anonymous */
    union U {
        // WARNING:  bitfield layout is MS-specific
        struct Tag
        {
            uint32_t : 17;
            uint32_t tag : 15;
        // Invalid_v<GameElement> helper
        private:
            constexpr Tag() = default;
        } tag;
        struct PieceElement
        {
            PieceID pid;
            uint16_t nSide : 1;
            uint16_t tag : 15;

            PieceElement(PieceID p, int s) :
                pid(p),
                nSide(s),
                tag(PIECE)
            {
            }
        // Invalid_v<GameElement> helper
        private:
            constexpr PieceElement() = default;
        } pieceElement;
        struct MarkerElement
        {
            MarkID mid;
            uint16_t pad : 1;
            uint16_t tag : 15;

            MarkerElement(MarkID m) :
                mid(m),
                pad(0),
                tag(MARKER)
            {
            }
        // Invalid_v<GameElement> helper
        private:
            constexpr MarkerElement() = default;
        } markerElement;
#if defined(GPLAY)
        struct ObjectElement
        {
            ObjectID oid;

            ObjectElement(ObjectID o) :
                oid(o)
            {
            }
        // Invalid_v<GameElement> helper
        private:
            constexpr ObjectElement() = default;
        } objectElement;
#endif
        uint32_t buf;

        U() {}
        constexpr U(Invalid_t) : buf(uint32_t(-1)) {};
        U(const U&) = default;
        U& operator=(const U&) = default;
        ~U() = default;
    } u;

    // helper for Invalid_v<GameElement>
    constexpr GameElement(Invalid_t) : u(Invalid_t()) {}

#if !defined(NDEBUG)
    void Test()
    {
        static_assert(sizeof(std::remove_reference_t<decltype(*this)>) == sizeof(GameElementLegacyCheck), "size mismatch");
        static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(GameElementLegacyCheck), "align mismatch");
    }
#endif

    friend Invalid<GameElement>;
};

template<>
struct Invalid<GameElement>
{
    static constexpr GameElement value = GameElement(GameElement::Invalid_t());
};

inline CArchive& operator<<(CArchive& ar, const GameElement& ge)
{
    ge.Serialize(ar);
    return ar;
}

inline CArchive& operator>>(CArchive& ar, GameElement& ge)
{
    ge.Serialize(ar);
    return ar;
}

template<>
inline UINT HashKey(GameElement key)
{
    return key.Hash();
}

#if !defined(NDEBUG)
// TODO:  remove the *LegacyCheck functions after testing replacements for a while
const DWORD GAMEELEM_MARKERID_FLAG = 0xF0000000L; // Top 4 bits set if a marker, else a piece
const DWORD GAMEELEM_OBJECTID_MASK = 0xFFFE0000L; // else...top 15 bits nonzero if objectID
                                                  // otherwise it's a piece ID with side code
#endif

#if !defined(NDEBUG)
inline GameElementLegacyCheck MakePieceElementLegacyCheck(PieceID pid, int nSide = 0)
    { return (GameElementLegacyCheck)(static_cast<WORD>(pid) | (DWORD)nSide << 16); }
#endif
inline GameElement MakePieceElement(PieceID pid, int nSide = 0)
    {
        GameElement retval = GameElement(pid, nSide);
#if !defined(NDEBUG)
        GameElementLegacyCheck legacyCheck = MakePieceElementLegacyCheck(pid, nSide);
        ASSERT(reinterpret_cast<GameElementLegacyCheck&>(retval) == legacyCheck);
#endif
        return retval;
    }

#if !defined(NDEBUG)
inline GameElementLegacyCheck MakeMarkerElementLegacyCheck(MarkID mid)
    { return (GameElementLegacyCheck)((DWORD)static_cast<WORD>(mid) | GAMEELEM_MARKERID_FLAG); }
#endif
inline GameElement MakeMarkerElement(MarkID mid)
    {
        GameElement retval = GameElement(mid);
#if !defined(NDEBUG)
        GameElementLegacyCheck legacyCheck = MakeMarkerElementLegacyCheck(mid);
        ASSERT(reinterpret_cast<GameElementLegacyCheck&>(retval) == legacyCheck);
#endif
        return retval;
    }

#if defined(GPLAY)
#if !defined(NDEBUG)
inline GameElementLegacyCheck MakeObjectIDElementLegacyCheck(ObjectID dwObjectID)
    { return (GameElementLegacyCheck)reinterpret_cast<uint32_t&>(dwObjectID); }
#endif
inline GameElement MakeObjectIDElement(ObjectID dwObjectID)
    {
        GameElement retval = GameElement(dwObjectID);
#if !defined(NDEBUG)
        GameElementLegacyCheck legacyCheck = MakeObjectIDElementLegacyCheck(dwObjectID);
        ASSERT(reinterpret_cast<GameElementLegacyCheck&>(retval) == legacyCheck);
#endif
        return retval;
    }
#endif

#if !defined(NDEBUG)
inline BOOL IsGameElementLegacyCheckAPiece(GameElementLegacyCheck elem)
    { return !(BOOL)(elem & GAMEELEM_MARKERID_FLAG); }
#endif
inline BOOL IsGameElementAPiece(GameElement elem)
    {
        BOOL retval = elem.IsAPiece();
        ASSERT(retval == IsGameElementLegacyCheckAPiece(reinterpret_cast<const GameElementLegacyCheck&>(elem)));
        return retval;
    }

#if !defined(NDEBUG)
inline BOOL IsGameElementLegacyCheckAMarker(GameElementLegacyCheck elem)
    { return (BOOL)((elem & GAMEELEM_OBJECTID_MASK) == GAMEELEM_MARKERID_FLAG); }
#endif
inline BOOL IsGameElementAMarker(GameElement elem)
    {
        BOOL retval = elem.IsAMarker();
        ASSERT(retval == IsGameElementLegacyCheckAMarker(reinterpret_cast<const GameElementLegacyCheck&>(elem)));
        return retval;
    }

#if !defined(NDEBUG)
inline BOOL IsGameElementLegacyCheckAnObject(GameElementLegacyCheck elem)
    {
        DWORD dwVal = elem & GAMEELEM_OBJECTID_MASK;
        return (BOOL)(dwVal != 0 && dwVal != GAMEELEM_MARKERID_FLAG);
    }
#endif
inline BOOL IsGameElementAnObjectID(GameElement elem)
    {
        BOOL retval = elem.IsAnObject();
        ASSERT(retval == IsGameElementLegacyCheckAnObject(reinterpret_cast<const GameElementLegacyCheck&>(elem)));
        return retval;
    }

#if !defined(NDEBUG)
inline PieceID GetPieceIDFromElementLegacyCheck(GameElementLegacyCheck elem)
    { return static_cast<PieceID>(elem & 0xFFFF); }
#endif
inline PieceID GetPieceIDFromElement(GameElement elem)
    {
        PieceID retval = static_cast<PieceID>(elem);
        ASSERT(retval == GetPieceIDFromElementLegacyCheck(reinterpret_cast<const GameElementLegacyCheck&>(elem)));
        return retval;
    }

#if defined(GPLAY)
#if !defined(NDEBUG)
ObjectID GetObjectIDFromElementLegacyCheck(GameElementLegacyCheck elem);
#endif
inline ObjectID GetObjectIDFromElement(GameElement elem)
    {
        ObjectID retval = static_cast<ObjectID>(elem);
        ASSERT(retval == GetObjectIDFromElementLegacyCheck(reinterpret_cast<const GameElementLegacyCheck&>(elem)));
        return retval;
    }
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
