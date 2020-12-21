// CyberBoard.h
//
// Copyright (c) 2020 By Bill Su, All Rights Reserved.
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

#ifndef _CYBERBOARD_H
#define _CYBERBOARD_H

// use explicitly sized ints for portable file format control
#include <cinttypes>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <WinExt.h>

#if defined(max)
    #undef max
#endif
#if defined(min)
    #undef min
#endif

/////////////////////////////////////////////////////////////////////////////

/* Invalid_v<T> is used as the "no-value" value for type T.
    It will probably be set to std::numeric_limits<T>::max(). */

/* N.B.:  semantically, std::optional<T> is a better way to
    represent the absence of a T, but T values sometimes get used
    in C-style functions, so std::optional<T> is unavailable */

/* KLUDGE:  bool handles uintptr_t being same type as size_t.
    Code using Invalid<> should never specify second template
    argument; let its default value be used.

    Additional notes:
    In this code, size_t is used all over because that's the index
    type for std::vector.  (I am too lazy to use std::vector::size_type.)
    The original code frequently used -1 to indicate "no-value" for an
    index parameter.  I wanted to use std::numeric_limits<size_t>::max()
    for this purpose, but that was too long, so I declared a
    constexpr Invalid_size_t or something like that.  However, I
    found a spot in the code where uintptr_t seems more
    appropriate.  I could have declared Invalid_uintptr_t, but I
    felt like a situation like this cried out for using template
    Invalid.  The problem is that both size_t and uintptr_t are
    specified by the MSVC compiler as typedefs to the same C++
    fundamental type.  That means that attempting to explicitly
    instantiate Invalid<size_t> and Invalid<uintptr_t> is viewed
    by the compiler as conflicting instantiations.  To get
    around this problem, I added a second default template
    argument to Invalid<>, and have the default value be
    different for size_t and uintptr_t.  By making that second
    argument different, the compiler sees them as separate
    instantiations, and is satisfied.  */
template<typename T, bool = true>
struct Invalid
{
};

template<>
struct Invalid<uintptr_t>
{
    static constexpr uintptr_t value = std::numeric_limits<uintptr_t>::max();
};

// this explicit instantiation is only relevant if size_t != uintptr_t
template<>
struct Invalid<size_t, !std::is_same_v<size_t, uintptr_t>>
{
    static constexpr size_t value = std::numeric_limits<size_t>::max();
};

// convenience helper
template<typename T>
constexpr T Invalid_v = Invalid<T>::value;

/////////////////////////////////////////////////////////////////////////////

template<typename T>
T& CheckedDeref(T* p)
{
    ASSERT(p);
    if (!p) {
        AfxThrowInvalidArgException();
    }
    return *p;
}

/////////////////////////////////////////////////////////////////////////////

/* test whether DEST can represent the exact value of src,
    not just the bit pattern
    (e.g., sint8_t can't represent 130) */

template<typename DEST, typename SRC>
struct is_always_value_preserving
{
    static_assert(std::is_integral_v<DEST>,
                    "only integral types supported");
    static constexpr bool value = std::is_integral_v<DEST> &&
                                    std::is_integral_v<SRC> &&
                                    std::is_signed_v<DEST> == std::is_signed_v<SRC> &&
                                    sizeof(DEST) >= sizeof(SRC);
};

// convenience helper
template<typename DEST, typename SRC>
constexpr bool is_always_value_preserving_v = is_always_value_preserving<DEST, SRC>::value;

template<typename DEST, typename SRC>
constexpr std::enable_if_t<is_always_value_preserving_v<DEST, SRC>, bool> is_value_preserving(SRC /*src*/)
{
    return true;
}

