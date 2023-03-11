// MapStrng.h
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

#ifndef _MAPSTRNG_H
#define _MAPSTRNG_H

#ifndef     __AFXTEMPL_H__
#include    <afxtempl.h>
#endif

#include    "Pieces.h"
#include    "Marks.h"
#include    "Versions.h"

#if defined(GPLAY)
#include    "DrawObj.h"
#endif

//////////////////////////////////////////////////////////////////////

class GameElement32;
class GameElement64;
using GameElement = std::conditional_t<std::is_same_v<TileID, TileID16>, GameElement32, GameElement64>;

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
class alignas(uint32_t) GameElement32
{
public:
    typedef uint32_t UNDERLYING_TYPE;

    // uninitialized data
    GameElement32() = default;
    explicit GameElement32(PieceID16 pid, unsigned nSide = unsigned(0))
        {
            new (&u.pieceElement) U::PieceElement(pid, nSide);
            ASSERT(IsAPiece() && !IsAMarker() && !IsAnObject());
            static_assert(sizeof(int32_t) == sizeof(*this), "need to adjust cast");
            ASSERT(reinterpret_cast<int32_t&>(*this) != INT32_C(-1));
        }
    explicit GameElement32(MarkID16 mid)
        {
            new (&u.markerElement) U::MarkerElement(mid);
            ASSERT(!IsAPiece() && IsAMarker() && !IsAnObject());
            static_assert(sizeof(int32_t) == sizeof(*this), "need to adjust cast");
            ASSERT(reinterpret_cast<int32_t&>(*this) != INT32_C(-1));
        }
#if defined(GPLAY)
    explicit GameElement32(ObjectID32 oid)
        {
            new (&u.objectElement) U::ObjectElement(oid);
            ASSERT(!IsAPiece() && !IsAMarker() && IsAnObject());
            static_assert(sizeof(int32_t) == sizeof(*this), "need to adjust cast");
            ASSERT(reinterpret_cast<int32_t&>(*this) != INT32_C(-1));
        }
#endif
    GameElement32(const GameElement32&) = default;
    GameElement32& operator=(const GameElement32&) = default;
    ~GameElement32() = default;

    bool operator==(const GameElement32& rhs) const
    {
        if (u.tag.tag != rhs.u.tag.tag) {
            return false;
        }
        else if (u.buf == GameElement32(Invalid_t()).u.buf &&
                rhs.u.buf == GameElement32(Invalid_t()).u.buf)
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
    bool operator!=(const GameElement32& rhs) const
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
                *this != GameElement32(Invalid_t());
    }

    explicit operator MarkID16() const
    {
        if (!IsAMarker()) {
            CbThrowBadCastException();
        }
        return u.markerElement.mid;
    }

    explicit operator PieceID16() const
    {
        if (!IsAPiece()) {
            CbThrowBadCastException();
        }
        return u.pieceElement.pid;
    }

    uint8_t GetSide() const
    {
        if (!IsAPiece()) {
            CbThrowBadCastException();
        }
        return static_cast<uint8_t>(u.pieceElement.nSide);
    }

#if defined(GPLAY)
    explicit operator ObjectID32() const
    {
        if (!IsAnObject()) {
            CbThrowBadCastException();
        }
        return u.objectElement.oid;
    }
#endif

    void Serialize(CArchive& ar) const;
    void Serialize(CArchive& ar);

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
            PieceID16 pid;
            uint16_t nSide : 1;
            uint16_t tag : 15;

            PieceElement(PieceID16 p, unsigned s) :
                pid(p),
                nSide(static_cast<uint16_t>(s)),
                tag(PIECE)
            {
                if (s > unsigned(1))
                {
                    ASSERT(!"side out of range");
                    CbThrowBadCastException();
                }
            }
        // Invalid_v<GameElement> helper
        private:
            constexpr PieceElement() = default;
        } pieceElement;
        struct MarkerElement
        {
            MarkID16 mid;
            uint16_t pad : 1;
            uint16_t tag : 15;

            MarkerElement(MarkID16 m) :
                mid(m),
                pad(0),     // Hash() won't work properly if bits uninitialized
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
            ObjectID32 oid;

            ObjectElement(ObjectID32 o) :
                oid(o)
            {
            }
        // Invalid_v<GameElement> helper
        private:
            constexpr ObjectElement() = default;
        } objectElement;