template<typename DEST, typename SRC>
constexpr std::enable_if_t<!is_always_value_preserving_v<DEST, SRC>, bool> is_value_preserving(SRC src)
{
    if (std::is_signed_v<SRC> &&
        std::is_unsigned_v<DEST> &&
        src < SRC(0))
    {
        return false;
    }
    else if (std::is_unsigned_v<SRC> &&
        std::is_signed_v<DEST> &&
        static_cast<DEST>(src) < DEST(0))
    {
        return false;
    }
    else if (static_cast<SRC>(static_cast<DEST>(src)) != src)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/////////////////////////////////////////////////////////////////////////////

// TODO:  when drop MFC THROW/AfxThrow, use std::bad_cast
class CBadCastException : public CException
{
};

inline void __declspec(noreturn) CbThrowBadCastException()
{
    ASSERT(!"bad cast");
    THROW(new CBadCastException());
}

/////////////////////////////////////////////////////////////////////////////

// exception if DEST can't represent src
template<typename DEST, typename SRC>
constexpr std::enable_if_t<is_always_value_preserving_v<DEST, SRC>, DEST> value_preserving_cast(SRC src)
{
    return static_cast<DEST>(src);
}

template<typename DEST, typename SRC>
constexpr std::enable_if_t<!is_always_value_preserving_v<DEST, SRC>, DEST> value_preserving_cast(SRC src)
{
    if (!is_value_preserving<DEST>(src))
    {
        CbThrowBadCastException();
    }
    return static_cast<DEST>(src);
}

/////////////////////////////////////////////////////////////////////////////

namespace CB
{
    /* defining Invalid<> required removing the MS min/max
        macros.  On the other hand, std::min/max don't allow
        different argument types.  So provide our own min/max,
        and check for value preservation while we're at it. */
    template<typename LEFT, typename RIGHT>
    std::common_type_t<LEFT, RIGHT> max(LEFT left, RIGHT right)
    {
        typedef std::common_type_t<LEFT, RIGHT> CommonType;
        static_assert(is_always_value_preserving_v<CommonType, LEFT> &&
                        is_always_value_preserving_v<CommonType, RIGHT>,
                        "unsafe conversion not supported");
        CommonType left2(static_cast<CommonType>(left));
        CommonType right2(static_cast<CommonType>(right));
        return !(left2 < right2) ? left2 : right2;
    }

    template<typename LEFT, typename RIGHT>
    std::common_type_t<LEFT, RIGHT> min(LEFT left, RIGHT right)
    {
        typedef std::common_type_t<LEFT, RIGHT> CommonType;
        static_assert(is_always_value_preserving_v<CommonType, LEFT> &&
                        is_always_value_preserving_v<CommonType, RIGHT>,
                        "unsafe conversion not supported");
        CommonType left2(static_cast<CommonType>(left));
        CommonType right2(static_cast<CommonType>(right));
        return left2 < right2 ? left2 : right2;
    }
}

/////////////////////////////////////////////////////////////////////////////

template<char PREFIX_>
class XxxxID
{
public:
    static constexpr char PREFIX = PREFIX_;

    XxxxID() = default;
    // goal is 32bit ids, but currently ids are 16 bit
    explicit constexpr XxxxID(uint32_t i) : id(value_preserving_cast<uint16_t>(i)) {}

    template<typename T>
    explicit constexpr XxxxID(T i) : XxxxID(value_preserving_cast<uint32_t>(i)) { static_assert(std::is_integral_v<T>, "id must be an integer"); }

    ~XxxxID() = default;

    explicit constexpr operator WORD() const { return id; }

    bool operator==(const XxxxID& rhs) const { return id == rhs.id; }
    bool operator!=(const XxxxID& rhs) const { return !operator==(rhs); }

private:
    uint16_t id;
};

template<char PREFIX>
CArchive& operator<<(CArchive& ar, const XxxxID<PREFIX>& oid)
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    static_assert(sizeof(uint16_t) == sizeof(oid), "wrong serialize type");
    return ar << reinterpret_cast<const uint16_t&>(oid);
}

template<char PREFIX>
CArchive& operator>>(CArchive& ar, XxxxID<PREFIX>& oid)
{
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::writeOnly);
    }
    static_assert(sizeof(uint16_t) == sizeof(oid), "wrong serialize type");
    return ar >> reinterpret_cast<uint16_t&>(oid);
}

// KLUDGE:  use unique_ptr since CWordArray doesn't have move operators
template<char PREFIX>
std::unique_ptr<const CWordArray> ToCWordArray(const std::vector<XxxxID<PREFIX>>& v)
{
    std::unique_ptr<CWordArray> retval(new CWordArray);
    retval->SetSize(value_preserving_cast<INT_PTR>(v.size()));
    for (INT_PTR i = 0; i < retval->GetSize(); ++i)
    {
        (*retval)[i] = static_cast<WORD>(v[value_preserving_cast<size_t>(i)]);
    }
    return retval;
}

template<typename T>
std::vector<T> ToVector(const CWordArray& a)
{
    static_assert(std::is_same_v<T, XxxxID<T::PREFIX>>, "requires XxxxID<>");
    std::vector<T> retval;
    retval.resize(value_preserving_cast<size_t>(a.GetSize()));
    for (INT_PTR i = 0; i < a.GetSize(); ++i)
    {
        retval[value_preserving_cast<size_t>(i)] = static_cast<T>(a[i]);
    }
    return retval;
}

template<char PREFIX>
CArchive& operator<<(CArchive& ar, const std::vector<XxxxID<PREFIX>>& v)
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    // KLUDGE:  current file format uses MFC CWordArray
    const_cast<CWordArray*>(ToCWordArray(v).get())->Serialize(ar);
    return ar;
}

template<char PREFIX>
CArchive& operator>>(CArchive& ar, std::vector<XxxxID<PREFIX>>& v)
{
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::writeOnly);
    }
    // KLUDGE:  current file format uses MFC CWordArray
    CWordArray temp;
    temp.Serialize(ar);
    v = ToVector<XxxxID<PREFIX>>(temp);
    return ar;
}

template<typename DEST, char PREFIX>
constexpr std::enable_if_t<is_always_value_preserving_v<DEST, WORD>, DEST> value_preserving_cast(XxxxID<PREFIX> src)
{
    return static_cast<DEST>(static_cast<WORD>(src));
}

template<typename DEST, char PREFIX>
constexpr std::enable_if_t<!is_always_value_preserving_v<DEST, WORD>, DEST> value_preserving_cast(XxxxID<PREFIX> src)
{
    if (!is_value_preserving<DEST>(static_cast<WORD>(src)))
    {
        CbThrowBadCastException();
    }
    return static_cast<DEST>(static_cast<WORD>(src));
}

/////////////////////////////////////////////////////////////////////////////

/* Factor some repetitive code for tables indexed by ID values
    into a new XxxxIDTable<>. */
template<typename KEY, typename ELEMENT,
        size_t maxSize, size_t baseSize, size_t incrSize,
        bool saveInGPlay>
class XxxxIDTable
{
    static_assert(std::is_same_v<KEY, XxxxID<KEY::PREFIX>>, "requires XxxxID<>");
    /* data is stored in memory that may be realloc()'ed, so data must be memcpy()'able
        (if ELEMENT is not trivially copyable, use std::vector) */
    static_assert(std::is_trivially_copyable_v<ELEMENT>, "ELEMENT must be trivially copyable");
public:
    XxxxIDTable() noexcept
    {
        m_pTbl = NULL;
        m_nTblSize = 0;
    }

    XxxxIDTable(const XxxxIDTable& other) :
        XxxxIDTable()
    {
        *this = other;
    }

    XxxxIDTable& operator=(const XxxxIDTable& other)
    {
        ResizeTable(other.GetSize(), nullptr);
        /* ELEMENT is trivially copyable, so this is safe
            (and more efficient than for loop with assignments) */
        memcpy(m_pTbl, other.m_pTbl, GetSize()*sizeof(ELEMENT));
        return *this;
    }

    XxxxIDTable(XxxxIDTable&& other) noexcept
    {
        m_pTbl = other.m_pTbl;
        m_nTblSize = other.m_pTbl;
        other.m_pTbl = NULL;
        other.m_pTbl = 0;
    }

    XxxxIDTable& operator=(XxxxIDTable&& other) noexcept
    {
        if (m_pTbl != NULL) free(m_pTbl);
        m_pTbl = other.m_pTbl;
        m_nTblSize = other.m_pTbl;
        other.m_pTbl = NULL;
        other.m_pTbl = 0;
    }

    ~XxxxIDTable()
    {
        Clear();
    }

    bool operator==(nullptr_t) const { return Empty(); }
    bool operator!=(nullptr_t) const { return !operator==(nullptr); }
    bool Empty() const { return !m_pTbl; }
    size_t GetSize() const { return m_nTblSize; }
    bool Valid(KEY tid) const { return static_cast<WORD>(tid) < m_nTblSize; }

    const ELEMENT& operator[](KEY tid) const { return m_pTbl[static_cast<WORD>(tid)]; }
    ELEMENT& operator[](KEY tid) { return const_cast<ELEMENT&>(std::as_const(*this)[tid]); }