#endif
        UNDERLYING_TYPE buf;

        U() {}
        constexpr U(Invalid_t) : buf(static_cast<UNDERLYING_TYPE>(INT32_C(-1))) {};
        U(const U&) = default;
        U& operator=(const U&) = default;
        ~U() = default;
    } u;

    // helper for Invalid_v<GameElement>
    constexpr GameElement32(Invalid_t) : u(Invalid_t()) {}

#if !defined(NDEBUG)
    void Test()
    {
        static_assert(sizeof(std::remove_reference_t<decltype(*this)>) == sizeof(GameElementLegacyCheck), "size mismatch");
        static_assert(alignof(std::remove_reference_t<decltype(*this)>) == alignof(GameElementLegacyCheck), "align mismatch");
    }
#endif

    // Serialize conversion helpers
    explicit operator GameElement64() const;
    friend class SerializeBackdoorGameElement;

    friend Invalid<GameElement32>;

    friend std::formatter<GameElement32, char>;
    friend std::formatter<GameElement32, wchar_t>;
};

template<>
struct Invalid<GameElement32>
{
    static constexpr GameElement32 value = GameElement32(GameElement32::Invalid_t());
};

template<typename CharT>
struct std::formatter<GameElement32, CharT> : private std::formatter<GameElement32::UNDERLYING_TYPE, CharT>
{
private:
    using BASE = formatter<GameElement32::UNDERLYING_TYPE, CharT>;
public:
    using BASE::parse;

    template<typename FormatContext>
    FormatContext::iterator format(GameElement32 ge, FormatContext& ctx)
    {
        return BASE::format(ge.u.buf, ctx);
    }
};

inline CArchive& operator<<(CArchive& ar, const GameElement32& ge)
{
    ge.Serialize(ar);
    return ar;
}

inline CArchive& operator>>(CArchive& ar, GameElement32& ge)
{
    ge.Serialize(ar);
    return ar;
}

template<>
inline UINT HashKey(GameElement32 key)
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
inline GameElementLegacyCheck MakePieceElementLegacyCheck(PieceID16 pid, unsigned nSide = unsigned(0))
    { return (GameElementLegacyCheck)(static_cast<WORD>(pid) | (DWORD)nSide << 16); }
#endif
inline GameElement32 MakePieceElement(PieceID16 pid, unsigned nSide)
    {
        GameElement32 retval(pid, nSide);
#if !defined(NDEBUG)
        GameElementLegacyCheck legacyCheck = MakePieceElementLegacyCheck(pid, nSide);
        ASSERT(reinterpret_cast<GameElementLegacyCheck&>(retval) == legacyCheck);
#endif
        return retval;
    }

#if !defined(NDEBUG)
inline GameElementLegacyCheck MakeMarkerElementLegacyCheck(MarkID16 mid)
    { return (GameElementLegacyCheck)((DWORD)static_cast<WORD>(mid) | GAMEELEM_MARKERID_FLAG); }
#endif
inline GameElement32 MakeMarkerElement(MarkID16 mid)
    {
        GameElement32 retval(mid);
#if !defined(NDEBUG)
        GameElementLegacyCheck legacyCheck = MakeMarkerElementLegacyCheck(mid);
        ASSERT(reinterpret_cast<GameElementLegacyCheck&>(retval) == legacyCheck);
#endif
        return retval;
    }

#if defined(GPLAY)
#if !defined(NDEBUG)
inline GameElementLegacyCheck MakeObjectIDElementLegacyCheck(ObjectID32 dwObjectID)
    { return (GameElementLegacyCheck)reinterpret_cast<uint32_t&>(dwObjectID); }