    void Clear()
    {
        if (m_pTbl != NULL) free(m_pTbl);
        m_pTbl = NULL;
        m_nTblSize = 0;
    }

    /* WARNING:  will need to be generalized if an ELEMENT
        without IsEmpty() wants to use this */
    KEY CreateIDEntry(void (ELEMENT::*initializer)())
    {
        // Allocate from empty entry if possible
        for (size_t i = 0; i < m_nTblSize; i++)
        {
            if (m_pTbl[i].IsEmpty())
                return static_cast<KEY>(i);
        }
        // Get TileID from end of table.
        if (m_nTblSize >= maxSize)
        {
            AfxThrowMemoryException();
        }
        KEY newXid = static_cast<KEY>(m_nTblSize);
        ResizeTable(m_nTblSize + 1, initializer);
        return newXid;
    }

    void ResizeTable(size_t nEntsNeeded, void (ELEMENT::*initializer)())
    {
        if (nEntsNeeded == 0)
        {
            Clear();
            return;
        }

        size_t nNewSize = CalcAllocSize(nEntsNeeded, baseSize, incrSize);
        if (m_pTbl != NULL)
        {
            ELEMENT* pNewTbl = (ELEMENT*)realloc(
                m_pTbl, nNewSize * sizeof(ELEMENT));
            if (pNewTbl == NULL)
                AfxThrowMemoryException();
            m_pTbl = pNewTbl;
        }
        else
        {
            m_pTbl = (ELEMENT*)malloc(nNewSize * sizeof(ELEMENT));
            if (m_pTbl == NULL)
                AfxThrowMemoryException();
        }
        if (initializer)
        {
            for (size_t i = m_nTblSize; i < nNewSize; i++)
                (m_pTbl[i].*initializer)();
        }
        m_nTblSize = nNewSize;
    }

    void Serialize(CArchive& ar)
    {
        if (ar.IsStoring())
        {
            constexpr bool inGplay =
#ifdef GPLAY
                true
#else
                false
#endif
                ;
            if (!inGplay || saveInGPlay)
            {
                ar << value_preserving_cast<WORD>(m_nTblSize);
                for (size_t i = 0; i < m_nTblSize; i++)
                    m_pTbl[i].Serialize(ar);
            }
        }
        else
        {
            WORD wTmp;
            ar >> wTmp;
            if (wTmp > 0)
            {
                ResizeTable(value_preserving_cast<size_t>(wTmp), nullptr);
                for (size_t i = 0; i < wTmp; i++)
                    m_pTbl[i].Serialize(ar);
            }
        }
    }

private:
    ELEMENT*    m_pTbl;         // Global def'ed
    size_t      m_nTblSize;     // Number of alloc'ed ents in tile table
};

template<typename KEY, typename ELEMENT,
        size_t maxSize, size_t baseSize, size_t incrSize,
        bool saveInGPlay>
CArchive& operator<<(CArchive& ar, const XxxxIDTable<KEY, ELEMENT, maxSize, baseSize, incrSize, saveInGPlay>& v)
{
    if (!ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::readOnly);
    }
    const_cast<XxxxIDTable<KEY, ELEMENT, maxSize, baseSize, incrSize, saveInGPlay>&>(v).Serialize(ar);
    return ar;
}

template<typename KEY, typename ELEMENT,
        size_t maxSize, size_t baseSize, size_t incrSize,
        bool saveInGPlay>
CArchive& operator>>(CArchive& ar, XxxxIDTable<KEY, ELEMENT, maxSize, baseSize, incrSize, saveInGPlay>& v)
{
    if (ar.IsStoring())
    {
        AfxThrowArchiveException(CArchiveException::writeOnly);
    }
    v.Serialize(ar);
    return ar;
}

/////////////////////////////////////////////////////////////////////////////

inline CArchive& operator<<(CArchive& ar, const std::string& s)
{
    CString cs = s.c_str();
    ar << cs;
    return ar;
}

inline CArchive& operator>>(CArchive& ar, std::string& s)
{
    CString cs;
    ar >> cs;
    s = cs;
    return ar;
}

/////////////////////////////////////////////////////////////////////////////

static_assert(std::is_same_v<std::vector<int>::size_type, size_t>, "std::vector index is not size_t");
static_assert(std::is_same_v<std::vector<int>::iterator::difference_type, ptrdiff_t>, "std::vector iterator offset is not ptrdiff_t");
#endif