#endif
inline GameElement32 MakeObjectIDElement(ObjectID32 dwObjectID)
    {
        GameElement32 retval(dwObjectID);
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
inline BOOL IsGameElementAPiece(GameElement32 elem)
    {
        BOOL retval = elem.IsAPiece();
        ASSERT(retval == IsGameElementLegacyCheckAPiece(reinterpret_cast<const GameElementLegacyCheck&>(elem)));
        return retval;
    }

#if !defined(NDEBUG)
inline BOOL IsGameElementLegacyCheckAMarker(GameElementLegacyCheck elem)
    { return (BOOL)((elem & GAMEELEM_OBJECTID_MASK) == GAMEELEM_MARKERID_FLAG); }
#endif
inline BOOL IsGameElementAMarker(GameElement32 elem)
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
inline BOOL IsGameElementAnObjectID(GameElement32 elem)
    {
        BOOL retval = elem.IsAnObject();
        ASSERT(retval == IsGameElementLegacyCheckAnObject(reinterpret_cast<const GameElementLegacyCheck&>(elem)));
        return retval;
    }

#if !defined(NDEBUG)
inline PieceID16 GetPieceIDFromElementLegacyCheck(GameElementLegacyCheck elem)
    { return static_cast<PieceID16>(elem & 0xFFFF); }
#endif
inline PieceID16 GetPieceIDFromElement(GameElement32 elem)
    {
        PieceID16 retval(elem);
        ASSERT(retval == GetPieceIDFromElementLegacyCheck(reinterpret_cast<const GameElementLegacyCheck&>(elem)));
        return retval;
    }

#if defined(GPLAY)
#if !defined(NDEBUG)
ObjectID32 GetObjectIDFromElementLegacyCheck(GameElementLegacyCheck elem);
#endif
inline ObjectID32 GetObjectIDFromElement(GameElement32 elem)
    {
        ObjectID32 retval(elem);
        ASSERT(retval == GetObjectIDFromElementLegacyCheck(reinterpret_cast<const GameElementLegacyCheck&>(elem)));
        return retval;
    }
#endif

class alignas(uint64_t) GameElement64
{
public:
    typedef uint64_t UNDERLYING_TYPE;

    // uninitialized data
    GameElement64() = default;
    explicit GameElement64(PieceID32 pid, unsigned nSide = unsigned(0))
    {
        new (&u.pieceElement) U::PieceElement(pid, nSide);
        ASSERT(IsAPiece() && !IsAMarker() && !IsAnObject());
        static_assert(sizeof(int64_t) == sizeof(*this), "need to adjust cast");
        ASSERT(reinterpret_cast<int64_t&>(*this) != INT64_C(-1));
    }
    explicit GameElement64(MarkID32 mid)
    {
        new (&u.markerElement) U::MarkerElement(mid);
        ASSERT(!IsAPiece() && IsAMarker() && !IsAnObject());
        static_assert(sizeof(int64_t) == sizeof(*this), "need to adjust cast");
        ASSERT(reinterpret_cast<int64_t&>(*this) != INT64_C(-1));
    }
#if defined(GPLAY)
    explicit GameElement64(ObjectID64 oid)
    {
        new (&u.objectElement) U::ObjectElement(oid);
        ASSERT(!IsAPiece() && !IsAMarker() && IsAnObject());
        static_assert(sizeof(int64_t) == sizeof(*this), "need to adjust cast");
        ASSERT(reinterpret_cast<int64_t&>(*this) != INT64_C(-1));
    }
#endif
    GameElement64(const GameElement64&) = default;
    GameElement64& operator=(const GameElement64&) = default;
    ~GameElement64() = default;

    bool operator==(const GameElement64& rhs) const
    {
        if (u.tag.tag != rhs.u.tag.tag) {
            return false;
        }
        else if (u.buf == GameElement64(Invalid_t()).u.buf &&
            rhs.u.buf == GameElement64(Invalid_t()).u.buf)
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
    bool operator!=(const GameElement64& rhs) const
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
            *this != GameElement64(Invalid_t());
    }

    explicit operator MarkID32() const
    {
        if (!IsAMarker()) {
            CbThrowBadCastException();
        }
        return u.markerElement.mid;
    }

    explicit operator PieceID32() const
    {
        if (!IsAPiece()) {
            CbThrowBadCastException();
        }
        return u.pieceElement.pid;
    }

    uint8_t GetSide() const
    {
        if (!IsAPiece()) {
            CbThrowBadCastException();
        }
        return static_cast<uint8_t>(u.pieceElement.nSide);
    }

#if defined(GPLAY)
    explicit operator ObjectID64() const
    {
        if (!IsAnObject()) {
            CbThrowBadCastException();
        }
        return u.objectElement.oid;
    }
#endif

    void Serialize(CArchive& ar) const;
    void Serialize(CArchive& ar);

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
            uint32_t : 32;
            uint32_t : 17;
            uint32_t tag : 15;
            // Invalid_v<GameElement> helper
        private:
            constexpr Tag() = default;
        } tag;
        struct PieceElement
        {
            PieceID32 pid;
            // allow for 100 sides for future development
            uint32_t nSide : 7;
            uint32_t pad : 10;
            uint32_t tag : 15;

            PieceElement(PieceID32 p, unsigned s) :
                pid(p),
                nSide(s),
                pad(0),     // Hash() won't work properly if bits uninitialized
                tag(PIECE)
            {
                if (s > unsigned(127))
                {
                    ASSERT(!"side out of range");
                    CbThrowBadCastException();
                }
            }
            // Invalid_v<GameElement> helper
        private:
            constexpr PieceElement() = default;
        } pieceElement;
        struct MarkerElement
        {
            MarkID32 mid;
            uint32_t pad : 17;
            uint32_t tag : 15;

            MarkerElement(MarkID32 m) :
                mid(m),
                pad(0),     // Hash() won't work properly if bits uninitialized
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
            ObjectID64 oid;

            ObjectElement(ObjectID64 o) :
                oid(o)
            {
            }
            // Invalid_v<GameElement> helper
        private:
            constexpr ObjectElement() = default;
        } objectElement;
#endif
        UNDERLYING_TYPE buf;

        U() {}
        constexpr U(Invalid_t) : buf(static_cast<UNDERLYING_TYPE>(INT64_C(-1))) {};
        U(const U&) = default;
        U& operator=(const U&) = default;
        ~U() = default;
    } u;

    // helper for Invalid_v<GameElement>
    constexpr GameElement64(Invalid_t) : u(Invalid_t()) {}

    // Serialize conversion helpers
    explicit operator GameElement32() const;
    friend class SerializeBackdoorGameElement;

    friend Invalid<GameElement64>;
};

template<>
struct Invalid<GameElement64>
{
    static constexpr GameElement64 value = GameElement64(GameElement64::Invalid_t());
};

template<typename CharT>
struct std::formatter<GameElement64, CharT>
{
public:
    template<typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
        typename ParseContext::iterator begin = ctx.begin();
        typename ParseContext::iterator retval = fmtUL.parse(ctx);
        ASSERT(ctx.begin() == begin);
        VERIFY(fmtPID.parse(ctx) == retval);
        ASSERT(ctx.begin() == begin);
        VERIFY(fmtMID.parse(ctx) == retval);
#if defined(GPLAY)
        ASSERT(ctx.begin() == begin);
        VERIFY(fmtOID.parse(ctx) == retval);
#endif
        return retval;
    }

    template<typename FormatContext>
    FormatContext::iterator format(GameElement64 ge, FormatContext& ctx)
    {
        if (ge.IsAPiece())
        {
            std::format_to(ctx.out(), "GameElement64(pid:");
            fmtPID.format(static_cast<PieceID32>(ge), ctx);
            std::format_to(ctx.out(), ", side:");
            fmtUL.format(ge.GetSide(), ctx);
            return std::format_to(ctx.out(), ")");
        }
        else if (ge.IsAMarker())
        {
            std::format_to(ctx.out(), "GameElement64(mid:");
            fmtMID.format(static_cast<MarkID32>(ge), ctx);
            return std::format_to(ctx.out(), ")");
        }
        else
        {
#if defined(GPLAY)
            ASSERT(ge.IsAnObject());
            std::format_to(ctx.out(), "GameElement64(");
            fmtOID.format(static_cast<ObjectID64>(ge), ctx);
            return std::format_to(ctx.out(), ")");
#else
            ASSERT(!"illegal GameElement64 subtype");
            std::format_to(ctx.out(), "GameElement64(unknown:");
            fmtUL.format(ge.u.buf, ctx);
            return std::format_to(ctx.out(), ")");
#endif
        }
    }

private:
    formatter<GameElement64::UNDERLYING_TYPE, CharT> fmtUL;
    formatter<PieceID32, CharT> fmtPID;
    formatter<MarkID32, CharT> fmtMID;
#if defined(GPLAY)
    formatter<ObjectID64, CharT> fmtOID;
#endif
};

inline CArchive& operator<<(CArchive& ar, const GameElement64& ge)
{
    ge.Serialize(ar);
    return ar;
}

inline CArchive& operator>>(CArchive& ar, GameElement64& ge)
{
    ge.Serialize(ar);
    return ar;
}

template<>
inline UINT HashKey(GameElement64 key)
{
    return key.Hash();
}

inline GameElement64 MakePieceElement(PieceID32 pid, unsigned nSide = unsigned(0))
{
    GameElement64 retval(pid, nSide);
    return retval;
}

inline GameElement64 MakeMarkerElement(MarkID32 mid)
{
    GameElement64 retval(mid);
    return retval;
}

#if defined(GPLAY)
inline GameElement64 MakeObjectIDElement(ObjectID64 dwObjectID)
{
    GameElement64 retval(dwObjectID);
    return retval;
}
#endif

inline BOOL IsGameElementAPiece(GameElement64 elem)
{
    BOOL retval = elem.IsAPiece();
    return retval;
}

inline BOOL IsGameElementAMarker(GameElement64 elem)
{
    BOOL retval = elem.IsAMarker();
    return retval;
}

inline BOOL IsGameElementAnObjectID(GameElement64 elem)
{
    BOOL retval = elem.IsAnObject();
    return retval;
}

inline PieceID32 GetPieceIDFromElement(GameElement64 elem)
{
    PieceID32 retval(elem);
    return retval;
}

#if defined(GPLAY)
inline ObjectID64 GetObjectIDFromElement(GameElement64 elem)
{
    ObjectID64 retval(elem);
    return retval;
}
#endif

class SerializeBackdoorGameElement : public SerializeBackdoor
{
public:
    using SerializeBackdoor::Convert;
    static GameElement64 Convert(const GameElement32& ge);
    static GameElement32 Convert(const GameElement64& ge);
};


//////////////////////////////////////////////////////////////////////
// This class is used to map rotated tiles related to playing pieces
// and markers to tile IDs.

template<typename KEY>
class CGameElementStringMapT : public CMap< KEY, KEY, CString, const CString& >
{
public:
    CGameElementStringMapT()
    {
        static_assert(std::is_same_v<KEY, GameElement32> || std::is_same_v<KEY, GameElement64>, "invalid key type");
    }

    void Serialize(CArchive& ar)
    {
        if (ar.IsStoring())
        {
            if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
            {
                uint32_t dwCount = value_preserving_cast<uint32_t>(this->GetCount());
                ar << dwCount;
            }
            else
            {
                CB::WriteCount(ar, value_preserving_cast<size_t>(this->GetCount()));
            }
            POSITION pos = this->GetStartPosition();
            while (pos != NULL)
            {
                KEY elem;
                CString str;
                this->GetNextAssoc(pos, elem, str);
                ar << elem;
                ar << str;
            }
        }
        else
        {
            this->RemoveAll();
            size_t count;
            if (!CB::GetFeatures(ar).Check(ftrSizet64Bit))
            {
                uint32_t dwCount;
                ar >> dwCount;
                count = dwCount;
            }
            else
            {
                count = CB::ReadCount(ar);
            }
            while (count--)
            {
                KEY dwElem;
                ar >> dwElem;
                CString str;
                ar >> str;
                this->SetAt(dwElem, str);
            }
        }
    }

    void Clone(const CGameElementStringMapT& pMapToCopy)
    {
        this->RemoveAll();

        POSITION pos = pMapToCopy.GetStartPosition();
        while (pos != NULL)
        {
            CString str;
            KEY elem;
            pMapToCopy.GetNextAssoc(pos, elem, str);
            this->SetAt(elem, str);
        }
    }

    BOOL Compare(const CGameElementStringMapT& pMapToCompare) const
    {
        if (this->GetCount() != pMapToCompare.GetCount())
            return FALSE;                       // Different sizes so no match
        POSITION pos = pMapToCompare.GetStartPosition();
        while (pos != NULL)
        {
            CString strToCompare;
            CString strOurs;
            KEY elem;
            pMapToCompare.GetNextAssoc(pos, elem, strToCompare);
            if (!Lookup(elem, strOurs))
                return FALSE;                   // Not found so no match
            if (strToCompare != strOurs)
                return FALSE;                   // values don't match
        }
        return TRUE;
    }
};

using CGameElementStringMap = CGameElementStringMapT<GameElement>;

#endif
